#define _CRT_SECURE_NO_WARNINGS
#define MAX_SIZE (8 * 1024 * 1024)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <am.h>
#include <klib-macros.h>
#include "vga_bin.h"

int cnt = 0;
uint32_t PC = 0;
uint32_t R[32] = { 0 };//GPR
uint8_t M[MAX_SIZE] = { 0 };    //内存

#define MAX_addr 0x20040000
#define MIN_addr 0x20000000
#define VGA_SIZE 0x40000
uint8_t vga[VGA_SIZE];


void inst_cycle()
{
    int max_cnt = 1000000;

    while (cnt < max_cnt)
    {
        uint32_t inst = *(uint32_t*)&M[PC];
        uint32_t opcode = (inst & 0x7f);
        uint32_t rd = (inst >> 7) & 0x1F;
        uint32_t funct3 = (inst >> 12) & 0x7;
        uint32_t rs1 = (inst >> 15) & 0x1F;
        uint32_t rs2 = (inst >> 20) & 0x1F;
        uint32_t funct7 = (inst >> 25) & 0x7F;

        int32_t Imm_I = (int32_t)inst >> 20;
        int32_t Imm_S = ((int32_t)(inst & 0xFE000000) >> 20) | (((inst) >> 7) & 0x1F);
        uint32_t Imm_U = (inst & 0xFFFFF000);


        uint32_t next_PC = PC + 4;
        R[0] = 0;

        switch (opcode)
        {
        case 0x13://addi
        {
            if (funct3 == 0x0)
            {
                if (rd != 0)
                    R[rd] = R[rs1] + Imm_I;
            }
            break;
        }
        case 0x67://jalr
        {
            if (funct3 == 0x0)
            {
                next_PC = (R[rs1] + Imm_I) & ~1;
                if (rd != 0)
                    R[rd] = PC + 4;
            }
            break;
        }
        case 0x33://add
        {
            if (funct3 == 0x0 && funct7 == 0x00)
                R[rd] = R[rs1] + R[rs2];
            break;
        }
        case 0x37://lui
        {
            R[rd] = Imm_U;
            break;
        }
        case 0x03://lw | lbu 从内存到寄存器
        {
            int32_t addr = Imm_I + R[rs1];

            if (addr < 0 || addr >= MAX_SIZE)
            {
                printf("ERROR: Load Access Fault as 0x%08x\n", addr);
                exit(1);
            }

            if (funct3 == 0x2)//lw
            {
                R[rd] = *(int32_t*)&M[addr];
            }
            else if (funct3 == 0x4)//lbu
            {
                R[rd] = M[addr];//相当于选择了哪个字节
            }
            break;
        }
        case 0x23://sb | sw 从寄存器到内存
        {
            int32_t addr1 = Imm_S + R[rs1];
            int offset = addr1 - MIN_addr;
            if (addr1 >= MIN_addr && addr1 < MAX_addr)//vga
            {
                if (funct3 == 0x2)
                    *(uint32_t*)&vga[offset] = R[rs2];
            }
            else if (addr1 >= 0 && addr1 < MAX_SIZE)
            {
                if (funct3 == 0x2)
                    *(uint32_t*)&M[addr1] = R[rs2];
                else if (funct3 == 0x0)
                    M[addr1] = R[rs2] & 0xFF;
            }
            else
                exit(1);
            break;
        }
        case 0x73://ebreak
        {
            if (Imm_I == 0x1)
            {
                if (R[10] == 0) printf("HIT GOOD TRAP\n");
                else  printf("HIT BAD TRAP\n");
                exit(1);
            }
        }
        default:
        {
            printf("UnKnown Instruction\n");
            return;
        }
        }
        PC = next_PC;
        R[0] = 0;
        cnt++;
        //printf("PC=%d\n", PC);
    }

}

void get_GPR()
{
    for (int i = 0; i < 32; ++i)
        printf("x%d = %u\n", i, R[i]);
}

void ebreak_init(char* filename)
{
    if (strcmp(filename, "sum.bin") == 0)//小端序
    {
        M[548 + 0] = 0x73;
        M[548 + 1] = 0x00;
        M[548 + 2] = 0x10;
        M[548 + 3] = 0x00;
        printf("sum.bin halt at 0x224\n");
    }
    else if (strcmp(filename, "mem.bin") == 0)
    {
        M[4632 + 0] = 0x73;
        M[4632 + 1] = 0x00;
        M[4632 + 2] = 0x10;
        M[4632 + 3] = 0x00;
        printf("sum.bin halt at 0x1218\n");
    }
}

int main(int argc, char* argv[])
{
    ioe_init();
    for (unsigned int i = 0; i < vga_bin_len; ++i)
        M[i] = vga_bin[i];

    //ebreak_init(NULL);
    inst_cycle();
    get_GPR();

    io_write(AM_GPU_FBDRAW, 0, 0, vga, 256, 256, true);
    while (1) {}

    return 0;
}

