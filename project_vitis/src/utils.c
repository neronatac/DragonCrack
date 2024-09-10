#include "utils.h"
#include "xil_mem.h"

extern u8 response_buffer[];

void add_u8_buf_to_response(u8* buffer, u32 len) {
    Xil_MemCpy(response_buffer+response_buffer[0]+1, buffer, len);
    response_buffer[0] += len;
}

void add_u32_to_response(u32 val) {
    response_buffer[response_buffer[0] + 1] = (val >> 24) & 0xFF;
    response_buffer[response_buffer[0] + 2] = (val >> 16) & 0xFF;
    response_buffer[response_buffer[0] + 3] = (val >> 8) & 0xFF;
    response_buffer[response_buffer[0] + 4] = (val >> 0) & 0xFF;
    response_buffer[0] += 4;
}

void add_u64_to_response(u64 val) {
    response_buffer[response_buffer[0] + 1] = (val >> 56) & 0xFF;
    response_buffer[response_buffer[0] + 2] = (val >> 48) & 0xFF;
    response_buffer[response_buffer[0] + 3] = (val >> 40) & 0xFF;
    response_buffer[response_buffer[0] + 4] = (val >> 32) & 0xFF;
    response_buffer[response_buffer[0] + 5] = (val >> 24) & 0xFF;
    response_buffer[response_buffer[0] + 6] = (val >> 16) & 0xFF;
    response_buffer[response_buffer[0] + 7] = (val >> 8) & 0xFF;
    response_buffer[response_buffer[0] + 8] = (val >> 0) & 0xFF;
    response_buffer[0] += 8;
}

void add_u56_to_response(u64 val) {
    response_buffer[response_buffer[0] + 1] = (val >> 48) & 0xFF;
    response_buffer[response_buffer[0] + 2] = (val >> 40) & 0xFF;
    response_buffer[response_buffer[0] + 3] = (val >> 32) & 0xFF;
    response_buffer[response_buffer[0] + 4] = (val >> 24) & 0xFF;
    response_buffer[response_buffer[0] + 5] = (val >> 16) & 0xFF;
    response_buffer[response_buffer[0] + 6] = (val >> 8) & 0xFF;
    response_buffer[response_buffer[0] + 7] = (val >> 0) & 0xFF;
    response_buffer[0] += 7;
}

void add_int_to_response(int val) {
    response_buffer[response_buffer[0] + 1] = (val >> 24) & 0xFF;
    response_buffer[response_buffer[0] + 2] = (val >> 16) & 0xFF;
    response_buffer[response_buffer[0] + 3] = (val >> 8) & 0xFF;
    response_buffer[response_buffer[0] + 4] = (val >> 0) & 0xFF;
    response_buffer[0] += 4;
}

u64 u64_from_buffer(u8 *buf){
	u64 ret = ((u64)buf[0] << 56) |
			  ((u64)buf[1] << 48) |
			  ((u64)buf[2] << 40) |
			  ((u64)buf[3] << 32) |
			  ((u64)buf[4] << 24) |
			  ((u64)buf[5] << 16) |
			  ((u64)buf[6] << 8) |
			  ((u64)buf[7] << 0);
	return ret;
}

u64 u56_from_buffer(u8 *buf){
	u64 ret = ((u64)buf[0] << 48) |
			  ((u64)buf[1] << 40) |
			  ((u64)buf[2] << 32) |
			  ((u64)buf[3] << 24) |
			  ((u64)buf[4] << 16) |
			  ((u64)buf[5] << 8) |
			  ((u64)buf[6] << 0);
	return ret;
}

u32 u32_from_buffer(u8 *buf){
	u32 ret = ((u32)buf[0] << 24) |
			  ((u32)buf[1] << 16) |
			  ((u32)buf[2] << 8) |
			  ((u32)buf[3] << 0);
	return ret;
}
