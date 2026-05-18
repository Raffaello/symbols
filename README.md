# Symbols

> [WIP]

Symbolic Computation Educational Project.

## Dependencies

- Boost
- GMP
- MPFR

Used for rational number computation.

# Lexical Scanner

> TODO: not completed yet

This is the NFA of the lexical scanner.

![DFA](./doc/NFA.drawio.svg)


# Parser

> TODO not completed yet
> TODO add rational number to be parsed as whole rational numbers (e.g. 1/2) alongside real numbers (e.g 0.5) ? (so it can output the same as user input eventually)

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
The evaluation is performed through rational numbers.

> TODO give a choice to display also real solutions?

Not supporting solving equation, but only if those are reduced to what is considered an assignment, e.g:
 `x=1`, but not `x+1=2`

 > TODO more?
 > TODO replace assignment `=` with `:=` specific for the assignment instead?

# Solver

Just a basic polynomial solver at the moment for cubic or lower polynomials.
The evaluations is performed through rational numbers,
to solve the equation it might switch to real values

> TODO: all polynomials, rational equation, etc..
> TODO: Simplify expression, e.g.: constant must be resolved numerically, etc..


# REPL

The REPL is the program using all other components.

Switch among interpreter and solver, with `:eval` and `:solver` keywords.

it supports the `,` comma operator for multiple statements in one line: in eval mode, each comma-separated statement is evaluated, e.g:

```shell
$eval> a=1, 1+a
|> a = 1
|> 1 + 1 = 2
```

In solver mode, each pair is: equation, symbol-to-solve-for

```shell
$solver> x+1=0, x, y-1=0, y
|> x = -1
|> y = 1
```

the solver can do basic symbol substitution if those are defined (must be defined in the interpreter, `$eval>` shell):
```shell
$eval> a=1
|> a = 1
$eval> :solver
$solver> x+a=1, x
|> x = 0

```

NOTE: if the symbol is defined but it will be solve for, its numerical value will be ignored.

> TODO: add complex numbers and solutions
> TODO: Gröbner bases ?
> TODO: solver should be able to define and assign values to symbols.
> TODO: should solve for undefined symbol returning the generic symbol as a result, e.g: `x+1=a, x => x = a-1`
> TODO: .... and so on
