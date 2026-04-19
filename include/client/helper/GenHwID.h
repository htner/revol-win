#pragma once
#include "helper/md5.h"

#include <iphlpapi.h>   
#include "GenHardDiskID.h"

#ifndef IO_CTL_MICRO
	#define  FILE_DEVICE_SCSI              0x0000001b   
	#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ( ( FILE_DEVICE_SCSI << 16 ) + 0x0501 )   
	   
	#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition   
	   
	#define  IDENTIFY_BUFFER_SIZE  512   
	#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088   
	#define  DFP_GET_VERSION          0x00074080   
	#define  SENDIDLENGTH  sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE
#endif //IO_CTL_MICRO



#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.   
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.   


class CGenHwID
{

protected:
	typedef struct _IDSECTOR   
	{   
		USHORT  wGenConfig;   
		USHORT  wNumCyls;   
		USHORT  wReserved;   
		USHORT  wNumHeads;   
		USHORT  wBytesPerTrack;   
		USHORT  wBytesPerSector;   
		USHORT  wSectorsPerTrack;   
		USHORT  wVendorUnique[3];   
		CHAR    sSerialNumber[20];   
		USHORT  wBufferType;   
		USHORT  wBufferSize;   
		USHORT  wECCSize;   
		CHAR    sFirmwareRev[8];   
		CHAR    sModelNumber[40];   
		USHORT  wMoreVendorUnique;   
		USHORT  wDoubleWordIO;   
		USHORT  wCapabilities;   
		USHORT  wReserved1;   
		USHORT  wPIOTiming;   
		USHORT  wDMATiming;   
		USHORT  wBS;   
		USHORT  wNumCurrentCyls;   
		USHORT  wNumCurrentHeads;   
		USHORT  wNumCurrentSectorsPerTrack;   
		ULONG   ulCurrentSectorCapacity;   
		USHORT  wMultSectorStuff;   
		ULONG   ulTotalAddressableSectors;   
		USHORT  wSingleWordDMA;   
		USHORT  wMultiWordDMA;   
		BYTE    bReserved[128];   
	} IDSECTOR, *PIDSECTOR;   

	typedef struct _DRIVERSTATUS   

	{   
		BYTE  bDriverError;  //  Error code from driver, or 0 if no error.   
		BYTE  bIDEStatus;    //  Contents of IDE Error register.   
		//  Only valid when bDriverError is SMART_IDE_ERROR.   
		BYTE  bReserved[2];  //  Reserved for future expansion.   
		DWORD  dwReserved[2];  //  Reserved for future expansion.   
	} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;   

	typedef struct _SENDCMDOUTPARAMS   
	{   
		DWORD         cBufferSize;   //  Size of bBuffer in bytes   
		DRIVERSTATUS  DriverStatus;  //  Driver status structure.   
		BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.   
	} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;   

	typedef struct _SRB_IO_CONTROL   
	{   
		ULONG HeaderLength;   
		UCHAR Signature[8];   
		ULONG Timeout;   
		ULONG ControlCode;   
		ULONG ReturnCode;   
		ULONG Length;   
	} SRB_IO_CONTROL, *PSRB_IO_CONTROL;   

	typedef struct _IDEREGS   
	{   
		BYTE bFeaturesReg;       // Used for specifying SMART "commands".   
		BYTE bSectorCountReg;    // IDE sector count register   
		BYTE bSectorNumberReg;   // IDE sector number register   
		BYTE bCylLowReg;         // IDE low order cylinder value   
		BYTE bCylHighReg;        // IDE high order cylinder value   
		BYTE bDriveHeadReg;      // IDE drive/head register   
		BYTE bCommandReg;        // Actual IDE command.   
		BYTE bReserved;          // reserved for future use.  Must be zero.   
	} IDEREGS, *PIDEREGS, *LPIDEREGS;   

	typedef struct _SENDCMDINPARAMS   
	{   
		DWORD     cBufferSize;   //  Buffer size in bytes   
		IDEREGS   irDriveRegs;   //  Structure with drive register values.   
		BYTE bDriveNumber;       //  Physical drive number to send    
		//  command to (0,1,2,3).   
		BYTE bReserved[3];       //  Reserved for future expansion.   
		DWORD     dwReserved[4]; //  For future use.   
		BYTE      bBuffer[1];    //  Input buffer.   
	} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;   

	typedef struct _GETVERSIONOUTPARAMS   
	{   
		BYTE bVersion;      // Binary driver version.   
		BYTE bRevision;     // Binary driver revision.   
		BYTE bReserved;     // Not used.   
		BYTE bIDEDeviceMap; // Bit map of IDE devices.   
		DWORD fCapabilities; // Bit mask of driver capabilities.   
		DWORD dwReserved[4]; // For future use.   
	} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;   

	//////////////////////////////////////////////////////////////////////   

	//结构定义    
	typedef struct _UNICODE_STRING    
	{    
		USHORT  Length;//长度    
		USHORT  MaximumLength;//最大长度    
		PWSTR  Buffer;//缓存指针    
	} UNICODE_STRING,*PUNICODE_STRING;    

	typedef struct _OBJECT_ATTRIBUTES    
	{    
		ULONG Length;//长度 18h    
		HANDLE RootDirectory;//  00000000    
		PUNICODE_STRING ObjectName;//指向对象名的指针    
		ULONG Attributes;//对象属性00000040h    
		PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR，0    
		PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE，0    
	} OBJECT_ATTRIBUTES;    
	typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;    

	//函数指针变量类型   
	typedef DWORD  (__stdcall *ZWOS )( PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);    
	typedef DWORD  (__stdcall *ZWMV )( HANDLE,HANDLE,PVOID,ULONG,ULONG,PLARGE_INTEGER,PSIZE_T,DWORD,ULONG,ULONG);    
	typedef DWORD  (__stdcall *ZWUMV )( HANDLE,PVOID);    

	BOOL WinNTHDSerialNumAsScsiRead( BYTE* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen )   
	{   
		BOOL bInfoLoaded = FALSE;   

		for( int iController = 0; iController  < 4; ++ iController )   
		{   
			HANDLE hScsiDriveIOCTL = 0;   
			char   szDriveName[256];   

			//  Try to get a handle to PhysicalDrive IOCTL, report failure   
			//  and exit if can't.   
			sprintf( szDriveName, "\\\\.\\Scsi%d:", iController );   

			//  Windows NT, Windows 2000, any rights should do   
			hScsiDriveIOCTL = CreateFileA( szDriveName,   
				GENERIC_READ | GENERIC_WRITE,    
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,   
				OPEN_EXISTING, 0, NULL);   

			// if (hScsiDriveIOCTL == INVALID_HANDLE_VALUE)   
			//    printf ("Unable to open SCSI controller %d, error code: 0x%lX\n",   
			//            controller, GetLastError ());   

			if( hScsiDriveIOCTL != INVALID_HANDLE_VALUE )   
			{   
				int iDrive = 0;   
				for( iDrive = 0; iDrive < 16; ++ iDrive )   
				{   
					char szBuffer[sizeof( SRB_IO_CONTROL ) + SENDIDLENGTH] = { 0 };   

					SRB_IO_CONTROL* p = ( SRB_IO_CONTROL* )szBuffer;   
					SENDCMDINPARAMS* pin = ( SENDCMDINPARAMS* )( szBuffer + sizeof( SRB_IO_CONTROL ) );   
					DWORD dwResult;   

					p->HeaderLength = sizeof( SRB_IO_CONTROL );   
					p->Timeout = 10000;   
					p->Length = SENDIDLENGTH;   
					p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;   
					strncpy( ( char* )p->Signature, "SCSIDISK", 8 );   

					pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;   
					pin->bDriveNumber = iDrive;   

					if( DeviceIoControl( hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT,    
						szBuffer,   
						sizeof( SRB_IO_CONTROL ) + sizeof( SENDCMDINPARAMS ) - 1,   
						szBuffer,   
						sizeof( SRB_IO_CONTROL ) + SENDIDLENGTH,   
						&dwResult, NULL ) )   
					{   
						SENDCMDOUTPARAMS* pOut = ( SENDCMDOUTPARAMS* )( szBuffer + sizeof( SRB_IO_CONTROL ) );   
						IDSECTOR* pId = ( IDSECTOR* )( pOut->bBuffer );   
						if( pId->sModelNumber[0] )   
						{   
							if( * puSerialLen + 20U <= uMaxSerialLen )   
							{   
								// 序列号   
								CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )pId ) + 10, 20 );   

								// Cut off the trailing blanks   
								UINT i = 20;
								for( ; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )   
								{}   
								* puSerialLen += i;   

								// 型号   
								CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )pId ) + 27, 40 );   
								// Cut off the trailing blanks   
								for( i = 40; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )   
								{}   
								* puSerialLen += i;   

								bInfoLoaded = TRUE;   
							}   
							else   
							{   
								::CloseHandle( hScsiDriveIOCTL );   
								return bInfoLoaded;   
							}   
						}   
					}   
				}   
				::CloseHandle( hScsiDriveIOCTL );   
			}   
		}   
		return bInfoLoaded;   
	}   

	BOOL DoIdentify( HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,   
		PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,   
		PDWORD lpcbBytesReturned )   
	{   
		// Set up data structures for IDENTIFY command.   
		pSCIP->cBufferSize                  = IDENTIFY_BUFFER_SIZE;   
		pSCIP->irDriveRegs.bFeaturesReg     = 0;   
		pSCIP->irDriveRegs.bSectorCountReg  = 1;   
		pSCIP->irDriveRegs.bSectorNumberReg = 1;   
		pSCIP->irDriveRegs.bCylLowReg       = 0;   
		pSCIP->irDriveRegs.bCylHighReg      = 0;   

		// calc the drive number.   
		pSCIP->irDriveRegs.bDriveHeadReg = (0xA0 | ( ( bDriveNum & 1 ) << 4 ));   

		// The command can either be IDE identify or ATAPI identify.   
		pSCIP->irDriveRegs.bCommandReg = bIDCmd;   
		pSCIP->bDriveNumber = bDriveNum;   
		pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;   

		return DeviceIoControl( hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,   
			( LPVOID ) pSCIP,   
			sizeof( SENDCMDINPARAMS ) - 1,   
			( LPVOID ) pSCOP,   
			sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE - 1,   
			lpcbBytesReturned, NULL );   
	}   

	BOOL WinNTHDSerialNumAsPhysicalRead( BYTE* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen )   
	{   

		BOOL bInfoLoaded = FALSE;   

		for( UINT uDrive = 0; uDrive < 4; ++ uDrive )   
		{   
			HANDLE hPhysicalDriveIOCTL = 0;   

			//  Try to get a handle to PhysicalDrive IOCTL, report failure   
			//  and exit if can't.   
			char szDriveName [256];   
			sprintf( szDriveName, "\\\\.\\PhysicalDrive%d", uDrive );   

			//  Windows NT, Windows 2000, must have admin rights   
			hPhysicalDriveIOCTL = CreateFileA( szDriveName,   
				GENERIC_READ | GENERIC_WRITE,    
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,   
				OPEN_EXISTING, 0, NULL);   

			if( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE )   
			{   
				GETVERSIONOUTPARAMS VersionParams = { 0 };   
				DWORD               cbBytesReturned = 0;   

				// Get the version, etc of PhysicalDrive IOCTL   
				if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_GET_VERSION,   
					NULL,    
					0,   
					&VersionParams,   
					sizeof( GETVERSIONOUTPARAMS ),   
					&cbBytesReturned, NULL ) )   
				{   
					// If there is a IDE device at number "i" issue commands   
					// to the device   
					if( VersionParams.bIDEDeviceMap != 0 )   
					{   
						BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd   
						SENDCMDINPARAMS  scip = { 0 };   

						// Now, get the ID sector for all IDE devices in the system.   
						// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,   
						// otherwise use the IDE_ATA_IDENTIFY command   
						bIDCmd = ( VersionParams.bIDEDeviceMap >> uDrive & 0x10 ) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;   
						BYTE IdOutCmd[sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE - 1] = { 0 };   

						if( DoIdentify( hPhysicalDriveIOCTL,    
							&scip,    
							( PSENDCMDOUTPARAMS )&IdOutCmd,    
							( BYTE )bIDCmd,   
							( BYTE )uDrive,   
							&cbBytesReturned ) )   
						{   
							if( * puSerialLen + 20U <= uMaxSerialLen )   
							{   
								CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )( ( ( PSENDCMDOUTPARAMS )IdOutCmd )->bBuffer ) ) + 10, 20 );  // 序列号   

								// Cut off the trailing blanks   
								UINT i = 20;
								for( ; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )  {}   
								* puSerialLen += i;   

								CopyMemory( dwSerial + * puSerialLen, ( ( USHORT* )( ( ( PSENDCMDOUTPARAMS )IdOutCmd )->bBuffer ) ) + 27, 40 ); // 型号   

								// Cut off the trailing blanks   
								for( i = 40; i != 0U && ' ' == dwSerial[* puSerialLen + i - 1]; -- i )  {}   
								* puSerialLen += i;   

								bInfoLoaded = TRUE;   
							}   
							else   
							{   
								::CloseHandle( hPhysicalDriveIOCTL );   
								return bInfoLoaded;   
							}   
						}   
					}   
				}   
				CloseHandle( hPhysicalDriveIOCTL );   
			}   
		}   
		return bInfoLoaded;   
	}   
public:	

	// 硬盘序列号，注意：有的硬盘没有序列号   
	BOOL GetHdSn(char* szSystemInfo,UINT& uSystemInfoLen)   
	{   
		OSVERSIONINFO ovi = { 0 };   
		ovi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );   
		GetVersionEx( &ovi );   

		if( ovi.dwPlatformId != VER_PLATFORM_WIN32_NT )   
		{   
			// Only Windows 2000, Windows XP, Windows Server 2003...   
			return FALSE;   
		}   
		else   
		{   
			if( !WinNTHDSerialNumAsPhysicalRead( (BYTE*)szSystemInfo, &uSystemInfoLen, 1024 ) )   
			{   
				return WinNTHDSerialNumAsScsiRead( (BYTE*)szSystemInfo, &uSystemInfoLen, 1024 );   
			}   
			else   
				return TRUE;   
		}   
	}   

	// CPU ID   
	BOOL GetCpuId(char* szSystemInfo,UINT& uSystemInfoLen)   
	{   
		BOOL bException = FALSE;   
		BYTE szCpu[16]  = { 0 };   
		UINT uCpuID     = 0U;   

		try    
		{   
			_asm    
			{   
				mov eax, 0   
					cpuid   
					mov dword ptr szCpu[0], ebx   
					mov dword ptr szCpu[4], edx   
					mov dword ptr szCpu[8], ecx   
					mov eax, 1   
					cpuid   
					mov uCpuID, edx   
			}   
		}   
		catch(...)
		{   
			bException = TRUE;   
		}   

		if( !bException )   
		{   

			uCpuID = strlen( ( char* )szCpu );   
			CopyMemory( szSystemInfo , szCpu, uCpuID );   
			uSystemInfoLen += uCpuID;   

			UINT nTemp = uCpuID;
			memset( szSystemInfo + uSystemInfoLen,'0',sizeof(uCpuID) * 2 );
			for (int i=sizeof(uCpuID) * 2; i >0; i--)
			{
				UINT n = (nTemp & 0xF);
				nTemp = nTemp >> 4;
				if( n >= 10 && n <= 15)
					*(szSystemInfo + uSystemInfoLen + i) = 'a' + n - 10;				
				else
					*(szSystemInfo + uSystemInfoLen + i) = '0' + n;				
			}
			uSystemInfoLen += sizeof(uCpuID) * 2;   

			return TRUE;   
		}   
		else   
			return FALSE;   
	}   

	static std::string GenID(bool bEncrypt = true)
	{
		std::string enCode;
		static std::string strGenId;
		if( strGenId.empty() )
		{
			try
			{
				DiskInfo di;
				std::string strHdid;
				for (int i = di.LoadDiskInfo()-1; i >= 0; i--)
				{
					strHdid += di.SerialNumber(i);
				}
				if( strHdid.empty() )
					return "";

				CGenHwID gen;			
				UINT uCpuId = 0;			
				char szCpuId[1024] = {0};
				gen.GetCpuId(szCpuId,uCpuId);

				strGenId = szCpuId;
				strGenId += strHdid;
			}catch(...){
				return "";
			}
		}
		if( bEncrypt )
			enCode = helper::encrypt::MakeMD5(strGenId);
		else
			enCode = strGenId;
		return enCode;
	}
};
