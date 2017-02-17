#ifndef __PHY_H
#define __PHY_H

#include <platform.h>
#include <kbuf.h>

#define MAX_PHY_OFDM_FRM_MULTI		4
#define MAX_PHY_OFDM_FRM_LEN		1888

#pragma pack(1)

#pragma pack()

bool_t phy_ofdm_send(kbuf_t *kbuf);
int8_t phy_ofdm_cca(void);
void phy_init(void);
void phy_deinit(void);

#endif
