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

/**
 * MX25R Series Register Command Table. 
 * x2 and x4 commands not currently supported with FRDM K64F platform

#define CMD_READ      0x03  // x1 Normal Read Data Byte 
#define CMD_FREAD     0x0B  // x1 Fast Read Data Byte
#define CMD_2READ     0xBB  // x2 2READ 
#define CMD_DREAD     0x3B  // x2 DREAD 
#define CMD_4READ     0xEB  // x4 4READ 
#define CMD_QREAD     0x6B  // x4 QREAD 
#define CMD_PP        0x02  // Page Program 
#define CMD_4PP       0x38  // x4 PP
#define CMD_SE        0x20  // 4KB Sector Erase 
#define CMD_32KBE     0x52  // 32KB Block Erase 
#define CMD_BE        0xD8  // 64KB Block Erase 
#define CMD_CE        0xC7  // Chip Erase 
#define CMD_RDSFDP    0x5A  // Read SFDP 
#define CMD_WREN      0x06  // Write Enable 
#define CMD_WRDI      0x04  // Write Disable
#define CMD_RDSR      0x05  // Read Status Register 
#define CMD_RDCR      0x15  // Read Configuration Register 
#define CMD_WRSR      0x01  // Write Status Register
#define CMD_PESUS     0xB0  // Program/Erase Suspend 
#define CMD_PERES     0x30  // Program/Erase Resume
#define CMD_DP        0xB9  // Enter Deep Power Down 
#define CMD_SBL       0xC0  // Set Burst Length 
#define CMD_RDID      0x9F  // Read Manufacturer and JDEC Device ID 
#define CMD_REMS      0x90  // Read Electronic Manufacturer and Device ID
#define CMD_RES       0xAB  // Read Electronic ID
#define CMD_ENSO      0xB1  // Enter Secure OTP
#define CMD_EXSO      0xC1  // Exit Secure OTP
#define CMD_RDSCUR    0x2B  // Read Security Register
#define CMD_WRSCUR    0x2F  // Write Security Register
#define CMD_NOP       0x00  // No Operation
#define CMD_RSTEN     0x66  // Reset Enable 
#define CMD_RST       0x99  // Reset 
#define CMD_RRE       0xFF  // Release Read Enhanced Mode
*/
 
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
 int _mode ;
 
/// Write Enable
  void writeEnable(void) ;
  
/// Write Disable
  void writeDisable(void) ;
  
/// Reset Enable
  void resetEnable(void) ;
  
/// Reset 
  void reset(void) ;
 
/// Program or Erase Suspend
  void pgmersSuspend(void) ;
 
/// Program or Erase Resume
  void pgmersResume(void) ;
 
/// Enter Deep Power Down
  void deepPowerdown(void) ;
 
/// Set Burst Length 
  void setBurstlength(void) ;
 
/// Release from Read Enhanced Mode 
  void releaseReadenhaced(void) ;
 
/// No Operation 
  void noOperation(void) ;
 
/// Enter OTP Area 
  void enterSecureOTP(void) ;
 
/// Exit OTP Area 
  void exitSecureOTP(void) ;
 
/// Chip Erase
  void chipErase(void) ;
  
/// Write Status and Configuration Reg 1 and 2
  void writeStatusreg(int addr) ;
  
/// Write Security Reg
  void writeSecurityreg(int addr) ;
  
** Page Program
 *
 * @param int addr start address
 * @param uint8_t *data data buffer
 * @param int numData the number of data to be written
 *
  void programPage(int addr, uint8_t *data, int numData) ;
  
** Sector Erase
 *
 * @param int addr specify the sector to be erased
 *
  void sectorErase(int addr) ;
  
** Block Erase
 *
 * @param int addr specify the sector to be erased
 *
  void blockErase(int addr) ;
  
** 32KB Block Erase
 *
 * @param int addr specify the sector to be erased
 *
  void blockErase32KB(int addr) ;
  
** Read Status Register
 *
 * @returns uint8_t status register value
 *
  uint8_t readStatus(void) ;
  
** Read Security Register
 *
 * @returns uint8_t security register value
 *
  uint8_t readSecurity(void) ;

** Read Manufacturer and JEDEC Device ID
 *
 * @returns uint32_t Manufacturer ID, Mem Type, Device ID
 *
  uint32_t readID(void) ;
  
** Read Electronic Manufacturer and Device ID
 *
 * @returns uint32_t Manufacturer ID, Device ID
 *
  uint32_t readREMS(void) ;
  
** Read Electronic ID
 *
 * @returns uint8_t Device ID
 *
  uint8_t readRES(void) ;
  
** Read Configuration Register
 *
 * @returns uint32_t configuration register value
 *
  uint32_t readConfig(void) ;
  uint8_t readSFDP(int addr) ;        
  uint8_t readFREAD(int addr) ; 
  uint8_t read8(int addr) ;
  void write8(int addr, uint8_t data) ;
  
*/
 

private:

    SPI             _at45spi;
    DigitalOut      _at45cs;
    unsigned int    _at45id;
    
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