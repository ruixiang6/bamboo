#ifndef __TSET_H
#define __TSET_H

#define TEST_OFDM_FRAME_MAX_LEN				472*16
#define TEST_OFDM_INSIDE_MAX_FRAME_SEQ		6u
#define TEST_FRAME_HEAD_SIZE				7
#define TEST_UART_MAX_LEN					512

#define AMBE_PTT_UP							0x00
#define AMBE_PTT_DOWN						0x01

#define AUDIO_BUF_SIZE						36
#define AUDIO_BUF_NUM						500

#define DDR_8BIT_SIZE 						(1024*1024*64)
#define DDR_16BIT_SIZE 						(1024*1024*32)
#define DDR_32BIT_SIZE 						(1024*1024*16)

#define UART_MENU_STATE						0
#define UART_CMD_STATE						1

#define TSET_OFDM_SCRIPT_LEN				8
#define TEST_OFDM_CMD_MAX					5
#define TEST_OFDM_CMD_TX					0
#define TEST_OFDM_CMD_RX					1
#define TEST_OFDM_CMD_CCA					2
#define TEST_OFDM_CMD_EXIT					3
#define TEST_OFDM_CMD_HELP					4

#define TEST_OFDM_PARA_MAX					10
#define TEST_OFDM_PARA_MODE					0
#define TEST_OFDM_PARA_DISPLAY				1
#define TEST_OFDM_PARA_LEN					2
#define TEST_OFDM_PARA_NUM					3
#define TEST_OFDM_PARA_INTEV				4
#define TEST_OFDM_PARA_SRC					5
#define TEST_OFDM_PARA_DST					6
#define TEST_OFDM_PARA_VALUE				7
#define TEST_OFDM_PARA_CSMA					8
#define TEST_OFDM_PARA_DMA					9

#define TEST_OFDM_RECV_SIG_DEV				0
#define TEST_OFDM_RECV_P2P					1
#define TEST_OFDM_RECV_DISP					2

#define TEST_OFDM_SEND_NORMAL				0
#define TEST_OFDM_SEND_CW					1
#define TEST_OFDM_SEND_FULL					2

#pragma pack(1)
typedef struct
{
	struct
	{
		uint8_t frm_len;
		uint8_t dst_addr;
		uint8_t src_addr;
		uint16_t seq_num;
		uint16_t seq_total_num;
	}head;
	
	uint8_t payload[TEST_OFDM_FRAME_MAX_LEN-TEST_FRAME_HEAD_SIZE];	
}test_ofdm_frame_t;

typedef struct
{
	uint32_t machine_state:			8,
			 uart_recv_date:		1,//获得串口数据
			 uart_state:			1,//串口状态
			 rf_inited:				1,//初始化rf接口
			 rf_state:				2,//rf状态
			 rf_recv_data:			1,//rf接收到帧
			 rf_recv_inside_first:	1,//rf接收信号源第一帧
			 rf_send_data:  		1,//rf发送
     		 rf_time_out:      		1,//超时
			 dummy:					15;
}test_cb_t;

typedef struct
{
	uint8_t cmd;
	uint8_t mode;
	uint16_t frm_disp;
	uint16_t frm_len;
	uint16_t frm_num;
	uint16_t frm_interv_ms;
	uint8_t src_addr;
	uint8_t dst_addr;
	uint8_t value;
	bool_t csma_flag;
	bool_t dma_flag;
}test_rf_ofdm_t;

typedef struct
{	
	uint8_t m_ptt_state;
	struct
	{
	  uint8_t ambe_out_flag:	1,
			  ambe_in_flag:		1,			  
	          ptt_flag:			1,
			  ptt_cnt:			4;
	}m_flag;
}ambe_cb_t;

typedef struct
{
	list_t list;
	uint8_t *p_buf;	
}audio_send_t;

#pragma pack()

#define STEP_LEVEL0				0x00
#define STEP_LEVEL1				0x10
#define STEP_LEVEL2				0x20
#define STEP_LEVEL3				0x30
#define STEP_LEVEL3_1			0x31
#define STEP_LEVEL3_2			0x32
#define STEP_LEVEL3_3			0x33
#define STEP_LEVEL3_4			0x34
#define STEP_LEVEL3_5			0x35
#define STEP_LEVEL3_6			0x36
#define STEP_LEVEL4				0x40
#define STEP_LEVEL4_1			0x41
#define STEP_LEVEL4_2			0x42
#define STEP_LEVEL4_3			0x43
#define STEP_LEVEL4_4			0x44
#define STEP_LEVEL4_5			0x45
#define STEP_LEVEL4_6			0x46
#define STEP_LEVEL5				0x50

void test_init(void);
void test_handler(void);

extern osel_event_t *test_event_h;

#endif
