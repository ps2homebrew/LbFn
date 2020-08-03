#include "launchelf.h"

#define	MAX_LINES	32768
#define	MAX_COLS	16384
#define	MAX_UNICODE	0x010000

typedef struct {
	unsigned int offset;
	unsigned int bytes;
} ofscache;

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
	FT_BINARY,
	FT_ELF,
	FT_JPG,
	FT_PNG,
	FT_GIF,
	FT_BMP,
	FT_MP3,
	FT_AAC,
	FT_AC3,
	FT_PCM,
	FT_TXT,
	FT_ZIP,
	FT_RAR,
	FT_LZH,
	FT_TEK,
	FT_GZ,
	FT_7Z,
	FT_AVI,
	FT_MPG,
	FT_MP4,
	FT_FNT,
	FT_XML,
	FT_HTM,
	FT_TYPES,
};
static unsigned int ft_type[FT_TYPES] = {
	FT_ELF, 
	FT_JPG, FT_PNG, FT_GIF, FT_BMP,
	FT_MP3, FT_AAC, FT_AC3, FT_PCM,
	FT_TXT, FT_XML, FT_HTM,
	FT_ZIP, FT_RAR, FT_LZH, FT_TEK, FT_GZ, FT_7Z,
	FT_AVI, FT_MPG, FT_MP4,
	FT_FNT,
	0
};
static unsigned char ft_char[FT_TYPES][4] = {
	"ELF",
	"JPG", "PNG", "GIF", "BMP",
	"MP3", "AAC", "AC3", "WAV",
	"TXT", "XML", "HTM",
	"ZIP", "RAR", "LZH", "TEK", "GZ ", "7Z ",
	"AVI", "MPG", "MP4",
	"FNT",
};
static unsigned char *clipbuffer=NULL;
static unsigned char editline[3][MAX_COLS];
static unsigned char displine[2][MAX_COLS];
static int redraw=2, charset=0, fullscreen=0;
int linenum=0, defaultcharset=0, tabmode=8, tabdisp=0, nldisp=0;
extern unsigned short sjistable[];
//extern unsigned short ucstable[];
static unsigned short ucstable[MAX_UNICODE];
static int ucstabled=0;
uint64 *activeclut=NULL;
//static ofscache *line[];
int chartable[] = {TXT_AUTO, TXT_SJIS, TXT_EUCJP, TXT_UTF8, -1};

int bineditfile(int mode, char *file);
int binedit(int mode, char *file, unsigned char *buffer, unsigned int size);
int imgviewfile(char *file);
int imgview(char *file, unsigned char *buffer, int w, int h, int bpp);
int viewer_file(int mode, char *file);
int viewer(int mode, char *file, unsigned char *buffer, unsigned int size);
int pcmpause(void);
int pcmplay(void);
int pcminit(int mode, int rate, int channels, int bits);
int pcmclear(void);
int pcmadd(char *buffer, int size);
//int txt_convert_encoding(unsigned char *dist, unsigned char *src, int dist_char, int src_char);
int txtdraw(unsigned char *buffer, unsigned int size, int charcode, uint64 color1, uint64 color2);
int utftosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int euctosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int ucstableinit();

int info_BMP(int *info, char *src, int size);
int decode_BMP(char *dist, char *src, int size, int bpp);
int info_JPEG(void *env, int *info, int size, unsigned char *fp);
int decode0_JPEG(void *env, int size, unsigned char *fp, int b_type, unsigned char *buf, int skip);
int decode0_JPEGpart(void *env, int xsz, int ysz, int x0, int y0, int size, unsigned char *fp, int b_type, unsigned char *buf, int skip);
int info_GIF(int *info, char *src, int size);
int decode_GIF(char *dist, char *src, int size, int bpp);

////////////////////////////////
// デバッグ用トラップ

static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("viewer: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("viewer: malloc vaild (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("viewer: free vaild (ofs: %08X)\n", (unsigned int) mallocdata);
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
//*/

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
	int ascii=0,sjis=0,eucjp=0,utf8=0,text=0,binary=0;
	int sjisf=0,eucjpf=0,utf8f=0;
	int cp=0,cs=0,maxbytes=0;
	uint64 color1, color2;
	int sel=0, top=0, selx=0, oldselx=0, oldsel=0;
	int y0, y1, i, j;
	int l2button=FALSE, oldl2=FALSE;
	int textwidth, editmode=0;
	
	int scrnshot=0;
	
	char msg0[MAX_PATH], msg1[MAX_PATH], tmp[MAX_PATH];
	ofscache line[MAX_LINES];
	
	charset = TXT_ASCII;
	strcpy(msg0, file);
	
	// テキストバッファの改行コードと文字コードを判定しながら行数カウント
	printf("viewer: detecting charset and counting lines...\n");
	line[0].offset = 0;
	line[0].bytes = 0;
	for (x=0;x<size;x++) {
		y = x + 1; z = y + 1; w = z + 1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		if (w>=size) w=size-1;
		// 新行判定
		if (nlf == 1) {
			//if (line[lines].bytes > maxbytes)
			//	maxbytes = line[lines].bytes;
			lines++;
			if (lines < MAX_LINES) {
				line[lines].offset = x;
				line[lines].bytes = 0;
			}
			if ((lines % 4096)==0) {
				printf("viewer: Line %d Start: %08X\r\n", lines, x);
			}
		}
		if (nlf > 0) nlf--;
		// 文字コード判定
		// 判定の対象はSJIS/EUC-JPとUTF-8のみサポート
		// (UTF-16などは判定できない)
		if ((c[x] > 0) && (c[x] <= 0x7F)) ascii++;
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
		if (((c[x] != 13) && (c[x] != 10) && (c[x] != 9) && (c[x] < 0x20)) || (c[x] > 0xFE)) binary++;
		if ((c[x] == 0) || (c[x] == 0xFF)) binary+=4;
		if ((c[x] >= 0x40) && (c[x] < 0x7F) && (c[y] >= 0x40) && (c[y] < 0x7F)) text++;
		// 改行コード判定
		if (nlf == 0) {
			if ((c[x] == 13) && (c[y] == 10)) {
				nlf=2; crlf++;
			} else if ((c[x] == 10) && (c[y] == 13)) {
				nlf=2; lfcr++;
			} else if (c[x] == 13) {
				nlf=1; cr++;
			} else if (c[x] == 10) {
				nlf=1; lf++;
			}
			if (lines < MAX_LINES)
				line[lines].bytes++;
			if ((lines < MAX_LINES) && (nlf == 2)) line[lines].bytes++;
			if (lines == MAX_LINES) break;
		}
	}
	if (nlf > 0) {
		//if (line[lines].bytes > maxbytes)
		//	maxbytes = line[lines].bytes;
		lines++;
		if (lines < MAX_LINES) {
			line[lines].offset = x;
			line[lines].bytes = 0;
		}
	}
	lines++;
	
	if (lines > MAX_LINES) lines = MAX_LINES;
	charset = TXT_SJIS; cp = sjis;
	if (eucjp > cp) {	charset = TXT_EUCJP; cp = eucjp;	}
	if (utf8 > cp) {	charset = TXT_UTF8; cp = utf8;	}
	if ((sjis == eucjp) && (sjis == utf8))
		{	charset = TXT_ASCII; cp = sjis;	}
	cs = TXT_AUTO; cp=0;
	printf("viewer: counted lines: %d\n", lines);
	printf("viewer: charset point list: \n");
	printf("\tASCII: %10d pts.\n", ascii);
	printf("\tSJIS:  %10d pts.\n", sjis);
	printf("\tEUCJP: %10d pts.\n", eucjp);
	printf("\tUTF-8: %10d pts.\n", utf8);
	printf("\tTEXT:  %10d pts.\n", text);
	printf("\tBINARY: %9d pts.\n", binary);
	redraw=fieldbuffers;
	maxbytes=MAX_COLS-1;
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			//if (new_pad) redraw=TRUE;
			if (l2button) {
				if (!(paddata & PAD_L2))
					l2button=FALSE;
				// o:linenum x:flicker _:tab v:char 
				if (new_pad & PAD_CIRCLE) {
					linenum = !linenum;
					redraw = fieldbuffers;
				} else if (new_pad & PAD_CROSS) {
					flickerfilter = !flickerfilter;
					redraw = fieldbuffers;
				} else if (new_pad & PAD_SQUARE) {
					if (tabmode >= 12) tabmode=2; else tabmode+=2;
					redraw = fieldbuffers;
				} else if (new_pad & PAD_TRIANGLE) {
					cp++;
					if (chartable[cp] < 0) cp=0;
					cs = chartable[cp];
					redraw = fieldbuffers;
				} else if (new_pad & PAD_UP) {
					sel = 0;
				} else if (new_pad & PAD_DOWN) {
					sel = lines-MAX_ROWS;
				} else if (new_pad & PAD_LEFT) {
					selx = 0;
				} else if (new_pad & PAD_RIGHT) {
					selx+= MAX_ROWS_X+6*(!linenum)-1;
				} else if (new_pad & PAD_L1) {
					selx-= MAX_ROWS_X;
				} else if (new_pad & PAD_R1) {
					selx+= MAX_ROWS_X;
				}
			} else {
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
				if (new_pad & PAD_CIRCLE) {
					linenum = !linenum;
					redraw = fieldbuffers;
				}
				if (new_pad & PAD_SQUARE) {
					if (tabmode >= 12) tabmode=2; else tabmode+=2;
					redraw = fieldbuffers;
				}
				if (new_pad & PAD_L1)
					selx-= 4;
				if (new_pad & PAD_R1)
					selx+= 4;
				if (paddata & PAD_L2)
					l2button = TRUE;
				if (new_pad & PAD_R2) {
					
					redraw = fieldbuffers;
				}
				if (new_pad & PAD_START) {
					tabdisp = !tabdisp;
					nldisp = !nldisp;
					redraw = fieldbuffers;
				}
				if (new_pad & PAD_SELECT)
					break;
			}
			if (new_pad & PAD_R2) scrnshot = 1;
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
		textwidth = MAX_ROWS_X+6*(!linenum);
		if(selx > MAX_COLS-textwidth)	selx=MAX_COLS-textwidth;
		if(selx < 0)		selx=0;
		top = sel;

		if((selx != oldselx) || (sel != oldsel)) {
			oldsel = sel;
			oldselx = selx;
			redraw = fieldbuffers;
		}
		if (l2button != oldl2) {
			oldl2 = l2button;
			redraw = fieldbuffers;
		}
		if (redraw) {
			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);
			x = 2*FONT_WIDTH;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			color1 = setting->color[COLOR_TEXT];
			color2 = setting->color[COLOR_HIGHLIGHTTEXT];
			for(i=0; i<MAX_ROWS ; i++) {
				if (top+i >= lines)
					break;
				if (linenum && (top+i < lines)) {
					sprintf(tmp, "%7d", top+i+1);
					printXY(tmp, 0, y, color1, TRUE);
				}
				if (top+i < lines) {
					if (top+i < MAX_LINES) {
						w = line[top+i].bytes;
						z = line[top+i].offset;
					} else {
						w = 0;
						z = line[MAX_LINES-1].offset;
					}
					w = txtdraw(c+z, w, cs, color1, color2);
					if (linenum) {
						if (w > selx+MAX_ROWS_X) w = MAX_ROWS_X+selx;
					} else {
						if (w > selx+MAX_ROWS_X+6) w = MAX_ROWS_X+6+selx;
					}
					for (j=w;j<w+2;j++) {
						if (j>=MAX_COLS) break;
						displine[0][j] = 0;
						displine[1][j] = 0;
					}
						
					if ((displine[0] != 0)&&(w>=selx))
						printXY(displine[0]+selx, x+linenum*6*FONT_WIDTH, y, color1, TRUE);
					if ((displine[1] != 0)&&(w>=selx))
						printXY(displine[1]+selx, x+linenum*6*FONT_WIDTH, y, color2, TRUE);
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
			sprintf(msg1, lang->editor_viewer_help, tabmode);
			if(l2button){
				//
				int dialog_x;		//ダイアログx位置
				int dialog_y;		//ダイアログy位置
				int dialog_width;	//ダイアログ幅
				int dialog_height;	//ダイアログ高さ

				dialog_width = FONT_WIDTH*28;
				dialog_height = FONT_HEIGHT*5;
				dialog_x = (SCREEN_WIDTH-dialog_width)/2;
				dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
				// 描画開始
				drawDark();
				drawDialogTmp(dialog_x, dialog_y,
					dialog_x+dialog_width, dialog_y+dialog_height,
					setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
				//
				x = dialog_x+FONT_WIDTH*1;
				y = dialog_y+FONT_HEIGHT*0.5;
				//
				sprintf(tmp, "○:%s", lang->editor_l2popup_linenum);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "△:%s", lang->editor_l2popup_charset);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "×:%s", lang->editor_l2popup_flicker);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "□:%s", lang->editor_l2popup_tabmode);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			}
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return 0;
}
/*
	編集モード操作方法
	・L1/R1	
	・L2/R2	
	・↑/↓	
	・←/→	
	・○	
	・×/△	
	・□	
	・START	
	

*/

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
	char fullpath[MAX_PATH], tmp[MAX_PATH];
	
	if (!strncmp(file, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &file[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		printf("viewer: hddpath: %s\n", fullpath);
		//fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			//fileXioUmount("pfs0:");
			return -1;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		printf("viewer: size: %d\n", size);
		fileXioLseek(fd, 0, SEEK_SET);
		buffer = (char*)malloc(size);
		if (buffer != NULL)
			fileXioRead(fd, buffer, size);
		fileXioClose(fd);
		//fileXioUmount("pfs0:");
	} else {
		strcpy(fullpath, file);
		printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
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
	}
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
	//printf("cp,ret,size,MAX: %d,%d,%d,%d\n", cp, ret, size, MAX_COLS);
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

////////////////////////////////
// 適切な形式で表示(ファイル指定版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能)
//  	file	ファイル名
int viewer_file(int mode, char *file)
{
	int fd,ret;
	char *p;
	unsigned char *buffer;
	unsigned int size;
	char fullpath[MAX_PATH], tmp[MAX_PATH];
	
	if (!strncmp(file, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &file[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		printf("viewer: hddpath: %s\n", fullpath);
		//fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			//fileXioUmount("pfs0:");
			return -1;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		printf("viewer: size: %d\n", size);
		fileXioLseek(fd, 0, SEEK_SET);
		buffer = (char*)malloc(size);
		if (buffer != NULL) {
			drawMsg(lang->gen_loading);
			fileXioRead(fd, buffer, size);
		}
		fileXioClose(fd);
		//fileXioUmount("pfs0:");
	} else {
		strcpy(fullpath, file);
		printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		fd = fioOpen(fullpath, O_RDONLY);
		if (fd<0)
			return -1;
		size = fioLseek(fd, 0, SEEK_END);
		printf("viewer: size: %d\n", size);
		fioLseek(fd, 0, SEEK_SET);
		buffer = (char*)malloc(size);
		if (buffer != NULL) {
			drawMsg(lang->gen_loading);
			fioRead(fd, buffer, size);
		}
		fioClose(fd);
	}
	if (buffer == NULL)
		return -2;
	ret = viewer(mode, file, buffer, size);
	free(buffer);
	return ret;
}


////////////////////////////////
// 適切な形式で表示(オンメモリ版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能)
//  	*file	ファイル名(表示用)
//  	*buffer	バッファ
//  	size	サイズ
/*
void seti32(unsigned char *p, int i) {
	p[0] = i & 0xFF;
	p[1] = (i & 0xFF00) >> 8;
	p[2] = (i & 0xFF0000) >> 16;
	p[3] = (i & 0xFF000000) >> 24;
}//*/
int viewer(int mode, char *file, unsigned char *c, unsigned int size)
{
	int type=FT_TXT,i,info[8],bpp,tvmode;
	int *env;
	unsigned char *buffer;
	tvmode = setting->tvmode;
	if (gsregs[tvmode].loaded != 1) tvmode = ITO_VMODE_AUTO-1;
	bpp = setting->screen_depth[tvmode] > 0 ? (setting->screen_depth[tvmode]-1):4-gsregs[tvmode].psm;
	drawMsg(lang->gen_decoding);
	// 簡易版ファイル形式判別
	if ((size >= 32) && (c[0] == 0xFF) && (c[1] == 0xD8) && (c[2] == 0xFF)) {
		type = FT_JPG;
	} else if ((size >= 16) && (c[0] == 0x89) && (c[1] == 0x50) && (c[2] == 0x4E) && (c[3] == 0x47) && (c[4] == 0x0D) && (c[5] == 0x0A)) {
		type = FT_PNG;
	} else if ((size >= 8) && (c[0] == 0x47) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x38) && (c[4] == 0x39) && (c[5] == 0x61)) {
		type = FT_GIF;
	} else if ((size >= 14) && (c[0] == 0x42) && (c[1] == 0x4D) && (c[6] == 0x00) && (c[7] == 0x00) && (c[8] == 0x00) && (c[9] == 0x00)) {
		type = FT_BMP;
	} else if ((size >= 64) && (c[0] == 0x7F) && (c[1] == 0x45) && (c[2] == 0x4C) && (c[3] == 0x46) && (c[4] == 0x01) && (c[5] == 0x01)) {
		type = FT_ELF;
	} else if ((size >= 6) && (c[0] == 0x52) && (c[1] == 0x61) && (c[2] == 0x72) && (c[3] == 0x21) && (c[4] == 0x1A) && (c[5] == 0x07)) {
		type = FT_RAR;
	} else if ((size >= 4) && (c[0] == 0x50) && (c[1] == 0x4B) && (c[2] == 0x03) && (c[3] == 0x04)) {
		type = FT_ZIP;
	} else if ((size >= 8) && (c[2] == 0x2D) && (c[3] == 0x6C) && (c[4] == 0x68) && (c[6] == 0x2D)) {
		type = FT_LZH;
	} else if ((size >= 4) && (c[0] == 0x1F) && (c[1] == 0x8B)) {
		type = FT_GZ;
	} else if ((size >= 24) && (c[1] == 0xFF) && (c[2] == 0xFF) && (c[3] == 0xFF) && (c[4] == 0x01) && (c[5] == 0x00) && (c[6] == 0x00) && (c[7] == 0x00) && (c[8] == 0x4F) && (c[9] == 0x53) && (c[10] == 0x41) && (c[11] == 0x53) && (c[12] == 0x4B) && (c[13] == 0x43) && (c[14] == 0x4D) && (c[15] == 0x50)) {
		type = FT_TEK;
	} else if ((size >= 44) && (c[0] == 0x52) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x46) && (c[8] == 0x57) && (c[9] == 0x41) && (c[10] == 0x56) && (c[11] == 0x45)) {
		type = FT_PCM;
	} else if ((size >= 64) && (((c[0] == 0xFF) && ((c[1] & 0xF0) == 0xF0) && (c[4] == 0x00) && (c[5] = 0x00)) || ((c[0] == 0x49) && (c[1] == 0x44) && (c[2] == 0x33)))) {
		type = FT_MP3;
	} else if ((size >= 64) && (c[4] == 0x66) && (c[5] == 0x74) && (c[6] == 0x79) && (c[7] == 0x70) && (c[8] == 0x4D) && (c[9] == 0x34) && (c[10] == 0x41) && (c[11] == 0x20)) {
		type = FT_AAC;
	} else if ((size >= 64) && (c[4] == 0x66) && (c[5] == 0x74) && (c[6] == 0x79) && (c[7] == 0x70) && (c[8] == 0x6D) && (c[9] == 0x70) && (c[10] == 0x34) && (c[11] == 0x32)) {
		type = FT_AAC;
	} else if ((size >= 64) && (c[0] == 0x0B) && (c[1] == 0x77) && (c[4] == 0x14) && (c[5] == 0x20) && (c[6] == 0x43) && (c[7] == 0xFE)) {
		type = FT_AC3;
	} else if ((size >= 64) && (c[0] == 0x52) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x46) && (c[8] == 0x41) && (c[9] == 0x56) && (c[10] == 0x49) && (c[11] == 0x20)) {
		type = FT_AVI;
	} else if ((size >= 64) && (c[0] == 0x00) && (c[1] == 0x00) && (c[2] == 0x01) && (c[3] == 0xBA)) {
		type = FT_MPG;
	} else if ((size >= 64) && (c[4] == 0x66) && (c[5] == 0x74) && (c[6] == 0x79) && (c[7] == 0x70)) {
		type = FT_MP4;
	} else if ((size >= 20) && (c[0] == 0x46) && (c[1] == 0x4F) && (c[2] == 0x4E) && (c[3] == 0x54) && (c[4] == 0x58) && (c[5] == 0x32)) {
		type = FT_FNT;
	} else if ((size >= 20) && (c[0] == 0x46) && (c[1] == 0x4F) && (c[2] == 0x4E) && (c[3] == 0x01)) {
		type = FT_FNT;
	} else if ((size >= 32) && (c[0] == 0x3C) && (c[1] == 0x21) && (c[2] == 0x44) && (c[3] == 0x4F) && (c[10] == 0x48) && (c[11] == 0x54) && (c[12] == 0x4D) && (c[13] == 0x4C)) {
		type = FT_HTM;
	} else if ((size >= 12) && (c[0] == 0x3C) && (c[1] == 0x68) && (c[2] == 0x74) && (c[3] == 0x6D) && (c[4] == 0x6C)) {
		type = FT_HTM;
	} else if ((size >= 32) && (c[0] == 0x3C) && (c[1] == 0x21) && (c[2] == 0x44) && (c[3] == 0x4F) && (c[10] == 0x68) && (c[11] == 0x74) && (c[12] == 0x6D) && (c[13] == 0x6C)) {
		type = FT_HTM;
	} else if ((size >= 12) && (c[0] == 0x3C) && (c[1] == 0x48) && (c[2] == 0x54) && (c[3] == 0x4D) && (c[4] == 0x4C)) {
		type = FT_HTM;
	} else if ((size >= 32) && (c[0] == 0x3C) && (c[1] == 0x3F) && (c[2] == 0x78) && (c[3] == 0x6D) && (c[4] == 0x6C) && (c[5] == 0x20)) {
		type = FT_XML;
	} else if ((size >= 32) && (c[1] == 0x3C) && (c[2] == 0x3F) && (c[3] == 0x78) && (c[4] == 0x6D) && (c[5] == 0x6C) && (c[6] == 0x20)) {
		type = FT_XML;
	}
	// BIN/ELF/JPG/PNG/GIF/BMP/MP3/AAC/AC3/PCM/TXT/ZIP/RAR/LZH/TEK/GZ/7Z
	for (i=0;i<FT_TYPES;i++) {
		if (ft_type[i] == 0) {
			printf("viewer: filetype: BINARY\n");
			break;
		} else if (ft_type[i] == type) {
			printf("viewer: filetype: %s\n", ft_char[i]);
			break;
		}
	}
	switch(type){
		case FT_JPG:
		{
			env = malloc(16384*sizeof(int));
			if (env == NULL) return -2;
			env[0] = 0; env[1] = 0;
			i = info_JPEG(&env, info, size, c);
			printf("viewer: info_JPEG returned: %d (env: %d,%d)\n", i, env[0], env[1]);
			printf("viewer: info data: %d,%d,%dx%d\n", info[0], info[1], info[2], info[3]);
			if ((info[0] != 0x0002) || (info[2] * info[3] == 0)) {
				free(env);
				break;
			}
			buffer = malloc(info[2] * info[3] * bpp);
			if (buffer == NULL) {
				if ((bpp <= 2) || ((bpp>2) && ((bpp=2)==2) && ((buffer = malloc(info[2] * info[3] * bpp))==NULL))) {
					free(env);
					return -2;
				}
			}
			i = decode0_JPEG(env, size, c, bpp, buffer, 0);
			printf("viewer: decode0_JPEG(%dbpp) returned: %d (env: %d,%d)\n", bpp*8, i, env[0], env[1]);
/*	デコード結果を表示できるようにヘッダをつけてPCにダンプ
			// 32bppのヘッダ
			static unsigned char header[66] = "BM____\x00\x00\x00\x00\x42\x00\x00\x00\x28\x00" "\x00\x00________\x01\x00\x20\x00\x03\x00" "\x00\x00\x00\x00\x00\x00\xA0\x05\x00\x00\xA0\x05\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\xFF\x00\x00\xFF\x00\x00\xFF\x00" "\x00\x00";
			int fd;
			fd = fioOpen("host:JPEGDEC.BMP", O_WRONLY | O_CREAT);
			if (fd == NULL) {
				printf("viewer: host open error!\n");
			} else {
				seti32(header+2, info[2]*info[3]*4+66);
				seti32(header+18, info[2]);
				seti32(header+22, -info[3]);
				fioWrite(fd, header, 66);
				fioWrite(fd, buffer, info[2] * info[3] * 4);
				fioClose(fd);
			}
*/			i = imgview(file, buffer, info[2], info[3], bpp);
			free(buffer);
			free(env);
			return i;
		}
		case FT_BMP:
		{
			i = info_BMP(info, c, size);
			if (bpp > ((info[1] +7)>>3))
				bpp = (info[1] +7)>>3;
			if (bpp == 0) bpp++;
			printf("viewer: info_BMP returned: %d\n", i);
			printf("viewer: info_data: %d, %dx%d (%dbpp)\n", info[0], info[2], info[3], info[1]);
			if ((info[0] != 0x0001) || (info[2] * info[3] == 0))
				break;
			if (bpp == 3) bpp++;
			buffer = malloc(info[2] * info[3] * bpp);
			if (buffer == NULL) {
				if ((bpp <= 2) || ((bpp>2) && ((bpp=2)==2) && ((buffer = malloc(info[2] * info[3] * bpp))==NULL))) {
					return -2;
				}
			}
			i = decode_BMP(buffer, c, size, bpp);
			printf("viewer: decode_BMP(%dbpp) returned: %d\n", bpp*8, i);
			i = imgview(file, buffer, info[2], info[3], bpp);
			free(buffer);
			return i;
		}
		case FT_GIF:
		{
			i = info_GIF(info, c, size);
			bpp = 1;
			printf("viewer: info_GIF returned: %d\n", i);
			printf("viewer: info data: %d,%dx%d(%dbpp)\n", info[0], info[2], info[3], info[1]);
			if ((info[0] != 0x0008) || (info[2] * info[3] == 0))
				break;
			buffer = malloc(info[2] * info[3] * bpp);
			if (buffer == NULL)
				return -2;
			i = decode_GIF(buffer, c, size, bpp);
			printf("viewer: decode_GIF(%dbpp) returned: %d\n", bpp*8, i);
			i = imgview(file, buffer, info[2], info[3], bpp);
			free(buffer);
			return i;
		}
		case FT_PCM:
		{
			break;
		}
	}
	return txtedit(mode, file, c, size);
}

static int memoryerror=0;

uint64 pget(unsigned char *buffer, int x, int y, int w, int h, int bpp)
{
	unsigned char *c;
	unsigned short *i;
	if ((x < 0) || (x >= w) || (y < 0) || (y >= h)) return 0;
	if (bpp == 0) {
		// 16色
		
	} else {
		c = buffer + (y*w+x)*bpp;
		switch(bpp) {
			case 1: // 256色
			{
			//	return ((uint64) (c[0] & 0x30) << 22)|((uint64) (c[0] & 0x0C) << 14)|((uint64) (c[0] & 0x03) << 6);
				return activeclut[*c];
			}
			case 2: // HighColor
			{
				i = (short *) c;
				return ((uint64) (i[0] & 0x7C00) << 9)|((uint64) (i[0] & 0x03E0) << 6)|((uint64) (i[0] & 0x001F) << 3);
			}
			case 3:
			case 4: // TrueColor
			{
			/*	if (c[3] != 0) {
					//printf("viewer: memory error at %08X\n", (int) &c[0]);
					if (!memoryerror) memoryerror = (int) &c[0];
					return 0;
				}
			*/	return ((uint64) c[0] << 16)|((uint64) c[1] << 8)|(uint64) c[2];
			}
		}
	}
	return 0;
}
int imgview(char *file, unsigned char *buffer, int w, int h, int bpp)
{
	int redraw=fieldbuffers;
	//int sl=0,st=0,sw=1,sh=1; // カーソル位置
	int tl,tt,tw,th,x,y;
	int vl,vt,vw,vh; // ビューポート(描画可能範囲)
	int dl,dt,dw,dh; // 描画範囲用
	int k,gx,gy,gz;
	//uint64 color;
	//unsigned char *d;
	char msg0[MAX_PATH], msg1[MAX_PATH];
	double mx,my,dx,dy;	// 倍率
	strcpy(msg0, file);
	sprintf(msg1, lang->editor_image_help, w, h);
	// イメージ全体をアスペクト比を保ったままビューポート全体に表示するように初期値を調整
	// イメージ1ドット進むごとに画面はbx,byピクセル進む(=拡大率)
	//mx = (double) vw / w;
	//my = (double) vh / h;
	// 画面1ピクセル進むごとにイメージはbx,byドット進む(=縮小率)
	//dx = (double) w / vw;
	//dy = (double) h / vh;
	//if (mx < my) my = mx; else mx = my;
	gz = setting->tvmode;
	if (gsregs[gz].loaded != 1) gz = ITO_VMODE_AUTO-1;
	gx = (gsregs[gz].magx +1)<<2;
	gy = (gsregs[gz].magy +1)<<2;
	if ((gsregs[gz].vmode == 2) || (gsregs[gz].vmode == 3))
		gx /= 4;
	else if ((gsregs[gz].vmode == 80) || ((gsregs[gz].vmode >=16) && (gsregs[gz].vmode < 64)))
		gx /= 2;
	printf("viewer: gx=%d, gy=%d, magx=%d, magy=%d\n", gx, gy, gsregs[gz].magx, gsregs[gz].magy);
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if (new_pad & PAD_TRIANGLE) break;
			if (new_pad & PAD_CROSS) break;
			if (new_pad & PAD_SELECT) break;
			if (new_pad & PAD_CIRCLE) {
				redraw = fieldbuffers;
				fullscreen = !fullscreen;
				if (ffmode) {
					clrScr(setting->color[COLOR_BACKGROUND]);
					itoGsFinish();
					if (framebuffers > 1) {
						itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
						clrScr(setting->color[COLOR_BACKGROUND]);
						itoGsFinish();
						itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
					}
				}
			}
		}
	
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);
			// ビューポートの設定
			if (fullscreen) {
				vl = 0;
				vt = 0;
				vw = SCREEN_WIDTH;
				vh = SCREEN_HEIGHT;
			} else {
				vl = 0;
				vt = SCREEN_MARGIN+FONT_HEIGHT*2.5+1;
				vw = SCREEN_WIDTH;
				vh = MAX_ROWS*FONT_HEIGHT+FONT_HEIGHT-1;
				// ついでに枠なども描画
				setScrTmp(msg0, msg1);
			}
			// リサイズ後のサイズを求める
			// イメージ1ドット進むごとに画面はbx,byピクセル進む(=拡大率)
			mx = (double) vw * gx / w;
			my = (double) vh * gy * (ffmode+1) / h;
			// 画面1ピクセル進むごとにイメージはbx,byドット進む(=縮小率)
			//dx = (double) w / vw;
			//dy = (double) h / vh;
			if (mx < my) my = mx; else mx = my;
			dx = 1/mx*gx;
			dy = 1/my*gy;
			//printf("screen: %dx%d, ps: %.3fx%.3f, %.3fx%.3f\n", vw, vh, dx, dy, mx, my);
			tw = w * mx / gx; th = h * my / (ffmode+1) / gy;
			tl = (vw - tw) / 2;
			tt = (vh - th) / 2;
			if (tw > vw) dw = vw; else dw = tw;
			if (th > vh) dh = vh; else dh = th;
			if (tl < 0) dl = vl; else dl = tl+vl;
			if (tt < 0) dt = vt; else dt = tt+vt;
			// イメージの描画
			// (テクスチャ読み込みをしてテクスチャを貼った方が良いかも知れない、特に極小サイズのイメージの場合)
			if (ffmode) {
				k=itoGetActiveFrameBuffer();
				for (y=0;y<dh;y++)
					for (x=0;x<dw;x++)
						itoPoint(pget(buffer,x*dx,(y*2+k)*dy,w,h,bpp), x+dl, y+dt, 0);
			} else {
				for (y=0;y<dh;y++)
					for (x=0;x<dw;x++)
						itoPoint(pget(buffer,x*dx,y*dy,w,h,bpp), x+dl, y+dt, 0);
			}
			if (memoryerror) {
//	デコード結果を表示できるようにヘッダをつけてPCにダンプ
				printf("viewer: memory error at %08X (buff:%08X, ptr:%08X)\n", memoryerror, (int)&buffer[0], memoryerror-(int)&buffer[0]);
/*				static unsigned char header[66] = "BM____\x00\x00\x00\x00\x42\x00\x00\x00\x28\x00" "\x00\x00________\x01\x00\x20\x00\x03\x00" "\x00\x00\x00\x00\x00\x00\xA0\x05\x00\x00\xA0\x05\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\xFF\x00\x00\xFF\x00\x00\xFF\x00" "\x00\x00";
				int fd;
				fd = fioOpen("host:JPEGDEC.BMP", O_WRONLY | O_CREAT);
				if (fd == 0) {
					printf("viewer: host open error!\n");
				} else {
					seti32(header+2, w*h*4+66);
					seti32(header+18, w);
					seti32(header+22, -h);
					fioWrite(fd, header, 66);
					fioWrite(fd, buffer, w * h * 4);
					fioClose(fd);
				}
*/				memoryerror = 0;
			}
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return 0;
}

int set_viewerconfig(int linedisp, int tabspaces, int chardisp, int screenmode)
{
	linenum = linedisp;
	tabmode = tabspaces;
	tabdisp = nldisp = chardisp;
	fullscreen = screenmode;
	return 0;
}

