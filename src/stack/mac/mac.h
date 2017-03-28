#ifndef __MAC_H
#define __MAC_H

#include <platform.h>
#include <kbuf.h>

#define MAC_EVENT_OF_RX			    (1u<<0)
#define MAC_EVENT_OF_TX			    (1u<<1)
#define MAC_EVENT_CSMA			    (1u<<2)
#define MAC_EVENT_OF_IDLE		    (1u<<3)
#define MAC_EVENT_OF_LIVE		    (1u<<4)

#define MAC_CSMA_RTS			    1
#define MAC_CSMA_CTS			    2
#define MAC_CSMA_DIFS			    3
#define MAC_CSMA_SIFS			    4
#define MAC_CSMA_SLOT			    5
#define MAC_CSMA_RDY			    6
#define MAC_CSMA_FREE			    0

#define	BROADCAST_ID			    0xFF

#define NONE						0
#define PROBE						1
#define RTS							1
#define CTS							2
#define ACK							3
#define QOS_H						3
#define QOS_M						2
#define QOS_L						1

#define MAC_PKT_LIVE_US			    25000
#define MAC_PKT_PROBE_LIVE_US	    25000
#define MAC_PKT_REPEAT_SEND_CNT		100
#define MAC_PKT_DIFS_US			    50
#define MAC_PKT_SLOT_UNIT_US	    10
#define MAC_IDLE_TO_SEND_US		    80
#define MAC_SEND_INTERVAL_US	    1050
#define MAC_SEND_PROBE_US			500000

#pragma pack(1)

typedef struct
{
	uint16_t protocol:		2,
			 ctl_mgmt:		2,
			 probe_flag:	1,
			 reserve:		1,
			 qos_level:		2,
			 to_ds:			1,
			 from_ds:		1,
			 more_frag:		1,
			 retry:			1,
			 pwr_mgmt:		1,
			 more_data:		1,
			 protected:		1,
			 order:			1;
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
	uint8_t sender_id;
	uint8_t target_id;
	mac_frm_seq_t seq_ctrl;
	uint16_t frm_len;
	uint16_t chksum;
} mac_frm_head_t;

typedef struct
{
	uint8_t csma_difs_cnt;
	uint8_t csma_slot_cnt;
	uint8_t csma_type;
	uint8_t csma_id;
	uint8_t send_id;	
	uint8_t live_id;
	uint16_t live_us;
	uint8_t idle_id;
	uint16_t idle_us;
	bool_t idle_state;
}mac_timer_t;

typedef struct
{
	list_t tx_list;
	uint32_t total_num;
	uint64_t total_size;
}mac_send_t;

typedef struct
{
	uint8_t src_id;
	uint8_t dest_id;
	uint8_t sender_id;
	uint8_t target_id;
	uint8_t seq_num;
	mac_frm_ctrl_t frm_ctrl;
	uint8_t snr;
}packet_info_t;

#define CHECK_OFFSET_LEN    (14+2) //多校验16字节（主要用于普通数据的eth_hdr中mac地址和数据对齐）

typedef struct
{
	mac_frm_head_t mac_head;
	uint8_t inc_chk[CHECK_OFFSET_LEN];    
}packet_chksum_t;

#pragma pack()

#define MAC_QOS_LIST_MAX_NUM	3	

extern mac_send_t mac_send_entity[MAC_QOS_LIST_MAX_NUM];
extern list_t mac_ofdm_recv_list;

extern osel_task_t *mac_task_h;
extern osel_event_t *mac_event_h;
extern mac_timer_t mac_timer;
extern kbuf_t *mac_rdy_snd_kbuf;

void mac_idle_hook(void);
bool_t mac_send(kbuf_t *kbuf, packet_info_t *p_send_info);
void mac_handler(uint16_t event_type);
void mac_init(void);
void mac_deinit(void);

void mac_send_cb(void);
void mac_idle_cb(void);
void mac_csma_cb(void);
void mac_ofdm_recv_cb(void);
void mac_ofdm_send_cb(void);
void mac_live_cb(void);
//@mac_meas.c
void mac_meas_cca_thred(void);


#endif
