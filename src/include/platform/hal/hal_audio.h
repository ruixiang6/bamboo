#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

#define VOICE_2400BPS_LEVEL				0
#define VOICE_2400BPS_FEC_LEVEL			1
#define VOICE_3600BPS_LEVEL				2
#define VOICE_3600BPS_FEC_LEVEL			3
#define VOICE_4000BPS_LEVEL				4
#define VOICE_4800BPS_LEVEL				5
#define VOICE_4800BPS_FEC1_LEVEL		6
#define VOICE_4800BPS_FEC2_LEVEL		7
#define VOICE_4800BPS_FEC3_LEVEL		8
#define VOICE_6400BPS_LEVEL				9
#define VOICE_7200BPS_LEVEL				10
#define VOICE_8000BPS_LEVEL				11
#define VOICE_8000BPS_FEC_LEVEL			12
#define VOICE_9600BPS_LEVEL				13
#define VOICE_9600BPS_FEC_LEVEL			14

#define AUDIO_FRAME_SIZE				34

void hal_audio_init(fpv_t epr_cb, fpv_t dpe_cb);
void hal_audio_deinit(void);
void hal_audio_reset(void);
void hal_audio_read(uint8_t *buffer);
void hal_audio_write(uint8_t *buffer);
void hal_audio_bypass(bool_t res);
void hal_audio_change_rate(uint8_t type);
bool_t hal_audio_is_ptt(void);
bool_t hal_audio_is_write(void);
bool_t hal_audio_is_read(void);
void hal_audio_change_gain(uint8_t level);

#endif