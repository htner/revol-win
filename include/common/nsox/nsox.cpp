// nsox.cpp : 定义控制台应用程序的入口点。
//

#include <string>
#include <iostream>
#include "nulog.h"
#include "nu_except.h"
#include "listen.h"
#include "tcpsocket.h"
#ifdef NSOX_WIN32
    #include "selector_sel.h"
#else
    #include "epoll_selector.h"
#endif

#include "udpsocket.h"
#include "asynudpsocket.h"
#include "nsoxapp.h"
#include "nutimer.h"
#include "nuinit.h"

#include "block_buffer.h"
#include "socket_buffer.h"
#include "nu_packet.h"

#define WIN32_DEBUG



using namespace nsox;

#include "tcpclient.h"

void kick_user(int port, char* file)
{
		tcpclient client;
		try{
				client.create();
				client.connect(address(port, "127.0.0.1"));
		}catch(nsox::socket_error& e){
				nsox::xlog(NsoxInfo, "++++++++++++++test_connect_success() test fail+++++++++++++++");
				return;
		}
}

void test_connect_success()
{
		tcpclient client;
		try{
				client.create();
				client.connect(address(80, "202.108.9.32"));
		}catch(nsox::socket_error& e){
				nsox::xlog(NsoxInfo, "++++++++++++++test_connect_success() test fail+++++++++++++++");
				return;
		}

		std::string sendstr = "GET / HTTP/1.1\r\n";
		sendstr += "Accept: */*\r\n";
		sendstr += "Accept-Language: zh-cn\r\n";
		sendstr += "User-Agent: neou\r\n";
		sendstr += "Host: yy.duowan.com\r\n";
		sendstr += "\r\n";
		client.write(sendstr.data(), sendstr.size());
		char buffer[1024];
		int len;
		while((len = client.read(buffer, 1024)) > 0)
		{
				std::cout << std::string(buffer,len);
		}
		client.close();
}

void test_connect_error()
{
		tcpclient client;
		try{
				client.create();
				client.connect(address(8089, "127.0.0.1"));
		}catch (nsox::connect_error& e) {
				nsox::xlog(NsoxInfo, e.what());
				return;
		}catch(nsox::socket_error& e){
		}
		nsox::xlog(NsoxInfo, "++++++++++++++test_connect_error() test fail+++++++++++++++");
}
void test_udp(int port, const char* addr, int count)
{
           udpsocket  us;
           us.create();
           //us.bind(address(13324));
           us.connect(address(port,addr));


            while(true)
           {
                        std::string line("xxxxx");
                        us.sendstream(line.data(), line.size());
           }

            us.close();

}

void test_connect_fail()
{
		tcpclient client;
		try{
				client.create();
				client.connect(address(8089, "127.0.0.1"));

		}catch(nsox::connect_timeout e){

		}catch (nsox::socket_error e) {
				client.close();
				nsox::xlog(NsoxDebug, e.what());
				return;
		}
		nsox::xlog(NsoxInfo, "++++++++++++++test_connect_fail() test fail+++++++++++++++");
}
void test_connect_timeout()
{
	tcpclient client;
	try{
			client.create();
			client.connect(address(8089, "220.181.23.215"), 100);
	}catch(nsox::connect_timeout e){
			nsox::xlog(NsoxDebug, e.what());
			return;
	}catch (nsox::socket_error e) {

	}
	nsox::xlog(NsoxInfo, "++++++++++++++test_connect_timeout() test fail+++++++++++++++");
}
#include "asyntcpclient.h"
#include "asyntcpsocket.h"

class my_tcp_client : public asyn_tcp_socket<asyn_tcp_client>
{
public:
	my_tcp_client(){};
	~my_tcp_client(){};
	virtual void on_connect_success()
	{
			asyn_tcp_client::onconnected();
			write_request();
	}
	virtual	void on_connect_timeout()
	{
			nsox::xlog(NsoxDebug, "connect timeout");
			asyn_tcp_socket<asyn_tcp_client>::close();
	}
	virtual void on_connect_error(int error)
	{
			nsox::xlog_err(NsoxDebug, error, "connect error");
			asyn_tcp_socket<asyn_tcp_client>::close();
	}
protected:
	void write_request()
	{
			std::string sendstr = "GET / HTTP/1.1\r\n";
			sendstr += "Accept: */*\r\n";
			sendstr += "Accept-Language: zh-cn\r\n";
			sendstr += "User-Agent: neou\r\n";
			sendstr += "Host: yy.duowan.com\r\n";
			sendstr += "\r\n";
			sendstream(sendstr.c_str(), sendstr.size());
	}
	virtual int ondata(const char* p, nu_size_t len)
	{
			std::cout << std::string(p,len) << std::endl;
			return len;
	}


};

void test_asyn_connect()
{
		nu_auto_ptr<my_tcp_client> client = nu_create_object<my_tcp_client>::create_inst();
		client->create();
		client->connect(address(8080, "121.9.249.198"), 500);
		env::inst()->selector()->run(500);
}

#define NETMASK 0xffff0000
bool	is_same_idc_room(uint32_t ip1, uint32_t ip2)
{
		ip1 = ntohl(ip1);
		ip2 = ntohl(ip2);
		return (ip1 & NETMASK)  == (ip2 & NETMASK);
}

#include "asyntcppeer.h"

class console_socket : public asyn_tcp_socket<asyn_tcp_peer>
{
public:
	virtual ~console_socket(){}
	virtual int ondata(const char* p, nu_size_t len)
	{
			if(len < 4) return 0;

			if(len == 4 && memcmp(p, "quit", 4) == 0)
			{
					close();
					env::inst()->selector()->stop();
			}else{
					std::cout << std::string(p, len);
			}
			return len;
	}
	virtual void onclose(int err)
	{
			std::cout << "Good Bye" << std::endl;
	}
};



#include "asynserversocket.h"

class test_udp_server;



void test_udp(int port, const char* addr)
{
		udpsocket  us;
		us.connect(address(port,addr));
		std::string line;
		for(;;)
		{
			std::getline(std::cin, line);
			if(line.empty()) break;
			us.sendstream(line.data(), line.size());
		}
}

class test_udp_server : public nsox_udp_server<asyn_udp_socket>
{
public:
	test_udp_server(int port, const char* ipaddr = NULL) : nsox_udp_server<asyn_udp_socket>(port, ipaddr){}
	virtual void ondata(const char* p, int len,  address& addr)
	{
			__server_socket->sendstream(p, len, addr);
	}
};






int timer_count = 0;

class timeout_handler : public timer_handler
{
public:
	virtual void timeout()
	{
				nsox::xlog(NsoxDebug, "timeout arrivce");
				env_timer_inst->set_timer(this, 255*50);
	}
};

void test_timer()
{
#ifdef NSOX_WIN32

		nu_auto_ptr<timeout_handler> handler = nu_create_object<timeout_handler>::create_inst();
		env_timer_inst->set_timer(handler,50);
		env_selector_inst->run(500);

#endif
}


void udp_flood(const char* p, int port, int count = 100)
{
		udpsocket  us;
		us.create();
		std::string sentdata("xxxxxxxxx");
		for(int i=0; i<count * 1000; ++ i)
		{
				us.sendstream(sentdata.c_str(), 250, address(port, p));
		}
		us.close();
}




struct string_util
{
		enum{
				max_line_size = 1024
		};

		//return length
		static int getline(const char* p, int len)
		{
				int nsize = 0;
				len = len < max_line_size ? len : max_line_size;

              int i = 0;
				for(; i<len; ++ i)
				{
						if(*(p+i) == 0) break;

						if(*(p+i) == '\n') {
								nsize = i + 1; break;
						}
				}
				return nsize;
		}


		static int trim(const char* p, int len)
		{
				while (len --) {
						if(*(p+len) == '\r' || *(p+len) == '\n') {
								continue;
						}
						return len + 1;
				}
				return 0;
		}
		//return length
		static int token(const char* p, int len, char c)
		{
				int i=0;
				for(; i<len; ++ i){
						if(*p == c) return (i + 1);
				}
				return 0;
		}
};



class http_handler : public asyn_tcp_socket<asyn_tcp_peer>
{
public:
		http_handler(){};
		~http_handler(){};
		struct http_header{
				std::string line;
		};

		virtual int ondata(const char* data, nu_size_t len)
		{
				 const char* p  = data;
				 do
				 {
						 int line_len = string_util::getline(p, len);
						 if(line_len){
								int trim_len = string_util::trim(p, line_len);
								if(trim_len == 0){
										on_http_end();
								}else{
										http_header header;
										header.line.append(p, trim_len);
										nsox::xlog(NsoxDebug, header.line.c_str());
								}
								p	+= line_len;
								len	-= line_len;
						 }else{
								 break;
						 }

				 }while(true);

				 return p - data;
		}
		virtual void on_http_end()
		{
				std::string http_response ="\
				\r\nHTTP/1.1 200 OK\
				\r\nDate: Fri, 23 May 2008 07:14:56 GMT\
				\r\nServer: Apache/2.2.4 (Ubuntu) PHP/5.2.3-1ubuntu6.3\
				\r\nLast-Modified: Fri, 21 Mar 2008 04:07:41 GMT\
				\r\nAccept-Ranges: bytes\
				\r\nContent-Length: 17\
				\r\nConnection: close\
				\r\nContent-Type: text/html\
				\r\n\r\nHi, You are here";
				asyn_tcp_socket<asyn_tcp_peer>::sendstream(
												http_response.data(),
												http_response.length());
				asyn_tcp_socket<asyn_tcp_peer>::close();
		}
protected:
		std::list<http_header> __http_headers;
};


void test_accept()
{
		nsoxapp netapp;


		nsox_tcp_server<console_socket> consoleserver(443);
		netapp.add_server(&consoleserver);

		nsox_tcp_server<http_handler> httpserver(80);
		netapp.add_server(&httpserver);

		//test_udp_server udpserver(3324);
		//netapp.add_server(&udpserver);

		env::inst()->selector()->run(500);

		httpserver.stop();
		consoleserver.stop();
}


void test_getline()
{
		std::string teststr= "1234\r\n567\r89\n\n\r";

		const char* p = teststr.data(); int len = teststr.length();
		do{
				int line_len = string_util::getline(p, len);
				if(line_len){
						 p		+= line_len;
						 len	-= line_len;
				}else{
						break;
				}

		}while(true);
}

struct timecounter
{
	int i;
	void start()
	{
		i = SocketUtil::get_now_time();	
	}
	void stop()
	{
		i = SocketUtil::get_now_time() - i;
	}
	int count()
	{
		 return i;
	}
};

#include "packet.h"

void test_buffer()
{
	

	

	timecounter tc;

	{

		nsox::default_pack_buffer	pb;
		nsox::default_pack			pk(pb);

		tc.start();
		for(int i=0; i < 500000; i ++)
		{
			pk.push_varstr(" i love china for ever !");
			pk.push_uint32(i);
		}
		tc.stop();
		nsox::xlog(NsoxDebug,"nubuffer %d", tc.count());

		tc.start();		
		nsox::default_unpack upk(pb.bb);
		for(int i=0; i < 500000; i ++)
		{			
			std::string xx;
			upk.pop_str(xx);
			upk.pop_uint32();
		}
		tc.stop();

		nsox::xlog(NsoxDebug,"nsox unpack %d", tc.count());
		assert(upk.xupb.length() == 0);
	}


	{
		

		sox::PackBuffer pbx;
		sox::Pack packx(pbx);

		tc.start();
		for(int i=0; i < 500000; i ++)
		{
			packx.push_varstr(" i love china for ever !");
			packx.push_uint32(i);
		}
		tc.stop();
		nsox::xlog(NsoxDebug,"sox buffer %d", tc.count());

		tc.start();
		sox::Unpack xupx(pbx.data(), pbx.size());
		for(int i=0; i < 500000; i ++)
		{
			std::string xx = xupx.pop_varstr();	
			xupx.pop_uint32();
		}
		tc.stop();
		nsox::xlog(NsoxDebug,"sox unpack %d", tc.count());
	}
	




}


void do_test()
{

		address xx(0, "58.248.30.132");

		uint32_t xxxx = ntohl(xx.ip);
		//test_asyn_connect();
		test_accept();
}

void dump_session(int port, char* session)
{
		tcpclient client;
		try{
				client.create();
				client.connect(address(port, "127.0.0.1"));

				std::string value = "dump ";
				value += session;
				value += "\r\n";

				client.write(value.c_str(), value.length());

				char buffer[1024]; memzero(buffer);
				client.read(buffer,1024);



				std::cout << buffer ;
		}catch (nsox::connect_error& e) {
				nsox::xlog(NsoxInfo, e.what());
				return;
		}catch(nsox::socket_error& e){
		}
}

#ifdef _DEBUG
#include "crtdbg.h"
#endif


template<class _Container>
class set_insert_iterator
{	// wrap inserts into container as output iterator
public:
		typedef _Container container_type;
		typedef typename _Container::reference reference;

		set_insert_iterator(_Container& _Cont)
				: container(&_Cont)
		{
		}

		set_insert_iterator<_Container>& operator=(
				typename _Container::const_reference _Val)
		{	// insert into container and increment stored iterator
				container->insert(_Val);
				return (*this);
		}

		set_insert_iterator<_Container>& operator*()
		{	// pretend to return designated value
				return (*this);
		}

		set_insert_iterator<_Container>& operator++()
		{	// pretend to preincrement
				return (*this);
		}

		set_insert_iterator<_Container>& operator++(int)
		{	// pretend to postincrement
				return (*this);
		}

protected:
		_Container *container;	// pointer to container
		typename _Container::iterator iter;	// iterator into container
};

template<class _Container> inline
set_insert_iterator<_Container> set_inserter(_Container& _Cont)
{	// return insert_iterator
		return (set_insert_iterator<_Container>(_Cont));
}





#include <iostream>
#include <set>
#include <deque>

using namespace std;


int main(int argc, char* argv[])
{

#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_crtBreakAlloc = 112;
#endif

#ifdef NSOX_WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD( 2, 2 ), &wsaData );
#endif

	using namespace nsox;

	nsox::init_timer<nutimer>				timer_initer;
#ifdef NSOX_WIN32
	nsox::init_selector<selector_select>	selector_inst;
#else
	nsox::init_selector<epoll_selector>	    selector_inst;
#endif

   test_udp(36349, "37.58.127.12", 1);

  //test_buffer();
#ifdef NSOX_WIN32
	WSACleanup();
#endif
}

