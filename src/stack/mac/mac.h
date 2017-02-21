#ifndef __MAC_H
#define __MAC_H

#include <platform.h>
#include <kbuf.h>

#define MAC_EVENT_OF_RX			(1u<<0)
#define MAC_EVENT_OF_TX			(1u<<1)
#define MAC_EVENT_CSMA			(1u<<2)

#define MAC_CSMA_RTS			1
#define MAC_CSMA_CTS			2
#define MAC_CSMA_DIFS			3
#define MAC_CSMA_SIFS			4
#define MAC_CSMA_SLOT			5
#define MAC_CSMA_RDY			6
#define MAC_CSMA_FREE			0

#define MAC_CCA_THREDHOLD		-68

#define	BROADCAST_ID			0xFF

#define MAC_FRM_DATA_TYPE		0
#define MAC_FRM_CTRL_TYPE		1
#define MAC_FRM_MGMT_TYPE		2
#define MAC_FRM_TEST_TYPE		3

#define MAC_FRM_PROBE_STYPE		6
#define MAC_FRM_RTS_STYPE		11
#define MAC_FRM_CTS_STYPE		12
#define MAC_FRM_ACK_STYPE		13

#define MAC_PKT_LIVE_US			100000
#define MAC_PKT_DIFS_US			20000
#define MAC_PKT_SLOT_UNIT_US	50
#define MAC_IDLE_TO_SEND_US		100


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

typedef struct
{
	uint8_t csma_difs_cnt;
	uint8_t csma_slot_cnt;
	uint8_t csma_type;
	uint8_t csma_id;
	uint8_t send_id;	
	uint8_t live_id;
}mac_timer_t;

#pragma pack()

#define MAC_QOS_LIST_MAX_NUM	5	

extern list_t *mac_ofdm_send_multi_list[MAC_QOS_LIST_MAX_NUM];
extern list_t mac_ofdm_recv_list;
extern list_t mac_ofdm_send_list;

extern osel_task_t *mac_task_h;
extern osel_event_t *mac_event_h;
extern mac_timer_t mac_timer;
extern kbuf_t *mac_rdy_snd_kbuf;

bool_t mac_send(kbuf_t *kbuf);
void mac_handler(uint16_t event_type);
void mac_init(void);
void mac_deinit(void);

void mac_tx_cb(void);
void mac_csma_cb(void);
void mac_ofdm_recv_cb(void);
void mac_ofdm_send_cb(void);
void mac_rdy_kbuf_live_cb(void);

#endif
