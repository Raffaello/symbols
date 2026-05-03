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
stmt      ::= assigment | expr

assigment ::= symbol = expr

expr      ::= expr + term
           |  expr - term
           |  term

term      ::= term * factor
           |  term / factor
           <!-- |  term = factor -->
           |  factor

factor    ::= unary predicate

unary     :: + | - | epsilon

predicate ::= digit
           | symbol
           | ( expr )
```

LL(1) compatible grammar:

```ebnf
E  ::= T E'
E' ::= + T E' | - T E' | = T E'| e
T  ::= F T'
T' ::= * F T' | / F T' | e
F  ::= U P
U  ::= + | - | e
P  ::= (E) | SYMBOL | NUM
```

# Interpreter

> TODO
