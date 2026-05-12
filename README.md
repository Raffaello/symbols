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
stmt      ::= stmt' END | stmt' , stmt
stmt'      ::= expr | equation

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

Basic interpreter evaluating a single `AST` at time for now.

Not supporting solving equation, but only if those are reduced to what is considered an assignment, e.g:
 `x=1`, but not `x+1=2`

 > TODO more?

# Solver

Just a basic polynomial solver at the moment for quadratic polynomials.

> TODO: all polynomials, rational equation, etc..


# REPL

The REPL is the program using all other components.

Switch among interpreter and solver, with `:eval` and `:solver` keywords.

it support the `,` comma operator to have multiple lines into one: for the eval each statement after the comma is one expression or assignment to be solved, e.g.:

```shell
$eval> a=1, 1+a
|> a = 1
|> 1 + 1 = 2
```

for the solver is an equation to solve for the 2nd argument
```shell
x+1=0, x, y-1=0, y
|> x = -1
|> y = 1
```

the solver can do basic symbol substitution if those are defined (must be defined in the interpreter, `$eval>` shell):
```shell
$eval> a=1
|> a = 1
$eval> :solver
$solver> x+a=1, x
$|> x = 0

```

NOTE: if the symbol is defined but it will be solve for it, its numerical value will be ignored.

> TODO: solver should be able to define and assign values to symbols.
> TODO: should solve for undefined symbol returning the generic symbol as a result, e.g: `x+1=a, x => x = a-1`
> TODO: .... and so on
