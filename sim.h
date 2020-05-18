#ifndef SIM_H
#define SIM_H

#include <map>
#include <cstdlib>
#include <vector>
#include "inst.h"

using namespace std;

// 相关部件个数
#define REG_NUM 32

#define ARS_NUM 6
#define MRS_NUM 3
#define LOAD_BUFFER_NUM 3

#define ADD_FU_NUM 3
#define MUL_FU_NUM 2
#define LOAD_FU_NUM 2

// 定义运算部件
struct function_unit
{
    string fu_name;
    int remain_cycle_num; // 剩余需要的周期数
    int result, dest;
};
// 定义保留站
struct reserve_station
{
    string rs_name;
    bool busy;                // 是否紧张
    int vj;                   // 源操作数的值，LOAD的偏移量
    int vk;                   // 源操作数的值
    int addr;                 // Load buffer的地址
    reserve_station *qj, *qk; // 产生源操作数的保留站
    instruction *inst;        // 当前执行的指令
    function_unit *fu;

    void print_rs_state()
    {
        printf("%s\t", rs_name.c_str());
        if (inst == nullptr)
            printf("Busy: %d\n", busy);
        else
        {
            switch (inst->type)
            {
            case LOAD:
                printf("Busy: %d\tCurrent Instruction Type: %s\tAddress: %d\n", busy, inst->get_name(inst->type).c_str(), addr);
                break;
            default:
                printf("Busy: %d\tCurrent Instruction Type: %s\tVj: %d\tVk: %d\tQj: %s\tQk: %s\n", busy, inst->get_name(inst->type).c_str(), vj, vk, (qj == nullptr) ? "empty" : qj->rs_name.c_str(), (qk == nullptr) ? "empty" : qk->rs_name.c_str());
                break;
            }
        }
    }
    void release()
    {
        busy = false;
        vj = vk = addr = 0;
        qj = nullptr;
        qk = nullptr;
        inst = nullptr;
        fu = nullptr;
    }
    void init(string name)
    {
        rs_name = name;
        release();
    }
};

// 定义寄存器
struct register_unit
{
    string reg_name;
    int value;
    reserve_station *rs;
};

struct write_back_unit
{
    int dest;
    int value;
    write_back_unit(int d, int r) : dest(d), value(r){};
};

// 辅助函数
bool cmp(reserve_station *a, reserve_station *b)
{
    return ((a->inst->id) < (b->inst->id));
}

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

    // 辅助队列
    vector<write_back_unit> write_back_vector;
    vector<reserve_station *> execute_vector;
    std::map<reserve_station *, int> just_finished_cdb;
    bool registers_updated[REG_NUM];
    int latest_registers_values[REG_NUM];

public:
    Simulator();
    ~Simulator();
    void print_sim_state();
    void write_back();
    bool check_rs(reserve_station *rs);
    void set_src(register_unit &reg, int &v, reserve_station *&q, int src);
    bool issue(reserve_station rs[], int num, instruction *inst);
    void execute(reserve_station *rs, function_unit fu[], int num, instruction *isnt);
    int Cycle(instruction *inst);
    void clear_help_vec();
};

Simulator::Simulator()
{
    cycle = 0;
    // 初始化各器件名
    char buf[128];
    int i;
    for (i = 0; i < REG_NUM; i++)
    {
        sprintf(buf, "Register %d", i);
        reg[i].reg_name = string(buf);
        reg[i].rs = nullptr;
        reg[i].value = 0;
    }
    for (i = 0; i < ARS_NUM; i++)
    {
        sprintf(buf, "ARS %d", i);
        ars[i].init(string(buf));
    }
    for (i = 0; i < MRS_NUM; i++)
    {
        sprintf(buf, "MRS %d", i);
        mrs[i].init(string(buf));
    }
    for (i = 0; i < LOAD_BUFFER_NUM; i++)
    {
        sprintf(buf, "Load Buffer %d", i);
        lb[i].init(string(buf));
    }
    for (i = 0; i < ADD_FU_NUM; i++)
    {
        sprintf(buf, "Add FU %d", i);
        afu[i].fu_name = string(buf);
        afu[i].remain_cycle_num = afu[i].dest = afu[i].result = 0;
    }
    for (i = 0; i < MUL_FU_NUM; i++)
    {
        sprintf(buf, "Mul FU %d", i);
        mfu[i].fu_name = string(buf);
        mfu[i].remain_cycle_num = mfu[i].dest = mfu[i].result = 0;
    }
    for (i = 0; i < LOAD_FU_NUM; i++)
    {
        sprintf(buf, "Load FU %d", i);
        lfu[i].fu_name = string(buf);
        lfu[i].remain_cycle_num = lfu[i].dest = lfu[i].result = 0;
    }
    write_back_vector.clear();
    execute_vector.clear();
    printf("Similator Initializing...Finished!\n\n");
}

Simulator::~Simulator() {}

void Simulator::print_sim_state()
{
    int i;
    printf("----------------------------------------------------------------------\n");
    printf("Current Cycle Num: %d\n\n", cycle);
    printf("Reservation State:\n");
    for (i = 0; i < ARS_NUM; i++)
        ars[i].print_rs_state();
    for (i = 0; i < MRS_NUM; i++)
        mrs[i].print_rs_state();
    for (i = 0; i < LOAD_BUFFER_NUM; i++)
        lb[i].print_rs_state();
    printf("\nRegister State:\n");
    for (i = 0; i < REG_NUM; i++)
        printf("%s\tValue: %d\tWating RS: %s\n", reg[i].reg_name.c_str(), reg[i].value, (reg[i].rs == nullptr) ? "empty" : reg[i].rs->rs_name.c_str());
    printf("\nFunctional Unit State:\n");
    for (i = 0; i < ADD_FU_NUM; i++)
        printf("%s\tRemain Cycles: %d\n", afu[i].fu_name.c_str(), afu[i].remain_cycle_num);
    for (i = 0; i < MUL_FU_NUM; i++)
        printf("%s\tRemain Cycles: %d\n", mfu[i].fu_name.c_str(), mfu[i].remain_cycle_num);
    for (i = 0; i < LOAD_FU_NUM; i++)
        printf("%s\tRemain Cycles: %d\n", lfu[i].fu_name.c_str(), lfu[i].remain_cycle_num);
    printf("----------------------------------------------------------------------\n");
}

void Simulator::set_src(register_unit &reg, int &v, reserve_station *&q, int src)
{
    if (reg.rs != nullptr)
        v = 0, q = reg.rs;
    else
        v = registers_updated[src] ? latest_registers_values[src] : reg.value, q = nullptr;
}

void Simulator::write_back()
{
    for (size_t i = 0; i < write_back_vector.size(); i++)
    {
        reg[write_back_vector[i].dest].value = write_back_vector[i].value;
    }
    write_back_vector.clear();
}

bool Simulator::check_rs(reserve_station *rs)
{
    bool running = false;
    if (!rs->busy)
        return running;
    else
    {
        running = true;
        if (rs->fu == nullptr)
        {
            execute_vector.push_back(rs);
        }
        else
        {
            rs->fu->remain_cycle_num--;
            if (rs->fu->remain_cycle_num == 0)
            {
                if (reg[rs->fu->dest].rs == rs)
                {
                    reg[rs->fu->dest].rs = nullptr;
                    write_back_vector.push_back(write_back_unit(rs->fu->dest, rs->fu->result));
                    registers_updated[rs->fu->dest] = true;
                    latest_registers_values[rs->fu->dest] = rs->fu->result;
                }
                just_finished_cdb[rs] = rs->fu->result;
                rs->inst->set_exec_cycle(cycle);
                rs->inst->set_write_cycle(cycle + 1);
                rs->release();
            }
        }
        return running;
    }
}

void Simulator::execute(reserve_station *rs, function_unit fu[], int num, instruction *inst)
{
    for (int i = 0; i < num; i++)
    {
        if (fu[i].remain_cycle_num == 0)
        {
            rs->fu = &fu[i];
            int remain_cycles, result;
            bool div_error = (rs->vk == 0);
            switch (rs->inst->type)
            {
            case ADD:
                remain_cycles = ADD_CYCLE_NUM, result = rs->vj + rs->vk;
                break;
            case SUB:
                remain_cycles = SUB_CYCLE_NUM, result = rs->vj - rs->vk;
                break;
            case MUL:
                remain_cycles = MUL_CYCLE_NUM, result = rs->vj * rs->vk;
                break;
            case DIV:
                remain_cycles = div_error ? DIV_BY_ZERO_CYCLE_NUM : DIV_CYCLE_NUM;
                result = div_error ? 0 : (rs->vj / rs->vk);
                break;
            case LOAD:
                remain_cycles = LOAD_CYCLE_NUM, result = rs->addr;
                break;
            default:
                break;
            }
            fu[i].dest = rs->inst->dest;
            fu[i].remain_cycle_num = remain_cycles;
            fu[i].result = result;
            break;
        }
    }
}

bool Simulator::issue(reserve_station rs[], int num, instruction *inst)
{
    bool step = false;
    for (int i = 0; i < num; i++)
    {
        if (!rs[i].busy)
        {
            step = true;
            rs[i].busy = true;
            rs[i].inst = inst;
            rs[i].fu = nullptr;
            switch (inst->type)
            {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
                set_src(reg[inst->src1], rs[i].vj, rs[i].qj, inst->src1);
                set_src(reg[inst->src2], rs[i].vk, rs[i].qk, inst->src2);
                break;
            case LOAD:
                rs[i].addr = inst->addr;
            default:
                break;
            }
            reg[inst->dest].rs = &rs[i];
            execute_vector.push_back(&rs[i]);
            inst->set_issue_cycle(cycle);
            return step;
        }
    }
    return step;
}

void Simulator::clear_help_vec()
{
    execute_vector.clear();
    just_finished_cdb.clear();
    for (int i = 0; i < REG_NUM; i++)
    {
        registers_updated[i] = false;
        latest_registers_values[i] = true;
    }
}

int Simulator::Cycle(instruction *inst)
{
    cycle++;
    clear_help_vec();
    // 写回操作
    printf("Write Back ...\n");
    write_back();

    // 检查RS是否可以结束并释放
    printf("Check RS ...\n");
    bool running = false;
    for (int i = 0; i < ARS_NUM; i++)
        if (check_rs(&ars[i]))
            running = true;
    for (int i = 0; i < MRS_NUM; i++)
        if (check_rs(&mrs[i]))
            running = true;
    for (int i = 0; i < LOAD_BUFFER_NUM; i++)
        if (check_rs(&lb[i]))
            running = true;
    if ((!running) && inst == nullptr)
    {
        return -1;
    }
    // 发射操作
    printf("Issue ...\n");
    bool step = true;
    if (inst != nullptr)
    {
        switch (inst->type)
        {
        case ADD:
        case SUB:
            step = issue(ars, ARS_NUM, inst);
            break;
        case MUL:
        case DIV:
            step = issue(mrs, MRS_NUM, inst);
            break;
        case LOAD:
            step = issue(lb, LOAD_BUFFER_NUM, inst);
            break;
        default:
            break;
        }
    }
    /*
    for (int i = 0; i < execute_vector.size(); i++)
    {
        execute_vector[i]->inst->print_inst_state();
    }
    */
    sort(execute_vector.begin(), execute_vector.end(), cmp);

    printf("Execute ...\n");
    for (int i = 0; i < execute_vector.size(); i++)
    {
        reserve_station *rs = execute_vector[i];
        if (rs->qj != nullptr and just_finished_cdb.count(rs->qj))
            rs->vj = just_finished_cdb[rs->qj], rs->qj = nullptr;
        if (rs->qk != nullptr and just_finished_cdb.count(rs->qk))
            rs->vk = just_finished_cdb[rs->qk], rs->qk = nullptr;
        if (not(rs->inst->type == LOAD or (rs->qj == nullptr and rs->qk == nullptr)))
            continue;

        switch (execute_vector[i]->inst->type)
        {
        case ADD:
        case SUB:
            execute(rs, afu, ADD_FU_NUM, inst);
            break;
        case MUL:
        case DIV:
            execute(rs, mfu, MUL_FU_NUM, inst);
            break;
        case LOAD:
            execute(rs, lfu, LOAD_FU_NUM, inst);
            break;
        default:
            break;
        }
    }
    return step ? 1 : 0;
}

#endif