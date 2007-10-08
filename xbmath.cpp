/*
* File		: xbmath.cpp
* Language	: C++
* Author	: Zbigniew Zagorski <longmanz@polbox.com>
* Description	: C++ Big number mathematic routines.

	Implementation of more complex functions.
	
	Changes:
	    
*
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

#include "xbmath.h"
#ifdef _MSC_VER
#pragma warning (disable: 4786) // long identifiers when creating debug info
#endif

xbmath::natural& xbmath::natural::set (const char* s)
{
    if( s ) {
	zero();
	if( *s ) {
	    if( isdigit(*s) )
		add( (atom)((*s) - '0') );
	    s++;
	}
	while( *s ) {
	    if( isdigit(*s) ) {
		mul10();
		add( (atom)((*s) - '0') );
	    }
	    s++;
	}
    } else
	p.insert(p.begin(),0);
    return *this;
}

void	xbmath::natural::str_dec(char* buf,int max) const
{
    //16 bits: 65535
    //32 bits: 4294967295
    //64 bits: 18446744073709551615
    switch( p.size() ) {
    case 0:
	strcpy(buf,"0");
	return;
    case 1:
	sprintf(buf,XBM_ATOM_FMT_DEC,*(p.begin()));
	return;
    }

    enum {
#if	_XBM_ATOM_LEN == 16
	atom_base =  10000
#elif	_XBM_ATOM_LEN == 32
	    atom_base =  1000000000
#elif	_XBM_ATOM_LEN == 64
#define 	atom_base  (UI64(1000000000000000000))
#else
	    atom_base =  1000000000
#endif
    };
    container summ;
    container pow2;

    pow2.insert(pow2.begin(),1);
    summ.insert(summ.begin(),0);

    const_iterator i = p.begin();
    const_iterator i_end = p.end();
    int shift_c = 0;
    for( ; i != i_end; ++i ) {
	register atom  b = first_bit;
	int left_bits = atom_bits;
	register atom act = *i;
	if( act != 0 )
	    do {
		if( act & b ) {
		    while( shift_c > 0 ) {
			xbmath::dec_add<atom_base>::add_container(pow2,pow2);
			shift_c--;
		    }
		    xbmath::dec_add<atom_base>::add_container(summ,pow2);
		    shift_c = 0;
		}
		shift_c++;
		b <<= 1;
		--left_bits;
		if( b > act ) {
		    shift_c += left_bits;
		    break;
		}
	    } while( b != 0 ); /* iterate bits in atom */
	    else {
		shift_c += atom_bits;
	    }
    } /* iterate atoms */
    { // output summ iterator to buffer
	const_iterator start = summ.begin();
	const_iterator i = summ.end();
	char nbuf[4*sizeof(atom)];
	char* s = buf;
	int l = 0;
	buf[0] = '\0';
	do {
	    if( l )
		sprintf(nbuf,XBM_ATOM_FMT_DEC_PAD, *(--i));
	    else
		sprintf(nbuf,XBM_ATOM_FMT_DEC, *(--i));
	    l = 1;
	    if( max == 0 )
		return;
	    strncpy(s,nbuf,max-1);
	    s[max-1] = '\0';
	    {
		int k = strlen(nbuf);
		max -= k;
		s += k;
	    }
	} while ( i != start );
    }
#if	_XBM_ATOM_LEN == 64
#undef atom_base
#endif
}


void	xbmath::natural::str_hex(char* buf,int max) const
{
    if( buf == NULL || max <= 0 )
	return;
    else if( p.size() == 0 ) {
	strncpy(buf,"0",max);
    } else {
	const_iterator start = p.begin();
	const_iterator i = p.end();
	char nbuf[2*sizeof(atom) + 2 ];
	char* s = buf;
	int l = 0;
	buf[0] = '\0';
	do {
	    if( l )
		sprintf(nbuf,XBM_ATOM_FMT_HEX_PAD, *(--i));
	    else
		sprintf(nbuf,XBM_ATOM_FMT_HEX, *(--i));
	    l = 1;
	    strncat(s,nbuf,max-1);
	    s[max-1] = '\0';
	    {
		int k = strlen(buf);
		max-= k;
		s+= k;
	    }
	} while ( i != start );
    }
} // strhex

xbmath::natural& xbmath::natural::inc()
{
    if( p.size() == 0 )
	p.insert(p.end(),1);
    else {
	iterator i = p.begin();
	if( ++(*i) == 0 ) {
	    iterator end = p.end();
	    int cf = 1;
	    while( ++i != end && cf )
		cf = (++(*i) == 0);
	    if( cf )
		p.insert(p.end(),1);
	}
    }
    return *this;
}

xbmath::natural& xbmath::natural::add (xbmath::atom t)
{
    iterator i = p.begin(),
	end = p.end();
    bool cf = false;
    if( i == end )
	p.insert(end,t);
    else {
	(*i) += t;
	cf = (*i < t);
	while( cf ) {
	    i++;
	    if( i == end ) {
		p.insert(i,1);
		break;
	    } else
		cf = (++(*i) == 0);
	}
    }
    return *this;
}

xbmath::natural& xbmath::natural::add (const xbmath::natural& n)
{
    const container& q = n.p;
    iterator i = p.begin();
    const_iterator j = q.begin();
    bool cf = false;

    int len = p.size() > q.size() ? q.size() : p.size();
    for(int l=0;l < len; ++l,++i,++j) {
	register atom iv = *i;
	register atom jv = *j;
	register atom t = iv + jv + cf;
	if( iv > jv ) {
	    cf = t < iv;
	} else {
	    cf = t < jv;
	}
	*i = t;
    }
    if( j != q.end() ) {
	if( cf ) {
	    do {
		register atom t = *j++ + 1;
		p.insert(p.end(),t);
		if( t != 0 ) {	// no carry
		    // append q.[j, ... ] to p
		    p.insert( p.end(), j, q.end());
		    return *this;
		}
		// there is carry
	    } while( j != q.end());
	} else {
	    // append q.[j, ... ] to p
	    p.insert( p.end(), j, q.end());
	}
    } else if( cf && i != p.end() ) {
	do {
	    if( !(cf = ++(*i) == 0) )
		return *this;
	} while( ++i != p.end());
    }
    if( cf )
	p.insert(p.end(),1);
    return *this;
} // add

xbmath::natural& xbmath::natural::mul (xbmath::atom act)
{
    if( act == 0 ) return zero();
    if( act == 1 ) return *this;
    natural summ;
    unsigned int shift_c = 0;
    register atom b = first_bit;
    int left_bits = atom_bits;
    do {
	if( act & b ) {
	    if( shift_c > 0 )
		shift_left(shift_c);
	    summ.add(*this);
	    shift_c = 0;
	}
	shift_c++;
	b = b << 1;
	--left_bits;
	if( b > act ) {
	    shift_c += left_bits;
	    break;
	}
    } while( b != 0 );
    *this = summ;
    return *this;
} // mul with atom

xbmath::natural& xbmath::natural::sqr (int n)
{
    natural summ;
    natural org;
    while ( n > 0 ) {
	summ.zero();
	org = *this;
	iterator i     = org.p.begin(),
	    i_end = org.p.end();
	int shift_c = 0;
	for( ; i != i_end; ++i ) {
	    register atom  b = first_bit;
	    int left_bits = atom_bits;
	    register atom act = *i;
	    do {
		if( act & b ) {
		    if( shift_c > 0 )
			shift_left(shift_c);
		    summ.add(*this);
		    shift_c = 0;
		}
		shift_c++;
		b = b << 1;
		--left_bits;
		if( b > act ) {
		    shift_c += left_bits;
		    break;
		}
	    } while( b != 0 );
	}
	*this = summ;
	n--;
    }
    return *this;
} // sqr

xbmath::natural& xbmath::natural::mul (const xbmath::natural& n)
{
    const_iterator i	 = n.p.begin(),
	i_end = n.p.end();
    natural summ;
    int shift_c = 0;
    for( ; i != i_end; ++i ) {
	register atom  b = first_bit;
	int left_bits = atom_bits;
	register atom act = *i;
	do {
	    if( act & b ) {
		if( shift_c > 0 )
		    shift_left(shift_c);
		summ.add(*this);
		shift_c = 0;
	    }
	    shift_c++;
	    b = b << 1;
	    --left_bits;
	    if( b > act ) {
		shift_c += left_bits;
		break;
	    }
	} while( b != 0 );
    }
    return *this = summ;
} // mul

xbmath::natural& xbmath::natural::pow(unsigned long c)
{
    switch( c ) {
    case 0:  one();
    case 1:  return *this;
    case 2:  return sqr();
    case 4:  return sqr(2);
    case 8:  return sqr(3);
    case 16: return sqr(4);
    case 32: return sqr(5);
    case 64: return sqr(6);
    case 128:return sqr(7);
    }
    natural t = *this;
    atom t_pow = 1;
    atom a_pow = 1;
    if( !( c & 1) ) {
	a_pow = 0;
	one();
    }
    atom b = 2;
    atom mul_c = 0;
    while( a_pow < c && b != 0 ) {
	++mul_c;
	if( c & b ) {
	    t.sqr(mul_c);
	    while( mul_c > 0 ) {
		t_pow *= 2;
		--mul_c;
	    }
	    mul_c = 0;
	    mul(t);
	    a_pow+=t_pow;
	}
	b = b << 1;
    }
    return *this;
} // pow

xbmath::natural& xbmath::natural::shift_left(int c)
{
    while( c >= atom_bits ) {
	p.insert(p.begin(),0);
	c-= atom_bits;
    }
    if( c > 0 ) {
	iterator i = p.begin();
	iterator end = p.end();
	atom carry = 0;
	while( i != end ) {
	    register atom t = *i >> (atom_bits - c);
	    *i = (*i << c) | carry;
	    carry = t;
	    ++i;
	}
	if( carry != 0 )
	    p.insert(p.end(),carry);
    }
    return *this;
}   /* shift_left */

xbmath::natural& xbmath::natural::shift_right(int c)
{
    while( c >= atom_bits ) {
	p.erase(p.begin());
	c-= atom_bits;
    }
    if( c > 0 ) {
	iterator i = p.end();
	iterator begin = p.begin();
	atom carry = 0;
	do { --i;
	register atom t = *i << (atom_bits - c);
	*i = (*i >> c) | carry;
	carry = t;
	} while ( i != begin );
    }
    delete_zeroes();
    return *this;
}   /* shift_right */

int xbmath::natural::cmp(const xbmath::natural& n) const
{
    register const container& q = n.p;
    /* check sizes */
    {
	register int d = p.size() - q.size();
	if( d )
	    return d;
    }
    const_iterator p_i = p.end();
    const_iterator q_i = q.end();
    do {
	--p_i, --q_i;
	if( *p_i > *q_i )
	    return 1;
	if( *p_i < *q_i)
	    return -1;
    } while ( p_i != p.begin() );
    return 0;
}

void xbmath::natural::delete_zeroes()
{
    iterator i = p.end();
    do {
	--i;
	if( *i != 0 )
	    break;
    } while (i != p.begin() );
    ++i;
    if( i != p.end() )
	p.erase(i,p.end());
}

void xbmath::integer::calc_div(
			       const xbmath::integer& a,
			       const xbmath::integer& b,
			       xbmath::integer& div_result,
			       xbmath::integer& mod_result)
// #define OUTPUT_CALC_DIV_STEPS cerr
{
    mod_result = a;
    div_result.zero();
    if( mod_result < b )
	return;
    else {
	integer current_div = b;
	integer current_multiplier = 1;
	int k;
	while(( k = current_div.cmp(mod_result)) < 0 ) {
	    current_multiplier.mul2();
	    current_div.mul2();
#ifdef OUTPUT_CALC_DIV_STEPS
	    OUTPUT_CALC_DIV_STEPS
		<< "1: mod_result="<< mod_result
		<< " current_mult=" << current_multiplier
		<< " current_div: " << current_div << endl;
#endif
	}
	do {
	    while(( k = current_div.cmp(mod_result)) > 0 ) {
		current_div.div2();
		current_multiplier.div2();
#ifdef OUTPUT_CALC_DIV_STEPS
		OUTPUT_CALC_DIV_STEPS
		    << "4: current_mult=" << current_multiplier
		    << " current_div=" << current_div << endl;
#endif
	    }
	    if( k == 0 ) {
		mod_result = 0;
		div_result += current_multiplier;
#ifdef OUTPUT_CALC_DIV_STEPS
		OUTPUT_CALC_DIV_STEPS
		    << "6: HIT k=0: mod_result=" << mod_result
		    << " div_result=" << div_result << endl;
#endif
		return;
	    }
	    mod_result -= current_div;
	    div_result += current_multiplier;

	    k = mod_result.cmp(b);
#ifdef OUTPUT_CALC_DIV_STEPS
	    OUTPUT_CALC_DIV_STEPS
		<< "7: mod_result("<< mod_result.p.size() <<")=" << mod_result
		<< ", b("<< b.p.size() <<")=" << b
		<< ", mod_result cmp b : " << k
		<< ", div_result=" << div_result << endl;
#endif
	} while( k >= 0 );
    }
}

bool xbmath::integer::calc_GCD1(
				xbmath::integer& result,
				const xbmath::integer& A,
				const xbmath::integer& B)
	    //
	    // A trivial implementation of Euklid algorithm
{
    integer a(A),b(B);
    int k;
    while( (k = a.cmp(b)))
	if( k > 0 ) // a > b :
	    a -= b;
	else	    // b > a
	    b -= a;
	result = a;
	return ! result.is_one();
}


bool xbmath::integer::calc_GCD(
			       xbmath::integer& result,
			       const xbmath::integer& A,
			       const xbmath::integer& B)
// A more obfuscated implementation of
// Euklid algorithm
// Derecursived.
{
    integer a(A),b(B);
    while( 1 ) {
	a.mod(b);
	if( a.is_zero() ) {
	    result = b;
	    return !result.is_one();
	}
	b.mod(a);
	if( b.is_zero() ) {
	    result = a;
	    return !result.is_one();
	}
    }
}

xbmath::integer& xbmath::integer::dec_nc()
{
    iterator i = p.begin();
    bool cf = false;
    if( p.size() == 0 ) {
	p.insert(p.end(),1);
	sign = !sign;
	return *this;
    }
    do {
	if( *i > 0 ) {
	    --(*i);
	    cf = false;
	    break;
	} else
	    cf = true;
	--(*i);
    } while( ++i != p.end() );
    if( cf ) {
	sign = false;
	*(p.end()-1) = ~*(p.end()-1) + 1;
    }
    delete_zeroes();
    if( is_zero() )
	sign = true;

    return *this;
}

xbmath::integer& xbmath::integer::sub_nc (const xbmath::integer& x)
{
    const container& q = x.p;
    iterator i = p.begin(),
	p_end = p.end();
    const_iterator j = q.begin(),
	q_end = q.end();
    bool cf = false;
    unsigned int len = p.size() < q.size() ? p.size() : q.size();
    for( unsigned int k = 0; k < len ; ++k,++i,++j ) {
	register atom t = *i - *j;
	if( cf ) {
	    cf = (*i <= *j );
	    --t;
#ifdef OUTPUT_SUB_STEPS
	    OUTPUT_SUB_STEPS << "1: sub: " << *i << " - " << *j << " - " << 1
		<< " = " << t << " ,cf=" << cf << endl;
#endif
	} else {
	    cf = ( t > *i );
#ifdef OUTPUT_SUB_STEPS
	    OUTPUT_SUB_STEPS << "1: sub: " << *i << " - " << *j
		<< " = " << t << " ,cf=" << cf << endl;
#endif
	}
	*i = t;
    }
    if( cf && i != p_end ) {
	do {
	    if( *i > 0 ) {
		--(*i);
		cf = 0;
		break;
	    } else
		cf = true;
	    --(*i);
	} while( ++i != p_end );
    } else if( j != q_end ) { do {
	/* append -*q */
	register atom t = *j + cf;
	cf = (t == 0);
	p.insert(p.end(),-t);
    } while( ++j != q_end );
    cf = true;
    }

    if( cf ) {
	sign = false;
	*(p.end()-1) = ~*(p.end()-1) + 1;
    }
    /*
    sign = !cf;
    if( !sign )
	*(p.end()-1) = ~*(p.end()-1) + 1;
    */
    delete_zeroes();
    if( is_zero() )
	sign = true;
    return *this;
}

xbmath::integer& xbmath::integer::div_nc(const xbmath::integer& b)
{
    if( cmp(b) < 0 ) {
	zero();
	return *this;
    }
    if( p.size() == 1 && b.p.size() == 1 ) {
	*p.begin() /= *b.p.begin();
	return *this;
    }
    integer div_result;
    integer current_div = b;
    integer current_multiplier = 1;
    int k;
    while(( k = current_div.cmp(*this)) < 0 ) {
	current_multiplier.mul2();
	current_div.mul2();
    }
    if( k == 0 ) {
	set(current_multiplier);
	return *this;
    }
    do {
	while(( k = current_div.cmp(*this)) > 0 ) {
	    current_div.div2();
	    current_multiplier.div2();
	}
	if( k == 0 ) {
	    set(div_result);
	    add(current_multiplier);
	    return *this;
	}
	sub_nc(current_div);
	div_result.add_nc(current_multiplier);
	k = cmp(b);
    } while( k >= 0 );
    set(div_result);
    return *this;
}

xbmath::integer& xbmath::integer::mod_nc(const xbmath::integer& b)
{
    if( cmp(b) < 0 )
	return *this;
    if( p.size() == 1 && b.p.size() == 1 ) {
	*p.begin() = *p.begin() % *b.p.begin();
	return *this;
    }
    integer current_div = b;
    integer current_multiplier = 1;
    int k;
    while( (k = cmp(current_div)) > 0 ) {
	current_div.mul2();
    }
    if( k ==0 ) {
	zero();
	return *this;
    }
    do {
	while(( k = current_div.cmp(*this)) > 0 ) {
	    current_div.div2();
	}
	if( k == 0 ) {
	    zero();
	    return *this;
	}
	sub_nc(current_div);

	k = cmp(b);
    } while( k >= 0 );
    return *this;
}

xbmath::rational& xbmath::rational::set(double f)
{
    return *this;
}

void xbmath::rational::insert_string_at(char* dest,int dest_len,int max,int at,const char* src)
{
    int src_len = strlen (src);
    register const char *s;
    register	 char *d;
    for (d = dest + dest_len + src_len, s = dest + dest_len;
    s > dest + at; )
    {
	*--d = *--s;
    }
    for (d = dest + at, s = src; d < dest + at + src_len; )
	*d++ = *s++;
    dest[dest_len + src_len] = 0;
}

int	xbmath::rational::str_dec_length(int prec) const
{
    integer x = p;
    x.mul10(prec+1);
    x.div(q);
    return x.str_dec_length() + prec + 2;

}

char*	xbmath::rational::str_dec(char* buf,int max,int prec) const
{
    integer x = p;
    x.mul10(prec);
    x.abs();
    x.div(q);
    if( buf == NULL ) {
	max = x.str_dec_length() + prec + 2;
	buf = new char[ max + 1];
    }
    if( x.is_zero() ) {
	int i = 0;
	buf[i++] = '0'; buf[i] = 0;
	if( prec == 0 )
	    return buf;
	if( max == 1 ) return buf;
	buf[i++] = '.'; buf[i] = 0;
	while( i < max-1 && i-2 <= prec) {
	    buf[i++] = '0'; buf[i] = 0;
	}
    } else {
	x.str_dec(buf,max-1);
	int len = strlen(buf);
	if( len <= prec ) {
	    while( len < prec ) {
		insert_string_at(buf,len,max,0,"0");
		len++;
	    }
	    insert_string_at(buf,len,max,0,"0.");
	} else {
	    insert_string_at(buf,len,max,len-prec,".");
	}
	if( !positive() )
	    insert_string_at(buf,len+1,max,0,"-");
    }
    return buf;
}

