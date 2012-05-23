/*
* File		: xbmath.h
* Language	: C++
* Author	: Zbigniew Zagorski <longmanz@polbox.com>
* Description	: C++ Big number mathematic routines
*
	Changes:
	    
* Header file contains definition af three classes:
    xbmath::natural
	Representation of positive integer (including zero).
	Math operations:
	    * addition
	    * multipication
	    * power
	Abstract class. Use integer instad of this class.

    xbmath::integer
	Representation of integer with sign.
	Math operations:
	    * addition
	    * substraction
	    * multipication
	    * integer division (calculating modulo)
	    * power
	    * string output: decimal/hexadecimal

    xbmath::rational
	    * addition
	    * substraction
	    * multipication
	    * division
	    * expanding
	    * shrinking (using Euklid GCD algorithm)
	    * decimal string output with specified 
	      precision
    
* Use:
    These defines may apear before including header:
    
    #define XBM_NO_IOSTREAM 
	if you don't want output operators for ostream

    #define XBM_NEED_NAMESPACE or
    #define XBM_WITH_NAMESPACE 
	if you want xbmath not to be class as it is set to default
	but xbmath will be namespace
	
    #define TRY_64BIT_ATOM 
	if you want atom to be long long (or __int64) (default is long)
    
    #define NO_STD_NAMESPACE 
	if STL templates are in global namespace: 
	    ::vector 
	instead of namespace std:
	    std::vector
    
* Copyright

This software is Copyright(c) Zbigniew Zagorski, 2001.
All rights reserved, and is distributed as free software under the
following license.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

- Any commercial use of this software without specific prior written
permission is not allowed.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND THE CONTRIBUTORS
"AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDERS OR THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __xbmath_h_
#define __xbmath_h_
#ifdef	__cplusplus

#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <string>

#define XBM_NEED_NAMESPACE
#define XBM_WITH_EXCEPTIONS

#if defined(XBM_NEED_NAMESPACE) || defined(XBM_WITH_NAMESPACE)
namespace xbmath {
#else
class xbmath {
public:
#endif

// #define TRY_64BIT_ATOM

    /* MSC & BORLANDC */
#ifdef TRY_64BIT_ATOM
#if	defined  _MSC_VER || defined __BORLANDC__
    typedef unsigned __int64	atom;
    typedef signed __int64	signed_atom;
#   define _XBM_ATOM_LEN	64
#   define _XBM_ATOM_FMT_DEC_	"I64u"
#   define _XBM_ATOM_FMT_HEX_	"I64x"
#   define UI64(x)  x ## ui64

    /* EGCS */
#elif	defined  __GNUC__
    typedef unsigned long long	atom;
    typedef signed long long	signed_atom;
#   define _XBM_ATOM_LEN	64
#   define _XBM_ATOM_FMT_DEC_	"llu"
#   define _XBM_ATOM_FMT_HEX_	"llx"
#   define UI64(x)  x ## ull
#endif
#endif

/* Force default atom to usigned long. */
#ifndef _XBM_ATOM_LEN
#   define _XBM_ATOM_LEN	32
#   define _XBM_ATOM_FMT_DEC_	"lu"
#   define _XBM_ATOM_FMT_HEX_	"lx"
    typedef unsigned long	atom;
    typedef signed long 	signed_atom;
#endif
/* Format specifier for output atom. */
#define XBM_ATOM_FMT_DEC    "%"_XBM_ATOM_FMT_DEC_
#define XBM_ATOM_FMT_HEX    "%"_XBM_ATOM_FMT_HEX_

/* Format specifier for padded output atom. */
#if	_XBM_ATOM_LEN == 16
#define XBM_ATOM_FMT_DEC_PAD	"%04"_XBM_ATOM_FMT_DEC_
#define XBM_ATOM_FMT_HEX_PAD	"%04"_XBM_ATOM_FMT_HEX_

#elif	_XBM_ATOM_LEN == 32
#define XBM_ATOM_FMT_DEC_PAD	"%09"_XBM_ATOM_FMT_DEC_
#define XBM_ATOM_FMT_HEX_PAD	"%08"_XBM_ATOM_FMT_HEX_

#elif	_XBM_ATOM_LEN == 64
#define XBM_ATOM_FMT_DEC_PAD	"%018"_XBM_ATOM_FMT_DEC_
#define XBM_ATOM_FMT_HEX_PAD	"%016"_XBM_ATOM_FMT_HEX_

#endif

    enum constants {
	atom_bits = sizeof( atom ) * 8,
	first_bit = 1,
	last_bit  = first_bit << (atom_bits-1)
    };

#ifndef NO_STD_NAMESPACE
    typedef std::vector<atom>	container;
#else
    typedef vector<atom>	container;
#endif

    typedef container::iterator iterator;
    typedef container::const_iterator const_iterator;

    typedef std::string		string;

    template <class NUM>
	static unsigned largest_bit(NUM a) {
	    NUM x = 1;
	    unsigned c = 0,i=1;
	    while( x ) {
		if( a & x )
		    c = i;
		i+=1;
		x <<= 1;
	    }
	    return c;
	}
    /* misceleanous */
//    template <class T>
//	T min(T a, T b) {
//	    return a < b ? a : b;
//	}
    template <atom base>
    // base < (2^atom_bits) / 2
	    class dec_add {
	    private:
		static inline atom atom_add(atom a,atom b,int& carry) {
		    register atom x = a + b + carry;
		    if (x >= base) {
			carry = x / base;
			x %= base;
		    } else
			carry = 0;
		    return x;
		}
	    public:
		static void add_container(container& a,const container& b) {
		    int len = a.size() < b.size() ? a.size() : b.size();
		    int cf = 0;
		    iterator i = a.begin();
		    const_iterator j = b.begin();
		    for(int l = 0; l<len;++l,++j,++i)
			*i = atom_add(*i,*j,cf);
		    if( j != b.end() ) {
			do {
			    if( cf )
				a.insert(a.end(),atom_add(0,*j,cf));
			    else
				a.insert(a.end(),*j);
			} while( ++j != b.end() );
		    } else if( cf && i != a.end() ) {
			do {
			    *i = atom_add(*i,0,cf);
			    if( !cf )
				break;
			} while ( ++i != a.end());
		    }
		    if( cf )
			a.insert(a.end(),1);
		}
	    }; // template <atom base> class dec_add
    class natural;
	/* class natural description
	    1. constructors
		natural (atom = 0 )
		natural (const natural&)
		natural (const container& c)
		natural (const char*)

	    2. set
		natural&    set (const char*)
		natural&    set (const natural&)
		natural&    set (atom)

		natural&    zero()
		natural&    one()
	    3. test
		bool	    is_zero()
	TODO!	int	    cmp(atom) TODO!
		int	    cmp(const natural& a)

	    4. math operations
		natural&    inc ()

		natural&    add (atom)
		natural&    add (const natural&)

		natural&    mul (atom)
		natural&    mul (const natural&)

		natural&    pow (unsigned long exp = 2)
		natural&    sqr (unsigned long exp = 1)

		natural&    shift_left(unsigned long shift_count = 1)
		natural&    shift_right(unsigned long shift_count = 1)

		natural&    mul10(int exponent = 1)
		natural&    mul2(int exponent = 1)
		matural&    div2(int exponent = 1)

	    5.	output
			    str_dec	(const char* buf,int max)
			    str_hex	(const char* buf,int max)

		int	    str_dec_length()
		int	    str_hex_length()
    */
    class integer;  // +/- natural
	/* INTERFACE description
	    1. constructors
		integer (signed_atom atom = 0 )
		integer (const integer&)
		integer (const natural&)
		integer (const container& c)
		integer (const char*)

	    2. set
		integer&    set (const char*)
		integer&    set (const integer&)
		integer&    set (atom)

		integer&    zero()
		integer&    one()

	    3. test
		bool	    is_zero()
		bool	    is_one()
	TODO!	unsigned    is_pow2()

	TODO!	int	    cmp(atom)
		int	    cmp(const integer& a)

	    4. math operations
		integer&    dec ()
		integer&    inc ()

	TODO!	integer&    add (atom)
		integer&    add (const integer&)

	TODO!	integer&    sub (atom)
		integer&    sub (const integer&)

	TODO!	integer&    mul (atom)
		integer&    mul (const integer&)

	TODO!	integer&    div (atom)
		integer&    div (const integer&)

	TODO!	integer&    mod (atom)
		integer&    mod (const integer&)

		integer&    pow (unsigned long exp = 2)
		integer&    sqr (unsigned long exp = 1)

		integer&    shift_left(unsigned long shift_count = 1)
		integer&    shift_right(unsigned long shift_count = 1)

		integer&    mul10(int exponent = 1)
		integer&    mul2(int exponent = 1)
		matural&    div2(int exponent = 1)

	    5.	output
			    str_dec	(const char* buf,int max)
			    str_hex	(const char* buf,int max)

		int	    str_dec_length()
		int	    str_hex_length()
    */
    class rational; // integer / integer
	/* INTERFACE description
	    1. constructors
	TODO!	rational (double f = 0 )
		rational (const rational&)
		rational (signed_atom p, signed_atom q = 1)
		rational (const integer& p, const integer& q)
		rational (const char*)

	    2. set
		rational&    set (const char*)
		rational&    set (const rational&)
		rational&    set (const integer&)
		rational&    set (double f = 0)

		rational&    zero()
		rational&    one()
		rational&    abs()
		rational&    chs()  - change sign

	    3. test
		bool	    positive()
		bool	    negative()

		bool	    is_zero()
		bool	    is_one()

		int	    cmp(const integer& i)
		int	    cmp(const rational& a)

	    4. math operations
		rational&    dec ()
		rational&    inc ()

		rational&    add (const rational&)
		rational&    add (const integer&)

		rational&    sub (const rational&)
		rational&    sub (const integer&)

		rational&    mul (const rational&)
		rational&    mul (const integer&)

		rational&    div (const rational&)
		rational&    div (const integer&)


		rational&    pow (unsigned long exp = 2)
		rational&    sqr (unsigned long exp = 1)

		rational&    expand(const integer& x) 
		rational&    shrink()

		rational&    mul10(int exponent = 1)
		rational&    mul2(int exponent = 1)
		rational&    div2(int exponent = 1)

	    5.	output

		char*	    str_dec	(const char* buf,int max,int prec = 4)
		int	    str_dec_length(int prec = 4)

	*/
    // class real;      under development
#ifdef XBM_WITH_EXCEPTIONS
    class exception {
    public:
	enum exc_code_e {
	    exc_unknown,
	    exc_division_by_zero,
	};
    private:

	string msg;
	exc_code_e exc_code;
	exception();

    public:
	
	exception(string s);
	exception(exc_code_e ecode);
	const char* get_str() const;
    };
#endif

    class natural {
    friend class integer;
    friend class rational;
    protected:
	container p;
    public:
	/* constructor
			(atom = 0 )
			(const natural&)
			(const container& c)
			(const char*)
	*/
	natural(atom n = 0) {
	    p.insert(p.begin(),n);
	}
	natural(const natural& n) : p(n.p)
	{  }
	natural(const container& c) : p(c)
	{  }

	const char* info() {
	    static char buf[200];
	    sprintf(buf,"big natural info: %i",(int)p.size());
	    return buf;
	}
	natural(const char* s){
	    set(s);
	}
	/*
	    set
		(const char*)
		(const natural&)
		(atom)
	*/
	natural& set (const char* s); /* xbmath.cpp */
	natural& set (const natural& n) {
	    p = n.p;
	    return *this;
	}
	natural& set (atom t) {
	    p.erase(p.begin(),p.end());
	    p.insert(p.end(),t);
	    return *this;
	}
	inline natural& zero() {
	    return set((atom)0);
	}
	inline bool is_zero() const {
	    switch( p.size() ) {
	    case 0:
		return true;
	    case 1:
		return ( *p.begin() ) == 0;
	    default:
		return false;
	    }
	}
	inline bool is_one() const {
	    switch( p.size() ) {
	    case 0:
		return false;
	    case 1:
		return ( *p.begin() ) == 1;
	    default:
		return false;
	    }
	}
	inline natural& one(){
	    return set((atom)1);
	}

	int	str_dec_length() const {
	    return 1 + (( p.size() == 0 ? 1 : p.size() ) * sizeof(atom) * 8) / 3;
	}
    public:
	inline void	str_dec_new(char* buf,int max) const { str_dec(buf,max); }

	void	str_dec(char* buf,int max) const; // xbmath.cpp

	int	str_hex_length() const
	{
	    return p.size() * sizeof(atom)*2;
	}

	void	str_hex(char* buf,int max) const; // xbmath.cpp

	natural& inc();				  // xbmath.cpp  
	natural& add (atom t);
	natural& add (const natural& n);
	natural& mul (atom act);
	natural& sqr (int n = 1);
	natural& mul (const natural& n);
	natural& pow(unsigned long c = 2);
	natural& shift_left(int c = 1);
	natural& shift_right(int c = 1);

	int cmp(const natural& n) const;

	void delete_zeroes();

	unsigned largest_bit() const {
	    return 
		(p.size()-1)*atom_bits + 
		(::xbmath::largest_bit( *(p.end()-1) ));
	}
	inline natural& mul10(int c = 1) {
	    natural x;
	    while( c > 0 ) {
		x.set(*this);
		x.shift_left(3);
		shift_left(1);
		add(x);
		--c;
	    }
	    return *this;
	}

	inline natural& mul2(int c = 1) {
	    return shift_left(c);
	}
	inline natural& div2(int c = 1) {
	    return shift_right(c);
	}

	natural&    factorial(atom f = 1)
	{
	    one();
	    for(atom i = 2; i <= f ; i++ )
		mul(i);
	    return *this;
	}

	inline natural& operator = (atom t)		{ return set(t); }
	inline natural& operator = (const natural& t)	{ return set(t); }

	inline natural& operator *= (atom t) { return mul(t); }
	inline natural& operator *= (const natural& n) { return mul(n); }
	inline natural& operator += (atom t) { return add(t); }
	inline natural& operator += (const natural& n) { return add(n); }

	natural operator << (unsigned int c) const { return natural(*this).shift_left(c); }
	natural operator >> (unsigned int c) const { return natural(*this).shift_right(c); }
	natural operator <<=(unsigned int c)	{ return shift_left(c); }
	natural operator >>=(unsigned int c)	{ return shift_right(c); }
	natural operator +  (const natural& n) const { return natural(*this) += n; }
	natural operator *  (const natural& n) const { return natural(*this) *= n; }
	inline operator signed_atom () const {
	    return (p.size() > 0) ? *p.begin() : (atom)0;
	}
	inline bool operator == (const natural& x) const { return cmp(x) == 0; };
	inline bool operator != (const natural& x) const { return cmp(x) != 0; };
	inline bool operator >	(const natural& x) const { return cmp(x) >  0; };
	inline bool operator <	(const natural& x) const { return cmp(x) <  0; };
	inline bool operator >= (const natural& x) const { return cmp(x) >= 0; };
	inline bool operator <= (const natural& x) const { return cmp(x) <= 0; };
    };

    class integer : public natural {
    public:
	// Sign: true means positive, false negative.
	bool sign;
	integer(signed_atom n = 0) : natural(n & ~last_bit),
	    sign(n >= 0 ) {}
	integer(const natural n,bool s = true) : natural(n),
	    sign(s) { }
	integer(const integer& i) : natural(i.p),
	    sign(i.sign) {}

	integer(const char* s) {
	    set(s);
	}

	/**
	    set
		(signed_atom)
		(const char)
		(const natural&)
		(const integer&)

	*/
	integer& set (signed_atom t) {
	    p.erase(p.begin(),p.end());
	    p.insert(p.end(),((atom)t) & ~last_bit);
	    sign = t >= 0;
	    return *this;
	}
	integer& set (const char* s) {
	    sign = (s) ? (*s == '-' ? (s++,false) :  true ): true;
	    natural::set(s);
	    return *this;
	}
	integer& set (const natural& n) {
	    p = n.p;
	    sign = true;
	    return *this;
	}
	integer& set (const integer& i) {
	    p = i.p;
	    sign = i.sign;
	    return *this;
	}
	// is one is inherited from natural
	//inline bool is_one() const {
	//   
	//}

protected:
	inline integer& add_nc (const integer& i) {
	    natural::add(i);
	    if( is_zero() )
		sign = true;
	    return *this;
	}
	integer& sub_nc (const integer& x);
	integer& div_nc(const integer& b);
	inline	integer& mul_nc(const integer& i) {
	    natural::mul(i);
	    return *this;
	}
	integer& mod_nc(const integer& b);
	inline	integer& inc_nc()
	{
	    natural::inc();
	    return *this;
	}
	integer& dec_nc();


public:	void	str_dec(char* buf,int max) 
	{
	    if( !sign ) {
		*buf++ = '-';
		*buf = '\0';
		max--;
	    }
	    natural::str_dec(buf,max);
	}


public: integer& add (int i) 
	{
	    if( sign )
		if( i>=0 )	// ( (+a) + (+b) )
		    add_nc(i);
		else		// ( (+a) + (-b) ) == ( a - b )
		    sub_nc(i);
	    else
		if( i>=0 ) {	// ( (-a) + (+b) ) == ( b - a ) == - ( a - b )
		    sign = true;
		    sub_nc(i);
		    sign = !sign;
		} else		// ( (-a) + (-b) ) == ( -( a + b)  )
		    add_nc(i);
	    return *this;
	}

public: integer& add (const integer& i) 
	{
	    if( sign )
		if( i.sign )	// ( (+a) + (+b) )
		    add_nc(i);
		else		// ( (+a) + (-b) ) == ( a - b )
		    sub_nc(i);
	    else
		if( i.sign ) {	// ( (-a) + (+b) ) == ( b - a ) == - ( a - b )
		    sign = true;
		    sub_nc(i);;
		    sign = !sign;
		} else		// ( (-a) + (-b) ) == ( -( a + b)  )
		    add_nc(i);
	    return *this;
	}

public: integer& sub (const integer& i) 
	{
	    if( sign ) if( i.sign )	// ( (+a) - (+b) ) == ( |a| - |b| )
		    sub_nc(i);
		else		// ( (+a) - (-b) ) == ( |a| + |b| )
		    add_nc(i);
	    else if( i.sign )	// ( (-a) - (+b) ) == (-|a| - |b| ) == -(|a| + |b|)
		    add_nc(i);
		else {		// ( (-a) - (-b) ) == ( -|a| + |b|) == -(|a| - |b|)
		    sub_nc(i);
		    sign = !sign;
		}
	    return *this;
	}

public:	inline	integer& mul(const natural& i) {
	    natural::mul(i);
	    if( is_zero() )
		sign = true;
	    return *this;
	}

public: inline	integer& mul(const integer& i) {
	    natural::mul(i);
	    sign = (sign == i.sign);
	    if( is_zero() )
		sign = true;
	    return *this;
	}

public:	integer& div(const integer& i) 
	{
	    bool s = (sign == i.sign);
	    div_nc(i);
	    sign = s;
	    return *this;
	}

public:	integer& mod(const integer& i) {
	    bool s = (sign == i.sign);
	    mod_nc(i);
	    sign = s;
	    return *this;
	}


public:	integer& dec() {
	    return sign ? dec_nc() : inc_nc();
	}
public:	integer& inc() {
	    return sign ? inc_nc() : dec_nc();
	}
public: static void calc_div(
	    const integer& a,
	    const integer& b,
		  integer& div_result,
		  integer& mod_result);

	
public: static bool calc_GCD1( // this version shouldn't be used 
		  integer& result,
	    const integer& A,
	    const integer& B);

public: static bool calc_GCD(
		  integer& result,
	    const integer& A,
	    const integer& B);

public: inline	int cmp(const integer& i) const {
	    if( sign == i.sign )
		return natural::cmp(i);
	    if( sign )
		return 1;
	    else
		return -1;
	}

public: inline integer& shift_left(unsigned long c=1) {
	    natural::shift_left(c);
	    return *this;
	}
public: inline integer& shift_right(unsigned long c=1) {
	    natural::shift_right(c);
	    return *this;
	}
public: inline integer& sqr (int n = 1) {
	    sign = true;
	    natural::sqr(n);
	    return *this;
	}
public: inline integer& pow(unsigned long c) {
	    natural::pow(c);
	    sign = (c & 1) ? sign : true;
	    return *this;
	}
public: inline integer& chs() {
	    sign = !sign;
	    return *this;
	}
public: inline integer& abs() {
	    sign = true;
	    return *this;
	}
public:	inline integer& operator  = (signed_atom i) { return set(i); }
	inline integer& operator  = (const natural& i) { return set(i); }
	inline integer& operator  = (const integer& i) { return set(i); }

        inline integer& operator *= (int i) { return mul(integer(i)); }
	inline integer& operator += (int i) { return add(i); }
	inline integer& operator -= (int i) { return sub(i); }

	inline integer& operator += (const integer& n) { return add(n); }
	inline integer& operator -= (const integer& n) { return sub(n); }
	inline integer& operator *= (const integer& n) { return mul(n); }
	inline integer& operator /= (const integer& n) { return div(n); }

	integer operator <<= ( unsigned int c) { return shift_left(c); }
	integer operator >>= ( unsigned int c) { return shift_right(c); }

	inline integer operator + (const integer& b) const { return integer(*this).add(b); }
	inline integer operator - (const integer& b) const { return integer(*this).sub(b); }
	inline integer operator * (const integer& b) const { return integer(*this).mul(b); }
	inline integer operator / (const integer& b) const { return integer(*this).div(b); }
	inline integer operator % (const integer& b) const { return integer(*this).mod(b); }
	inline integer operator - ()		     const { return integer(*this).chs();  }
	inline integer operator << ( unsigned int c) const { return integer(*this).shift_left(c); }
	inline integer operator >> ( unsigned int c) const { return integer(*this).shift_right(c); }

	inline bool operator == (const integer& x) const { return cmp(x) == 0; };
	inline bool operator != (const integer& x) const { return cmp(x) != 0; };
	inline bool operator >	(const integer& x) const { return cmp(x) >  0; };
	inline bool operator <	(const integer& x) const { return cmp(x) <  0; };
	inline bool operator >= (const integer& x) const { return cmp(x) >= 0; };
	inline bool operator <= (const integer& x) const { return cmp(x) <= 0; };

	inline operator signed_atom () {
	    return (p.size() > 0 ) ? ( sign ? *p.begin() : -*p.begin() ) : 0;
	}
    };// xbmath:: integer

    class rational {
public:
	integer p,q;
	rational( double f = 0 ) : p(1), q(1) { set(f); };
	rational( signed_atom _p, signed_atom _q = 1) : p(_p),q(_q) { }
	rational( const integer& _p,const integer& _q) : p(_p),q(_q) { }
	rational( const integer& _p) : p(_p), q(1) { }
	rational( const rational& r) : p(r.p), q(r.q) { }
	rational( const char* s) : p(1), q(1) { set(s); }

	rational& set(double f = 0); //xbmath.h
	/** Set number from string.

	                  1234587654
	   12345.87654 =  ----------
                            100000
	    then shrink()
	*/
	rational& set(const char* s) {
	    int id = strcspn(s,".,");
	    if( id == 0 ) id++;
	    int len = strlen(s)-1;
	    p.set(s);
	    q.set(1);
	    q.mul10(len-id);
	    shrink();
	    return *this;
	}

	rational& set(const integer& i) {
	    p = i;
	    q.one();
	    return *this;
	}
	rational& set(const rational& r) {
	    p = r.p;
	    q = r.q;
	    return *this;
	}
	rational&   expand(const integer& x) {
	    p.mul(x);
	    q.mul(x);
	    return *this;
	}
	rational&   shrink() {
	    integer result;

	    while( integer::calc_GCD(result,p,q) ) {
		p.div( result );
		q.div( result );
	    }
	    return *this;
	}
	rational& add(const integer& i) {
	    integer x = i;
	    x *= q;
	    p += x;
	    return *this;
	}
	rational& sub(const integer& i) {
	    integer x = i;
	    x *= q;
	    p -= x;
	    return *this;
	}
	rational& mul(const integer& i) {
	    p *= i;
	    return *this;
	}
	rational& div(const integer& i) {
	    q *= i;
	    return *this;
	}
	rational& add(const rational& r) {
	    integer t;
	    p *= r.q;
	    t = r.p;
	    t *= q;
	    p += t;
	    q *= r.q;
	    return *this;
	}
	rational& sub(const rational& r) {
	    integer t;
	    p *= r.q;
	    t = r.p;
	    t *= q;
	    p -= t;
	    q *= r.q;
	    return *this;
	}
	rational& mul(const rational& r) {
	    p *= r.p;
	    q *= r.q;
	    return *this;
	}
	rational& div(const rational& r) {
	    p *= r.q;
	    q *= r.p;
	    return *this;
	}
	inline bool positive() const  {
	    return p.sign == q.sign;
	}
	inline bool negative() const  {
	    return p.sign != q.sign;
	}
	inline bool is_zero() const  {
	    return p.is_zero();
	}
	inline bool is_one() const  {
	    return p == q;
	}
	inline rational& sqr (int n = 1) {
	    p.sqr(n);
	    q.sqr(n);
	    return *this;
	}
	inline rational& pow(unsigned long e = 2) {
	    p.pow(e);
	    q.pow(e);
	    return *this;
	}
	inline rational& inc() {
	    p += q;
	    return *this;
	}
	inline rational& dec() {
	    p -= q;
	    return *this;
	}
	inline rational& chs() {
	    p.sign = !p.sign;
	    return *this;
	}
	inline rational& abs() {
	    p.sign = q.sign = true;
	    return *this;
	}
	rational& zero() {
	    p.zero();
	    q.one();
	    return *this;
	}
	rational& one() {
	    p.one();
	    q.one();
	    return *this;
	}
	inline int cmp(const integer& i) const {
	    // very temporary version.
	    return cmp( rational( i ) ) ;
	}

	int cmp(const rational& r) const {
	    // temporary version.
	    rational x = *this;
	    x -= r;
	    if( x.is_zero() ) return 0;
	    if( x.positive() ) return 1;
	    return -1;
	}
	
	inline rational&    mul10(int exponent = 1) 
	{
	    p.mul10(exponent);
	    return *this;
	}
	inline rational&    mul2(int exponent = 1)
	{
	    p.shift_left(exponent);
	    return *this;
	}
	inline rational&    div2(int exponent = 1)
	{
	    q.shift_left(exponent);
	    return *this;
	}

public:	int	str_dec_length(int prec = 4) const;
public:	char*	str_dec(char* buf,int max,int prec = 4) const;

private:static  void insert_string_at(char* dest,int dest_len,int max,int at,const char* src);

public:
	inline rational& operator += (const rational& r) { return add(r); }
	inline rational& operator -= (const rational& r) { return sub(r); }
	inline rational& operator *= (const rational& r) { return mul(r); }
	inline rational& operator /= (const rational& r) { return div(r); }

	inline rational& operator += (const integer& i) { return add(i); }
	inline rational& operator -= (const integer& i) { return sub(i); }
	inline rational& operator *= (const integer& i) { return mul(i); }
	inline rational& operator /= (const integer& i) { return div(i); }

	inline rational& operator  = (const integer& i) { return set(i); }
	inline rational& operator  = (const rational& r){ return set(r); }
	inline rational& operator  = (const char* s)	{ return set(s); }
	inline rational& operator  = (float f)		{ return set(f); }

	inline rational  operator +  (const rational& r) const { return rational(*this).add(r); }
	inline rational  operator -  (const rational& r) const { return rational(*this).sub(r); }
	inline rational  operator *  (const rational& r) const { return rational(*this).mul(r); }
	inline rational  operator /  (const rational& r) const { return rational(*this).div(r); }

	inline rational  operator +  (const integer& i) const { return rational(*this).add(i); }
	inline rational  operator -  (const integer& i) const { return rational(*this).sub(i); }
	inline rational  operator *  (const integer& i) const { return rational(*this).mul(i); }
	inline rational  operator /  (const integer& i) const { return rational(*this).div(i); }

	inline rational operator - () const { return rational(*this).chs();  }

	inline bool operator == (const rational& r) const { return cmp(r) == 0; };
	inline bool operator != (const rational& r) const { return cmp(r) != 0; };
	inline bool operator >	(const rational& r) const { return cmp(r) >  0; };
	inline bool operator <	(const rational& r) const { return cmp(r) <  0; };
	inline bool operator >= (const rational& r) const { return cmp(r) >= 0; };
	inline bool operator <= (const rational& r) const { return cmp(r) <= 0; };

	inline bool operator == (const integer& i) const { return cmp(i) == 0; };
	inline bool operator != (const integer& i) const { return cmp(i) != 0; };
	inline bool operator >	(const integer& i) const { return cmp(i) >  0; };
	inline bool operator <	(const integer& i) const { return cmp(i) <  0; };
	inline bool operator >= (const integer& i) const { return cmp(i) >= 0; };
	inline bool operator <= (const integer& i) const { return cmp(i) <= 0; };

    }; // xbmath:: rational

#ifndef XBM_NO_IOSTREAM
inline std::ostream& operator << (std::ostream& s, const xbmath::natural& n)
{
    if( s.flags() & std::ios::hex ) {
	int max = n.str_hex_length()+1;
	char* buf = new char[max+1];
	memset(buf,0,max+1);
	n.str_hex(buf,max);
	s << buf;
	delete [] buf;
    } else {
	int max = n.str_dec_length()+1;
	char* buf = new char [max+1];
	memset(buf,0,max+1);
	n.str_dec(buf,max);
	s << buf;
	delete [] buf;
    }
    return s;
}

inline std::ostream& operator << (std::ostream& s, const integer& i)
{
    if( ! i.sign )
	s << '-';
    return s << (const natural)i;
}

inline std::ostream& operator << (std::ostream& s, const rational& r)
{
    char* buf = r.str_dec(NULL,0,s.precision());
    s << buf;
    delete [] buf;
    return s;
}
#endif // XBM_NO_IOSTREAM
    
#ifdef XBM_NEED_NAMESPACE
} // namespace xbmath
#else
}; // class xbmath
#endif



#endif // __cplusplus


#endif // __bmath_h_
