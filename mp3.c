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
////////////////////////////////
// デバッグ用トラップ
extern int viewmallocs;
static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("viewer: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("viewer: malloc valid (ofs: %08X, size: %d) [%d]\n", (unsigned int) ret, mallocsize, ++viewmallocs);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("viewer: free valid (ofs: %08X) [%d]\n", (unsigned int) mallocdata, --viewmallocs);
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
enum {
	MP3,WAV,FLV,NUL
};
enum {
	MP3FrameSync=0, 
	MP3Version, 
	MP3Layer, 
	MP3Protected, 
	MP3Bitrate, 
	MP3Frequency, 
	MP3Padding, 
	MP3Private, 
	MP3Channel, 
	MP3Extension, 
	MP3Copyright, 
	MP3Original, 
	MP3Emphasis,
};
enum {
	soffset=0,
	sprivate,
	sscfsi,
};
enum {
	part2_3_length=0,
	big_values,
	global_gain,
	scalefac_compress,
	window_switching_flag,
	window_data,
	preflag,
	scalefac_scale,
	count1table_select,
	gras,
};
static int listsync[] = {11,2,2,1,4,2,1,1,2,2,1,1,2};
static int freq[4] = {44100, 48000, 32000, 0};
static int kbps[16*5] = {	0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0,	//	V1L1
							0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0,	//	V1L2
							0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0,	//	V1L3
							0,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,0,	//	V2L1
							0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0,	//	V2L2-3
						};
static int ver[4] = {3,0,2,1};
static int listgranule[] = {12,9,8,4,1,22,1,1,1};
static int ratesrc[9] = {44100,48000,32000,22050,24000,16000,11025,12000,8000};
static int lls[9] = {320,294,441,320,294,441,640,588,882};
static int pos=0, seeked=1;
static SND_INFO *info;
static int type;
static char *fmtname[] = {"MP3","WAV(MP3)", "FLV(MP3)","unknown"};
typedef struct {
	//	int pos;	// start sampling data number in mp3
	int ofs;	// offset of frame
	//	int rate;	// rate of frame
	//	short len;	// sample per frame
	//	short ch;	// channels of frame
} MP3_FI;
static MP3_FI *mp3;
int getb32(unsigned char *s);
short geti16(unsigned char *s);
int scasbn(unsigned char *buf, unsigned int limit, unsigned char chr, int ofs);
void dmp_sndinfo(SND_INFO *info);
static int mp3_explode_sync(unsigned int i, int *data) {
	int k,sz;
	for (k=0,sz=0; k<13; sz+=listsync[k],k++) {
		//printf("%2d: %d,%d\n", k, (32 - sz -list[k]) , (1 << list[k])-1);
		//printf("%c: %d\n", k+0x41, (i >> (32 - sz -list[k])) & (1 << list[k])-1);
		data[k] = (i >> (32 - sz -listsync[k])) & ((1 << listsync[k])-1);
	}
	return 0;
}
static int mp3_id3size(char *buff) {
	int m = -1;
	if ((buff[0] == 0x49) && (buff[1] == 0x44)) {
		m = 10 + (((int)buff[6] & 127) << 21) + (((int)buff[7] & 127) << 14) + (((int)buff[8] & 127) << 7) + (((int)buff[9] & 127) << 0);
		printf("mp3: ID3v2 header size: %d ( 0x%08X )\n", m, m);
	}
	return m;
}
static int mp3_detect_sync(unsigned int i) {
	int data[13];
	mp3_explode_sync(i, &data[0]);
	return (data[MP3Version] == 1) || (data[MP3Layer] == 0) || (data[MP3Bitrate] == 15) || (data[MP3Frequency] == 3) || (data[MP3Emphasis] == 2);
}
static int mp3_search_sync(unsigned char *buff, unsigned int size, int ofs) {
	int m=ofs;
	if (ofs < 0) m = 0;
	if (type == FLV) {
		while((m=scasbn(buff, size, 0xFF, m))>=0) {
			if (buff[++m] >=0xE0 && buff[m] < 0xFF && (buff[m-2] & 0xFE) == 0x2E && buff[m-3] == 0x00) {
				if (mp3_detect_sync(getb32(buff+m-1))) continue;
				break;
			}
		}
	} else {
		while((m=scasbn(buff, size, 0xFF, m))>=0)
			if (buff[++m] >=0xE0 && buff[m] < 0xFF) {
				if (mp3_detect_sync(getb32(buff+m-1))) continue;
				break;
			}
	}
	if (m > 0) m--; else m = -1;
	return m;
}
int strpos(char *target, int limit, char *src, int start);
static int mp3_xing_info(unsigned char *src, unsigned int size, int *dist) {
	int ofs,nex,p,i,f=0;
	ofs = mp3_search_sync(src, size, -1);
	if (ofs < 0) return -1;
	nex = mp3_search_sync(src, size, ofs+1);
	if (nex < 0) nex = ofs + 256;
	if (nex > size) nex = size;
	p = strpos(src, nex, "Xing", ofs);
	if (p <= 0) 
		p = strpos(src, nex, "Info", ofs);
	printf("mp3_xing_info: offset:%08X ( %d )\n", p, p);
	if (p > 0) {
		if (dist) {
			for(i=0;i<4;i++) {
				dist[i] = getb32(&src[p+i*4]);
				dist[i+4] = getb32(&src[p+i*4+116]);
			}
			f = dist[2];
		} else
			f = getb32(&src[p+8]);
		printf("mp3_xing_info: flags:%02X, frames:%6d, filesize:%9d\n", getb32(&src[p+4]), f, getb32(&src[p+12]));
	}
	return f;
}
static unsigned int bitptr, bitend;					// ビット位置、ビット数
static unsigned char *bitbuf;
static unsigned int getbits(u32 bits)
{	// ・コードは上位のビット、低位のバイトから割り当て
	// ・残りビット数が足りないときはマイナス値を返す
	//   ビット配置例:
	//         b76543210
	//   (低位) aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh(高位)
	//   (上位) 43210432 (下位)
	//	bitptr:            111111 11112222
	//       <- 01234567 89012345 67890123 ->
	//			aaaaaaaa abbbcccc cccccccc
	//			aaaaabbb bbbccccc cccccccc
	//		  
	int p,b,u,i;

	p = (bitptr + bits) >> 3; b = (bitptr + bits) & 7;
	u = ((bitptr & 7) + bits +7) >> 3;
	i = (unsigned int) bitbuf[p];
	if (u > 1) {
		i |= (unsigned int) bitbuf[p-1] << 8;
		if (u > 2) {
			i |= (unsigned int) bitbuf[p-2] << 16;
			if (u > 3) {
				i |= (unsigned int) bitbuf[p-3] << 24;
				if (u > 4) printf("MP3_getbits: code size overflow ( %d bytes )\n", u);
			}
		}
	}
	bitptr += bits;
	return (i >> ((8 - b) & 7)) & ((1 << bits) -1);
}
static int mp3_sideinfo(unsigned char *buff, int ch, int rate, int *scfsi, int *data) {
	int ofs,g,c,i;
	bitptr = 0; bitbuf = buff;
	ofs = getbits(9);
	if (rate > 24000) {
		if (ch == 1) {
			bitptr += 5;
			*scfsi = getbits(4);
		} else {
			bitptr += 3;
			*scfsi = getbits(8);
		}
	} else {
		if (ch == 1) {
			*scfsi = getbits(4);
		} else {
			bitptr ++;
			*scfsi = getbits(8);
		}
	}
	for(g=0;g<1+(rate>24000);g++){
		for(c=0;c<ch;c++){
			 for(i=0;i<gras;i++){
			 	data[(g*2+c)*gras+i] = getbits(listgranule[i]);
			 }
		}
	}
	return ofs;
}
int mp3_check(char *src, unsigned int size) {
	// ファイルヘッダのチェック
	unsigned char *buff=(unsigned char *)src;
		 if (buff[0] == 0x49) type = MP3;
	else if (buff[0] == 0xFF) type = MP3;
	else if (buff[0] == 0x46) type = FLV;
	else if (buff[0] == 0x52) type = WAV;
	else if (buff[0] == 0x00) type = NUL;
	else type = NUL;
	if ((buff[0]==0xFF && buff[1]>=0xE0) || (buff[0]==0x49&&buff[1]==0x44&&buff[2]==0x33) || !strncmp(buff, "FLV\x01", 4))	// syncbit and "ID3"
		return 1;
	return 0;
}
int mp3_setup(char *src, unsigned int size, SND_INFO *inf) {
	// コーデックチェック
	unsigned char *buff=(unsigned char *)src;
	int k,m=-1,p=0,mxf=0,osz=0; MP3_FI *mp3t;
	info = inf;
	//info->fmt_type = 
	info->body = 0; info->head = buff; mp3 = NULL;
	if ((buff[0]==0xFF && buff[1]>=0xE0) || (buff[0]==0x49&&buff[1]==0x44&&buff[2]==0x33) || !strncmp(buff, "FLV\x01", 4) || !strncmp(buff, "RIFF", 4)) {	// syncbit and "ID3"
		//info->fmt_id = 0x55; 
		info->bit = 16;
		k = mp3_id3size(buff);
		if (k >= 0) m = k;
		m = mp3_search_sync(buff, size, m);
		k = mp3_xing_info(buff, size, 0);
		if (k)	mxf = k+1;
		else	mxf = 2000;
		mp3 = (MP3_FI*)malloc(sizeof(MP3_FI) * mxf);
		if (!mp3) return -2;
		if (m >= 0) {
			printf("mp3_setup: mp3 first frame offset: %d ( 0x%08X )\n", m, m);
			info->body = m;
			int data[13],ll=0,tt=0,sz,lll[9]={0,0,0,0,0,0,0,0,0};
			/*//
				7*7*5*5*5*2=12250
				rate	samples	msec	x12250s	fps		
				  48000	   1152	  24	  294	41.667	
				  44100	   1152	  26.12	  320	38.281	
				  32000	   1152	  36	  441	27.778	
				  24000		576	  24	  294	41.667	
				  22050		576	  26.12	  320	38.281	
				  16000		576	  36	  441	27.778	
				  12000		576	  48	  588	20.833	
				  11025		576	  52.24	  640	19.14	
				   8000		576	  72	  882	13.889	
			//*/
			printf("frame  offset   rate  bps  size\n");
			//		_____0 00000000 44100 320k  418
			while(m >= 0) {
				mp3_explode_sync(getb32(buff+m), &data[0]);
				info->rate = freq[data[MP3Frequency]] >> (ver[data[MP3Version]]-1);
				sz = kbps[((3-data[MP3Layer])+(ver[data[MP3Version]]>>1)*3 -((data[MP3Layer]==1)&&(ver[data[MP3Version]]>1)))*16+data[MP3Bitrate]]*1000;
				if (!sz && osz) sz = osz;
				if (sz * info->rate) {
					if (data[MP3Layer]==3) 	k = ((12 >> (data[MP3Version] != 3)) * sz / info->rate + data[MP3Padding]) * 4;
					else					k = (144 >> (data[MP3Version] != 3)) * sz / info->rate + data[MP3Padding];
				} else {	k = 0;}
				osz = sz;
				if (k || !data[MP3Bitrate]) {
					if (mp3 && ll == mxf) {
						mxf += 2000;
						mp3t = (MP3_FI*)realloc(mp3, sizeof(MP3_FI) * mxf);
						if (mp3t) mp3 = mp3t; else mxf -= 2000;
					}
					if (mp3 && ll<mxf) {
						mp3[ll] = (MP3_FI){/*.pos=p,*/.ofs=m};//,.rate=info->rate,.ch=(data[MP3Channel]!=3)+1,.len=1152>>(data[MP3Version]!=3)};
						p += 1152 >> (info->rate < 32000);
					}
					ll++;
					lll[data[MP3Frequency] + 3 * (ver[data[MP3Version]]-1)]++;
					if (ll <= 50)	printf("%6d %08X %5d %3dk %4d\n", ll, m, info->rate, sz/1000, k);
					tt += sz / 1000;
				}
				if ((m+k >= size) || (buff[m+k] != 0xFF) || (k==0)) {
					if ((m+k < size) && (buff[0] == 0x46)) {
						m+=k; if (!k) m++;
						m = mp3_search_sync(buff, size, m);
						if (m > 0) {continue;}
					} else if (!data[MP3Bitrate]) {
						int z;
						for(z=1;z<15;z++) {
							osz = kbps[((3-data[MP3Layer])+(ver[data[MP3Version]]>>1)*3 -((data[MP3Layer]==1)&&(ver[data[MP3Version]]>1)))*16+z]*1000;
							if (data[MP3Layer]==3) 	k = ((12 >> (data[MP3Version] != 3)) * osz / info->rate + data[MP3Padding]) * 4;
							else					k = (144 >> (data[MP3Version] != 3)) * osz / info->rate + data[MP3Padding];
							if(((buff[m+k] == 0xFF && buff[m+k+1] == buff[m+1]))
							|| ((buff[m+k+1]==0xFF && buff[m+k+2] == buff[m+1]))) {
								if (buff[m+k+2] == buff[m+1]) m++;
								m += k; 
								tt += osz / 1000;
								if (ll <= 50)	printf("%6d %08X %5d %3dk %4d %3dk\n", ll, m, info->rate, sz/1000, k, osz/1000);
								break;
							}
						}
						if (z<15&&k&&m>0) continue;
					}	break;
				}
				m+=k;
			}
			delay(1);
			if (mp3 && mxf > ll) {
				mp3t = (MP3_FI*)realloc(mp3, sizeof(MP3_FI) * ll);
				if (mp3t) {
					mp3 = mp3t;
					mxf = ll;
				}
			}
			info->len = m - info->body;
			info->ch = (data[MP3Channel] != 3) +1;
			printf("mp3_setup: mpeg audio total frames: %d\n", ll);
			{
				int sv,ss,maxrate=0,mm;
				for(k=0,sv=0,ss=0;k<9;k++){
					mm = lll[k] * lls[k] / 1225;
					sv += lll[k] * lls[k]; ss += lll[k] * 576 * ((k<3)+1);
					if (lll[k] && maxrate < ratesrc[k]) maxrate = ratesrc[k];
					printf("mp3_setup: %5d Hz *%8d frames =>%6d.%d sec,%10ld samples\n", ratesrc[k], lll[k], mm / 10, mm % 10, 576ul * lll[k] * ((k<3)+1));
				}
				mm = sv / 1225;
				printf("mp3_setup: total%9d frames:%7d.%d sec,%10d samples\n", ll, mm / 10, mm % 10, ss);
				info->sec = sv / 12250;
				info->rate = 12250ul * ss / sv;// maxrate;
				if (!info->samples) info->samples = ss;
			}
			info->bps = tt / ll * 1000;
			//	sprintf(format, "MP%d", 4-data[MP3Layer]);
			info->fmt_id = 0x55; info->fmt_type = 0x0033504D;
			if (type != WAV) info->fmt_name = fmtname[type];
			if (mp3) {
				char temp[512], tmpb[4096]; int rate,z,fd,templ,tmpbl,ofs,si,side[gras*4];
				memset(&side[0], 0, gras*16);
				printf("mp3_setup: temp=%08X, tmpb=%08X\n", (int)&temp[0], (int)&tmpb[0]);
				fd = nopen("host:/mp3list.txt", O_WRONLY | O_CREAT | O_TRUNC);
				if (fd >= 0) {
					strcpy(temp, "frame  offset   rate  bps  size"
								" main si bits big gain sc w data   p s 1\r\n");
					nwrite(fd, temp, strlen(temp));
					for(z=0,osz=0,templ=0,tmpbl=0,tmpb[0]=0;z<ll;z++){
						mp3_explode_sync(getb32(buff+mp3[z].ofs), &data[0]);
						rate = freq[data[MP3Frequency]] >> (ver[data[MP3Version]]-1);
						sz = kbps[((3-data[MP3Layer])+(ver[data[MP3Version]]>>1)*3 -((data[MP3Layer]==1)&&(ver[data[MP3Version]]>1)))*16+data[MP3Bitrate]]*1000;
						if (!sz && osz) sz = osz;
						if (sz * rate) {
							if (data[MP3Layer]==3) 	k = ((12 >> (data[MP3Version] != 3)) * sz / rate + data[MP3Padding]) * 4;
							else					k = (144 >> (data[MP3Version] != 3)) * sz / rate + data[MP3Padding];
						} else {	k = 0;}
						osz = sz;
						ofs = mp3_sideinfo(buff+mp3[z].ofs+4, (data[MP3Channel]!=3)+1, rate, &si, &side[0]);
						if (rate > 24000) {
							if (data[MP3Channel] != 3) 
								sprintf(temp, "%6d %08X %5d %3dk %4d"
										 " %4d %02x %4d %3d %4d %2d %d %06X %d %d %d\r\n"
												  "%44d %3d %4d %2d %d %06X %d %d %d\r\n"
												  "%44d %3d %4d %2d %d %06X %d %d %d\r\n"
												  "%44d %3d %4d %2d %d %06X %d %d %d\r\n",
										 	z+1, mp3[z].ofs, rate, sz/1000, k, 
											-ofs,si,side[ 0], side[ 1], side[ 2], side[ 3], side[ 4], side[ 5], side[ 6], side[ 7], side[ 8],
													side[ 9], side[10], side[11], side[12], side[13], side[14], side[15], side[16], side[17],
													side[18], side[19], side[20], side[21], side[22], side[23], side[24], side[25], side[26],
													side[27], side[28], side[29], side[30], side[31], side[32], side[33], side[34], side[35]
														);
							else 
								sprintf(temp, "%6d %08X %5d %3dk %4d"
										 " %4d %02x %4d %3d %4d %2d %d %06X %d %d %d\r\n"
												  "%44d %3d %4d %2d %d %06X %d %d %d\r\n",
										 	z+1, mp3[z].ofs, rate, sz/1000, k, 
											-ofs,si,side[ 0], side[ 1], side[ 2], side[ 3], side[ 4], side[ 5], side[ 6], side[ 7], side[ 8],
													side[18], side[19], side[20], side[21], side[22], side[23], side[24], side[25], side[26]
														);
						} else {
							if (data[MP3Channel] != 3) 
								sprintf(temp, "%6d %08X %5d %3dk %4d"
										 " %4d %02x %4d %3d %4d %2d %d %06X %d %d %d\r\n"
												  "%44d %3d %4d %2d %d %06X %d %d %d\r\n",
										 	z+1, mp3[z].ofs, rate, sz/1000, k, 
											-ofs,si,side[ 0], side[ 1], side[ 2], side[ 3], side[ 4], side[ 5], side[ 6], side[ 7], side[ 8],
													side[ 9], side[10], side[11], side[12], side[13], side[14], side[15], side[16], side[17]
														);
							else 
								sprintf(temp, "%6d %08X %5d %3dk %4d"
										 " %4d %02x %4d %3d %4d %2d %d %06X %d %d %d\r\n",
										 	z+1, mp3[z].ofs, rate, sz/1000, k, 
											-ofs,si,side[ 0], side[ 1], side[ 2], side[ 3], side[ 4], side[ 5], side[ 6], side[ 7], side[ 8]
														);
						}
						if (!templ) templ = strlen(temp);
						if (tmpbl + templ >= 4096) {
							nwrite(fd, tmpb, tmpbl);
							tmpbl = 0;
						}
						memcpy(&tmpb[tmpbl], temp, templ);
						tmpbl += templ;
					}
					if (tmpbl) {
						nwrite(fd, tmpb, tmpbl);
					}
					nclose(fd);
				}
			}
		} else {
			info->fmt_id = 0; info->sec = 0;
			//	strcpy(format, "MPA");
		}
		dmp_sndinfo(info);
		pos = 0; seeked = 1;
	}
	return 0;
}
int mp3_seek(int sample, int mode) {
	switch(mode) {
		case SEEK_SET:
			pos = sample;
			break;
		case SEEK_CUR:
			pos+= sample;
			break;
		case SEEK_END:
			pos = info->samples +sample;
			break;
	}
	if (info->samples) {
		if (pos < 0) {
			pos = info->samples + (pos % info->samples);
		}
		if (pos >= info->samples)
			pos %= info->samples;
		//	pos = sample;// * info->bit * info->ch;
	}
	seeked = 1;
	sndview_now = ((pos +info->samples -sndview_delay) % info->samples) / info->rate;
	return pos;
}
int mp3_decode(unsigned int data) {
	return 0;
}
int mp3_close(void) {
	printf("mp3_close: called\n");
	if (mp3) free(mp3);	mp3 = NULL;
	return 0;
}
int mp3_clear(unsigned char *buff) {
	return 0;
}
int mp3_decoder(unsigned char *buff, int *rate, int *ch) {
	int data[13],sz,k;
	
	mp3_explode_sync(getb32(buff), &data[0]);
	*rate = freq[data[MP3Frequency]] >> (ver[data[MP3Version]]-1);
	sz = kbps[((3-data[MP3Layer])+(ver[data[MP3Version]]>>1)*3 -((data[MP3Layer]==1)&&(ver[data[MP3Version]]>1)))*16+data[MP3Bitrate]]*1000;
	if (*rate * sz) {
		if (data[MP3Layer]==3) 	k = ((12 >> (data[MP3Version] != 3)) * sz / *rate + data[MP3Padding]) * 4;
		else					k = (144 >> (data[MP3Version] != 3)) * sz / *rate + data[MP3Padding];
	} else {	k = 0;}
	*ch = (data[MP3Channel] != 3) +1;
	return 0;
}
int mp3_callback(void) {
	static short temp[1152 * 2];
	int f,o,i,k,m=0,limit=32,rate,ch;//block=info->ch * 2,
	memset(temp, 0, 1152 * 2);
	while(pcmempty && pos < info->samples) {
		f = pos / 1152;	o = pos % 1152;
		i = 1152 - o; if (pos + i > info->samples) i = info->samples - pos;
		if (seeked) mp3_clear(&info->head[mp3[f-(f>0)].ofs]);
		mp3_decoder(&info->head[mp3[f].ofs], &rate, &ch);
		if (!rate) rate = info->rate;
		if (!ch) ch = info->ch;
		k = pcmadds((char*)&temp[o * info->ch], i, rate, 16, ch, 0);
		pos += k; m += k;
		if (pcmloop && pos >= info->samples) pos = 0;
		if (--limit == 0) break;
	}
	sndview_now = ((pos +info->samples -sndview_delay) % info->samples) / info->rate;
	return m;
}
int mp3_func(SND_FUNC *data) {
	data->check		= mp3_check;
	data->open		= mp3_setup;
	data->seek		= mp3_seek;
	data->decode	= mp3_decode;
	data->close		= mp3_close;
	data->callback	= mp3_callback;
	return 0;
}
