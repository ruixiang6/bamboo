#ifndef __CRC_H
#define __CRC_H

//8λcrcУ���㷨
uint8_t crc8_tab(uint8_t* uc_ptr, uint8_t crc8, uint16_t uc_len);
//32λcrcУ���㷨
uint32_t crc32_tab (uint8_t* uc_ptr, uint32_t crc8, uint16_t uc_len);
//����dataptrΪ�����ֽ��򣬷���Ϊ16λС���ֽ���
uint16_t check16_sum(void *data_ptr, uint16_t len);
#endif

