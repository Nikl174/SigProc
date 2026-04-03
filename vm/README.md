# Super simple implementation of a stack-based virtual-machine

- a simplified version of a stack-machine, similar priciple used in WASM
- from [tsodings video](https://www.youtube.com/watch?v=2qV-1JhxWeE)

```bash
# compile
gcc -o stack_machine.out stack_machine.c

# start and run VM with programm initial programm
./stack_machine.out

# look into hexcode output of programm run in VM
xxd programm.bin

#         _________ First instruction Enum (INST_PUSH)
#                   _________ Operant for first instruction in little endian (20)
#                                       _________ Second Operant for second operation (22)
00000000: 0000 0000 1400 0000 0000 0000 1600 0000  ................
00000010: 0200 0000 0000 0000 0400 0000 0000 0000  ................
00000020: 0000 0000 2800 0000 0300 0000 0000 0000  ....(...........
00000030: 0400 0000 0000 0000 0100 0000 0000 0000  ................
00000040: 0400 0000 0000 0000                      ........
```
