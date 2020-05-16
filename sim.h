#ifndef SIM_H
#define SIM_H

#include <map>
#include <cstdlib>
#include "inst.h"

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
    bool busy;                      // 是否紧张
    instruction_type current_inst;  // 当前的指令类型
    int vj;                         // 源操作数的值，LOAD的偏移量
    int vk;                         // 源操作数的值
    int addr;                       // Load buffer的地址
    reserve_station *qj, *qk;       // 产生源操作数的保留站
    instruction *inst;              // 当前执行的指令
    
};

// 定义运算部件
struct function_unit
{
    instruction_type current_inst;  // 当前执行的指令
    int remain_cycle_num;           // 剩余需要的周期数
};

// 定义寄存器
struct register_unit
{
    reserve_station *rs;
};





#endif