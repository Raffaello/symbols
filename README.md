# Symbols

> [WIP]

Symbolic Computation Educational Project.

# Scanner

TODO: Create the DFA

![DFA](./doc/NFA.drawio.svg)

Tokens:
- num
- symbol
- sum_op
- mul_op
- left_parentheses
- right_parentheses

# Parser

BNF:

expr   ::= expr + term
        |  expr - term
        |  term

term   ::= term * factor
        |  term / factor
        |  factor

factor := digit 
        | symbol
        | left_parentheses expr right_parentheses

---

E  ::= T E'
E' ::= + T E' | - T E' | e
T  ::= F T'
T' ::= * F T' | / F T' | e
F  ::= (E) | SYMBOL | NUM


