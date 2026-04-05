#pragma once
#include "third/openssl/crypto.h"
#include "third/openssl/err.h"
#include "third/openssl/rand.h"
#include "third/openssl/bn.h"
#include "third/openssl/rsa.h"
#include <string>

class CRSAKey
{
public:
	CRSAKey(void);
	~CRSAKey(void);
	void getRSAKeyString(std::string &pub, std::string &e);
	void rsaDecodeRc4(const std::string &ctext, std::string &ptext);
protected:
	RSA	*		__rsaKey;
};
