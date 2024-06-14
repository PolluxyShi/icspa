#include "cpu/cpu.h"

// Set_Flag
void set_OF_add(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    switch(data_size){
        case 8:
            result = sign_ext(result & 0xff, 8);
            src = sign_ext(src & 0xff, 8);
            dest = sign_ext(dest & 0xff, 8);
            break;
        case 16:
            result = sign_ext(result & 0xffff, 16);
            src = sign_ext(src & 0xffff, 16);
            dest = sign_ext(dest & 0xffff, 16);
            break;
        default:break;
    }
    if(sign(src) == sign(dest)){
        if(sign(src) != sign(result)){
            cpu.eflags.OF = 1;
        }else{
            cpu.eflags.OF = 0;
        }
    }else{
        cpu.eflags.OF = 0;
    }
}

void set_OF_adc(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    set_OF_add(result,src,dest,data_size);
}

void set_OF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    switch(data_size){
        case 8:
            result = sign_ext(result & 0xff, 8);
            src = sign_ext(src & 0xff, 8);
            dest = sign_ext(dest & 0xff, 8);
            break;
        case 16:
            result = sign_ext(result & 0xffff, 16);
            src = sign_ext(src & 0xffff, 16);
            dest = sign_ext(dest & 0xffff, 16);
            break;
        default:break;
    }
    if(sign(src) != sign(dest)){
        if(sign(dest) != sign(result)){
            cpu.eflags.OF = 1;
        }else{
            cpu.eflags.OF = 0;
        }
    }else{
        cpu.eflags.OF = 0;
    }
}

void set_OF_sbb(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    set_OF_sub(result,src,dest,data_size);
}

void set_OF_mul(uint64_t result,size_t data_size){
    result &= (0xffffffffffffffff >> (64 - 2*data_size));
    cpu.eflags.OF = ((result >> data_size) != 0);
}

void set_CF_add(uint32_t result, uint32_t src, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xffffffff >> (32 - data_size)), data_size);
    cpu.eflags.CF = result < src;
}

void set_CF_adc(uint32_t result, uint32_t src, uint32_t cf, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xffffffff >> (32 - data_size)), data_size);
    if(cf == 0){
        cpu.eflags.CF = result < src;
    }else{
        cpu.eflags.CF = result <= src;
    }
}

void set_CF_sub(uint32_t result, uint32_t dest, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xffffffff >> (32 - data_size)), data_size);
    cpu.eflags.CF = result > dest;
}

void set_CF_sbb(uint32_t result, uint32_t dest, uint32_t cf, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xffffffff >> (32 - data_size)), data_size);
    if(cf == 0){
        cpu.eflags.CF = result > dest;
    }else{
        cpu.eflags.CF = result >= dest;
    }
}

void set_CF_shl(uint32_t src, uint32_t dest, size_t data_size){
    cpu.eflags.CF = (dest >> (data_size-src)) & 0x1;
}

void set_CF_shr(uint32_t src, uint32_t dest, size_t data_size){
	cpu.eflags.CF = (dest >> (src-1)) & 0x1;
}

void set_CF_sar(uint32_t src, uint32_t dest, size_t data_size){
	set_CF_shr(src,dest,data_size);
}

void set_CF_mul(uint64_t result,size_t data_size){
    result &= (0xffffffffffffffff >> (64 - 2*data_size));
    cpu.eflags.CF = ((result >> data_size) != 0);
}

void set_SF(uint32_t result, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    cpu.eflags.SF = sign(result);
}

void set_ZF(uint32_t result, size_t data_size){
    result = sign_ext(result & (0xffffffff >> (32 - data_size)), data_size);
    cpu.eflags.ZF = (result == 0);
}
void set_PF(uint32_t result){
    uint32_t tmp = result;
    int cnt = 0;
    for(int i = 0; i < 8; ++i){
        if((tmp&0x1) == 0) cnt++;
        tmp >>= 1;
    }
    cpu.eflags.PF = (cnt%2 == 0);
}

// Instruction
uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src;       //获取计算结果
	
	set_OF_add(res,src,dest,data_size);     //设置标志位
	set_CF_add(res,src,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest + src + (uint32_t)cpu.eflags.CF;       //获取计算结果
	
	set_OF_adc(res,src,dest,data_size);     //设置标志位
	set_CF_adc(res,src,(uint32_t)cpu.eflags.CF,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src;       //获取计算结果
	
	set_OF_sub(res,src,dest,data_size);     //设置标志位
	set_CF_sub(res,dest,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest - src - (uint32_t)cpu.eflags.CF;       //获取计算结果
	
	set_OF_sbb(res,src,dest,data_size);     //设置标志位
	set_CF_sbb(res,dest,(uint32_t)cpu.eflags.CF,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
	uint64_t res = 0;
	src &= (0xffffffff >> (32 - data_size));
	dest &= (0xffffffff >> (32 - data_size));
	res = ((uint64_t)dest) * src;       //获取计算结果

	set_OF_mul(res,data_size);     //设置标志位
	set_CF_mul(res,data_size);
	
	return res;
#endif
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t res = 0;
	src = sign_ext(src & (0xffffffff >> (32 - data_size)),data_size);
	dest = sign_ext(dest & (0xffffffff >> (32 - data_size)),data_size);
	res = ((int64_t)dest) * src;       //获取计算结果
	
	return res;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	uint32_t res = 0;
	src &= (0xffffffffffffffff >> (64 - data_size));
	dest &= (0xffffffffffffffff >> (64 - data_size));
	assert(src != 0);
	res = ((uint64_t)dest) / src;       //获取计算结果
	
	return res;
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	int32_t res = 0;
	src = sign_ext_64(src & (0xffffffffffffffff >> (64 - data_size)),data_size);
	dest = sign_ext_64(dest & (0xffffffffffffffff >> (64 - data_size)),data_size);
	assert(src != 0);

    int sign1 = sign(src), sign2 = sign(dest);
    if(sign1) src = (~src + 1);
    if(sign2) dest = (~dest + 1);
	res = dest / src;       //获取计算结果
    if(sign1 != sign2) res = (~res + 1);
	
	return res;
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	uint32_t res = 0;
	assert(src != 0);
	res = dest % src;       //获取计算结果
	
	return res;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	int32_t res = 0;
	assert(src != 0);
	res = dest % src;       //获取计算结果
	
	return res;
#endif
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest & src;       //获取计算结果
	
	cpu.eflags.OF = 0;    //设置标志位
	cpu.eflags.CF = 0;
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest ^ src;       //获取计算结果
	
	cpu.eflags.OF = 0;    //设置标志位
	cpu.eflags.CF = 0;
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t res = 0;
	res = dest | src;       //获取计算结果
	
	cpu.eflags.OF = 0;    //设置标志位
	cpu.eflags.CF = 0;
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	uint32_t res = 0;
	res = ((dest & (0xffffffff >> (32 - data_size))) << src);       //获取计算结果
	
	set_CF_shl(src,dest,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)   // CF=移出的最低位 高位补0
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
	uint32_t res = 0;
	res = ((dest & (0xffffffff >> (32 - data_size))) >> src);       //获取计算结果
	
    set_CF_shr(src,dest,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)     // CF=移出的最低位 高位补符
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
	uint32_t res = 0;
	if(sign(sign_ext(dest & (0xffffffff >> (32 - data_size)),data_size)) == 1){
	    res = ((dest | (0xffffffff << data_size)) >> src) | (0xffffffff << data_size);
	}else{
	    res = ((dest & (0xffffffff >> (32 - data_size))) >> src);       //获取计算结果
	}
	
    set_CF_sar(src,dest,data_size);
	set_SF(res,data_size);
	set_ZF(res,data_size);
	set_PF(res);
	
	return res & (0xffffffff >> (32 - data_size));
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	return alu_shl(src,dest,data_size);
#endif
}
