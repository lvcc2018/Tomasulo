#ifndef INST_H
#define INST_H

#include <string>
#include <cstdlib>

// 定义指令需要运行的cycle数
#define LOAD_CYCLE_NUM 3
#define ADD_CYCLE_NUM 3
#define SUB_CYCLE_NUM 3
#define MUL_CYCLE_NUM 4
#define DIV_CYCLE_NUM 4
#define DIV_BY_ZERO_CYCLE_NUM 1

using namespace std;

enum instruction_type
{
    LOAD,
    ADD,
    SUB,
    MUL,
    DIV
};

struct instruction
{
    instruction_type type;
    int id;
    // 对于算数指令
    int dest, src1, src2;
    int cycle_num;
    int issued_cycle_num, executed_cycle_num, written_cycle_num;
    // 对于Load指令
    int addr;

    static string get_name(instruction_type _type)
    {
        switch (_type)
        {
        case LOAD:
            return "LOAD";
        case ADD:
            return "ADD";
        case SUB:
            return "SUB";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        default:
            printf("Illigal Instruction Type");
        }
    }

    static instruction_type get_type(char _type)
    {
        switch (_type)
        {
        case 'L':
            return LOAD;
        case 'A':
            return ADD;
        case 'S':
            return SUB;
        case 'M':
            return MUL;
        case 'D':
            return DIV;
        default:
            return LOAD;
        }
    }

    string print_inst_state()
    {
        string inst_str;
        char buf[128];
        switch (type)
        {
        case LOAD:
            sprintf(buf, "%s %d %d", get_name(type).c_str(), dest, addr);
            break;
        default:
            sprintf(buf, "%s %d %d %d", get_name(type).c_str(), dest, src1, src2);
            break;
        }
        inst_str = buf;
        return inst_str;
    }

    static int get_cycle_num(char _type)
    {
        switch (_type)
        {
        case 'L':
            return LOAD_CYCLE_NUM;
        case 'A':
            return ADD_CYCLE_NUM;
        case 'S':
            return SUB_CYCLE_NUM;
        case 'M':
            return MUL_CYCLE_NUM;
        case 'D':
            return DIV_CYCLE_NUM;
        }
        return 0;
    }

    void set_issue_cycle(int cycle)
    {
        issued_cycle_num = cycle;
    }
    void set_exec_cycle(int cycle)
    {
        executed_cycle_num = cycle;
    }
    void set_write_cycle(int cycle)
    {
        written_cycle_num = cycle;
    }

    static instruction init_instruction(int _id, char _type, int _dest, int _src1, int _src2)
    {
        instruction inst;
        inst.id = _id;
        inst.type = get_type(_type);
        inst.cycle_num = get_cycle_num(_type);

        if (_type == 'A' || _type == 'S' || _type == 'M' || _type == 'D')
        {
            inst.src1 = _src1;
            inst.src2 = _src2;
            inst.dest = _dest;
        }
        else if (_type == 'L')
        {
            inst.dest = _dest;
            inst.addr = _src1;
        }
        return inst;
    }
};

#endif
