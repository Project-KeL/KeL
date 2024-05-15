# KeL
KeL (for **Keys & Locks**) is intended to be a low-level programming language with a very straightforward syntax.

> ℹ️
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
A single line comment begins with `!--` and a multiline comment is surrounded by `|--` and `--|`.

```
!-- This is a single line comment.

|--
 - This is another comment
 - using more than one line.
--|
```

## Keys and locks
The central symbol in KeL is the colon. The colon is "blank-sensitive" meaning the characters surrounding the colon (including non-glyph characters) are significant. A word right before a colon is called a _key_ and a word right after a _lock_.

Beside the colon symbol there are two important symbols called _commands_.
1. `#` refers to declarations or other actions to be executed at compile-time.
2. `@` is the equivalent at run-time.

Commands also escape the blank-sensitiveness of the colon depending on their position.

### Keys
At this moment, a key is a constant, a variable, a function, brackets for arrays  or symbols relative to pointers called _leveling symbols_.

#### Constants and variable
```
@var1:u32 1; !-- `var` is declared as a run-time constant 32-bit unsigned integer and is set to `1`.
@var2 :u32 1; !-- The `@` escapes the blank-sensitiveness before the colon.
[mut] @var3 :u32; !-- `var3` is mutable and uninitialized.
[mut] @var4 :u32 var2; !-- `var4` is initialized with the value of `var2` so `1`.
@var5 var1; !-- The type is deduced.
```

After its declaration, a variable can be assigned as usual.
```
@var :u32 1;
var = 2;
```

#### Functions
```
!-- This function returns a compile-time 32-bit signed integer equal `x - 1` with `x` its parameter.
@fn1 :i32(x :u32) scope:
    .ret x - 1;
.
!-- The type returned by the function can be specified before as another lock when ``` is used. `fn2` is set to `fn1`.
@fn2 :i32 :`(x :u32) fn1;
```

##### Call
The syntax for calling a macro is different than the one to call a function.
```
#inc :u32(x :u32) scope:
    .ret x + 1;
.

inc: 41;
inc 41; !-- only possible because `macro` has one parameter.
inc#(41); !-- function style

#add :u32(a :u32, b :u32) scope:
    .ret a + b;
.

add: 23, 19;
add#(23, 19);

@dec :i32(x :i32) scope:
    .ret x - 1;
.

dec(43);
dec@ 43 !-- macro style
dec@: 43;

@sub (a :i32, b :i32) scope:
    .ret a - b;
.

dec(51, 19);
dec@: 51, 19; !-- macro style
```

#### Pointers and aliases, references and dereferences
The `&` symbol is used to get the address of a variable and the `|` symbol is used to dereference a variable; these are the leveling symbols. The blanks are important here because these symbols are considered as keys in the following declarations.
```
@var :u32 1;
@ptr &:u32 &var; !-- `ptr` points to `var`.
|ptr = 2; !-- `ptr` is dereferenced and `var` is set to `2`.
@als |:u32 |ptr; !-- `als` is a dereferenced pointer.
als = 3; !-- `var` is set to `3`.
```

References and dereferences must be initialized and only with a variable, but pointers and aliases may be uninitialized. Pointers and references represent the same concept with a different syntax, the same goes for aliases and dereferences.
```
@var :u32;
@addr &:u32;
!-- These variables process addresses. Both can be dereferenced.
@ptr &:u32 &var; !-- pointer
@ref :u32| var; !-- reference (to `var`).
!-- These variables process values. Both cannot be dereferenced.
@als |:u32 |addr; !-- alias (from `addr`).
@der :u32& addr; !-- dereference
```

To say the least, a reference is a pointer that does not need the `&` symbol to get the address when we initialize or when we assign it. Dereferences are similar, it is a pointer that does not need the `|` symbol. The key precede the lock in the process of the type when the lock does not contain symbols relative to pointers. When the lock contains this kind of symbols, these symbols are processed first, then the key and the rest of the lock.
```
@var :u32;
@var2 :u32|& var; !-- `var2` is `var` (dereferenced reference).
@var3 |:u32| var; !-- error: `var` is referenced but an alias wait for a dereference symbol `|`.
```

In fact, instructions like the third lead to errors. The precedence rule established before makes sense for arrays.

`[mut]` is applied to the type pointed to.
```
[mut] @var :u32;
@ptr1 &:u32 var; !-- `ptr1` points to a `var` but `var` cannot be assigned.
[mut] @ptr2 &:u32 var; !-- `ptr2` points to `var` and can be assigned.
```

To declare constant pointer, aliases, references and dereferences, just replace `&` by `+` and `|` by `-`.
```
@var :u32;
@ptr +:u32 var; !-- Constant pointer to the constant variable `var`.
```

Besides the constness, the usage is the same.

#### Arrays
```
@var1 :u32;
@var2 :u32;
@var3 :u32;

@arr []:u32 [var1, var2, var3]; !-- Compile-time array of constant 32-bit unsigned integers initialized with the previous variables.
@ders []:u32|& [var1, var2, var3]; !-- The variables are used as dereferenced references.
```

#### Cast
```
@unsigned :u32 1;
@signed1 :i32 unsigned:i32; !-- Cast `unsigned` to a 32-bit signed integer.
@signed2 unsigned:i32; !-- The type of `signed2` is implicit.
```

A function can be overloaded, the counterpart is the type must be deducible.
