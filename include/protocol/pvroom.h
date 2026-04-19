#ifndef  __pvroom_h
#define  __pvroom_h



#include <nsox/nu_request.h>
#include <nsox/property.h>
#include <set>
#include <list>


namespace vroom
{

enum RQR
{
	CHAANEL

};

//...client to server
// 请求登陆
//...
struct CLogin :
		public nproto::request
{
		
		enum{uri = 100};

		std::string username;
		std::string password;


		virtual ~CLogin(){}
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << username << password;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> username >> password;
		}
};



//...server to client
//是否登陆成功
//...
struct SLoginRep :
		public nproto::request
{
		
		enum{uri = 100};

		uint32_t	error;
		std::string describe;


		virtual ~SLoginRep(){}
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << error << describe;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> error >> describe;
		}
};


//...client to server
//请求号(向服务器请求资源)
//...
struct Crequestresources:public nproto::request
{
		enum{uri = 100};

		uint32_t	resourceNum;


		virtual ~Crequestresources(){}
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << resourceNum;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> resourceNum;
		}
};

//... both
//个人信息 (如果客户端向服务器端发送这个信息，表示更改)
//...
 struct BGetPrivacyInfo:
		public nproto::request
{
		
		enum{uri = 102};

		uint32_t	userid;
		std::string username;
		std::string nickname;
		std::string PersonalSignature;
		std::string sex;
		std::string brithday;
		//...

		virtual ~BGetPrivacyInfo(){}
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << username ;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> username ;
		}
};


//... 
//朋友的个人信息
//...
struct FriendInfo : public sox::Marshallable
{
		std::string username;
		uint32_t userid;
		std::string other;

		virtual void marshal(sox::Pack & pk) const
		{
				pk << username << userid << other ;
				//sox::marshal_container(pk, slaves);
		}
		virtual void unmarshal(const sox::Unpack& upk)
		{
				upk >> username >> userid >> other;
				//sox::unmarshal_container(upk, std::back_inserter(slaves));
		}
};


//...server to client
//...返回朋友的详细信息
//
struct SFriendInfo: public nproto::response
{
		enum {uri = 103};
		uint32_t friendId;
		FriendInfo finfo;

		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << finfo << friendId;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> finfo >> friendId;
		}
};



//...
//...朋友的详细列表
//
struct FriendList:
		public  sox::Marshallable
{	
		std::list<std::string> friendlist;

		virtual void marshal(sox::Pack & pk) const
		{
				sox::marshal_container(pk, friendlist);
		}
		virtual void unmarshal(const sox::Unpack& upk)
		{
				sox::unmarshal_container(upk, std::back_inserter(friendlist));
		}
};



//...server to client
//...返回读取朋友列表
struct SFriendList: public nproto::response
{
		enum {uri = 105};
		FriendList finfo;

		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << finfo ;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> finfo ;
		}
};


//..频道列表
struct ChannelList:
		public  sox::Marshallable
{	
		std::list<std::string> Channels;

		virtual void marshal(sox::Pack & pk) const
		{
				sox::marshal_container(pk, Channels);
		}
		virtual void unmarshal(const sox::Unpack& upk)
		{
				sox::unmarshal_container(upk, std::back_inserter(Channels));
		}
};


//...某个频道的详细信息
struct ChannelInfo : public sox::Marshallable
{
		uint32_t channelId;
		std::string name;
		uint32_t isp;
		uint32_t pid;
		ChannelList subchannel;

		virtual void marshal(sox::Pack & pk) const
		{
				pk << channelId << name << isp << pid << subchannel;
				//sox::marshal_container(pk, slaves);
		}
		virtual void unmarshal(const sox::Unpack& upk)
		{
				upk >> channelId >> name >> isp >> pid >> subchannel;
				//sox::unmarshal_container(upk, std::back_inserter(slaves));
		}
};
//...server to client
//...返回某个频道的详细信息
struct SChannelInfo
	: public nproto::response
{
		enum {uri = 103};
		ChannelInfo ci;

		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << ci ;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> ci ;
		}
};


//...server to client
//...返回某个频道列表
struct SChannelList: public nproto::response
{
		enum {uri = 105};
		ChannelList cl;
		uint32_t 	cid;

		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << cl << cid ;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> cl >> cid ;
		}
};


//...client to server
//...创建一个频道
//
struct CCreateChannelRep : public nproto::response
{
	enum {uri = 105};
	std::string name;
	uint32_t pid;
	uint32_t isp;
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << name << pid << isp;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> name >> pid >>  isp;
		}
};

//...client to server
//...进入某一个频道
//
struct CLoginInChannelRep : public nproto::response
{
	enum {uri = 105};
	std::string id;
	uint32_t  xx;
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << id << xx;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> id >> xx;
		}
};


//...client to server
//...退出某一个频道
//
struct CLoginOutChannelRep : public nproto::response
{
	enum {uri = 105};
	uint32_t id;
		virtual void pack(sox::Pack& pk){
				nproto::request::pack(pk);
				pk << id;
		}
		virtual void unpack(sox::Unpack& upk){
				nproto::request::unpack(upk);
				upk >> id;
		}
};




}


#endif // 