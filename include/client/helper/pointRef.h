#pragma once


template<typename POINTER>
class PointerRef
{
public:
	PointerRef()
	{
		_ref = 0;
		_filter = NULL;
	};
	~PointerRef()
	{

	};
public:
	void Attach(POINTER filter){
		_filter = filter;
	}
	void Detach(){
		_filter = NULL;
	}
	POINTER GetPointer()const{
		return _filter;
	}
	void AddRef(){
		_ref ++;
	};
	void Release()
	{
		_ref--;
		if(_ref == 0 )
		{
			delete this;
		}
	};
protected:
	int			_ref;
	POINTER		_filter;
};
