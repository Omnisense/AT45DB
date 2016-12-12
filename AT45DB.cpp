/* 
 * @file    AT45DB.c
 * @brief   Device driver - Adesto AT45DB serial flash driver (low level)
 * @author  David Bartlett
 * @version 1.0
 * Copyright (c) 2016 Omnisense Limited (www.omnisense.co.uk)
 *
 * Licensed under the Apache Licence, Version 2.0 (the "Licence");
 * you may not use this file except in compliance with the Licence.
 * You may obtain a copy of the Licence at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 */

#include "AT45DB.h"

/* 
 * Its 17,301,504 bits of memory are organized as 4,096 pages of 
 * 512 bytes or 528 bytes each. In addition to the main memory, 
 * the AT45DB161E also contains two SRAM buffers of 512/528 bytes 
 * each. The buffers allow receiving of data while a page in the 
 * main memory is being reprogrammed.
 *
 * NOTE: All instructions, addresses, and data are 
 * transferred with the Most Significant
 * Bit (MSB) first.
 */

#include "mbed_debug.h"

#define AT45DB_DEBUG 1

AT45DB::AT45DB(PinName mosi, PinName miso, PinName sclk, PinName cs) :
        _at45spi(mosi, miso, sclk), _at45cs(cs) 
{ 
    _at45id = AT45DB::init();
    return;
}
 
AT45DB::~AT45DB() { }

unsigned int AT45DB::init(void)
{
    unsigned int at45dbid = 0;
    
    // set CS high
    _at45cs = AT45_CS_HIGH ;
    
    // set up frequency for serial flash SPI
    _at45spi.frequency(AT45_SPI_FREQ); 
    
    // read device ID
    at45dbid = AT45DB::at45_get_id();
    if (at45dbid == AT45DB161E_ID) {
#if AT45DB_DEBUG
        debug("AT45DB161E found\n");
#endif
    } else {
        at45dbid = 0;
#if AT45DB_DEBUG
        debug("SFlash wrong ID: %x\n",at45dbid);
#endif
    }
    
    // read status byte
    uint16_t status = AT45DB::at45_get_status();
    // configure for binary page size
    if (AT45_STATUS_BINARY(status)) {
#if AT45DB_DEBUG
        debug("AT45DB binary page size, SPI frequency %d\n",AT45_SPI_FREQ);
#endif
    } else {
        if (AT45DB::at45_set_pagesize_binary()) {
#if AT45DB_DEBUG
            debug("AT45DB binary page size\n");
#endif
        } else {
            at45dbid = 0;           // reset ID is page size not configured correctly
#if AT45DB_DEBUG
            debug("AT45DB NOT binary page size\n");
#endif
        }
    }
    
    return at45dbid;
}



/*

void SPI_MX25R::writeEnable(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_WREN) ;
    m_cs = CS_HIGH ;
}
 
void SPI_MX25R::writeDisable(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_WRDI) ;
    m_cs = CS_HIGH ;
}    
 
void SPI_MX25R::resetEnable(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_RSTEN) ;
    m_cs = CS_HIGH ;
}  
 
void SPI_MX25R::reset(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_RST) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::pgmersSuspend(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_PESUS) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::pgmersResume(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_PERES) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::deepPowerdown(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_DP) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::setBurstlength(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_SBL) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::releaseReadenhaced(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_RRE) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::noOperation(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_NOP) ;
    m_cs = CS_HIGH ;
} 
 
void SPI_MX25R::enterSecureOTP(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_ENSO) ;
    m_cs = CS_HIGH ;
}
 
void SPI_MX25R::exitSecureOTP(void)
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_EXSO) ;
    m_cs = CS_HIGH ;
} 
 
uint8_t SPI_MX25R::readStatus(void)
{   
    uint8_t data ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_RDSR) ;
    data = m_spi.write(DUMMY) ;                     // dummy
    m_cs = CS_HIGH ;
    return( data ) ;
}
  
uint32_t SPI_MX25R::readConfig(void)
{   
    uint8_t data;
    uint32_t config32 = 0 ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_RDCR) ;                         // send 15h
    data= m_spi.write(DUMMY)  ;                     // dumy to get 1st Byte out
    config32 = config32 | data ;                    // put in 32b reg
    data= m_spi.write(DUMMY) ;                      // dummy to get 2nd Byte out
    config32 = (config32 << 8) | data ;             // shift and put in reg
    m_cs = CS_HIGH ;
    return( config32 ) ;  
}

uint8_t SPI_MX25R::readSecurity(void)
{   
    uint8_t data ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_RDSCUR) ;                       // send 2Bh
    data = m_spi.write(DUMMY) ;                     // dummy
    m_cs = CS_HIGH ;
    return( data ) ;
}
  
uint32_t SPI_MX25R::readID(void)
{   
    uint8_t data;
    uint32_t data32 = 0 ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_RDID) ;                         // send 9Fh
    data= m_spi.write(DUMMY)  ;                     // dumy to get 1st Byte out
    data32 = data32 | data ;                        // put in 32b reg
    data= m_spi.write(DUMMY) ;                      // dummy to get 2nd Byte out
    data32 = (data32 << 8) | data ;                 // shift and put in reg
    data= m_spi.write(DUMMY)  ;                     // dummy to get 3rd Byte out
    data32 = (data32 << 8) | data ;                 // shift again and put in reg
    m_cs = CS_HIGH ;
    return( data32 ) ;  
}

uint32_t SPI_MX25R::readREMS(void)
{   
    uint8_t data;
    uint32_t data32 = 0 ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_REMS) ;                         // send 90h
    m_spi.write(DUMMY) ;                            // send DUMMY1
    m_spi.write(DUMMY) ;                            // send DUMMY2
    m_spi.write(0) ;                                // send address=0x00 to get Manu ID 1st.
    data= m_spi.write(DUMMY)  ;                     // dumy to get Manufacturer ID= C2h out
    data32 = data32 | data ;                        // put in 32b reg
    data= m_spi.write(DUMMY) ;                      // dummy to get 2nd Byte = Device ID out
    data32 = (data32 << 8) | data ;                 // shift and put in reg
    m_cs = CS_HIGH ;
    return( data32 ) ;  
}

uint8_t SPI_MX25R::readRES(void)
{   
    uint8_t data;
    m_cs = CS_LOW ;
    m_spi.write(CMD_RES) ;                          // send ABh
    m_spi.write(DUMMY) ;                            // send DUMMY1
    m_spi.write(DUMMY) ;                            // send DUMMY2
    m_spi.write(DUMMY) ;                            // send DUMMY3
    data= m_spi.write(DUMMY)  ;                     // dumy to get Electronic Sig. out
    m_cs = CS_HIGH ;
    return( data ) ;  
}
 
void SPI_MX25R::programPage(int addr, uint8_t *data, int numData)
{
    int i ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_PP) ;                           // Program Page 02h
    m_spi.write((addr >> 16)&0xFF) ;                // adr 23:16
    m_spi.write((addr >>  8)&0xFF) ;                // adr 15:8
    m_spi.write(addr & 0xFF) ;                      // adr 7:0
    for (i = 0 ; i < numData ; i++ ) {              // data = 00, 01, 02, .. to FEh, FFh = all 256 Bytes in 1 page. 
        m_spi.write(data[i]) ;
    }
    m_cs = CS_HIGH ;
    // poll in main
}
 
void SPI_MX25R::writeStatusreg(int addr)            // Write SR cmd 01h + 3B data
{   
    m_cs = CS_LOW ;
    m_spi.write(CMD_WRSR) ;                         // Write SR cmd 01h
    m_spi.write((addr >> 16)&0xFF) ;                // address
    m_spi.write((addr >>  8)&0xFF) ;
    m_spi.write(addr & 0xFF) ;
    m_cs = CS_HIGH ;
}

void SPI_MX25R::writeSecurityreg(int addr)          // WRSCUR cmd 2Fh + 1B data
{   
    m_cs = CS_LOW ;
    m_spi.write(CMD_WRSCUR) ;                         // Write SR cmd 01h
    m_spi.write(addr & 0xFF) ;
    m_cs = CS_HIGH ;
}

void SPI_MX25R::blockErase(int addr)                // 64KB Block Erase
{
    uint8_t data[3] ;
    data[0] = (addr >> 16) & 0xFF ;
    data[1] = (addr >> 8) & 0xFF ;
    data[2] = (addr & 0xFF) ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_BE) ;
    for (int i = 0 ; i < 3 ; i++ ) {                // Address setting
        m_spi.write(data[i]) ;
    }
    m_cs = CS_HIGH ;
    // poll in main
}
 
void SPI_MX25R::blockErase32KB(int addr)            // 32KB Block Erase
{
    uint8_t data[3] ;
    data[0] = (addr >> 16) & 0xFF ;
    data[1] = (addr >> 8) & 0xFF ;
    data[2] = (addr & 0xFF) ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_32KBE) ;
    for (int i = 0 ; i < 3 ; i++ ) {                // Address Setting
        m_spi.write(data[i]) ;
    }
    m_cs = CS_HIGH ;
    // poll in main
}
 
void SPI_MX25R::sectorErase(int addr)               //  4KB Sector Erase
{
    uint8_t data[3] ;
    data[0] = (addr >> 16) & 0xFF ;
    data[1] = (addr >> 8) & 0xFF ;
    data[2] = (addr & 0xFF) ;
    m_cs = CS_LOW ;
    m_spi.write(CMD_SE) ;
    for (int i = 0 ; i < 3 ; i++ ) {                // Address Setting
        m_spi.write(data[i]) ;
    }
    m_cs = CS_HIGH ;
    // poll in main
}
 
void SPI_MX25R::chipErase(void)                     // Chip Erase
{
    m_cs = CS_LOW ;
    m_spi.write(CMD_CE) ;
    m_cs = CS_HIGH ;
    // poll in main
}
 
uint8_t SPI_MX25R::read8(int addr)                  // Single Byte Read
{
    uint8_t data ;    
    m_cs = CS_LOW ;
    m_spi.write(CMD_READ) ;                         // send 03h
    m_spi.write((addr >> 16)&0xFF) ;
    m_spi.write((addr >>  8)&0xFF) ;
    m_spi.write(addr & 0xFF) ;
    data = m_spi.write(DUMMY) ;                     // write data is dummy 
    m_cs = CS_HIGH ;
    return( data ) ;                                // return 1 byte 
}
 
uint8_t SPI_MX25R::readSFDP(int addr)               // Read SFDP
{
    uint8_t data ;    
    m_cs = CS_LOW ;
    m_spi.write(CMD_RDSFDP) ;                       // send cmd 5Ah
    m_spi.write((addr >> 16)&0xFF) ;                // address[23:16]
    m_spi.write((addr >>  8)&0xFF) ;                // address[15:8]
    m_spi.write(addr & 0xFF) ;                      // address[7:0]
    m_spi.write(DUMMY) ;                            // dummy cycle
    data = m_spi.write(DUMMY) ;                     // return 1 byte 
    m_cs = CS_HIGH ;
    return( data ) ;
}
 
uint8_t SPI_MX25R::readFREAD(int addr)              // x1 Fast Read Data Byte
{
    uint8_t data ;    
    m_cs = CS_LOW ;
    m_spi.write(CMD_FREAD) ;                        // send cmd 0BH
    m_spi.write((addr >> 16)&0xFF) ;                // address[23:16]
    m_spi.write((addr >>  8)&0xFF) ;                // address[15:8]
    m_spi.write(addr & 0xFF) ;                      // address[7:0]
    m_spi.write(DUMMY) ;                            // dummy cycle
    data = m_spi.write(DUMMY) ;                     // return 1 byte 
    m_cs = CS_HIGH ;
    return( data ) ;
}

*/


/*
 * Return 16bit value with status byte1 in the upper byte 
 * and byte2 in the lower byte.
 */
uint16_t AT45DB::at45_get_status(void)
{
    uint16_t data;
    uint16_t dataval;
    
    _at45cs = AT45_CS_LOW ;
    _at45spi.write(AT45_STATUS_READ) ;
    dataval = _at45spi.write(DUMMY) ;              // first byte
    data = dataval << 8;
    dataval = _at45spi.write(DUMMY) ;              // second byte
    data |= dataval;
    _at45cs = AT45_CS_HIGH ;
    return data ;
}

/* Read the ID value from the chip.
 * Return 32 bit unsigned integer, 00, manufacturer, device family, device series
 */
unsigned int AT45DB::at45_get_id(void)
{
    unsigned int data32;
    unsigned int data;
    
    _at45cs = AT45_CS_LOW;
    _at45spi.write(AT45_ID_READ) ;
    data32 = _at45spi.write(DUMMY)  ;                   // dumy to get 1st Byte out
    data = _at45spi.write(DUMMY) ;                      // dummy to get 2nd Byte out
    data32 = (data32 << 8) | data ;                 // shift and put in reg
    data = _at45spi.write(DUMMY)  ;                     // dummy to get 3rd Byte out
    data32 = (data32 << 8) | data ;                 // shift again and put in reg
    _at45cs = AT45_CS_HIGH;
    _at45id = data32;
    return data32 ;
}

/*
 * The configured setting is stored in an internal nonvolatile 
 * register so that the buffer and page size configuration is 
 * not affected by power cycles. 
 *
 * NOTE: The nonvolatile register has a limit of 10,000 erase/program 
 * cycles; therefore, care should be taken to not switch
 * between the size options more than 10,000 times.
 *
 * return: TURE if in binary mode; otherwise FALSE.
 */
bool AT45DB::at45_set_pagesize_binary(void)
{
    uint16_t status;
    uint8_t devcmd[] = {AT45_BINARY_PAGE_FIRST_OPCODE,AT45_BINARY_PAGE};

    status = AT45DB::at45_get_status();
    if (!AT45_STATUS_BINARY(status)) {
        _at45cs = AT45_CS_LOW;
        _at45spi.write(devcmd[0]) ;
        _at45spi.write(devcmd[1]) ;
        _at45spi.write(devcmd[2]) ;
        _at45spi.write(devcmd[3]) ;
        _at45cs = AT45_CS_HIGH;
        do {
            status = AT45DB::at45_get_status();
        } while (!AT45_STATUS_READY(status));
    }
    return AT45_STATUS_BINARY(status);
}


/* 
 * Read data directly from a single page in the main memory, 
 * bypassing both of the data buffers and leaving the contents 
 * of the buffers unchanged.
 *
 * When the end of a page in main memory is reached, the device will
 * continue reading back at the beginning of the same page rather 
 * than the beginning of the next page.
 *
 * Opcode (D2h) + 3-byte address + 4-byte dummy
 *
bool_t at45_readpage(uint32 addr, uint8 *buff, uint32 size)
{
    uint8 opcode[8] = {AT45_PAGE_READ};
    struct lwspi_data spi_data;

    opcode[1] = (addr & 0x00ff0000) >> 16;
    opcode[2] = (addr & 0x0000ff00) >> 8;
    opcode[3] = (addr & 0x000000ff) >> 0;

    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = buff;
    spi_data.data_len = size;
    lwspi_dma_rx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_rx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}
*/
/*
 * With the Main Memory Page Program through Buffer with Built-In Erase command, 
 * data is first clocked into either Buffer 1 or Buffer 2, the addressed page in 
 * memory is then automatically erased, and then the contents of the appropriate 
 * buffer are programmed into the just-erased main memory page.
 *
 * When there is a low-to-high transition on the CS pin, the device will first 
 * erase the selected page in main memory (the erased state is a Logic 1) and 
 * then program the data stored in the buffer into that main memory page.
 *
 * Opcode (82h or 85h) + 3-byte address
 *
 * NOTE: 1. The 'addr' should always align with the boundary of a page, 
 *          otherwise the AT45's internal buffer may wrap.
 *       2. The 'buff' should always contain a whole page's data, 
 *          namely the 'size' should always be 512, otherwise
 *          uninitialised data in AT45's internal buffer would be 
 *          programmed into the Main Memory page.
 *
bool_t at45_writepage(uint32 addr, uint8 *buff, uint32 size)
{
    static bool_t at45_buffer1 = TRUE;
    struct lwspi_data spi_data;
    uint8 opcode[4];

    opcode[0] = at45_buffer1 ? AT45_PAGE_WRITE_BUF1 : AT45_PAGE_WRITE_BUF2;
    at45_buffer1 = !at45_buffer1;

    opcode[1] = (addr & 0x00ff0000) >> 16;
    opcode[2] = (addr & 0x0000ff00) >> 8;
    opcode[3] = (addr & 0x000000ff) >> 0;
    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = buff;
    spi_data.data_len = size;
    lwspi_dma_tx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_tx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}

static bool_t g_at45_buffer1 = TRUE;
bool_t at45_writebuffer(uint32 addr, uint8 *buff, uint32 size)
{
    struct lwspi_data spi_data;
    uint8 opcode[4];

    opcode[0] = g_at45_buffer1 ? AT45_BUFFER_WRITE_BUF1 : AT45_BUFFER_WRITE_BUF2;

    opcode[1] = (addr & 0x00ff0000) >> 16;
    opcode[2] = (addr & 0x0000ff00) >> 8;
    opcode[3] = (addr & 0x000000ff) >> 0;
    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = buff;
    spi_data.data_len = size;
    lwspi_dma_tx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_tx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}

bool_t at45_buffer2memory(uint32 addr)
{
    struct lwspi_data spi_data;
    uint8 opcode[4];

    opcode[0] = g_at45_buffer1 ? AT45_BUFFER_TO_MAIN_MEMORY_BUF1 : 
                                    AT45_BUFFER_TO_MAIN_MEMORY_BUF2;
    g_at45_buffer1 = !g_at45_buffer1;

    opcode[1] = (addr & 0x00ff0000) >> 16;
    opcode[2] = (addr & 0x0000ff00) >> 8;
    opcode[3] = (addr & 0x000000ff) >> 0;
    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = NULL;
    spi_data.data_len = 0;
    lwspi_dma_tx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_tx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}

bool_t at45_erasepage(uint32 addr)
{
    uint8 opcode[4] = {AT45_PAGE_ERASE};
    struct lwspi_data spi_data;

    opcode[1] = (addr & 0x00ff0000) >> 16;
    opcode[2] = (addr & 0x0000ff00) >> 8;
    opcode[3] = (addr & 0x000000ff) >> 0;
    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = NULL;
    spi_data.data_len = 0;
    lwspi_dma_tx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_tx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}
*/
/*
 * In ultra deep power down mode it consumes less than 1uA.
 * In ultra deep power down mode, all commands including the 
 * Status Register Read and Resume from Deep Power-Down commands
 * will be ignored.
 *
bool_t at45_ultra_deep_pwrdown_enter(void)
{
    uint8 opcode[1] = {AT45_ULTRA_DEEP_PDOWN};
    struct lwspi_data spi_data;

    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    spi_data.header = opcode;
    spi_data.header_len = NUM_OF_MEMBERS(opcode);
    spi_data.data = NULL;
    spi_data.data_len = 0;
    lwspi_dma_tx(g_apl.sflash_spi_hdl, &spi_data);
    lwspi_dma_flush_tx(g_apl.sflash_spi_hdl);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}

bool_t at45_ultra_deep_pwrdown_exit(void)
{
    struct lwspi_data spi_data;
*/    
    /* 
     * exit from ultra deep power down mode by 
     * asserting CS pin for more than 20ns, 
     * deasserting the CS then wait for 120us.
     *
    lwspi_dma_cs_assert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    BUSY_WAIT_US(1);
    lwspi_dma_cs_deassert(g_apl.sflash_spi_hdl, BSPCFG_APP_SFLASH_SPI_CS);
    return TRUE;
}

bool_t at45_is_ready(void)
{
    uint16 status = at45_get_status();
    return AT45_STATUS_READY(status);
}

bool_t at45_is_ep_failed(void)
{
    uint16 status = at45_get_status();
    return AT45_STATUS_EP_ERROR(status);
}

*/
