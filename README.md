# KeL
KeL (for **Keys & Locks**) is intended to be a low-level programming language with a very straightforward syntax.

> **Note**
> This project is a work in progress and the master branch is anything but stable at this moment. The syntax may evolve but the main ideas are here.

## "Hello, world"
```
imod system;

[entry] #main scope:
    system.out `Hello, world!`;
    system.exit SUCCESS;
.
```

## Comments
A single line comment begins with `--` and a multiline comment is surrounded by `|--` and `--|`.

```
-- This is a single line comment.

|--
 - This is another comment
 - using more than one line.
--|
```

## Keys and locks
The central symbol in KeL is the colon. The colon is "blank sensitive" to say the characters surrounding the colon (including non-glyph characters) are significant. A word right before a colon is called a _key_ and a word right after a _lock_.

Beside the colon symbol there are two important symbols called _commands_.
1. `#` referes to declarations or other actions to be executed at compile-time.
2. `@` is the equivalent for run-time.

Commands also escape the blank sensitiveness of the colon depending on their position.

### Keys
At this moment, a key is a constant, a variable, a function or symbols relative to pointers.

#### Constants and variable
```
@var1:u32 1; -- `var` is declared as a run-time constant 32-bits unsigned integer and is set to `1`.
@var2 :u32 1; -- The `@` escapes the blank sensitiveness before the colon.
[mut] @var3 :u32; -- `var3` is mutable and uninitialized.
[mut] @var4 :u32 var2; -- `var4` is initialized with the value of `var2` so `1`.
@var5 var1; -- The type is deduced.
var4 = 2; -- `var4` is set to `2`.
```

#### Functions
```
-- This function returns a compile-time 32-bits signed integer equals `x - 1` with `x` its parameter.
@fn1 :i32(x :u32) scope:
    .ret x - 1;
.
-- The type returned by the function can be specified before as another key when `` ` `` is used. `fn2` is set to `fn1`.
@fn2 :i32 :`(x :u32) fn1;
```

#### Pointer and alias, references and dereferences
The `&` symbol is used to get the address of a variable and the `|` symbol is used to dereference a variable. The blanks are important here because this symbols are considered as keys in the following declarations.
```
@var :u32 1;
@ptr &:u32 &var; -- `ptr` points to `var`.
|ptr = 2; -- `ptr` is dereferenced and `var` is set to `2`.
@der |:u32 |ptr; -- `der` is a dereferenced pointer.
der = 3; -- `var` is set to `3`.
```

Some of these declarations have equivalent types.
```
@var :u32;
@addr &:u32;
-- These are equivalent.
@ptr &:u32 &var; -- pointer
@ref :u32| var; -- reference
-- These are equivalent.
@als |:u32 |ptr; -- alias
@der :u32& ptr; -- dereference
```

`[mut]` is applied to the type pointed to.
```
[mut] @var :u32;
@ptr1 &:u32 var; -- `ptr1` points to a constant.
[mut] @ptr2 &:u32 var; -- `ptr2` points to a mutable.
```

#### Cast
```
@unsigned :u32 1;
@signed1 :i32 unsigned:i32; -- Cast `unsigned` to a 32-bit signed integer.
@signed2 unsigned:i32; -- The type of `signed2` is implicit.
```

A function can be overloaded, the counterpart is the type must be deductible.
