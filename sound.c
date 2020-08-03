#include "launchelf.h"
#include <audsrv.h>
#include "audsrv2.h"
#define	u127	uint64
//	#define	SND_FV	((uint64)4960687226880)
//	#define	SND_FV	((uint64)  21387816450)
//	#define	SND_FV	((uint64)4237833600000)
//	#define	SND_FV	((uint64) 541900800000)
	#define	SND_FV	((uint64)      7680000)
//	#define	SND_FV	((uint64)        48000)
//	48000 =	2*2*2*2*2*2*2 * 3 * 5*5*5
//		  =	2^7 * 3 * 5^3
//	44100 =	2*2 * 3*3 * 5*5 * 7*7
//		  =	2^2 * 3^2 * 5^2 * 7^2
//	19200 =	2*2*2*2*2*2*2*2 * 3 * 5*5
//			2^8 * 3 * 5^2
//	=> 2^9 * 3^3 * 5^5 * 7^2
unsigned char sndview_filename[MAX_PATH], sndview_format[32], sndview_totaltime[16], *sndview_head=NULL, *sndview_body=NULL;
unsigned int sndview_fmt=0, sndview_size=0, sndview_len=0, sndview_rate=48000, sndview_speed=100, sndview_delay=2048, sndview_avrbps=1536000;//sndview_rptr=0, 
int sndview_now=0, sndview_sec=0, pcmoldsent=0, sndview_redraw=0;
int pcmspos=0, nobgmpos=0, pcmloop=1, audsrv_bufsize=0;
void pcmdummy(void) {return;}
//char pcmpkt[8192];	// 48000Hz * 2ch * 16bits / 50fps = 3840bytes
signed short pcmpkt[4096];	// 2048 samples, 2ch
//signed short pcmpkt[2048];	// 1024 samples, 2ch
int pcmmode=0,pcmrate=48000,pcmch=2,pcmbit=16,pcmfloat=0,pcmnative=1,spurate=48000,pcmwptr=0,pcmempty=1,pcmnplay=0;
u127 pcmstep=SND_FV, pcmoffset=0;
void (*pcmfunc)(void)=pcmdummy;
static int oldtime=0;
static uint64 oldcount=0;
extern int line_Margin;
int vol=-1,bgplaying=0;
SND_FUNC dec;
SND_INFO info;
////////////////////////////////
// デバッグ用トラップ

static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("viewer: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("viewer: malloc valid (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("viewer: free valid (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("viewer: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
static void *X_realloc(void *mallocdata, size_t mallocsize)
{
	void *ret;
	ret = realloc(mallocdata, mallocsize);
	if (ret != NULL)
		printf("viewer: realloc valid (ofs: %08X -> %08X, size: %d)\n", (unsigned int) mallocdata, (unsigned int) ret, mallocsize);
	else
		printf("viewer: realloc failed: (ofs: %08X, size: %d)\n", (unsigned int) mallocdata, mallocsize);
	return ret;
}
#define	malloc	X_malloc
#define free	X_free
#define realloc	X_realloc
// サウンド関係
#ifdef	itoVSync
#undef	itoVSync
#endif
int pcmpause(void)
{
	pcmnplay=0;
	return audsrv_stop_audio();
}
int pcmplay(void)
{
	pcmnplay=1;
	audsrv_play_audio((char*)pcmpkt, 0);
	pcmoldsent = totalcount;
	if (pcmempty)// pcmfunc();
		dec.callback();

	return 0;
}
//	18446744073709551616
//	6571966140579840
//	4960687226880
int pcmconfig(int mode, int drate, int dch, int dbits)
{	// 元ソース設定
	audsrv_fmt_t format;
	
	pcmrate = drate;
	pcmch = dch;
	pcmbit = dbits;
	pcmmode = mode;
	pcmfloat = 0;
	if (dbits < 0) {
		pcmbit = -pcmbit;
		pcmfloat = 1;
	}
	pcmstep = (SND_FV * drate + (spurate>>1)) / spurate;
	pcmoffset = 0;
	pcmnative = (pcmrate==spurate&&pcmbit==16&&pcmch==2);
	printf("viewer: %d Hz, %d bit, %d ch\n", pcmrate, pcmbit, pcmch);
	printf("viewer: pcmstep=%ld ( 0x%016lX )\n", pcmstep, pcmstep);
	
	format.bits = 16;
	format.freq = spurate;
	format.channels = 2;
	return audsrv_set_format(&format);;
}
int pcmspeed(int rate)
{
	pcmrate = rate;
	pcmstep = (SND_FV * rate + (spurate>>1)) / spurate;
	pcmnative = (pcmrate==spurate&&pcmbit==16&&pcmch==2);
	printf("viewer: pcmstep=%18ld ( 0x%016lX )\r", pcmstep, pcmstep);
//	printf("viewer: %6d Hz\r", pcmrate);
	return 0;
}
int pcmclear(void)
{	// 全消去
	audsrv_lbfn_control(10);
	pcmempty = 1;
	pcmwptr = 0;
	pcmspos = 0;
	return audsrv_lbfn_control(9);
}
int pcmadds(char *src, int sample, int rate, int pcmbit, int pcmch, int pcmfloat)
{	// 波形追加
	if (!pcmempty) return 0;
	int bsample=sizeof(pcmpkt)/(sizeof(short)*2)-pcmwptr,pcmrate=rate * sndview_speed /100;
	u127 pcmstep = (SND_FV * pcmrate + (spurate>>1)) / spurate;
	//	if ((sndview_fmt != 1) && (sndview_fmt != 3) && (sndview_fmt != 768)) {
//	printf("pcmadds: rate=%6dHz (%3d%%), step=%18ld ( 0x%016lX )\r", pcmrate, sndview_speed, pcmstep, pcmstep);
	if ((pcmbit < 8) || (!sample && pcmwptr)) {
		// 未対応形式→無音化
		if (sample && (sample < bsample)) bsample = sample;
		memset(((char*)pcmpkt) + pcmwptr*4, 0, bsample*4);
		pcmwptr += bsample; pcmwptr &= 2047;
		if (!pcmwptr) pcmempty = 0;
		return bsample;
	}
	if (!sample) return 0;
	if ((sndview_rate != rate) && rate){//|| (sndview_ch != pcmch)) {
		sndview_rate = rate; //sndview_ch = pcmch;
		sndview_redraw = 1;
		sndview_delay = ((2048 + audsrv_bufsize + sizeof(pcmpkt)) / 4) * pcmrate / 4800000;
	}
	if (pcmrate==spurate&&pcmbit==16&&pcmch==2) {
		// サンプリング周波数やビット数の変換がないとき→バッファへコピーする
		//	printf("pcmadds0: wptr:%4d, sample:%5d, bsample:%5d\n", pcmwptr, sample, bsample);
		if (sample < bsample) bsample = sample;
		//	printf("pcmadds1: wptr:%4d, sample:%5d, bsample:%5d\n", pcmwptr, sample, bsample);
		//printf("copied size: %d samples\n", bsample);
		memcpy(&pcmpkt[pcmwptr*2], src, bsample*4);
		pcmwptr = (pcmwptr + bsample) & 2047;
		if (pcmwptr == 0) pcmempty = 0;
		return bsample;
	}
	// サンプリングレートやビット数の変換が必要な場合
	int v, i, block, ret, bb;
	short *dst=&pcmpkt[pcmwptr*2];
	
	//shift = (pcmbit < 16) + (pcmch < 2);
	//ssize = size >> (2 - shift);
	//	shift = (pcmbit >> 3) * pcmch;
	//	ssize = size / shift;
	block = pcmbit * pcmch / 8;
	if (pcmfloat) {
		if (pcmbit == 8) {
			char temp;
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				temp = src[v] ^ (char)0x80; if (temp < 0) temp ^= 127;
				*dst++ = (257 * temp) ^ 0x0080;
				*dst++ = (257 * temp) ^ 0x0080;
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		} else if (pcmbit < 64) {
			float *f=(float *)src;
			if (pcmch > 1) {
				for (i=0; i<bsample; i++) {
					v = ( pcmoffset + pcmstep * i ) / SND_FV;
					if (v >= sample) {	i--;	break;	}
					*dst++ = f[v * pcmch +0] * 32767;
					*dst++ = f[v * pcmch +1] * 32767;
					pcmwptr = (pcmwptr + 1) & 2047;
					if (pcmwptr == 0) {	pcmempty = 0;	break;	}
				}
			} else {
				for (i=0; i<bsample; i++) {
					v = ( pcmoffset + pcmstep * i ) / SND_FV;
					if (v >= sample) {	i--;	break;	}
					*dst++ = f[v] * 32767;
					*dst++ = f[v] * 32767;
					pcmwptr = (pcmwptr + 1) & 2047;
					if (pcmwptr == 0) {	pcmempty = 0;	break;	}
				}
			}
		} else {
			double *d=(double *)src;
			if (pcmch > 1) {
				for (i=0; i<bsample; i++) {
					v = ( pcmoffset + pcmstep * i ) / SND_FV;
					if (v >= sample) {	i--;	break;	}
					*dst++ = d[v * pcmch +0] * 32767;
					*dst++ = d[v * pcmch +1] * 32767;
					pcmwptr = (pcmwptr + 1) & 2047;
					if (pcmwptr == 0) {	pcmempty = 0;	break;	}
				}
			} else {
				for (i=0; i<bsample; i++) {
					v = ( pcmoffset + pcmstep * i ) / SND_FV;
					if (v >= sample) {	i--;	break;	}
					*dst++ = d[v] * 32767;
					*dst++ = d[v] * 32767;
					pcmwptr = (pcmwptr + 1) & 2047;
					if (pcmwptr == 0) {	pcmempty = 0;	break;	}
				}
			}
		}
	} else if (pcmbit == 16) {
		short *s=(short *)src;
		if (pcmch > 1) {
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = s[v * pcmch +0];
				*dst++ = s[v * pcmch +1];
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		} else {
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = s[v];
				*dst++ = s[v];
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		}
	} else if (pcmbit > 16) {
		int bp, bc;
		bp = bc = bb = pcmbit >> 3;
		bp--; bc += bp;
		if (pcmch > 1) {
			bb *= pcmch;
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = ( (short) src[v * bb + bp] << 8);
				*dst++ = ( (short) src[v * bb + bc] << 8);
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		} else {
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = ( (short) src[v * bb + bp] << 8);
				*dst++ = ( (short) src[v * bb + bp] << 8);
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		}
	} else {
		if (pcmch > 1) {
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = ( (short) 257*src[v * pcmch +0] ) ^ (short)0x8000;
				*dst++ = ( (short) 257*src[v * pcmch +1] ) ^ (short)0x8000;
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		} else {
			for (i=0; i<bsample; i++) {
				v = ( pcmoffset + pcmstep * i ) / SND_FV;
				if (v >= sample) {	i--;	break;	}
				*dst++ = ( (short) 257*src[v] ) ^ (short)0x8000;
				*dst++ = ( (short) 257*src[v] ) ^ (short)0x8000;
				pcmwptr = (pcmwptr + 1) & 2047;
				if (pcmwptr == 0) {	pcmempty = 0;	break;	}
			}
		}
	}
	i++;
	ret = (pcmoffset + pcmstep * i) / SND_FV;
	if ((pcmoffset + pcmstep * (i - 1)) / SND_FV == ret) ret--;
	if (ret < 0) ret = 0;
	pcmoffset = (pcmoffset + pcmstep * i) % SND_FV;
	//printf("old:%9.4lf,%9.4lf ofs:%016lXh ret:%5d\n", (double)( pcmoffset + pcmstep * ( i -1 ) ) / SND_FV, (double)( pcmoffset + pcmstep * i ) / SND_FV, pcmoffset, ret);
	return ret;
}
int pcmvolume(int vol)
{	// 音量設定
	if (vol >= 100) return audsrv_set_volume(MAX_VOLUME);
	else if (vol < 0) return audsrv_set_volume(0);
	else return audsrv_set_volume(((MAX_VOLUME+1)&~1) * vol / 100);
}
/*
int pcmregist(void (*func)(void))
{	// 転送完了呼び出しルーチンの登録
	pcmfunc = func;
	return 0;
}
int pcmdelete(void)
{	// 転送完了呼び出しルーチンの削除
	pcmfunc = pcmdummy;
	return 0;
}
//*/
int pcmquit(void)
{	// 終了処理
	//	pcmdelete();
	pcmvolume(0);
	pcmpause();
	pcmclear();
	return audsrv_quit();
}
int bgmredraw=0;
#ifndef	itoVSync
#define	itoVSync	X_itoVSync
#endif
/*
void sndview_callback(void)
{
	// とりあえずWAVE(LPCM)形式専用
	int pktlimit, pktsize, limit=512, sent=0;
	pktlimit = (4096 * sndview_speed / 100 +3) & ~3;
	while(pcmempty&&limit) {
		if (sndview_rptr >= sndview_len) {
			sndview_rptr = 0;
			if (setting->snd_repeat != 1) {
				pcmnplay = 0;
				break;
			}
		}
		if (sndview_rptr+pktlimit > sndview_len) pktsize=sndview_len-sndview_rptr; else pktsize=pktlimit;
		if (pktsize > 0) {
			sent = pcmadds(&sndview_body[sndview_rptr], pktsize, sndview_rate, pcmbit, pcmch, sndview_fmt == 3);
			sndview_rptr += sent;
		}
		limit--;
	}
	sndview_now = ( (sndview_rptr +sndview_len -sndview_delay) % sndview_len ) / (sndview_rate * pcmch * pcmbit / 8);
	//if (paddata & PAD_L3) printf("sndview: sent %d bytes\n", sent);
}
//*/
int geti32(unsigned char *s){return (int)s[0] | ((int)s[1]<<8) | ((int)s[2]<<16) | ((int)s[3]<<24);}
int getb32(unsigned char *s){return (int)s[3] | ((int)s[2]<<8) | ((int)s[1]<<16) | ((int)s[0]<<24);}
short geti16(unsigned char *s){return (short)s[0] | ((short)s[1]<<8);}
int scasbn(unsigned char *buf, unsigned int limit, unsigned char chr, int ofs)
	{int i;	for (i=ofs; i<limit; i++) {if (buf[i] == chr) return i;} return -1;}
int sndview_file(int mode, char *file);
int sndview(int mode, char *file, unsigned char *buff, unsigned int size)
{
	enum{
		mnu_info=0,	//	none					
		mnu_speed,	//	up/down/+fast/default	
		mnu_volume,	//	up/down/+fast/default	
		mnu_pause,	//	enter					
		mnu_seek,	//	back/skip				
		mnu_quit,	//	enter					
		mnu_items
	};
	int i,fmt=0,ch=2,bits=16,rate=44100,bitrate=0,sz=0,body=48,len=size-48,redraw=fieldbuffers,sy=0,oy=0,z=0,oz=0,sec=1;
	int pos;
	char format[32];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	char config[mnu_items+1][MAX_PATH];
	u32 msg1index[mnu_items] = {31, 30, 30, 29, 28, 29};
	static int playermode=0;
	if (vol < 0) vol = setting->snd_volume;
	bgplaying = 0;	pos = 0;
	pcmloop = setting->snd_repeat == 1;
	
	strcpy(msg0, file);
	//	sprintf(msg1, "○:%s ×:%s △:%s ↑/↓:%s ←/→/R1:%s L1/L2/R2:%s",
	//			"Play", "Stop", "Quit", 
	//			"Volume", "Speed", "Position"
	//		   );
	sprintf(msg1, "○:%s ×:%s △:%s ↑/↓:%s ←/→/R1:%s L1/L2/R2:%s",
			lang->sound[21], lang->sound[22], lang->sound[23], 
			lang->sound[12], lang->sound[24], lang->sound[25]
		   );
	//	printf("file: %s\nback: %s\nbuff: %08X ( %08X )\nsize: %8d ( %8d )\n", 
	//			file, sndview_filename, (int)buff, (int)sndview_head, size, sndview_size);
	if (strcmp(file, sndview_filename)==0&&buff==sndview_head&&size==sndview_size) {
		rate = sndview_rate;
		bits = pcmbit;
		ch = pcmch;
		body = sndview_body-sndview_head;
		len = sndview_len;
		fmt = sndview_fmt;
		bitrate = sndview_avrbps;
		strcpy(format, sndview_format);
	} else {
		format[0] = 0;
		printf("sndview: buff=%08X, size=%8d\n", (int) buff, size);
		//printf("sndview: i32: %d\n", geti32(buff));
		info.fmt_id = info.fmt_type = info.samples = 0;
		if (wav_check(buff, size) && ((i=wav_setup(buff, size, &info))>=0)) {	// "RIFF" "WAVE"
			wav_func(&dec);
		} else if (mp3_check(buff, size) && ((i=mp3_setup(buff, size, &info))>=0)) {
			mp3_func(&dec);
		}
		rate	= info.rate;
		bits	= info.bit;
		ch		= info.ch;
		body	= info.body;
		len		= info.len;
		fmt		= info.fmt_id;
		sec		= info.sec;
		bitrate	= info.bps;
		strcpy(format, info.fmt_name);

		//	if (fmt != 1 && fmt != 3 && fmt != 768) {
		if (!fmt) {
			printf("viewer: error format type of 0x%04X. ( %dHz, %dbits, %dch, %dkbps )\n", fmt, rate, bits, ch, bitrate / 1000);
			return -1;
		}
		pcmconfig(0, rate, ch, bits * (-2 * (fmt == 3) +1));
	//	printf("viewer: %d Hz, %d bit, %d ch\n", rate, bits * (-2 * (fmt == 3) +1), ch);

		//printf("sndview: completed\n");
		//	id = (rate*ch*bits/8 / SCANRATE +3) & ~3;
		sz = audsrv_lbfn_control(0)+1;
		//	printf("viewer: packet size per frame: %d\n", id);
		if(sz)	printf("viewer: audsrv is LbFn modifed version\n");
		else	printf("viewer: audsrv is normal version\n");
		if (!sz) {
			audsrv_bufsize = spurate * 4 * 512 * 10 / 48000;// +sizeof(pcmpkt);	// 20480 +2048;
		} else {
			audsrv_bufsize = audsrv_lbfn_control(4);// +sizeof(pcmpkt);	// +2048;
		}
		printf("viewer: audsrv ringbuffer size:%6d bytes\n", audsrv_bufsize);
		strcpy(sndview_filename, file);
		if (sndview_head != NULL) free(sndview_head);
		if (body + len > size) len = size - body;
		//printf("viewer: offset 0x%08X, alignment %d byte, check result: %d\n", body, bits, );
		sndview_head = buff;
		sndview_size = size;
		sndview_body = &buff[body];
		sndview_len = len;
		sndview_rate = rate;
		sndview_avrbps = bitrate;
		sndview_speed = 100;
		sndview_now = 0;
		sndview_sec = sec;
		//	sndview_delay = (id + id * audsrv_bufsize / 4096) / 4;
		sndview_delay = ((2048 + audsrv_bufsize + sizeof(pcmpkt)) / 4) * sndview_speed * sndview_rate / 4800000;
		// 64KB ÷ 48kHz ＝ 0.341333秒
		sndview_fmt = fmt;
		if (sndview_sec >= 3600) {
			i = sndview_sec / 60;
			sprintf(sndview_totaltime, "%d:%02d:%02d", i / 60, i % 60, sndview_sec % 60);
		} else 
			sprintf(sndview_totaltime, "%d:%02d", sndview_sec / 60, sndview_sec % 60);
		strcpy(sndview_format, format);
		printf("viewer: audsrv buffer %d bytes, delay %d samples ( %d msec )\n", audsrv_bufsize, sndview_delay, sndview_delay * 1000 / rate);
	}
	
	pcmvolume(vol);
	pcmplay();
	sprintf(config[mnu_info], "%s: %s, %dHz, %dbit, %dch, %dkbps", lang->sound[26], format, rate, bits, ch, (bitrate +500) / 1000);
	strcpy(config[mnu_pause], lang->sound[27]);
	strcpy(config[mnu_seek], lang->sound[14]);
	strcpy(config[mnu_quit], lang->sound[23]);
	while(1){
		//waitPadReady(0, 0);
		enum{
			PAD_V = PAD_UP | PAD_DOWN,
			PAD_H = PAD_LEFT | PAD_RIGHT,
		};
		if(readpad() && ((paddata & PAD_V) != PAD_V) && ((paddata & PAD_H) != PAD_H)){
			if (new_pad & PAD_SELECT) break;
			if (new_pad & PAD_TRIANGLE) break;
			/*	参考: OSDSYS の CDプレイヤーの仕様
				・POV: カーソル移動
				・○ボタン: 決定(再生や一時停止など)
				・×ボタン: 戻る(再生は停止される)
				・L1ボタン: トラックの先頭または前のトラックへ
				・R1ボタン: 次のトラックへ
				・L2ボタン: 巻き戻し
				・R2ボタン: 早送り
				・光音声出力は 48kHz から 44.1kHz になる
				・リピートは 切/全曲/1曲 から選択可能
				・再生モードは 標準/プログラム/シャッフル から選択可能
					/===========================================/
					/											/
					/											/
					/		_______			トラック 1			/
					/	   /      /|							/
					/	  |~~~~~~| |      00 min. 33 sec.		/
					/	  |      | | 							/
					/	  |______|/    K< << >> >>| ＞ || □ 	/
					/											/
					/											/
					/			   ×戻る   ○決定				/
					/===========================================/
			*/
			/*
				 LbFn v0.70.17											
				 host:/deza_#29_for_loop.wav             	0:19 / 0:29	
				────────────────────────────
			26	  >	音声情報: 44100Hz, 16bit, 2ch, 1411kbps					none					
			13		再生速度: 100% (  44100Hz )								up/down/+fast/default
			12		音量:     100%											up/down/+fast/default
			27		再生/一時停止											enter
			14		再生位置												back/skip
			23		終了													enter
					
				────────────────────────────
				 ○:add ×:sub +□:fast △:quit L2/R2:seek L1:default
			*/
			if (playermode) {
				// 直接操作モード
				if (new_pad & PAD_CIRCLE) {
					pcmplay();
				}
				if (new_pad & PAD_CROSS) {
					pcmpause();
				}
				if (new_pad & PAD_L1) {
					//	sndview_rptr = 0;
					dec.seek(0, SEEK_SET);
				}
				if (new_pad & PAD_R1) {
					sndview_speed = 100;
					pcmspeed(sndview_rate);
				}
				if (new_pad & PAD_UP) {
					if (paddata & PAD_SQUARE) vol+= 5; else vol++;
					if (vol > 100) vol = 100;
					pcmvolume(vol);
				}
				if (new_pad & PAD_DOWN) {
					if (paddata & PAD_SQUARE) vol-= 5; else vol--;
					if (vol < 0) vol = 0;
					pcmvolume(vol);
				}
				if (new_pad & PAD_LEFT) {
					if (paddata & PAD_SQUARE) sndview_speed-= 5; else sndview_speed--;
					if (sndview_speed < 20) sndview_speed = 20;
					pcmspeed(sndview_rate * sndview_speed /100);
				}
				if (new_pad & PAD_RIGHT) {
					if (paddata & PAD_SQUARE) sndview_speed+= 5; else sndview_speed++;
					if (sndview_speed > 500) sndview_speed = 500;
					pcmspeed(sndview_rate * sndview_speed /100);
				}
			} else {
				// メニュー操作モード
				if (new_pad & PAD_CIRCLE) {
					if (sy == mnu_quit) break;
					switch(sy) {
						case mnu_speed:
							if (paddata & PAD_SQUARE) sndview_speed+= 5; else sndview_speed++;
							if (sndview_speed > 1000) sndview_speed = 1000;
							pcmspeed(sndview_rate * sndview_speed /100);
							redraw = fieldbuffers;
							break;
						case mnu_volume:
							if (paddata & PAD_SQUARE) vol+= 5; else vol++;
							if (vol > 100) vol = 100;
							pcmvolume(vol);
							redraw = fieldbuffers;
							break;
						case mnu_pause:
							if (pcmnplay) pcmpause(); else pcmplay();
							break;
						case mnu_seek:
						//	sndview_rptr = (sndview_rptr + sndview_len + ((rate * (bits>>3) * ch) & ~3)) % sndview_len;
						//	if (sndview_rptr > len) sndview_rptr = len;
							pos = dec.seek(+info.rate, SEEK_CUR);
							if (!pcmnplay) redraw = fieldbuffers;
							break;
					}
				}
				if (new_pad & PAD_CROSS) {
					switch(sy) {
						case mnu_speed:
							if (paddata & PAD_SQUARE) sndview_speed-= 5; else sndview_speed--;
							if (sndview_speed < 10) sndview_speed = 10;
							pcmspeed(sndview_rate * sndview_speed /100);
							redraw = fieldbuffers;
							break;
						case mnu_volume:
							if (paddata & PAD_SQUARE) vol-= 5; else vol--;
							if (vol < 0) vol = 0;
							pcmvolume(vol);
							redraw = fieldbuffers;
							break;
						case mnu_pause:
							if (pcmnplay) pcmpause(); else pcmplay();
							break;
						case mnu_seek:
						//	sndview_rptr = (sndview_rptr + sndview_len - ((rate * (bits>>3) * ch) & ~3)) % sndview_len;
							pos = dec.seek(-info.rate, SEEK_CUR);
							if (!pcmnplay) redraw = fieldbuffers;
							break;
					}
				}
				if (new_pad & PAD_L1) {
					switch(sy) {
						case mnu_speed:
							if (sndview_speed != 100) {
								sndview_speed = 100;
								pcmspeed(sndview_rate);
								redraw = fieldbuffers;
							}
							break;
						case mnu_volume:
							if (vol != setting->snd_volume) {
								vol = setting->snd_volume;
								pcmvolume(vol);
								redraw = fieldbuffers;
							}
							break;
						case mnu_seek:
							pos = dec.seek(0, SEEK_SET);
							if (!pcmnplay) redraw = fieldbuffers;
							break;
					}
				}
				if (new_pad & PAD_UP) 		sy--;
				if (new_pad & PAD_DOWN) 	sy++;
				if (new_pad & PAD_LEFT) 	sy -= MAX_ROWS >> 1;
				if (new_pad & PAD_RIGHT)	sy += MAX_ROWS >> 1;
				if (sy >= mnu_items)		sy = mnu_items -1;
				if (sy < 0)					sy = 0;
				if (sy >= z + MAX_ROWS)		z = sy - MAX_ROWS +1;
				if (sy < z)					z = sy;
				if (z >= mnu_items)			z = mnu_items -1;
				if (z < 0)					z = 0;
				if ((sy != oy) || (z != oz)) {
					oy = sy; oz = z;
					redraw = fieldbuffers;
				}
			}
			if (new_pad & PAD_START) {
				pcmpause();
				pcmclear();
				pcmplay();
			}
			if (new_pad & PAD_L2) {
			//	sndview_rptr = (sndview_rptr + sndview_len - ((rate * bits * ch / 8) & ~3)) % sndview_len;
				pos = dec.seek(-info.rate, SEEK_CUR);
				if (!pcmnplay) redraw = fieldbuffers;
			}
			if (new_pad & PAD_R2) {
			//	sndview_rptr = (sndview_rptr + sndview_len + ((rate * bits * ch / 8) & ~3)) % sndview_len;
			//	if (sndview_rptr > len) sndview_rptr = len;
				pos = dec.seek(+info.rate, SEEK_CUR);
				if (!pcmnplay) redraw = fieldbuffers;
			}
		}
		if (sndview_redraw) {
			sndview_redraw = 0;
			redraw = fieldbuffers;
		}
		
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);
			if (!playermode) {
				uint64 fclr; u32 x, y;
				x = FONT_WIDTH *5;
				y = SCREEN_MARGIN + FONT_HEIGHT *3;
				sprintf(config[mnu_speed], "%s:%5d%% (%7dHz )", lang->sound[13], sndview_speed, sndview_rate * sndview_speed / 100);
				sprintf(config[mnu_volume], "%s: %4d%%", lang->sound[12], vol);
				for(i=0;i<MAX_ROWS;i++){
					if (i+z >= mnu_items) break;
					if (sy==i+z) {
						fclr = setting->color[COLOR_HIGHLIGHTTEXT]; 
						printXY(">", x - FONT_WIDTH*2, y, fclr, TRUE);
					} else fclr = setting->color[COLOR_TEXT];
					printXY(config[i+z], x, y, fclr, TRUE);
					y += FONT_HEIGHT;
				}
				if (mnu_items > MAX_ROWS) {
					int y0,y1;
					drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
						(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),
						setting->color[COLOR_FRAME]);
					y0=FONT_HEIGHT*MAX_ROWS*((float)z/mnu_items);
					y1=FONT_HEIGHT*MAX_ROWS*((float)(z+MAX_ROWS)/mnu_items);
					itoSprite(setting->color[COLOR_FRAME],
						(MAX_ROWS_X+8)*FONT_WIDTH,	SCREEN_MARGIN+FONT_HEIGHT*3+y0,
						(MAX_ROWS_X+9)*FONT_WIDTH,	SCREEN_MARGIN+FONT_HEIGHT*3+y1,
						0);
				}
				strcpy(msg1, lang->sound[msg1index[sy]]);
			}
			setScrTmp(msg0, msg1);
			if (!pcmnplay) {
				sndview_now = ((pos +info.samples -sndview_delay) % info.samples) / info.rate;
			}
			//	if (((oldtime != sndview_now) || bgmredraw) && !nobgmpos) {
			{
				char temp[64]; int w, h, x, y;//, f;
				//	sprintf(temp, "%2d:%02d /%2d:%02d", sndview_now / 60, sndview_now % 60, sndview_sec / 60, sndview_sec % 60);
				i = sndview_now / 60;
				if (sndview_sec >= 3600) 
					sprintf(temp, "%2d:%02d:%02d / %s", i / 60, i % 60, sndview_now % 60, sndview_totaltime);
				else
					sprintf(temp, "%2d:%02d / %s", i, sndview_now % 60, sndview_totaltime);
				x = (MAX_ROWS_X+10-strlen(temp))*FONT_WIDTH; y = SCREEN_MARGIN+FONT_HEIGHT; 
				w = x + (strlen(temp) +1) * FONT_WIDTH +1; h = y + FONT_HEIGHT - line_Margin +1;
			//	for (f=0; f<framebuffers; f++) {
			//		itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
			//		itoSprite(setting->color[COLOR_BACKGROUND], x -FONT_WIDTH/2, y, w +FONT_WIDTH/2, h, 0);
					X_itoSprite(x -FONT_WIDTH/2, y, w +FONT_WIDTH/2, h, 0);
					drawString(temp, TXT_SJIS, x, y, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0);
			//	}
			//	itoGsFinish();
				oldtime = sndview_now;
			}
			drawScr();
			redraw--;
		} else {//if (m < 1) {
			itoVSync();
		}
	}
	if (vol == 99) setting->snd_bgplay = 1;
	else if (vol == 1) setting->snd_bgplay = 0;
	if (vol == 0) {
		pcmpause();
	}
	if (!setting->snd_bgplay && pcmnplay) {
		pcmpause();
	}
	if (!pcmnplay) {
		pcmvolume(0);
		pcmclear();
		//	pcmdelete();
		dec.close();
		sndview_filename[0] = 0;
		sndview_totaltime[0] = 0;
		sndview_head = NULL;
		sndview_size = 0;
		sndview_body = NULL;
		sndview_len = 0;
	}
	bgplaying = pcmnplay;
	return 0;
}

void itoNoVSync(void)
{
	if (pcmnplay) {
		char temp[64];
		int x,y,w,h,i;
		if (((oldtime != sndview_now) || bgmredraw) && !nobgmpos) {
			i = sndview_now / 60;
			if (sndview_sec >= 3600) 
				sprintf(temp, "%2d:%02d:%02d / %s", i / 60, i % 60, sndview_now % 60, sndview_totaltime);
			else
				sprintf(temp, "%2d:%02d / %s", i, sndview_now % 60, sndview_totaltime);
			x = (MAX_ROWS_X+10-strlen(temp))*FONT_WIDTH; y = SCREEN_MARGIN+FONT_HEIGHT; 
			w = x + (strlen(temp) +1) * FONT_WIDTH +1; h = y + FONT_HEIGHT - line_Margin +1;
			for (i=0; i<framebuffers; i++) {
				itoSetActiveFrameBufferWithMatrix(itoGetActiveFrameBuffer()^1);
			//	itoSprite(setting->color[COLOR_BACKGROUND], x -FONT_WIDTH/2, y, w +FONT_WIDTH/2, h, 0);
				X_itoSprite(x -FONT_WIDTH/2, y, w +FONT_WIDTH/2, h, 0);
				drawString(temp, TXT_SJIS, x, y, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0);
			}
			itoGsFinish();
			oldtime = sndview_now;
			bgmredraw = 0;
		}
	//	if (!audsrv_lbfn_control(0)) {
	//		if ((pcmoldsent +2 < totalcount) && (totalcount -oldcount == 0)) {
	//			pcmpause();
	//			pcmclear();
	//			pcmplay();
	//			printf("viewer_vsync: pcm reseted \r");
	//		}
	//		while(!pcmempty && audsrv_empty_audio()>=4096) {
	//			audsrv_play_audio(pcmpkt, 4096);
	//			pcmoldsent = totalcount;
	//			pcmempty=1;
	//			pcmfunc();
	//		}
	//	} else {
			int sent;
			pcmloop = setting->snd_repeat == 1;
			while(!pcmempty){// && audsrv_wait_audio(4096)==AUDSRV_ERR_NOERROR) {
				sent = audsrv_play_audio(((char*)pcmpkt) +pcmspos, sizeof(pcmpkt) -pcmspos);
				pcmoldsent = totalcount;
				pcmspos += sent;
				if (pcmspos >= sizeof(pcmpkt)) {
					pcmempty = 1;
					//	pcmfunc();
					dec.callback();
					pcmspos = 0;
				} else if (sent < sizeof(pcmpkt)) {
					break;
				}
			}
	//	}
		if (!audsrv_lbfn_control(0)) {
			i = audsrv_lbfn_control(1);
			//	printf("remain: %6d\n", i);
			if (pcmempty && (i < 2048)) {
				pcmpause();
			}
		}
		if (!pcmnplay && bgplaying) {
			pcmvolume(0);
			pcmclear();
			//	pcmdelete();
			dec.close();
			free(sndview_head);
			sndview_filename[0] = 0;
			sndview_totaltime[0] = 0;
			sndview_head = NULL;
			sndview_size = 0;
			sndview_body = NULL;
			sndview_len = 0;
		//	sndview_rptr = 0;
		}
		oldcount = totalcount+1;
	}
}

