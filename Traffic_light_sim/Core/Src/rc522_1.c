#include "rc522_1.h"

/*
 * Function Name: RC522_SPI_Transfer_1
 * Description: A common function used by Write_MFRC522_1 and Read_MFRC522_1
 * Input Parameters: data - the value to be written
 * Returns: a byte of data read from the module
 */
uint8_t RC522_SPI_Transfer_1(uchar data)
{
	uchar rx_data;
	HAL_SPI_TransmitReceive(HSPI_INSTANCE_1, &data, &rx_data, 1, 100);
	return rx_data;
}

/*
 * Function Name: Write_MFRC522_1
 * Description: Write a byte of data to a specific MFRC522 register
 */
void Write_MFRC522_1(uchar addr, uchar val)
{
	HAL_GPIO_WritePin(MFRC522_CS_PORT_1, MFRC522_CS_PIN_1, GPIO_PIN_RESET);

	RC522_SPI_Transfer_1((addr << 1) & 0x7E);
	RC522_SPI_Transfer_1(val);

	HAL_GPIO_WritePin(MFRC522_CS_PORT_1, MFRC522_CS_PIN_1, GPIO_PIN_SET);
}

/*
 * Function Name: Read_MFRC522_1
 * Description: Read a byte of data from a specific MFRC522 register
 */
uchar Read_MFRC522_1(uchar addr)
{
	uchar val;

	HAL_GPIO_WritePin(MFRC522_CS_PORT_1, MFRC522_CS_PIN_1, GPIO_PIN_RESET);

	RC522_SPI_Transfer_1(((addr << 1) & 0x7E) | 0x80);
	val = RC522_SPI_Transfer_1(0x00);

	HAL_GPIO_WritePin(MFRC522_CS_PORT_1, MFRC522_CS_PIN_1, GPIO_PIN_SET);

	return val;
}

/*
 * Function Name: SetBitMask_1
 * Description: Set bits in RC522 register
 */
void SetBitMask_1(uchar reg, uchar mask)
{
	uchar tmp = Read_MFRC522_1(reg);
	Write_MFRC522_1(reg, tmp | mask);
}

/*
 * Function Name: ClearBitMask_1
 * Description: Clear bits in RC522 register
 */
void ClearBitMask_1(uchar reg, uchar mask)
{
	uchar tmp = Read_MFRC522_1(reg);
	Write_MFRC522_1(reg, tmp & (~mask));
}

/*
 * Function Name: AntennaOn_1
 * Description: Turn on the RF field (antenna)
 */
void AntennaOn_1(void)
{
	Read_MFRC522_1(TxControlReg_1);
	SetBitMask_1(TxControlReg_1, 0x03);
}

/*
 * Function Name: AntennaOff_1
 * Description: Turn off the RF field (antenna)
 */
void AntennaOff_1(void)
{
	ClearBitMask_1(TxControlReg_1, 0x03);
}

/*
 * Function Name: MFRC522_Reset_1
 * Description: Reset the MFRC522 module
 */
void MFRC522_Reset_1(void)
{
	Write_MFRC522_1(CommandReg_1, PCD_RESETPHASE_1);
}

/*
 * Function Name: MFRC522_Init_1
 * Description: Initialize the MFRC522 RFID module
 */
void MFRC522_Init_1(void)
{
	HAL_GPIO_WritePin(MFRC522_CS_PORT_1, MFRC522_CS_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MFRC522_RST_PORT_1, MFRC522_RST_PIN_1, GPIO_PIN_SET);
	MFRC522_Reset_1();

	Write_MFRC522_1(TModeReg_1, 0x8D);
	Write_MFRC522_1(TPrescalerReg_1, 0x3E);
	Write_MFRC522_1(TReloadRegL_1, 30);
	Write_MFRC522_1(TReloadRegH_1, 0);

	Write_MFRC522_1(TxAutoReg_1, 0x40);
	Write_MFRC522_1(ModeReg_1, 0x3D);

	AntennaOn_1();
}

/*
 * Function Name: MFRC522_ToCard_1
 * Description: Communicate with a card via MFRC522
 */
uchar MFRC522_ToCard_1(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
	uchar status = MI_ERR_1;
	uchar irqEn = 0x00;
	uchar waitIRq = 0x00;
	uchar lastBits;
	uchar n;
	uint i;

	switch (command)
	{
	case PCD_AUTHENT_1:
		irqEn = 0x12;
		waitIRq = 0x10;
		break;

	case PCD_TRANSCEIVE_1:
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}

	Write_MFRC522_1(CommIEnReg_1, irqEn | 0x80);
	ClearBitMask_1(CommIrqReg_1, 0x80);
	SetBitMask_1(FIFOLevelReg_1, 0x80);
	Write_MFRC522_1(CommandReg_1, PCD_IDLE_1);

	for (i = 0; i < sendLen; i++)
		Write_MFRC522_1(FIFODataReg_1, sendData[i]);

	Write_MFRC522_1(CommandReg_1, command);

	if (command == PCD_TRANSCEIVE_1)
		SetBitMask_1(BitFramingReg_1, 0x80);

	i = 2000;
	do
	{
		n = Read_MFRC522_1(CommIrqReg_1);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	ClearBitMask_1(BitFramingReg_1, 0x80);

	if (i != 0)
	{
		if (!(Read_MFRC522_1(ErrorReg_1) & 0x1B))
		{
			status = MI_OK_1;
			if (n & irqEn & 0x01)
				status = MI_NOTAGERR_1;

			if (command == PCD_TRANSCEIVE_1)
			{
				n = Read_MFRC522_1(FIFOLevelReg_1);
				lastBits = Read_MFRC522_1(ControlReg_1) & 0x07;
				if (lastBits)
					*backLen = (n - 1) * 8 + lastBits;
				else
					*backLen = n * 8;

				if (n == 0)
					n = 1;
				if (n > MAX_LEN)
					n = MAX_LEN;

				for (i = 0; i < n; i++)
					backData[i] = Read_MFRC522_1(FIFODataReg_1);
			}
		}
		else
			status = MI_ERR_1;
	}

	return status;
}

/*
 * Function Name: MFRC522_Request_1
 * Description: Request card type
 */
uchar MFRC522_Request_1(uchar reqMode, uchar *TagType)
{
	uchar status;
	uint backBits;

	Write_MFRC522_1(BitFramingReg_1, 0x07);
	TagType[0] = reqMode;

	status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, TagType, 1, TagType, &backBits);

	if ((status != MI_OK_1) || (backBits != 0x10))
		status = MI_ERR_1;

	return status;
}

/*
 * Function Name: MFRC522_Anticoll_1
 * Description: Anti-collision, get the card serial number
 */
uchar MFRC522_Anticoll_1(uchar *serNum)
{
	uchar status;
	uchar i, serNumCheck = 0;
	uint unLen;

	Write_MFRC522_1(BitFramingReg_1, 0x00);

	serNum[0] = PICC_ANTICOLL_1;
	serNum[1] = 0x20;
	status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, serNum, 2, serNum, &unLen);

	if (status == MI_OK_1)
	{
		for (i = 0; i < 4; i++)
			serNumCheck ^= serNum[i];
		if (serNumCheck != serNum[i])
			status = MI_ERR_1;
	}

	return status;
}

/*
 * Function Name: CalulateCRC_1
 * Description: Calculate CRC using MFRC522 hardware
 */
void CalulateCRC_1(uchar *pIndata, uchar len, uchar *pOutData)
{
	uchar i, n;

	ClearBitMask_1(DivIrqReg_1, 0x04);
	SetBitMask_1(FIFOLevelReg_1, 0x80);

	for (i = 0; i < len; i++)
		Write_MFRC522_1(FIFODataReg_1, *(pIndata + i));

	Write_MFRC522_1(CommandReg_1, PCD_CALCCRC_1);

	i = 0xFF;
	do
	{
		n = Read_MFRC522_1(DivIrqReg_1);
		i--;
	} while ((i != 0) && !(n & 0x04));

	pOutData[0] = Read_MFRC522_1(CRCResultRegL_1);
	pOutData[1] = Read_MFRC522_1(CRCResultRegH_1);
}

/*
 * Function Name: MFRC522_SelectTag_1
 * Description: Select a card and get its memory size
 */
uchar MFRC522_SelectTag_1(uchar *serNum)
{
	uchar i, status, size;
	uint recvBits;
	uchar buffer[9];

	buffer[0] = PICC_SElECTTAG_1;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++)
		buffer[i + 2] = *(serNum + i);

	CalulateCRC_1(buffer, 7, &buffer[7]);
	status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK_1) && (recvBits == 0x18))
		size = buffer[0];
	else
		size = 0;

	return size;
}

/*
 * Function Name: MFRC522_Auth_1
 * Description: Authenticate with a card sector key
 */
uchar MFRC522_Auth_1(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
	uchar status;
	uint recvBits;
	uchar i;
	uchar buff[12];

	buff[0] = authMode;
	buff[1] = BlockAddr;

	for (i = 0; i < 6; i++)
		buff[i + 2] = *(Sectorkey + i);
	for (i = 0; i < 4; i++)
		buff[i + 8] = *(serNum + i);

	status = MFRC522_ToCard_1(PCD_AUTHENT_1, buff, 12, buff, &recvBits);

	if ((status != MI_OK_1) || (!(Read_MFRC522_1(Status2Reg_1) & 0x08)))
		status = MI_ERR_1;

	return status;
}

/*
 * Function Name: MFRC522_Read_1
 * Description: Read data block from card
 */
uchar MFRC522_Read_1(uchar blockAddr, uchar *recvData)
{
	uchar status;
	uint unLen;

	recvData[0] = PICC_READ_1;
	recvData[1] = blockAddr;
	CalulateCRC_1(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, recvData, 4, recvData, &unLen);

	if ((status != MI_OK_1) || (unLen != 0x90))
		status = MI_ERR_1;

	return status;
}

/*
 * Function Name: MFRC522_Write_1
 * Description: Write data block to card
 */
uchar MFRC522_Write_1(uchar blockAddr, uchar *writeData)
{
	uchar status;
	uint recvBits;
	uchar i;
	uchar buff[18];

	buff[0] = PICC_WRITE_1;
	buff[1] = blockAddr;
	CalulateCRC_1(buff, 2, &buff[2]);
	status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, buff, 4, buff, &recvBits);

	if ((status != MI_OK_1) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		status = MI_ERR_1;

	if (status == MI_OK_1)
	{
		for (i = 0; i < 16; i++)
			buff[i] = *(writeData + i);

		CalulateCRC_1(buff, 16, &buff[16]);
		status = MFRC522_ToCard_1(PCD_TRANSCEIVE_1, buff, 18, buff, &recvBits);

		if ((status != MI_OK_1) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
			status = MI_ERR_1;
	}

	return status;
}

/*
 * Function Name: MFRC522_Halt_1
 * Description: Put card into halt state
 */
void MFRC522_Halt_1(void)
{
	uint unLen;
	uchar buff[4];

	buff[0] = PICC_HALT_1;
	buff[1] = 0;
	CalulateCRC_1(buff, 2, &buff[2]);

	MFRC522_ToCard_1(PCD_TRANSCEIVE_1, buff, 4, buff, &unLen);
}
