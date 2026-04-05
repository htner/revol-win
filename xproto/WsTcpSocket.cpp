#include "wstcpsocket.h"
#include <common/nsox/nulog.h>
#include <common/nsox/nuautoptr.h>
#include <common/nsox/nu_request.h>


CWsTcpSocket::CWsTcpSocket(void)
{
		__protoHandler	= NULL;
		__connected		= false;
		__closed		= false;
		InitializeCriticalSection(&__output_cs);
}

CWsTcpSocket::~CWsTcpSocket(void)
{
	DeleteCriticalSection(&__output_cs);
}

void CWsTcpSocket::OnClose(int err)
{
	__closed = true;

	nsox::xlog_err(NsoxError, err, "Socket Closed");
	if(err != 0 && __protoHandler){
			return __protoHandler->onclose(err,this);
	}
	if (__protoHandler)
		__protoHandler->onclose(err, this);		
}
void CWsTcpSocket::OnConnect(int err)
{
	if(err != 0 && __protoHandler){
		return __protoHandler->onclose(err,this);
	}
	__connected = true;
	if (__protoHandler) 
		__protoHandler->oncreate(this);\
	EnterCriticalSection(&__output_cs);
	if (!__output.empty()) {
			AsyncSelect(FD_READ | FD_OOB | FD_CONNECT | FD_CLOSE | FD_WRITE);
	}	
	LeaveCriticalSection(&__output_cs);
}
void CWsTcpSocket::OnReceive(int err)
{
	nsox::nu_auto_ptr<nproto::connection> pthis = (nproto::connection*)this; //hold this to prevent delete

	if(err != 0){
		nsox::xlog_err(NsoxError, err, "Socket Recieved Error 1");
		if (__protoHandler)
			__protoHandler->onclose(err,this);
		return;
	}
	if (__input.freespace() < __input.blocksize()) {
		__input.reserve(__input.size() + __input.blocksize());
	}
	__input.reserve(__input.size() + __input.blocksize());	//reserve some space

	unsigned free = __input.freespace();

	int len = Receive(__input.tail(), free, 0); //put data into buffer
	if(len <= 0){ // if error occured 
		int err = GetLastError();
		if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS){ //receive blocked, next time
			nsox::xlog(NsoxInfo, "receive tcp blocked");
		}else{	//i am not sure this code will arrive?
			nsox::xlog_err(NsoxInfo, err, "Socket Recieved Error 2"); 
			if (__protoHandler)
				__protoHandler->onclose(err,this);
		}

	}else{
		__input.size(__input.size() + len);
		if (__protoHandler) {
			int erase = __protoHandler->ondata(__input.data(), __input.size(),this);
			if(erase){
				__input.erase(0, erase);
			}				
		}
	}
}
void	CWsTcpSocket::AsynConnect(const char* host, UINT port)
{
	BOOL ret = CAsyncSocketEx::Connect(host, port);
	if(!ret){
		int err = GetLastError();
		if (err == WSAEWOULDBLOCK) {
			return;
		}else{
			nsox::xlog_err(NsoxInfo, err, "Connect Error ");
			if (__protoHandler)
				__protoHandler->onclose(err,this);
		}
	}
}
void CWsTcpSocket::OnSend(int err)
{
	if(err != 0){
		nsox::xlog_err(NsoxError, err, "OnSend Error 1");
		if (__protoHandler)
			__protoHandler->onclose(err,this);
		return;
	}
	EnterCriticalSection(&__output_cs);  // lock __output
	if(!__output.empty()){
		int sended = CAsyncSocketEx::Send(__output.data(), __output.size());
		if (sended <= 0) {//ÅÐ¶Ï·¢ËÍ½á¹û
			LeaveCriticalSection(&__output_cs);
			int err = GetLastError();
			if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS){
				nsox::xlog(NsoxInfo, "send tcp blocked");
			}else{
				nsox::xlog_err(NsoxInfo, err, "OnSend Error 2");
				if (__protoHandler)
					__protoHandler->onclose(err,this);
				return;
			}
		}else{
			__output.erase(0, sended);
		}
	}

	if(!__output.empty()){
		AsyncSelect(FD_READ | FD_OOB | FD_CONNECT | FD_CLOSE | FD_WRITE);
	}else{
		AsyncSelect(FD_READ | FD_OOB | FD_CONNECT | FD_CLOSE);
	}
	LeaveCriticalSection(&__output_cs);
}
void CWsTcpSocket::AsynSend(const char* data, size_t len)
{
	EnterCriticalSection(&__output_cs);  // lock __output
	__output.append(data, len);
	if(!__output.empty()){
		AsyncSelect(FD_READ | FD_OOB | FD_CONNECT | FD_CLOSE | FD_WRITE);
	}
	LeaveCriticalSection(&__output_cs);
	//OnSend(0);
}
void CWsTcpSocket::SetProtoHandler(nproto::proto_handler* handler)
{
	__protoHandler = handler;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

bool CWsTcpSocket::write_data(const char* p, nsox::nu_size_t len)
{
		AsynSend(p,len);
		return true;
}
void CWsTcpSocket::close_conn()
{
		__closed = true;
		CWsTcpSocket::Close();
}
bool CWsTcpSocket::is_conn_close()
{
		return __closed;
}
bool CWsTcpSocket::is_conn_alive()
{
		return __connected;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

CEncryTcpSocket::CEncryTcpSocket()
{
	__bEncypt = false;
}

bool CEncryTcpSocket::write_data(const char* p, nsox::nu_size_t len)
{
		AsynSend(p,len);
		return true;
}

void CEncryTcpSocket::AsynSend(const char* data, size_t len)
{		
		if (__bEncypt) {
			std::string xxx; xxx.assign(data,len);
			RC4(&__sendKey, len, (const unsigned char*)xxx.data(), (unsigned char *)xxx.data());
			CWsTcpSocket::AsynSend(xxx.data(),len);
		}else{
			CWsTcpSocket::AsynSend(data,len);
		}

}
void CEncryTcpSocket::setEncKey(const unsigned char *k, size_t len){
	if(k){
			RC4_set_key(&__sendKey, len, (const unsigned char *)k);
			RC4_set_key(&__recvKey, len, (const unsigned char *)k);
			__bEncypt = true;
	}else{
			__bEncypt = false;
	}
}
void CEncryTcpSocket::OnReceive(int err)
{ // copy from CWsTcpSocket

	nsox::nu_auto_ptr<nproto::connection> pthis = (nproto::connection*)this; //hold this to prevent delete

	if(err != 0){
		nsox::xlog_err(NsoxError, err, "Socket Recieved Error 1");
		if (__protoHandler)
			__protoHandler->onclose(err,this);
	}
	if (__input.freespace() < __input.blocksize()) {
		__input.reserve(__input.size() + __input.blocksize());
	}
	__input.reserve(__input.size() + __input.blocksize());	//reserve some space

	unsigned int free = __input.freespace();
	unsigned int tail = __input.size();

	int len = Receive(__input.tail(), free, 0); //put data into buffer
	__input.size(__input.size() + len);

	if(len <= 0){ // if error occured 
		int err = GetLastError();
		if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS){ //receive blocked, next time
			nsox::xlog(NsoxInfo, "receive tcp blocked");
		}else{	//i am not sure this code will arrive?
			nsox::xlog_err(NsoxInfo, err, "Socket Recieved Error 2"); 
			if (__protoHandler)
				__protoHandler->onclose(err,this);
		}

	}else{
		
		if (__bEncypt){
			RC4(&__recvKey, __input.size()-tail, (const unsigned char *)__input.data()+tail, (unsigned char *)__input.data()+tail);
		}
		if (__protoHandler) {
			int erase = __protoHandler->ondata(__input.data(), __input.size(),this);
			if(erase){
				__input.erase(0, erase);
			}		
		}
	}
}

bool    CEncryTcpSocket::isEncrypto()
{
		return __bEncypt;
}

void	CEncryTcpSocket::SendBuffer() {
	OnSend(0);
}