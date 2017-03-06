#ifndef _APP_H
#define _APP_H

#include <test.h>
#include <nwk_eth.h>

#define APP_GPS_BUF_SIZE			256
#define APP_GPS_RMC					0
#define APP_GPS_GGA					1
#define NMEA_TIME_SIZE				16
#define NMEA_FLAG_SIZE				2
#define NMEA_LAT1_SIZE				12
#define NMEA_LAT2_SIZE				2
#define NMEA_LON1_SIZE				14
#define NMEA_LON2_SIZE				2
#define NMEA_DATE_SIZE				10
#define NMEA_SAT_SIZE				5
#define NMEA_ALT_SIZE				8

#define TIME_CHINA_ZONE				8
#define NORTH	0
#define SOUTH	1

#define EAST	0
#define WEST	1

#define APP_SNIFF_HEAD				0xD5C8

extern osel_task_t *app_task_h;
extern osel_event_t *app_event_h;

#pragma pack(1)

typedef struct
{
	kbuf_t *rx_kbuf;
	kbuf_t *tx_kbuf;
	list_t kbuf_tx_list;
	list_t kbuf_rx_list;
	ambe_cb_t mcb;
}app_audio_t;

typedef struct
{	char_t time[NMEA_TIME_SIZE];
	char_t flag[NMEA_FLAG_SIZE];
	char_t latitude[NMEA_LAT1_SIZE];
	char_t latitude2[NMEA_LAT2_SIZE];
	char_t longitude[NMEA_LON1_SIZE];
	char_t longitude2[NMEA_LON2_SIZE];	
	char_t date[NMEA_DATE_SIZE];
	char_t satellites[NMEA_SAT_SIZE];
	char_t altitude[NMEA_ALT_SIZE];
}NMEA_RMC_GGA_MSG;

typedef struct
{
	uint8_t state;
	uint32_t send_frm_num;
	uint32_t send_frm_interval;
	uint32_t send_frm_seq;
	list_t kbuf_rx_list;
}app_test_mac_t;

typedef struct
{
	int32_t socket_id;
	struct sockaddr_in s_addr;
	list_t kbuf_rx_list;
}app_sniffer_t;

typedef struct
{
	uint16_t head;
	uint16_t length;
	uint32_t type;
}app_sniffer_frm_head_t;

#pragma pack()

extern app_audio_t app_audio;
extern const uint8_t slience_enforce_2400bps_voice[];	//@test.c
extern NMEA_RMC_GGA_MSG *p_nmea_gps_msg;
extern uint8_t *app_gps_buf;
extern app_test_mac_t app_test_mac;
extern app_sniffer_t app_sniffer;

#define APP_EVENT_GPS			(1u<<0)
#define APP_EVENT_UART			(1u<<1)
#define APP_EVENT_AUDIO			(1u<<2)
#define APP_EVENT_SNIFFER		(1u<<6)
#define APP_EVENT_TEST_MAC		(1u<<7)

void app_handler(uint16_t event_type);
void app_timeout_handler(void);

void gps_recv_callback(void);
void uart_recv_callback(void);
void audio_input_callback(void);
void audio_output_callback(void);

#endif
