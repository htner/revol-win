#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>
#include "macros.h"

class cb_receiver
{
public:
	cb_receiver(){};
	virtual ~cb_receiver(){};
public:
	/*
	* 函数名称：cb_caller::on_call
	* 函数描述：收到回调消息
	* 输入参数：dwCode,回调的代码; wParam, lParam, 参数
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	* 注意	  ：返回的值要根据具体的对象才能决定其意义
	*/
	virtual int on_call(DWORD dwCode, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	};
};

template<typename D, class T> class cb_id_caller
{
public:
	cb_id_caller(){};
	virtual ~cb_id_caller()
	{
		m_map_receiver.clear();
	};
public:
	/*
	* 函数名称：cb_id_caller::empty
	* 函数描述：检查回调对象的数组是否为空
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：如果回调对象数组为空返回true;否则返回false
	*/
	bool empty(void)
	{
		return (0 == m_map_receiver.size());
	};

	/*
	* 函数名称：cb_id_caller::registercb
	* 函数描述：注册回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool registercb(const D& id, T* p)
	{
		if(NULL == p)
			return false;

		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == p)
				return true;
		}

		m_map_receiver[id].push_back(p);

		return true;
	};

	/*
	* 函数名称：cb_id_caller::unregistercb
	* 函数描述：注销回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool unregistercb(const D& id, T* p)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == p)
				it->second.erase(ait);
		}

		return true;
	};

	/*
	* 函数名称：cb_id_caller::call
	* 函数描述：发出回调消息
	* 输入参数：dwCode, 回调的代码; wpraram, lparam, 参数
	* 输出参数：void
	* 返回值  ：返回一个整数
	* 注意	  ：返回的值要根据具体的对象才能决定其意义
	*/
	virtual int call(const D& id, DWORD dwCode, WPARAM wParam, LPARAM lParam)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			(*ait)->on_call(dwCode, wParam, lParam);
		}

		return 0;
	};

	/*
	* 函数名称：cb_id_caller::is_rev_exisit
	* 函数描述：检查callback对象是否存在
	* 输入参数：pReceiver, 接收者的指针
	* 输出参数：void
	* 返回值  ：如果pReceiver已经存在于callback列表中，返回true;否则返回false
	*/
	bool is_rev_exisit(const D& id, T* ptr)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == p)
				return true;
		}
		return false;
	};

	/*
	* 函数名称：cb_id_caller::clear
	* 函数描述：清空回调对象
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：void
	*/
	void	clear(void)
	{
		m_map_receiver.clear();
	};

	/*
	* 函数名称：cb_call::size
	* 函数描述：获取回调对象的数量
	* 输入参数：void
	* 输出参数：返回回调对象的数量
	* 返回值  ：void
	*/
	unsigned int size(void)
	{
		return m_vt_receiver.size();
	}
protected:
	std::map<D, std::vector<T*> > m_map_receiver;
};

template<typename D, class T> class cb_id_call
{
public:
	cb_id_call(){};
	virtual ~cb_id_call()
	{
		m_map_receiver.clear();
	};
public:
	/*
	* 函数名称：cb_id_call::empty
	* 函数描述：检查回调对象的数组是否为空
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：如果回调对象数组为空返回true;否则返回false
	*/
	bool empty(void)
	{
		return (0 == m_map_receiver.size());
	};

	/*
	* 函数名称：cb_id_call::registercb
	* 函数描述：注册回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool registercb(const D& id, T* ptr)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == ptr)
			{
				return true;
			}
		}

		m_map_receiver[id].push_back(ptr);

		return true;
	};

	/*
	* 函数名称：cb_id_call::unregistercb
	* 函数描述：注销回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool unregistercb(const D& id, T* ptr)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == ptr)
			{
				it->second.erase(ait);

				return true;
			}
		}

		return false;
	};

	/*
	* 函数名称：cb_id_call::is_rev_exisit
	* 函数描述：检查callback对象是否存在
	* 输入参数：pReceiver, 接收者的指针
	* 输出参数：void
	* 返回值  ：如果pReceiver已经存在于callback列表中，返回true;否则返回false
	*/
	bool is_rev_exisit(const D& id, T* ptr)
	{
		std::map<D, std::vector<T*> >::iterator it = m_map_receiver.find(id);
		if(it == m_map_receiver.end())
			return false;

		for(std::vector<T*>::iterator ait = it->second.begin(); ait != it->second.end(); ++ait)
		{
			if(*ait == ptr)
				return true;
		}
		return false;
	};

	/*
	* 函数名称：cb_id_call::clear
	* 函数描述：清空回调对象
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：void
	*/
	void	clear(void)
	{
		m_map_receiver.clear();
	};

	/*
	* 函数名称：cb_call::size
	* 函数描述：获取回调对象的数量
	* 输入参数：void
	* 输出参数：返回回调对象的数量
	* 返回值  ：void
	*/
	unsigned int size(void)
	{
		return m_vt_receiver.size();
	}
protected:
	std::map<D, std::vector<T*> > m_map_receiver;
};

template<class T> class cb_call
{
public:
	cb_call(){};
	virtual ~cb_call()
	{
		m_vt_receiver.clear();
	};
public:
	/*
	* 函数名称：cb_call::empty
	* 函数描述：检查回调对象的数组是否为空
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：如果回调对象数组为空返回true;否则返回false
	*/
	bool empty(void)
	{
		return (0 == m_vt_receiver.size());
	};

	/*
	* 函数名称：cb_call::registercb
	* 函数描述：注册回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool registercb(T* p)
	{
		if(NULL == p)
			return false;

		for(int i=0; i<(int)m_vt_receiver.size(); ++i)
		{
			if(m_vt_receiver[i] == p)
				return true;
		}
		m_vt_receiver.push_back(p);

		return true;
	};

	/*
	* 函数名称：cb_call::unregistercb
	* 函数描述：注销回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool unregistercb(T* p)
	{
		if(NULL == p)
			return false;

		std::vector<T*>::iterator it = m_vt_receiver.begin();
		for(; it != m_vt_receiver.end(); ++it)
		{
			if((*it) != p)
				continue;

			m_vt_receiver.erase(it);

			return true;
		}
		return false;
	};

	/*
	* 函数名称：cb_call::is_rev_exisit
	* 函数描述：检查callback对象是否存在
	* 输入参数：pReceiver, 接收者的指针
	* 输出参数：void
	* 返回值  ：如果pReceiver已经存在于callback列表中，返回true;否则返回false
	*/
	bool is_rev_exisit(T* pReceiver)
	{
		int nSize=(int)m_vt_receiver.size();
		for(int i=0; i<nSize; ++i)
		{
			if(m_vt_receiver[i] == pReceiver)
				return true;
		}

		return false;
	};

	/*
	* 函数名称：cb_call::clear
	* 函数描述：清空回调对象
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：void
	*/
	void	clear(void)
	{
		m_vt_receiver.clear();
	};

	/*
	* 函数名称：cb_call::copy
	* 函数描述：复制回调对象
	* 输入参数：void
	* 输出参数：复制的目的
	* 返回值  ：void
	*/
	void	copy(std::vector<T*>& vt_copy)
	{
		int nSize=(int)m_vt_receiver.size();
		for(int i=0; i<nSize; ++i)
		{
			vt_copy.push_back(m_vt_receiver[i]);
		}
	}

	/*
	* 函数名称：cb_call::size
	* 函数描述：获取回调对象的数量
	* 输入参数：void
	* 输出参数：返回回调对象的数量
	* 返回值  ：void
	*/
	unsigned int size(void)
	{
		return m_vt_receiver.size();
	}
public:
	T*	operator[](int index)
	{
		return m_vt_receiver[index];
	};
protected:
	std::vector<T*> m_vt_receiver;
};

template<class T> class cb_caller
{
public:
	cb_caller(){};
	virtual ~cb_caller()
	{
		m_vt_receiver.clear();
	};
public:
	/*
	* 函数名称：cb_caller::empty
	* 函数描述：检查回调对象的数组是否为空
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：如果回调对象数组为空返回true;否则返回false
	*/
	bool empty(void)
	{
		return (0 == m_vt_receiver.size());
	};

	/*
	* 函数名称：cb_caller::registercb
	* 函数描述：注册回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool registercb(T* p)
	{
		if(NULL == p)
			return false;

		for(int i=0; i<(int)m_vt_receiver.size(); ++i)
		{
			if(m_vt_receiver[i] == p)
				return true;
		}

		m_vt_receiver.push_back(p);

		return true;
	};

	/*
	* 函数名称：cb_caller::unregistercb
	* 函数描述：注销回调对象
	* 输入参数：p，回调对象的指针
	* 输出参数：void
	* 返回值  ：成功返回true;否则返回false
	*/
	bool unregistercb(T* p)
	{
		if(NULL == p)
			return false;

		std::vector<T*>::iterator it = m_vt_receiver.begin();
		for(; it != m_vt_receiver.end(); ++it)
		{
			if((*it) != p)
				continue;

			m_vt_receiver.erase(it);

			return true;
		}
		return false;
	};

	/*
	* 函数名称：cb_caller::call
	* 函数描述：发出回调消息
	* 输入参数：dwCode, 回调的代码; wpraram, lparam, 参数
	* 输出参数：void
	* 返回值  ：返回一个整数
	* 注意	  ：返回的值要根据具体的对象才能决定其意义
	*/
	virtual int call(DWORD dwCode, WPARAM wParam, LPARAM lParam)
	{
		int nSize = (int)m_vt_receiver.size();
		for(int i=0; i<nSize; ++i)
		{
			if(NULL == m_vt_receiver[i])
				continue;
			int	nResult = m_vt_receiver[i]->on_call(dwCode, wParam, lParam);
			if(0 == nResult)
				continue;
			return nResult;
		};

		return 0;
	};

	/*
	* 函数名称：cb_caller::is_rev_exisit
	* 函数描述：检查callback对象是否存在
	* 输入参数：pReceiver, 接收者的指针
	* 输出参数：void
	* 返回值  ：如果pReceiver已经存在于callback列表中，返回true;否则返回false
	*/
	bool is_rev_exisit(T* pReceiver)
	{
		int nSize=(int)m_vt_receiver.size();
		for(int i=0; i<nSize; ++i)
		{
			if(m_vt_receiver[i] == pReceiver)
				return true;
		}

		return false;
	};

	/*
	* 函数名称：cb_caller::clear
	* 函数描述：清空回调对象
	* 输入参数：void
	* 输出参数：void
	* 返回值  ：void
	*/
	void	clear(void)
	{
		m_vt_receiver.clear();
	};

	/*
	* 函数名称：cb_caller::copy
	* 函数描述：复制回调对象
	* 输入参数：void
	* 输出参数：复制的目的
	* 返回值  ：void
	*/
	void	copy(std::vector<T*>& vt_copy)
	{
		int nSize=(int)m_vt_receiver.size();
		for(int i=0; i<nSize; ++i)
		{
			vt_copy.push_back(m_vt_receiver[i]);
		}
	}

	/*
	* 函数名称：cb_call::size
	* 函数描述：获取回调对象的数量
	* 输入参数：void
	* 输出参数：返回回调对象的数量
	* 返回值  ：void
	*/
	unsigned int size(void)
	{
		return m_vt_receiver.size();
	}
protected:
	std::vector<T*> m_vt_receiver;
};
