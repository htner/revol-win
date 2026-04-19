#pragma once

#include <string>
#include <helper/blowfish.h>
#include "md5.h"

namespace helper
{
		class Buffer
		{
		public:
			Buffer(int size) : p (NULL), l(0)
			{
				if(size == 0) return;
				l = size;
				p = new char[l];
				ZeroMemory(p,l);
			}
			~Buffer()
			{
				if(p) {
					delete[] p; p= NULL;
				}
			}
			char* data(){return p;}
			int   size(){return l;}
			const char* c_str(){return p;}
		protected:
			char* p;
			int	  l;
		};


	class encrypt
	{
	public:
		static std::string bf_encode(const std::string& encryptData, const std::string& key)
		{
			try{
				CBlowFish bf((unsigned char*)key.data(), key.size());
				int length = encryptData.size();
				int nModLen = ((length/8) + 1) * 8;
				Buffer buffIn(nModLen), buffOut(nModLen);
				memcpy(buffIn.data(), encryptData.c_str(), encryptData.size());
				bf.Encrypt((const unsigned char*)buffIn.c_str(),
							(unsigned char*)buffOut.data(), nModLen);
				return std::string(buffOut.data(), buffOut.size());
			}catch(std::exception& e){
				return "";
			}				
		}
		static std::string bf_decode(const std::string& decyptData, const std::string& key)
		{
			if(decyptData.size() % 8 != 0)
			{
				return "";
			}
			try{
				CBlowFish bf((unsigned char*)key.data(), key.size());
				Buffer buffOut(decyptData.size());
				bf.Decrypt((const unsigned char*)decyptData.data(),
							(unsigned char*)buffOut.data() ,buffOut.size());
				return std::string(buffOut.data(), buffOut.size());
			}catch(std::exception& e){
				return "";
			}			
		}
		static std::string MakeMD5(const std::string& data){
			return MakeMD5(data.c_str(),data.size());
		}
		static std::string MakeMD5(const char* lpData,int nSize){
			MD5 md5;
			md5.feed((const unsigned char*)lpData,nSize);
			md5.finalize();
			return md5.hex();
		}

	};
}