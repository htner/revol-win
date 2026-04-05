#pragma once


#include "protocol/plogin.h"
#include "common/iproperty.h"
#include "infobase.h"
#include <map>

#include "proto/proto_obj_name.h"
#include "common/nsox/default_proto_handler.h"
#include "proto/iproto.h"
#include "common/nsox/proto_cast.h"
#include "common/nsox/nuautoptr.h"
#include "timer/TimerWrap.h"

//更新群的收藏数，喜欢数量，会员数，个人
#define SESSION_LIKES_PATH					_T("session.likes") 

//获取个人的第三方应用情况
#define LINK_GET_APP_INFO					_T("link.getappinfos") 

//设置个人的第三方应用情况
#define LINK_SET_APP_INFO					_T("link.setappinfos") 



using namespace protocol::uinfo;
namespace xproto
{
	class CProtoLink;
	class CProxyManager;

	class CProxyElem : public IProxyElem
					, public XConnPoint<IProxyEvent>
	{
			friend class CProxyManager;
	public:
			CProxyElem();
			virtual ~CProxyElem(){}

			virtual  xstring	getMethod()		const;
			 void  setMethod(const xstring& m);

			virtual void request(uint16_t type,  const xstring& s, const xstring& );
	protected:
			void	onGetProxy(XCast<PClientDcProxyRes>& cmd, nproto::connection* c);
			void	setManager(CProxyManager* mgr);

	private:
		xstring		__method;
		CProxyManager* __proxyMgr;

	};


	class CProxyManager : public IProxyManager
						   , public nproto::default_proto_handler
	{
		friend class CProxyElem;
	public:
		CProxyManager();
		virtual ~CProxyManager(void);

		DECLARE_PROTO_OBJECT_NAME(PROXY_MANAGER_OBJ);

		BEGIN_REQUEST_MAP(CProxyElem)
			REQUEST_HANDLER(XCast<PClientDcProxyRes>, onGetProxy)
		END_REQUEST_MAP()
		void onclose(int err, nproto::connection* c){};		
		void oncreate(nproto::connection* c){};

		void setProtoLink(CProtoLink* link);
		void onGetProxy(XCast<PClientDcProxyRes>& cmd, nproto::connection* c);
		virtual IProxyElem* getProxyElem(const xstring&);
		void onListTimeout();
	protected:
		void writeRequest(XCast<PClientDcProxy>& req);
		
	protected:
		typedef std::map<std::string, nsox::nu_auto_ptr<CProxyElem> > raid_proxy_map;
		typedef std::list<XCast<PClientDcProxy> > req_list_t;
	protected:
		CProtoLink* __protoLink;
		
		raid_proxy_map	__elems;
		DWORD 			__lasttime;
		uint32_t			__count;
		req_list_t		__reqlist;
		TimerHandler<CProxyManager>	__listTimer; 
	};

	template <typename P>
	class ProxyHanler{
	public:
		struct CProxyEvent : public IProxyEvent {
				public:
					void setProxyHanler(ProxyHanler* p) {
						_p = p;	
					}

					virtual void onProxyArrive(const xstring& method, uint32_t  resCode, const xstring& res, uint32_t idd)  {
						if (_p) {		
							_p->onProxyArrive(method, resCode, res, idd);
						}
					}

					CProxyEvent() {
					}

					~CProxyEvent() {
						if (m_proxyelem)  {
							m_proxyelem->Revoke(this);
							m_proxyelem = NULL;
						}
					}

					void Init(const xstring& method) {
						 IProxyManager* pIpm = (IProxyManager*)CoCreateProto()->queryObject(PROXY_MANAGER_OBJ) ;
						 m_proxyelem = pIpm->getProxyElem(method);
						  if (m_proxyelem) {
								m_proxyelem->Watch(this);
						}
					}

					void Uninit() {
						if (m_proxyelem)  {
							m_proxyelem->Revoke(this);
							m_proxyelem = NULL;
						}
					}
						
			private:
					ProxyHanler* _p;
					nsox::nu_auto_ptr<xproto::IProxyElem> m_proxyelem;
		};
		
		typedef void (P::*callback)(const xstring& method, uint32_t resCode, const xstring& res, uint32_t idd);

		void init(P* x, callback cb, const xstring& method) {
			if (m_h == NULL) {
				m_h = nsox::nu_create_object<CProxyEvent>::create_inst();
				m_h->setProxyHanler(this);
				m_h->Init(method);
				
				_x = x;
				ptr = cb;
				__method = method;
			}
		}

		void uninit() {
			if (m_h != NULL) {
				m_h->Uninit();
				m_h = NULL;
				__method .clear();
				_x = NULL;
			}
		}

		virtual void request(uint16_t type,  const xstring& s, const xstring& p )
		{
			if (__method != _T("")) {
				IProxyManager* pIpm = (IProxyManager*)CoCreateProto()->queryObject(PROXY_MANAGER_OBJ) ;
				nsox::nu_auto_ptr<xproto::IProxyElem> m_proxyelem = pIpm->getProxyElem(__method);
				if (m_proxyelem) {
					m_proxyelem->request(type, s, p);
				}
			}
		}

		virtual void onProxyArrive(const xstring& method, uint32_t  resCode, const xstring& res, uint32_t idd)  {
			if (_x) {		
				(_x->*ptr)(method, resCode, res, idd);
			}
		}
		
	protected:
		P* _x;
		callback ptr;
		nsox::nu_auto_ptr<CProxyEvent>			m_h;
		xstring		__method;
		
	public:
		ProxyHanler() { }
		~ProxyHanler() { }
	};
}