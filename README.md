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
At this moment, a key is a constant, a variable, a function, brackets for arrays or operators to manipulate addresses called _leveling operators_.

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
The `&` operator (level up) is used to get the address of a variable and the `|` operator (level down) is used to dereference a variable; these are the leveling symbols. They must be placed after variables. The blanks are important here because these symbols are considered as keys in the following declarations.
```
@var :u32 1;
@ptr &:u32 var&; !-- `ptr` points to `var`.
ptr| = 2; !-- `ptr` is dereferenced and `var` is set to `2`.
@als |:u32 ptr|; !-- `als` is `var` as a dereferencing of `ptr`.
als = 3; !-- `var` is set to `3`.
```

References and dereferences must be initialized and only with a variable and are binded with it during its lifetime (its scope), but pointers and aliases may be uninitialized. Pointers and references represent the same concept with a different syntax, the same goes for aliases and dereferences.
```
@var :u32;
@addr &:u32;
!-- These variables process addresses. Both can be dereferenced.
@ptr &:u32 var&; !-- pointer
@ref :u32| var; !-- reference (to `var`).
!-- These variables process values. Both cannot be dereferenced.
@als |:u32 addr|; !-- alias (of `var`).
@der :u32& addr; !-- dereference (from `addr`)
```

When a key has a type (because the lock represent a type like `u32`), the determination of it follows this order: first, the _marker operators_ (leveling operators and brackets) at the left of the lock are processed, then the leveling operators at the right of the lock and finally, the lock.
```
@var :u32;
@var2 :u32|& var; !-- `var2` is `var` (dereferenced reference).
@var3 |:u32| ptr|; !-- `var3` is processed as an alias of the variable at the address held by `ptr` and reference it.
@ptr &:u32 var&;
var2 = 1; !-- `var` is set to `1`.
var3| = 2; !-- `var` is set to `2`.
```

This is some examples to understand type compatibility.
```
@  var1 |&&|&:u32;
@  buf1 |&&|&    &&|:u32 var1&&|; !-- The marker operators are applied left to right.
@thing1 |&&|&:u32&&| buf1;

@  var2         :u32;
@  buf2    |&&|&    &&|:u32 var2|&&|&&&|;
@thing2    |&&|&:u32&&| buf2;

@  var3     |:u32;
@  buf3 &&&|&    |:u32 var3&&&&|&| !-- extra level up at start to cancel out the `|`.
@thing3 |&&|&:u32| |&&|&&var3; !-- Level up once more to cancel out the `|`.
```

If the marker operators were placed before a variable and read left to right, it would require to reverse the operators. Furthermore, blanks can be used while we are not right before or after a colon so it would be ambiguous.

Marker operators cancels out when applied at a variable. For example, `var&&|` is the same as `var&`.

`[mut]` is applied to the type pointed to.
```
[mut] @var :u32;
@ptr1 &:u32 var&; !-- `ptr1` points to a `var` but `var` cannot be assigned.
[mut] @ptr2 &:u32 var&; !-- `ptr2` points to `var` and can be assigned.
```

To declare constant pointer, aliases, references and dereferences, just replace `&` by the `+` operator and `|` by the `-` operator.
```
@var :u32;
@ptr +:u32 var&; !-- Constant pointer to the constant variable `var`.
```

Besides the constness, the usage is the same.

#### Arrays

The precedence rule to process a type established before makes more sense for arrays.
```
@var1 :u32;
@var2 :u32;
@var3 :u32;

@arr []:u32 [var1, var2, var3]; !-- Compile-time array of constant 32-bit unsigned integers initialized with the previous variables.
@ders1 []:u32|& [var1, var2, var3]; !-- The variables are used as dereferenced references.
@ders2 :[]u32|& [var1, var2, var3]; !-- This is equivalent, the lock is `u32`. What matters is what is at the left and at the right of it.

arr[0] = 1; !-- `var1` is set to `1`.

@ptr &:u32 arr; !-- Array types are compatible with left level up.
(ptr + 1)| = 2; !-- `var2` is set to `2`.
@ref :u32| arr; !-- Right level down too but only the first element of the array will be accessible.
```

An array is always constant.

#### Freezing

The mutability of a variable can be frozen, but the constness of a variable cannot be undone. Freezing a lock freeze the marker operators following it.
```
[mut] @var1 :u32 2;
:[frz] var1; !-- Freeze the lock. Now var is a constant `u32`.

@var2 &&:u32;
[frz] var2 +&:u32; !-- Freeze the mutability of the first level.

@buf             &|:u32 var1; !-- `&|var1` cancels out.
[mut] @var3 &:u32&| buf&;
[frz]:[frz] var3 +:u32; !-- `var3` now is constant and has the type `+:u32+-`.
```

#### Cast
```
@unsigned :u32 1;
@signed1 :i32 unsigned:i32; !-- Cast `unsigned` to a 32-bit signed integer.
@signed2 unsigned:i32; !-- The type of `signed2` is implicit.
```

A function can be overloaded, the counterpart is the type must be deducible.
