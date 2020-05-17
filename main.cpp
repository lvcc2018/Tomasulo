
#include <iostream>
#include <cstdlib>
#include <fstream>

#include "sim.h"

void read_files(string path, vector<instruction> &insts)
{
    char buffer[256];
    printf("Reading files from %s...", path.c_str());
    FILE *input_file;
    input_file = fopen(path.c_str(), "r");
    int id = 0;
    while (true)
    {
        printf("%d",id);
        char *eof = fgets(buffer, 256, input_file);
        if (eof == nullptr)
            break;
        char c = buffer[0];
        if (c == 'A' or c == 'S' or c == 'M' or c == 'D') {
                char x, y, z;
                int dest, src1, src2;
                sscanf(buffer, "%c%c%c,R%d,R%d,R%d", &x, &y, &z, &dest, &src1, &src2);
                insts.push_back(instruction::init_instruction(id, c, dest, src1, src2));
            } else if (c == 'L') {
                int dest, addr;
                sscanf(buffer, "LD,R%d,%x", &dest, &addr);
                insts.push_back(instruction::init_instruction(id, c, dest, addr, 0));
            }
            id++;
    }
}

int main()
{
    vector<instruction> insts;
    read_files("TestCase/0.basic.nel", insts);
    
    return 0;
}