/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
//#include "bsp_spi_flash.h"
#include "bsp_sdio_sdcard.h"
#include "string.h"
#include "atk_m750.h"

extern SD_CardInfo SDCardInfo;
extern ST_Time Timedat;

/* Definitions of physical drive number for each drive */
#define SPI_FLASH		0      //0x16	/* Example: Map Ramdisk to physical drive 0 */
#define SDIO_SD		  1	    /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		  2	    /* Example: Map USB MSD to physical drive 2 */




/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
  stat = RES_OK;
	switch (pdrv) {
	case SPI_FLASH :
//		result = RAM_disk_status();
//    stat = result;
		// translate the reslut code here

		return stat;

	case SDIO_SD :
		
		//result = MMC_disk_status();

		// translate the reslut code here
//    stat = RES_OK;
		return stat;

	case DEV_USB :
		result = 3;       //= USB_disk_status();

		// translate the reslut code here
    stat = result;
		return stat;
	}
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	stat = RES_OK;
	switch (pdrv) {
	case SPI_FLASH :
//		result = RAM_disk_initialize(SPI_FLASH);

//		// translate the reslut code here
//    stat = result;
		return stat;

	case SDIO_SD :
		if(SD_Init() != SD_OK)
		  stat = STA_NOINIT;
		else
			stat &=~STA_NOINIT;
		return stat;

	case DEV_USB :
		result = 3;//= USB_disk_initialize();

		// translate the reslut code here
    stat = result;
		return stat;
	}
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
//	SD_Error status;
	int result;
	res = RES_OK;
	switch (pdrv) {
	case SPI_FLASH :
//		//扇区偏移2MB ，外部Flash文件系统空间放在SPI Flash后面 6MB 空间
//		sector += 512;
//		result = RAM_disk_read(buff, sector<<12, count<<12);

//		// translate the reslut code here
//    res = (DRESULT)result;
		return res;

	case SDIO_SD :
		if((DWORD)buff&3)
		{
//				DRESULT res = RES_OK;
        __align(4) DWORD tempbuff[SDCardInfo.CardBlockSize/4];
				while (count--) 
				{
          res = (DRESULT)disk_read(SDIO_SD,(BYTE *)tempbuff,sector++,1);
					if (res != RES_OK) 
					{
						return res;
					}
					memcpy(buff,tempbuff,SDCardInfo.CardBlockSize);	
					buff += SDCardInfo.CardBlockSize;
		    }
		    return res;
		}
		else
		{
			res = (DRESULT)SD_ReadMultiBlocks((uint8_t *)buff, sector * SDCardInfo.CardBlockSize, SDCardInfo.CardBlockSize, count);
			res = (DRESULT)SD_WaitReadOperation();
			while(SD_GetStatus() != SD_TRANSFER_OK);
			return res;
		}

	case DEV_USB :
		// translate the arguments here

		result = RES_NOTRDY;// USB_disk_read(buff, sector, count);

		// translate the reslut code here
    res = (DRESULT)result;
		return res;
	}

	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
	res = RES_OK;
//  uint32_t write_addr;
	if(!count){return RES_PARERR;}
	
	switch (pdrv) {
	case SPI_FLASH :
		// translate the arguments here
//    sector += 512;
//	  write_addr = sector<<12;
//    SPI_Sector_Erase(write_addr);
//		result = RAM_disk_write((u8 *)buff, write_addr, count<<12);

//		// translate the reslut code here
//    res = (DRESULT)result;
		return res;

	case SDIO_SD :
		// translate the arguments here
		if((DWORD)buff&3)
		{
//			DRESULT res = RES_OK;
			__align(4) DWORD tempbuff[SDCardInfo.CardBlockSize/4];
			while (count--) 
			{
				memcpy(tempbuff,buff,SDCardInfo.CardBlockSize);	
				res = (DRESULT)disk_write(SDIO_SD,(BYTE *)tempbuff,sector++,1);
				if (res != RES_OK) 
				{
					return res;
				}
				buff += SDCardInfo.CardBlockSize;
			}
			return res;
		}
		else
		{
			res = (DRESULT)SD_WriteMultiBlocks((uint8_t *)buff, sector * SDCardInfo.CardBlockSize, SDCardInfo.CardBlockSize, count);
			res = (DRESULT)SD_WaitWriteOperation();
			while(SD_GetStatus() != SD_TRANSFER_OK);
			return res;	
		}

	case DEV_USB :
		// translate the arguments here

		result = RES_NOTRDY;//= USB_disk_write(buff, sector, count);

		// translate the reslut code here
    res = (DRESULT)result;
		return res;
	}

	return RES_PARERR;
}


#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	
	switch (pdrv) {
	case SPI_FLASH :
    switch( cmd ){
			case GET_SECTOR_COUNT:
				*(DWORD *)buff = 1536;
				break;
			case GET_SECTOR_SIZE:
				*(WORD *)buff = 4096;
				break;
			case GET_BLOCK_SIZE:
				*(DWORD *)buff = 1;
				break;
		}
		res = RES_OK;
		// Process of the command for the RAM drive
		return res;

	case SDIO_SD :
		switch( cmd ){
			case GET_SECTOR_COUNT:
				*(DWORD *)buff = SDCardInfo.CardCapacity/SDCardInfo.CardBlockSize;
				break;
			case GET_SECTOR_SIZE:
				*(WORD *)buff = SDCardInfo.CardBlockSize;
				break;
			case GET_BLOCK_SIZE:
				*(DWORD *)buff = 1;
				break;
		}
	  res = RES_OK;
		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive
		res = RES_NOTRDY;
		return res;
	}

	return RES_PARERR;
}


__weak DWORD get_fattime(void)
{
		/* 返回当前时间戳 */
	return	  ((DWORD)(Timedat.year - 1980) << 25)	/* Year 2015 */
			| ((DWORD)Timedat.month << 21)				/* Month 1 */
			| ((DWORD)Timedat.day << 16)				/* Mday 1 */
			| ((DWORD)Timedat.hour << 11)				/* Hour 0 */
			| ((DWORD)Timedat.minute << 5)				  /* Min 0 */
			| ((DWORD)Timedat.second >> 1);				/* Sec 0 */
}



