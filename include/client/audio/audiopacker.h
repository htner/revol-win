#pragma once

#include "audio/iaudiocodec.h"
#include "common/xstring.h"
#include "common/packet.h"

namespace audio
{	
	class DefaultPacker: public IAudioPacker
	{
	public:
		template<class STRING_TYPE>
		bool pack_t(const framelist& frames, STRING_TYPE& voice)
		{
			sox::PackBuffer bf;
			sox::Pack pk(bf);
			sox::marshal_container(pk,frames);
			voice =  STRING_TYPE(pk.data(), pk.size());
			return true;
		}
		template<class STRING_TYPE>
		bool unpack_t(const STRING_TYPE& voice, framelist& frames)
		{
			if(voice.empty())
			{
				std::string  strEmpty;
				frames.push_back(strEmpty);
				frames.push_back(strEmpty);
				return true;
			}

			try{
				sox::Unpack up(voice.data(), voice.size());
				sox::unmarshal_container(up, std::back_inserter(frames));
				//return true;
			}catch(sox::UnpackError& ){
				// log unpack error
				std::string  strEmpty;
				frames.push_back(strEmpty);
				frames.push_back(strEmpty);
			}
			return true;

		}
		
		bool unpack(const char* voice, int size,framelist& frames)
		{
			if(voice == NULL)
			{
				std::string  strEmpty;
				frames.push_back(strEmpty);
				frames.push_back(strEmpty);
				return true;
			}

			try{
				sox::Unpack up(voice, size);
				sox::unmarshal_container(up, std::back_inserter(frames));
				//return true;
			}catch(sox::UnpackError& ){
				// log unpack error
				std::string  strEmpty;
				frames.push_back(strEmpty);
				frames.push_back(strEmpty);
			}
			return true;

		}
		inline bool pack(const framelist& frames, std::string& voice)
		{
			return pack_t(frames,voice);
		}
		inline bool unpack(const std::string& voice, framelist& frames)
		{
			return unpack_t(voice,frames);
		}
	};
}