#ifndef SIM_H
#define SIM_H

#include <map>
#include <cstdlib>
#include "inst.hpp"

using namespace std;

// 相关部件个数
#define REG_NUM 32

#define ARS_NUM 6
#define MRS_NUM 3
#define LOAD_BUFFER_NUM 3

#define ADD_FU_NUM 3
#define MUL_FU_NUM 2
#define LOAD_FU_NUM 2

// 定义保留站
struct reserve_station
{
    string rs_name;
    bool busy;                     // 是否紧张
    instruction_type current_inst; // 当前的指令类型
    int vj;                        // 源操作数的值，LOAD的偏移量
    int vk;                        // 源操作数的值
    int addr;                      // Load buffer的地址
    reserve_station *qj, *qk;      // 产生源操作数的保留站
    instruction *inst;             // 当前执行的指令

    void print_rs_state()
    {
        switch (current_inst)
        {
        case LOAD:
            printf("Busy: %d  Current Instruction Type: %s  Address: %d", busy, inst->get_name().c_str(), addr);
            break;
        default:
            printf("Busy: %d  Current Instruction Type: %s  Vj: %d  Vk: %d  Qj: %s, Qk: %s\n", busy, inst->get_name().c_str(), vj, vk, (qj==nullptr)?"empty":qj->rs_name.c_str(), (qk==nullptr)?"empty":qk->rs_name.c_str());
            break;
        }
    }
};

// 定义运算部件
struct function_unit
{
    string fu_name;
    instruction_type current_inst; // 当前执行的指令
    int remain_cycle_num;          // 剩余需要的周期数
    instruction *inst;
};

// 定义寄存器
struct register_unit
{
    string reg_name;
    int value;
    reserve_station *rs;
};

class Simulator
{
private:
    int cycle; // 当前的指令周期数
    // 寄存器序列
    register_unit reg[REG_NUM];
    // 保留站序列
    reserve_station ars[ARS_NUM];
    reserve_station mrs[MRS_NUM];
    reserve_station lb[LOAD_BUFFER_NUM];
    // 运算部件序列
    function_unit afu[ADD_FU_NUM];
    function_unit mfu[MUL_FU_NUM];
    function_unit lfu[LOAD_FU_NUM];

public:
    Simulator();
    ~Simulator();
    void print_sim_state();
};

Simulator::Simulator()
{
    // 初始化各器件名
    char buf[32];
    int i;
    for (i = 0; i < REG_NUM; i++)
    {
        sprintf(buf, "Register %d", i);
        reg[i].reg_name = string(buf);
    }
    for (i = 0; i < ARS_NUM; i++)
    {
        sprintf(buf, "ARS %d", i);
        ars[i].rs_name = string(buf);
    }
    for (i = 0; i < MRS_NUM; i++)
    {
        sprintf(buf, "MRS %d", i);
        mrs[i].rs_name = string(buf);
    }
    for (i = 0; i < LOAD_BUFFER_NUM; i++)
    {
        sprintf(buf, "Load Buffer %d", i);
        lb[i].rs_name = string(buf);
    }
    for (i = 0; i < ADD_FU_NUM; i++)
    {
        sprintf(buf, "Add FU %d", i);
        afu[i].fu_name = string(buf);
    }
    for (i = 0; i < MUL_FU_NUM; i++)
    {
        sprintf(buf, "Mul FU %d", i);
        mfu[i].fu_name = string(buf);
    }
    for (i = 0; i < LOAD_FU_NUM; i++)
    {
        sprintf(buf, "Load FU %d", i);
        lfu[i].fu_name = string(buf);
    }
}

Simulator::~Simulator() {}

void Simulator::print_sim_state()
{
    int i;
    printf("Current Cycle Num: %d\n", cycle);
    printf("Reservation State:\n");
    for(i=0;i<ARS_NUM;i++)
        ars[i].print_rs_state();
    for(i=0;i<MRS_NUM;i++)
        mrs[i].print_rs_state();
    for(i=0;i<LOAD_BUFFER_NUM;i++)
        lb[i].print_rs_state();
    printf("Register State:\n");
    for(i=0;i<REG_NUM;i++)
        printf("%s Value: %d Wating RS: %s\n", reg[i].reg_name.c_str(), reg[i].value, (reg[i].rs==nullptr)?"empty":reg[i].rs->rs_name.c_str());
    printf("Functional Unit State:\n");
    for(i =0;i<ADD_FU_NUM;i++)
        printf("%s Instruction: %s Remain Cycles: %d", afu[i].fu_name.c_str(), (afu[i].inst==nullptr)?"empty":afu[i].inst->print_inst_state().c_str(), afu[i].remain_cycle_num);
    for(i =0;i<MUL_FU_NUM;i++)
        printf("%s Instruction: %s Remain Cycles: %d", mfu[i].fu_name.c_str(), (mfu[i].inst==nullptr)?"empty":mfu[i].inst->print_inst_state().c_str(), mfu[i].remain_cycle_num);
    for(i =0;i<LOAD_FU_NUM;i++)
        printf("%s Instruction: %s Remain Cycles: %d", lfu[i].fu_name.c_str(), (lfu[i].inst==nullptr)?"empty":lfu[i].inst->print_inst_state().c_str(), lfu[i].remain_cycle_num);
}
#endif