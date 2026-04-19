#pragma once

#include "common/nsox/nuautoptr.h"

template<class T>
struct IConnPoint
{
	virtual void Watch(T* handler)  = 0;
	virtual void Revoke(T* handler)  = 0;
};

#include <set>



template <class T>
struct XTYPE
{
	typedef T type;
};


#ifndef BOOST_REF_HPP_INCLUDED

namespace boost
{
	template<class T> class reference_wrapper
	{ 
	public:
		typedef T type;
		explicit reference_wrapper(T& t): t_(&t) {}

		operator T& () const { return *t_; }

		T& get() const { return *t_; }

		T* get_pointer() const { return t_; }
	private:
		T* t_;
	};

	template<class T> inline reference_wrapper<T>  ref(T & t)
	{ 
		return reference_wrapper<T>(t);
	}

	template<class T> inline reference_wrapper<T const>  cref(T const & t)
	{
		return reference_wrapper<T const>(t);
	}
}
#endif


template<class T>
class XConnPoint : virtual public IConnPoint<T>
{
public:
	XConnPoint(void)
	{}
	virtual ~XConnPoint(void)
	{assert(handlers.empty());}
public:
	void Watch(T* handler){
			handlers.insert(handler);
	}
	void Revoke(T* handler){
			handlers.erase(handler);
	}
	bool Exist(T* handler){
			return handlers.find(handler) != handlers.end();
	}

	void forEachWatcher0(void (T::*fun)(void))
	{
		iterator itr = handlers.begin();
		for(; itr != handlers.end(); ++itr)
		{
			T* x = *itr;
			(x->*fun)();
		}
	}


	template<typename U> 
	void forEachWatcher1(void (T::*fun)(U), typename XTYPE<U>::type param)
	{
		iterator itr = handlers.begin();
		for(; itr != handlers.end(); ++itr)
		{
				T* x = *itr;
				(x->*fun)(param);
		}
	}

	template<typename U, typename V> 
		void forEachWatcher2(void (T::*fun)(U, V), typename XTYPE<U>::type param, typename XTYPE<V>::type  p2){

		iterator itr = handlers.begin();
		for(; itr != handlers.end(); ++itr)
		{
			T* x = *itr;
			(x->*fun)(param, p2);
		}
	}

	template<typename U, typename V, typename X> 
	void forEachWatcher3(void (T::*fun)(U, V, X), typename XTYPE<U>::type param,typename  XTYPE<V>::type p2,typename  XTYPE<X>::type p3){

		iterator itr = handlers.begin();
		for(; itr != handlers.end(); ++itr)
		{
			T* x = *itr;
			(x->*fun)(param, p2, p3);
		}
	}

	template<typename U, typename V, typename X, typename Y> 
		void forEachWatcher4(void (T::*fun)(U, V, X, Y), typename XTYPE<U>::type param, typename XTYPE<V>::type p2, typename XTYPE<X>::type p3, typename XTYPE<Y>::type p4){

			iterator itr = handlers.begin();
			for(; itr != handlers.end(); ++itr)
			{
				T* x = *itr;
				(x->*fun)(param, p2, p3, p4);
			}
		}

	template<typename U, typename V, typename X, typename Y, typename Z> 
		void forEachWatcher5(void (T::*fun)(U, V, X, Y, Z),typename  XTYPE<U>::type param, typename XTYPE<V>::type p2,typename  XTYPE<X>::type p3,typename  XTYPE<Y>::type p4, typename XTYPE<Z>::type p5){

			iterator itr = handlers.begin();
			for(; itr != handlers.end(); ++itr)
			{
				T* x = *itr;
				(x->*fun)(param, p2, p3, p4, p5);
			}
		}


		template<typename U, typename V, typename X, typename Y, typename Z, typename O> 
		void forEachWatcher6(void (T::*fun)(U, V, X, Y, Z, O),typename  XTYPE<U>::type param, typename XTYPE<V>::type p2,typename  XTYPE<X>::type p3,typename  XTYPE<Y>::type p4, typename XTYPE<Z>::type p5, typename XTYPE<O>::type p6){

			iterator itr = handlers.begin();
			for(; itr != handlers.end(); ++itr)
			{
				T* x = *itr;
				(x->*fun)(param, p2, p3, p4, p5, p6);
			}
		}




protected:
	typedef nsox::nu_auto_ptr<T> type_ptr_t;
	typedef typename std::set<type_ptr_t>::iterator iterator;
	std::set<type_ptr_t> handlers;
};
