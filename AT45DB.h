/* 
 * @file    AT45DB.h
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
 *
 */
 
/*
 * This driver library does not implement all available chip functions.
 * Specifically missing are:
 *      software reset
 *      sector protection, lockdown and security
 *      block, page, chip erase functions
 *      freeze sector, and OTP programming
 */
 
#ifndef _AT45DB_H_
#define _AT45DB_H_
 
#include "mbed.h"
#include "device.h"
 
/**
 * Adesto Serial Flash Low Power Memories
 * AT45DB Series SPI-Flash Memory - AT45DB161E, 16Mbit as basis
 */
#define AT45_CS_LOW         0                   // SPI CS# (Chip Select) Setting 
#define AT45_CS_HIGH        1                   // SPI CS# (Chip Select) Setting 
#define DUMMY               0x00                // Dummy byte which can be changed to any value
#define AT45_CHIP_ERASE     0x94, 0x80, 0x9A    // extended erase command bytes
#define AT45_BINARY_PAGE    0x2A, 0x80, 0xA6    // extended binary page command bytes

#ifndef MAX_SPI_CLK
#define MAX_SPI_CLK         8000000
#endif  // MAX_SPI_CLK
#define AT45_SPI_FREQ       (((MAX_SPI_CLK) < (16000000)) ? (MAX_SPI_CLK) : (16000000))         // SPI frequency

#define AT45_PAGE_SIZE      512

#define AT45DB161E_ID       0x1F2600            // device ID of standard device supported

/* 
 * status is 16-bit value with status byte1 in the 
 * upper byte and byte2 in the lower byte. 
 */
/// Returns 0x80 if the device is ready; otherwise 0.
#define AT45_STATUS_READY(status)       (((status) >> 8) & 0x80)
/// Returns the device ID code.
#define AT45_STATUS_ID(status)          (((status) >> 8) & 0x3c)
/// Returns 1 if the device is configured in binary page mode; otherwise 0.
#define AT45_STATUS_BINARY(status)      (((status) >> 8) & 0x01)
/// Returns 1 if erase or program operation failed; otherwise 0.
#define AT45_STATUS_EP_ERROR(status)    (((status) & 0xff) & 0x20)
/// Returns 1 if the manufacture and device ID are correct.
#define AT45_MANU_AND_DEVICE_ID(id)     ((id) == 0x1f260001)
 
class AT45DB 
{

public:

    /**
     *  @enum CMDCODES
     *  @brief The device command register table for the AT45DB
     */

    enum CMDCODES
    {
        AT45_PAGE_READ              = 0xD2,         /// Main memory page read command code.
        AT45_CONTINUOUS_READ_LEG    = 0xE8,         /// Continous array read (legacy) command code.
        AT45_CONTINUOUS_READ_LF     = 0x03,         /// Continous array read (low frequency) command code.
        AT45_CONTINUOUS_READ_LP     = 0x01,         /// Continous array read (low power) command code.
        AT45_CONTINUOUS_READ        = 0x0B,         /// Continous array read command code.
        AT45_BUF1_READ_LF           = 0xD1,         /// Buffer 1 read (low frequency) command code.
        AT45_BUF2_READ_LF           = 0xD3,         /// Buffer 2 read (low frequency) command code.
        AT45_BUF1_READ_SER          = 0xD4,         /// Buffer 1 read (serial) command code.
        AT45_BUF2_READ_SER          = 0xD6,         /// Buffer 2 read (serial) command code.
        AT45_BUF1_READ_8B           = 0x54,         /// Buffer 1 read (8-bit) command code.
        AT45_BUF2_READ_8B           = 0x56,         /// Buffer 2 read (8-bit) command code.
        AT45_BUF1_WRITE             = 0x84,         /// Buffer 1 write command code.
        AT45_BUF2_WRITE             = 0x87,         /// Buffer 2 write command code.
        AT45_BUF1_MEM_ERASE         = 0x83,         /// Buffer 1 to main memory page program with erase command code.
        AT45_BUF2_MEM_ERASE         = 0x86,         /// Buffer 2 to main memory page program with erase command code.
        AT45_BUF1_MEM_NOERASE       = 0x88,         /// Buffer 1 to main memory page program without erase command code.
        AT45_BUF2_MEM_NOERASE       = 0x89,         /// Buffer 2 to main memory page program without erase command code.
        AT45_PAGE_ERASE             = 0x81,         /// Page erase command code.
        AT45_BLOCK_ERASE            = 0x50,         /// Block erase command code.
        AT45_SECTOR_ERASE           = 0x7C,         /// Sector erase command code.
        AT45_CHIP_ERASE_FIRST       = 0xC7,         /// Chip erase command code.
        AT45_PAGE_WRITE_BUF1        = 0x82,         /// Main memory page program through buffer 1 command code.
        AT45_PAGE_WRITE_BUF2        = 0x85,         /// Main memory page program through buffer 2 command code.
        AT45_BUFFER_WRITE_BUF1      = 0x84,         /// Buffer Write to buffer 1 command code.
        AT45_BUFFER_WRITE_BUF2      = 0x87,         /// Buffer Write to buffer 2 command code.
        AT45_BUFFER_TO_MAIN_MEMORY_BUF1  = 0x83,    /// Buffer to Main memory page through buffer 1 command code.
        AT45_BUFFER_TO_MAIN_MEMORY_BUF2  = 0x86,    /// Buffer to Main memory page through buffer 2 command code.
        AT45_PAGE_BUF1_TX           = 0x53,         /// Main memory page to buffer 1 transfer command code.
        AT45_PAGE_BUF2_TX           = 0x55,         /// Main memory page to buffer 2 transfer command code.
        AT45_PAGE_BUF1_CMP          = 0x60,         /// Main memory page to buffer 1 compare command code.
        AT45_PAGE_BUF2_CMP          = 0x61,         /// Main memory page to buffer 2 compare command code.
        AT45_AUTO_REWRITE_BUF1      = 0x58,         /// Auto page rewrite through buffer 1 command code.
        AT45_AUTO_REWRITE_BUF2      = 0x59,         /// Auto page rewrite through buffer 2 command code.
        AT45_ULTRA_DEEP_PDOWN       = 0x79,         /// Ultra Deep power-down command code.
        AT45_DEEP_PDOWN             = 0xB9,         /// Deep power-down command code.
        AT45_RES_DEEP_PDOWN         = 0xAB,         /// Resume from deep power-down command code.
        AT45_STATUS_READ            = 0xD7,         /// Status register read command code.
        AT45_ID_READ                = 0x9F,         /// Manufacturer and device ID read command code.
        AT45_BINARY_PAGE_FIRST_OPCODE   = 0x3D,     /// Power-of-2 binary page size configuration command code.
    };

    /**
     * Adesto AT45DB Low Power and Wide Vcc SPI-Flash Memory Family 
     *
     * @param mosi = SPI_MOSI pin
     * @param miso = SPI_MISO pin
     * @param sclk = SPI_CLK pin
     * @param cs   = SPI_CS  pin
     */
    AT45DB(PinName mosi, PinName miso, PinName sclk, PinName cs) ;
     
    ~AT45DB() ;
     
    /*
     * Read status byte from Adesto AT45DB serial flash chip
     *
     * @return 16bit value with status byte1 in the upper byte and byte2 in the lower byte.
     */
    uint16_t at45_get_status(void);
    
    /* Read the ID value from the chip.
     *
     * @return 32 bit unsigned integer: 00, manufacturer, device family, device series
     */
    unsigned int at45_get_id(void);

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
     * @param addr = address from which to start reading
     * @param *buff = pointer to destination memory buffer
     * @param size = number of bytes to read
     * @return true = success
     */
    bool at45_readpage(uint32_t addr, uint8_t *buff, uint32_t size);

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
     * @param addr = address to start writing into flash
     * @param *buff = pointer to memory buffer to use as data source
     * @param size = buffer size, should always be 512 bytes
     * @return true = success
     */
    bool at45_writepage(uint32_t addr, uint8_t *buff, uint32_t size);

    /*
     * Writes data into the currently selected RAM buffer
     *
     * @param addr = destination address in RAM buffer (9 bits)
     * @param *buff = pointer to source in CPU memory space
     * @param size = size - number of bytes to be transferred
     * @return true = success
     */
    bool at45_writebuffer(uint32_t addr, uint8_t *buff, uint32_t size);

    /*
     * Writes pre-loaded buffer into flash page
     *
     * @param addr = destination page address in flash (low 9 bits = 0)
     * @return true = success
     */
    bool at45_buffer2memory(uint32_t addr);
    
    /*
     * Erases flash page
     *
     * @param addr = destination page address in flash (low 9 bits = 0)
     * @return true = success
     */
    bool at45_erasepage(uint32_t addr);

    /*
     * In ultra deep power down mode it consumes less than 1uA.
     * In ultra deep power down mode, all commands including the 
     * Status Register Read and Resume from Deep Power-Down commands
     * will be ignored.
     */
    bool at45_ultra_deep_pwrdown_enter(void);

    /* 
     * exit from ultra deep power down mode by 
     * asserting CS pin for more than 20ns, 
     * deasserting the CS then wait for 120us.
     * the RAM buffers are undefined after wake from deep power down
     */
    bool at45_ultra_deep_pwrdown_exit(void);

    /*
     * test for AT45DB chip ready
     */
    bool at45_is_ready(void);

    /*
     * test for erase failed status
     */
    bool at45_is_ep_failed(void);
 

private:

    SPI             _at45spi;
    DigitalOut      _at45cs;
    unsigned int    _at45id;
    bool            _at45_buffer = true;
    bool            _g_at45_buffer = true;
    
    /** Initialise the device and SPI
     *  Set to the power on reset conditions
     *  @return - ID of attached device, 0 = none configured
     */
    unsigned int init(void);

    /*
     * Set page size to binary 512 bytes per page (chip default is 528)
     *
     * The configured setting is stored in an internal nonvolatile 
     * register so that the buffer and page size configuration is 
     * not affected by power cycles. 
     *
     * NOTE: The nonvolatile register has a limit of 10,000 erase/program 
     * cycles; therefore, care should be taken to not switch
     * between the size options more than 10,000 times.
     *
     * @return: TURE if in binary mode; otherwise FALSE.
     */
    bool at45_set_pagesize_binary(void);

};

#endif // _AT45DB_H_