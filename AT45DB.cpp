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
 */
bool AT45DB::at45_readpage(uint32_t addr, uint8_t *buff, uint32_t size)
{
    uint8_t     opcode[8];
    uint32_t    i;
    
    opcode[0] = AT45_PAGE_READ;
    opcode[1] = (uint8_t)((addr >> 16) & 0xff);
    opcode[2] = (uint8_t)((addr >> 8) & 0xff);
    opcode[3] = (uint8_t)(addr & 0xff);
    opcode[4] = opcode[5] = opcode[6] = opcode[7] = DUMMY;
    // now send command to chip and read back data
    _at45cs = AT45_CS_LOW;
    for (i=0; i<8; i++) {
        _at45spi.write(opcode[i]) ;
    }
    for (i=0; i<size; i++) {
        buff[i] = _at45spi.write(DUMMY) ;
    }
    _at45cs = AT45_CS_HIGH;
    return 1;
}

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
 */
bool AT45DB::at45_writepage(uint32_t addr, uint8_t *buff, uint32_t size)
{
    uint8_t     opcode[4];
    uint32_t    i;

    // load buffer code and toggle buffer
    opcode[0] = _at45_buffer ? AT45_PAGE_WRITE_BUF1 : AT45_PAGE_WRITE_BUF2;
    _at45_buffer = !_at45_buffer;
    opcode[1] = (uint8_t)((addr >> 16) & 0xff);
    opcode[2] = (uint8_t)((addr >> 8) & 0xff);
    opcode[3] = (uint8_t)(addr & 0xff);
    // now send data the chip
    _at45cs = AT45_CS_LOW;
    for (i=0; i<4; i++) {
        _at45spi.write(opcode[i]) ;
    }
    for (i=0; i<size; i++) {
        _at45spi.write(buff[i]) ;
    }
    _at45cs = AT45_CS_HIGH;
    return 1;
}

bool AT45DB::at45_writebuffer(uint32_t addr, uint8_t *buff, uint32_t size)
{
    uint8_t     opcode[4];
    uint32_t    i;

    opcode[0] = _g_at45_buffer ? AT45_BUFFER_WRITE_BUF1 : AT45_BUFFER_WRITE_BUF2;
    opcode[1] = (uint8_t)((addr >> 16) & 0xff);
    opcode[2] = (uint8_t)((addr >> 8) & 0xff);
    opcode[3] = (uint8_t)(addr & 0xff);
    // now send data the chip
    _at45cs = AT45_CS_LOW;
    for (i=0; i<4; i++) {
        _at45spi.write(opcode[i]) ;
    }
    for (i=0; i<size; i++) {
        _at45spi.write(buff[i]) ;
    }
    _at45cs = AT45_CS_HIGH;
    return 1;
}

bool AT45DB::at45_buffer2memory(uint32_t addr)
{
    uint8_t     opcode[4];
    uint32_t    i;

    opcode[0] = _g_at45_buffer ? AT45_BUFFER_TO_MAIN_MEMORY_BUF1 : AT45_BUFFER_TO_MAIN_MEMORY_BUF2;
    _g_at45_buffer = !_g_at45_buffer;
    opcode[1] = (uint8_t)((addr >> 16) & 0xff);
    opcode[2] = (uint8_t)((addr >> 8) & 0xff);
    opcode[3] = (uint8_t)(addr & 0xff);
    // send command to chip
    _at45cs = AT45_CS_LOW;
    for (i=0; i<4; i++) {
        _at45spi.write(opcode[i]) ;
    }
    _at45cs = AT45_CS_HIGH;
    return 1;
}

bool AT45DB::at45_erasepage(uint32_t addr)
{
    uint8_t     opcode[4];
    uint32_t    i;

    opcode[0] = AT45_PAGE_ERASE;
    opcode[1] = (uint8_t)((addr >> 16) & 0xff);
    opcode[2] = (uint8_t)((addr >> 8) & 0xff);
    opcode[3] = (uint8_t)(addr & 0xff);
    // send command to chip
    _at45cs = AT45_CS_LOW;
    for (i=0; i<4; i++) {
        _at45spi.write(opcode[i]) ;
    }
    _at45cs = AT45_CS_HIGH;
    return 1;
}

/*
 * In ultra deep power down mode it consumes less than 1uA.
 * In ultra deep power down mode, all commands including the 
 * Status Register Read and Resume from Deep Power-Down commands
 * will be ignored.
 */
bool AT45DB::at45_ultra_deep_pwrdown_enter(void)
{
    uint8_t     opcode[4];

    opcode [0] = AT45_ULTRA_DEEP_PDOWN;
    _at45cs = AT45_CS_LOW;
    _at45spi.write(opcode[0]) ;
    _at45cs = AT45_CS_HIGH;
    return 1;
}

/* 
 * exit from ultra deep power down mode by 
 * asserting CS pin for more than 20ns, 
 * deasserting the CS then wait for 120us.
 * the RAM buffers are undefined after wake from deep power down
 */
bool AT45DB::at45_ultra_deep_pwrdown_exit(void)
{
    _at45cs = AT45_CS_LOW;
    wait_us(1);                 // 1us
    _at45cs = AT45_CS_HIGH;
    Thread::wait(1);            // 1ms
    return 1;
}

bool AT45DB::at45_is_ready(void)
{
    uint16_t status = AT45DB::at45_get_status();
    return AT45_STATUS_READY(status);
}

bool AT45DB::at45_is_ep_failed(void)
{
    uint16_t status = AT45DB::at45_get_status();
    return AT45_STATUS_EP_ERROR(status);
}

