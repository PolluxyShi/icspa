#include "memory/mmu/cache.h"
#include "memory/memory.h"
#include <stdlib.h>

#ifdef CACHE_ENABLED

CacheLine cache[1024];

// init the cache
void init_cache()
{
	// implement me in PA 3-1
	for(int i = 0; i < 1024; ++i) cache[i].valid_bit = 0;
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
    // 写入内存
    memcpy(hw_mem + paddr, &data, len);

    // 写入cache
    
        // 取得 行内偏移量，组号，标志位
    unsigned offset = paddr & 0x3f; // 低6位
    unsigned grp = ((unsigned)paddr >> 6) & 0x7f; // 中间7位
    unsigned sign = (((unsigned)paddr) >> 13) & 0x7ffff; // 高19位
    
        // 取得cache中地址，首先查找是否命中，若命中则写入，未命中则不写入
    for(int i = 0; i < 8; ++i){
        if(cache[8*grp+i].valid_bit && cache[8*grp+i].sign == sign){
            if(64 - offset < len){
                size_t nlen = len - (64 - offset);
                len = 64 - offset;
                paddr_t naddr = paddr + len;
                unsigned noffset = naddr & 0x3f; // 低6位
                unsigned ngrp = ((unsigned)naddr >> 6) & 0x7f; // 中间7位
                unsigned nsign = (((unsigned)naddr) >> 13) & 0x7ffff; // 高19位
                for(int j = 0; j < 8; ++j){
                    if(cache[8*ngrp+j].valid_bit && cache[8*ngrp+j].sign == nsign){
                        memcpy(cache[8*ngrp+j].block + noffset,&data+len,nlen);
                        break;
                    }
                }
            }
            memcpy(cache[8*grp+i].block + offset,&data,len);
            return;
        }
    }

	// implement me in PA 3-1
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
    // 取得 行内偏移量，组号，标志位
    unsigned offset = paddr & 0x3f; // 低6位
    unsigned grp = ((unsigned)paddr >> 6) & 0x7f; // 中间7位
    unsigned sign = (((unsigned)paddr) >> 13) & 0x7ffff; // 高19位
    
    // 取得cache中地址，首先查找是否命中，若未命中则查找是否有空闲行，若有则直接载入，若无则进行替换
    uint8_t* cache_ptr = NULL;
    for(int i = 0; i < 8; ++i){
        if(cache[8*grp+i].valid_bit && cache[8*grp+i].sign == sign){
            cache_ptr = cache[8*grp+i].block + offset;
            break;
        }
    }
    if(!cache_ptr){ // 未命中
        for(int i = 0; i < 8; ++i){
            if(!cache[8*grp+i].valid_bit){
                cache[8*grp+i].valid_bit = 1;
                cache[8*grp+i].sign = sign;
                memcpy(cache[8*grp+i].block, hw_mem + paddr - offset, 64);
                cache_ptr = cache[8*grp+i].block + offset;
                break;
            }
        }
        if(!cache_ptr){ // 没有空闲行
            srand((unsigned)time(NULL));
            int i = rand() % 8;
            cache[8*grp+i].sign = sign;
            memcpy(cache[8*grp+i].block, hw_mem + paddr - offset, 64);
            cache_ptr = cache[8*grp+i].block + offset;
        }
    }
    
    // 取值，注意跨块
    if(offset + len > 64){ // 跨块
        uint32_t ret_l = 0;
	    memcpy(&ret_l, cache_ptr, 64 - offset);
	    uint32_t ret_h = cache_read(paddr + (64-offset), len - (64-offset));
	    ret_h = ret_h << (8 * (64 - offset));
	    return ret_l + ret_h;
    }else{
        uint32_t ret = 0;
	    memcpy(&ret, cache_ptr, len);
	    return ret;
    }
    
	// implement me in PA 3-1
	return 0;
}

#endif

