#pragma	  once
#include <wincrypt.h>
#include <atlstr.h>


#define  ASSERT ATLASSERT

#pragma comment(lib, "Crypt32.lib")

BOOL HMACHash(HCRYPTPROV hProvider, HCRYPTKEY hKey, ALG_ID HashAlgId, BYTE * pBytesOut, DWORD dwLengthOut, BYTE * pBytesIn1, DWORD dwLengthIn1, BYTE * pBytesIn2, DWORD dwLengthIn2);
BOOL DeriveLoginKey(BYTE * key, DWORD dwKeySize, CAtlStringA magic, BYTE * pOutBytes, DWORD dwOutLen);
CAtlStringA GenerateLoginBlob(CAtlStringA key, CAtlStringA challenge);

const BYTE cKeyStdHeader[] = {0x08,0x02,0x00,0x00,0x03,0x66,0x00,0x00,0x18,0x00,0x00,0x00};
#define STDKEYHDRSIZE 12
typedef struct tagMSGRUSRKEY
{
	ULONG uStructHeaderSize;
	ULONG uCryptMode;
	ULONG uCipherType;
	ULONG uHashType;
	ULONG uIVLen;
	ULONG uHashLen;
	ULONG uCipherLen;
	BYTE aIVBytes[8];
	BYTE aHashBytes[20];
	BYTE aCipherBytes[72];

	tagMSGRUSRKEY()
	{
		uStructHeaderSize = 28;
		uCryptMode = CRYPT_MODE_CBC;
		uCipherType = CALG_3DES;
		uHashType = CALG_SHA1;
		uIVLen = sizeof (aIVBytes);
		uHashLen = sizeof(aHashBytes);
		uCipherLen = sizeof(aCipherBytes);
	}
}MSGUSRKEY;

BOOL HMACHash(HCRYPTPROV hProvider, HCRYPTKEY hKey, ALG_ID HashAlgId, BYTE * pBytesOut, DWORD dwLengthOut, BYTE * pBytesIn1, DWORD dwLengthIn1, BYTE * pBytesIn2, DWORD dwLengthIn2)
{
	HCRYPTHASH hHash;
	BOOL bResult = FALSE;

	if ( CryptCreateHash(hProvider,CALG_HMAC,hKey,0,&hHash) )
	{
		HMAC_INFO hmcinfo;
		ZeroMemory(&hmcinfo, sizeof(HMAC_INFO));
		hmcinfo.HashAlgid = HashAlgId;
		CryptSetHashParam(hHash,HP_HMAC_INFO,(BYTE*)&hmcinfo,0);

		if (CryptHashData(hHash,pBytesIn1,dwLengthIn1,0))
		{
			if (dwLengthIn2 != 0 && pBytesIn2 != 0)
			{
				CryptHashData(hHash,pBytesIn2,dwLengthIn2,0);
			}
			DWORD dwHashSize;
			DWORD dwParamSize = 4;
			if ( CryptGetHashParam(hHash,HP_HASHSIZE,(BYTE*)&dwHashSize,&dwParamSize,0) )
			{
				if ( dwHashSize <=dwLengthOut)
				{
					dwParamSize = dwLengthOut;
					if ( CryptGetHashParam(hHash,HP_HASHVAL,pBytesOut,&dwParamSize,0) )
					{
						bResult = TRUE;
					}
				}
			}
		}
		CryptDestroyHash(hHash);
	}
	return bResult;
}

BOOL DeriveLoginKey(BYTE * key, DWORD dwKeySize, CAtlStringA magic, BYTE * pOutBytes, DWORD dwOutLen)
{
	HCRYPTPROV hProvider;
	BOOL bRet = FALSE;
	if ( CryptAcquireContext(&hProvider,0,0,PROV_RSA_FULL,0) )
	{
		HCRYPTKEY hCryptKey;

		BYTE * pImportKey = new BYTE[ dwKeySize+STDKEYHDRSIZE ];
		memcpy(pImportKey,cKeyStdHeader,STDKEYHDRSIZE);
		memcpy(pImportKey+STDKEYHDRSIZE,key,dwKeySize);
		((DWORD*)pImportKey)[2]=dwKeySize;
		if (CryptImportKey(hProvider,pImportKey,dwKeySize+STDKEYHDRSIZE,0,CRYPT_SF,&hCryptKey))
		{
			BYTE bHash1[20];
			BYTE bHash2[20];
			BYTE bHash3[20];
			BYTE bHash4[20];
			HMACHash(hProvider,hCryptKey,CALG_SHA1,bHash1,20,(BYTE*)(LPCSTR)magic,magic.GetLength(),0,0);
			HMACHash(hProvider,hCryptKey,CALG_SHA1,bHash2,20,bHash1,20,(BYTE*)(LPCSTR)magic,magic.GetLength());
			HMACHash(hProvider,hCryptKey,CALG_SHA1,bHash3,20,bHash1,20,0,0);
			HMACHash(hProvider,hCryptKey,CALG_SHA1,bHash4,20,bHash3,20,(BYTE*)(LPCSTR)magic,magic.GetLength());

			if ( dwOutLen>=24)
			{
				memcpy(pOutBytes,bHash2,20);
				memcpy(pOutBytes+20,bHash4,4);
				bRet = TRUE;
			}
			CryptDestroyKey(hCryptKey);
		}
		delete[] pImportKey;

		CryptReleaseContext(hProvider,0);
	}

	return bRet;
}


CAtlStringA GenerateLoginBlob(CAtlStringA key, CAtlStringA challenge)
{
	BYTE key1[24] = {0};
	BYTE key2[24] = {0}; 
	BYTE key3[24] = {0};
	BYTE hash[20] = {0};
	BYTE randomdata[8] = {0};
	CAtlStringA szRet ="";
	DWORD dwBase64Size;

	CryptStringToBinaryA(key,0,CRYPT_STRING_BASE64,0,&dwBase64Size,0,0);
	ASSERT(dwBase64Size<=24);
	if (dwBase64Size>24)
		return "";

	dwBase64Size = 24;
	CryptStringToBinaryA(key,0,CRYPT_STRING_BASE64,key1,&dwBase64Size,0,0);
	DeriveLoginKey(key1,24,"WS-SecureConversationSESSION KEY HASH",key2,24);
	DeriveLoginKey(key1,24,"WS-SecureConversationSESSION KEY ENCRYPTION",key3,24);

	HCRYPTPROV hProvider;
	if ( CryptAcquireContext(&hProvider,0,0,PROV_RSA_FULL,0) )
	{
		HCRYPTKEY hCryptKey;
		HCRYPTKEY hCryptKey2;

		BYTE * pImportKey = new BYTE[ 24+STDKEYHDRSIZE ];
		memcpy(pImportKey,cKeyStdHeader,STDKEYHDRSIZE);
		memcpy(pImportKey+STDKEYHDRSIZE,key2,24);

		CryptImportKey(hProvider,pImportKey,24+STDKEYHDRSIZE,0,CRYPT_SF,&hCryptKey2);

		memcpy(pImportKey+STDKEYHDRSIZE,key3,24);
		if ( CryptImportKey(hProvider,pImportKey,24+STDKEYHDRSIZE,0,CRYPT_SF,&hCryptKey) )
		{
			HCRYPTKEY hKeyDupe1;
			HCRYPTKEY hKeyDupe2;
			HCRYPTHASH hHash;

			CryptDuplicateKey(hCryptKey,0,0,&hKeyDupe1);
			DWORD dwMode = CRYPT_MODE_CBC;
			CryptSetKeyParam(hKeyDupe1,KP_MODE,(BYTE*)&dwMode,0);
			if (CryptCreateHash(hProvider,CALG_HMAC,hCryptKey2,0,&hHash))
			{
				HMAC_INFO hmcinfo;
				ZeroMemory(&hmcinfo, sizeof(HMAC_INFO));
				hmcinfo.HashAlgid = CALG_SHA1;
				CryptSetHashParam(hHash,HP_HMAC_INFO,(BYTE*)&hmcinfo,0);

				DWORD dwDataLen = challenge.GetLength();
				CryptDuplicateKey(hKeyDupe1,0,0,&hKeyDupe2);
				CryptEncrypt(hKeyDupe2,0,TRUE,0,0,&dwDataLen,0);
				CryptDestroyKey(hKeyDupe2);

				if ( dwDataLen > 0)
				{
					CryptGenRandom(hProvider,8,randomdata);
					CryptSetKeyParam(hKeyDupe1,KP_IV,randomdata,0);

					BYTE * pEncryptBytes = new BYTE[dwDataLen];
					ZeroMemory(pEncryptBytes,dwDataLen);
					memcpy(pEncryptBytes,(LPCSTR)challenge,challenge.GetLength());

					DWORD dwData = challenge.GetLength();
					if (CryptEncrypt(hKeyDupe1,hHash,TRUE,0,pEncryptBytes,&dwData,dwDataLen))
					{
						ASSERT(dwData == 72); // The size of the encryption *should* always be 72. If it's not you'll need to fix it.
						dwData = 20;
						CryptGetHashParam(hHash,HP_HASHVAL,hash,&dwData,0);

						MSGUSRKEY usrkey;
						memcpy(usrkey.aIVBytes, randomdata,8);
						memcpy(usrkey.aHashBytes, hash,20);
						memcpy(usrkey.aCipherBytes , pEncryptBytes,72);

						CryptBinaryToStringA((BYTE*)&usrkey,sizeof(MSGUSRKEY),CRYPT_STRING_BASE64, 0,&dwBase64Size);
						CryptBinaryToStringA((BYTE*)&usrkey,sizeof(MSGUSRKEY),CRYPT_STRING_BASE64, szRet.GetBuffer(dwBase64Size),&dwBase64Size);
						szRet.ReleaseBuffer();

						szRet.Replace("\r\n","");
					}
					delete[] pEncryptBytes;
				}
				CryptDestroyHash(hHash);
			}
			CryptDestroyKey(hKeyDupe1);

			CryptDestroyKey(hCryptKey);
		}
		delete[] pImportKey;
		if ( hCryptKey2 )
			CryptDestroyKey(hCryptKey2);
		CryptReleaseContext(hProvider,0);
	}
	return szRet;
}