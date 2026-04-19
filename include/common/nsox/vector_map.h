#ifndef	__vector_map_h
#define __vector_map_h

#include <map>

#define INVALID_INDEX_VAL  (uint16_t)(~0)
#define VALID_INDEX(x) (x != INVALID_INDEX_VAL)


template<typename T>
class nsox_vector_map
{
public:
		struct xx_item {
				xx_item(){invalid();}
				uint16_t		__prev;		
				uint16_t		__next;
				uint8_t			__inuse;
				T				__value;
				inline bool in_use(){
						return __inuse;
				}
				inline void invalid(){
						__prev = __next = INVALID_INDEX_VAL;
						__inuse = false;
				}
		};

		typedef std::map<uint16_t,T>			    item_map;
		typedef typename item_map::iterator			 item_map_itr;

		nsox_vector_map(uint16_t vectorsize)
		{
				__array			= new xx_item[vectorsize];
				__first			= INVALID_INDEX_VAL;
				__vector_size  = vectorsize;
				
		}
		~nsox_vector_map(){
				delete[] __array;
		}
		void set(uint16_t fd, T& value){
				if(fd < __vector_size){
						xx_item& item = __array[fd];
						item.__value = value;
						if(!item.in_use()){
								if(VALID_INDEX(__first)) {
										item.__next = __first;
										__array[__first].__prev = fd;
								}
								__first = fd;
						}
						item.__inuse = true;
				}else{
						__map[fd] = value;
				}
		}
		T&  get(uint16_t fd){
				if(!exist(fd)){
						throw std::out_of_range("invalid fd");
				}
				if(fd < __vector_size){
						return __array[fd].__value;
				}else{
						return __map[fd];
				}
		}
		void remove(uint16_t fd){
				if(fd < __vector_size){
						xx_item& item = __array[fd];
						if(item.in_use()){
								if(VALID_INDEX(item.__prev)){
										__array[item.__prev].__next = item.__next;
								}else{
										__first = item.__next;
								}
								if(VALID_INDEX(item.__next)){
										__array[item.__next].__prev = item.__prev;
								}
						}
						item.invalid();
						
				}else{
						__map.erase(fd);
				}

		}

		bool exist(uint16_t fd){
				if(fd < __vector_size)
				{
						return __array[fd].in_use();
				}else
				{
						return __map.find(fd) != __map.end();
				}
		}
		bool empty(){
				return (!VALID_INDEX(__first)) && (__map.empty());
		}

		void first()
		{
				__cursor		= __first;
				__map_cur		= __map.begin();
		}
		T& next()
		{
				if(!VALID_INDEX(__cursor)){
						if(__map_cur == __map.end()){
								throw std::out_of_range("out of range");
						}
						T& value = __map_cur->second;
						__map_cur ++ ;
						return value;
				}else{
						xx_item& item =  __array[__cursor];
						__cursor = item.__next;
						return item.__value;
				}
		}
		bool end()
		{
				return (!VALID_INDEX(__cursor)) && (__map_cur==__map.end());
		}


protected:
		uint16_t				__vector_size;
		xx_item*				__array;
		item_map				__map;		
		item_map_itr			__map_cur;
		uint16_t				__first;
		uint16_t				__cursor;
};

#endif