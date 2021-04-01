#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct rv32_cpu {
    uint32_t regs[32];
    uint32_t pc;

    uint32_t *mem;
    size_t byte_cnt;
};

struct inst_R { // regsiter/register
    uint32_t opcode : 7;
    uint32_t rd : 5;
    uint32_t func3 : 3;
    uint32_t rs1 : 5;
    uint32_t rs2 : 5;
    uint32_t func7 : 7;
};

struct inst_I { // immediate
    union {
        struct {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t func3 : 3;
            uint32_t rs1 : 5;
            uint32_t imm : 12;
        } imm;

        struct {
            uint32_t opcode : 7;
            uint32_t rd : 5;
            uint32_t func3 : 3;
            uint32_t rs1 : 5;
            uint32_t shamt : 5;
            uint32_t func7 : 7;
        } shift;
    };
};

struct inst_U { // upper-immidate
    uint32_t opcode : 7;
    uint32_t rd : 5;
    uint32_t imm : 20;
};

struct inst_S { // store
    uint32_t opcode : 7;
    uint32_t imm0 : 5;
    uint32_t func3 : 3;
    uint32_t rs1 : 5;
    uint32_t rs2 : 5;
    uint32_t imm1 : 7;
};

struct inst_B { // branch
    uint32_t opcode : 7;
    uint32_t imm0 : 1;
    uint32_t imm1 : 4;
    uint32_t func3 : 3;
    uint32_t rs1 : 5;
    uint32_t rs2 : 5;
    uint32_t imm2: 6;
    uint32_t imm3: 1;
};

struct inst_J { // jump
    uint32_t opcode : 7;
    uint32_t rd : 5;
    uint32_t imm0 : 8;
    uint32_t imm1 : 1;
    uint32_t imm2 : 10;
    uint32_t imm3 : 1;
};

struct rv32_inst_R {
    struct inst_R inst;
    void (*handler)(struct rv32_cpu *, struct inst_R *);
};

static void rv32_add(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] + cpu->regs[inst->rs2];
}

static void rv32_sub(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] - cpu->regs[inst->rs2];
}

static void rv32_sll(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] << cpu->regs[inst->rs2];
}

static void rv32_srl(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] >> cpu->regs[inst->rs2];
}

static void rv32_slt(struct rv32_cpu *cpu, struct inst_R *inst) {
    if((signed)cpu->regs[inst->rs1] < (signed)cpu->regs[inst->rs2]) {
        cpu->regs[inst->rd] = 1;  
    } else {
        cpu->regs[inst->rd] = cpu->regs[inst->rs1] ^ cpu->regs[inst->rs2];
    }
}

static void rv32_sltu(struct rv32_cpu *cpu, struct inst_R *inst) {
    if(cpu->regs[inst->rs1] < cpu->regs[inst->rs2]) {
        cpu->regs[inst->rd] = 1;  
    } else {
        cpu->regs[inst->rd] = cpu->regs[inst->rs1] ^ cpu->regs[inst->rs2];
    }
}

static void rv32_xor(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] ^ cpu->regs[inst->rs2];
}

static void rv32_or(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] | cpu->regs[inst->rs2];
}

static void rv32_and(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = cpu->regs[inst->rs1] & cpu->regs[inst->rs2];
}

static void rv32_sra(struct rv32_cpu *cpu, struct inst_R *inst) {
    cpu->regs[inst->rd] = (signed)cpu->regs[inst->rs1] >> (signed)cpu->regs[inst->rs2];
}

static struct rv32_inst_R rv32_R_list[] = {
    { .inst = { .opcode = 0b0110011, .func3 = 0b000, .func7 = 0b0000000 }, .handler = rv32_add },
    { .inst = { .opcode = 0b0110011, .func3 = 0b000, .func7 = 0b0100000 }, .handler = rv32_sub },
    { .inst = { .opcode = 0b0110011, .func3 = 0b001, .func7 = 0b0000000 }, .handler = rv32_sll },
    { .inst = { .opcode = 0b0110011, .func3 = 0b010, .func7 = 0b0000000 }, .handler = rv32_slt },
    { .inst = { .opcode = 0b0110011, .func3 = 0b011, .func7 = 0b0000000 }, .handler = rv32_sltu },
    { .inst = { .opcode = 0b0110011, .func3 = 0b100, .func7 = 0b0000000 }, .handler = rv32_xor },
    { .inst = { .opcode = 0b0110011, .func3 = 0b101, .func7 = 0b0000000 }, .handler = rv32_srl },
    { .inst = { .opcode = 0b0110011, .func3 = 0b101, .func7 = 0b0100000 }, .handler = rv32_sra }, 
    { .inst = { .opcode = 0b0110011, .func3 = 0b110, .func7 = 0b0000000 }, .handler = rv32_or },
    { .inst = { .opcode = 0b0110011, .func3 = 0b111, .func7 = 0b0000000 }, .handler = rv32_and }
};

struct rv32_inst_I {
    struct inst_I inst;
    void (*handler)(struct rv32_cpu *, struct inst_I *);
};

static void rv32_addi(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = cpu->regs[inst->imm.rs1] + inst->imm.imm;
}

static void rv32_slti(struct rv32_cpu *cpu, struct inst_I *inst) {
    if((signed)cpu->regs[inst->imm.rs1] < (signed)inst->imm.imm) {
        cpu->regs[inst->imm.rd] = 1;  
    } else {
        cpu->regs[inst->imm.rd] = 0;  
    }
}

static void rv32_sltiu(struct rv32_cpu *cpu, struct inst_I *inst) {
    if(cpu->regs[inst->imm.rs1] < inst->imm.imm) {
        cpu->regs[inst->imm.rd] = 1;  
    } else {
        cpu->regs[inst->imm.rd] = 0;  
    }
}

static void rv32_xori(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = cpu->regs[inst->imm.rs1] ^ inst->imm.imm;
}

static void rv32_ori(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = cpu->regs[inst->imm.rs1] | inst->imm.imm;
}

static void rv32_andi(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = cpu->regs[inst->imm.rs1] & inst->imm.imm;
}

static void rv32_slli(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->shift.rd] = cpu->regs[inst->shift.rs1] << inst->shift.shamt;
}

static void rv32_srli(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->shift.rd] = cpu->regs[inst->shift.rs1] >> inst->shift.shamt;
}

static void rv32_srai(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->shift.rd] = (signed)cpu->regs[inst->shift.rs1] >> (signed)inst->shift.shamt;
}

static void rv32_lb(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = (uint8_t)cpu->mem[(cpu->regs[inst->imm.rs1] + inst->imm.imm) / 4] | 0xff000000;
}

static void rv32_lh(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = (uint16_t)cpu->mem[(cpu->regs[inst->imm.rs1] + inst->imm.imm) / 4] | 0xffff0000;
}

static void rv32_lw(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = cpu->mem[(cpu->regs[inst->imm.rs1] + inst->imm.imm) / 4];
}

static void rv32_lbu(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = (uint8_t)cpu->mem[(cpu->regs[inst->imm.rs1] + inst->imm.imm) / 4];
}

static void rv32_lhu(struct rv32_cpu *cpu, struct inst_I *inst) {
    cpu->regs[inst->imm.rd] = (uint16_t)cpu->mem[(cpu->regs[inst->imm.rs1] + inst->imm.imm) / 4];
}

static struct rv32_inst_I rv32_I_list[] = {
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b000 } }, .handler = rv32_addi },
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b010 } }, .handler = rv32_slti },
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b011 } }, .handler = rv32_sltiu } ,
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b100 } }, .handler = rv32_xori },
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b110 } }, .handler = rv32_ori },
    { .inst = { .imm = { .opcode = 0b0010011, .func3 = 0b111 } }, .handler = rv32_andi },
    { .inst = { .shift = { .opcode = 0b0010011, .func3 = 0b001, .func7 = 0b0000000 } }, .handler = rv32_slli },
    { .inst = { .shift = { .opcode = 0b0010011, .func3 = 0b101, .func7 = 0b0000000 } }, .handler = rv32_srli },
    { .inst = { .shift = { .opcode = 0b0010011, .func3 = 0b101, .func7 = 0b0100000 } }, .handler = rv32_srai },
    { .inst = { .imm = { .opcode = 0b0000011, .func3 = 0b000 } }, .handler = rv32_lb },
    { .inst = { .imm = { .opcode = 0b0000011, .func3 = 0b001 } }, .handler = rv32_lh },
    { .inst = { .imm = { .opcode = 0b0000011, .func3 = 0b010 } }, .handler = rv32_lw },
    { .inst = { .imm = { .opcode = 0b0000011, .func3 = 0b100 } }, .handler = rv32_lbu },
    { .inst = { .imm = { .opcode = 0b0000011, .func3 = 0b101 } }, .handler = rv32_lhu }
};

struct rv32_inst_S {
    struct inst_S inst;
    void (*handler)(struct rv32_cpu *, struct inst_S *);
};

static void rv32_sb(struct rv32_cpu *cpu, struct inst_S *inst) {
    ((uint8_t*)cpu->mem)[(cpu->regs[inst->rs1] + ((inst->imm1 << 7) | inst->imm0))] = (uint8_t)cpu->regs[inst->rs2];
}

static void rv32_sh(struct rv32_cpu *cpu, struct inst_S *inst) {
    ((uint16_t*)cpu->mem)[(cpu->regs[inst->rs1] + ((inst->imm1 << 7) | inst->imm0)) / 2] = (uint16_t)cpu->regs[inst->rs2];
}

static void rv32_sw(struct rv32_cpu *cpu, struct inst_S *inst) {
    cpu->mem[(cpu->regs[inst->rs1] + ((inst->imm1 << 7) | inst->imm0)) / 4] = cpu->regs[inst->rs2];
}

static struct rv32_inst_S rv32_S_list[] = {
    { .inst = { .opcode = 0b0100011, .func3 = 0b000 }, .handler = rv32_sb },
    { .inst = { .opcode = 0b0100011, .func3 = 0b001 }, .handler = rv32_sh },
    { .inst = { .opcode = 0b0100011, .func3 = 0b010 }, .handler = rv32_sw }
};

struct rv32_inst_B {
    struct inst_B inst;
    void (*handler)(struct rv32_cpu *, struct inst_B *inst);
};

static void rv32_beq(struct rv32_cpu *cpu, struct inst_B *inst) {
    if(cpu->regs[inst->rs1] == cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static void rv32_bne(struct rv32_cpu *cpu, struct inst_B *inst) {
    if(cpu->regs[inst->rs1] != cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static void rv32_blt(struct rv32_cpu *cpu, struct inst_B *inst) {
    if((signed)cpu->regs[inst->rs1] < (signed)cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static void rv32_bge(struct rv32_cpu *cpu, struct inst_B *inst) {
    if((signed)cpu->regs[inst->rs1] > (signed)cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static void rv32_bltu(struct rv32_cpu *cpu, struct inst_B *inst) {
    if(cpu->regs[inst->rs1] < cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static void rv32_bgeu(struct rv32_cpu *cpu, struct inst_B *inst) {
    if(cpu->regs[inst->rs1] > cpu->regs[inst->rs2]) {
        cpu->pc += (((((inst->imm0 << 1) | inst->imm1) << 5) | inst->imm2) << 11) | inst->imm3;
    }
}

static struct rv32_inst_B rv32_B_list[] = {
    { .inst = { .opcode = 0b1100011, .func3 = 0b000 }, .handler = rv32_beq }, 
    { .inst = { .opcode = 0b1100011, .func3 = 0b001 }, .handler = rv32_bne }, 
    { .inst = { .opcode = 0b1100011, .func3 = 0b100 }, .handler = rv32_blt }, 
    { .inst = { .opcode = 0b1100011, .func3 = 0b101 }, .handler = rv32_bge }, 
    { .inst = { .opcode = 0b1100011, .func3 = 0b110 }, .handler = rv32_bltu }, 
    { .inst = { .opcode = 0b1100011, .func3 = 0b111 }, .handler = rv32_bgeu }
};

static int instruction_decode(struct rv32_cpu *cpu, uint32_t *word) {
    uint8_t opcode = *word & 0x7f;

    switch(opcode) {
        case 0b0110011: { // type_R
            struct inst_R inst_R = *(struct inst_R*)word;

            for(size_t i = 0; i < sizeof(rv32_R_list) / sizeof(struct rv32_inst_R); i++) {
                if(rv32_R_list[i].inst.func3 == inst_R.func3 && rv32_R_list[i].inst.func7 == inst_R.func7) {
                    rv32_R_list[i].handler(cpu, &inst_R);
                    goto end;
                }
            }

            break;
        }
        case 0b0000011: // TYPE_I
        case 0b0010011: {
            struct inst_I inst_I = *(struct inst_I*)word;

            for(size_t i = 0; i < sizeof(rv32_I_list) / sizeof(struct rv32_inst_I); i++) {
                if(rv32_I_list[i].inst.imm.func3 == inst_I.imm.func3) {
                    rv32_I_list[i].handler(cpu, &inst_I);
                    goto end;
                }

                if(rv32_I_list[i].inst.shift.func3 == inst_I.shift.func3 && rv32_I_list[i].inst.shift.func7 == inst_I.shift.func7) {
                    rv32_I_list[i].handler(cpu, &inst_I);
                    goto end;
                }
            }

            break;
        }
        case 0b0100011: { // type_S
            struct inst_S inst_S = *(struct inst_S*)word;

            for(size_t i = 0; i < sizeof(rv32_S_list) / sizeof(struct rv32_inst_S); i++) {
                if(rv32_S_list[i].inst.func3 == inst_S.func3) {
                    rv32_S_list[i].handler(cpu, &inst_S);
                    goto end;
                }
            }

            break;
        }
        case 0b1100011: { // type_B
            struct inst_B inst_B = *(struct inst_B*)word;
            
            for(size_t i = 0; i < sizeof(rv32_B_list) / sizeof(struct rv32_inst_B); i++) {
                if(rv32_B_list[i].inst.func3 == inst_B.func3) {
                    rv32_B_list[i].handler(cpu, &inst_B);
                    goto end;
                }
            }

            break;
        }
        case 0b0110111: { // lui
            struct inst_U inst_U = *(struct inst_U*)word;

            cpu->regs[inst_U.rd] = inst_U.imm << 12;

            goto end;
        }
        case 0b0010111: { // auipc
            struct inst_U inst_U = *(struct inst_U*)word;

            cpu->regs[inst_U.rd] = cpu->pc + (inst_U.imm << 12);

            goto end;
        }
        case 0b1101111: { // jal
            struct inst_J inst_J = *(struct inst_J*)word;

            cpu->regs[inst_J.rd] = cpu->pc + 4;
            cpu->pc += (((((inst_J.imm2 << 10) | inst_J.imm1) << 11) | inst_J.imm0) << 19) | inst_J.imm3;

            goto end;
        }
        case 0b1100111: { // jalr
            struct inst_I inst_I = *(struct inst_I*)word;

            cpu->regs[inst_I.imm.rd] = cpu->pc + 4;
            cpu->pc = cpu->regs[inst_I.imm.rs1] + inst_I.imm.imm;

            goto end;
        }
    }

    return -1;

end:
    cpu->pc += 4;
    return 0;
}

static void rv32_reg_dump(struct rv32_cpu *cpu) {
    for(int i = 0; i < 32; i++) {
        printf("x%d: %x\n", i, cpu->regs[i]);
    }
    printf("pc: %x\n", cpu->pc);
}

static size_t read_file(const char *path, uint32_t **data) { 
    FILE *fp = fopen(path, "r");
    if(fp == NULL) { 
        printf("Unable to open file %s\n", path);
        exit(0);
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *data = malloc(file_size);
    fread(*data, 1, file_size, fp);

    return file_size;
}

static void create_rv32_cpu(const char *program_path, struct rv32_cpu **cpu) {
    *cpu = calloc(sizeof(struct rv32_cpu), 1);
    size_t program_size = read_file(program_path, &(*cpu)->mem);
    (*cpu)->mem = realloc((*cpu)->mem, 0x8000);
    (*cpu)->byte_cnt = 0x8000;
}

int main(int argc, char *argv[]) {
    if(argc < 1) {
        printf("No parameters\n");
        exit(0);
    }

    struct rv32_cpu *cpu;
    create_rv32_cpu(argv[1], &cpu);

    for(;;) {
        if(instruction_decode(cpu, &cpu->mem[cpu->pc / 4]) == -1) {\
            rv32_reg_dump(cpu);
            exit(0);
        }
    }
}
