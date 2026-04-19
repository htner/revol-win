#pragma	  once
#include "client/xconnpoint.h"
#include <protocol/const.h>
#include <client/macros.h>
#include <common/nsox/selector.h>
#include <common/xstring.h>
#include <common/nsox/nulog.h>

#include <string>
#include <vector>
#include <xstring>
#include <list>


typedef uint32_t HULI;				//樹的基本索引
typedef std::vector<HULI> HULILIST;	//

typedef uint32_t UID_T;				//用戶ID
typedef xstring  ACCOUNT_T;			//用戶帳號
typedef uint32_t SID_T;				//頻道ID

#define UID_NULL 0
#define HULI_NULL 0
#define SID_NULL 0

using namespace protocol::login;

 
namespace xproto
{
	//用户性别
	enum UserSex {
			female = 0,
			male
	};

	struct IXObject : virtual public nsox::IAutoPtr {
	};

	struct IProtoObject : public IXObject
	{
			virtual LPCSTR getObjectName() = 0;
	};

	//连接到linkd通知事件
	struct ILinkEvent : public IXObject
	{
			virtual void onLinkStatus(LinkStatus status, int err)  = 0;
			virtual void onUpdateSessKey(const xstring& key) = 0;
	};

	//linkd代理通知
	struct IProxyNoticeEvent : public IXObject
	{
			virtual void onProxyNotice(int type, int res, const xstring& info)  = 0;
	};

	//用户事件通知
	struct IUserEvent : public IXObject
	{
			virtual void onRefresh() = 0;
			virtual void onJoinSessRefresh(uint32_t pid) = 0;
			virtual void onEffectRefresh() = 0;
			virtual void onOpenPlatformRefresh() = 0;
	};

	//獲得用戶id通知
	struct IGetUidEvent : public IXObject
	{
			virtual void onRefresh() = 0;
	};


	//用户信息
	struct CRaidUserSess {
			uint32_t pid;
			uint32_t sid;
			uint32_t contribute;
			uint16_t roler;
			xstring sname;

	};
	struct CRaidUserEffect {
			uint32_t eid;
			uint32_t sid;
			uint32_t type;
			uint32_t starttime;
			uint32_t endtime;
	};
	typedef std::vector<CRaidUserSess> userJoinSess;
	typedef std::vector<CRaidUserEffect> userEffects;


	
	struct OpenPlatformInfo {
		uint32_t id;
		uint32_t id_type;
		uint32_t platform_id;
		xstring platform_user_id;
		xstring platform_user_name;
		xstring pf_token;
		xstring pf_email;
		xstring pf_url;
	};

	typedef std::vector<OpenPlatformInfo> OpenPlatformInfoVector;

	struct IUserInfo : public IXObject
					 , virtual public IConnPoint<IUserEvent>
					 , virtual public IConnPoint<IGetUidEvent>
	{
			
			enum PrivacyLevel {
				EPL_DEAFINE  = 0,
				EPL_NOPRIVACY  = 1,
				EPL_ABSOLUTE = 1024
			};

			virtual UID_T		getUid()		const   = 0;				//用户ID
			virtual ACCOUNT_T	getAccount() const	= 0;				//帐号
			virtual void		setPasswd(xstring&)	= 0;

			virtual xstring		getNick()	const	= 0;				//昵称
			virtual void		setNick(xstring&)	= 0;				//

			

			virtual protocol::uinfo::SEX getSex() const			= 0;	//性别
			virtual void		setSex(protocol::uinfo::SEX)	= 0;

			virtual long		getBirthday() const = 0;				
			virtual void		setBirthday(long)	= 0;

			virtual int			getArea() const		= 0;
			virtual void		setArea(int)		= 0;

			virtual int			getProvince() const = 0;
			virtual void		setProvince(int)	= 0;

			virtual int			getCity() const		= 0;
			virtual void		setCity(int)		= 0;

			virtual uint32_t	getUserJF() const	= 0;				//用户积分

			//主动refresh
			virtual void		refresh()			= 0;				
			virtual void		update()			= 0;
			virtual void		updateUid()			= 0;
			virtual void		updateUserSess(uint32_t pid)	= 0;	
			virtual void		updateEffect()		= 0;

			virtual xstring		getSign()			= 0;				//个性签名
			virtual void		setSign(xstring&)	= 0;

			virtual xstring		getIntro() const	= 0;				//个人简介
			virtual void		setIntro(xstring&)	= 0; 

			virtual uint32_t	getUpdateTime()	= 0; 
			// facebook
			virtual xstring  getFbId() const	= 0;
			virtual xstring  getFbToken() const = 0;

			// money
			virtual int		getGold() const	= 0	;
			virtual int		getSilver() const = 0		;
			virtual int		getFlowers() const	= 0	;

			// user session
			virtual const userJoinSess& getUserSess() const = 0;
			// 勋章
			virtual const userEffects& getEffects() const				= 0;

			virtual PrivacyLevel		getPrivacy() const		 = 0;
			virtual void	setPrivacy(PrivacyLevel privacy) = 0;

			virtual bool  getVisitHistoryPrivacy() const = 0;
			virtual void	setVisitHistoryPrivacy(bool) = 0;
			virtual bool  getCollectSessionPrivacy() const = 0;
			virtual void	setCollectSessionPrivacy(bool) = 0;
			virtual const OpenPlatformInfoVector& getOpenPlatformInfo() = 0;
			virtual void setOpenPlatformInfo(OpenPlatformInfo) = 0;
			virtual void removeOpenPlatformInfo(OpenPlatformInfo) = 0;
	};

	//用户管理模块
	struct IUserManager : public IProtoObject
	{
			virtual IUserInfo* getUserInfo(const UID_T& uid) = 0;
			virtual	IUserInfo*  getUserInfo(const ACCOUNT_T &account) = 0;
	};

	/************************************************************************/
	/* Manage Link Connection to Server                                     */
	/************************************************************************/
	class ILbsConfig {
	public:
			virtual xstring lbsServerAddr() = 0;
			virtual uint16_t	lbsServerPort() = 0;
			virtual const std::list<xstring>* lbsServerIpAddrs() { return NULL; };
	};

	class IAddrConfig {
	public:
			virtual xstring ReportServerAddr() = 0;
	};


	struct IProtoLink : virtual public IConnPoint<ILinkEvent>
								,virtual public IConnPoint<IProxyNoticeEvent>
					  , public IProtoObject
	{
			virtual int				login(const ACCOUNT_T& account,const xstring&, BOOL)			= 0;
			virtual int				login(const UID_T& uid,const xstring&, BOOL)					= 0;
			virtual int				loginByToken(const UID_T &uid, const xstring& token, const xstring& kind)	= 0;
			virtual int				loginByToken(const ACCOUNT_T& account, const xstring& token,const xstring& kind) = 0;
			virtual void			logout()											= 0;
			virtual UID_T			getUid()											= 0;
			virtual ACCOUNT_T		getAccount()										= 0;
			virtual LinkStatus		getStatus()											= 0;
			virtual void			cancelLogin()										= 0;
			virtual xstring			getEncryptPasswd()									= 0;
			virtual void			setMachineCode(const xstring&)						= 0;
			virtual xstring			geytMachineCode() const								= 0;
			virtual void			setClientVersion(uint32_t version)					= 0;
			virtual uint32_t		getClientVersion() const							= 0;
			virtual void			setLbsConfig(ILbsConfig*) 							= 0;
			virtual void			SetAddrConfig(IAddrConfig*) 						= 0;
			virtual xproto::IAddrConfig*	GetAddrConfig()								= 0;
	};

	struct ITreeObject : public IXObject 
	{
	};


	template<typename T = HULI>
	struct ITempTreeEvent;

	//複雜的樹單元
	template<typename T = HULI>
	struct ITempTreeNode
	{
		enum etype {
			E_CHANNEL,
			E_USER
		};

		T value;
		etype type;
		ITempTreeNode<T>& operator =(T& t)
		{
			value=t; 
			type = E_CHANNEL;
			return *this;
		}
		ITempTreeNode<T>(T t, etype e = E_CHANNEL): value(t), type(e){};
		ITempTreeNode<T>(){};

	};

	//树的基本信息
	template<typename T = HULI>
	struct ITempTreeBase : public IXObject
					 , virtual public IConnPoint< ITempTreeEvent<T> >
	{
			virtual bool				hasChild()				= 0;
			virtual	ITempTreeBase<T>*		nextSibling()			= 0;
			virtual	ITempTreeBase<T>*		prevSibling()			= 0;
			virtual ITempTreeBase<T>*		firstChild(const T&)	= 0;
			virtual ITempTreeBase<T>*		lastChild(const T&)		= 0;
			virtual	ITempTreeBase<T>*		find(const T&)			= 0;
			virtual T					getId()					= 0;
			virtual ITempTreeBase<T>*		remove(const T&)		= 0;
			virtual ITempTreeBase<T>*		parent()				= 0;
			virtual	int					size()					= 0;
			virtual ITempTreeBase<T>*		insertBefore(const T&)	= 0;
			virtual ITempTreeBase<T>*		appendChild()			= 0;
			virtual ITreeObject*		getObject()				= 0;
	};






	/************************************************************************/
	/* copy from xxspeak                                                    */
	/************************************************************************/

	//列表更新通知
	template<typename T = HULI>
	struct ITempListEvent : public IXObject
	{
		virtual void onRefresh() = 0;
		virtual void onChange(const T &id, const T &p) = 0;
		virtual void onMoved(const T &id, const T &from, const T &to) = 0;
		virtual void onAdd(const T &id, const T &p) = 0;
		virtual void onRemove(const T &id, const T &p) = 0;
		virtual void onChangeOrder() = 0;
	};

	template<typename T>
	struct ITempListFiller {
		virtual void fillNode(const T& hid, const T& pid) = 0;
	};


	//基本树结构体
	template<typename T = HULI>
	struct ITempListBase : public virtual IConnPoint<ITempListEvent<T> >
					 , public IXObject
	{
			typedef T nodeType;
			typedef std::vector<std::pair<nodeType, nodeType> > PlainTree_t;
			typedef ITempListFiller<nodeType> IListFiller;
			enum { DT_USER = 0x01,DT_CHANNEL = 0x02};

			virtual bool isFolder(const nodeType &hid) const		= 0;
			virtual bool isExisit(const nodeType& hid, const nodeType& pid) const = 0;
			virtual void getParent(const nodeType &hid, std::vector<nodeType>& parents) const= 0;
			virtual LPCWSTR getNick(const nodeType &hid) const	= 0;
			virtual nodeType  getAliaseId(const nodeType &hid) const = 0;
			virtual PlainTree_t getTreeData() const= 0;
			virtual void fillTreeData(IListFiller*,int) const = 0;
			
	};


	typedef  ITempTreeNode<> ITreeNode;
	
	typedef  ITempListBase<> IListBase;
	typedef  ITempListEvent<> IListEvent;
	
	typedef  ITempListBase<ITreeNode> IRoomBase;
	typedef  ITempListEvent<ITreeNode> IRoomEvent;

	//
	struct IActionEvent : public IXObject
	{
		virtual void onResult(int operate, int res) = 0;
	};
	

	/************************************************************************/
	/*                            Im Proto                                  */
	/************************************************************************/
	struct IImEvent: public IXObject {
		virtual void onImResponse(const std::string &data) = 0;
	};

	struct IImBase: public virtual IConnPoint<IImEvent> , public IProtoObject {
		virtual void send(const std::string &data) = 0;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/



	/************************************************************************/
	/*                            linkd proxy                                  */
	/************************************************************************/
	struct IProxyEvent : public IXObject
	{
			virtual void onProxyArrive(const xstring& method, uint32_t  resCode, const xstring&, uint32_t idd)  = 0;
	};

	struct IProxyElem : public IXObject
					 , virtual public IConnPoint<IProxyEvent>
	{
		virtual  xstring getMethod()	 const = 0;
		virtual void request(uint16_t type, const xstring& params, const xstring& postdata) = 0;
	};

	struct IProxyManager : public IProtoObject
	{
			virtual IProxyElem* getProxyElem(const xstring&) = 0;
	};
	/************************************************************************/
	/*                            Im Proto                                  */
	/************************************************************************/


	struct IRaidProto 
	{
			virtual IProtoObject*	queryObject(LPCSTR objName) = 0;
			virtual void set_logger_func(output_function_t func) = 0;
			virtual void set_logger_level(int) = 0;
	};

	extern "C"
	{
			__declspec(dllexport) IRaidProto* CoCreateProto();
	}	

#define DECLARE_PROTO_OBJECT_NAME(xxx)\
	virtual LPCSTR getObjectName() \
	{ return xxx;}
}

