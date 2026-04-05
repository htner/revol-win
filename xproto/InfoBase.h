#pragma once

#include "proto/iproto.h"
#include <common/nsox/proto_cast.h>
#include <common/nsox/nuautoptr.h>
#include <protocol/puinfo.h>
#include <common/iproperty.h>

namespace xproto
{
	class InfoBase
	{
	public:
		InfoBase(void);
		InfoBase(sox::Properties &, sox::Properties &);
		~InfoBase(void);

		xstring		getString(uint16_t tag) const;
		std::string	getBString(uint16_t tag) const;
		void	setString(uint16_t tag, const xstring& str);
		void	setBString(uint16_t tag, const std::string& str,bool bForceWrite = false);
		int		getInt(uint16_t tag) const;
		void	setInt(uint16_t tag, int i);
		bool	getBool(uint16_t tag) const;
		void	setBool(uint16_t tag, bool b);
		uint64_t getInt64(uint16_t tag) const;

		sox::Properties	  __getter;
		sox::Properties	  __setter;
	};

}
