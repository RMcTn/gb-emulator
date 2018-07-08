#include <stdio.h>
#include "cpu.h"
#include "memory.h"
#include "gpu.h"

int main(int argc, char** argv) {
    Cpu cpu;
    reset_cpu(&cpu);

    if (argc == 2) {
        //Load rom
        FILE* rom = fopen(argv[1], "r");
        if (rom == NULL) {
            printf("Could not open %s\n", argv[1]);
            return 1;
        } 
        fseek(rom, 0, SEEK_END);
        long filelength = ftell(rom);
        rewind(rom);
        printf("filelength %ld\n", filelength);
        fread(cpu.memory, filelength, 1, rom);

        fclose(rom);
    }
    
    //execution stats at 0x100
    cpu.pc = 0x100;
    loop(&cpu);
    return 0;
}
