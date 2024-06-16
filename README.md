# DonutOS
I wrote an OS to print the donut

## About
So this is something I started working on because I wanted to know how the hell do you even approach writing an operating system, the details of how everything works and stuff.

The main goal was to just write an OS that was able to run the [donut.c code](https://www.a1k0n.net/2021/01/13/optimizing-donut.html) (hence the name), but after having to implement so much stuff just to make that possible, it started becoming an actual thing, which made me want to keep developing on it. 

Right now, there is a college project that is just too perfect not to use this OS as a starting point, so it just keeps on growing.. all hail the DonutOS!

### Current state
- Runs in 32 bit protected mode
- Has a working IDT
- Blue screen exception handler
- Keyboard driver
- PIT (timer) driver
- TTY prompt
- Spins the donut

## Dependencies
I used a [tutorial](https://youtube.com/playlist?list=PLm3B56ql_akNcvH8vvJRYOc7TbYhRs19M&si=2bToaSQWffHsuESF) as a starting point, which explained everything I needed to know up to getting some C going. After that, it was a **lot** of reading, especially [osdev](https://wiki.osdev.org/Expanded_Main_Page). The thing is, that tutorial had a [shell script](https://github.com/mell-o-tron/MellOs/blob/main/A_Setup/setup-gcc-debian.sh) for installing all dependencies. The only dependency that is a pain to install would be the cross compiler, so the reason I recommend running the script is just to get that going, the other stuff (qemu, nasm) are way easier to install.

Still talking about the cross compiler, we also tried using regular gcc instead and it works.. to a certain extent. While gcc does have flags that allow you to set the target machine that you want it to compile to, as well as flags to not use system dependencies, some things still bug out. For instance, if you try to divide by 0 to force an exception (which this OS will throw a blue screen), it will be caught by the cross compiler and it'll call the exception handler, but regular gcc will just ignores it and nothing happens. With that said, everything else seemed to work just fine with gcc instead of the cross compiler but it's obviously not reliable. Use the cross compiler if you can, and gcc only as a second option (the makefile should be handling already).


**TL;DR**

you'll need:
- nasm
- qemu
- gcc cross compiler

## How to run
### Linux:
Just open the project's root folder and 
- `make all` to compile it
- `make run` to compile and run it
- `make clean` to delete the binaries

### Windows:
Good luck!

## TO DO
- Process scheduling
