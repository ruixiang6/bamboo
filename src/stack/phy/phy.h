#ifndef __PHY_H
#define __PHY_H

#include <platform.h>
#include <kbuf.h>

#define MAX_PHY_OFDM_FRM_MULTI		4
#define MAX_PHY_OFDM_FRM_LEN		1888
#define MAX_PHY_TMR_NUM				8

#pragma pack(1)
typedef struct
{
	uint8_t tmr_int;
	bool_t used;
	uint32_t count;
}phy_tmr_t;
#pragma pack()

void phy_ofdm_recv(void);
void phy_ofdm_send(void);
void phy_ofdm_idle(void);
bool_t phy_ofdm_cca(void);
uint16_t phy_ofdm_snr(void);

bool_t phy_ofdm_write(uint8_t *buf, uint32_t size);
bool_t phy_ofdm_read(uint8_t *buf, uint32_t size);

void phy_init(void);
void phy_ofdm_init(fpv_t send_func, fpv_t recv_func);
void phy_tmr_init(void);

void phy_deinit(void);

uint16_t phy_version(void);

uint8_t phy_tmr_alloc(fpv_t func);
bool_t phy_tmr_free(uint8_t id);
bool_t phy_tmr_start(uint8_t id, uint32_t delay_us);
bool_t phy_tmr_stop(uint8_t id);
bool_t phy_tmr_add(uint8_t id, uint32_t delay_us);
bool_t phy_tmr_repeat(uint8_t id);

#endif
