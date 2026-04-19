#ifndef __nu_log_h
#define __nu_log_h

#include <iostream>
#include <stdarg.h>		/* ANSI C header file */
#include "sockcomm.h"
#include "socketutil.h"
#include <time.h>
#include <stdio.h>



#define MAXLINE     4096

enum nsox_log_level
{
	NsoxDebug = 0,
	NsoxInfo  = 1,
	NsoxWarn  = 2,
	NsoxError = 3,
	NsoxFatal  = 4,
};

typedef void (*output_function_t)(const char*, const char*, const char*);

nsox_namespace_begin


static const char * levelnames[NsoxFatal+1] ={" debug - "," info  - ", " WARN  - ", " ERROR - ", " FATAL - "};


#ifdef NSOX_WIN32
    inline void default_log_function(const char* time, const char* level, const char* logstr)
    {	     
         OutputDebugStringA(time);
         OutputDebugStringA(level);
         OutputDebugStringA(logstr);
    }
#else
    inline void default_log_function(const char* time, const char* level, const char* logstr)
    {
         std::cout << time << level << logstr;
		  std::cout.flush();
    }
#endif



class logger
{
public:
    static logger* inst(){
            static logger inst;
            return &inst;
    }
private:
    logger()
    {
		__log_level = NsoxDebug;

		__log_func  = default_log_function;
		set_logger_time();
    }
    virtual ~logger()
    {}
public:
    void err_doit(int  error, int level, const char *fmt, va_list ap)
    {
#ifdef LOG_ALL
		set_logger_time();
#endif
        char	buf[MAXLINE + 1];
        VSPRINTF(buf, MAXLINE, fmt, ap);		/* safe */

        int n = strlen(buf);
        if (error){
            std::string errmsg; SocketUtil::format_error(error, errmsg);
            SNPRINTF(buf + n, MAXLINE - n, ": %s", errmsg.c_str());
        }
        strcat(buf, "\r\n");

        __log_func(__log_time, lever_to_name(level),buf);
    }
    void log(int level, const char* fmt, ...)
    {
            if(level < __log_level) return;

            va_list		ap;

            va_start(ap, fmt);
            err_doit(0, level, fmt, ap);
            va_end(ap);
    }

     void log_err(int level, int err, const char* fmt, ...)
    {
            if(level < __log_level) return;

            va_list		ap;

            va_start(ap, fmt);
            err_doit(err, level, fmt, ap);
            va_end(ap);
    }

    void   set_logger_time()
    {
           memzero(__log_time);

           time_t now = (time_t)time(NULL);
           char* timestr =  ctime(&now);
           if(timestr){
              char * p =  (char *)memccpy(__log_time, timestr, 0, 255);
			   if(p) p[-2] = NULL; // remove "\r"
           }
    }

	void set_logger_func(output_function_t func){
			__log_func = func;
	}

	void set_logger_level(int nlevel)
	{
		__log_level = nlevel;
	}

    char*  get_logger_time()
    {
            return __log_time;
    }
private:
    static const char * lever_to_name(int l)
    {
        return levelnames[l];
    }
private:
    int							__log_level;
    output_function_t			__log_func;
    char						__log_time[256];
};

#define xlog logger::inst()->log

#define xlog_err logger::inst()->log_err

#define xlog_set_logfun(fun) logger::inst()->set_logger_func(fun)
#define xlog_set_loglevel(level) logger::inst()->set_logger_level(level)

nsox_namespace_end
#endif



