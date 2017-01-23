#ifndef __CRC_H
#define __CRC_H

//8位crc校验算法
uint8_t crc8_tab(uint8_t* uc_ptr, uint8_t crc8, uint16_t uc_len);
//32位crc校验算法
uint32_t crc32_tab (uint8_t* uc_ptr, uint32_t crc8, uint16_t uc_len);
//进入dataptr为网络字节序，返回为16位小段字节序
uint16_t check16_sum(void *data_ptr, uint16_t len);
#endif

