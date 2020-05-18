
#include <iostream>
#include <cstdlib>
#include <fstream>

#include "sim.h"

void read_files(string path, vector<instruction> &insts)
{
    char buffer[256];
    printf("Reading files from %s ...", path.c_str());
    FILE *input_file;
    input_file = fopen(path.c_str(), "r");
    int id = 0;
    while (true)
    {
        char *eof = fgets(buffer, 256, input_file);
        if (eof == nullptr)
            break;
        char c = buffer[0];
        if (c == 'A' or c == 'S' or c == 'M' or c == 'D')
        {
            char x, y, z;
            int dest, src1, src2;
            sscanf(buffer, "%c%c%c,R%d,R%d,R%d", &x, &y, &z, &dest, &src1, &src2);
            insts.push_back(instruction::init_instruction(id, c, dest, src1, src2));
        }
        else if (c == 'L')
        {
            int dest, addr;
            sscanf(buffer, "LD,R%d,%x", &dest, &addr);
            insts.push_back(instruction::init_instruction(id, c, dest, addr, 0));
        }
        id++;
    }
    printf("finished!\n");
}

void write_files(string path, vector<instruction> insts)
{
    printf("Writing files to %s ...", path.c_str());
    FILE *output_file;
    output_file = fopen(path.c_str(),"w");
    for(int i=0;i<insts.size();i++)
        fprintf(output_file, "%d %d %d\n", insts[i].issued_cycle_num, insts[i].executed_cycle_num, insts[i].written_cycle_num);
    fclose(output_file);
    printf("Finished!");
}

int main()
{
    vector<instruction> insts;
    read_files("TestCase/1.basic.nel", insts);
    Simulator sim;
    int id = 0, step;
    //sim.print_sim_state();
    printf("Simulate start...\n");
    while (true)
    {
        printf("%s\n", insts[id].print_inst_state().c_str());
        step = sim.Cycle(id < insts.size() ? &insts[id] : nullptr);
        sim.print_sim_state();
        if (step == -1)
            break;
        id += step;
    }
    printf("Finished\n");
    write_files("2017011441_1.basic.log", insts);
    return 0;
}