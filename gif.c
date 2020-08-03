#include "launchelf.h"
//#define	DUMP_DECODE
//#define	DEBUG
//#define printf(...)	
//#ifdef DEBUG
static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("GIF: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("GIF: malloc vaild (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("GIF: free vaild (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("GIF: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
#define	malloc	X_malloc
#define free	X_free
//#endif
////////////////////////////////
// GIF形式の詳細情報取得
//	info[0]	形式ID(固定値)
//	info[1]	色ビット数
//	info[2]	幅
//	info[3]	高さ
//	size:	バッファのサイズ
//	buff:	バッファ
//	ret	=0:	invaild format
//		!0:	OK
int info_GIF(int *info, char *buff, int size)
{
	unsigned char *c=(unsigned char *) buff;

	if ((c[0] != 0x47) || (c[1] != 0x49) || (c[2] != 0x46) || (c[3] < 0x30) || (c[3] > 0x39) || (c[4] < 0x30) || (c[4] > 0x39) || (c[5] < 0x61) || (c[5] > 0x7A))
		return 0;
	c+=6;
	
	info[0] = 0x0008;
	info[1] = (c[4] & 0x07)+1;
	info[2] = (int) c[0] | ((int) c[1] << 8);
	info[3] = (int) c[2] | ((int) c[3] << 8);
	return 1;
}

////////////////////////////////
// LZW圧縮のデコード用
static unsigned char *package=NULL;
static int bitptr, bitrem;
static unsigned int gifleft, giftop, gifwidth, gifheight, gifsize, gifstatus;
static unsigned int width, height, gifx, gify, interlaced, changes;
static unsigned char *gifbuff, *gifoldtmp;
static unsigned char *gifext;
//static unsigned int gifol, gifot, gifow, gifoh;
static unsigned char gifof;
#define	TCF	((gifext != NULL) && (gifext[1] & 0x01))
#define	TCI (gifext[4])
#define delaytime	(256*gifext[3]+gifext[2])
static int codebits;
static int clearcode, eoicode, firstbits, prevcode, tableindex;
static uint64 globalclut[256];
static uint64 localclut[256];
extern uint64 *activeclut;
typedef struct {
	signed   short length;
	signed   short prevcode;
	unsigned char  charcode;
	unsigned char  firstchar;
} LZWDATA;
static LZWDATA *lzwtable=NULL;

static void setpackage(unsigned char *src)
{
	package = (unsigned char *) src;
	bitptr = 8;
	bitrem = (int) package[0] << 3;
	return;
}
static int getcode(void)
{	// ・コードは下位のビット、低位のバイトから割り当て
	// ・コードのバット配置は通常
	//   ビット配置例:
	//   (低位) bbbaaaaa dcccccbb eeeedddd ggfffffe hhhhhggg(高位)
	//   (上位) 21043210 (下位)
	int p,b,u,r,i;
	
	if (package[0] == 0) return -1;
	p = bitptr >> 3; b = bitptr & 7;
	u = (b+codebits+7) >> 3; r = (bitrem +7) >> 3;
	//i = (( ((int) package[p+(u==0)]) | ((int) package[p+1] << 8) | ((int) package[p+2] << 16)) >> b) & ((1 << codebits) -1);
	i = ((int) package[p+(r<1)]);
	if (u > 1) {
		i|= ((int) package[p+1+(r<2)] << 8);
		if (u > 2)
			i|= ((int) package[p+2+(r<3)] << 16);
	}
	
	bitptr+= codebits;
	bitrem-= codebits;
	if (bitrem <= 0) {
		//printf("getcode: set next package before (ptr,rem,package=%4d,%4d,%08x)\n", bitptr, bitrem, (int) package);
		bitptr -= 8*package[0];
		package += package[0]+1;
		bitrem += 8*package[0];
		//printf("getcode: set next package after  (ptr,rem,package=%4d,%4d,%08x)\n", bitptr, bitrem, (int) package);
	}
	//u = (i >> b) & ((1 << codebits) -1);
	//printf("getcode:%4d (package=%08x,bitptr=%4d,bitrem=%4d,codebits=%2d,len=%3d,tablenext=%4d)\n", u, (int) package, bitptr, bitrem, codebits, lzwtable[u].length, tableindex);
	//for (i=0; i<500000; i++) {}
	//return u;
	return (i >> b) & ((1 << codebits) -1);
}
static void gifsetup(int bits)
{
	int i;
	clearcode = 1<<bits;
	eoicode = clearcode+1;
	tableindex = eoicode+1;
	codebits = bits+1;
	//printf("gifsetup: firstbits=%d, codebits=%d, clearcode=%d, eoi=%d\n", bits, codebits, clearcode, eoicode);
	for (i=0; i<4096; i++) {
		lzwtable[i].length = 0;
		lzwtable[i].prevcode = 0;
		lzwtable[i].charcode = 0;
		lzwtable[i].firstchar = 0;
	}
	for (i=0; i<(1<<bits); i++) {
		lzwtable[i].length = 1;
		lzwtable[i].charcode = i;
		lzwtable[i].firstchar = i;
	}
}
static int gifadd(int prev, unsigned char add)
{
	int i;
	i = tableindex++;
	lzwtable[i].firstchar = lzwtable[prev].firstchar;
	lzwtable[i].length = lzwtable[prev].length +1;
	lzwtable[i].prevcode = prev;
	lzwtable[i].charcode = add;
	//printf("gifadd: to=%4d(first=%02x, add=%02x, len=%3d), prev=%4d(first=%02x, char=%02x, len=%3d)\n", i, lzwtable[i].firstchar, lzwtable[i].charcode, lzwtable[i].length, prev, lzwtable[prev].firstchar, lzwtable[prev].charcode, lzwtable[prev].length);
	if ((tableindex == (1 << codebits)) && (codebits < 12)) codebits++;
	return i;
}
static void gifpset(unsigned char c)
{
	if (!TCF || (TCI != c) || (changes == 0)) 
		gifbuff[(giftop+gify)*width+gifleft+gifx] = c;
	if (++gifx >= gifwidth) {
		gifx = 0; 
		if (interlaced == 0) {
			gify++;
			if (gify >= gifheight)
				gifstatus = 1;
		} else if (interlaced == 1) {
			gify+=8;
			if (gify >= gifheight) {
				interlaced++;
				gify = 4;
			}
		} else if (interlaced == 2) {
			gify+=8;
			if (gify >= gifheight) {
				interlaced++;
				gify = 2;
			}
		} else if (interlaced == 3) {
			gify+=4;
			if (gify >= gifheight) {
				interlaced++;
				gify = 1;
			}
		} else {
			gify+=2;
			if (gify >= gifheight)
				gifstatus = 1;
		}
	}
}
static void gifwrite(int code, int wlen)
{
	static int nest=0;
	
	if (wlen == 0)
		printf("gifwrite: write length is 0, set to %d\n", wlen=lzwtable[code].length);
	//	wlen = lzwtable[code].length;
	if (wlen == 0) {
		gifstatus = 1;
		return;
	}
	
	nest++;
	if (wlen != lzwtable[code].length) {
		printf("gifwrite: wrong length. addr=(%d,%d), wlen=%d, code=%4d, tlen=%d\n", gifx, gify, wlen, code, lzwtable[code].length);
		gifstatus = 1;
		nest--;
		return;
	}
	
	if (wlen > 1) {
		gifwrite(lzwtable[code].prevcode, wlen-1);
		gifpset(lzwtable[code].charcode);
	//	printf("%02x,", lzwtable[code].charcode);
	} else {
		gifpset(lzwtable[code].charcode);
	//	printf("gifwrite: %08x = %02x,", gifptr-1, lzwtable[code].charcode);
	}
	//if (--nest == 0) printf("\n");
	--nest;
}

////////////////////////////////
// GIF形式のデコード
#ifdef DUMP_DECODE
void seti32(unsigned char *p, int i) {
	p[0] = i & 0xFF;
	p[1] = (i & 0xFF00) >> 8;
	p[2] = (i & 0xFF0000) >> 16;
	p[3] = (i & 0xFF000000) >> 24;
}//*/
#endif
int decode_GIF(char *dist, char *src, int size, int bpp)
{
	unsigned char *c=(unsigned char *) src, gifback;
	int siz;
	//unsigned char bgcolor;
	int i,m;
	int cnt,imgs;
	c+=6; imgs=changes=0;
	lzwtable = (LZWDATA*)malloc(sizeof(LZWDATA)*4096);
	if (lzwtable == NULL) return -2;
	width  = gifwidth  = (int) c[0] | ((int) c[1] << 8);
	height = gifheight = (int) c[2] | ((int) c[3] << 8);
	gifleft = giftop = 0;
	gifsize = gifwidth * gifheight;
	gifext = gifoldtmp = NULL;
	//gifot = gifol = gifoh = gifow = 
	gifof = 0;
	gifback = c[5];
	printf("decode_GIF: background color index = %d\n", gifback);
	memset(dist, gifback, gifwidth*gifheight);
	if (c[4] & 0x80) {
		// グローバルカラーテーブル
		m = 2 << (c[4] & 7);
		c+=7;
		for (i=0; i<m; i++) {
			globalclut[i] = ((uint64) c[0]) | ((uint64) c[1] << 8) | ((uint64) c[2] << 16) | 0x80000000;
			//printf("clut[%3d]=#%06x\n", i, activeclut[i]);
			c+=3;
		}
		printf("decode_GIF: loaded global clut\n");
	} else {
		c+=7;
	}
	
	while (c[0] != 0x3B) {
		if (c[0] == 0x2C) {	// イメージ記述子
			// 前の画像の処理
			if (gifoldtmp) {
				// 以前の場所を復元する必要があればする
				for (i = 0; i < gifheight; i++)
					memcpy(&dist[(i+giftop)*width+gifleft], &gifoldtmp[i*gifwidth], gifwidth);
				free(gifoldtmp);
				changes--;
				gifoldtmp = NULL;
			} else if (gifof == 2) {
				// 背景塗りつぶしで代用
				for (i = 0; i < gifheight; i++)
					memset(&dist[(i+giftop)*width+gifleft], gifback, gifwidth);
				//	gifbuff[(giftop+gify)*width+gifleft+gifx] = c;
				changes--;
			}
			if (gifext)
				gifof = (gifext[1] >> 2) & 7;
			else
				gifof = 0;
			// イメージのデコード
			gifleft  = (int) c[1] | ((int) c[2] << 8);
			giftop   = (int) c[3] | ((int) c[4] << 8);
			gifwidth = (int) c[5] | ((int) c[6] << 8);
			gifheight= (int) c[7] | ((int) c[8] << 8);
			printf("decode_GIF: image offset = (%d,%d), size = (%d,%d)\n", gifleft, giftop, gifwidth, gifheight);
			// 必要に応じて表示前の画像を保存
			if (gifof == 3) {
				gifof = 2;
				gifoldtmp = (unsigned char*)malloc(gifwidth*gifheight);
				if (gifoldtmp != NULL) {
					for (i = 0; i < gifheight; i++)
						memcpy(&gifoldtmp[i*gifwidth], &dist[(i+giftop)*width+gifleft], gifwidth);
				}
			}
			interlaced = (c[9] & 0x40) != 0;
			if (interlaced)
				printf("decode_GIF: interlaced image\n");
			if (c[9] & 0x80) {
				activeclut = localclut;
				m = 2 << (c[9] & 7);
				c+=10;
				// ローカルカラーテーブル
				for (i=0; i<m; i++) {
					localclut[i] = ((uint64) c[0]) | ((uint64) c[1] << 8) | ((uint64) c[2] << 16) | 0x80000000;
					//printf("clut[%3d]=#%06lx\n", i, activeclut[i]);
					c+=3;
				}
				printf("decode_GIF: loaded local clut\n");
			} else {
				activeclut = globalclut;
				c+=10;
			}
			if (TCF) activeclut[TCI] &= 0x00FFFFFF;
			// テーブルベースのイメージデータ
			firstbits = *c++;
			printf("decode_GIF: LZW Minimum Code Size (first bits) = %d\n", firstbits);
			gifsetup(firstbits);
			siz=0; cnt=0; setpackage(c); prevcode = -1;
			// ポインタをイメージの後ろへ
			while(c[0] != 0) {
				siz+=c[0];
				c+=c[0] +1;
				cnt++;
			}
			// イメージのデコード
			gifbuff = (unsigned char*) dist;
			gifstatus = 0;
			gifx = 0; gify = 0;
			while( ((m=getcode())>=0) && (m != eoicode) && (!gifstatus) && (imgs == imgs)) {
				if (m == clearcode) {
					gifsetup(firstbits);
					if ((m=getcode())==eoicode) break;
					gifwrite(m, lzwtable[m].length);
					tableindex = eoicode+1;
				} else if (lzwtable[m].length > 0) {
					gifwrite(m, lzwtable[m].length);
					gifadd(prevcode, lzwtable[m].firstchar);
				} else {
					gifwrite(prevcode, lzwtable[prevcode].length);
					gifwrite(lzwtable[prevcode].firstchar, 1);
					gifadd(prevcode, lzwtable[prevcode].firstchar);
				}
				if ((tableindex >= (1 << codebits)) && (codebits < 12)) codebits++;
				prevcode=m;
			}
			//printf("decode_GIF: last code = %d (%d bits)\n", m, codebits);
			
			printf("decode_GIF: total package size = %d bytes (%d packages)\n", siz, cnt);
#ifdef DUMP_DECODE
			//デコード結果を表示できるようにヘッダをつけてPCにダンプ
			// 8bppのヘッダ
			static unsigned char header[54] = "BM____\x00\x00\x00\x00\x42\x00\x00\x00\x28\x00" "\x00\x00________\x01\x00\x08\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\xA0\x05\x00\x00\xA0\x05\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00";
			int fd;
			unsigned char filename[64], bmpclut[1024];
			sprintf(filename, "host:GIF%03d.BMP", imgs+1);
			fd = fioOpen(filename, O_WRONLY | O_CREAT);
			if (fd == NULL) {
				printf("decode_GIF: host open error!\n");
			} else {
				seti32(header+2, w*h+54+1024);
				seti32(header+10, 54+1024);
				seti32(header+18, w);
				seti32(header+22, -h);
				for (i=0;i<256;i++) {
					bmpclut[i*4+0] = (activeclut[i] >> 16) & 0xFF;
					bmpclut[i*4+1] = (activeclut[i] >>  8) & 0xFF;
					bmpclut[i*4+2] =  activeclut[i]        & 0xFF;
					bmpclut[i*4+3] = 0;
				}
				fioWrite(fd, header, 54);
				fioWrite(fd, bmpclut, 1024);
				fioWrite(fd, gifbuff, w*h);
				fioClose(fd);
			}//*/
#endif
			imgs++;
			changes++;
			c++;
			//break;
			if (gifext) gifext=NULL;
		} else if (c[0] == 0x21) {
			// 拡張
			printf("decode_GIF: Extension Label = 0x%02x, Block Size = %d bytes\n", (int) c[1], (int) c[2]);
			int type;
			unsigned char *d;
			unsigned char tmps[256];
			type = c[1]; d = c + 2;
			c+=3+c[2];
			while(c[0] != 0x00) c+=c[0]+1;
			c++;
			switch(type) {
				case 0x01:
					// プレーンテキスト拡張
					printf("decode_GIF: PlainText: textbox offset=(%d,%d), size=%dx%d, fontsize=%dx%d, fill=%d, back=%d\n", 
							256*d[2]+d[1], 256*d[4]+d[3], 256*d[6]+d[5], 256*d[8]+d[7], 
							(int) d[9], (int) d[10], (int) d[11], (int) d[12]);
					printf("decode_GIF: PlainText: fill=0x%08X, back=0x%08X\n", (int) globalclut[d[11]], (int) globalclut[d[12]]);
					d+= d[0]+1;
					printf("decode_GIF: PlainText: \x22");
					while(d[0] != 00) {
						memcpy(tmps, &d[1], 255);
						tmps[255] = 0;
						printf("%s", tmps);
						d+=d[0]+1;
					}
					printf("\x22\n");
					if (gifext) gifext=NULL;
					break;
				case 0xF9:
					// グラフィック制御拡張
					if (gifext) {
						printf("decode_GIF: warning: graphics extened block was already existed.\n");
					}
					gifext = d;
					//if (TCI) printf("decode_GIF: graphics extersion: transpent=%d, index=%d (0x%02X)\n", TCF, TCI, TCI);
					printf("decode_GIF: graphics: desp=%d, user=%d, trans=%d, delay=%d, index=%d (0x%02X)\n", (d[1] >> 2) & 7, (d[1] >> 1) & 1, TCF, delaytime, TCI, TCI);
					//if (TCF && (imgs == 0) && (TCI != gifback)/* && (((d[1] >> 2) & 7) > 1) */) globalclut[gifback] &= 0x00FFFFFF;
					break;
				case 0xFE:
					// コメント拡張
					printf("decode_GIF: comment: \x22");
					while(d[0] != 00) {
						memcpy(tmps, &d[1], 255);
						tmps[255] = 0;
						printf("%s", tmps);
						d+=d[0]+1;
					}
					printf("\x22\n");
				case 0xFF:
					// アプリケーション拡張
					break;
			}
		} else {
			printf("decode_GIF: invaild ID #%02X\n", (int) c[0]);
			break;
		}
	}
	printf("decode_GIF: last byte: %02X\n", (int) c[0]);
	printf("decode_GIF: image stream: %d image(s)\n", imgs);
	if (gifoldtmp) free(gifoldtmp);
	free(lzwtable);
	return 0;
}

