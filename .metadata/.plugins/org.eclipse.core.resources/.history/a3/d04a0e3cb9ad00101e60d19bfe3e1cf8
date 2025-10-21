#include "stm32f4xx_hal.h"

#define	uchar	unsigned char
#define	uint	unsigned int
extern SPI_HandleTypeDef hspi1;

//Maximum length of the array
#define MAX_LEN 16

#define HSPI_INSTANCE_1				&hspi1
#define MFRC522_CS_PORT_1			GPIOA
#define MFRC522_CS_PIN_1				GPIO_PIN_4
#define MFRC522_RST_PORT_1			GPIOB
#define MFRC522_RST_PIN_1				GPIO_PIN_0

// MFRC522 commands. Described in chapter 10 of the datasheet.
#define PCD_IDLE_1              0x00               // no action, cancels current command execution
#define PCD_AUTHENT_1           0x0E               // performs the MIFARE standard authentication as a reader
#define PCD_RECEIVE_1           0x08               // activates the receiver circuits
#define PCD_TRANSMIT_1          0x04               // transmits data from the FIFO buffer
#define PCD_TRANSCEIVE_1        0x0C               // transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
#define PCD_RESETPHASE_1        0x0F               // resets the MFRC522
#define PCD_CALCCRC_1           0x03               // activates the CRC coprocessor or performs a self-test

// Commands sent to the PICC.
#define PICC_REQIDL_1           0x26               // REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define PICC_REQALL_1           0x52               // Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define PICC_ANTICOLL_1         0x93               // Anti collision/Select, Cascade Level 1
#define PICC_SElECTTAG_1        0x93               // Anti collision/Select, Cascade Level 2
#define PICC_AUTHENT1A_1        0x60               // Perform authentication with Key A
#define PICC_AUTHENT1B_1        0x61               // Perform authentication with Key B
#define PICC_READ_1             0x30               // Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define PICC_WRITE_1            0xA0               // Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define PICC_DECREMENT_1        0xC0               // Decrements the contents of a block and stores the result in the internal data register.
#define PICC_INCREMENT_1        0xC1               // Increments the contents of a block and stores the result in the internal data register
#define PICC_RESTORE_1          0xC2               // Reads the contents of a block into the internal data register.
#define PICC_TRANSFER_1         0xB0               // Writes the contents of the internal data register to a block.
#define PICC_HALT_1             0x50               // HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.


// Success or error code is returned when communication
#define MI_OK_1                 0
#define MI_NOTAGERR_1           1
#define MI_ERR_1                2


// MFRC522 registers. Described in chapter 9 of the datasheet.
// Page 0: Command and Status
#define     Reserved00_1            0x00
#define     CommandReg_1            0x01
#define     CommIEnReg_1            0x02
#define     DivlEnReg_1             0x03
#define     CommIrqReg_1            0x04
#define     DivIrqReg_1             0x05
#define     ErrorReg_1              0x06
#define     Status1Reg_1            0x07
#define     Status2Reg_1            0x08
#define     FIFODataReg_1           0x09
#define     FIFOLevelReg_1          0x0A
#define     WaterLevelReg_1         0x0B
#define     ControlReg_1            0x0C
#define     BitFramingReg_1         0x0D
#define     CollReg_1               0x0E
#define     Reserved01_1            0x0F
//Page 1: Command
#define     Reserved10_1            0x10
#define     ModeReg_1               0x11
#define     TxModeReg_1             0x12
#define     RxModeReg_1             0x13
#define     TxControlReg_1          0x14
#define     TxAutoReg_1             0x15
#define     TxSelReg_1              0x16
#define     RxSelReg_1              0x17
#define     RxThresholdReg_1        0x18
#define     DemodReg_1              0x19
#define     Reserved11_1            0x1A
#define     Reserved12_1            0x1B
#define     MifareReg_1             0x1C
#define     Reserved13_1            0x1D
#define     Reserved14_1            0x1E
#define     SerialSpeedReg_1        0x1F
//Page 2: Configuration
#define     Reserved20_1            0x20
#define     CRCResultRegH_1         0x21
#define     CRCResultRegL_1         0x22
#define     Reserved21_1            0x23
#define     ModWidthReg_1           0x24
#define     Reserved22_1            0x25
#define     RFCfgReg_1              0x26
#define     GsNReg_1                0x27
#define     CWGsPReg_1	          0x28
#define     ModGsPReg_1             0x29
#define     TModeReg_1              0x2A
#define     TPrescalerReg_1         0x2B
#define     TReloadRegH_1           0x2C
#define     TReloadRegL_1           0x2D
#define     TCounterValueRegH_1     0x2E
#define     TCounterValueRegL_1     0x2F
//Page 3: Test Registers
#define     Reserved30_1            0x30
#define     TestSel1Reg_1           0x31
#define     TestSel2Reg_1           0x32
#define     TestPinEnReg_1          0x33
#define     TestPinValueReg_1       0x34
#define     TestBusReg_1            0x35
#define     AutoTestReg_1           0x36
#define     VersionReg_1            0x37
#define     AnalogTestReg_1         0x38
#define     TestDAC1Reg_1           0x39
#define     TestDAC2Reg_1           0x3A
#define     TestADCReg_1            0x3B
#define     Reserved31_1            0x3C
#define     Reserved32_1            0x3D
#define     Reserved33_1            0x3E
#define     Reserved34_1			  0x3F

// Functions for manipulating the MFRC522
void MFRC522_Init_1(void);
uchar MFRC522_Request_1(uchar reqMode, uchar *TagType);
uchar MFRC522_Anticoll_1(uchar *serNum);
uchar MFRC522_SelectTag_1(uchar *serNum);
uchar MFRC522_Auth_1(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum);
uchar MFRC522_Write_1(uchar blockAddr, uchar *writeData);
uchar MFRC522_Read_1(uchar blockAddr, uchar *recvData);
void MFRC522_Halt_1(void);
