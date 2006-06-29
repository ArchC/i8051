/**
 * @file      i8051_isa.cpp
 * @author    Tiago Sampaio Lins
 *            Silvio Veloso
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   version?
 * @date      Mon, 19 Jun 2006 15:50:50 -0300
 * 
 * @brief     The ArchC i8051 functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "i8051_isa.H"
#include "i8051_isa_init.cpp"
#include "i8051_bhv_macros.H"
#include <systemc.h>

// Debug defines
//#define _I8051_FORCE_END_ // Force the simulation to end.
//#define _I8051_DUMP_MEMORY_ // Get a memory dump at the end of simulation.
// Defines
#define ACC 224
#define PSW 208
#define B   240
#define DPTRH 131
#define DPTRL 130
#define SP 129

using namespace i8051_parms;

void memdump(const char* fn, ac_memport<ac_word, ac_Hword>& sto)
{
 fstream dfile;
 unsigned i;

 dfile.open(fn, ios::out);
 for (i = 0; i < sto.get_size(); i += (sizeof(ac_word)))
  dfile << hex << i << "  " << sto.read(i) << endl;
 dfile.close();
 return;
}

// Initialize special registers for simulation
void ac_behavior(begin)
{
 IRAM.write(0x81, 0x7);
#ifdef _I8051_FORCE_END_
 pc_stability = 0;
 old_pc = 0;
 curr_pc = 0;
#endif
 return;
}

// Must be implemented.
void ac_behavior(end)
{
#ifdef _I8051_DUMP_MEMORY_
 char* filename;
#endif

#ifdef _I8051_FORCE_END_
 fprintf(stdout, "ACC:   %04lx\n",
         static_cast<unsigned long>(IRAM.read(ACC)));
 fprintf(stdout, "PSW:   %04lx\n",
         static_cast<unsigned long>(IRAM.read(PSW)));
 fprintf(stdout, "B:     %04lx\n",
         static_cast<unsigned long>(IRAM.read(B)));
 fprintf(stdout, "DPTRH: %04lx\n",
         static_cast<unsigned long>(IRAM.read(DPTRH)));
 fprintf(stdout, "DPTRL: %04lx\n",
         static_cast<unsigned long>(IRAM.read(DPTRL)));
 fprintf(stdout, "SP:    %04lx\n",
         static_cast<unsigned long>(IRAM.read(SP)));
#endif
#ifdef _I8051_DUMP_MEMORY_
 filename = new char[40];
 sprintf(filename, "iram.%x.%lu.dump", ac_pc.read(), ac_instr_counter);
 memdump(filename, IRAM);
 sprintf(filename, "iramx.%x.%lu.dump", ac_pc.read(), ac_instr_counter);
 memdump(filename, IRAMX);
 sprintf(filename, "irom.%x.%lu.dump", ac_pc.read(), ac_instr_counter);
 memdump(filename, IROM);
 delete[] filename;
#endif
 return;
}

//!Generic instruction behavior method.
void ac_behavior(instruction)
{
#ifdef _I8051_FORCE_END_
 if (old_pc == curr_pc)
  pc_stability++;
 else
  pc_stability = 0;
#endif
 ac_pc += get_size();
 pc = ac_pc.read();
#ifdef _I8051_FORCE_END_
 old_pc = curr_pc;
 curr_pc = pc;
 if ((pc_stability > 31) || (ac_instr_counter > 5000000))
  stop(0);
#endif
 return;
}

//! Instruction Format behavior methods.
void ac_behavior(Type_3bytes)
{
 return;
}

void ac_behavior(Type_2bytes)
{
 return;
}

void ac_behavior(Type_OP_R)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = reg;
 else if (psw.range(4, 3) == 1)
  reg_indx = reg + 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = reg + 16;
 else
  reg_indx = reg + 24;
 return;
}

void ac_behavior(Type_IBRCH)
{
 return;
}

void ac_behavior(Type_1byte)
{
 return;
}

void ac_behavior(Type_3bytesReg)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = reg2;
 else if (psw.range(4, 3) == 1)
  reg_indx = reg2 + 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = reg2 + 16;
 else
  reg_indx = reg2 + 24;
 return;
}

void ac_behavior(Type_2bytesReg)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = reg2;
 else if (psw.range(4, 3) == 1)
  reg_indx = reg2 + 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = reg2 + 16;
 else
  reg_indx = reg2 + 24;
 return;
}

void ac_behavior(xch_ar)
{
 sc_uint<8> acc, temp, data, psw;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 data = IRAM.read(reg_indx);
 temp = acc;
 acc = data;
 data = temp;
 IRAM.write(reg_indx, data);
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(xch_arr_R0)
{
 sc_uint<8> acc, temp, data, psw;
 int reg_indx;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 data = IRAM.read(IRAM.read(reg_indx));
 temp = acc;
 acc = data;
 data = temp;
 IRAM.write(IRAM.read(reg_indx), data);
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(xch_arr_R1)
{
 sc_uint<8> acc, temp, data, psw;
 int reg_indx;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 data = IRAM.read(IRAM.read(reg_indx));
 temp = acc;
 acc = data;
 data = temp;
 IRAM.write(IRAM.read(reg_indx), data);
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(xch_a_iram)
{
 sc_uint<8> acc, temp, data;

 acc = IRAM.read(ACC);
 data = IRAM.read(byte2);
 temp = acc;
 acc = data;
 data = temp;
 IRAM.write(byte2, data);
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(jb)
{
 sc_uint<8> aux, data;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;
 int addr;

 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 if (data[aux.range(2, 0)] == 1)
  ac_pc = pc + tempByte3;
 return;
}

void ac_behavior(jnb)
{
 sc_uint<8> aux, data;
 int addr;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 if (data[aux.range(2, 0)] == 0)
  ac_pc = pc + tempByte3;
 return;
}

void ac_behavior(jbc)
{
 sc_uint<8> aux, data, psw;
 int addr;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 psw = IRAM.read(PSW);
 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 if (data[aux.range(2, 0)] == 1)
 {
  ac_pc = pc + tempByte3;
  data[aux.range(2, 0)] = 0;
  IRAM.write(addr, data);
 }
 return;
}

void ac_behavior(jc)
{
 sc_uint<8> psw;
 sc_int<8> reladd;

 psw = IRAM.read(PSW);
 reladd = (sc_int<8>) byte2;
 if (psw[7] == 1)
  ac_pc = pc + reladd;
 return;
}

void ac_behavior(jnc)
{
 sc_uint<8> psw;
 sc_int<8> reladd;

 psw = IRAM.read(PSW);
 reladd = (sc_int<8>) byte2;
 if (psw[7] == 0)
  ac_pc = pc + reladd;
 return;
}

void ac_behavior(jmp)
{
 sc_uint<8> acc, temp;
 sc_uint<16> dptr;

 dptr.range(7, 0) = IRAM.read(DPTRL);
 dptr.range(15, 8) = IRAM.read(DPTRH);
 acc = IRAM.read(ACC);
 pc = acc + dptr;
 ac_pc = pc;
 return;
}

void ac_behavior(jz)
{
 sc_uint<8> acc;
 sc_int<8> reladd;

 acc = IRAM.read(ACC);
 reladd = (sc_int<8>) byte2;
 if (acc == 0)
  ac_pc = pc + reladd;
 return;
}

void ac_behavior(jnz)
{
 sc_uint<8> acc;
 sc_int<8> reladd;

 acc = IRAM.read(ACC);
 reladd = (sc_int<8>) byte2;
 if (acc != 0)
  ac_pc = pc + reladd;
 return;
}

void ac_behavior(nop)
{
 //nao faz nada
 return;
}

void ac_behavior(setb_c)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 psw[7] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(setb_bit)
{
 sc_uint<8> aux, data;
 int addr;

 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 if (data[aux.range(2, 0)] == 0)
  data[aux.range(2, 0)] = 1;
 IRAM.write(addr, data);
 return;
}

void ac_behavior(djnz_r)
{
 sc_uint<8> aux;
 sc_int<8> addrtmp = (sc_int<8>) addr;

 aux = IRAM.read(reg_indx);
 if (aux != 0)
  aux = aux - 1;
 else
  aux = 255;
 if (aux != 0)
  ac_pc = (pc + addrtmp);
 IRAM.write(reg_indx, aux);
 return;
}

void ac_behavior(djnz_iram_reladd)
{
 sc_uint<8> psw;
 sc_uint<8> aux;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 psw = IRAM.read(PSW);
 aux = IRAM.read(byte2);
 if (aux != 0)
  aux = aux - 1;
 else
  aux = 255;
 if (aux != 0)
  ac_pc = (pc + tempByte3);
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(lcall)
{
 sc_uint<9> aux;

 aux = IRAM.read(SP) + 1;
 IRAM.write(aux.range(7, 0), pc.range(7, 0));
 IRAM.write(SP, aux.range(7, 0));
 aux = IRAM.read(SP) + 1;
 IRAM.write(aux.range(7, 0), pc.range(15, 8));
 IRAM.write(SP, aux.range(7, 0));
 pc.range(7, 0) = byte3;
 pc.range(15, 8) = byte2;
 ac_pc = pc;
 return;
}

void ac_behavior(ljmp)
{
 pc.range(7, 0) = byte3;
 pc.range(15, 8) = byte2;
 ac_pc = pc;
 return;
}

void ac_behavior(sjmp)
{
 sc_int<8> tempByte2 = (sc_int<8>) byte2;

 ac_pc = pc + tempByte2;
 return;
}

void ac_behavior(swap)
{
 sc_uint<8> acc, temp;

 acc = IRAM.read(ACC);
 temp = acc;
 acc.range(7, 4) = temp.range(3, 0);
 acc.range(3, 0) = temp.range(7, 4);
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(xchd_R0)
{
 sc_uint<8> acc, temp, aux, psw;
 int reg_indx;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 temp = IRAM.read(IRAM.read(reg_indx));
 aux = temp;
 temp.range(3, 0) = acc.range(3, 0);
 acc.range(3, 0) = aux.range(3, 0);
 IRAM.write(ACC, acc);
 IRAM.write(IRAM.read(reg_indx), temp);
 return;
}

void ac_behavior(xchd_R1)
{
 sc_uint<8> acc, temp, aux, psw;

 int reg_indx;
 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 temp = IRAM.read(IRAM.read(reg_indx));
 aux = temp;
 temp.range(3, 0) = acc.range(3, 0);
 acc.range(3, 0) = aux.range(3, 0);
 IRAM.write(ACC, acc);
 IRAM.write(IRAM.read(reg_indx), temp);
 return;
}

void ac_behavior(add_ar)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 aux = IRAM.read(reg_indx);
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0);
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 sum = IRAM.read(ACC) + IRAM.read(reg_indx);
 IRAM.write(PSW, psw);
 IRAM.write(ACC, sum.range(7, 0));
 return;
}

void ac_behavior(add_a_data)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 aux = byte2;
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0);
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 sum = IRAM.read(ACC) + byte2;
 IRAM.write(PSW, psw);
 IRAM.write(ACC, sum.range(7, 0));
 return;
}

void ac_behavior(add_a_iram)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;
 bool bit7, bit8;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 psw[2] = 0;                    // clear Overflow bit
 psw[7] = 0;                    //clear carry bit
 psw[6] = 0;                    //clear nibble carry
 sum = 0;
 aux = IRAM.read(byte2);
 //checking overflow
 sum = acc.range(6, 0) + aux.range(6, 0);
 bit7 = sum[7];
 sum = 0;
 sum = acc.range(7, 0) + aux.range(7, 0);
 bit8 = sum[8];
 if (bit7 ^ bit8)
  psw[2] = 1;
 sum = 0;
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 //checking carry
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
  psw[7] = 1;
 sum = IRAM.read(ACC) + IRAM.read(byte2);
 IRAM.write(PSW, psw);
 IRAM.write(ACC, sum.range(7, 0));
 return;
}

void ac_behavior(add_arr_R0)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;
 int reg_indx;
 bool bit7, bit8;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 psw[2] = 0;                    // clear Overflow bit
 psw[7] = 0;                    //clear carry bit
 psw[6] = 0;                    //clear nibble carry
 sum = 0;
 aux = IRAM.read(IRAM.read(reg_indx));
 //checking overflow
 sum = acc.range(6, 0) + aux.range(6, 0);
 bit7 = sum[7];
 sum = 0;
 sum = acc.range(7, 0) + aux.range(7, 0);
 bit8 = sum[8];
 if (bit7 ^ bit8)
  psw[2] = 1;
 sum = 0;
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 //checking carry
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
  psw[7] = 1;
 sum = IRAM.read(IRAM.read(reg_indx)) + IRAM.read(ACC);
 IRAM.write(PSW, psw);
 IRAM.write(ACC, sum.range(7, 0));
 return;
}

void ac_behavior(add_arr_R1)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;
 bool bit7, bit8;
 int reg_indx;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 psw[2] = 0;                    // clear Overflow bit
 psw[7] = 0;                    //clear carry bit
 psw[6] = 0;                    //clear nibble carry
 sum = 0;
 aux = IRAM.read(IRAM.read(reg_indx));
 //checking overflow
 sum = acc.range(6, 0) + aux.range(6, 0);
 bit7 = sum[7];
 sum = 0;
 sum = acc.range(7, 0) + aux.range(7, 0);
 bit8 = sum[8];
 if (bit7 ^ bit8)
  psw[2] = 1;
 sum = 0;
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 //checking carry
 sum = 0;
 sum = acc + aux;               //sum all
 if (sum[8])
  psw[7] = 1;
 sum = IRAM.read(IRAM.read(reg_indx)) + IRAM.read(ACC);
 IRAM.write(PSW, psw);
 IRAM.write(ACC, sum.range(7, 0));
 return;
}

void ac_behavior(addc_ar)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 aux = IRAM.read(reg_indx);
 sum = IRAM.read(ACC) + IRAM.read(reg_indx) + psw[7];
 IRAM.write(ACC, sum.range(7, 0));
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0) + psw[7];
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(addc_a_data)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 aux = byte2;
 sum = IRAM.read(ACC) + byte2 + psw[7];
 IRAM.write(ACC, sum.range(7, 0));
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0) + psw[7];
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(addc_a_iram)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 sum = IRAM.read(ACC) + IRAM.read(byte2) + psw[7];
 IRAM.write(ACC, sum.range(7, 0));
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0) + psw[7];
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(addc_arr_R0)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;
 int reg_indx;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 aux = IRAM.read(IRAM.read(reg_indx));
 sum = IRAM.read(IRAM.read(reg_indx)) + IRAM.read(ACC) + psw[7];
 IRAM.write(ACC, sum.range(7, 0));
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0) + psw[7];
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(addc_arr_R1)
{
 sc_uint<8> psw;
 sc_uint<8> acc;
 sc_uint<8> aux;
 sc_uint<9> sum;
 int reg_indx;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 aux = IRAM.read(IRAM.read(reg_indx));
 sum = IRAM.read(IRAM.read(reg_indx)) + IRAM.read(ACC) + psw[7];
 IRAM.write(ACC, sum.range(7, 0));
 //checking overflow
 psw[2] = 0;                    // clear Overflow bit
 sum = 0;
 sum = acc.range(6, 0) + aux.range(6, 0) + psw[7];
 if (sum[7])
  psw[2] = 1;
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
 {
  if (psw[2])
   psw[2] = 0;
  else
   psw[2] = 1;
 }
 //checking auxiliary carry
 sum = 0;
 sum = acc.range(3, 0) + aux.range(3, 0); // sum nibble
 if (sum[4])
  psw[6] = 1;
 else
  psw[6] = 0;
 //checking carry
 sum = 0;
 sum = acc + aux + psw[7];      //sum all
 if (sum[8])
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(subb_ar)
{
 sc_uint<8> psw;
 sc_uint<8> acc, aux;
 sc_uint<9> temp;
 sc_int<9> num;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 temp = IRAM.read(ACC) - (IRAM.read(reg_indx) + psw[7]);
 aux = IRAM.read(reg_indx);
 IRAM.write(ACC, temp.range(7, 0));
 //checking overflow
 bool borrow7 = false;
 bool borrow6 = false;
 if (acc < (aux + psw[7]))
  borrow7 = true;
 if (acc.range(6, 0) < (aux.range(6, 0) + psw[7]))
  borrow6 = true;
 psw[2] = borrow7 ^ borrow6;
 //checking borrow (carry)
 psw[7] = borrow7;
 //checking auxiliary carry
 if (aux.range(3, 0) > (acc.range(3, 0) + psw[7]))
  psw[6] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(subb_a_data)
{
 sc_uint<9> sub;
 sc_uint<8> aux = byte2;
 sc_uint<8> acc = IRAM.read(ACC);
 sc_uint<8> psw = IRAM.read(PSW);

 sub = acc - (aux + psw[7]);
 IRAM.write(ACC, sub.range(7, 0));
 //checking overflow
 bool borrow7 = false;
 bool borrow6 = false;
 if (acc < (aux + psw[7]))
  borrow7 = true;
 if (acc.range(6, 0) < (aux.range(6, 0) + psw[1]))
  borrow6 = true;
 psw[2] = borrow7 ^ borrow6;
 //checking borrow (carry)
 psw[7] = borrow7;
 //checking auxiliary carry
 if (aux.range(3, 0) > (acc.range(3, 0) + psw[7]))
  psw[6] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(subb_a_iram)
{
 sc_uint<9> sub;
 sc_uint<8> aux = IRAM.read(byte2);
 sc_uint<8> acc = IRAM.read(ACC);
 sc_uint<8> psw = IRAM.read(PSW);

 sub = acc - (aux + psw[7]);
 IRAM.write(ACC, sub.range(7, 0));
 //checking overflow
 bool borrow7 = false;
 bool borrow6 = false;
 if (acc < (aux + psw[7]))
  borrow7 = true;
 if (acc.range(6, 0) < (aux.range(6, 0) + psw[7]))
  borrow6 = true;
 psw[2] = borrow7 ^ borrow6;
 //checking borrow (carry)
 psw[7] = borrow7;
 //checking auxiliary carry
 if (aux.range(3, 0) > (acc.range(3, 0) + psw[7]))
  psw[6] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(subb_a_arr_R0)
{
 sc_uint<8> psw;
 sc_uint<9> sub;
 sc_uint<8> acc, aux;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 aux = IRAM.read(IRAM.read(reg_indx));
 sub = acc - (aux + psw[7]);
 IRAM.write(ACC, sub.range(7, 0));
 //checking overflow
 bool borrow7 = false;
 bool borrow6 = false;
 if (acc < (aux + psw[7]))
  borrow7 = true;
 if (acc.range(6, 0) < (aux.range(6, 0) + psw[7]))
  borrow6 = true;
 psw[2] = borrow7 ^ borrow6;
 //checking borrow (carry)
 psw[7] = borrow7;
 //checking auxiliary carry
 if (aux.range(3, 0) > (acc.range(3, 0) + psw[7]))
  psw[6] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(subb_a_arr_R1)
{
 sc_uint<8> psw;
 sc_uint<8> acc, aux;
 sc_uint<9> sub;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 aux = IRAM.read(IRAM.read(reg_indx));
 sub = acc - (aux + psw[7]);
 IRAM.write(ACC, sub.range(7, 0));
 //checking overflow
 bool borrow7 = 0, borrow6 = 0;
 if (acc < (aux + psw[7]))
  borrow7 = true;
 if (acc.range(6, 0) < (aux.range(6, 0) + psw[7]))
  borrow6 = true;
 psw[2] = borrow7 ^ borrow6;
 //checking borrow (carry)
 psw[7] = borrow7;
 //checking auxiliary carry
 if (aux.range(3, 0) > (acc.range(3, 0) + psw[7]))
  psw[6] = 1;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(mov_arr_R0_data)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(IRAM.read(reg_indx), byte2);
 return;
}

void ac_behavior(mov_arr_R1_data)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(IRAM.read(reg_indx), byte2);
 return;
}

void ac_behavior(mov_arr_R0_a)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(IRAM.read(reg_indx), IRAM.read(ACC));
 return;
}

void ac_behavior(mov_arr_R1_a)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(IRAM.read(reg_indx), IRAM.read(ACC));
 return;
}

void ac_behavior(mov_arr_R0_iram)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(IRAM.read(reg_indx), IRAM.read(byte2));
 return;
}

void ac_behavior(mov_arr_R1_iram)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(IRAM.read(reg_indx), IRAM.read(byte2));
 return;
}

void ac_behavior(mov_iram_arr_R0)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(byte2, IRAM.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(mov_iram_arr_R1)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(byte2, IRAM.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(mov_a_data)
{
 IRAM.write(ACC, byte2);
 return;
}

void ac_behavior(mov_a_arr_R0)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(ACC, IRAM.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(mov_a_arr_R1)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(ACC, IRAM.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(mov_ar)
{
 IRAM.write(ACC, IRAM.read(reg_indx));
 return;
}

void ac_behavior(mov_a_iram)
{
 IRAM.write(ACC, IRAM.read(byte2));
 return;
}

void ac_behavior(mov_iram_a)
{
 IRAM.write(byte2, IRAM.read(ACC));
 return;
}

void ac_behavior(mov_c_bit)
{
 sc_uint<8> aux, data, psw;
 int addr;

 psw = IRAM.read(PSW);
 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 psw[7] = data[aux.range(2, 0)];
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(mov_bit_c)
{
 sc_uint<8> aux, data, psw;
 int addr;

 psw = IRAM.read(PSW);
 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 data[aux.range(2, 0)] = psw[7];
 IRAM.write(addr, data);
 return;
}

void ac_behavior(mov_iram_data)
{
 IRAM.write(byte2, byte3);
 return;
}

void ac_behavior(mov_dptr_data)
{
 IRAM.write(DPTRH, byte2);
 IRAM.write(DPTRL, byte3);
 return;
}

void ac_behavior(mov_r_data)
{
 IRAM.write(reg_indx, addr);
 return;
}

void ac_behavior(mov_r_iram)
{
 IRAM.write(reg_indx, IRAM.read(addr));
 return;
}

void ac_behavior(mov_iram_r)
{
 IRAM.write(addr, IRAM.read(reg_indx));
 return;
}

void ac_behavior(mov_ra)
{
 IRAM.write(reg_indx, IRAM.read(ACC));
 return;
}

void ac_behavior(mov_iram_iram)
{
 IRAM.write(byte3, IRAM.read(byte2));
 return;
}

void ac_behavior(movc_dptr)
{
 sc_uint<16> dptr;
 sc_uint<8> acc;

 dptr.range(7, 0) = IRAM.read(DPTRL);
 dptr.range(15, 8) = IRAM.read(DPTRH);
 acc = IRAM.read(ACC);
 IRAM.write(ACC, IROM.read(acc + dptr));
 return;
}

void ac_behavior(movc_pc)
{
 sc_uint<16> pc = (sc_uint<16>) ac_pc.read();
 sc_uint<8> acc = (sc_uint<8>) IRAM.read(ACC);
 IRAM.write(ACC, IROM.read(acc + pc));
 return;
}

void ac_behavior(acall)
{
 sc_uint<9> aux;

 aux = IRAM.read(SP) + 1;
 IRAM.write(aux.range(7, 0), pc.range(7, 0));
 IRAM.write(SP, aux.range(7, 0));
 aux = IRAM.read(SP) + 1;
 IRAM.write(aux.range(7, 0), pc.range(15, 8));
 IRAM.write(SP, aux.range(7, 0));
 pc.range(10, 8) = page;
 pc.range(7, 0) = addr0;
 ac_pc = (unsigned int) pc;
 return;
}

void ac_behavior(ajmp)
{
 pc.range(10, 8) = page;
 pc.range(7, 0) = addr0;
 ac_pc = (unsigned int) pc;
 return;
}

void ac_behavior(cjne_addr)
{
 sc_uint<8> psw, acc;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (acc != (unsigned) IRAM.read(byte2))
  ac_pc = (pc + tempByte3);
 if (IRAM.read(ACC) < IRAM.read(byte2))
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(cjne_data)
{
 sc_uint<8> psw, acc;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 psw = IRAM.read(PSW);
 acc = IRAM.read(ACC);
 if (acc != byte2)
  ac_pc = (pc + tempByte3);
 if (acc < byte2)
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(cjne_arr_R0)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 sc_int<8> tempByte3 = (sc_int<8>) byte3;
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 if (IRAM.read(IRAM.read(reg_indx)) != byte2)
  ac_pc = (pc + tempByte3);
 if (IRAM.read(IRAM.read(reg_indx)) < byte2)
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(cjne_arr_R1)
{
 sc_uint<8> psw;
 sc_int<8> tempByte3 = (sc_int<8>) byte3;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 if (IRAM.read(IRAM.read(reg_indx)) != byte2)
  ac_pc = (pc + tempByte3);
 if (IRAM.read(IRAM.read(reg_indx)) < byte2)
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(cjne_r)
{
 sc_uint<8> psw;
 sc_int<8> tempReladd = (sc_int<8>) reladd;

 psw = IRAM.read(PSW);
 if (IRAM.read(reg_indx) != data)
  ac_pc = (pc + tempReladd);
 if (IRAM.read(reg_indx) < data)
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(clr_bit)
{
 sc_uint<8> aux, data;
 int addr;

 aux = byte2;
 if (aux < 128)
  addr = aux.range(6, 3) + 32;
 else
  addr = aux.range(6, 3) * 8 + 128;
 data = IRAM.read(addr);
 data[aux.range(2, 0)] = 0;
 IRAM.write(addr, data);
 return;
}

void ac_behavior(clr_c)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(clr_a)
{
 IRAM.write(ACC, 0);
 return;
}

void ac_behavior(pop)
{
 sc_uint<8> value;
 unsigned idx;

 idx = IRAM.read(SP);
 value = IRAM.read(idx);
 idx = idx - 1;
 IRAM.write(SP, idx);
 IRAM.write(byte2, value);
 return;
}

void ac_behavior(push)
{
 sc_uint<8> stack = IRAM.read(SP);

 stack = stack + 1;
 IRAM.write(SP, stack);
 IRAM.write(stack, IRAM.read(byte2));
 return;
}

void ac_behavior(anl_iram_data)
{
 sc_uint<8> aux, data;

 aux = IRAM.read(byte2);
 data = byte3;
 aux = aux & data;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(orl_iram_data)
{
 sc_uint<8> aux, data;

 aux = IRAM.read(byte2);
 data = byte3;
 aux = aux | data;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(xrl_iram_data)
{
 sc_uint<8> aux, data;

 aux = IRAM.read(byte2);
 data = byte3;
 aux = aux ^ data;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(anl_iram_a)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux & acc;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(orl_iram_a)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux | acc;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(xrl_iram_a)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux ^ acc;
 IRAM.write(byte2, aux);
 return;
}

void ac_behavior(anl_a_data)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = byte2;
 aux = aux & acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(orl_a_data)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = byte2;
 aux = aux | acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(xrl_a_data)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = byte2;
 aux = aux ^ acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(anl_a_iram)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux & acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(orl_a_iram)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux | acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(xrl_a_iram)
{
 sc_uint<8> aux, acc;

 acc = IRAM.read(ACC);
 aux = IRAM.read(byte2);
 aux = aux ^ acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(anl_arr_R0)
{
 sc_uint<8> tmpA, acc, psw;

 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA & acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(anl_arr_R1)
{
 sc_uint<8> tmpA, acc, psw;
 int reg_indx;

 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 acc = IRAM.read(ACC);
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA & acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(orl_arr_R0)
{
 sc_uint<8> tmpA, acc, psw;

 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA | acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(orl_arr_R1)
{
 sc_uint<8> tmpA, acc, psw;
 int reg_indx;

 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 acc = IRAM.read(ACC);
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA | acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(xrl_arr_R0)
{
 sc_uint<8> tmpA, acc, psw;

 acc = IRAM.read(ACC);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA ^ acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(xrl_arr_R1)
{
 sc_uint<8> tmpA, acc, psw;
 int reg_indx;

 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 acc = IRAM.read(ACC);
 tmpA = IRAM.read(IRAM.read(reg_indx));
 tmpA = tmpA ^ acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(anl_c_bit)
{
 sc_uint<8> aux, psw, temp, end;

 psw = IRAM.read(PSW);
 temp = byte2;
 if (temp > 128)
  aux = IRAM.read(temp.range(6, 3) * 8 + 128);
 else
  aux = IRAM.read(temp.range(6, 3) + 32);
 psw[7] = psw[7] & aux[temp.range(2, 0)];
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(anl_c_nbit)
{
 sc_uint<8> aux, psw, temp;

 psw = IRAM.read(PSW);
 temp = byte2;

 if (temp > 128)
  aux = IRAM.read(temp.range(6, 3) * 8 + 128);
 else
  aux = IRAM.read(temp.range(6, 3) + 32);
 unsigned idx = temp.range(2, 0);
 bool bit;
 //complement bit
 if (aux[idx] == 1)
  bit = false;
 else
  bit = true;
 psw[7] = psw[7] & bit;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(orl_c_bit)
{
 sc_uint<8> aux, psw, temp;

 psw = IRAM.read(PSW);
 temp = byte2;
 if (temp > 128)
  aux = IRAM.read(temp.range(6, 3) * 8 + 128);
 else
  aux = IRAM.read(temp.range(6, 3) + 32);
 psw[7] = psw[7] | aux[temp.range(2, 0)];
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(orl_c_nbit)
{
 sc_uint<8> aux, psw, temp;

 psw = IRAM.read(PSW);
 temp = byte2;
 if (temp > 128)
  aux = IRAM.read(temp.range(6, 3) * 8 + 128);
 else
  aux = IRAM.read(temp.range(6, 3) + 32);
 psw[7] = psw[7] | !(aux[temp.range(2, 0)]);
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(anl_ar)
{
 sc_uint<8> psw, acc, aux;
// int reg_indx;

/* psw = IRAM.read(PSW);
 if (psw.range(4,3) == 0)
  reg_indx = reg;
	else	if (psw.range(4,3) == 1)
  reg_indx = reg+8;
	else if (psw.range(4,3) == 2)
  reg_indx = reg+16;
	else
  reg_indx = reg+24;
*/
 aux = IRAM.read(reg_indx);
 acc = IRAM.read(ACC);
 aux = aux & acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(orl_ar)
{
 sc_uint<8> psw, acc, aux;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = reg;
 else if (psw.range(4, 3) == 1)
  reg_indx = reg + 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = reg + 16;
 else
  reg_indx = reg + 24;
 aux = IRAM.read(reg_indx);
 acc = IRAM.read(ACC);
 aux = aux | acc;
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(xrl_ar)
{
 sc_uint<8> psw, acc, tmpA;

 psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = reg;
 else if (psw.range(4, 3) == 1)
  reg_indx = reg + 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = reg + 16;
 else
  reg_indx = reg + 24;
 tmpA = IRAM.read(reg_indx);
 acc = IRAM.read(ACC);
 tmpA = tmpA ^ acc;
 IRAM.write(ACC, tmpA);
 return;
}

void ac_behavior(cpl_a)
{
 sc_uint<8> aux;
 int i;

 aux = IRAM.read(ACC);
 for (i = 0; i <= 7; i++)
 {
  if (aux[i] == 0)
   aux[i] = 1;
  else
   aux[i] = 0;
 }
 IRAM.write(ACC, aux);
 return;
}

void ac_behavior(cpl_c)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 if (psw[7] == 0)
  psw[7] = 1;
 else
  psw[7] = 0;
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(cpl_bit)
{
 sc_uint<8> aux, temp, end;

 temp = byte2;
 if (temp < 128)
 {
  aux = IRAM.read(temp.range(6, 3) + 32);
  end = temp.range(6, 3) + 32;
 }
 else
 {
  aux = IRAM.read(temp.range(6, 3) * 8 + 128);
  end = temp.range(6, 3) * 8 + 128;
 }
 if (aux[temp.range(2, 0)] == 0)
  aux[temp.range(2, 0)] = 1;
 else
  aux[temp.range(2, 0)] = 0;
 IRAM.write(end, aux);
 return;
}

void ac_behavior(inc_r)
{
 sc_uint<8> aux;

 aux = IRAM.read(reg_indx);
 if (aux == 255)
  IRAM.write(reg_indx, 0);
 else
  IRAM.write(reg_indx, aux + 1);
 return;
}

void ac_behavior(inc_a)
{
 if (IRAM.read(ACC) == 255)
  IRAM.write(ACC, 0);
 else
  IRAM.write(ACC, IRAM.read(ACC) + 1);
 return;
}

void ac_behavior(inc_dptr)
{
 sc_uint<16> temp;
 sc_uint<8> dpl;
 sc_uint<8> dph;

 dpl = IRAM.read(DPTRL);
 dph = IRAM.read(DPTRH);
 temp.range(15, 8) = dph;
 temp.range(7, 0) = dpl;
 if (temp == 255)
 {
  IRAM.write(DPTRL, 0);
  IRAM.write(DPTRH, 0);
 }
 else
 {
  temp = temp + 1;
  IRAM.write(DPTRH, temp.range(15, 8));
  IRAM.write(DPTRL, temp.range(7, 0));
 }
 return;
}

void ac_behavior(inc_iram)
{
 if (IRAM.read(byte2) == 255)
  IRAM.write(byte2, 0);
 else
  IRAM.write(byte2, IRAM.read(byte2) + 1);
 return;
}

void ac_behavior(inc_arr_R0)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 int reg_indx;
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 if (IRAM.read(IRAM.read(reg_indx)) == 255)
  IRAM.write(IRAM.read(reg_indx), 0);
 else
  IRAM.write(IRAM.read(reg_indx), IRAM.read(IRAM.read(reg_indx)) + 1);
 return;
}

void ac_behavior(inc_arr_R1)
{
 sc_uint<8> psw;

 psw = IRAM.read(PSW);
 int reg_indx;
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 if (IRAM.read(IRAM.read(reg_indx)) == 255)
  IRAM.write(IRAM.read(reg_indx), 0);
 else
  IRAM.write(IRAM.read(reg_indx), IRAM.read(IRAM.read(reg_indx)) + 1);
 return;
}

void ac_behavior(ret)
{
 sc_uint<8> lsb, msb, index;
 sc_uint<16> pc;

 index = IRAM.read(SP);
 msb = IRAM.read(index);
 index--;
 IRAM.write(SP, index);
 lsb = IRAM.read(index);
 index--;
 IRAM.write(SP, index);
 pc.range(7, 0) = lsb;
 pc.range(15, 8) = msb;
 ac_pc = (unsigned int) pc;
 return;
}

void ac_behavior(rr_a)
{
 sc_uint<8> aux, acc;
 int i;

 acc = IRAM.read(ACC);
 aux = acc;
 for (i = 6; i >= 0; i--)
  acc[i] = aux[i + 1];
 acc[7] = aux[0];
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(rl_a)
{
 sc_uint<8> aux, acc;
 int i;

 acc = IRAM.read(ACC);
 aux = acc;
 for (i = 1; i <= 7; i++)
  acc[i] = aux[i - 1];
 acc[0] = aux[7];
 IRAM.write(ACC, acc);
 return;
}

void ac_behavior(rrc_a)
{
 sc_uint<8> acc, psw;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 bool tmp = psw[7];
 psw[7] = acc[0];
 acc.range(6, 0) = acc.range(7, 1);
 acc[7] = tmp;
 IRAM.write(ACC, acc);
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(rlc_a)
{
 sc_uint<8> acc, psw;

 acc = IRAM.read(ACC);
 psw = IRAM.read(PSW);
 bool tmp = psw[7];
 psw[7] = acc[7];
 acc.range(7, 1) = acc.range(6, 0);
 acc[0] = tmp;
 IRAM.write(ACC, acc);
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(dec_iram)
{
 if (IRAM.read(byte2) == 0)
  IRAM.write(byte2, 255);
 else
  IRAM.write(byte2, IRAM.read(byte2) - 1);
 return;
}

void ac_behavior(dec_a)
{
 if (IRAM.read(ACC) == 0)
  IRAM.write(ACC, 255);
 else
  IRAM.write(ACC, IRAM.read(ACC) - 1);
 return;
}

void ac_behavior(dec_arr_R0)
{
 sc_uint<8> psw;
 unsigned idx;
 sc_uint<8> value;
 
 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 idx = IRAM.read(reg_indx);
 value = IRAM.read(idx) - 1;
 if (IRAM.read(idx) == 0)
  IRAM.write(idx, 255);
 else
  IRAM.write(idx, value);
 return;
}

void ac_behavior(dec_arr_R1)
{
 sc_uint<8> psw;
 int reg_indx;

 psw = IRAM.read(PSW);
 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 if (IRAM.read(IRAM.read(reg_indx)) == 0)
  IRAM.write(IRAM.read(reg_indx), 255);
 else
  IRAM.write(IRAM.read(reg_indx), IRAM.read(IRAM.read(reg_indx)) - 1);
 return;
}

void ac_behavior(dec_r)
{
 sc_uint<8> aux;

 aux = IRAM.read(reg_indx);
 if (aux == 255)
  IRAM.write(reg_indx, 0);
 else
  IRAM.write(reg_indx, aux - 1);
 return;
}

void ac_behavior(mul)
{
 sc_uint<8> acc, b, psw;
 sc_uint<16> result;

 acc = IRAM.read(ACC);
 b = IRAM.read(B);
 psw = IRAM.read(PSW);
 result = acc * b;
 psw[7] = 0;
 if (result > 255)
  psw[2] = 1;
 else
  psw[2] = 0;
 IRAM.write(ACC, result.range(7, 0));
 IRAM.write(B, result.range(15, 8));
 IRAM.write(PSW, psw);
 return;
}

void ac_behavior(div)
{
 sc_uint<8> acc, b, psw;
 sc_uint<8> result, mod;

 acc = IRAM.read(ACC);
 b = IRAM.read(B);
 psw = IRAM.read(PSW);
 if (b != 0)
 {
  result = acc / b;
  mod = acc % b;
  psw[7] = 0;
  IRAM.write(ACC, result);
  IRAM.write(B, mod);
  IRAM.write(PSW, psw);
 }
 else
 {
  psw[2] = 1;
  IRAM.write(PSW, psw);
 }
 return;
}

void ac_behavior(movx_dptr_a)
{
 sc_uint<16> dptr;

 dptr = IRAM.read(DPTRL);
 dptr.range(15, 8) = IRAM.read(DPTRH);
 IRAMX.write(dptr, IRAM.read(ACC));
 return;
}

void ac_behavior(movx_r0_a)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAMX.write(IRAM.read(reg_indx), IRAM.read(ACC));
 return;
}

void ac_behavior(movx_r1_a)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAMX.write(IRAM.read(reg_indx), IRAM.read(ACC));
 return;
}

void ac_behavior(reti)
{
 sc_uint<16> pc;

 pc.range(15, 8) = IRAM.read(IRAM.read(SP));
 IRAM.write(SP, (IRAM.read(SP) - 1));
 pc.range(7, 0) = IRAM.read(IRAM.read(SP));
 IRAM.write(SP, (IRAM.read(SP) - 1));
 ac_pc = (unsigned int) pc;
 return;
}

void ac_behavior(movx_a_dptr)
{
 sc_uint<16> address;

 address.range(7, 0) = IRAM.read(DPTRL);
 address.range(15, 8) = IRAM.read(DPTRH);
 IRAM.write(ACC, IRAMX.read(address));
 return;
}

void ac_behavior(movx_a_R0)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = 0;
 else if (psw.range(4, 3) == 1)
  reg_indx = 8;
 else if (psw.range(4, 3) == 2)
  reg_indx = 16;
 else
  reg_indx = 24;
 IRAM.write(ACC, IRAMX.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(movx_a_R1)
{
 sc_uint<8> psw = IRAM.read(PSW);

 if (psw.range(4, 3) == 0)
  reg_indx = 1;
 else if (psw.range(4, 3) == 1)
  reg_indx = 9;
 else if (psw.range(4, 3) == 2)
  reg_indx = 17;
 else
  reg_indx = 25;
 IRAM.write(ACC, IRAMX.read(IRAM.read(reg_indx)));
 return;
}

void ac_behavior(da)
{
 sc_uint<9> sum;
 sc_uint<8> tempPSW = IRAM.read(PSW);
 sc_uint<8> tempACC = IRAM.read(ACC);

 if (tempPSW[6] || tempACC.range(3, 0) > 9)
 {
  IRAM.write(ACC, IRAM.read(ACC) + 0x06);
  sum = tempACC + 6;
  tempPSW[7] = sum[8];
 }
 if (tempPSW[7] || tempACC.range(7, 4) > 9)
  IRAM.write(ACC, IRAM.read(ACC) + 0x60);
 if (tempACC > 0x99)
  IRAM.write(PSW, (IRAM.read(PSW) | 0x80));
 return;
}
