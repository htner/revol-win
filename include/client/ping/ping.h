#pragma once


#include <Ws2tcpip.h>
#include <int_types.h>
#include "iphdr.h"

class CPing
{
	struct EchoRequest
	{		
		ICMP_HDR icmp;
		ping_item pingcb;
	};
public:
	static bool  SendPing(int sock,LPCSTR host,int seq)
	{
		if(strlen(host) < 3 )
			return false;
				
		sockaddr_in  desAddr;  
		ZeroMemory(&desAddr, sizeof(desAddr));
		desAddr.sin_family = AF_INET;

		if(((host[0] >='a' && host[0] <= 'z') || (host[0] >='A' && host[0] <= 'Z')) ||
			((host[1] >='a' && host[1] <= 'z') || (host[1] >='A' && host[1] <= 'Z')) ||
			((host[2] >='a' && host[2] <= 'z') || (host[2] >='A' && host[2] <= 'Z')) )
		{
			addrinfo* addOut = NULL;
			getaddrinfo(host, "", NULL, &addOut);
			if(NULL == addOut) {
				return FALSE;
			}
			desAddr.sin_addr.s_addr= ((sockaddr_in *)addOut->ai_addr)->sin_addr.s_addr;
		}else
		{
			desAddr.sin_addr.s_addr= inet_addr(host);
		}
		
		EchoRequest echo = {0};
		//ZeroMemory(&echo,sizeof(echo));

		ICMP_HDR& icmp = echo.icmp;
		static int nseq = 0;
		icmp.type = 8;
		icmp.code = 0;
		icmp.checksum = 0;
		icmp.id = (WORD)GetCurrentProcessId();
		icmp.sequence = seq;
		icmp.timestamp = GetTickCount(); 
		icmp.checksum = CalcCheckSum((uint16_t*)&echo,sizeof(echo));
		
		if(sendto(sock,(LPSTR)&echo,sizeof(echo),0,(SOCKADDR *)&desAddr,sizeof(desAddr)) == SOCKET_ERROR) 
			return false;

		return true;
	}
	static int RecvEchoReply(int sock)
	{	
		char rawdata[sizeof(IPV4_HDR) + sizeof(EchoRequest)] = {0};
		struct sockaddr_in addr		= {0};		
		int nAddrLen				= sizeof(addr);

		int nRet = recvfrom(sock, (char *)rawdata, sizeof(rawdata),
			0, (struct sockaddr *)& addr, &nAddrLen);

		EchoRequest* echo = (EchoRequest*)(rawdata + sizeof(IPV4_HDR));
		IPV4_HDR* iphdr = (IPV4_HDR*)rawdata;
		if (nRet >= sizeof(IPV4_HDR) + sizeof(icmphdr)) 
		{
			if ((iphdr->protocol == IPPROTO_ICMP)) {
				return echo->icmp.sequence;			

			}	
		}		
		return -1;
	};

	/* IN_CKSUM: Internet checksum routine */
	static uint16_t CalcCheckSum(uint16_t *addr, int len)
	{
		int nleft = len;
		uint16_t *w = addr;
		uint16_t answer;
		int sum = 0;
	    
		/*
		 * Our algorithm is simple, using a 32 bit accumulator (sum), we
		 * add sequential 16 bit words to it, and at the end, fold back
		 * all the carry bits from the top 16 bits into the lower 16 bits.
		 */
		while( nleft > 1 )  {
			sum += *w++;
			nleft -= 2;
		}
	    
		/* mop up an odd byte, if necessary */
		if (nleft == 1) {
			uint16_t u = 0;
			
			*(uint8_t *)(&u) = *(uint8_t *)w ;
			sum += u;
		}
	    
		/* add back carry outs from top 16 bits to low 16 bits */
		sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
		sum += (sum >> 16);			/* add carry */
		answer = ~sum;			/* truncate to 16 bits */
		return (answer);
	}
};