#pragma	  once

#include <list>
#include <string>



typedef std::pair<std::string, std::string> http_header;
typedef std::pair<std::string, std::string> http_post_value;


struct http_post_data
{
	typedef std::list<http_post_value>::iterator value_iterator;
	typedef std::list<http_header>::iterator header_iterator;

	std::list<http_header>		headers;
	std::list<http_post_value>	values;
	void x_push_header(const std::string& key, const std::string& value)
	{
		headers.push_back(http_post_value(key,value));
	}
	void x_push_value(const std::string& key, const std::string& value){
		values.push_back(http_post_value(key,value));
	}
};

