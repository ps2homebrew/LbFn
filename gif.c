#include "launchelf.h"
//#define	DUMP_DECODE
//#define	DEBUG

#ifdef DEBUG
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
#endif
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
static unsigned int width, height, gifx, gify, interlaced;
static unsigned char *gifbuff;
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
	gifbuff[(giftop+gify)*width+gifleft+gifx++] = c;
	if (gifx >= gifwidth) {
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
	unsigned char *c=(unsigned char *) src;
	int siz;
	//unsigned char bgcolor;
	int i,m;
	int cnt,imgs=0;
	c+=6;
	lzwtable = (LZWDATA*)malloc(sizeof(LZWDATA)*4096);
	if (lzwtable == NULL) return -2;
	gifwidth  = (int) c[0] | ((int) c[1] << 8);
	gifheight = (int) c[2] | ((int) c[3] << 8);
	gifleft = 0; giftop = 0;
	gifsize = gifwidth * gifheight;
	width = gifwidth; height = gifheight;
	printf("decode_GIF: background color index = %d\n", (int) c[5]);
	memset(dist, c[5], gifwidth*gifheight);
	if (c[4] & 0x80) {
		// グローバルカラーテーブル
		m = 2 << (c[4] & 7);
		c+=7;
		for (i=0; i<m; i++) {
			globalclut[i] = ((uint64) c[0]) | ((uint64) c[1] << 8) | ((uint64) c[2] << 16);
			//printf("clut[%3d]=#%06x\n", i, activeclut[i]);
			c+=3;
		}
		printf("decode_GIF: loaded global clut\n");
	} else {
		c+=7;
	}
	
	while (c[0] != 0x3B) {
		if (c[0] == 0x2C) {
			// イメージ記述子
			gifleft  = (int) c[1] | ((int) c[2] << 8);
			giftop   = (int) c[3] | ((int) c[4] << 8);
			gifwidth = (int) c[5] | ((int) c[6] << 8);
			gifheight= (int) c[7] | ((int) c[8] << 8);
			printf("decode_GIF: image offset = (%d,%d), size = (%d,%d)\n", gifleft, giftop, gifwidth, gifheight);
			interlaced = (c[9] & 0x40) != 0;
			if (interlaced)
				printf("decode_GIF: interlaced image\n");
			if (c[9] & 0x80) {
				activeclut = localclut;
				m = 2 << (c[9] & 7);
				c+=10;
				// ローカルカラーテーブル
				for (i=0; i<m; i++) {
					localclut[i] = ((uint64) c[0]) | ((uint64) c[1] << 8) | ((uint64) c[2] << 16);
					//printf("clut[%3d]=#%06lx\n", i, activeclut[i]);
					c+=3;
				}
				printf("decode_GIF: loaded local clut\n");
			} else {
				activeclut = globalclut;
				c+=10;
			}
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
			c++;
			//break;
		} else if (c[0] == 0x21) {
			// 拡張
			printf("decode_GIF: Extension Label = 0x%02x, Block Size = %d bytes\n", (int) c[1], (int) c[2]);
			if (c[1] == 0xF9) {	
				// グラフィック制御拡張
			} else if (c[1] == 0xFE) {
				// コメント拡張
			} else if (c[1] == 0xFF) {
				// アプリケーション拡張
			}
			c+=3+c[2];
			while(c[0] != 0x00) c+=c[0]+1;
			c++;
		} else {
			printf("decode_GIF: invaild ID #%02X\n", (int) c[0]);
			break;
		}
	}
	printf("decode_GIF: last byte: %02X\n", (int) c[0]);
	printf("decode_GIF: image stream: %d image(s)\n", imgs);
	free(lzwtable);
	return 0;
}

