# Simpl-Virtual-Stack-Machine
### Screencast:
![alt-text](https://github.com/Acool4ik/Simpl-Virtual-Stack-Machine/blob/master/images/Screencastgif)
### Supported inctructions:
Byte-code | Instruction | Cnt args | high level lang analog
:---: | :---: | :---: | :---: |
0x00 | "push" | 1 | stack[top++] = N1
0x01 | "pop" | 0 | ret_value = stack[-- top]; 
0x02 | "store" | 2 | if(N < 0) stack[top-N1] = stack[top+N2]
0x02 | "store" | 2 | if(N >= 0) stack[N1] = stack[N2]
0x03 | "load" | 1 | if(N < 0) { stack[top] = stack[top+N1]; top++; }
0x03 | "load" | 1 | if(N >= 0) { stack[top] = stack[N1]; top++; }
0x04 | "add" | 0 | stack[top-2] = stack[top-1] + stack[top-2]; top --;
0x05 | "sub" | 0 | stack[top-2] = stack[top-1] - stack[top-2]; top --;
0x06 | "mul" | 0 | stack[top-2] = stack[top-1] * stack[top-2]; top --;
0x07 | "div" | 0 | stack[top-2] = stack[top-1] / stack[top-2]; top --;
0x08 | "jmp" | 1 | IP = N1;
0x09 | "jl" | 1 | if(stack[top-1] < stack[top-2]) { top = N1;} top -= 2;
0x0A | "jg" | 1 | if(stack[top-1] > stack[top-2]) { top = N1;} top -= 2;
0x0B | "je" | 1 | if(stack[top-1] == stack[top-2]) { top = N1;} top -= 2;
0x0C | "jne" | 1 | if(stack[top-1] != stack[top-2]) { top = N1;} top -= 2;
0x0D | "call" | 1 | stack[top++] = IP; IP = N1;
0x0E | "ret" | 0 | IP = stack[-- top];
0x0F | "label" | 0 | generate current IP in byte-code 
0x10 | ";" | 0 | - 
0x11 | "exit" | 0 | -
### Notes:
- `stack` - container for storage data
- `top` - top of the stack
- `IP` - instruction pointer (point on the current instruction)
- `ret_value` - value which have been returned from programm
- Instructions weight 1 byte
- One argument have weight 4 byte
- `main.asm` - file which stored program for compiling
- `exe` - file which stored byte for executing on virtual machine
