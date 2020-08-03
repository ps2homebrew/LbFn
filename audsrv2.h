#ifndef __AUDSRV2_H__
#define __AUDSRV2_H__

#ifdef __cplusplus
extern "C" {
#endif

int audsrv_lbfn_control(int func);
int audsrv_empty_audio();
/*
#define AUDSRV_LBFN_VERSION         0x0019
#define AUDSRV_EMPTY_AUDIO          0x001A

#define I_audsrv_lbfn_version      DECLARE_IMPORT(27, audsrv_lbfn_version)
#define I_audsrv_empty_audio       DECLARE_IMPORT(28, audsrv_empty_audio)
*/

typedef struct {
	int fmt_type;	// WAVE, MP3, ...
	int fmt_id; 	// 1:LPCM, 3:float PCM, ...
	char *fmt_name;	// "ms adpcm"
	int rate;		// 48000 Hz
	short bit;		// 16 bit
	short ch;			// 2 ch
	int bps;		// 1536000 bps
	int body;		// 
	int len;		
	int sec;		// 138 sec
	int samples;	// decoded samples
	char *head;
	int block;
} SND_INFO;
typedef struct {
	int (*check)(char*, unsigned int);
	int (*open)(char*, unsigned int, SND_INFO*);
	int (*seek)(signed int, int);
	int (*decode)(unsigned int);
	int (*close)(void);
	int (*callback)(void);
} SND_FUNC;
int geti32(unsigned char *s);
short geti16(unsigned char *s);
void dmp_sndinfo(SND_INFO *info);
int wav_func(SND_FUNC *data);
int wav_check(char *buff, unsigned int size);
int wav_setup(char *buff, unsigned int size, SND_INFO *info);
int wav_seek(int sample, int mode);
int wav_decode(unsigned int data);
int wav_close(void);
int mp3_func(SND_FUNC *data);
int mp3_check(char *buff, unsigned int size);
int mp3_setup(char *buff, unsigned int size, SND_INFO *info);
int mp3_seek(int sample, int mode);
int mp3_decode(unsigned int data);
int mp3_close(void);
int mp3_callback(void);
int pcmadds(char *src, int size, int rate, int pcmbit, int pcmch, int pcmfloat);
extern unsigned int sndview_fmt, sndview_size, sndview_len, sndview_rptr, sndview_rate, sndview_speed, sndview_delay, sndview_avrbps;
extern int sndview_now, pcmmode,pcmrate,pcmch,pcmbit,pcmfloat,pcmnative,spurate,pcmwptr,pcmempty,pcmnplay,pcmloop;

#ifdef __cplusplus
}
#endif

#endif
