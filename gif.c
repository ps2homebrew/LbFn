#include "launchelf.h"

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
static unsigned int bitptr=0, bitrem=0;
static int codebits=9;
static int bt[16] = {0,1,3,7,15,31,63,127,255,511,1023,2047,4095};
static int bits[16] = {2,4,8,16,32,64,128,256,512,1024,2048,4096};
static int clearcode, eoicode, firstbits, prevcode;
static uint64 globalclut[256];
static uint64 localclut[256];
extern uint64 *activeclut;

void setpackage(unsigned char *src)
{
	package = (unsigned char *) src;
	bitptr = 8;
	bitrem = (int) package[0] << 3;
	return;
}
int getcode(void)
{	// ・コードは下位のビット、低位のバイトから割り当て
	// ・コードのバット配置は通常
	//   ビット配置例:
	//   (低位) bbbaaaaa dcccccbb eeeedddd ggfffffe hhhhhggg(高位)
	//   (上位) 21043210 (下位)
	int p,b,u,r,i;
	
	if (package[0] == 0) return -1;
	p = bitptr >> 3; b = bitptr & 7;
	u = (b+codebits+7) >> 3; r = (bitrem +7) >> 3;
	//i = (( ((int) package[p+(u==0)]) | ((int) package[p+1] << 8) | ((int) package[p+2] << 16)) >> b) & bt[codebits];
	i = ((int) package[p+(r<1)]);
	if (u > 1) {
		i|= ((int) package[p+1+(r<2)] << 8);
		if (u > 2)
			i|= ((int) package[p+2+(r<3)] << 16);
	}
	
	bitptr+= codebits;
	bitrem-= codebits;
	if (bitrem <= 0) {
		bitptr -= 8*package[0];
		package += package[0]+1;
		bitrem += 8*package[0];
	}
	
	return (i >> b) & bt[codebits];;
}
void cleartable(int setbits)
{
	clearcode = bits[setbits-1];
	eoicode = clearcode+1;
	return;
}
////////////////////////////////
// GIF形式のデコード
int decode_GIF(char *dist, char *src, int size, int bpp)
{
	unsigned char *c=(unsigned char *) src;
	int siz;
	//unsigned char bgcolor;
	int i,m;
	int sw,sh;
	int l,t,w,h,interlaced;
	int cnt,imgs=0;
	c+=6;
	sw = (int) c[0] | ((int) c[1] << 8);
	sh = (int) c[2] | ((int) c[3] << 8);
	
	printf("decode_GIF: background color index = %d\n", (int) c[5]);
	memset(dist, c[5], sw*sh);
	if (c[4] & 0x80) {
		// グローバルカラーテーブル
		m = bits[(c[4] & 7)];
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
			l = (int) c[1] | ((int) c[2] << 8);
			t = (int) c[3] | ((int) c[4] << 8);
			w = (int) c[5] | ((int) c[6] << 8);
			h = (int) c[7] | ((int) c[8] << 8);
			interlaced = (c[9] & 0x40) != 0;
			if (c[9] & 0x80) {
				activeclut = localclut;
				m = bits[c[9] & 7];
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
			siz=0; cnt=0; setpackage(c); prevcode = -1;
			while(c[0] != 0) {
				siz+=c[0];
				c+=c[0] +1;
				cnt++;
			}
			while( ((m=getcode())>=0) && (m != eoicode) ) {}
			//printf("decode_GIF: last code = %d (%d bits)\n", m, codebits);
			printf("decode_GIF: total package size = %d bytes (%d packages)\n", siz, cnt);
			imgs++;
			c++;
			//break;
		} else if (c[0] == 0x21) {
			// 拡張
			printf("decode_GIF: Extension Label = 0x%02x, Block Size= %d bytes\n", (int) c[1], (int) c[2]);
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
	return 0;
}

