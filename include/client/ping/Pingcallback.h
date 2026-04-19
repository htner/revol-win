#pragma once

struct ping_cb;
struct ping_item
{
	int		_id;
	char	_host[128];
	int		_timeout;
	int		_pingfrm;
	int		_pings;
	int		_seq;
	ping_cb* _cb;
};
struct ping_cb
{
	virtual void on_ping_cb(const ping_item& item)	= 0;
};