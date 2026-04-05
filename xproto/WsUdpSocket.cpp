#include "WsUdpSocket.h"
#include "common/nsox/nulog.h"
#include "mswsock.h"

CWsUdpSocket::CWsUdpSocket(void)
{
	__protoHandler = NULL; 
	__closed = false;
	__connected = false;
}

CWsUdpSocket::~CWsUdpSocket(void)
{
}
void CWsUdpSocket::connect(const char* ip, unsigned port)
{
	memset(&m_addr, 0, sizeof(m_addr));

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = ::inet_addr(ip);
	m_addr.sin_port = htons(port);

	__connected = true;
}
void CWsUdpSocket::send(const char* data, unsigned len) {
	//SendTo(data, len, (SOCKADDR *)&addr, sizeof(addr));
	Sendto(data, len, 0, (sockaddr *)&m_addr, sizeof(m_addr));
}

void CWsUdpSocket::OnReceive(int err) {
	if (err != 0){
			goto error;
	}
	char buffer[1024]; // too big? will stack size overflow?
	int fl = sizeof(m_addr);
	int len = Recvfrom(buffer, sizeof(buffer), 0, (sockaddr *)&m_addr, &fl);
	if (len <= 0 || len >= sizeof(buffer))
	{
			int err = GetLastError();
			if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS) //blocked
			{
				nsox::xlog(NsoxInfo, "receive udp blocked , %s:%s", __FILE__, __LINE__);
					return;
			}else{
					goto error;
			}
	}
	if (__protoHandler){
			__protoHandler->ondata(buffer, len, this);
	}
	return;

error:
	nsox::xlog_err(NsoxError, err, inet_ntoa(m_addr.sin_addr));
	if(__protoHandler){
			__protoHandler->onclose(err, this);
	}
}

void CWsUdpSocket::setProtoHandler(nproto::proto_handler* handler)
{
		__protoHandler = handler;
}

void CWsUdpSocket::create()
{
	 Create(0,SOCK_DGRAM);
	 DWORD   dwBytesReturned;   
	 BOOL   bNewBehavior   =   FALSE;   
	 WSAIoctl(GetSocketHandle(),   SIO_UDP_CONNRESET,   &bNewBehavior,   sizeof(bNewBehavior),   
			NULL,   0,   &dwBytesReturned,   NULL,   NULL);  

}


bool CWsUdpSocket::write_data(const char* p, nsox::nu_size_t len)
{
	Sendto(p, len, 0, (sockaddr *)&m_addr, sizeof(m_addr));
	return true;
}
void CWsUdpSocket::close_conn()
{
	__closed = true;
	Close();
}
bool CWsUdpSocket::is_conn_close()
{
	return __closed;
}
bool CWsUdpSocket::is_conn_alive()
{
	return __connected;
}
