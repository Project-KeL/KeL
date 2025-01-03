# KeL
KeL (for **Keys & Locks**) is intended to be a low-level programming language with a compiler customizable enough to support as much binary formats as possible and dumb enough to provide as less optimization as possible. However, these concepts are intended to counterbalance to give a 'user-friendly' experience.

> [!NOTE]
> This project is a work in progress and the master branch is anything but stable at this moment. The syntax may evolve but the main ideas are here.
>
> It does not compile sources yet, but you can try `./kel <source_file.kl>` to get debugging informations about tokenization and parsing.

## Hello, world!
```
imod sys;

[entry] #main scope
    sys.out(`Hello, world!`);
    sys.exit(SUCCESS);
.
```

## Comments
A single line comment begins with `!--` and a multiline comment is surrounded by `|--` and `--|` (there is no decrementation operator anyway).

```
!-- This is a single line comment.

|--
 - This is another comment
 - using more than one line.
--|
```

## Keys and locks
The central symbol in KeL is `:`. It is 'blank-sensitive' meaning the characters surrounding it (including non-glyph characters) are significant. To simplify, a word right before `:` is called a _key_ and a word right after a _lock_. Everything is a key by default so a key alone does not need `:` right after it. The `.` is right blank-sensitive. These symbols are the only ones to have this property.

There are three other important symbols called _commands_.
1. `#` refers to declarations or other actions to be executed at compile-time.
2. `@` is the equivalent at run-time.
3. `!` means something has to be resolved at most during the binary generation.

Commands also escape the blank-sensitiveness of the `:` depending on their position.

A scope is delimited by `scope` and `.` where the `.` is followed by a blank. An instruction ends with a `;` but the last instruction of a scope may ends with a `.`.

Keys and locks are processed separately. This means a key may have the same name of a lock.

#### Identification
Keys can be used as identifiers (like constants and variables). In this configuration, the lock sets the type partially.

Let's say the target of the compiler is an architecture where we can define 32-bit unsigned integers and let's say we had encapsulated this concept in the `u32` lock.
```
@var1:u32 1;      !-- var is set to 1.
@var2 :u32 1;     !-- The `@` escapes the left blank-sensitiveness.
[mut] @var3 :u32; !-- var3 is declared mutable.
@var4 :u32 var2;  !-- var4 is initialized with the value of var2.
@var5 var1;       !-- The type is deduced.

var3 = 3;         !-- var3 is set to 3.
```

Keys can also be used as _parameterized labels_ (calling conventions are part of the compiler 'customizability'). A label has the type `scope` which is a key and a lock at the same time, and parameterized labels can be initialized with a `scope`. Let `A` and `B` be locks.
```
!-- Declaration of a parameterized label returning a B taking a A.
@lab_x :B(x :A);

!-- A label.
#lab1 :scope;

!-- This is a declaration of an entry point.
[entry] #main :scope;
!-- Because main is not initialized, the execution continues here.

!-- This is an initialization but the type is deducible.
#lab2 :scope scope: !-- #lab2 scope: would be right
    !-- code
.

!-- b takes no parameter and returns a B.
!-- c takes a parameterized label taking a A returning nothing and returns a B.
@lab3 :(a :A, b :B(), c :B(:(:A)));
!-- c can be rewrote c :B(L :(x :A)) because parameters are ignored after the first nesting level.

!-- lab4 returns a B (alternative syntax).
#lab4 :_(a :A) :B

!-- lab5 returns a parameterized label returning a B taking a A.
#lab5 :_(a :A, b :B) :B(a :A);
```

- [ ] Lexer (may evolve)
    - [x] Commands
    - [x] Qualifiers
    - [x] Scopes
    - [x] Keys
    - [x] Locks
    - [x] Literals
    - [ ] R special symbols (partial support)
    - [x] Identifiers
- [ ] Parser
    - [x] Modules
        - [x] Module
        - [x] Submodule
    - [ ] Types
        - [x] Lock alone
        - [x] Parameterized label
        - [ ] R underscore syntax
        - [ ] L and R modifiers (arrays, pointers, ...)
    - [ ] Identification
        - [x] Without type deduction
        - [ ] With type deduction
    - [ ] Initialization (work in progress)
        - [x] With literals
        - [x] With scopes
        - [ ] With an expression
    - [ ] Parameterized labels calls
    - [ ] Expressions
    - [ ] Conditions and loops
- [ ] Modules
- [ ] Generator
    - [ ] Built-in macros to write in the output

_NB: "./binary" and "./linker" are just placeholders._
