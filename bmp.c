#include "launchelf.h"

// 構造体定義(インディアン依存)
typedef struct {
	unsigned char  bfType[2];
	unsigned int   bfSize;
	unsigned short bfReserved[2];
	unsigned int   bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct {
	unsigned int   biSize;
	unsigned int   biWidth;
	signed   int   biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int   biCompression;
	unsigned int   biSizeImage;
	unsigned int   biXPelsPerMeter;
	unsigned int   biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} __attribute__((packed)) RGBQUAD;

typedef struct {
	unsigned int data[3];
} __attribute__((packed)) BITFIELD_RGB;

typedef struct {
	unsigned int   bcSize;
	unsigned short bcWidth;
	unsigned short bcHeight;
	unsigned short bcPlanes;
	unsigned short bcBitCount;
} __attribute__((packed)) BITMAPCOREINFO;

typedef struct {
	unsigned char rgbtBlue;
	unsigned char rgbtGreen;
	unsigned char rgbtRed;
} __attribute__((packed)) RGBTRIPLE;

enum {
	RLE8=1,
	RLE4=2,
	BITFIELDS=3,
};

static uint64 clut[256];
extern uint64 *activeclut;
static BITMAPFILEHEADER dibheader;
static BITMAPINFOHEADER bmpheader;
static int bitoffset=0;
static unsigned char *buffer=NULL;
static int shift[4], linebyte;
static BITFIELD_RGB mask;

////////////////////////////////
// BMP形式の詳細情報取得
//	info[0]	形式ID(固定値)
//	info[1]	色ビット数
//	info[2]	幅
//	info[3]	高さ
//	size:	バッファのサイズ
//	buff:	バッファ
//	ret	=0:	invaild format
//		!0:	OK
int info_BMP(int *info, char *buff, int size)
{
	unsigned char *c=(unsigned char *) buff;

	if ((c[0] != 0x42) || (c[1] != 0x4D) || (c[6] != 0) || (c[7] != 0) || (c[8] != 0) || (c[9] != 0))
		return 0;
	//printf("BITMAPFILEHEADER: %d\nBITMAPINFOHEADER: %d\n", sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
	memcpy(&dibheader, &buff[0], sizeof(BITMAPFILEHEADER));
	memcpy(&bmpheader, &buff[sizeof(BITMAPFILEHEADER)], sizeof(BITMAPINFOHEADER));
	if (bmpheader.biSize < 40) return 0;
	if (bmpheader.biPlanes != 1) return 0;
	info[0] = 0x0001;
	info[2] = bmpheader.biWidth;
	info[3] = bmpheader.biHeight;
	info[1] = bmpheader.biBitCount;
	//if (bmpheader.biCompression == BITFIELDS) info[1] = 32;
	if (info[3] < 0) info[3] = -info[3];
	return 1;
}

////////////////////////////////
// BMP形式のデコード
void dsets(int y)
{
	if (bmpheader.biHeight < 0) {
		bitoffset = y * linebyte;
	} else {
		bitoffset = (bmpheader.biHeight-1-y) * linebyte;
	}
	if (bmpheader.biBitCount < 8)
		bitoffset <<= 3;
}
uint64 dgets()
{
	unsigned int i,b;
	uint64 k=0,t;
	if (bmpheader.biBitCount < 8) {
		//printf("dgets: bitoffset %08X\n", bitoffset);
		k = ((uint64) buffer[bitoffset >> 3] << (bmpheader.biBitCount+(bitoffset & 7))) >> 8;
		k &= (1 << bmpheader.biBitCount)-1;
		bitoffset+= bmpheader.biBitCount;
	} else {
		for (i=0,b=0;i<bmpheader.biBitCount>>3;i++,b+=8)
			k |= (uint64) buffer[bitoffset++] << b;
		if (shift[3]) {
			t = 0;
			for (i=0; i<3; i++) {
				b = k & mask.data[i];
				if (shift[i] != (2-i)*8+7) {
					if (shift[i] > (2-i)*8+7)
						b >>= shift[i] - ((2-i)*8+7);
					else	
						b <<= ((2-i)*8+7) - shift[i];
				}
				t |= b;
			}
			k = t;
		}
	}
	return k;
}
void dputs(void *dist, int w, int h, int bpp, int x, int y, uint64 color)
{
	/* ※注意:イメージの色深度とデコード先バッファの色深度が違う場合は色が変わります */
	if ((x<0)||(y<0)||(x>=w)||(y>=h)) return;
	//if (dist == NULL) return;
	//printf("dputs: dist:%08X, size:(%4d,%4d), depth:%d, pos:(%4d,%4d), color:#%08X\r", (int) dist, w, h, bpp, x, y, color);
	if (bpp == 1) {
		unsigned char *dst = (unsigned char *) dist;
		dst[y*w+x] = color;
	} else if (bpp == 2) {
		unsigned short *dst = (unsigned short *) dist;
		if ((bmpheader.biBitCount > 16) || shift[3]) {
		//	dst[y*w+x]=(((color >> 3) & 0x001F) |
		//				((color >> 6) & 0x03E0) |
		//				((color >> 9) & 0x7C00) );
			dst[y*w+x]=(((color >>19) & 0x001F) |
						((color >> 6) & 0x03E0) |
						((color << 7) & 0x7C00) );
		} else {	
			dst[y*w+x] = color;
		}
	} else if (bpp == 3) {
		unsigned char *dst = (unsigned char *) dist;
		unsigned int i = (y*w+x)*3;
		dst[i++] = color & 0x0000FF;
		dst[i++] = (color & 0x00FF00) >> 8;
		dst[i++] = (color & 0xFF0000) >>16;
	} else if (bpp == 4) {
	//	unsigned int *dst = (unsigned int *) dist;
	//	dst[y*w+x] = color;
		unsigned char *dst = (unsigned char *) dist;
		unsigned int i = (y*w+x)*4;
		dst[i++] = (color & 0xFF0000) >>16;
		dst[i++] = (color & 0x00FF00) >> 8;
		dst[i++] = color & 0x0000FF;
		dst[i++] = (color >> 24) & 0xFF;
	}
}
int decode_BMP(char *dist, char *src, int size, int bpp)
{
	unsigned char *c=(unsigned char *) src;
	int i, b, k, x, y, height, yar;
	activeclut = clut;
	
	if ((bmpheader.biBitCount <= 8) && (bmpheader.biClrUsed == 0))
		bmpheader.biClrUsed = 1 << bmpheader.biBitCount;
	if (bmpheader.biBitCount > 8) bmpheader.biClrUsed = 0;
	if (dibheader.bfOffBits == 0)
		dibheader.bfOffBits = sizeof(BITMAPFILEHEADER)+bmpheader.biSize+bmpheader.biClrUsed*4;
	height = bmpheader.biHeight;
	if (height < 0) height = -height;
	linebyte = ((bmpheader.biWidth * bmpheader.biBitCount +31) & -32) >> 3;

	printf("decode_BMP: byte per line: %d bytes\n", linebyte);
	printf("decode_BMP: bmpheader.biHeight: %d\n", bmpheader.biHeight);
	
	c = src +sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);
	if (bmpheader.biBitCount <= 8) {
		printf("decode_BMP: loaded %dbpp clut (size: %d colors)\n", bmpheader.biBitCount, bmpheader.biClrUsed);
		for (i=0; i<bmpheader.biClrUsed; i++) {
			clut[i] = ((uint64) c[0] << 16) | ((uint64) c[1] << 8) | ((uint64) c[2]);
			c+=4;
		}
		shift[3] = 0;
	} else if (bmpheader.biCompression != BITFIELDS) {
		if (bmpheader.biBitCount <= 16) {
			mask.data[0] = 0x001F; shift[0] = 4;
			mask.data[1] = 0x03E0; shift[1] = 9;
			mask.data[2] = 0x7C00; shift[2] =14;
		} else {
			mask.data[0] = 0x0000FF; shift[0] = 7;
			mask.data[1] = 0x00FF00; shift[1] =15;
			mask.data[2] = 0xFF0000; shift[2] =23;
		}
		shift[3] = 0;
	} else {
		printf("decode_BMP: compression: BITFIELDS\n");
		memcpy(&mask, c, 12);
		for (i=0; i<3; i++) {
			printf("decode_BMP: mask[%d]=%08X, ", i, mask.data[i]);
			for (k=0,b=1; k<32; k++,b<<=1)
				if (mask.data[i] & b) shift[i]=k;
			printf("msb=%d\n", shift[i]);
		}
		c+=12;
		bmpheader.biCompression = 0;
		shift[3] = 1;
	}
	
	//if (c != src + dibheader.bfOffBits)
	//	printf("decode_BMP: warning: bfOffBits(%08x) != current offset(%08x)\n", dibheader.bfOffBits, (int) &c[0] - (int) &src[0]);
	c = src + dibheader.bfOffBits;
	buffer = c;
	if (bmpheader.biCompression == 0) {
		printf("decode_BMP: compression: none\n");
		for (y=0;y<height;y++) {
			dsets(y);
			for (x=0;x<bmpheader.biWidth;x++) {
				dputs(dist, bmpheader.biWidth, height, bpp, x, y, dgets());
			}
		}
	} else if ((bmpheader.biCompression == RLE4) || (bmpheader.biCompression == RLE8)) {
		printf("decode_BMP: compression: run length encoded\n");
		memset(dist, 0, bmpheader.biWidth*height*bpp);
		i=0; x=0; y=height-1; yar=-1;
		if (bmpheader.biHeight < 0)	{ y=0; yar=1; }
		while(1) {
			b = c[i++];
			//printf(" %02X", b);
			if (b == 0) {
				b = c[i++];
				//printf("_%02X", b);
				if (b == 0) {
					//printf("\n%08X", i);
					y+=yar; x=0;
					//if (i & 3) i = (i + 3) & -4;
				} else if (b == 1) {
					break;
				} else if (b == 2) {
					x+=c[i++];
					y+=c[i++]*yar;
					if (x >= bmpheader.biWidth) {
						x-=bmpheader.biWidth;
						y+=yar;
					}
				} else {
					if (bmpheader.biBitCount == 4) {
						for(k=0; k<(b+1)/2; k++) {
							dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i] >> 4);
							dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i++] & 15);
						}
						if (b & 1) x--;
						if (((b-1) & 3) < 2) i++;
					} else {
						for(k=0; k<b; k++)
							dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i++]);
						if (b & 1) i++;
					}
				}
			} else {
				if (bmpheader.biBitCount == 4) {
					for(k=0; k<b; k+=2) {
						dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i] >> 4);
						dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i] & 15);
					}
					if (b & 1) x--;
				} else {
					for(k=0; k<b; k++)
						dputs(dist, bmpheader.biWidth, height, bpp, x++, y, c[i]);
				}
				++i;
			}
			if (x > bmpheader.biWidth) {
				printf("decode_BMP: warning: too long line in RLE decode\n");
				break;
			} 
			if ((y >= height) || (y < 0)) break;
		}
	}
	return 0;
}

