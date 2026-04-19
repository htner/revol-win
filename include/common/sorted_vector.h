

#ifndef SORTED_VECTOR_
#define SORTED_VECTOR_

#include <algorithm>
#include <vector>
#include <utility>
#include <functional>


namespace std{
		// TEMPLATE CLASS sorted_vector

template<class K, class Pr = std::less<K>, class A = std::allocator<K> >
class sorted_vector : public std::vector<K,A>{
public:
   typedef Pr	key_compare;	
   typedef K    value_type;
   typedef typename std::vector<K,A>::iterator iterator;
   typedef typename std::vector<K,A>::const_iterator const_iterator;

   
   void insert(const value_type& x)
	{       
		iterator p= lower_bound(x);
		if(p==end()|| key_compare_(x,*p)){
			std::vector<K,A>::insert(p,x);
		}         
	}

    void erase(const value_type& x)     
	{
		iterator p= lower_bound(x);
		if(p!=end() && (x == *p)){
			std::vector<K,A>::erase(p);
		}		
	}
    bool contain(const value_type& x)  const
    {
		const_iterator p= lower_bound(x);
		return p!=end() && (x == *p);
    }
     iterator end()
   	{
		return std::vector<K,A>::end();
	}
   iterator begin()
   	{
		return std::vector<K,A>::begin();
	}
	const_iterator end() const
   	{
		return std::vector<K,A>::end();
	}
   const_iterator begin() const
   	{
		return std::vector<K,A>::begin();
	}	
	iterator lower_bound(const K& k)
        {return std::lower_bound(begin(), end(), k, key_compare_); }
	const_iterator lower_bound(const K& k) const
        {return std::lower_bound(begin(), end(), k, key_compare_); }
	iterator upper_bound(const K& k)
		{return std::upper_bound(begin(), end(), k, key_compare_); }
	const_iterator upper_bound(const K& k) const
		{return std::upper_bound(begin(), end(), k, key_compare_); }

protected:
    key_compare         key_compare_;
};
}
#endif
