#ifndef __MAC_H
#define __MAC_H

#include <platform.h>
#include <kbuf.h>

#define MAC_EVENT_OF_RX			(1u<<0)
#define MAC_EVENT_OF_TX			(1u<<1)

#define	BROADCAST_ID			0xFF

#define MAC_FRM_DATA_TYPE		0
#define MAC_FRM_CTRL_TYPE		1
#define MAC_FRM_MGMT_TYPE		2

#define MAC_FRM_PROBE_STYPE		6
#define MAC_FRM_RTS_STYPE		11
#define MAC_FRM_CTS_STYPE		12
#define MAC_FRM_ACK_STYPE		13

#pragma pack(1)

typedef struct
{
	uint16_t protocol:	2,
			 type:		2,
			 sub_type:	4,
			 to_ds:		1,
			 from_ds:	1,
			 retry:		1,
			 pwr_mgmt:	1,
			 more_data:	1,
			 protected:	1,
			 order:		1;
}mac_frm_ctrl_t;

typedef struct
{
	uint16_t nav:		15,
			 reserve:	1;
}mac_frm_duration_t;

typedef struct
{
	uint16_t frag_num:	4,
			 seq_num:	12;
}mac_frm_seq_t;

typedef struct
{
	uint8_t phy;
	mac_frm_ctrl_t frm_ctrl;
	uint16_t duration;
	uint8_t mesh_id;
	uint8_t src_dev_id;
	uint8_t dest_dev_id;
	mac_frm_seq_t seq_ctrl;
	uint16_t frm_len;
	uint32_t crc32;
} mac_frm_head_t;

#pragma pack()

#define MAC_QOS_LIST_MAX_NUM	5	

extern list_t *mac_ofdm_send_multi_list[MAC_QOS_LIST_MAX_NUM];
extern list_t mac_ofdm_recv_list;
extern list_t mac_ofdm_send_list;

extern osel_task_t *mac_task_h;
extern osel_event_t *mac_event_h;

bool_t mac_send(kbuf_t *kbuf);
void mac_handler(uint16_t event_type);
void mac_init(void);
void mac_deinit(void);

#endif
