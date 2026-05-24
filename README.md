# KeL
KeL (for **Keys & Locks**) is intended to be a low-level programming language with a compiler customizable enough to support as much binary formats as possible and dumb enough to provide as less optimization as possible. However, these concepts are intended to counterbalance to give a 'user-friendly' experience.

> [!NOTE]
> This project is a work in progress and the master branch is anything but stable at this moment. The syntax may evolve but the main ideas are here.
>
> It does not compile sources yet, but you can try `./kel <source_file.kl>` to get debugging informations about tokenization and parsing.

## Hello, world!
```
imod sys;

[entry] @main:() scope
    sys..out(`Hello, world!`);
    sys..exit(.SUCCESS);
.
```

_NB: "./binary" and "./linker" are just placeholders._
