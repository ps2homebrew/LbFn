#include "launchelf.h"
/*
static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("ps2ico: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("ps2ico: malloc vaild (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("ps2ico: free vaild (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("ps2ico: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
#define	malloc	X_malloc
#define free	X_free
*/
////////////////////////////////
// PS2セーブデータアイコン形式のテクスチャの詳細情報取得
//	info[0]	形式ID(固定値)
//	info[1]	色ビット数
//	info[2]	幅
//	info[3]	高さ
//	size:	バッファのサイズ
//	buff:	バッファ
//	ret	=0:	invaild format
//		!0:	OK
int info_PS2ICO(int *info, char *buff, int size)
{
	unsigned char *c=(unsigned char *) buff;
	
	if ((c[0] != 0) || (c[1] != 0) || (c[2] != 1) || (c[3] != 0) || 
		(c[4] >= 16) || (c[5] != 0) || (c[6] != 0) || (c[7] != 0) || 
		(c[9] != 0) || (c[10] != 0) || (c[11] != 0)
	)
		return 0;
	if ((c[8] & 4) == 0)
		return 0;
	
	info[0] = 0x7009;
	info[1] = 16;
	info[2] = 128;
	info[3] = 128;
	return 1;
}

////////////////////////////////
// PS2セーブデータアイコン形式のテクスチャのデコード
int decode_PS2ICO(char *dist, char *src, int size, int bpp)
{
	unsigned int *i=(unsigned int *) src;
	short *s=NULL;
	//float *f=NULL;
	
	int k,m,n;
	unsigned int nks,nv,attr,ns,nkf,ofs;
	
	nks = i[1];
	attr = i[2];
	nv = i[4];
	ns = nkf = 0;
	ofs = 20;
	printf("ps2ico: nks=%d, nv=%d, attr=%x, ofs=%d ( 0x%08X )\n", nks, nv, attr, ofs, ofs);
	if ( 1) {
		ofs+= (2*4*nks+16)*nv;
		printf("ps2ico: offset after attr : %d ( 0x%08X )\n", ofs, ofs);
	}
	if (attr & 2) {
		i = (unsigned int *) &src[ofs];
		ns = i[4]; i+=5; ofs += 20;
		printf("ps2ico: ns=%d\n", ns);
		for (k=0; k<ns; k++) {
			nkf = i[1];
			i+= 2 + 2*nkf;
			ofs+= 2*4*nkf + 8;
			printf("ps2ico: anime[%2d]= %d, offset=%d ( 0x%08X )\n", k, nkf, ofs, ofs);
			if (ofs >= size) break;
		}
		printf("ps2ico: offset after anime: %d ( 0x%08X )\n", ofs, ofs);
	}
	if (attr & 4) {
		if (attr & 8) {
			s = (short *) &src[ofs+4];
			short *d=(short *)dist, ss;
			k = 16384;
			while(k > 0) {
				m = *s++;
				if (m > 0) {
					ss = *s++;
					for(n=0; n<m; n++) {
						*d++ = ss;
						k--;
					}
				} else if (m < 0) {
					for(n=0; n<-m; n++) {
						*d++ = *s++;
						k--;
					}
				} else {
					printf("ps2ico: tex decode error\n");
				}
			}
		} else {
			memcpy(dist, src +ofs, 32768);
		}
	}
	return 0;
}

////////////////////////////////
// PS1セーブデータ形式のアイコンの詳細情報取得
//	info[0]	形式ID(固定値)
//	info[1]	色ビット数
//	info[2]	幅
//	info[3]	高さ
//	size:	バッファのサイズ
//	buff:	バッファ
//	ret	=0:	invaild format
//		!0:	OK
int info_PS1ICO(int *info, char *buff, int size)
{
	unsigned char *c=(unsigned char *) buff;
	
	if ((c[0] != 0x53) || (c[1] != 0x43) || (c[2] < 0x11) || (c[2] > 0x1F) || (c[3] == 0) || (c[3] > 15))
		return 0;
	
	info[0] = 0x700A;
	info[1] = 4;
	info[2] = 16;
	info[3] = 16;
	info[4] = c[2] & 15;
	return 1;
}
////////////////////////////////
// PS1セーブデータ形式のアイコンのデコード
static uint64 iconclut[16];
extern uint64 *activeclut;
int decode_PS1ICO(char *dist, char *src, int size, int bpp)
{
	int i,x,y,z,w;
	short *s = (short*)src, r, g, b;
	for (i=0; i<16; i++) {
		r = (s[48+i] & 0x001F) << 3;
		g = (s[48+i] & 0x03E0) >> 2;
		b = (s[48+i] & 0x7C00) >> 7;
		r |= r >> 5; g |= g >> 5; b |= b >> 5;
		iconclut[i] = ((uint64) r) | ((uint64) g << 8) | ((uint64) b << 16);
	}
	activeclut = iconclut;
/*	w = 16 * (src[2] & 15);
	for (z=0,i=128; z<(src[2]&15); z++) {
		for (y=0; y<16; y++) {
			for (x=0; x<16; x+=2,i++) {
				dist[y*w+z*16+x+0] = src[i] & 15;
				dist[y*w+z*16+x+1] = (src[i] >> 4) & 15;
			}
		}
	}//*/
	for (z=0,i=128; z<(src[2]&15); z++) {
		for (y=0; y<16; y++) {
			for (x=0; x<16; x+=2,i++) {
				dist[y*16+z*256+x+0] = src[i] & 15;
				dist[y*16+z*256+x+1] = (src[i] >> 4) & 15;
			}
		}
	}
	return 0;
}
