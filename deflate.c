// ★
#include "launchelf.h"
//typedef unsigned char u8;
//typedef unsigned short u16;
//typedef unsigned int u32;
//#define DUMP_DECODE
enum {
	MAX_CHUNKS = 128,
};
typedef struct {
	u16 index;
	u16 bit;
	u32 dat;
} hufftreedata;
typedef struct {
	u32 start;
	u32 count;
} hufftreeinfo;
typedef struct {
	u8 *buff;
	u32 size;
} chunkindex;
static chunkindex chunkd[MAX_CHUNKS], *chunklist=0;	// データチャンクリスト
static u32 bitptr, bitrem, bitnum, bitreg;			// ビット位置、ビット数、チャンクリスト用番号、登録数
static hufftreedata clst[20], cdat[288], clen[32];	// カスタムハフマン符号圧縮用
static hufftreeinfo hlst[8], hdat[16], hlen[16];
static u32 hlit, hdist, hclen;
static u8 *buff=0, *idatchunk;			// 展開先バッファ、チャンクバッファポインタ
static u32 bsiz, btype, bfinal, wpos, totalcsize;	// (展開先)バッファサイズ、btype, bfinal (中断対応)
int inflate(u8 *dist, u32 limit, u8 *src, u32 size);// 全体指定用
int inflate_dist(u8 *dist, u32 limit);				// 逐一展開用(展開先指定、圧縮データ追加、初期化、後始末)
int inflate_part(u8 *src, u32 size);
int inflate_init(void);
int inflate_quit(void);
int inflate_chunk_add(u8 *src, u32 size);			// データチャンク追加用
void inflate_chunk_clr(void);						// データチャンクリスト初期化
void inflate_chunk_dst(u8 *dist, u32 limit);		// 展開先の指定と実行
int inflate_chunk_exe(void);						// デコード実行
int gz_check(u8 *src);
int gz_getsize(u8 *src, u32 size);
int gzfilesize(u8 *filename);
int gzdecode(u8 *dst, u32 limit, u8 *src, u32 size);
int gzencode(u8 *dst, u32 limit, u8 *src, u32 size);
static int getbits(u32 bits, u32 xswap);
static int getcode(void);
static int treemake(hufftreedata *s, hufftreeinfo *d, u32 maxbits, u32 maxindex);
static int getcode2(hufftreedata *s, hufftreeinfo *d, u32 maxbits, u32 maxindex);
static u8 clistindex[20] = {16,17,18, 0, 8, 7, 9, 6,10, 5,11, 4,12, 3,13, 2,14, 1,15};
static u32 lenlst[] = {  0, 11, 19, 35, 67,131};
unsigned int CRC32Check(unsigned char *buff, unsigned int size);

int gz_check(u8 *c) {
	if ((c[0] == 0x1F) && (c[1] == 0x8B) && (c[2] == 0x08) && (c[3] < 0x20))
		return 0;
	return -1;
}
/*//
2.3. メンバーフォーマット

各メンバーは、次に示す構造になっています。

    +---+---+---+---+---+---+---+---+---+---+
    |ID1|ID2|CM |FLG|     MTIME     |XFL|OS | (続く-->)
    +---+---+---+---+---+---+---+---+---+---+

(FLG.FEXTRA がセットされた場合)

	    +---+---+=================================+
	    | XLEN  | "拡張フィールド" の XLEN バイト | (続く-->)
	    +---+---+=================================+

(FLG.FNAME がセットされた場合)

	    +=========================================+
	    |..元ファイルの名前。ゼロで終端されます...| (続く-->)
	    +=========================================+

(FLG.FCOMMENT がセットされた場合)

	    +===================================+
	    |..ファイルのコメント。ゼロで終端...| (続く-->)
	    +===================================+

(FLG.FHCRC がセットされた場合)

	    +---+---+
	    | CRC16 | (続く-->)
	    +---+---+

    +=======================+
    |..圧縮されたブロック...| (続く-->)
    +=======================+

      0   1   2   3   4   5   6   7
    +---+---+---+---+---+---+---+---+
    |     CRC32     |     ISIZE     |
    +---+---+---+---+---+---+---+---+

//*/
int gz_getsize(u8 *c, u32 size) {
	u32 p,crc32s;
	int s=-1;
	if ((size >= 18) && (c[0] == 0x1F) && (c[1] == 0x8B) && (c[2] == 0x08) && (c[3] < 0x20)) {
		p = 10;
		if (c[3] & 0x04)	// FLG.FEXTRA
			p += c[p] + 256 * c[p+1] +2;
		if (c[3] & 0x08)	// FLG.FNAME
			while(c[p++] && (p < size));
		if (c[3] & 0x10)	// FLG.FCOMMENT
			while(c[p++] && (p < size));
		if (c[3] & 0x02)	// FLG.FHCRC
			p+=2;
		// c[p]==圧縮されたブロック
		crc32s = c[size-8] + 256*c[size-7] + 65536*c[size-6] + 16777216*c[size-5];
		//	crc32d = CRC32Check(&c[p], size-p-8);
		s = c[size-4] + 256*c[size-3] + 65536*c[size-2] + 16777216*c[size-1];
	//	printf("gz_getsize: uncompressed crc32=%08X\n", crc32s);
		printf("gz_getsize: decoded size: %d\n", s);
		if (s > 0x3FFFFFFF) {
			s = -1;
			printf("gz_getsize: decoded filesize was to bigger, wrong or broken file?\n");
		}
	}
	return s;
}
static int getbits(u32 bits, u32 xswap)
{	// ・コードは下位のビット、低位のバイトから割り当て
	// ・可変長符号のビット配置は逆、固定長符号と拡張符号は通常通り
	// ・開始位置はパッケージの最初のバイトの3ビット目(bfinal/btypeの次のビット)から
	//   ビット配置例:
	//         b76543210
	//   (低位) bbbaaaaa dcccccbb eeeedddd ggfffffe hhhhhggg(高位)
	//   (上位) 21043210 (下位)	:固定長符号 : 11001b => 25 ( 11001b )
	//	 (下位) 23401234 (上位) :可変長符号 : 11001b => 19 ( 10011b )
	// ・残りビット数が足りないときはマイナス値を返す
	int p,b,u,i;

	if (bitrem < bitptr+bits) {
		if (bitnum >= bitreg) return bitrem - bitptr - bits;
		else {
			int r;
			p = bitptr >> 3; r = 0; b = bitptr & 7;
			u = (b+bits+7) >> 3;
			if (p < chunklist[bitnum].size) i = ((int) idatchunk[p]); 
			else i = ((int) chunklist[bitnum+1].buff[r++]);
			if (u > 1) {
				if (p+1 < chunklist[bitnum].size) i |= ((int) idatchunk[p+1] << 8);
				else i |= ((int) chunklist[bitnum+1].buff[r++] << 8);
				if (u > 2) {
					if (p+2 < chunklist[bitnum].size) i |= ((int) idatchunk[p+2] << 16);
					else i |= ((int) chunklist[bitnum+1].buff[r++] << 16);
					if (u > 3) printf("PNG_getbits: code size overflow ( %d bytes )\n", u);
				}
			}
			
		}
	} else {
		p = bitptr >> 3; b = bitptr & 7;
		u = (b+bits+7) >> 3; 
		i = ((int) idatchunk[p]);
		if (u > 1) {
			i |= ((int) idatchunk[p+1] << 8);
			if (u > 2)
				i |= ((int) idatchunk[p+2] << 16);
			if (u > 3) printf("PNG_getbits: code size overflow ( %d bytes )\n", u);
		}
	}
	bitptr += bits;
	if ((bitptr >= bitrem) && (bitnum < bitreg)) {
		bitptr -= bitrem;
		idatchunk = chunklist[++bitnum].buff;
		bitrem = chunklist[bitnum].size << 3;
	}
	if (xswap && (bits > 1)) {
		u32 is,id;
		is = (i >> b) & ((1 << bits) -1);
		id = 0;
		for(i=0;i<bits;i++){
			id <<= 1;
			id |= is & 1;
			is >>= 1;
		}
		i = id; b = 0;
	}
	return (i >> b) & ((1 << bits) -1);
}
static int getcode(void)
{	// 可変長符号値の取得
	//	可変長符号:
	//	値(リテラル)	種別	ビット長	符号		見分け方(先頭5bit)
	//	  0〜143		通常		8		00110000-	00110( 6)-
	//										10111111	10111(23)
	//	144〜255		通常		9		110010000-	11001(25)-
	//										111111111	11111(31)
	//	256〜279		長さ		7		0000000-	00000( 0)-
	//										0010111		00101( 5)
	//	280〜287		長さ		8		11000000-	11000(24)
	//										11000111
	u32 i,k;
	// まずは5ビット読み込み
	i = getbits(5, 1);
	if (i < 0) return -1;	// 失敗
	if (i <= 5) {
		// 7bit/256-279/0000000b-0010111b
		i = (((i -  0) << 2) | (k = getbits(2, 1))) + 256;
	} else if (i <= 23) {
		// 8bit/0-143/00110000b-10111111b
		i = (((i -  6) << 3) | (k = getbits(3, 1))) +   0;
	} else if (i == 24) {
		// 8bit/280-287/11000000b-11000111b
		//i = (((i - 24) << 3) | getbits(3, 1)) + 280;
		i = (k = getbits(3, 1)) + 280;
	} else {
		// 9bit/144-255/110010000b-111111111b
		i = (((i - 25) << 4) | (k = getbits(4, 1))) + 144;
	}
	if (k < 0) {
		// 拡張ビットの読み込みに失敗したら既に読み込んだ分を戻す
		bitptr -= 5;
		i = -1;	// 失敗
	}
	return i;
}
static int treemake(hufftreedata *s, hufftreeinfo *d, u32 maxbits, u32 maxindex) {
	// 符号長リストからハフマン木とインデックスの作成
	u32 p,l,i,b,ret;
	//	u8 tmpss[8];	// 符号表チェック用
	hufftreedata tmp;
	for(p=1,l=0,b=0,ret=0;p<=maxbits;p++,l<<=1){
		//sprintf(tmpss, "%%0%db\n", p);
		for(i=0;i<=maxindex;i++){
			if (s[i].bit == p) {
				s[i].dat = l++;
			//	printf("pnginflate: [%3d]=%3d bits, ", i, p);
			//	printf(tmpss, ptn[i]);
				ret = p;
			}
		}
	}
	// ビット数順にソート
	for(i=0,b=0;i<maxindex;i++){
		for(l=i+1;l<=maxindex;l++){
			if ((s[l].bit > 0) && ((s[i].bit == 0) || (s[i].bit > s[l].bit))) {
				tmp = s[i];
				s[i] = s[l];
				s[l] = tmp;
			}
		}
	}
	if (d) {
		// ビットごとのインデックスを作成
		for(i=0;i<=maxbits;i++) d[i].start = d[i].count = 0;
		for(i=0,p=0,b=0,l=0;i<=maxindex;i++){
			if ((s[i].bit > p) || (s[i].bit == 0)) {
				d[p].start = l;
				d[p].count = b;	b = 0;
				if ((s[i].bit > maxbits) || (s[i].bit == 0)) break;
				p = s[i].bit;
				l = i;
			}
			b++;
		}
		if (b) {
			d[p].start = l;
			d[p].count = b;
		}
		//delay(3);
	//	for(i=1;i<=maxbits;i++){
	//		printf("pnginflate: %2dbit: start=%3d, count=%3d\n", i, d[i].start, d[i].count);
	//	}
	}
	return ret;
}
static int getcode2(hufftreedata *s, hufftreeinfo *d, u32 maxbits, u32 maxindex) {
	// カスタムハフマン符号値の取得
	u32 p,b,i;
	u32 oldptr=bitptr,oldrem=bitrem;
	if (d) {
		for(b=1,p=0;b<=maxbits;b++,p<<=1){
			if (bitrem <= bitptr) break;
			p |= getbits(1, 0);
			for(i=d[b].start;i<d[b].start+d[b].count;i++){
				if ((s[i].bit == b) && (s[i].dat == p)) {
					return s[i].index;
				}
			}
		}
	} else {
		for(b=1,p=0;b<=maxbits;b++,p<<=1){
			if (bitptr >= bitrem) break;
			p |= getbits(1, 0);
			for(i=0;i<=maxindex;i++){
				if ((s[i].bit == b) && (s[i].dat == p)) {
					return s[i].index;
				}
			}
		}
	}
	bitptr = oldptr; bitrem = oldrem;
	return -maxbits;
}
static int inflate_main(void) {
	// deflate圧縮のデコード(共通部)
	u32 i,bt,l,p,maxa,maxb,a,b;
#ifdef	DUMP_DECODE
	{
				int	fd;
				fd = fioOpen("host:dumpsrc.bin", O_WRONLY | O_CREAT);
				if (fd < 0) {
					printf("decode_PNG: host open error!\n");
				} else {
					fioWrite(fd, buff, bsiz);
					fioClose(fd);
				}
	}
#endif
	memset(buff, 0, bsiz);
	do {
		bfinal = getbits(1, 0);
		btype = getbits(2, 0);
//		printf("inflate: rpos=%8d/%8d, wpos=%8d/%8d, bfinal=%d, btype=%d\n", bitptr >> 3, totalcsize, wpos, bsiz, bfinal, btype);
//		itoVSync();
//		itoVSync();
		switch(btype) {
			case 0:	/* 無圧縮 */
				//	| BFINAL/BTYPE | LEN x2 | NLEN x2 | DATA xN |
				bitptr = (bitptr + 7) & ~7;
				l = getbits(16, 0); bt = getbits(16, 0);
				if ((bt < 0) || (l < 0)) {bfinal = 2; break;}
			//	printf("inflate: btype=0, len=%04x, nlen=%04x, xor=%04x, add=%04x, !=%d, wpos=%8d, bsiz=%8d\n", l, bt, l ^ bt, l + bt, (l ^ bt) != 0xFFFF, wpos, bsiz);
				if ((l ^ bt) != 0xFFFF) {bfinal = 2; break;}
				if (l + wpos > bsiz) {bfinal = 2; break;}
				p = bitptr >> 3;
				if (p + l > chunklist[bitnum].size) {
					bt = chunklist[bitnum].size - p;
					bitptr += bt << 3; l -= bt;
					memcpy(buff +wpos, idatchunk +p, bt);
					wpos += bt;
					for (i=0; i<l; i++) {
						buff[wpos++] = getbits(8, 0);
				//		if (wpos >= bsiz) {bfinal = 2; break;}
					}
				} else {
					memcpy(buff +wpos, idatchunk +p, l);
					bitptr += l << 3;
					wpos += l;
				}
				break;
			case 1: /* 固定ハフマン符号圧縮(LZ77) */
				while(wpos < bsiz) {
					i = getcode();
					if (i < 0) break;
					if (i < 256) {
						// そのまま
						buff[wpos++] = i;
					} else if (i == 256) {
						// 終端符号
						break;
					} else {
						//	長さ符号:可変長符号(>256)+拡張符号
						//		 拡張				 拡張				 拡張			
						//	値	ビット	長さ	値	ビット	長さ	値	ビット	長さ	
						//	257		0	    3	267		1	15-16	277		4	 67- 82	
						//	258		0	    4	268		1	17-18	278		4	 83- 98	
						//	259		0	    5	269		2	19-22	279		4	 99-114	
						//	260		0	    6	270		2	23-26	280		4	115-130	
						//	261		0	    7	271		2	27-30	281		5	131-162	
						//	262		0	    8	272		2	31-34	282		5	163-194	
						//	263		0	    9	273		3	35-42	283		5	195-226	
						//	264		0	   10	274		3	43-50	284		5	227-257	
						//	265		1	11-12	275		3	51-58	285		0	    258	
						//	266		1	13-14	276		3	59-66						
						if (i < 265)		l = i -257 +3;
						else if (i >= 285)	l = 258;
						else {
							//	拡張ビット数 = (x - 261) >> 2;
							//	長さ = lenlst[拡張ビット数-1] + ((x-1) & 3) * (1 << 拡張ビット数) + 拡張ビット
							bt = (i -261) >> 2;
							l = lenlst[bt] + ((i -1) & 3) * (1 << bt) + getbits(bt, 0);
						}
						
						//	距離符号:5ビット+拡張符号
						//		 拡張				 拡張					 拡張				
						//	値	ビット	距離	値	ビット	距離		値	ビット	距離		
						//	0		0	    1	10		4	 33-  48	20		 9	 1025- 1536	
						//	1		0	    2	11		4	 49-  64	21		 9	 1537- 2048	
						//	2		0	    3	12		5	 65-  96	22		10	 2049- 3072	
						//	3		0	    4	13		5	 97- 128	23		10	 3073- 4096	
						//	4		1	 5- 6	14		6	129- 192	24		11	 4097- 6144	
						//	5		1	 7- 8	15		6	193- 256	25		11	 6145- 8192	
						//	6		2	 9-12	16		7	257- 384	26		12	 8193-12288	
						//	7		2	13-16	17		7	385- 512	27		12	12289-16384	
						//	8		3	17-24	18		8	513- 768	28		13	16385-24576	
						//	9		3	25-32	19		8	769-1024	29		13	24577-32768	
						i = getbits(5, 1);
						if (i < 0) break;
						if (i < 4) p = i +1;
						else	p = (1 << (i >> 1)) +1 + getbits((i >> 1) -1, 0) + ((i & 1) << ((i >> 1) -1));
						if (wpos+l <= bsiz)	for(i=0;i<l;i++)	buff[wpos+i] = buff[wpos-p+i];
						else i = -1;
						wpos+=l;
					}
				}
				if (i < 0) bfinal = 2;
				break;
			case 2:	/* カスタムハフマン符号圧縮(LZ77+ハフマン木) */
				//printf("pnginflate: limit=%d, btype/bfinal byte=0x%02X, firstcode=%d\n", limit, c[0], getcode());
				//	HLIT	5bit
				//	HDIST	5bit
				//	HCLEN	4bit
				//	符号長リストの圧縮符号のリスト
				//	リテラル・長さの符号長リスト
				//	距離の符号長リスト
				//	カスタムハフマン符号列
				hlit = getbits(5, 0)+257;
				hdist = getbits(5, 0) + 1;
				hclen = getbits(4, 0) + 4;
				memset(clst, 0, sizeof(clst));
				memset(cdat, 0, sizeof(cdat));
				memset(clen, 0, sizeof(clen));
				for(i=0;i<288;i++){
					if (i < 20) clst[i].index = i;
					if (i < 32) clen[i].index = i;
					cdat[i].index = i;
				}
				for(i=0;i<hclen;i++)	clst[clistindex[i]].bit = getbits(3, 0);
				treemake(clst, hlst, 7, 19);
				for(i=0;i<hlit;i++){
					a = getcode2(clst, hlst, 7, 19);
					if (a < 16) {
						cdat[i].bit = a;
					} else if (a == 16) {
						b = getbits(2, 0)+3;
						for(p=0;p<b;p++)	cdat[i+p].bit = cdat[i-1].bit;
						i+=b-1;
					} else {
						if (a==17)	b = getbits(3, 0)+3;
						else		b = getbits(7, 0)+11;
						for(p=0;p<b;p++)	cdat[i+p].bit = 0;
						i+=b-1;
					}
				}
				maxa = treemake(cdat, hdat, 15, hlit-1);
				for(i=0;i<hdist;i++){
					a = getcode2(clst, hlst, 7, 19);
					if (a < 16) {
						clen[i].bit = a;
					} else if (a == 16) {
						b = getbits(2, 0)+3;
						for(p=0;p<b;p++)	clen[i+p].bit = clen[i-1].bit;
						i+=b-1;
					} else {
						if (a==17)	b = getbits(3, 0)+3;
						else		b = getbits(7, 0)+11;
						for(p=0;p<b;p++)	clen[i+p].bit = 0;
						i+=b-1;
					}
				}
				//delay(1);
				maxb = treemake(clen, hlen, 15, hdist-1);
				while(wpos < bsiz) {
					i = getcode2(cdat, hdat, maxa, 287);
					if (i < 0) break;
					if (i < 256) {
						// そのまま
						buff[wpos++] = i;
					} else if (i == 256) {
						// 終端符号
						break;
					} else {
						//	長さ符号:可変長符号(>256)+拡張符号
						if (i < 265)		l = i -257 +3;
						else if (i >= 285)	l = 258;
						else {
							//	拡張ビット数 = (x - 261) >> 2;
							//	長さ = lenlst[拡張ビット数-1] + ((x-1) & 3) * (1 << 拡張ビット数) + 拡張ビット
							bt = (i -261) >> 2;
							l = lenlst[bt] + ((i -1) & 3) * (1 << bt) + getbits(bt, 0);
						}
						
						//	距離符号:可変長符号+拡張符号
						i = getcode2(clen, hlen, maxb, 31);
						if (i < 0) break;
						if (i < 4) p = i +1;
						else	p = (1 << (i >> 1)) +1 + getbits((i >> 1) -1, 0) + ((i & 1) << ((i >> 1) -1));
						if (wpos+l <= bsiz) {
							for(i=0;i<l;i++)	buff[wpos+i] = buff[wpos-p+i];
						} else i = -1;
						wpos+=l;
					}
				}
				if (i < 0) bfinal = 2;
				break;
			case 3:
				printf("inflate: btype number error!\n");
				break;
		}
		if (wpos > bsiz) break;
		if (btype > 2) break;
	} while (!bfinal);
	if ((bfinal == 1) && (wpos == bsiz) && (i >= 0)) printf("inflate: return=ok\n"); else printf("inflate: return=ng\n");
	if (!bfinal) printf("inflate: bfinal error!\n");
	printf("inflate: limit=%d, wpos=%d, remain=%d\n", bsiz, wpos, bsiz - wpos);
#ifdef	DUMP_DECODE
	{
				int	fd;
				fd = fioOpen("host:dump.bin", O_WRONLY | O_CREAT);
				if (fd < 0) {
					printf("decode_PNG: host open error!\n");
				} else {
					fioWrite(fd, buff, bsiz);
					fioClose(fd);
				}
	}
#endif
//	return -((bfinal != 1) || (wpos != bsiz) || (i < 0));
	return -(i < 0);
}
int gzdecode(u8 *dst, u32 limit, u8 *c, u32 size) {
	u32 p,crc16s,crc16d,crc32s,crc32d;
	int s=-1;
	if (!((size >= 18) && (c[0] == 0x1F) && (c[1] == 0x8B) && (c[2] == 0x08) && (c[3] < 0x20)))	return -1;
	p = 10;
	if (c[3] & 0x04) {	// FLG.FEXTRA
		crc16d = c[p] + 256 * c[p+1];
		printf("gzdecode: found FLG.FEXTRA, %d bytes\n", crc16d);
		p += crc16d +2;
	}
	if (c[3] & 0x08) {	// FLG.FNAME
		printf("gzdecode: found FLG.FNAME: %s\n", &c[p]);
		while(c[p++] && (p < size));
	}
	if (c[3] & 0x10) {	// FLG.FCOMMENT
		printf("gzdecode: found FLG.FCOMMENT: %s\n", &c[p]);
		while(c[p++] && (p < size));
	}
	if (c[3] & 0x02) {	// FLG.FHCRC
		crc16s = c[p] + 256*c[p+1];
		p+=2;
		crc16d = CRC32Check(c, p);
		printf("gzdecode: found FLG.FHCRC: crc16=%04X, result=%08X\n", crc16s, crc16d);
	}
	// c[p]==圧縮されたブロック
	crc32s = c[size-8] + 256*c[size-7] + 65536*c[size-6] + 16777216*c[size-5];
	s = inflate(dst, limit, &c[p], size -p -8);
	crc32d = CRC32Check(dst, limit);
	printf("gzdecode: uncompressed crc32=%08X ( %08X )\n", crc32d, crc32s);
	return s;
}
int inflate_chunk_add(u8 *src, u32 size) {
//	printf("inflate_chunk_add: ");
	if (bitreg == MAX_CHUNKS) {
		// バッファ確保
		void *data;
		data = (void*)malloc(sizeof(chunkindex) * (bitreg +1));
		if (data == NULL) {
			return -1;
		}
		memcpy(data, chunklist, sizeof(chunkindex) * bitreg);
		chunklist = data;
	} else if (bitreg > MAX_CHUNKS) {
		// バッファ拡張
		void *data;
		data = (void*)realloc(chunklist, sizeof(chunkindex) * (bitreg +1));
		if (data == NULL) {
			return -1;
		} else if (data != chunklist) {
			chunklist = data;
		}
	}
	if (!idatchunk) {
		idatchunk = src;
		bitrem = size << 3;
		bitnum = bitreg;
	}
	chunklist[bitreg].buff = src;
	chunklist[bitreg++].size = size;
	totalcsize += size;
//	printf("\rinflate_chunk_add: ok\n");
	return 0;
}
void inflate_chunk_clr(void) {
//	printf("inflate_chunk_clr: ");
	bitptr = 0; bitrem = 0; bitnum = 0; bitreg = 0; totalcsize = 0; idatchunk = NULL;
	if (chunklist && (chunklist != chunkd)) {
		free(chunklist);
	}
	chunklist = chunkd;
//	printf("\rinflate_chunk_clr: ok\n");
}
void inflate_chunk_dst(u8 *dist, u32 limit) {
//	printf("inflate_chunk_dst: ");
	buff = dist; bsiz = limit; wpos = 0;
//	printf("\rinflate_chunk_dst: ok\n");
}
int inflate_chunk_exe(void) {
	int ret;
//	printf("inflate_chunk_exe: ");
	if (!buff) return -1;
	if (!bitreg) return -1;
	ret = inflate_main();
//	printf("\rinflate_chunk_exe: ok\n");
	return ret;
}
int inflate(u8 *dst, u32 limit, u8 *c, u32 size) {
	// deflate圧縮のデコード(全領域版)
	int ret;
//	printf("inflate_chunk_exe: start\n");
	inflate_chunk_clr();
	inflate_chunk_dst(dst, limit);
	inflate_chunk_add(c, size);
	ret = inflate_chunk_exe();
	inflate_chunk_clr();
//	printf("inflate_chunk_exe: ok\n");
	return ret;
}
