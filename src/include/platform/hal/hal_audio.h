#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

//对应管脚BPS[3210]
#define VOICE_2400BPS_LEVEL				0
#define VOICE_2400BPS_FEC_LEVEL			5
#define VOICE_3600BPS_LEVEL				1
#define VOICE_3600BPS_FEC_LEVEL			11
#define VOICE_4000BPS_LEVEL				14
#define VOICE_4800BPS_LEVEL				3
#define VOICE_4800BPS_FEC1_LEVEL		7
#define VOICE_4800BPS_FEC2_LEVEL		2
#define VOICE_4800BPS_FEC3_LEVEL		8
#define VOICE_6400BPS_LEVEL				10
#define VOICE_7200BPS_LEVEL				9
#define VOICE_8000BPS_LEVEL				12
#define VOICE_8000BPS_FEC_LEVEL			13
#define VOICE_9600BPS_LEVEL				4
#define VOICE_9600BPS_FEC_LEVEL			6
#define VOICE_MAX_LEVEL                 15

#define AUDIO_FRAME_SIZE				34

extern const uint8_t slience_enforce_voice[VOICE_MAX_LEVEL][AUDIO_FRAME_SIZE];

void hal_audio_init(void);
void hal_audio_deinit(void);
void hal_audio_reset(void);
void hal_audio_read(uint8_t *buffer);
void hal_audio_write(uint8_t *buffer);
void hal_audio_bypass(bool_t res);
void hal_audio_set_rate(uint8_t rate);
uint8_t hal_audio_get_rate(void);
uint8_t hal_audio_get_valid_data_len(void);
bool_t hal_audio_is_write(void);
bool_t hal_audio_is_read(void);
void hal_audio_change_gain(uint8_t level);
bool_t hal_audio_is_ptt(void);

#endif