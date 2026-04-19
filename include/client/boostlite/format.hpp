
#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <limits>
#include <xlocale>
#include "atl_streambuf.hpp"


namespace boost {

#if ! defined(BOOST_NO_STD_LOCALE)
	typedef ::std::locale locale_t;
#else 
	typedef int          locale_t;
#endif

//------------------- stream_format_state declaration------------------------//

	//   set of params that define the format state of a stream
	template<class Ch, class Tr> 
	struct stream_format_state 
	{
		typedef ::std::basic_ios<Ch, Tr>   basic_ios;

		stream_format_state(Ch fill)                 { reset(fill); }
		//        stream_format_state(const basic_ios& os)     { set_by_stream(os); }

		void reset(Ch fill);			              //- sets to default state.
		void set_by_stream(const basic_ios& os);    //-- //- sets to os's state.
		void apply_on(basic_ios & os) const;		    //- applies format_state to the stream
			
		template<class T> 
		void apply_manip(T manipulator)          //- modifies state by applying manipulator
		{ apply_manip_body<Ch, Tr, T>( *this, manipulator) ; }

		// --- data ---
		std::streamsize width_;
		std::streamsize precision_;
		Ch fill_; 
		std::ios_base::fmtflags flags_;
		std::ios_base::iostate  rdstate_;
//		std::ios_base::iostate  exceptions_;
//		boost::optional<boost::io::detail::locale_t>  loc_;
	}; 

//------------------- End stream_format_state declaration------------------------//


//------------------- stream_format_state implement-----------------------------//

	template<class Ch, class Tr> inline
		void stream_format_state<Ch,Tr>:: reset(Ch fill) {
			// set our params to standard's default state.   cf 27.4.4.1 of the C++ norm
			width_=0; precision_=6; 
			fill_=fill; // default is widen(' '), but we cant compute it without the locale
			flags_ = std::ios_base::dec | std::ios_base::skipws; 
			// the adjust_field part is left equal to 0, which means right.
//			exceptions_ = std::ios_base::goodbit;
			rdstate_ = std::ios_base::goodbit;
	}

	template<class Ch, class Tr>
	void stream_format_state<Ch,Tr>:: set_by_stream(const basic_ios& os) {
		// set our params according to the state of this stream
		flags_ = os.flags();
		width_ = os.width();
		precision_ = os.precision();
		fill_ = os.fill();
		rdstate_ = os.rdstate();
		//	exceptions_ = os.exceptions();
	}

	template<class Ch, class Tr>
	void stream_format_state<Ch,Tr>:: apply_on (basic_ios & os) const {
			// set the state of this stream according to our params
			if(width_ != -1)
				os.width(width_);
			if(precision_ != -1)
				os.precision(precision_);
			if(fill_ != 0)
				os.fill(fill_);
			os.flags(flags_);
			os.clear(rdstate_);
	}
	
	template<class Ch, class Tr, class T>
	void apply_manip_body( stream_format_state<Ch, Tr>& self, T manipulator) 
	{
			// modify our params according to the manipulator
		io::basic_oaltstringstream<Ch, Tr>  ss;
		self.apply_on( ss );
		ss << manipulator;
		self.set_by_stream( ss );
	}

//------------------------ End stream_format_state implement ----------------------//




//------------------------ format_item declaration -------------------------------//

	//   stores all parameters that can be specified in format strings
	template<class Ch, class Tr, class Alloc>  
	struct format_item 
	{     
		enum pad_values { zeropad = 1, spacepad =2, centered=4, tabulation = 8 };
		// 1. if zeropad is set, all other bits are not, 
		// 2. if tabulation is set, all others are not.
		// centered and spacepad can be mixed freely.
		enum arg_values { argN_no_posit   = -1, // non-positional directive. will set argN later
			argN_tabulation = -2, // tabulation directive. (no argument read) 
			argN_ignored    = -3  // ignored directive. (no argument read)
		};
		typedef ::std::basic_ios<Ch, Tr>				  basic_ios;
		typedef stream_format_state<Ch, Tr>               stream_format_state;
		typedef ::std::basic_string<Ch, Tr, Alloc>        string_type;

		format_item(Ch fill) :argN_(argN_no_posit), fmtstate_(fill), 
			truncate_(max_streamsize()), pad_scheme_(0)  {}
		void reset(Ch fill);
		void compute_states();   // sets states  according to truncate and pad_scheme.

		static std::streamsize max_streamsize() { 
			return (::std::numeric_limits<std::streamsize>::max)();
		}

		// --- data ---
		int         argN_;  //- argument number (starts at 0,  eg : %1 => argN=0)
		//  negative values for items that don't process an argument
		string_type  res_;      //- result of the formatting of this item
		string_type  appendix_; //- piece of string between this item and the next

		stream_format_state fmtstate_;// set by parsing, is only affected by modify_item

		std::streamsize truncate_;//- is set for directives like %.5s that ask truncation
		unsigned int pad_scheme_;//- several possible padding schemes can mix. see pad_values
	}; 

//------------------------ End format_item declaration-------------------------------//

//------------------------ format_item implement------------------------------------//

	template<class Ch, class Tr, class Alloc> 
	void format_item<Ch, Tr, Alloc>:: 
		reset (Ch fill) { 
			argN_=argN_no_posit; truncate_ = max_streamsize(); pad_scheme_ =0; 
			res_.resize(0); appendix_.resize(0);
			fmtstate_.reset(fill);
	}

	template<class Ch, class Tr, class Alloc> 
	void format_item<Ch, Tr, Alloc>:: 
		compute_states() {
			// reflect pad_scheme_   on  fmt_state_
			//   because some pad_schemes has complex consequences on several state params.
			if(pad_scheme_ & zeropad) {
				// ignore zeropad in left alignment :
				if(fmtstate_.flags_ & std::ios_base::left) {
					// only left bit might be set. (not right, nor internal)
					pad_scheme_ = pad_scheme_ & (~zeropad); 
				}
				else { 
					pad_scheme_ &= ~spacepad; // printf ignores spacepad when zeropadding
					fmtstate_.fill_='0'; 
					fmtstate_.flags_ = (fmtstate_.flags_ & ~std::ios_base::adjustfield) 
						| std::ios_base::internal;
					// removes all adjustfield bits, and adds internal.
				}
			}
			if(pad_scheme_ & spacepad) {
				if(fmtstate_.flags_ & std::ios_base::showpos)
					pad_scheme_ &= ~spacepad;
			}
	}

//------------------------ End format_item implement-------------------------------//



//------------------------ basic_format declaration-------------------------------//

    template<class Ch, class Tr, class Alloc>
    class basic_format 
    {
    public:
        typedef Ch  CharT;   // borland fails in operator% if we use Ch and Tr directly
        typedef std::basic_string<Ch, Tr, Alloc>              string_type;
        typedef typename string_type::size_type               size_type;
		typedef format_item<Ch, Tr, Alloc>					  format_item_t;
		typedef io::basic_altstringbuf<Ch, Tr, Alloc>         internal_streambuf_t;

        
        explicit basic_format(const Ch* str=NULL);
		explicit basic_format(const string_type& s);
		basic_format(const basic_format& x);
		basic_format& operator= (const basic_format& x);
		void swap(basic_format& x);

		basic_format& clear();				       // empty all converted string buffers (except bound items)
		basic_format& clear_binds(); // unbind all bound items, and call clear()
		basic_format& parse(const string_type&); // resets buffers and parse a new format string

        size_type   size() const;//--    // sum of the current string pieces sizes
		string_type str()  const;    // final string 


		template<class Ch2, class Tr2, class Alloc2>
		friend std::ostream & 
			operator<<( std::ostream & ,
			const basic_format<Ch2, Tr2, Alloc2>& );
		template<class Ch2, class Tr2, class Alloc2, class T>  
		friend basic_format<Ch2, Tr2, Alloc2>&  
			feed (basic_format<Ch2, Tr2, Alloc2>&, T);
		template<class Ch2, class Tr2, class Alloc2, class T>
		friend void distribute(basic_format<Ch2, Tr2, Alloc2>&, T);
		template<class Ch2, class Tr2, class Alloc2, class T>  friend
			basic_format<Ch2, Tr2, Alloc2>& 
			modify_item_body (basic_format<Ch2, Tr2, Alloc2>&, int, T);

		template<class Ch2, class Tr2, class Alloc2, class T> friend
			basic_format<Ch2, Tr2, Alloc2>&  
			bind_arg_body (basic_format<Ch2, Tr2, Alloc2>&, int, const T&);

        // ** arguments passing ** //
        template<class T>  
        basic_format&   operator%(T& x)
		{ return feed<CharT, Tr, Alloc, T&>(*this,x); }
		template<class T>
		basic_format& operator%(const T& x)
		{ return feed<CharT, Tr, Alloc, const T&>(*this, x); }

		// The total number of arguments expected to be passed to the format objectt
		int expected_args() const
		{ return num_args_; }
		// The number of arguments currently bound (see bind_arg(..) )
		int bound_args() const;
		// The number of arguments currently fed to the format object
		int fed_args() const;
		// The index (1-based) of the current argument (i.e. next to be formatted)
		int cur_arg() const;
		// The number of arguments still required to be fed
		int remaining_args() const; // same as expected_args() - bound_args() - fed_args()
		
		void make_or_reuse_data(std::size_t nbitems);// used for (re-)initialisation

		// ** object modifying **//
		template<class T>
		basic_format&  bind_arg(int argN, const T& val) 
		{ return bind_arg_body(*this, argN, val); }
		basic_format&  clear_bind(int argN);
		template<class T> 
		basic_format&  modify_item(int itemN, T manipulator) 
		{ return modify_item_body<Ch,Tr, Alloc, T> (*this, itemN, manipulator);}

		enum style_values  { ordered = 1, // set only if all directives are  positional
			special_needs = 4 }; 

	private:

		std::vector<format_item_t>  items_; // each '%..' directive leads to a format_item
		std::vector<bool>			bound_; // stores which arguments were bound. size() == 0 || num_args

		int							style_; // style of format-string :  positional or not, etc
		int							cur_arg_; // keep track of wich argument is current
		int							num_args_; // number of expected arguments
		mutable bool				dumped_; // true only after call to str() or <<
		string_type					prefix_; // piece of string to insert before first item
		internal_streambuf_t		buf_; // the internal stream buffer.

    }; // class basic_format

//------------------------ End basic_format declaration-------------------------------//


//------------------------ basic_format implement------------------------------------//

	template< class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>:: basic_format(const Ch* s)
		: style_(0), cur_arg_(0), num_args_(0), dumped_(false)
	{
		if( s )
			parse( s );
	}

	template<class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>::basic_format(const string_type& s)
		: style_(0), cur_arg_(0), num_args_(0), dumped_(false)
	{
		parse(s);
	}

	template<class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>::basic_format(const basic_format& x)
		: items_(x.items_), bound_(x.bound_), style_(x.style_), cur_arg_(x.cur_arg_), 
		num_args_(x.num_args_), dumped_(x.dumped_), prefix_(x.prefix_)
	{		
	}

	template<class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>& basic_format<Ch, Tr, Alloc>::
		operator=(const basic_format& x)
	{
		if ( this == &x)
		{
			return *this;
		}

		// 调用复制构造函数，构造出临时对象与*this对象交换
		(basic_format<Ch, Tr, Alloc>(x)).swap(*this);
		return *this;

	}

	template<class Ch, class Tr, class Alloc>
	void basic_format<Ch, Tr, Alloc>::swap(basic_format& x)
	{
		::std::swap(style_, x.style_);
		::std::swap(cur_arg_, x.cur_arg_);
		::std::swap(num_args_, x.num_args_);
		::std::swap(dumped_, x.dumped_);

		items_.swap(x.items_);
		prefix_.swap(x.prefix_);
		bound_.swap(x.bound_);
	}

	template<class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>& basic_format<Ch, Tr, Alloc>::
		clear()	{

			for (unsigned long i = 0; i < items_.size(); ++i)
			{
				if ( bound_.size() == 0 || items_[i].argN_ < 0 || !bound_[items_[i].argN_])
				{
					items_[i].res_.resize(0);
				}
			}
			cur_arg_ = 0;
			dumped_ = false;

			// maybe first arg is bound:
			if(bound_.size() != 0) {
				for(; cur_arg_ < num_args_ && bound_[cur_arg_]; ++cur_arg_)
				{}
			}
			return *this;		
	}

	template< class Ch, class Tr, class Alloc>
	basic_format<Ch,Tr, Alloc>& basic_format<Ch,Tr, Alloc>:: 
		clear_binds () {
			// remove all binds, then clear()
			bound_.resize(0);
			clear();
			return *this;
	}

	template< class Ch, class Tr, class Alloc>
	basic_format<Ch,Tr, Alloc>& basic_format<Ch,Tr, Alloc>:: 
		clear_bind (int argN) {
			// remove the bind of ONE argument then clear()
			if(argN<1 || argN > num_args_ || bound_.size()==0 || !bound_[argN-1] ) {
				return *this;
			}
			bound_[argN-1]=false;
			clear();
			return *this;
	}

	template< class Ch, class Tr, class Alloc>
	int basic_format<Ch,Tr, Alloc>::
		bound_args() const {
			if(bound_.size()==0)
				return 0;
			int n=0;
			for(int i=0; i<num_args_ ; ++i)
				if(bound_[i])
					++n;
			return n;
	}

	template< class Ch, class Tr, class Alloc>
	int basic_format<Ch,Tr, Alloc>::
		fed_args() const {
			if(bound_.size()==0)
				return cur_arg_;
			int n=0;
			for(int i=0; i<cur_arg_ ; ++i)
				if(!bound_[i])
					++n;
			return n;
	}

	template< class Ch, class Tr, class Alloc>
	int basic_format<Ch,Tr, Alloc>::
		cur_arg() const {
			return cur_arg_+1; }

	template< class Ch, class Tr, class Alloc>
	int basic_format<Ch,Tr, Alloc>::
		remaining_args() const {
			if(bound_.size()==0)
				return num_args_-cur_arg_;
			int n=0;
			for(int i=cur_arg_; i<num_args_ ; ++i)
				if(!bound_[i])
					++n;
			return n;
	}


	template<class Ch, class Tr, class Alloc, class T> 
	basic_format<Ch, Tr, Alloc>&  
		bind_arg_body (basic_format<Ch, Tr, Alloc>& self, int argN, const T& val) {
			// bind one argument to a fixed value
			// this is persistent over clear() calls, thus also over str() and <<
			if(self.dumped_) 
				self.clear(); // needed because we will modify cur_arg_
			if(argN<1 || argN > self.num_args_) {
				return self;
			}
			if(self.bound_.size()==0) 
				self.bound_.assign(self.num_args_,false);

			int o_cur_arg = self.cur_arg_;
			self.cur_arg_ = argN-1; // arrays begin at 0

			self.bound_[self.cur_arg_]=false; // if already set, we unset and re-sets..
			self.operator%(val); // put val at the right place, because cur_arg is set


			// Now re-position cur_arg before leaving :
			self.cur_arg_ = o_cur_arg; 
			self.bound_[argN-1]=true;
			if(self.cur_arg_ == argN-1 ) {
				// hum, now this arg is bound, so move to next free arg
				while(self.cur_arg_ < self.num_args_ && self.bound_[self.cur_arg_])   
					++self.cur_arg_;
			}
			// In any case, we either have all args, or are on an unbound arg :
			return self;
	}

	template<class Ch, class Tr, class Alloc, class T> basic_format<Ch, Tr, Alloc>&
		modify_item_body (basic_format<Ch, Tr, Alloc>& self, int itemN, T manipulator) {
			// applies a manipulator to the format_item describing a given directive.
			// this is a permanent change, clear or reset won't cancel that.
			if(itemN<1 || itemN > static_cast<signed int>(self.items_.size() )) {
				return self;
			}
			self.items_[itemN-1].fmtstate_. template apply_manip<T> ( manipulator );
			return self;
	}

	template<class Ch, class Tr, class Alloc>
	basic_format<Ch, Tr, Alloc>& basic_format<Ch, Tr, Alloc>:: 
		parse (const string_type& buf) {
			// parse the format-string 
			using namespace std;

//			io::basic_oaltstringstream<Ch, Tr, Alloc> fac;

			const Ch arg_mark = '%';
			bool ordered_args=true; 
			int max_argN=-1;

			// A: find upper_bound on num_items and allocates arrays
			int num_items = upper_bound_from_fstring(buf, arg_mark);
			make_or_reuse_data(num_items);

			// B: Now the real parsing of the format string :
			num_items=0;
			typename string_type::size_type i0=0, i1=0;
			typename string_type::const_iterator it;
			bool special_things=false;
			int cur_item=0;
			while( (i1=buf.find(arg_mark,i1)) != string_type::npos ) {
				string_type & piece = (cur_item==0) ? prefix_ : items_[cur_item-1].appendix_;
				if( buf[i1+1] == buf[i1] ) { // escaped mark, '%%' 
					append_string(piece, buf, i0, i1+1);
					i1+=2; i0=i1;
					continue; 
				}

				if(i1!=i0) {
					append_string(piece, buf, i0, i1);
					i0=i1;
				}
				++i1;
				it = buf.begin()+i1;
				bool parse_ok = parse_printf_directive(it, buf.end(), &items_[cur_item], i1);
				i1 = it - buf.begin();
				if( ! parse_ok ) // the directive will be printed verbatim
					continue; 
				i0=i1;
				items_[cur_item].compute_states(); // process complex options, like zeropad, into params

				int argN=items_[cur_item].argN_;
				if(argN == format_item_t::argN_ignored)
					continue;
				if(argN ==format_item_t::argN_no_posit)
					ordered_args=false;
				else if(argN == format_item_t::argN_tabulation) special_things=true;
				else if(argN > max_argN) max_argN = argN;
				++num_items;
				++cur_item;
			} // loop on %'s
			//	BOOST_ASSERT(cur_item == num_items);
			if (cur_item > num_items) return *this;

			// store the final piece of string
			{
				string_type & piece = (cur_item==0) ? prefix_ : items_[cur_item-1].appendix_;
				append_string(piece, buf, i0, buf.size());
			}

			if (!ordered_args)
			{
				int non_ordered_items = 0;
				for (int i = 0; i < num_items; ++i)
				{
					if (items_[i].argN_ == format_item_t::argN_no_posit)
					{
						items_[i].argN_ = non_ordered_items;
						++non_ordered_items;
					}
				}
				max_argN = non_ordered_items - 1;
				
			}
			

			// C: set some member data :
			items_.resize(num_items, format_item_t(' ') );

			if(special_things) style_ |= special_needs;
			num_args_ = max_argN + 1;
			if(ordered_args) style_ |=  ordered;
			else style_ &= ~ordered;
			return *this;
	}

	template< class Ch, class Tr, class Alloc>
	typename basic_format<Ch, Tr, Alloc>::string_type 
		basic_format<Ch,Tr, Alloc>::str () const {
			if(items_.size()==0)
				return prefix_;

			unsigned long i;

			string_type res;
			res.reserve(size());
			res += prefix_;
			for(i=0; i < items_.size(); ++i) {
				const format_item_t& item = items_[i];
				res += item.res_;
				if( item.argN_ == format_item_t::argN_tabulation) { 
					if( static_cast<size_type>(item.fmtstate_.width_) > res.size() )
						res.append( static_cast<size_type>(item.fmtstate_.width_) - res.size(),
						item.fmtstate_.fill_ );
				}
				res += item.appendix_;
			}
			dumped_=true;
			return res;
	}

	template< class Ch, class Tr, class Alloc>
	typename std::basic_string<Ch, Tr, Alloc>::size_type 
		basic_format<Ch,Tr, Alloc>::size () const {
#ifdef BOOST_MSVC
			// If std::min<unsigned> or std::max<unsigned> are already instantiated
			// at this point then we get a blizzard of warning messages when we call
			// those templates with std::size_t as arguments.  Weird and very annoyning...
#pragma warning(push)
#pragma warning(disable:4267)
#endif
			size_type sz = prefix_.size();
			unsigned long i;
			for(i=0; i < items_.size(); ++i) {
				const format_item_t& item = items_[i];
				sz += item.res_.size();
				if( item.argN_ == format_item_t::argN_tabulation)
					sz = max  (sz,
					static_cast<size_type>(item.fmtstate_.width_) );
				sz += item.appendix_.size();
			}
			return sz;
#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
	}

	template<class Ch, class Tr, class Alloc>
	void basic_format<Ch, Tr, Alloc>::make_or_reuse_data (std::size_t nbitems) {
		Ch fill = ' ';
		if(items_.size() == 0)
			items_.assign( nbitems, format_item_t(fill) );
		else {
			if(nbitems>items_.size())
				items_.resize(nbitems, format_item_t(fill));
			bound_.resize(0);
			for(std::size_t i=0; i < nbitems; ++i)
				items_[i].reset(fill); //  strings are resized, instead of reallocated
		}
		prefix_.resize(0);
	}

//------------------------ End basic_format implement ------------------------------//



//------------------------ Other function implement-------------------------------//

	template<class Ch, class Tr, class Alloc>
	void mk_str( std::basic_string<Ch,Tr, Alloc> & res, 
		const Ch * beg,
		typename std::basic_string<Ch,Tr,Alloc>::size_type size,
		std::streamsize w, 
		const Ch fill_char,
		std::ios_base::fmtflags f, 
		const Ch prefix_space, // 0 if no space-padding
		bool center) 
		// applies centered/left/right  padding  to the string  [beg, beg+size[
		// Effects : the result is placed in res.
	{
		typedef typename std::basic_string<Ch,Tr,Alloc>::size_type size_type;
		res.resize(0);
		if(w<=0 || static_cast<size_type>(w) <=size) {
			// no need to pad.
			res.reserve(size + !!prefix_space);
			if(prefix_space) 
				res.append(1, prefix_space);
			if (size)
				res.append(beg, size);
		}
		else { 
			std::streamsize n=static_cast<std::streamsize>(w-size-!!prefix_space);
			std::streamsize n_after = 0, n_before = 0; 
			res.reserve(static_cast<size_type>(w)); // allocate once for the 2 inserts
			if(center) 
				n_after = n/2, n_before = n - n_after; 
			else 
				if(f & std::ios_base::left)
					n_after = n;
				else
					n_before = n;
			// now make the res string :
			if(n_before) res.append(static_cast<size_type>(n_before), fill_char);
			if(prefix_space) 
				res.append(1, prefix_space);
			if (size)  
				res.append(beg, size);
			if(n_after) res.append(static_cast<size_type>(n_after), fill_char);
		}
	} // -mk_str(..) 

	
	template< class Ch, class Tr, class T> inline
		void put_head (::std:: basic_ostream<Ch, Tr> &, const T& ) {
	}

	template< class Ch, class Tr, class T> inline
		void put_last( ::std:: basic_ostream<Ch, Tr> & os, const T& x ) {
			os << x ;
	}

	template< class Ch, class Tr, class Alloc, class T> 
	void put(T x, 
		const format_item<Ch, Tr, Alloc>& specs, 
		typename basic_format<Ch, Tr, Alloc>::string_type& res, 
		typename basic_format<Ch, Tr, Alloc>::internal_streambuf_t & buf)
	{
#ifdef BOOST_MSVC
		// If std::min<unsigned> or std::max<unsigned> are already instantiated
		// at this point then we get a blizzard of warning messages when we call
		// those templates with std::size_t as arguments.  Weird and very annoyning...
#pragma warning(push)
#pragma warning(disable:4267)
#endif


		typedef typename basic_format<Ch, Tr, Alloc>::string_type   string_type;
		typedef typename basic_format<Ch, Tr, Alloc>::format_item_t format_item_t;
		typedef typename string_type::size_type size_type;

		boost::io::basic_oaltstringstream<Ch, Tr, Alloc>  oss( &buf );
		specs.fmtstate_.apply_on(oss);

		put_head(oss, x);

		const std::ios_base::fmtflags fl=oss.flags();
		const bool internal = (fl & std::ios_base::internal) != 0;
		const std::streamsize w = oss.width();
		const bool two_stepped_padding= internal && (w!=0);

		res.resize(0);
		if(! two_stepped_padding) {
			if(w>0) // handle padding via mk_str, not natively in stream 
				oss.width(0);
			put_last( oss, x);
			const Ch * res_beg = buf.pbase();
			Ch prefix_space = 0;
			if(specs.pad_scheme_ & format_item_t::spacepad)
				if(buf.pcount()== 0 || 
					(res_beg[0] !=oss.widen('+') && res_beg[0] !=oss.widen('-')  ))
					prefix_space = oss.widen(' ');
			size_type res_size = (std::min)(
				static_cast<size_type>(specs.truncate_ - !!prefix_space), 
				buf.pcount() );
			mk_str(res, res_beg, res_size, w, oss.fill(), fl, 
				prefix_space, (specs.pad_scheme_ & format_item_t::centered) !=0 );
		}
		else  { // 2-stepped padding
			// internal can be implied by zeropad, or user-set.
			// left, right, and centered alignment overrule internal,
			// but spacepad or truncate might be mixed with internal (using manipulator)
			put_last( oss, x); // may pad
			const Ch * res_beg = buf.pbase();
			size_type res_size = buf.pcount();
			bool prefix_space=false;
			if(specs.pad_scheme_ & format_item_t::spacepad)
				if(buf.pcount()== 0 || 
					(res_beg[0] !=oss.widen('+') && res_beg[0] !=oss.widen('-')  ))
					prefix_space = true;
			if(res_size == static_cast<size_type>(w) && w<=specs.truncate_ && !prefix_space) {
				// okay, only one thing was printed and padded, so res is fine
				res.assign(res_beg, res_size);
			}
			else { //   length w exceeded
				// either it was multi-output with first output padding up all width..
				// either it was one big arg and we are fine.
				// Note that res_size<w is possible  (in case of bad user-defined formatting)
				res.assign(res_beg, res_size);
				res_beg=NULL;  // invalidate pointers.

				// make a new stream, to start re-formatting from scratch :
				buf.clear_buffer();
				boost::io::basic_oaltstringstream<Ch, Tr, Alloc>  oss2( &buf);
				specs.fmtstate_.apply_on(oss2);
				put_head( oss2, x );

				oss2.width(0);
				if(prefix_space)
					oss2 << ' ';
				put_last(oss2, x );
				if(buf.pcount()==0 && specs.pad_scheme_ & format_item_t::spacepad) {
					prefix_space =true;
					oss2 << ' ';
				}
				// we now have the minimal-length output
				const Ch * tmp_beg = buf.pbase();
				size_type tmp_size = (std::min)(static_cast<size_type>(specs.truncate_),
					buf.pcount() );


				if(static_cast<size_type>(w) <= tmp_size) { 
					// minimal length is already >= w, so no padding (cool!)
					res.assign(tmp_beg, tmp_size);
				}
				else { // hum..  we need to pad (multi_output, or spacepad present)
					//find where we should pad
					size_type sz = (std::min)(res_size + (prefix_space ? 1 : 0), tmp_size);
					size_type i = prefix_space;
					for(; i<sz && tmp_beg[i] == res[i - (prefix_space ? 1 : 0)]; ++i) {}
					if(i>=tmp_size) i=prefix_space;
					res.assign(tmp_beg, i);
					std::streamsize d = w - static_cast<std::streamsize>(tmp_size);
			//		BOOST_ASSERT(d>0);
					res.append(static_cast<size_type>( d ), oss2.fill());
					res.append(tmp_beg+i, tmp_size-i);
			//		BOOST_ASSERT(i+(tmp_size-i)+(std::max)(d,(std::streamsize)0) 
			//			== static_cast<size_type>(w));
			//		BOOST_ASSERT(res.size() == static_cast<size_type>(w));
				}
			}
		}
		buf.clear_buffer();
#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

	} // end- put(..)


	template< class Ch, class Tr, class Alloc, class T> 
	void distribute (basic_format<Ch,Tr, Alloc>& self, T x) {
		// call put(x, ..) on every occurence of the current argument :
		if(self.cur_arg_ >= self.num_args_)  {
			return;
		}
		for(unsigned long i=0; i < self.items_.size(); ++i) {
			if(self.items_[i].argN_ == self.cur_arg_) {
				put<Ch, Tr, Alloc, T> (x, self.items_[i], self.items_[i].res_, self.buf_);
			}
		}
	}

	template<class Ch, class Tr, class Alloc, class T> 
	basic_format<Ch, Tr, Alloc>&  
		feed (basic_format<Ch,Tr, Alloc>& self, T x) {
			if(self.dumped_) self.clear();
			distribute<Ch, Tr, Alloc, T> (self, x);
			++self.cur_arg_;
			if(self.bound_.size() != 0) {
				while( self.cur_arg_ < self.num_args_ && self.bound_[self.cur_arg_] )
					++self.cur_arg_;
			}
			return self;
	}


//------------------------------------------------------------------------------

	// Input : [start, last) iterators range and a
	//          a Facet to use its widen/narrow member function
	// Effects : read sequence and convert digits into integral n, of type Res
	// Returns : n
	template<class Res, class Iter>
	Iter str2int (const Iter & start, const Iter & last, Res & res) 
	{
		using namespace std;
		Iter it;
		res=0;
		for(it=start; it != last && isdigit(*it); ++it ) {
			char cur_ch = *it; // cant fail.
			res *= 10;
			res += cur_ch - '0'; // 22.2.1.1.2.13 of the C++ standard
		}
		return it;
	}

	// skip printf's "asterisk-fields" directives in the format-string buf
	// Input : char string, with starting index *pos_p
	//         a Facet merely to use its widen/narrow member function
	// Effects : advance *pos_p by skipping printf's asterisk fields.
	// Returns : nothing
	template<class Iter>
	Iter skip_asterisk(Iter start, Iter last) 
	{
		using namespace std;
		++ start;
		start = wrap_scan_notdigit(start, last);
		if(start!=last && *start== '$' )
			++start;
		return start;
	}


	// Input: the position of a printf-directive in the format-string
	//    a basic_ios& merely to use its widen/narrow member function
	//    a bitset'exceptions' telling whether to throw exceptions on errors.
	// Returns:
	//  true if parse succeeded (ignore some errors if exceptions disabled)
	//  false if it failed so bad that the directive should be printed verbatim
	// Effects:
	//  start is incremented so that *start is the first char after
	//     this directive
	//  *fpar is set with the parameters read in the directive
	template<class Ch, class Tr, class Alloc, class Iter>
	bool parse_printf_directive(Iter & start, const Iter& last, 
		format_item<Ch, Tr, Alloc> * fpar, std::size_t offset)
	{
		typedef typename basic_format<Ch, Tr, Alloc>::format_item_t format_item_t;

		fpar->argN_ = format_item_t::argN_no_posit;  // if no positional-directive
		bool precision_set = false;
		bool in_brackets=false;
		Iter start0 = start;
		std::size_t fstring_size = last-start0+offset;

		if(start>= last) { // empty directive : this is a trailing %
			return false;
		}          

		if(*start== '|') {
			in_brackets=true;
			if( ++start >= last ) {
				return false;
			}
		}

		// the flag '0' would be picked as a digit for argument order, but here it's a flag :
		if(*start== '0') 
			goto parse_flags;

		// handle argument order (%2$d)  or possibly width specification: %2d
		if(isdigit(*start)) {
			int n;
			start = str2int(start, last, n);
			if( start >= last ) {
				return false;
			}

			// %N% case : this is already the end of the directive
			if( *start ==  '%') {
				fpar->argN_ = n-1;
				++start;
				if( !in_brackets)
					return true;
				else if (start >= last)
					return false;
			}

			if ( *start== '$' ) {
				fpar->argN_ = n-1;
				++start;
			} 
			else {
				// non-positionnal directive
				fpar->fmtstate_.width_ = n;
				fpar->argN_  = format_item_t::argN_no_posit;
				goto parse_precision;
			}
		}

parse_flags: 
		// handle flags
		while ( start != last) { // as long as char is one of + - = _ # 0 l h   or ' '
			// misc switches
			switch ( *start) {
	case '\'' : break; // no effect yet. (painful to implement)
	case 'l':
	case 'h':  // short/long modifier : for printf-comaptibility (no action needed)
		break;
	case '-':
		fpar->fmtstate_.flags_ |= std::ios_base::left;
		break;
	case '=':
		fpar->pad_scheme_ |= format_item_t::centered;
		break;
	case '_':
		fpar->fmtstate_.flags_ |= std::ios_base::internal;
		break;
	case ' ':
		fpar->pad_scheme_ |= format_item_t::spacepad;
		break;
	case '+':
		fpar->fmtstate_.flags_ |= std::ios_base::showpos;
		break;
	case '0':
		fpar->pad_scheme_ |= format_item_t::zeropad;
		// need to know alignment before really setting flags,
		// so just add 'zeropad' flag for now, it will be processed later.
		break;
	case '#':
		fpar->fmtstate_.flags_ |= std::ios_base::showpoint | std::ios_base::showbase;
		break;
	default:
		goto parse_width;
			}
			++start;
		} // loop on flag.

		if( start>=last) {
			return true; 
		}
parse_width:
		// handle width spec
		// first skip 'asterisk fields' :  *, or *N$
		if(*start == '*' )
			start = skip_asterisk(start, last); 
		if(start!=last && isdigit( *start))
			start = str2int(start, last, fpar->fmtstate_.width_);

parse_precision:
		if( start>= last) { 
			return true;
		}
		// handle precision spec
		if (*start== '.') {
			++start;
			if(start != last && *start == '*' )
				start = skip_asterisk(start, last); 
			if(start != last && isdigit(*start)) {
				start = str2int(start, last, fpar->fmtstate_.precision_);
				precision_set = true;
			}
			else
				fpar->fmtstate_.precision_ =0;
		}

		// handle  formatting-type flags :
		while( start != last &&  *start== 'l' || *start== 'L' || *start== 'h' )
			++start;
		if( start>=last) {
			return true;
		}

		if( in_brackets && *start== '|' ) {
			++start;
			return true;
		}
		switch ( *start ) {
	case 'X':
		fpar->fmtstate_.flags_ |= std::ios_base::uppercase;
	case 'p': // pointer => set hex.
	case 'x':
		fpar->fmtstate_.flags_ &= ~std::ios_base::basefield;
		fpar->fmtstate_.flags_ |= std::ios_base::hex;
		break;

	case 'o':
		fpar->fmtstate_.flags_ &= ~std::ios_base::basefield;
		fpar->fmtstate_.flags_ |=  std::ios_base::oct;
		break;

	case 'E':
		fpar->fmtstate_.flags_ |=  std::ios_base::uppercase;
	case 'e':
		fpar->fmtstate_.flags_ &= ~std::ios_base::floatfield;
		fpar->fmtstate_.flags_ |=  std::ios_base::scientific;

		fpar->fmtstate_.flags_ &= ~std::ios_base::basefield;
		fpar->fmtstate_.flags_ |=  std::ios_base::dec;
		break;

	case 'f':
		fpar->fmtstate_.flags_ &= ~std::ios_base::floatfield;
		fpar->fmtstate_.flags_ |=  std::ios_base::fixed;
	case 'u':
	case 'd':
	case 'i':
		fpar->fmtstate_.flags_ &= ~std::ios_base::basefield;
		fpar->fmtstate_.flags_ |=  std::ios_base::dec;
		break;

	case 'T':
		++start;
		if( start >= last);
		else
			fpar->fmtstate_.fill_ = *start;
		fpar->pad_scheme_ |= format_item_t::tabulation;
		fpar->argN_ = format_item_t::argN_tabulation; 
		break;
	case 't': 
		fpar->fmtstate_.fill_ = ' ';
		fpar->pad_scheme_ |= format_item_t::tabulation;
		fpar->argN_ = format_item_t::argN_tabulation; 
		break;

	case 'G':
		fpar->fmtstate_.flags_ |= std::ios_base::uppercase;
		break;
	case 'g': // 'g' conversion is default for floats.
		fpar->fmtstate_.flags_ &= ~std::ios_base::basefield;
		fpar->fmtstate_.flags_ |=  std::ios_base::dec;

		// CLEAR all floatield flags, so stream will CHOOSE
		fpar->fmtstate_.flags_ &= ~std::ios_base::floatfield; 
		break;

	case 'C':
	case 'c': 
		fpar->truncate_ = 1;
		break;
	case 'S':
	case 's': 
		if(precision_set) // handle truncation manually, with own parameter.
			fpar->truncate_ = fpar->fmtstate_.precision_;
		fpar->fmtstate_.precision_ = 6; // default stream precision.
		break;
	case 'n' :  
		fpar->argN_ = format_item_t::argN_ignored;
		break;
	default: 
		break;
		}
		++start;

		if( in_brackets ) {
			if( start != last && *start== '|' ) {
				++start;
				return true;
			}
		}
		return true;
	}
	// -end parse_printf_directive()


	template<class Iter> 
	Iter wrap_scan_notdigit(Iter beg, Iter end) {
		using namespace std;
		for( ; beg!=end && isdigit(*beg); ++beg) ;
		return beg;
	}

	template<class String>
	int upper_bound_from_fstring(const String& buf, 
		const typename String::value_type arg_mark) 
	{
		// quick-parsing of the format-string to count arguments mark (arg_mark, '%')
		// returns : upper bound on the number of format items in the format strings
		
		//using namespace boost::io;
		typename String::size_type i1=0;
		int num_items=0;
		while( (i1=buf.find(arg_mark,i1)) != String::npos ) {
			if( i1+1 >= buf.size() ) {
				++num_items;
				break;
			}
			if(buf[i1+1] == buf[i1] ) {// escaped "%%"
				i1+=2; continue; 
			}

			++i1;
			// in case of %N% directives, dont count it double (wastes allocations..) :
			i1 = wrap_scan_notdigit(buf.begin()+i1, buf.end()) - buf.begin();
			if( i1 < buf.size() && buf[i1] == arg_mark )
				++i1;
			++num_items;
		}
		return num_items;
	}

	template<class String> inline
		void append_string(String& dst, const String& src, 
		const typename String::size_type beg, 
		const typename String::size_type end) {
			dst += src.substr(beg, end-beg);
	}


	template<class Ch, class Tr, class Alloc> inline 
		std::basic_string<Ch, Tr, Alloc> str(const basic_format<Ch, Tr, Alloc>& f) {
			// adds up all pieces of strings and converted items, and return the formatted string
			return f.str();
	}


	typedef basic_format<char, std::char_traits<char>, std::allocator<char> >				  format;
	typedef basic_format<wchar_t , std::char_traits<wchar_t >, std::allocator<wchar_t > >     wformat;

#ifdef UNICODE
	typedef basic_format<wchar_t , std::char_traits<wchar_t >, std::allocator<wchar_t > >     tformat;
#else
	typedef basic_format<char, std::char_traits<char>, std::allocator<char> >     tformat;
#endif
	



} // namespace boost

