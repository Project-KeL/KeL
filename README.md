# KeL
KeL (for **Keys & Locks**) is intended to be a low-level programming language with a compiler customizable enough to support as much binary format as possible and dumb enough to provide as less optimization as possible. However, these concepts are intended to counterbalance to provide a 'user-friendly' experience.

> [!NOTE]
> This project is a work in progress and the master branch is anything but stable at this moment. The syntax may evolve but the main ideas are here.
>
> It does not compile sources yet, but you can try `./kel <source_file.kl>` to get debugging informations about tokenization and parsing.

## "Hello, world"
```
imod sys;

[entry] #main scope:
    sys.out `Hello, world!`;
    sys.exit SUCCESS;
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
The central symbol in KeL is the colon. The colon is "blank-sensitive" meaning the characters surrounding the colon (including non-glyph characters) are significant. To simplify, a word right before a colon is called a _key_ and a word right after a _lock_.

Beside the colon symbol there are two important symbols called _commands_.
1. `#` refers to declarations or other actions to be executed at compile-time.
2. `@` is the equivalent at run-time.

Commands also escape the blank-sensitiveness of the colon depending on their position.

#### Identification
Keys can be used as identifiers (like constants and variables). In this configuration, the lock sets the type partially.

Let's say the target of the compiler is an architecture where we can define 32-bit unsigned integers and let's say we had encapsulated this concept in the `u32` lock.
```
@var1:u32 1; !-- `var` is initialized as a run-time constant 32-bit unsigned integer and is set to `1`.
@var2 :u32 1; !-- The `@` escapes the blank-sensitiveness before the colon.
[mut] @var3 :u32; !-- `var3` is declared mutable.
[mut] @var4 :u32 var2; !-- `var4` is initialized with the value of `var2` so `1`.
@var5 var1; !-- The type is deduced.
```

After the _identification_ of a key (declaration or initialization), it can be reassigned.
```
@var :u32 1;
var = 2;
```
