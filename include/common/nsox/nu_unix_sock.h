#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>         /* TCP_NODELAY */
#include <netdb.h>   
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <syslog.h>
#include <string.h>
#include <limits.h>

#define POLL_INFTIM					(-1)

typedef bool boolean;
typedef int8_t  BYTE; 
typedef int32_t DWORD_PTR;
typedef int16_t WORD;

#define VSPRINTF vsnprintf
#define	SNPRINTF snprintf


#define NU_EWOULDBLOCK					EWOULDBLOCK
#define NU_EINPROGRESS					EINPROGRESS
#define NU_ECONNABORTED					ECONNABORTED
#define NU_EINTR						EINTR
#define NU_EAGAIN						EAGAIN
#define NU_EINTR						EINTR
#define NU_ETIMEDOUT					ETIMEDOUT
#define NU_EADDRINUSE					EADDRINUSE



