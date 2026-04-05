#include "rsakey.h"
#include <protocol/const.h>
#include "assert.h"

static const char rnd_seed[] = "Our product aim to make game player happy !!!";


CRSAKey::CRSAKey(void)
{
	RAND_seed(rnd_seed, sizeof rnd_seed); 

	__rsaKey =  RSA_generate_key(512, 3, NULL, NULL);

	while(RSA_check_key(__rsaKey) != 1){
		RSA_free(__rsaKey);
		__rsaKey = RSA_generate_key(512, 3, NULL, NULL);
	}

}

CRSAKey::~CRSAKey(void)
{
	if(__rsaKey){
		RSA_free(__rsaKey);
		CRYPTO_cleanup_all_ex_data();
	}
}

void CRSAKey::getRSAKeyString(std::string &pub, std::string &e){
	unsigned char keybuf[1024];

	int size = BN_bn2bin(__rsaKey->n, keybuf);
	pub = std::string((char *)keybuf, size);

	size = BN_bn2bin(__rsaKey->e, keybuf);
	e = std::string((char *)keybuf, size);

}

void CRSAKey::rsaDecodeRc4(const std::string &ctext, std::string &ptext)
{
	unsigned char rc4key[100];

	int num = RSA_private_decrypt(ctext.length(), (const unsigned char *)ctext.data(), rc4key, __rsaKey,
		RSA_PKCS1_PADDING);

	if (num != DEF_SESSIONKEY_LENGTH)
	{
		assert(false);
		ptext = "";
		return;
	}
	rc4key[16] = 0;

	ptext = std::string((const char *)rc4key, DEF_SESSIONKEY_LENGTH);
}