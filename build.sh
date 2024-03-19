#!/bin/sh
gcc main.c -o assemble.exe -Wl,--subsystem=3 -luser32 -lmsvcrt -nostdlib -L/cygdrive/c/Windows/System32 -Wl,--image-base=0x400000 -Wl,-e_start -Os
strip assemble.exe
