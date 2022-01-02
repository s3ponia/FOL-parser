#BNF

Parser follows following grammar rules
```
<term>        ::= <constant>
		| <var>
		| <fn_symbol> (<term_list>)
<term_list>   ::= <term>
		| <term>, <term_list>
<form>        ::= <pred>
		| not <form>
		| <form> and <form>
		| <form> or <form>
		| <form> -> <form>
		| @ <var> <form> // forall
		| ? <var> <form> // exists
<char>        ::= ('A'...'Z'|'a'...'z'|'0'...'9'|''')
<name>        ::= <char>
		| <char><name>
<constant>    ::= c<name>
<var>         ::= v<name>
<fn_symbol>   ::= f<name>
<pred>        ::= p<name>
```
