#pragma once
#include <xmemory>

//// TEMPLATE CLASS _Allocator_base_X<const _Ty>
//template<class _Ty>
//struct _Allocator_base_X<const _Ty>
//{	// base class for generic allocators for const _Ty
//	typedef _Ty value_type;
//};

template<class _Ty>
struct _Alloc_Base
{	// base class for generic allocators
	typedef _Ty value_type;
};

// TEMPLATE CLASS _Allocator_base<const _Ty>
template<class _Ty>
struct _Alloc_Base<const _Ty>
{	// base class for generic allocators for const _Ty
	typedef _Ty value_type;
};

typedef void* (*ALLOC_FUNC_PTR)(size_t);
typedef void (*FREE_FUNC_PTR)(void*);

#if _MSC_VER >= 1600 // vc2010
#ifndef _FARQ   /* specify standard memory model */
 #define _FARQ
 #define _PDFT  ptrdiff_t
 #define _SIZT  size_t
#endif
#endif

// TEMPLATE CLASS allocator
template<class _Ty,ALLOC_FUNC_PTR ALLOC_FUNC,FREE_FUNC_PTR FREE_FUNC>
class allocator_X
	: public _Alloc_Base<_Ty>
{	// generic allocator for objects of class _Ty
public:
	typedef _Alloc_Base<_Ty> _Mybase;
	typedef typename _Mybase::value_type value_type;
	typedef value_type _FARQ *pointer;
	typedef value_type _FARQ& reference;
	typedef const value_type _FARQ *const_pointer;
	typedef const value_type _FARQ& const_reference;

	typedef _SIZT size_type;
	typedef _PDFT difference_type;

	template<class _Other>
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef allocator_X<_Other,ALLOC_FUNC,FREE_FUNC> other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}

	allocator_X() _THROW0()
	{	// construct default allocator (do nothing)
	}

	allocator_X(const allocator_X<_Ty,ALLOC_FUNC,FREE_FUNC>&) _THROW0()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	allocator_X(const allocator_X<_Other,ALLOC_FUNC,FREE_FUNC>&) _THROW0()
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	allocator_X<_Ty,ALLOC_FUNC,FREE_FUNC>& operator=(const allocator_X<_Other,ALLOC_FUNC,FREE_FUNC>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
#if 1		
		FREE_FUNC(_Ptr);
#else
		::operator delete(_Ptr);
#endif
	}

	pointer allocate(size_type _Count)
	{	// allocate array of _Count elements

#if 1	
		return (pointer)(ALLOC_FUNC(_Count * sizeof(value_type)));
#else
		return ((pointer)::operator new(_Count * sizeof (value_type)));
		//return (_Allocate(_Count, (pointer)0));
#endif
		
	}

	pointer allocate(size_type _Count, const void _FARQ *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer _Ptr, const _Ty& _Val)
	{	// construct object at _Ptr with value _Val
		std::_Construct(_Ptr, _Val);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		std::_Destroy(_Ptr);
	}

	_SIZT max_size() const _THROW0()
	{	// estimate maximum array size
		_SIZT _Count = (_SIZT)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
	}
};