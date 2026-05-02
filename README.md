# Symbols

> [WIP]

Symbolic Computation Educational Project.

# Lexical Scanner

This is the NFA of the lexical scanner.

![DFA](./doc/NFA.drawio.svg)


# Parser

BNF:
```
expr   ::= expr + term
        |  expr - term
        |  term

term   ::= term * factor
        |  term / factor
        |  factor

factor := digit 
        | symbol
        | left_parentheses expr right_parentheses
```

LL(1) compatible grammar:

```
E  ::= T E'
E' ::= + T E' | - T E' | e
T  ::= F T'
T' ::= * F T' | / F T' | e
F  ::= (E) | SYMBOL | NUM
```
