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
		printf("PNG: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("PNG: malloc vaild (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("PNG: free vaild (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("PNG: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
// -16:現在のアドレス, -12:サイズ, -8:前のアドレス, -4:次のアドレス
#define	malloc	X_malloc
#define free	X_free
//#endif
#ifdef DUMP_DECODE
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
#endif
////////////////////////////////
// PNG形式の詳細情報取得
//	info[0]	形式ID(固定値)
//	info[1]	色ビット数
//	info[2]	幅
//	info[3]	高さ
//	size:	バッファのサイズ
//	buff:	バッファ
//	ret	=0:	invaild format
//		!0:	OK
int decode_PNG(char *dist, char *src, int size, int bpp);
int info_PNG(int *info, char *buff, int size)
{
	unsigned char *c=(unsigned char *) buff;
	int bp, bsize, ww, bit, clr, width, height;

	// ファイルシグネチャ
	if ((c[0] != 0x89) || (c[1] != 0x50) || (c[2] != 0x4E) || (c[3] != 0x47) || (c[4] != 0x0D) || (c[5] != 0x0A) || (c[6] != 0x1A) || (c[7] != 0x0A))
		return 0;
	c+=8;
	// イメージヘッダチャンク
	if ((c[0] != 0x00) || (c[1] != 0x00) || (c[2] != 0x00) || (c[3] != 0x0d) || (c[4] != 0x49) || (c[5] != 0x48) || (c[6] != 0x44) || (c[7] != 0x52))
		return 0;
	c+=8;
	printf("info_PNG: colortype=%d ( %d bits ), compress=%02xh, filter=%02xh, interlace=%02xh\n", c[9], c[8], c[10], c[11], c[12]);
	info[0] = 0x000b;
	info[1] = c[8] * (((((c[9] & 2) == 2) * 3) + ((c[9] & 4) == 4)) * ((c[9] & 1) == 0) + (c[9] & 1)) ;
	info[2] = width = (int) c[3] | ((int) c[2] << 8) | ((int) c[1] << 16) | ((int) c[0] << 24);
	info[3] = height= (int) c[7] | ((int) c[6] << 8) | ((int) c[5] << 16) | ((int) c[4] << 24);
	info[4] = bit = c[8];	// bit
	info[5] = clr = c[9];	// clr
	info[6] = c[12];	// interlace
	if (!info[1]) info[1] = 8;

	bp = ((bit * (((clr & 2) + 1 + ((clr & 4) == 4)) * !(clr & 1) + (clr & 1))) >> 0);
	//if (!bp) bp = 1;
	if (bp < 8) {
		bsize = 8 / bp -1;
		ww = (width + bsize) & ~bsize;
	} else ww = width;
	if (c[12])	bsize = ((ww * height * bp) >> 3) + ((height * 15 +7) >> 3);
	else		bsize = ((ww * height * bp) >> 3) + height;
	info[7] = bsize;
	return 1;
}
////////////////////////////////
static unsigned int width, height, bit, clr, bppp, idatsize;
static u8 cmf, flg, cmp, fil, interlaced;
static uint64 globalclut[256];
extern uint64 *activeclut;
void delay(int count);
////////////////////////////////
// 読み書き用
static char* pnginflate(void) {
	// deflate圧縮のデコード
	unsigned char *buff;
	int bp, bsize, ww;
	bp = ((bit * (((clr & 2) + 1 + ((clr & 4) == 4)) * !(clr & 1) + (clr & 1))) >> 0);
	//if (!bp) bp = 1;
	if (bp < 8) {
		bsize = 8 / bp -1;
		ww = (width + bsize) & ~bsize;
	} else ww = width;
	if (interlaced)	bsize = ((ww * height * bp) >> 3) + ((height * 15 +7) >> 3);
	else			bsize = ((ww * height * bp) >> 3) + height;
	buff = (char*)malloc(bsize);//+258);
	if (!buff) return NULL;
	memset(buff, 0, bsize);
	//	printf("pnginflate: width=%d=>%d, height=%d, bp=%d, interlace=%d, bsize=%d\n", width, ww, height, bp, interlaced, bsize);
	printf("pnginflate: CMF:%02x, FLG:%02x\n", cmf, flg);
	inflate_chunk_dst(buff, bsize);
	if (inflate_chunk_exe() < 0) {
		free(buff);
		buff = NULL;
	}
	return buff;
}
static int Paeth(int l,int u,int c) {
    int p,pa,pb,pc;    
        
    p = l + u - c; //予測値を計算
    //予測値に最も近い値を探す
    pa = p - l;
    pb = p - u;
    pc = p - c;
	if (pa < 0) pa *= -1;
	if (pb < 0) pb *= -1;
	if (pc < 0) pc *= -1;
    if(pa <= pb && pa <= pc) return l;
    else if(pb <= pc) return u;
    else return c;
}
//static void pngwrite(uint64 color);
////////////////////////////////
// PNG形式のデコード
int decode_PNG(char *dist, char *src, int size, int bpp0)
{
	unsigned char *c=(unsigned char *) src, *ca, *ds;
	unsigned int chunkid, chunksize, chunkcrc, crc, cb, i, xp, idats, x, y;
	int ret=0,bpp,imgs;//,btype,bfinal;
	c+=8; bppp = bpp = bpp0 & 15; imgs = bpp0 >> 8; if (imgs < 1) imgs = 1;
	//printf("PNG: bpp=%d\n", bpp);
	width = height = bit = clr = xp = idatsize = chunkid = idats = chunkcrc = crc = 0;
	inflate_chunk_clr();
	// チャンク調査
	while((int)c < (int)src + size) {
		chunksize = (int) c[3] | ((int) c[2] << 8) | ((int) c[1] << 16) | ((int) c[0] << 24);
		chunkid = (int) c[7] | ((int) c[6] << 8) | ((int) c[5] << 16) | ((int) c[4] << 24);
		ca = c + 4; cb = chunksize + 4;
	//	printf("PNG chunk info: [%c%c%c%c] (%08x) = %7d bytes, ", c[4], c[5], c[6], c[7], chunkid, chunksize);
	//	printf("PNG chunk info: [%c%c%c%c] (%08x) = %7d bytes\n", c[4], c[5], c[6], c[7], chunkid, chunksize);
		c += 8;
		switch(chunkid) {
			case 0x49484452:	// IHDR:イメージヘッダ (必須)
				width = (int) c[3] | ((int) c[2] << 8) | ((int) c[1] << 16) | ((int) c[0] << 24);
				height = (int) c[7] | ((int) c[6] << 8) | ((int) c[5] << 16) | ((int) c[4] << 24);
				printf("decode_PNG: width=%d, height=%d, bpp=%d, imgs=%d, dist=%08x, size=%d, end=%08x\n", width, height, bpp, imgs, (int)dist, width*height*bpp*imgs, ((int)dist) + width*height*bpp*imgs);
				memset(dist, 0, width * height * bpp * imgs); // メモリ初期化
				clr = c[9]; bit = c[8];
				cmp = c[10]; fil = c[11]; interlaced = c[12];
				if (clr == 3) {
					if (bit)
						xp = 8 / bit;
					else
						xp = 1;
				}
				if (clr == 2) xp = 1;
				if (clr == 0) {
					if (bit >= 8) {
						for (i=0; i<256; i++) {
							globalclut[i] = i * 0x010101;
						}
					} else {
						u32 pp;
						if (bit < 2)		pp = 0xFFFFFF;
						else if (bit < 4)	pp = 0x555555;
						else if (bit < 8)	pp = 0x111111;
						else				pp = 0x010101;
						for (i=0; i<1<<bit; i++) {
							globalclut[i] = i * pp;
						}
					}
					activeclut = globalclut;
				}
				break;
			case 0x504c5445:	// PLTE:パレット 
				for (i=0; i*3<chunksize; i++) {
					if (i > 255) break;
					globalclut[i] = c[i*3+0] | ((int) c[i*3+1] << 8) | ((int) c[i*3+2] << 16);
				}
				activeclut = globalclut;
				break;
			case 0x49444154:	// IDAT:イメージデータ (必須,複数設置可)
				if (!idats) {
					cmf = c[0]; flg = c[1];
					inflate_chunk_add(&c[2], chunksize-2);
				} else {
					if (inflate_chunk_add(c, chunksize) < 0) ret = -2;
				}
				idats++; 
				idatsize+=chunksize;
				break;
			case 0x49454e44:	// IEND:終端
				break;
		}
		c += chunksize;
	//	chunkcrc = (int) c[3] | ((int) c[2] << 8) | ((int) c[1] << 16) | ((int) c[0] << 24);
		c += 4;
	//	crc = CRC32Check(ca, cb);
	//	printf("CRC = %08x ( %08x )\n", crc, chunkcrc);
		if (chunkid == 0x49454e44) break;
	}
	if (chunkid != 0x49454e44) {
		printf("PNG: error: IEND chunk is not found.\n");
	}
	printf("PNG: image data %d chunks, %d bytes\n", idats, idatsize);
	
	// deflate圧縮のデコード
	if (ret == 0) {
		ds = pnginflate();
	} else ds = NULL;
	inflate_chunk_clr();
	if (ds == NULL) {
		ret = -2;
	} else {
		// イメージの復元
		int pngsubimage(u8 *dist, u8 *ds, int width, int height, int ww, int hh, int sx, int sy, int sl, int st) {
			int l,u,lu,bp,pb,pp,f,p,wwwww;
			//c[8] * (((((c[9] & 2) == 2) * 3) + ((c[9] & 4) == 4)) * ((c[9] & 1) == 0) + (c[9] & 1))
			pb = bp = ((bit * (((clr & 2) + 1 + ((clr & 4) == 4)) * !(clr & 1) + (clr & 1))) >> 3);
			if (bp) wwwww = bp * width; else {
				wwwww = 0;
				switch(bit) {
					case 1:
						wwwww = ( width + 7 ) >> 3;
						break;
					case 2:
						wwwww = ( width + 3 ) >> 2;
						break;
					case 4:
						wwwww = ( width + 1 ) >> 1;
						break;
				}
				bp = 1; 
			};
			//	フィルターの解除
			for (y=0,p=0; y<height; y++) {
				f = ds[p++]; pp=p;
			//	printf("pngdraw: filter[%4d]=%02x\n", y, f);
				for (x=0; x<wwwww; x++) {
					if (x >= bp) l = ds[pp-bp]; else l = 0;
					if (y > 0) u = ds[pp-wwwww-1]; else u = 0;
					if ((x >= bp) && y) lu = ds[pp-wwwww-1-bp]; else lu = 0;
					switch(f) {
						case 0:	// そのまま
							break;
						case 1:	// 左との差(現在-左)=(現在=左+値)
							ds[pp++] += l;
							break;
						case 2: // 上
							ds[pp++] += u;
							break;
						case 3: // 左と上の平均との差(値=現在-(左+上)/2)
							ds[pp++] += (l+u)>>1;
							break;
						case 4: // 左/上/左上
							ds[pp++] += Paeth(l,u,lu);
							break;
					}
				}
				p += wwwww;
			//	itoVSync();
			}
			// ビットマップデータ転送
			u32 bppt,yy,xx;
			u8 temp[width * bpp];
			printf("decode_PNG: src=%dbpp, dist=%dbpp, clr=%d, bit=%d, width=%d, wwwww=%d, bp=%d\n", pb*8, bpp*8, clr, bit, width, wwwww, bp);
			for(yy=0;yy<height;yy++){
				y = yy * sy + st;
				switch(bpp) {
					case 4:	// PNG:8(for Gray)-64(for Color)bpp対応 (bit==16は動作未検証)
						switch(bp) {
							case 8:	// 48bpp+α(16bpp*4)
								for(x=0;x<width;x++){
									temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+3];
									temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+5];
									temp[x*bpp+3] = (ds[yy*(wwwww+1)+x*bp+7] >> 1) + (ds[yy*(wwwww+1)+x*bp+7] >> 7);
								}
								break;
							case 6: // 48bpp(16bpp*3)
								for(x=0;x<width;x++){
									temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+3];
									temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+5];
									temp[x*bpp+3] = 0;
								}
								break;
							case 4:	// 24bpp+α(8bpp*4/16bpp*2)
								if (bit == 8) {
									for(x=0;x<width;x++){
										temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+2];
										temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+3];
										temp[x*bpp+3] = (ds[yy*(wwwww+1)+x*bp+4] >> 1) + (ds[yy*(wwwww+1)+x*bp+4] >> 7);
									}
								} else {
									for(x=0;x<width;x++){
										temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+3] = (ds[yy*(wwwww+1)+x*bp+3] >> 1) + (ds[yy*(wwwww+1)+x*bp+3] >> 7);
									}
								}
								break;
							case 3:	// 24bpp(8bpp*3)
								for(x=0;x<width;x++){
									temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+2];
									temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+3];
									temp[x*bpp+3] = 0;
								}
								break;
							case 2:	// 16bpp(16bpp*1/8bpp*2)
								if (bit == 8) {
									for(x=0;x<width;x++){
										temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+3] = (ds[yy*(wwwww+1)+x*bp+2] >> 1) + (ds[yy*(wwwww+1)+x*bp+2] >> 7);
									}
								} else {
									for(x=0;x<width;x++){
										temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+1];
										temp[x*bpp+3] = 0;
									}
								}
								break;
							case 1:	// 8bpp(8bpp*1)
								for(x=0;x<width;x++){
									temp[x*bpp+0] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+1] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+2] = ds[yy*(wwwww+1)+x*bp+1];
									temp[x*bpp+3] = 0;
								}
								break;
						}
						break;
					case 2:	// PNG:24/32bppのみ
						switch(bp) {
							case 4:	// 32bpp(8bpp*4)
								for(x=0;x<width;x++){
									bppt = ((ds[yy*(wwwww+1)+x*bp+1] >> 3) & 0x001F)
									| (((u32)ds[yy*(wwwww+1)+x*bp+2] << 2) & 0x03E0)
									| (((u32)ds[yy*(wwwww+1)+x*bp+3] << 7) & 0x7C00);
									if (bp==4) bppt |= 0x8000 * (ds[yy*(wwwww+1)+x*bp+4] >> 7);
									temp[x*bpp+0] = bppt &255;
									temp[x*bpp+1] = bppt >> 8;
								}
								break;
							case 3:	// 24bpp(8bpp*3)
								for(x=0;x<width;x++){
									bppt = ((ds[yy*(wwwww+1)+x*bp+1] >> 3) & 0x001F)
									| (((u32)ds[yy*(wwwww+1)+x*bp+2] << 2) & 0x03E0)
									| (((u32)ds[yy*(wwwww+1)+x*bp+3] << 7) & 0x7C00);
									temp[x*bpp+0] = bppt &255;
									temp[x*bpp+1] = bppt >> 8;
								}
								break;
						}
						break;
					case 1:	// PNG:パレット(1-8bpp)のみ
						switch(bit) {
							case 8:
								for(x=0;x<width;x++){
									temp[x] = ds[yy*(wwwww+1)+x*bp+1];
								}
								break;
							case 4:
								for(x=0;x<width>>1;x++){
									temp[x*2+0] = ds[yy*(wwwww+1)+x+1] >> 4;
									temp[x*2+1] = ds[yy*(wwwww+1)+x+1] & 15;
								}
								if (width & 1)	temp[x*2] = ds[yy*(wwwww+1)+x+1] >> 4;
								break;
							case 2:
								for(x=0;x<width>>2;x++){
									temp[x*4+0] = (ds[yy*(wwwww+1)+x+1] >> 6) & 3;
									temp[x*4+1] = (ds[yy*(wwwww+1)+x+1] >> 4) & 3;
									temp[x*4+2] = (ds[yy*(wwwww+1)+x+1] >> 2) & 3;
									temp[x*4+3] = (ds[yy*(wwwww+1)+x+1] >> 0) & 3;
								}
								if ((width & 3) > 0)	temp[x*4+0] = (ds[yy*(wwwww+1)+x+1] >> 6) & 3;
								if ((width & 3) > 1)	temp[x*4+1] = (ds[yy*(wwwww+1)+x+1] >> 4) & 3;
								if ((width & 3) > 2)	temp[x*4+2] = (ds[yy*(wwwww+1)+x+1] >> 2) & 3;
								break;
							case 1:
								for(x=0;x<width>>3;x++){
									temp[x*8+0] = (ds[yy*(wwwww+1)+x+1] >> 7) & 1;
									temp[x*8+1] = (ds[yy*(wwwww+1)+x+1] >> 6) & 1;
									temp[x*8+2] = (ds[yy*(wwwww+1)+x+1] >> 5) & 1;
									temp[x*8+3] = (ds[yy*(wwwww+1)+x+1] >> 4) & 1;
									temp[x*8+4] = (ds[yy*(wwwww+1)+x+1] >> 3) & 1;
									temp[x*8+5] = (ds[yy*(wwwww+1)+x+1] >> 2) & 1;
									temp[x*8+6] = (ds[yy*(wwwww+1)+x+1] >> 1) & 1;
									temp[x*8+7] = (ds[yy*(wwwww+1)+x+1] >> 0) & 1;
								}
								if ((width & 7) > 0)	temp[x*8+0] = (ds[yy*(wwwww+1)+x+1] >> 7) & 1;
								if ((width & 7) > 1)	temp[x*8+1] = (ds[yy*(wwwww+1)+x+1] >> 6) & 1;
								if ((width & 7) > 2)	temp[x*8+2] = (ds[yy*(wwwww+1)+x+1] >> 5) & 1;
								if ((width & 7) > 3)	temp[x*8+3] = (ds[yy*(wwwww+1)+x+1] >> 4) & 1;
								if ((width & 7) > 4)	temp[x*8+4] = (ds[yy*(wwwww+1)+x+1] >> 3) & 1;
								if ((width & 7) > 5)	temp[x*8+5] = (ds[yy*(wwwww+1)+x+1] >> 2) & 1;
								if ((width & 7) > 6)	temp[x*8+6] = (ds[yy*(wwwww+1)+x+1] >> 1) & 1;
								break;
						}
						break;
				}
				if (sx > 1) {
					for(xx=0;xx<width;xx++){
						for(x=xx*sx+sl;x<xx*sx+sx;x++){
							if (x < ww) memcpy(&dist[(y*ww+x)*bpp], &temp[xx*bpp], bpp);
						}
					}
				} else	memcpy(&dist[y*ww*bpp], temp, width*bpp);
				for(y=yy*sy+st+1;y<yy*sy+sy;y++)
					if (y < hh)	memcpy(&dist[y*ww*bpp], &dist[(y-1)*ww*bpp], ww*bpp);
			}
			return p;
		}
	/*//	インタレースモードの場合
		area	(left,top)-step(x,y)
		pass	1	2	3	4	5	6	7
		left	0	4	0	2	0	1	0
		top		0	0	4	0	2	0	1
		x		8	8	4	4	2	2	1
		y		8	8	8	4	4	2	2
		
			pass	width	height	total	size	scale
				1	 80		 60		 14460	 80x 60		8x8
				2	 80		 60		 14460	160x 60		4x8
				3	160		 60		 28860	160x120		4x4
				4	160		120		 57720	320x120		2x4
				5	320		120		115320	320x240		2x2
				6	320		240		230640	640x240		1x2
				7	640		240		461040	640x480		1x1
			------------------------------
			total	1760	900		922500
	//*/
		if (interlaced) {
			int p=0,dsiz=width*height*bpp,f=0;
			void nb(void) {if (f < imgs-1) {memcpy(&dist[dsiz * (f+1)], &dist[dsiz * f], dsiz); f++;}}
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -0 +7) >> 3, (height -0 +7) >> 3, width, height, 8, 8, 0, 0); nb(); //			memcpy(&dist[dsiz * 1], &dist[dsiz * 0], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -4 +7) >> 3, (height -0 +7) >> 3, width, height, 8, 8, 4, 0); nb(); //			memcpy(&dist[dsiz * 2], &dist[dsiz * 1], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -0 +3) >> 2, (height -4 +7) >> 3, width, height, 4, 8, 0, 4); nb(); //			memcpy(&dist[dsiz * 3], &dist[dsiz * 2], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -2 +3) >> 2, (height -0 +3) >> 2, width, height, 4, 4, 2, 0); nb(); //			memcpy(&dist[dsiz * 4], &dist[dsiz * 3], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -0 +1) >> 1, (height -2 +3) >> 2, width, height, 2, 4, 0, 2); nb(); //			memcpy(&dist[dsiz * 5], &dist[dsiz * 4], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -1 +1) >> 1, (height -0 +1) >> 1, width, height, 2, 2, 1, 0); nb(); //			memcpy(&dist[dsiz * 6], &dist[dsiz * 5], dsiz);
			p+=pngsubimage(&dist[dsiz * f], &ds[p], (width -0 +0) >> 0, (height -1 +1) >> 1, width, height, 1, 2, 0, 1);
		} else pngsubimage(dist, ds, width, height, width, height, 1, 1, 0, 0);
		free(ds);
	}
#ifdef DUMP_DECODE
				//デコード結果を表示できるようにヘッダをつけてPCにダンプ
				// 8bppのヘッダ
				static unsigned char header[54] = "BM____\x00\x00\x00\x00\x42\x00\x00\x00\x28\x00" "\x00\x00________\x01\x00\x08\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\xA0\x05\x00\x00\xA0\x05\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00";
				int fd;
				unsigned char filename[64], bmpclut[1024];
				strcpy(filename, "host:PNG_DUMP.BMP");
				fd = fioOpen(filename, O_WRONLY | O_CREAT | O_TRUNC);
				if (fd < 0) {
					printf("decode_PNG: host open error!\n");
				} else {
					seti32(header+2, width*height*bpp+54+1024*(bpp<2));
					seti32(header+10, 54+1024*(bpp<2));
					seti32(header+18, width);
					seti32(header+22, -height);
					seti16(header+28, bpp*8);
					if (bpp<2) {
						for (i=0;i<256;i++) {
							bmpclut[i*4+0] = (activeclut[i] >> 16) & 0xFF;
							bmpclut[i*4+1] = (activeclut[i] >>  8) & 0xFF;
							bmpclut[i*4+2] =  activeclut[i]        & 0xFF;
							bmpclut[i*4+3] = (activeclut[i] >> 24) & 0xff;
						}
					}
					fioWrite(fd, header, 54);
					if (bpp<2)	fioWrite(fd, bmpclut, 1024);
					fioWrite(fd, dist, width*height*bpp);
					fioClose(fd);
				}//*/
#endif
	return ret;
}
/*//
				for (y=0,p=0; y<height; y++) {
					if (p >= dsize) break;
					f = ds[p++];
					printf("PNG: filter[%4d]=%02x, offset=%06x, bp=%d, bpp=%d\n", y, f, p, bp, bpp);
					if (f == 0) {
						if (clr & 1) {
							// パレットインデックス
							for (x=0; x<width; x+=xp) {
								if (p >= dsize) break;
								d = ds[p++];
								if (bit == 8) {
									dist[y*width+x] = d;
								} 
								if (bit == 4) {
									dist[y*width+x+0] = (d >> 4) & 15;
									dist[y*width+x+1] = (d >> 0) & 15;
								}
								if (bit == 2) {
									dist[y*width+x+0] = (d >> 6) & 3;
									dist[y*width+x+1] = (d >> 4) & 3;
									dist[y*width+x+2] = (d >> 2) & 3;
									dist[y*width+x+3] = (d >> 0) & 3;
								}
								if (bit == 1) {
									dist[y*width+x+0] = (d >> 7) & 1;
									dist[y*width+x+1] = (d >> 6) & 1;
									dist[y*width+x+2] = (d >> 5) & 1;
									dist[y*width+x+3] = (d >> 4) & 1;
									dist[y*width+x+4] = (d >> 3) & 1;
									dist[y*width+x+5] = (d >> 2) & 1;
									dist[y*width+x+6] = (d >> 1) & 1;
									dist[y*width+x+7] = (d >> 0) & 1;
								}
							}
						} else if (clr & 2) {
							// ダイレクトカラー
							for (x=0; x<width; x++) {
								//d = ds[p++] | ((int)ds[p++] << 8) | ((int)ds[p++] << 16);
								dist[(y*width+x)*bpp+2] = ds[p++];
								dist[(y*width+x)*bpp+1] = ds[p++];
								dist[(y*width+x)*bpp+0] = ds[p++];
							}
						} else if (bit <= 8) {
							// グレースケール(256階調)
							
						} else {
							// グレースケール(65536階調)
						}
					} else if (f > 4) {
						p += wwwww;
					} else {
						for (x=0; x<wwwww; x++) {
							if (x < bp) l = 0;
							else l = ds[y*width*bpp+x+x/3-bpp];
							if (y == 0) u = 0;
							else u = ds[(y-1)*width*bpp+x+x/3];
							if ((x<bp)||!y) lu = 0;
							else lu = ds[(y-1)*width*bpp+x-bpp+x/3];
							switch(f) {
								case 1:
									// 左との差分
									ds[y*width*bpp+x] = l + ds[p++];
									break;
								case 2:
									// 上との差分
									ds[y*width*bpp+x] = u + ds[p++];
									break;
								case 3:
									// 左と上の平均との差分
									ds[y*width*bpp+x] = ((l + u) >> 1) + ds[p++];
									break;
								case 4:
									// 左上と左と上の値による差分
									ds[y*width*bpp+x] = Paeth(l, u, lu) + ds[p++];
									break;
							}
						}
					}
				}
//*/
