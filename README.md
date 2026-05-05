# Symbols

> [WIP]

Symbolic Computation Educational Project.

# Lexical Scanner

> TODO: not completed yet

This is the NFA of the lexical scanner.

![DFA](./doc/NFA.drawio.svg)


# Parser

> TODO not completed yet

The Grammar for the arithmetical expression is the following, it doesn't allow + or - chains operator like in C (`+-+-1` is valid in C, but i prefer a more mathematical approach so a user must write: `+(-(+(-1)))` eventually.

EBNF:

```ebnf
stmt      ::= expr END | equation END

equation ::= expr = expr

expr      ::= expr + term
           |  expr - term
           |  term

term      ::= term * factor
           |  term / factor
           |  factor

factor    ::= unary predicate

unary     :: + | - | epsilon

predicate ::= digit
           | symbol
           | ( expr )
```

LL(1) compatible grammar:

```ebnf
S    ::= S' END
S'   ::= E | E = E
E    ::= T E'
E'   ::= + T E' | - T E' | e
T    ::= F T'
T'   ::= * F T' | / F T' | e
F    ::= U POW
U    ::= + | - | e
POW  ::= P POW'
POW' ::= ^ F | e
P    ::= (E) | SYMBOL | NUM
```

# Interpreter

> TODO

Basic interpreter evaluating a single `AST` at time for now.

Not supporting solving equation, but only if those are reduced to what is considered an assignment, e.g:
 `x=1`, but not `x+1=2`
