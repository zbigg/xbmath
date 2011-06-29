#include <iostream>
#include <iomanip>
#include <string.h>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <stack>

#include "xbmath.h"


class	instruction;
typedef xbmath::integer 	    number_t;
//typedef xbmath::rational 	    number_t;

typedef std::stack<number_t> stack_t;
typedef std::string		    string_t;
typedef std::map<string_t,number_t> number_map_t;


class instruction {
public:
    enum code_e {
        add,
	load,
        dup,
        msg,
        echo,
	sub,
        mul,
        div,
        mod,
	pow,
        chs,
        dec,
        inc,
	load_var,
        store_var,
	factorial,
	invalid
    };
    instruction() : code(invalid) {}
    instruction( code_e c , const char* msg = "" )
	: code(c), text(msg) { }

    instruction( const number_t& n )
	: code(load), number(n) { }

    instruction( const char* s ) {
	if( isdigit(*s) ) {
	    code = load;
	    number.set(s);
	}
	else if ( *s == '+' )
	    code = add;
	else if ( *s == '-' ) {
	    if( isdigit(s[1]) ) {
		code = load;
		number.set(s+1);
		number.chs();
	    } else
		code = sub;
	}
	else if ( *s == 'i' )
	    code = inc;
	else if ( *s == 'd' )
	    code = dec;
	else if ( *s == '!' )
	    code = factorial;
	else if ( *s == '*' )
	    code = mul;
	else if ( *s == '/' )
	    code = div;
	else if ( *s == '%' )
	    code = mod;
	else if ( *s == '~' )
	    code = chs;
	else if ( *s == '^' )
	    code = pow;
	else if(  *s == '$' )
	    code = echo;
	else if(  *s == '#' )
	    code = dup;
	else if(  *s == '>' || *s == 's') {
	    if( s[1] == '\0' )
		throw "operator > requires variable name";
	    code = store_var;
	    text = s+1;
	}
	else if(  *s == '<' || *s == 'r') {
	    if( s[1] == '\0' )
		throw "operator < requires variable name";
	    code = load_var;
	    text = s+1;
	}
	else if(  *s == '"' || *s == '\'') {
	    code = msg;
	    text = s+1;
	}
	else
	    throw string_t("bad string: ")+string_t(s);
    }

    void     exec(stack_t&   stack,number_map_t& map) const;

    inline void test_stack(stack_t &s) const {
	if( s.empty() )
	    throw "expected value on stack";
    };

    inline operator number_t& () {
	return number;
    }
protected:
    code_e	code;
    number_t	number;
    string_t	text;

};

typedef std::vector<instruction>    code_t;


const char* pname = "rpn";

int main(int argc,char** argv)
{
    using std::cerr;
    using std::endl;
    pname = *argv++;
    string_t	work_name = "initialization";
    code_t  code;
    number_map_t    vars;
    stack_t stack;

    try {
	work_name = "reading data";
	while( *argv )
	    code.insert(code.end(), instruction(*argv++));

	work_name = "calculation";
	for( code_t::const_iterator i = code.begin() ;
	     i != code.end();
	     ++i )
	    i->exec(stack,vars);

    } catch (char* msg ) {
	cerr << pname << ": " << work_name.c_str() << " : " << msg << endl;
	return 1;
    } catch (string_t msg) {
	cerr << pname << ": " << work_name.c_str() << " : " << msg.c_str() << endl;
	return 1;
    }
    return 0;
}

void instruction::exec(stack_t&   stack,number_map_t& map) const
{
    using std::cout;
    using std::endl;
    switch( code ) {
    case dup:
	test_stack(stack);
	stack.push(stack.top());
	break;
    case load_var:
	{
	    number_map_t::iterator i = map.find(text);
	    if( i == map.end() )
		throw string_t("variable `" + text + "' not found");
	    stack.push(i->second);
	}
	break;
    case store_var:
	test_stack(stack);
	map[text] = stack.top();
	stack.pop();
	break;
    case load:
	stack.push(number);
	break;
    case msg:
	cout << text.c_str() << " ";
	break;
    case echo:
	test_stack(stack);
	cout << stack.top() << endl;
	stack.pop();
	break;
    case add:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
		     b += stack.top();	 stack.pop();
	    stack.push( b );
	}
	break;
    case mul:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
		     b *= stack.top();	 stack.pop();
	    stack.push( b );
	}
	break;
    case sub:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
	    number_t a	= stack.top();	 stack.pop();
	    a -= b;
	    stack.push( a );
	}
	break;
    case div:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
	    number_t a	= stack.top();	 stack.pop();
	    if( b.is_zero() )
		throw "division by zero";
	    a.div(b);
	    stack.push( a );
	}
	break;
    case mod:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
	    number_t a	= stack.top();	 stack.pop();
	    if( b.is_zero() )
		throw "division by zero";

	    // a.mod(b);
	    stack.push( a );
	}
	break;
    case pow:
	{
	    test_stack(stack);
	    number_t b	= stack.top();	 stack.pop();
	    test_stack(stack);
	    number_t a	= stack.top();	 stack.pop();
//	    a.pow( b );
	    stack.push( a );
	}
	break;
    case chs:
	{
	    test_stack(stack);
	    stack.top().chs();
	}
	break;
    case dec:
	{
	    test_stack(stack);
	    stack.top().dec();
	}
	break;
    case inc:
	{
	    test_stack(stack);
	    stack.top().inc();
	}
	break;
    case factorial:
	{
	    test_stack(stack);
	    stack.top().factorial( stack.top() );
	}
	break;
    default:
	throw "unknown instruction code";
    }
}

