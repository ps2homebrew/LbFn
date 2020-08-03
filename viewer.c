#include "launchelf.h"

#define	MAX_CLIP_SIZE	1024
#define	MAX_LINES	32768
#define	MAX_COLS	2048
#define MAX_UNICODE	0x010000

typedef struct {
	unsigned int offset;
	unsigned int bytes;
} ofscache;

//static unsigned char *clipbuffer=NULL;
//static unsigned char editline[MAX_COLS];
static unsigned char displine[2][MAX_COLS];
static int redraw=0, charset=0;
int linenum=0, defaultcharset=0, tabmode=8, tabdisp=0, nldisp=0;
extern unsigned short sjistable[];
//extern unsigned short ucstable[];
static unsigned short ucstable[MAX_UNICODE];
static int ucstabled=0;

enum
{
	TXT_AUTO,
	TXT_BINARY=TXT_AUTO,
	TXT_ASCII,
	TXT_SJIS,
	TXT_EUCJP,
	TXT_JIS,
	TXT_ISO2022JP,
	TXT_BIG5,
	TXT_EUCTW,
	TXT_GB2312,
	TXT_EUCCN,
	TXT_JOHAB,
	TXT_EUCKR,
	TXT_UTF7,
	TXT_UTF8,
	TXT_UTF16BE,
	TXT_UTF16LE,
	TXT_UTF32BE,
	TXT_UTF32LE,
};

enum
{
	TXT_CR,
	TXT_LF,
	TXT_CRLF,
	TXT_LFCR,
};

int txteditfile(int mode, char *file);
int txtedit(int mode, char *file, unsigned char *buffer, unsigned int size);
int bineditfile(int mode, char *file);
int binedit(int mode, char *file, unsigned char *buffer, unsigned int size);
//int txt_convert_encoding(unsigned char *dist, unsigned char *src, int dist_char, int src_char);
int txtdraw(unsigned char *buffer, unsigned int size, int charcode, uint64 color1, uint64 color2);
int utftosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int euctosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int ucstableinit();

////////////////////////////////
// デバッグ用トラップ
void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("bim2bin: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("bim2bin: malloc vaild (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("bim2bin: free vaild (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("bim2bin: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
#define	malloc	X_malloc
#define free	X_free

////////////////////////////////
// テキストビューア(オンメモリバッファ版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能)
//  	*file	ファイル名(表示用)
//  	*buffer	テキストデータ
//  	size	テキストサイズ
int txtedit(int mode, char *file, unsigned char *c, unsigned int size)
{
	int x,y,z,w,u;
	int crlf=0,cr=0,lf=0,lfcr=0,nlf=0,lines=0;
	int sjis=0,eucjp=0,utf8=0;
	int sjisf=0,eucjpf=0,utf8f=0;
	int cp=0,cs=0;
	uint64 color1, color2;
	int sel=0, top=0;
	int y0, y1, i, j;
	
	char msg0[MAX_PATH], msg1[MAX_PATH], tmp[MAX_PATH];
	ofscache line[MAX_LINES];
	
	charset = TXT_ASCII;
	strcpy(msg0, file);
	
	// テキストバッファの改行コードと文字コードを判定しながら行数カウント
	line[0].offset = 0;
	line[0].bytes = 0;
	for (x=0;x<size;x++) {
		y = x + 1; z = y + 1; w = z + 1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		if (w>=size) w=size-1;
		// 新行判定
		if (nlf == 1) {
			lines++;
			if (lines < MAX_LINES) {
				line[lines].offset = x;
				line[lines].bytes = 0;
			}
		}
		if (nlf > 0) nlf--;

		// 文字コード判定
		// 判定の対象はSJIS/EUC-JPとUTF-8のみサポート
		// (UTF-16などは判定できない)
		if (sjisf == 0) {
			if (((c[x] > 0x00) && (c[x] <= 0x7F)) || ((c[x] >= 0xA1) && (c[x] <= 0xDF))) {
				sjisf=1;
			} else if ((((c[x] >= 0x81) && (c[x] <= 0x9F)) || ((c[x] >= 0xE0) && (c[x] <= 0xFC))) && (c[y] >= 0x40) && (c[y] != 0x7F) && (c[y] <= 0xFC)) {
				sjisf=2;
			}
			sjis+=sjisf;
		}
		if (sjisf > 0) sjisf--;
		if (eucjpf == 0) {
			if ((c[x] > 0x00) && (c[x] <= 0x7F)) {
				eucjpf=1;
			} else if ((c[x] >= 0xA1) && (c[x] <= 0xFE) && (c[y] >= 0xA1) && (c[y] <= 0xFE)) {
				eucjpf=2;
			} else if ((c[x] == 0x8E) && (c[y] >= 0xA1) && (c[y] <= 0xDF)) {
				eucjpf=2;
			} else if ((c[x] == 0x8F) && (c[y] >= 0xA1) && (c[y] <= 0xFE) && (c[z] >= 0xA1) && (c[z] <= 0xFE)) {
				eucjpf=3;
			}
			eucjp+=eucjpf;
		}
		if (eucjpf > 0) eucjpf--;
		if (utf8f == 0) {
			if ((c[x] > 0x00) && (c[x] <= 0x7F)) {
				utf8f=1;
			} else if ((c[x] >= 0xC0) && (c[x] <= 0xE0) && (c[y] >= 0x80) && (c[y] <= 0xBF)) {
				u = 0x40 * (c[x] & 0x1F) + (c[y] & 0x3F);
				if (u >= 0x80) {
					utf8f=2;
				}
			} else if ((c[x] >= 0xE0) && (c[x] <= 0xF0) && (c[y] >= 0x80) && (c[y] <= 0xBF) && (c[z] >= 0x80) && (c[z] <= 0xBF)) {
				u = 0x1000 * (c[x] & 0x0F) + 0x40 * (c[y] & 0x3F) + (c[z] & 0x3F);
				if (((u >= 0x800) && (u < 0xD800)) || (u >= 0xE000)) {
					utf8f=3;
				}
			} else if ((c[x] >= 0xF0) && (c[x] <= 0xF8) && (c[y] >= 0x80) && (c[y] <= 0xBF) && (c[z] >= 0x80) && (c[z] <= 0xBF) && (c[w] >= 0x80) && (c[w] <= 0xBF)) {
				u = 0x40000 * (c[x] & 0x07) + 0x1000 * (c[y] & 0x3F) + 0x40 * (c[z] & 0x3F) + (c[w] & 0x3F);
				if ((u >= 0x10000) && (u <= 0x10FFFF)) {
					utf8f=4;
				}
			}
			utf8+=utf8f;
		}
		if (utf8f > 0) utf8f--;
		// 改行コード判定
		if (nlf == 0) {
			if ((c[x] == 13) && (c[y] == 10)) {
				nlf=2; crlf++;
				line[lines].bytes++;
			} else if ((c[x] == 10) && (c[y] == 13)) {
				nlf=2; lfcr++;
				line[lines].bytes++;
			} else if (c[x] == 13) {
				nlf=1; cr++;
			} else if (c[x] == 10) {
				nlf=1; lf++;
			}
			if (lines < MAX_LINES)
				line[lines].bytes++;
		}
	}
	if (nlf > 0) {
		lines++;
		if (lines < MAX_LINES) {
			line[lines].offset = x;
			line[lines].bytes = 0;
		}
	}
	lines++;
	charset = TXT_SJIS; cp = sjis;
	if (eucjp > cp) {	charset = TXT_EUCJP; cp = eucjp;	}
	if (utf8 > cp) {	charset = TXT_UTF8; cp = utf8;	}
	if ((sjis == eucjp) && (sjis == utf8))
		{	charset = TXT_ASCII; cp = sjis;	}
	cs = TXT_AUTO;
	redraw=TRUE;
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if (new_pad) redraw=TRUE;
			if (new_pad & PAD_UP)
				sel--;
			if (new_pad & PAD_DOWN)
				sel++;
			if (new_pad & PAD_LEFT)
				sel-=MAX_ROWS-1;
			if (new_pad & PAD_RIGHT)
				sel+=MAX_ROWS-1;
			if (new_pad & PAD_TRIANGLE)
				break;
			if (new_pad & PAD_CROSS)
				break;
			if (new_pad & PAD_CIRCLE)
				linenum = !linenum;
			if (new_pad & PAD_SQUARE) {
				if (tabmode >= 10) tabmode=2; else tabmode+=2;
			}
			if (new_pad & PAD_START) {
				tabdisp = !tabdisp;
				nldisp = !nldisp;
			}
			if (new_pad & PAD_SELECT)
				break;
			
		}
		// ファイルリスト表示用変数の正規化
	/*	if(top > lines-MAX_ROWS)	top=lines-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= lines)		sel=lines-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;
	*/	if(sel > lines-MAX_ROWS)	sel=lines-MAX_ROWS;
		if(sel < 0)			sel=0;
		top = sel;
		if (redraw) {
			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);
			x = 2*FONT_WIDTH;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			color1 = setting->color[COLOR_TEXT];
			color2 = setting->color[COLOR_HIGHLIGHTTEXT];
			for(i=0; i<MAX_ROWS; i++) {
				if (top+i >= lines)
					break;
				if (linenum && (top+i < lines)) {
					sprintf(tmp, "%5d", top+i+1);
					printXY(tmp, x, y, color1, TRUE);
				}
				if (top+i < lines) {
					if (top+i < MAX_LINES) {
						w = line[top+i].bytes;
						z = line[top+i].offset;
					} else {
						w = line[MAX_LINES-1].bytes;
						z = line[MAX_LINES-1].offset;
					}
					j = txtdraw(c+z, w, cs, color1, color2);
				//	printf("viewer: j: %d\n", j);
					w = j;
					if (linenum) {
						if (w > MAX_ROWS_X) w = MAX_ROWS_X;
					} else {
						if (w > MAX_ROWS_X+6) w = MAX_ROWS_X+6;
					}
				//	printf("viewer: w: %d\n", w);
					for (j=w;j<w+2;j++) {
						displine[0][j] = 0;
						displine[1][j] = 0;
					}
						
				//	printf("viewer: disp0\n");
					if (displine[0] != 0)
						printXY(displine[0], x+linenum*6*FONT_WIDTH, y, color1, TRUE);
				//	printf("viewer: disp1\n");
					if (displine[1] != 0)
						printXY(displine[1], x+linenum*6*FONT_WIDTH, y, color2, TRUE);
				//	printf("viewer: disp ok\n");
				}
        
				y += FONT_HEIGHT;
			}
			if (linenum) {
				itoLine(setting->color[COLOR_FRAME], 7.5*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3, 0,
					setting->color[COLOR_FRAME], 7.5*FONT_WIDTH, y, 0);	
			}
			// スクロールバー
			if(lines > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/lines);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/lines);
				if (y0 == y1) y1++;
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			redraw = FALSE;
			sprintf(msg1, lang->editor_viewer_help, tabmode);
			setScrTmp(msg0, msg1);
			drawScr();
		} else {
			itoVSync();
		}
	}
	return 0;
}

////////////////////////////////
// テキストビューア(ファイル指定版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能)
//  	file	ファイル名
int txteditfile(int mode, char *file)
{
	int fd,ret;
	char *p;
	unsigned char *buffer;
	unsigned int size;
	char fullpath[MAX_PATH];
	
	if (!strncmp(file, "hdd0:/", 6)) {
		strcpy(fullpath, "pfs0:");
		p = strchr(&file[6], '/');
		if (p == NULL) return -1;
		strcat(fullpath, p);
	} else {
		strcpy(fullpath, file);
	}
	printf("viewer: mode: %d\nviewer: file: %s\n", mode, fullpath);
	fd = fioOpen(fullpath, O_RDONLY);
	if (fd<0)
		return -1;
	size = fioLseek(fd, 0, SEEK_END);
	printf("viewer: size: %d\n", size);
	fioLseek(fd, 0, SEEK_SET);
	buffer = (char*)malloc(size);
	if (buffer != NULL)
		fioRead(fd, buffer, size);
	fioClose(fd);
	if (buffer == NULL)
		return -2;
	ret = txtedit(mode, file, buffer, size);
	free(buffer);
	return ret;
}

////////////////////////////////
// 文字列変換
int txtdraw(unsigned char *buffer, unsigned int size, int charcode, uint64 color1, uint64 color2)
{
	int cp,i,j,w,x,c,c0,c1,siz,ret=0;
	unsigned char temp[MAX_COLS];

	// テキストの文字コード
	if (charcode == TXT_AUTO) 
		cp = charset;
	else
		cp = charcode;
	
	// 表示文字列初期化
	displine[0][0] = 0;
	displine[1][0] = 0;
	
	// SJISに変換
	if (cp == TXT_EUCJP)
		ret=euctosjis2(temp, buffer, MAX_COLS, size);
	else if (cp == TXT_UTF8)
		ret=utftosjis2(temp, buffer, MAX_COLS, size);
	else {
		if (size > MAX_COLS) siz = MAX_COLS; else siz = size;
		for(ret=0;ret<siz;ret++)
			temp[ret] = buffer[ret];
	}
	// 表示用変換
	j=0;
	for(i=0;i<ret;i++) {
		c=temp[i];c0=c;c1=32;
		if (c == 9) {
			if (tabmode > 0) {
				w = tabmode - (j % tabmode);
				for (x=j; x<j+w; x++) {
					displine[0][x] = 32;
					displine[1][x] = 32;
				}
				if (tabdisp)
					displine[1][j] = '>';
				c = -1; c0 = 0; j+=w;
			}
		} else if (c == 10) {
			c = 32; c0 = 32;
			if (nldisp) c1 = 'v'; else c1 = 32;
		} else if (c == 13) {
			c = 32; c0 = 32;
			if (nldisp) c1 = '<'; else c1 = 32;
		}
		if ((c >= 0) && (c < 32)) {
			// 制御コード
			//	displine[0][j] = 32;
			//	displine[1][j++] = '^';
			displine[0][j] = 32;
			displine[1][j++] = c+64;
		} else if (c0 > 0) {
			// 通常コード
			displine[0][j] = c0;
			displine[1][j++] = c1;
		}
		displine[0][j] = 0;
		displine[1][j] = 0;
		if (j >= MAX_COLS-2) break;
	}
	//printf("viewer: %d/%d\n", j, ret);
	return j;
}

////////////////////////////////
// 文字列変換
//  in:	*dist	変換先
//  	*src	変換対象
//  	limit	変換先バッファのバイト数
//  	size	変換対象のサイズ
int euctosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size)
{
	int i,j=0;
	int x,y,z,u,t1,t2;
	unsigned char *c, *d;

	c = src; d = dist;
	
	d[j] = 0;
	for(i=0;i<size;i++) {
		x=i;y=x+1;z=y+1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		
		if ((c[x] > 0x00) && (c[x] < 0x80)) {
			d[j++]=c[x];
		} else if ((c[x] == 0x8E) && (c[y] >= 0xA1) && (c[y] <= 0xDF)) {
			d[j++]=c[y];
			i++;
		} else if ((c[x] == 0x8F) && (c[y] >= 0xA1) && (c[y] <= 0xFE) && (c[z] >= 0xA1) && (c[z] <= 0xFE)) {
			// 変換結果にない
			if (j<limit-2)
				d[j++]=0x81;d[j++]=0x45;
			i+=2;
		} else if ((c[x] >= 0xA1) && (c[x] <= 0xFE) && (c[y] >= 0xA1) && (c[y] <= 0xFE)) {
			if (j<limit-2) {
				u=94*(c[x]-0xA1)+(c[y]-0xA1);
				t1=(u/188)+0x81;t2=(u%188)+0x40;
				if (t1 >= 0xA0) t1+=0x40;
				if (t2 >= 0x7F) t2++;
				d[j++]=t1;d[j++]=t2;
			}
			i++;
		}
		d[j] = 0;
		if (j >= limit-1) break;
	}
	return j;
}
int utftosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size)
{
	int i,j=0;
	int x,y,z,w,u,v,t1,t2;
	unsigned char *c, *d;
	
	c = src; d = dist;
	ucstableinit();
	
	d[j] = 0;
	for(i=0;i<size;i++){
		x=i;y=x+1;z=y+1;w=z+1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		if (w>=size) w=size-1;
		
		if (c[x] < 0x80) {
			u=c[x];
		} else if ((c[x] >= 0xC0) && (c[x] < 0xE0) && (c[y] >= 0x80) && (c[y] < 0xC0)) {
			u=0x40*(c[x]&0x1F)+(c[y]&0x3F);
			if (u<0x80) u=0x3F;
			if (u!=0x3F) i++;
		} else if ((c[x] >= 0xE0) && (c[x] < 0xF0) && (c[y] >= 0x80) && (c[y] < 0xC0) && (c[z] >= 0x80) && (c[z] < 0xC0)) {
			u=0x1000*(c[x]&0x0F)+0x40*(c[y]&0x3F)+(c[z]&0x3F);
			if ((u < 0x800) || ((u >= 0xD800) && (u <= 0xDFFF))) u=0x3F;
			if (u!=0x3F) i+=2;
		} else if ((c[x] >= 0xF0) && (c[x] < 0xF8) && (c[y] >= 0x80) && (c[y] < 0xC0) && (c[z] >= 0x80) && (c[z] < 0xC0) && (c[w] >= 0x80) && (c[w] < 0xC0)) {
			u=0x10000*(c[x]&0x07)+0x1000*(c[y]&0x3F)+0x40*(c[z]&0x3F)+(c[w]&0x3F);
			if ((u < 0x10000) || (u > 0x10FFFF)) u=0x3F;
			if (u!=0x3F) i+=3;
		} else {
			u=0x3F;
		}
		
		if (u > 0) {
			if (u < MAX_UNICODE)
				v = ucstable[u];
			else 
				v = 0x3F;
			//printf("viewer: ucs: %X, v: %4X", u, v);
			if (v <= 0xFF)
				d[j++] = v;
			else {
				if (j < limit-2) {
					v-=0x100;
					t1=(v/188)+0x81;t2=(v%188)+0x40;
					if (t1 >= 0xA0) t1+=0x40;
					if (t2 >= 0x7F) t2++;
					d[j++]=t1;d[j++]=t2;
					//printf(" sjis: %02X%02X", t1, t2);
				}
			}
			//printf("\n");
		}
		d[j] = 0;
		if (j >= limit-1) break;
	}
	return j;
}

int ucstableinit()
{	// UCS convert table init
	unsigned int c, u=-1;
	if (ucstabled) return !0;
	// init all charactors to default charactor
	printf("viewer: ucstableinit start\n");
	for (c = 0; c < MAX_UNICODE; c++)
		ucstable[c] = 0x3F;	// question mark
	// making ASCII <-> Unicode convert table
	for (c = 0; c < 128; c++)
		ucstable[c] = c;	// Unicode⇒ANK (it is init, override the next steps)
	// making ANK(HALF WIDTH KATAKANA) <-> Unicode convert table
	for (c = 1; c < 64; c++)
		ucstable[c + 0xFF60] = c + 0xA0;
	// making japanese <-> Unicode convert table
	for (c = 0; c < 11280; c++) {
		u = sjistable[c];
		if ((u > 0x10FFFF) || ((u >= 0xD800) && (u <= 0xDFFF)) || (u == 0)) u = 0x3F;
		if (u >= MAX_UNICODE) u = 0x3F;
		sjistable[c] = u;	// jis codepoint number -> Unicode
		if ((u != 0x3F) && (ucstable[u] == 0x3F))
			ucstable[u] = c + 256;	// Unicode -> codepoint number + 256
	}
	ucstabled = !0;
	printf("viewer: ucstableinited: %d\n", ucstabled);
	return !0;
}
