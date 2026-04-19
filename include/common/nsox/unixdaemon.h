#ifndef UNIXDAEMON_H_
#define UNIXDAEMON_H_

#endif /*UNIXDAEMON_H_*/
#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
//#include <io.h>
#include <stdlib.h>
#include <stdio.h>

class init_daemon {
	FILE *newFile;
	int old;
public:
	init_daemon(const char* logfile): old(0) {
		if(logfile == NULL){
				nsox::xlog(NsoxDebug, "need file log name");
				return;
		}
		
		int pid;
		newFile = NULL;
		if ((pid=fork()) != 0)
				exit(0);//是父进程，结束父进程
		else if (pid< 0)
			exit(1);//fork失败，退出
		//是第一子进程，后台继续执行

		setsid();//第一子进程成为新的会话组长和进程组长,//并与控制终端分离
		if ((pid=fork()) != 0)
			exit(0);//是第一子进程，结束第一子进程
		else if (pid< 0)
			exit(1);//fork失败，退出
		//是第二子进程，继续
		//第二子进程不再是会话组长

		//signal(SIGPIPE, SIG_IGN);
			
		for (int i=2; i< NOFILE; ++i)
			//关闭打开的文件描述符
			close(i);
		char *fn = args[1];
		if(!sox::setlog(fn,100)){
				exit(1);
		}
		if (fn) {
			old = dup( 1); // "old" now refers to "stdout" 
			if ((newFile = fopen(fn, "aw")) == NULL ) {
					puts("Can't open file 'data'\n");
					exit( 1);
			}
			dup2(fileno(newFile), 1);
			fflush(stdout);
		}
	}

	~init_daemon() {
		if (newFile) {
			fclose(newFile);
			dup2(old, 1);
		}
	}
};

