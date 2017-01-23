#ifndef __CHECK_H
#define __CHECK_H

uint8_t crc8_tab(uint8_t* uc_ptr, uint8_t crc8, uint16_t uc_len);

uint16_t check16_sum(void *data_ptr, uint16_t len);
#endif

