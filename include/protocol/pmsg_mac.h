#ifndef PMSG_H_
#define PMSG_H_
#include "common/misc.h"
#include "common/packet.h"


namespace protocol {
namespace session {
struct TextChatFormat : public sox::Marshallable {
public:
	/*TextChatFormat(const LOGFONT& lf) {
		clr = 0;

		setLogfont(lf);
	}
	;*/

	TextChatFormat(void) {
		clr = 0;
	}
	;
public:
        void setLogfont(long h = -13, const std::wstring& name = L"MS Shell", int effects = 0) {
                height = (uint32_t)h;
                ft.name = name;
                ft.effects = effects;
                /*
		if (lf.lfWeight == FW_BOLD)
			ft.effects |= protocol::FE_BOLD;
		if (lf.lfUnderline)
			ft.effects |= protocol::FE_UNDERLINE;
		if (lf.lfItalic)
			ft.effects |= protocol::FE_ITALIC;
		if (lf.lfStrikeOut)
			ft.effects |= protocol::FE_STRIKETHROUGH;*/
	}

	virtual void marshal(sox::Pack &pak) const {
		pak<<ft.effects<<ft.name<<clr<<height;
	}
	virtual void unmarshal(const sox::Unpack &pak) {
		pak>>ft.effects>>ft.name>>clr>>height;
	}
	;

	/*LOGFONT toLogfont(void) const {
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
::		GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);

		::wcscpy(lf.lfFaceName, ft.name.c_str());
		lf.lfWeight = include(ft.effects, protocol::FE_BOLD)?FW_BOLD:FW_NORMAL;
		lf.lfItalic = include(ft.effects, protocol::FE_ITALIC);
		lf.lfUnderline = include(ft.effects, protocol::FE_UNDERLINE);
		lf.lfStrikeOut = include(ft.effects, protocol::FE_STRIKETHROUGH);
		lf.lfHeight = (LONG)height;
		return lf;
	};*/

	COLOR getColor(void) const
	{
		return clr;
	}

	void setColor(COLOR c)
	{
		clr = c;
	}
public:
	FONT ft;

	COLOR clr;

	uint32_t height;
};

struct TextChat : public TextChatFormat {

	uint32_t sd;
protected:
        std::string  netmsg;
public:
	TextChat& operator=(const TextChatFormat& tf)
	{
		ft = tf.ft;
		clr= tf.clr;
		height = tf.height;
		return *this;
	}

       virtual void getUtf16Data(ushort* msg, uint32_t& len) const
        {
            assert(netmsg.size() % sizeof(ushort) == 0);
            if (len < netmsg.size() / sizeof(ushort)) {
                memcpy(msg, netmsg.c_str(),  len * sizeof(ushort));
            } else {
                memcpy(msg, netmsg.c_str(),  netmsg.size());
                len = netmsg.size() / sizeof(ushort);
            }
        }

       virtual uint32_t getutf16len() const{
             assert(netmsg.size() % sizeof(ushort) == 0);
             return netmsg.size() / sizeof(ushort);
        }

        virtual void setutf16Data(const ushort* msg, uint32_t len)
        {
            int nLen = len * sizeof(ushort);
            char *data =  new char[nLen];
            memcpy(data, msg, nLen);
            netmsg.assign(data, nLen);
            delete[] data;
        }
	virtual void marshal(sox::Pack &pak) const
	{
		TextChatFormat::marshal(pak);
                pak.push_varstr32(netmsg.data(), netmsg.size());
                pak << sd;
	}
	virtual void unmarshal(const sox::Unpack &pak)
	{
		TextChatFormat::unmarshal(pak);
                netmsg = pak.pop_varstr32();
                pak >> sd;
        }

	std::string toString() const {
		sox::PackBuffer bf;
		sox::Pack pk(bf);
		this->marshal(pk);
		return std::string(pk.data(), pk.size());
	}

	static TextChat fromString(const std::string &str) {
		sox::Unpack up(str.data(), str.size());
		TextChat tc;
		tc.unmarshal(up);
		return tc;
	}

};

struct VoiceChat: public sox::Marshallable
{
public:
	VoiceChat(void)
	{
		sample_rate = 0;
		sample_type= 0;
	}

	virtual void marshal(sox::Pack &pak) const
	{
		pak << sample_rate << sample_type << payload;
	}
	virtual void unmarshal(const sox::Unpack &pak)
	{
		pak >> sample_rate >> sample_type >> payload;
	};

	std::string toString() const {
		sox::PackBuffer bf;
		sox::Pack pk(bf);
		this->marshal(pk);
		return std::string(pk.data(), pk.size());
	}

	static VoiceChat fromString(const std::string &str) {
		VoiceChat tc;
		if(!str.empty())
		{
			sox::Unpack up(str.data(), str.size());
			tc.unmarshal(up);
		}	
		return tc;
	}
public:
	uint16_t sample_rate;
	uint16_t sample_type;
	std::string payload;
};
}
}
#endif /*PMSG_H_*/
