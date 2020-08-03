#include "launchelf.h"
#include <audsrv.h>
#include "audsrv2.h"

// ☆
/*
	int nopen(char *path, int attr);
	int nclose(int fd);
	int nquit(char *path);
	int nmkdir(char *path, char *dir, int attr);
	int nrmdir(char *path, char *dir);
	unsigned int ngetc(int fd, char data);
	unsigned int nputc(int fd, char data);
	unsigned int ngets(int fd, char *dst, unsigned int limit);
	int nseek(int fd, signed int ofs, int mode);
	unsigned int nread(int fd, void *dst, unsigned int size);
	unsigned int nwrite(int fd, void *src, unsigned int size);
*/
static void seti32(unsigned char *p, int i) {
	p[0] = i & 0xFF;
	p[1] = (i & 0xFF00) >> 8;
	p[2] = (i & 0xFF0000) >> 16;
	p[3] = (i & 0xFF000000) >> 24;
}
static void seti16(unsigned char *p, int i) {
	p[0] = i & 0xFF;
	p[1] = (i & 0xFF00) >> 8;
}

int geti32(unsigned char *s);
short geti16(unsigned char *s);
void dmp_sndinfo(SND_INFO *info) {
	int min = info->sec / 60;
	printf("sndinfo: fmt_type=%08X, fmt_id=%04Xh, fmt_name=\x22%s\x22\n", info->fmt_type, info->fmt_id, info->fmt_name);
	printf("sndinfo: sound= %d Hz, %d bit, %d ch, %d bps, %d sample\n", info->rate, info->bit, info->ch, info->bps, info->samples);
	printf("sndinfo: body=%08X, len=%d, block=%d\n", info->body, info->len, info->block);
	printf("sndinfo: time=%6d sec (%3d:%02d:%02d)\n", info->sec, min / 60, min % 60, info->sec % 60);
}
//static char fmtname[] = "WAV";
static char *fmtname[] = {
	"PCM", "MS ADPCM", "IEEE PCM", "IBM CSVD", "A-Law", "u-Law", "OKI ADPCM", "IMA ADPCM",
	"MediaSpace ADPCM", "Sierra ADPCM", "ADPCM(G.723)", "YAMAHA ADPCM", "TrueSpeech", "DOLBY AC2",
	"GSM 6.10", "ADPCM(G.721)", "MPEG", "MP3", "CREATIVE ADPCM", "FMTOWNS SND", 
	"unknown"
};
static int fmttype[] = {
	0x0001, 0x0002, 0x0003, 0x0005, 0x0006, 0x0007, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0x0020, 0x0022, 0x0030, 
	0x0031, 0x0040, 0x0050, 0x0055, 0x0200, 0x0300, 
	0
};
static char fmt_name[32];
static int imastep[] = {
            7,     8,     9,    10,    11,    12,    13,    14,
           16,    17,    19,    21,    23,    25,    28,    31,
           34,    37,    41,    45,    50,    55,    60,    66,
           73,    80,    88,    97,   107,   118,   130,   143,
          157,   173,   190,   209,   230,   253,   279,   307,
          337,   371,   408,   449,   494,   544,   598,   658,
          724,   796,   876,   963,  1060,  1166,  1282,  1411,
         1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
         3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
         7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
static int imanext[] = {
	-1, -1, -1, -1, +2, +4, +6, +8
};
static int stepsizeTable[16] =
{
	57, 57, 57, 57, 77,102,128,153,
	57, 57, 57, 57, 77,102,128,153
};
static int pos=0, seeked=1;
static SND_INFO *inf;
static int fd;
int wav_check(char *buff, unsigned int size) {
	// ファイルヘッダのチェック
	//if ((size >= 44) && (c[0] == 0x52) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x46) && (c[8] == 0x57) && (c[9] == 0x41) && (c[10] == 0x56) && (c[11] == 0x45))	// RIFF....WAVE
	if ((size >= 44) && geti32(&buff[0]) == 0x46464952 && geti32(&buff[8]) == 0x45564157)	// RIFF...WAVE
		return 1;
	if (geti32(&buff[28]) == 0x3C && geti32(&buff[12]) && geti16(&buff[24]))	// key&reserve,samples,rate	return 0;
		return 1;
	return 0;
}
int wav_setup(char *buff, unsigned int size, SND_INFO *info) {
	// コーデックチェック
	int i,k,m,sz,id,sid,ssz;
	info->fmt_type = info->fmt_id = 0; info->head = buff; fd = -1;
	if (geti32(&buff[0]) == 0x46464952 && geti32(&buff[8]) == 0x45564157) {	// "RIFF" "WAVE"
		for (i=12; i<size; i+=sz+(sz&1)) {
			id = geti32(&buff[i]);
			sz = geti32(buff+i+4);
			//printf("wave: chunk=\x22%c%c%c%c\x22 ( 0x%08X ) , size=%d\n", buff[i], buff[i+1], buff[i+2], buff[i+3], id, sz);
			i+=8;
			if (id == 0x20746d66) {	// "fmt "
				info->fmt_id = geti16(buff+i);
				info->ch = geti16(buff+i+2);
				info->rate = geti32(buff+i+4);
				info->bps = geti32(buff+i+8)*8;
				info->block=geti16(buff+i+12);
				info->bit = geti16(buff+i+14);
				info->samples = 0;
			} else if (id == 0x74636166) {	// "fact"
				info->samples = geti32(buff+i);
			} else if (id == 0x61746164) {	// "data"
				info->body = i;
				info->len = sz;
			} else if (id == 0x5453494c && geti32(&buff[i]) == 0x4f464e49) {	// "LIST" "INFO"
			/*	MP3形式ファイルからWAV形式に変換したファイルの例
				0x44524349	ICRD	作成日			2009
				0x44525049	IPRD	製品名			Crazy for you
				0x4a425349	ISBJ	サブタイトル	Crazy for you (TV EDITION)
				0x4d414e49	INAM	名前			Crazy for you (TV EDITION)
				0x4b525449	ITRK	トラック		02
				0x524e4749	IGNR	ジャンル		Anime
				0x544d4349	ICMT	コメント		ウェブアニメ「こちらニコニコ放送局！」テーマソング
				0x54524149	IART	アーティスト	蘭(CV.ななひら)・奈帆(CV.これまり)・日刊たん(CV.彩)
			*/
				for(k=i+4,m=0; k<i+sz; k+=ssz+(ssz&1)) {
					sid = geti32(buff+k);
					ssz = geti32(buff+k+4);
					k+=8;
					if (sid == 0x4d414e49) {	// "INAM":名前
						printf("title: %s\n", &buff[k]);
					} else if (sid == 0x544d4349) {	// "ICMT":コメント
						printf("comment: %s\n", &buff[k]);
					} else if (sid == 0x54524149) {	// "IART":アーティスト
						printf("artist: %s\n", &buff[k]);
					} else if (sid == 0x504f4349) {	// "ICOP":著作権情報
						printf("copyright: %s\n", &buff[k]);
					}
				}
			}
		}
		//	if (info->bit)
		//		info->sec = info->len / (info->rate * info->ch * info->bit / 8);
		//	else 
		if (info->samples)
			info->sec = info->samples / info->rate;
		else if (info->bps)
			info->sec = info->len * 8 / info->bps;
		else 
			info->sec = -1;
		info->fmt_type = 0x45564157;
		for(i=0;;i++){
			if (!fmttype[i] || (info->fmt_id == fmttype[i])) {
				sprintf(fmt_name, "WAV(%s)", fmtname[i]);
				info->fmt_name = fmt_name;
				break;
			}
		}
		dmp_sndinfo(info);
		if(((info->fmt_id == 3) && (info->body & ((info->bit >> 3)-1))) 
		|| ((info->fmt_id ==17) && (info->body & 3))){
			// メモリアラインメント修正(float/double用)
			if (info->fmt_id==17)	sz = info->body & 3;
			else sz = info->body & ((info->bit >> 3)-1);
			printf("wav_setup: moving data chunk for memory alignment ( moving -%d bytes )\n", sz);
			info->body -= sz;
			memmove(&buff[info->body], &buff[info->body+sz], info->len);
		}
		int samples=0;
		if ((info->fmt_id == 1) || (info->fmt_id == 3) || (info->fmt_id == 6) || (info->fmt_id == 7)) 
			samples = info->len / (info->bit * info->ch / 8);
		else if (info->fmt_id == 0x0011) 
			samples = info->len / info->block * ((info->block - info->ch * 4) * 2 / info->ch +1);
		else if (info->fmt_id == 0x0020)
			samples = info->len / info->block * (info->block * 2 / info->ch);
		else if (info->fmt_id == 0x0300)
			samples = info->len - 32;
		printf("wav_setup: fact_samples: %d, bodysamples: %d, diffsamples: %d\n", info->samples, samples, samples - info->samples);
		if ((!info->samples || (info->samples > samples)) && samples) { 
			info->samples = samples;
		}
		if (!info->bit) info->bit = 16;
		//dmp_sndinfo(info);
		if (info->fmt_id == 0x11) 
			printf("wav_setup: sample per block: %d samples ( %d byte per block )\n", (info->block - info->ch * 4) * 2 / info->ch +1, info->block);
		inf = info;
		pos = 0;
		if (info->fmt_id == 0x55) return mp3_setup(buff, size, info);
		if (0 && info->fmt_id != 1 && info->fmt_id != 3 && info->fmt_id != 768) {
			fd = nopen("host:/dump.wav", O_WRONLY | O_CREAT | O_TRUNC);
			if (fd >= 0) {
				void seti32(unsigned char *p, int i);
				void seti16(unsigned char *p, int i);
				char head[] = "RIFF    WAVEfmt aaaabbccddddeeeeffggdata    ";
				seti32(head + 0x04, 36 + info->samples * info->ch * 2);
				seti32(head + 0x10, 16);
				seti16(head + 0x14, 1);
				seti16(head + 0x16, info->ch);
				seti32(head + 0x18, info->rate);
				seti32(head + 0x1C, info->rate * info->ch * 2);
				seti16(head + 0x20, info->ch * 2);
				seti16(head + 0x22, 16);
				seti32(head + 0x28, info->samples * info->ch * 2);
				nwrite(fd, head, 44);
			}
		}
	} else if (geti32(&buff[28]) == 0x3C && geti32(&buff[12]) && geti16(&buff[24])) {	// key&reserve,samples,rate
		info->fmt_id = 0x0300;
		info->ch = 1;
		info->rate = (geti16(&buff[24]) * 1000) / 98;// + ((signed short)geti16(&buff[26]));
		info->bps = info->rate * 8;
		info->block = 1;
		info->bit = 8;
		info->samples = geti32(&buff[12]);
		info->body = 0;
		info->len = info->samples +32;
		if (info->rate == 19193) info->rate = 19200;
		else if (info->rate == 9591) info->rate = 9600;
		if (!info->samples) info->samples = size - 32;
		info->sec = info->samples / info->rate;
		inf = info;
		info->fmt_type = 0x00444E53;
		for(i=0;;i++){
			if (!fmttype[i] || (info->fmt_id == fmttype[i])) {
				info->fmt_name = fmtname[i];
				break;
			}
		}
		dmp_sndinfo(info);
		pos = 0;
	}
	seeked = 1;
	return 0;
}
int wav_seek(int sample, int mode) {
	if (fd >= 0) return pos;
	switch(mode) {
		case SEEK_SET:
			pos = sample;
			break;
		case SEEK_CUR:
			pos+= sample;
			break;
		case SEEK_END:
			pos = inf->samples +sample;
			break;
	}
	if (pos < 0) {
		pos = inf->samples + (pos % inf->samples);
	}
	if (pos >= inf->samples)
		pos %= inf->samples;
	//	pos = sample;// * inf->bit * inf->ch;
	sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
	seeked = 1;
	if (inf->fmt_id == 0x55) return mp3_seek(sample, mode);
	return pos;
}

int wav_decode(unsigned int data) {
	if (inf->fmt_id == 0x55) return mp3_decode(data);
	return 0;
}

int wav_close(void) {
	if (fd >= 0) nclose(fd);
	if (inf->fmt_id == 0x55) return mp3_close();
	printf("wav_close: called\n");
	return 0;
}
void wav_alaw_decode(short *dst, char *src, int size) {
	unsigned char *s=(unsigned char *)src;
	unsigned short *d=(unsigned short *)dst;
	int i,j;//,a,b,c;
	for(i=0;i<size;i++) {
		j = *s++ ^ 0xD5;
		*d++ = (((j & 15) *2 +1 + 32 * ((j & 0x70) != 0)) << (((j >> 4) & 7) +2 + ((j & 0x70) == 0))) * ((j >> 7) *2 -1);
	//	j = s[i] ^ 0xD5;
	//	a = j & 0x0F; b = (j >> 4) & 7; c = j >> 7;
	//	if (b) a = (a * 2 + 0x21) << (b + 2); else a = (a * 2 + 1) << 3;
	//	d[i] = a * (c * 2 -1);
	}
}
void wav_ulaw_decode(short *dst, char *src, int size) {
	unsigned char *s=(unsigned char *)src;
	unsigned short *d=(unsigned short *)dst,j;
	int i;
	for(i=0;i<size;i++) {
		j = *s++ ^ 0xFF;
		*d++ = (((((j & 15) *2 + 33) << (((j >> 4) & 7) +2)) - 0x84) & 0x7FFF) * ((j >> 7) * 2 -1);
	}
}
void wav_ima_decode(short *dst, char *src, int block, int ch) {
	unsigned int *s=(unsigned int *)src,k;
	signed short *d=(signed short *)dst;
	int i,a,c,b,e,st,sn,v[ch],t[ch],dd;
	for(i=0;i<ch;i++){
		v[i] = *(signed short*)&src[i*4];
		t[i] = src[i*4+2];
		*d++ = v[i];
	}
	dd = (block - ch * 4) / 4 / ch;
	for(i=0;i<dd;i++){
		for(c=0;c<ch;c++){
			k = s[i*ch+c+ch];
			for(b=0;b<8;b++) {
				a = (k >> (b * 4)) & 7; sn = k & (1 << (b * 4 +3));
				st = imastep[t[c]];
				e = (st >> 3) + a * st / 4; if (sn) e = -e;
				v[c] += e;
				t[c] += imanext[a];
				if (v[c] < -32768) v[c] = -32768;
				if (v[c] > 32767) v[c] = 32767;
				if (t[c] < 0) t[c] = 0;
				if (t[c] > 88) t[c] = 88;
				d[(i*8+b)*ch+c] = v[c];
			}
		}
	}
}
static int yv[8], ys[8];
void wav_yamaha_decode(short *dst, unsigned char *src, int block, int ch) {
	signed short *d=(signed short *)dst;
	int i,a,c,b,e,dd,k,cm;
	dd = block / ch;
	cm = ch; if (cm > 8) cm = 8;
	if (seeked) {
		for(c=0;c<cm;c++){
			yv[c] = 0;
			ys[c] = 127;
		}
		seeked = 0;
	}
	for(i=0;i<dd;i++){
		for(c=0;c<cm;c++){
			k = src[i*ch+c];
			for(b=0;b<2;b++) {
				a = (k >> ((b^1) * 4)) & 15;
				e = ((a & 7) * 2 +1) * ys[c] / 8;
				if (a & 8) yv[c] += e; else yv[c] -= e;
				if (yv[c] < -32768) yv[c] = -32768;
				if (yv[c] > 32767) yv[c] = 32767;
				ys[c] = ys[c] * stepsizeTable[a] / 64;
				if (ys[c] < 127) ys[c] = 127;
				if (ys[c] > 24576) ys[c] = 24576;
				d[(i*2+b)*ch+c] = yv[c];
			}
		}
	}
}
int wav_callback(void) {
	int k,m=0;
	if ((inf->fmt_id == 1) || (inf->fmt_id == 3)) {
		int block=inf->bit * inf->ch / 8,limit=64;
		while(pcmempty && pos < inf->samples) {
			k = pcmadds(&inf->head[inf->body + pos * block], inf->samples - pos, inf->rate, inf->bit, inf->ch, inf->fmt_id==3);
			pos += k; m += k;
			if (pcmloop && pos >= inf->samples) pos = 0;
			if (--limit == 0) break;
		}
		//printf("wav_callback: pos=%7d, reads=%7d\n", pos, m);
		sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
	} else if (inf->fmt_id == 768) {
		if (inf->bit == 8) {
			int limit=64,rate=inf->rate;
			//	rate = geti16(&inf->head[inf->body + 24]) * 1000 / 98;// + geti16(&inf->head[inf->body + 26]);
			//	if (rate == 19193) rate = 19200;
			//	else if (rate == 9591) rate = 9600;
			//	sndview_rate = rate;
			while(pcmempty && pos < inf->samples) {
				k = pcmadds(&inf->head[inf->body + pos +32], inf->samples - pos, rate, inf->bit, inf->ch, 1);
				pos += k; m += k;
				if (pcmloop && pos >= inf->samples) pos = 0;
				if (--limit == 0) break;
			}
			sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
		}
	} else if ((inf->fmt_id == 6) || (inf->fmt_id == 7)) {
		if (inf->bit == 8) {
			short temp[1024 * inf->ch];	// 4KB
			int i,r,limit=64;
			while(pcmempty && pos < inf->samples) {
				r = inf->samples -pos; i = pos * inf->ch;
				if (r > 1024) r = 1024;
				if (inf->fmt_id == 6)	wav_alaw_decode(temp, &inf->head[inf->body + i], r * inf->ch);
				else					wav_ulaw_decode(temp, &inf->head[inf->body + i], r * inf->ch);
				k = pcmadds((char*)&temp[0], r, inf->rate, 16, inf->ch, 0);
				pos += k; m += k;
				if (fd >= 0) {nwrite(fd,temp,k*2*inf->ch);if(k&&pos>=inf->samples){nclose(fd);fd=-1;}}
				if (pcmloop && pos >= inf->samples) pos = 0;
				if (--limit == 0) break;
			}
			sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
		}
	} else if (inf->fmt_id == 17) {
		if (inf->bit == 4) {
			int b,o,i,r,limit=64;
			r = (inf->block - inf->ch * 4) * 2 / inf->ch +1;
			short temp[r * inf->ch];
			while(pcmempty && pos < inf->samples) {
				b = pos / r * inf->block;	o = pos % r;
				wav_ima_decode(temp, &inf->head[inf->body + b], inf->block, inf->ch);
				i = r - o; if (pos + i > inf->samples) i = inf->samples - pos;
				k = pcmadds((char*)&temp[o * inf->ch], i, inf->rate, 16, inf->ch, 0);
				pos += k; m += k;
				if (fd >= 0) {nwrite(fd,&temp[o*inf->ch],k*2*inf->ch);if(k&&pos>=inf->samples){nclose(fd);fd=-1;}}
				if (pcmloop && pos >= inf->samples) pos = 0;
				if (--limit == 0) break;
			}
			sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
		}
	} else if (inf->fmt_id == 32) {
		if (inf->bit == 4) {
			int b,o,i,r,limit=64;
			r = inf->block * 2 / inf->ch;
			short temp[r * inf->ch];
			while(pcmempty && pos < inf->samples) {
				b = pos / r * inf->block;	o = pos % r;
				wav_yamaha_decode(temp, &inf->head[inf->body + b], inf->block, inf->ch);
				i = r - o; if (pos + i > inf->samples) i = inf->samples - pos;
				k = pcmadds((char*)&temp[o * inf->ch], i, inf->rate, 16, inf->ch, 0);
				pos += k; m += k;
				if (fd >= 0) {nwrite(fd,&temp[o*inf->ch],k*2*inf->ch);if(k&&pos>=inf->samples){nclose(fd);fd=-1;}}
				if (pcmloop && pos >= inf->samples) pos = 0;
				if (--limit == 0) break;
			}
			
			sndview_now = ((pos +inf->samples -sndview_delay) % inf->samples) / inf->rate;
		}
	}
	else if (inf->fmt_id == 0x55) return mp3_callback();
	//	printf("wav_callback: added %5d samples\r", m);
//	sndview_rptr = pos * block;
	if (pcmempty && m) m += pcmadds(0, 0, 0, 0, 0, 0);
	return m;
}
int wav_func(SND_FUNC *data) {
	data->check		= wav_check;
	data->open		= wav_setup;
	data->seek		= wav_seek;
	data->decode	= wav_decode;
	data->close		= wav_close;
	data->callback	= wav_callback;
	return 0;
}
