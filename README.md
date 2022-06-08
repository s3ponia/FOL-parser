# BNF

Parser follows following grammar rules
```
<impl>        ::= <disj> -> <impl>
                | <disj>
<disj>        ::= <conj> <disj'>
<disj'>       ::= or <conj> <disj'>
                | EPS
<conj>        ::= <unary> <conj'>
<conj'>       ::= and <unary> <conj'>
                | EPS
<unary>       ::= ( <impl> )
                | ~ unary
                | @ <var> . <impl>
                | ? <var> . <impl>
                | <pred> (<term_list>)
<term>        ::= <constant>
		| <var>
                | <fn_symbol> (<term_list>)
<term_list>   ::= <term> <term_list'>
<term_list'>  ::= , <term_list>
		| EPS
<name>        ::= <char>
                | <char><name>
<char>        ::= ('A'...'Z'|'a'...'z'|'0'...'9'|''')
<constant>    ::= c<name>
<var>         ::= v<name>
<renamed_var> ::= vu<name>
<fn_symbol>   ::= f<name>
<pred>        ::= p<name>
```

# Example
## Command
```
cat options/here_unification options/support_policy |./build/bin/fol_prover < remade_teorems/custom0.p 
```
## Output
```
Choose unification algorithm:
[1] Robinson unification
[2] Here unification
[3] Martelli-Montanari unification
Choose clause choosing policy:
[1] Saturation policy
[2] Short precedence policy
[3] Strikeout policy
[4] Support policy
Enter axioms' number: Error in parsing at position 0 '': Unhandled variant in unary parsing
Axiom: (@ vX . ~(pE(vX) and ~pV(vX)) or ((? vY . pS(vX, vY) and pC(vY))))
@ vX . ? vu1 . (((~(pE(vX))) or (pV(vX))) or (pS(vX, vu1))) and (((~(pE(vX))) or (pV(vX))) or (pC(vu1)))
Axiom: (? vX . pP(vX) and pE(vX) and ((@ vY . ~pS(vX, vY) or pP(vY))))
? vX . @ vu3 . (pP(vX)) and ((~(pS(vX, vu3))) or (pP(vu3))) and pE(vX)
Axiom: (@ vX . ~pP(vX) or ~pV(vX))
@ vX . ~pP(vX) or ~(pV(vX))
Enter hypothesis: @ vX . ~pP(vX) or ~(pC(vX))
[1] pS(vX, funiq0(vX)) or pV(vX) or ~pE(vX)
[2] pC(funiq0(vX)) or pV(vX) or ~pE(vX)
[3] pP(funiq1(cEMPTY))
[4] pP(vu3) or ~pS(funiq1(cEMPTY), vu3)
[5] pE(funiq1(cEMPTY))
[6] ~pP(vX) or ~pV(vX)
[7] ~pC(vX) or ~pP(vX)
Get clause: ~pC(vX) or ~pP(vX)
Resolution: ~pC(vX) or ~pP(vX) RESOLVE pP(funiq1(cEMPTY)) >>> ~pC(funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vX} ]
Resolution: ~pC(vX) or ~pP(vX) RESOLVE pP(vu3) or ~pS(funiq1(cEMPTY), vu3) >>> ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3)
Sub: [ {vu3/vX} ]
Get clause: ~pC(funiq1(cEMPTY))
Get clause: ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3)
Resolution: ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3) RESOLVE pS(vX, funiq0(vX)) or pV(vX) or ~pE(vX) >>> pV(funiq1(cEMPTY)) or ~pC(funiq0(funiq1(cEMPTY))) or ~pE(funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vX} {funiq0(funiq1(cEMPTY))/vu3} ]
Choose unification algorithm:
[1] Robinson unification
[2] Here unification
[3] Martelli-Montanari unification
Choose clause choosing policy:
[1] Saturation policy
[2] Short precedence policy
[3] Strikeout policy
[4] Support policy
Enter axioms' number: Error in parsing at position 0 '': Unhandled variant in unary parsing
Axiom: (@ vX . ~(pE(vX) and ~pV(vX)) or ((? vY . pS(vX, vY) and pC(vY))))
@ vX . ? vu1 . (((~(pE(vX))) or (pV(vX))) or (pS(vX, vu1))) and (((~(pE(vX))) or (pV(vX))) or (pC(vu1)))
Axiom: (? vX . pP(vX) and pE(vX) and ((@ vY . ~pS(vX, vY) or pP(vY))))
? vX . @ vu3 . (pP(vX)) and ((~(pS(vX, vu3))) or (pP(vu3))) and pE(vX)
Axiom: (@ vX . ~pP(vX) or ~pV(vX))
@ vX . ~pP(vX) or ~(pV(vX))
Enter hypothesis: @ vX . ~pP(vX) or ~(pC(vX))
[1] pS(vX, funiq0(vX)) or pV(vX) or ~pE(vX)
[2] pC(funiq0(vX)) or pV(vX) or ~pE(vX)
[3] pP(funiq1(cEMPTY))
[4] pP(vu3) or ~pS(funiq1(cEMPTY), vu3)
[5] pE(funiq1(cEMPTY))
[6] ~pP(vX) or ~pV(vX)
[7] ~pC(vX) or ~pP(vX)
Get clause: ~pC(vX) or ~pP(vX)
Resolution: ~pC(vX) or ~pP(vX) RESOLVE pP(funiq1(cEMPTY)) >>> ~pC(funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vX} ]
Resolution: ~pC(vX) or ~pP(vX) RESOLVE pP(vu3) or ~pS(funiq1(cEMPTY), vu3) >>> ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3)
Sub: [ {vu3/vX} ]
Get clause: ~pC(funiq1(cEMPTY))
Get clause: ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3)
Resolution: ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3) RESOLVE pS(vX, funiq0(vX)) or pV(vX) or ~pE(vX) >>> pV(funiq1(cEMPTY)) or ~pC(funiq0(funiq1(cEMPTY))) or ~pE(funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vX} {funiq0(funiq1(cEMPTY))/vu3} ]
...
Resolution: ~pE(vu3) or ~pS(funiq1(cEMPTY), funiq0(vu3)) or ~pS(funiq1(cEMPTY), vu3) RESOLVE pE(funiq1(cEMPTY)) >>> ~pS(funiq1(cEMPTY), funiq0(funiq1(cEMPTY))) or ~pS(funiq1(cEMPTY), funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vu3} ]
Get clause: ~pP(funiq1(cEMPTY))
Resolution: ~pP(funiq1(cEMPTY)) RESOLVE pP(funiq1(cEMPTY)) >>> EMPTY
Sub: [ ]
Resolution: ~pP(funiq1(cEMPTY)) RESOLVE pP(vu3) or ~pS(funiq1(cEMPTY), vu3) >>> ~pS(funiq1(cEMPTY), funiq1(cEMPTY))
Sub: [ {funiq1(cEMPTY)/vu3} ]
[1] pS(vX, funiq0(vX)) or pV(vX) or ~pE(vX)[ AXIOM ]
[2] pC(funiq0(vX)) or pV(vX) or ~pE(vX)[ AXIOM ]
[3] pP(funiq1(cEMPTY))[ AXIOM ]
[4] pP(vu3) or ~pS(funiq1(cEMPTY), vu3)[ AXIOM ]
[5] pE(funiq1(cEMPTY))[ AXIOM ]
[6] ~pP(vX) or ~pV(vX)[ AXIOM ]
[7] ~pC(vX) or ~pP(vX)[ AXIOM ]
[9] ~pC(vu3) or ~pS(funiq1(cEMPTY), vu3)[ 7 4 ]
[10] pV(funiq1(cEMPTY)) or ~pC(funiq0(funiq1(cEMPTY))) or ~pE(funiq1(cEMPTY))[ 9 1 ]
[12] pV(funiq1(cEMPTY)) or ~pE(funiq1(cEMPTY))[ 10 2 ]
[18] pV(funiq1(cEMPTY))[ 12 5 ]
[32] ~pP(funiq1(cEMPTY))[ 18 6 ]
[54] EMPTY[ 32 3 ]
```
