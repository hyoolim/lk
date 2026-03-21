# Link Programming Language

Link (`.lk`) is a systems language currently in early development. It is dynamically typed and prototype-based today, with class-based OOP and static typing planned. The current implementation is a tree-walking interpreter written in C, with a compiler as the next milestone.

## Building

Requires CMake and Ninja.

```sh
make        # configure + build
make test   # run tests
make stdlib # build standard library modules
```

The binary is at `build/debug/lk`.

## Usage

```sh
build/debug/lk -l stdlib script.lk
```

## Syntax

### Variables

`:=` defines and assigns. `=` reassigns.

```
x := 10
x = 20
```

### Method calls

Method calls use `.`:

```
'hello'.toUpperCase
List.new.push!['a', 'b', 'c'].size
```

A leading `.` sends to the implicit receiver (self):

```
.size := 1
.also(Comparable)
```

Low-precedence chaining uses `->`, typically for trailing blocks:

```
Directory.new['..'].directories -> println
[1, 2, 3].each { v | v * 2 } -> println
```

### Blocks

`{ params | body }` for one-liners, `{ params --- body }` for multi-line:

```
double := { x | x * 2 }

add := { x, y ---
    x + y
}
```

Varargs use `...`:

```
func := { x, rest... --- rest }
func(1, 2, 3)   # [ 2, 3 ]
```

Closures capture their enclosing scope:

```
maker := { x --- { x += 1 } }
c := maker(0)
c   # 1
c   # 2
```

### Conditionals

`?` and `!` are the if/else operators:

```
x > 0 ? 'positive' ! 'non-positive'
```

### Operators

```
++      # string/list concatenation
**      # repeat  ('x' ** 3 = 'xxx', [1,2] ** 2 = [1,2,1,2])
$       # to string  ($[1,2,3] = '1 2 3 ')
!       # logical not
&&  ||  # logical and/or
<=>     # comparison (-1, 0, 1)
```

### Objects

Objects are prototype-based. `new` clones an object, `extend` adds slots in bulk, `also` mixes in another object:

```
Animal := Object.new
Animal.sound := 'generic'
Animal.speak := { .sound.println }

Dog := Animal.new
Dog.sound := 'woof'
Dog.speak   # 'woof'
```

```
Animal.extend {
    .sound := 'generic'
    .speak := { .sound.println }
}
```

### Lists and Maps

```
list := [1, 2, 3]
list(0)         # 1
list.size       # 3
list.each { v | v.println }

map := Map.new
map.set('key', 'value')
map.at('key')   # 'value'
```

### Including files

```
include('path/to/file.lk')
```

## Project layout

```
src/        C source files
include/    C header files
stdlib/     Link standard library (.lk)
tests/      Test suite
extra/      Extensions and examples
```

## Status

Early development. Expect breaking changes.
