#include "launchelf.h"

#define	MAX_LINES	32768
#define	MAX_COLS	16384
#define	MAX_UNICODE	0x010000

typedef struct {
	unsigned int offset;
	unsigned int bytes;
} ofscache;

static unsigned int ft_type[FT_TYPES] = {
	FT_ELF, FT_EXE,
	FT_JPG, FT_PNG, FT_GIF, FT_BMP, FT_P2T, FT_PS1, FT_ICO,
	FT_MP3, FT_AAC, FT_AC3, FT_PCM, FT_MID,
	FT_TXT, FT_XML, FT_HTM,
	FT_ZIP, FT_RAR, FT_LZH, FT_TEK, FT_GZ, FT_7Z,
	FT_AVI, FT_MPG, FT_MP4,
	FT_FNT,
	0
};
static unsigned char ft_char[FT_TYPES][4] = {
	"ELF", "EXE",
	"JPG", "PNG", "GIF", "BMP", "ICO", "PS1", "ICO",
	"MP3", "AAC", "AC3", "WAV", "MID",
	"TXT", "XML", "HTM",
	"ZIP", "RAR", "LZH", "TEK", "GZ ", "7Z ",
	"AVI", "MPG", "MP4",
	"FNT",
};
//static unsigned char *clipbuffer=NULL;
//static unsigned char editline[2][MAX_COLS];
static unsigned char displine[MAX_COLS];
static int redraw=2, charset=0, fullscreen=0, resizer=1;
int linenum=0, defaultcharset=0, tabmode=8, tabdisp=0, nldisp=0;
extern unsigned short sjistable[];
//extern unsigned short ucstable[];
static unsigned short ucstable[MAX_UNICODE];
static int ucstabled=0;
static int wordwrap=0;
uint64 *activeclut=NULL;
static int alphablend=0;
//static ofscache *line[];
static int chartable[] = {TXT_AUTO, TXT_ASCII, TXT_SJIS, TXT_EUCJP, TXT_JIS, TXT_UTF8, -1};
static char chartablename[][8] = {"AUTO", "ASCII", "SJIS", "EUCJP", "JIS", "UTF8", ""};
static unsigned char ctrlchars[32];
//*/
//int txt_convert_encoding(unsigned char *dist, unsigned char *src, int dist_char, int src_char);
int txtdraw(unsigned char *buffer, unsigned int size, int charcode);
int utftosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int euctosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int jistosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size);
int ucstableinit();

int info_BMP(int *info, char *src, int size);
int decode_BMP(char *dist, char *src, int size, int bpp);
int info_JPEG(void *env, int *info, int size, unsigned char *fp);
int decode0_JPEG(void *env, int size, unsigned char *fp, int b_type, unsigned char *buf, int skip);
int decode0_JPEGpart(void *env, int xsz, int ysz, int x0, int y0, int size, unsigned char *fp, int b_type, unsigned char *buf, int skip);
int info_GIF(int *info, char *src, int size);
int decode_GIF(char *dist, char *src, int size, int bpp);
int info_PS2ICO(int *info, char *src, int size);
int decode_PS2ICO(char *dist, char *src, int size, int bpp);
int info_PS1ICO(int *info, char *src, int size);
int decode_PS1ICO(char *dist, char *src, int size, int bpp);

////////////////////////////////
// デバッグ用トラップ

static void *X_malloc(size_t mallocsize)
{
	void *ret;
	ret = malloc(mallocsize);
	if (ret == NULL)
		printf("viewer: malloc failed (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	else
		printf("viewer: malloc valid (ofs: %08X, size: %d)\n", (unsigned int) ret, mallocsize);
	return ret;
}
static void X_free(void *mallocdata)
{
	if (mallocdata != NULL) {
		printf("viewer: free valid (ofs: %08X)\n", (unsigned int) mallocdata);
		free(mallocdata);
	} else 
		printf("viewer: free failed (ofs: %08X)\n", (unsigned int) mallocdata);
	mallocdata = NULL;
}
/*
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
*/
#define	malloc	X_malloc
#define free	X_free
//#define realloc	X_realloc
//*/

void makectrlchars(void)
{
	int i;
	for (i=0; i<32; i++)
		ctrlchars[i] = i+64;
	if (nldisp) {
		ctrlchars[13] = '<';
		ctrlchars[10] = 'v';
	} else {
		ctrlchars[13] = ' ';
		ctrlchars[10] = ' ';
	}
	if (tabdisp) ctrlchars[9] = '>'; else ctrlchars[9] = ' ';
}
////////////////////////////////
// 文字コードの判定
//	in:	*buffer	テキストデータ
//		size	テキストサイズ
//	out:(ret)	文字コード
int txt_detect(unsigned char *c, unsigned int size)
{
	int w,x,y,z,u,charset,cp;
	int ascii=0,jis=0,sjis=0,eucjp=0,utf8=0,text=0,binary=0;
	int jisb=0,jisf=0,jism=0,sjisf=0,eucjpf=0,utf8f=0;
	// 文字コードを判定
	printf("textdetect: detecting charset...\n");
	for (x=0;x<size;x++) {
		y = x + 1; z = y + 1; w = z + 1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		if (w>=size) w=size-1;
		// 文字コード判定
		// 判定の対象はSJIS/EUC-JPとUTF-8のみサポート
		// (UTF-16などは判定できない)
		if ((c[x] > 0) && (c[x] <= 0x7F)) ascii++;
		if ((c[x] == 0xCD) && (c[y] == 0xCD)) ascii+=4;
		if ((c[x] == 0xC4) && (c[y] == 0xC4)) ascii+=4;
		if ((c[x] == 0x80) || (c[x] == 0xA0) || (c[x] > 0xFC)) ascii+=2;
		if (sjisf == 0) {
			if (((c[x] > 0x00) && (c[x] <= 0x7F)) || ((c[x] >= 0xA1) && (c[x] <= 0xDF))) {
				sjisf=1;
			} else if ((((c[x] >= 0x81) && (c[x] <= 0x9F)) || ((c[x] >= 0xE0) && (c[x] <= 0xFC))) && (c[y] >= 0x40) && (c[y] != 0x7F) && (c[y] <= 0xFC)) {
				sjisf=2;
			}
			sjis+=sjisf;
			if (sjisf == 0) sjis--;
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
			if (eucjpf == 0) eucjp--;
		}
		if (eucjpf > 0) eucjpf--;
		if (jisf == 0) {
			if ((c[x] == 0x1B) && ((c[y] == 0x24) || (c[y] == 0x26)) && ((c[z] == 0x40) || (c[z] == 0x42))) {
				jisf=3; jism=3;
			} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x42)) {
				// ^[(B ASCII(7bit)
				jisf=3; jism=0;
			} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x4A)) {
				// ^[(J JISローマ字?(=ASCII+KANA=ANK?=8bit?)
				jisf=3; jism=1;
			} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x49)) {
				// ^[(I 半角カタカナ(7bit?)
				jisf=3; jism=2;
			} else if (c[x] == 0x0E) {
				jisb=jism; jism=2; jisf=1;
			} else if (c[x] == 0x0F) {
				jism=jisb; jisb=0; jisf=1;
			} else if (jism == 1) {
				// JISローマ字(ANK?)
				if ((c[x] < 0x80) || ((c[x] >= 0xA1) && (c[x] <= 0xDF)))
					jisf=1;
			} else if (jism == 2) {
				// 半角カタカナ
				if ((c[x] >= 0x21) && (c[x] <= 0x5F))
					jisf=1;
			} else if (jism == 3) {
				// 漢字
				if ((c[x] < 0x21) || (c[x] > 0x7E) || (c[y] < 0x21) || (c[y] > 0x7E))
					jis-=4;
				jisf=2;
			} else if ((c[x] > 0) && (c[x] != 27) && (c[x] < 0x80)) {
				jisf=1;
			} else {
				jis--;
			}
			jis+=jisf+(jism>1);
			if (jisf == 0) jis--;
		}
		if (jisf > 0) jisf--;
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
			if (utf8f == 0) utf8--;
		}
		if (utf8f > 0) utf8f--;
		if (((c[x] != 13) && (c[x] != 10) && (c[x] != 9) && (c[x] < 0x20)) || (c[x] > 0xFE)) binary++;
		if ((c[x] == 0) || (c[x] == 0xFF)) binary+=4;
		if ((c[x] >= 0x40) && (c[x] < 0x7F) && (c[y] >= 0x40) && (c[y] < 0x7F)) text++;
	}
	charset = TXT_SJIS; cp = sjis;
	if (eucjp >= cp)	{charset = TXT_EUCJP; cp = eucjp;}
	if (utf8 >= cp) 	{charset = TXT_UTF8; cp = utf8;	}
	if (jis >= cp)		{charset = TXT_JIS; cp = jis;}
	if (ascii >= cp)	{charset = TXT_ASCII; cp = ascii;}
	if (binary >= cp)	{charset = TXT_BINARY; cp = binary;}
	printf("textdetect:%d txt=%d,bin=%d, asc=%d,sjis=%d,euc=%d,jis=%d,utf8=%d pts.\n", charset, text, binary, ascii, sjis, eucjp, jis, utf8);
	return charset;
}

////////////////////////////////
// 行数のカウント
//	in:	*buffer	テキストデータ
//		size	テキストサイズ
//		*dist	ポインタキャッシュ配列へのポインタ
//		maxcols	自動折り返しの設定(=0:折り返しをしない)
//		maxrows	用意した*distの行数(カウントを中止する行数)
//	out:(ret)	行数(最後の行を含む)
int txt_count(unsigned char *c, unsigned int size, ofscache *dist, int maxcols, int maxrows)
{
	unsigned int x, y;
	int t=0, line=0, maxcol;
	
	maxcol = maxcols;
	if (maxcol < 0) maxcol = MAX_ROWS_X;
	if (maxcol == 0) maxcol = size+1;
	dist[line].offset = 0;
	dist[line].bytes = 0;
	
	// 行数をカウント
	printf("textcount: counting lines...\n");
	for (x=0; x<size; x++) {
		y = x + 1;
		if (y >= size) y = x;
		if ((c[x] == 13) || (c[x] == 10)) {
			if ((c[x] != c[y]) && ((c[y] == 13) || (c[y] == 10))) {
				dist[line].bytes++;
				x++;
			}
			t = 1;
		}
		if ((++dist[line].bytes >= maxcol) || t) {
			t = 0;
			if (++line < maxrows) {
				dist[line].offset = x+1;
				dist[line].bytes = 0;
			} else {
				printf("textcount: abort the line count.\n");
				break;
			}
		}
	}
	if (++line > maxrows) line = maxrows;
	printf("textcount: result is %d line(s).\n", line);
	return line;
}

////////////////////////////////
// テキストビューア(オンメモリバッファ版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能)
//  	*file	ファイル名(表示用)
//  	*buffer	テキストデータ
//  	size	テキストサイズ
int txtedit(int mode, char *file, unsigned char *c, unsigned int size)
{
	int x,y,z,w,lines=0,linenum2;
	//int crlf=0,cr=0,lf=0,lfcr=0,nlf=0,lines=0;
	int cp=0,cs=0,maxbytes=0;
	uint64 color1, color2;
	int sel=0, top=0, selx=0, oldselx=0, oldsel=0;
	int i, j;
	int l2button=FALSE, oldl2=FALSE;
	int textwidth;
	char *onoff[2] = {lang->conf_off, lang->conf_on};
	
	int scrnshot=0;
	
	char msg0[MAX_PATH], msg1[MAX_PATH], tmp[MAX_PATH];
	ofscache line[MAX_LINES];
	
	strcpy(msg0, file);
	
	lines = txt_count(c, size, line, wordwrap * (MAX_ROWS_X +6), MAX_LINES);
	if (lines == MAX_LINES)
		size = line[lines-1].offset + line[lines-1].bytes;
	charset = txt_detect(c, size);
	
	cs = TXT_AUTO; cp=0;
	redraw=fieldbuffers;
	maxbytes=MAX_COLS-1;
	makectrlchars();
	
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
					mkfontcacheset();
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
				} else if (new_pad & PAD_R2) {
					wordwrap ^= 1;
					lines = txt_count(c, size, line, wordwrap * (MAX_ROWS_X +6), MAX_LINES);
					redraw = fieldbuffers;
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
					makectrlchars();
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
		if(wordwrap)		selx=0;
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
			linenum2 = wordwrap ? 0:linenum;
			for(i=0; i<MAX_ROWS ; i++) {
				if (top+i >= lines)
					break;
				if (linenum2 && (top+i < lines)) {
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
					w = txtdraw(c+z, w, cs);
					if (linenum2) {
						if (w > selx+MAX_ROWS_X) w = MAX_ROWS_X+selx;
					} else {
						if (w > selx+MAX_ROWS_X+6) w = MAX_ROWS_X+6+selx;
					}
					for (j=w;j<w+2;j++) {
						if (j>=MAX_COLS) break;
						displine[j] = 0;
					}
						
					if (cs == TXT_ASCII) {
						if ((displine[0] != 0)&&(w>=selx))
							drawString(displine+selx, TXT_ASCII, x+linenum2*6*FONT_WIDTH, y, color1, color2, ctrlchars);
					} else {
						if ((displine[0] != 0)&&(w>=selx))
							drawString(displine+selx, TXT_SJIS, x+linenum2*6*FONT_WIDTH, y, color1, color2, ctrlchars);
					}
				}
        
				y += FONT_HEIGHT;
			}
			if (linenum2) {
				itoLine(setting->color[COLOR_FRAME], 7.5*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3, 0,
					setting->color[COLOR_FRAME], 7.5*FONT_WIDTH, y, 0);	
			}
			// スクロールバー
			if(lines > MAX_ROWS)
				drawBar((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3, (MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),
						setting->color[COLOR_FRAME], top, MAX_ROWS, lines);
			sprintf(msg1, lang->editor_viewer_help, tabmode);
			if(l2button){
				//
				int dialog_x;		//ダイアログx位置
				int dialog_y;		//ダイアログy位置
				int dialog_width;	//ダイアログ幅
				int dialog_height;	//ダイアログ高さ

				dialog_width = FONT_WIDTH*29;
				dialog_height = FONT_HEIGHT*6;
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
				sprintf(tmp, "△:%s [%s]", lang->editor_l2popup_charset, chartablename[cp]);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "×:%s", lang->editor_l2popup_flicker);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "□:%s", lang->editor_l2popup_tabmode);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "R2:%s [%s]", lang->editor_l2popup_wordwrap, onoff[wordwrap&1]);
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
int txtdraw(unsigned char *buffer, unsigned int size, int charcode)
{
	int cp,i,j,w,x,c,siz,ret=0;
	unsigned char temp[MAX_COLS];

	// テキストの文字コード
	if (charcode == TXT_AUTO) 
		cp = charset;
	else
		cp = charcode;
	
	// 表示文字列初期化
	displine[0] = 0;
	
	// SJISに変換
	if (cp == TXT_EUCJP)
		ret=euctosjis2(temp, buffer, MAX_COLS, size);
	else if (cp == TXT_JIS)
		ret=jistosjis2(temp, buffer, MAX_COLS, size);
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
		c=temp[i];
		if (c == 9) {
			if (tabmode > 0) {
				w = tabmode - (j % tabmode);
				displine[j] = 9;
				for (x=j+1; x<j+w; x++) {
					displine[x] = 32;
				}
				j+=w;
			}
		} else if (c > 0) {
			displine[j++] = c;
		}
		displine[j] = 0;
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

int jistosjis2(unsigned char *dist, unsigned char *src, unsigned int limit, unsigned int size)
{
	int i,j=0,b=0,m=0;
	int x,y,z,u,t1,t2;
	unsigned char *c, *d;

	c = src; d = dist;
	
	d[j] = 0;
	for(i=0;i<size;i++) {
		x=i;y=x+1;z=y+1;
		if (y>=size) y=size-1;
		if (z>=size) z=size-1;
		// 4バイトのエスケープシーケンスには未対応
		if ((c[x] == 0x1B) && ((c[y] == 0x24) || (c[y] == 0x26)) && ((c[z] == 0x40) || (c[z] == 0x42))) {
			// ^[$Bなど 漢字
			i+=2; m=3;
		} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x42)) {
			// ^[(B ASCII(7bit)
			i+=2; m=0;
		} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x4A)) {
			// ^[(J JISローマ字?(=ASCII+KANA=ANK?=8bit?)
			i+=2; m=1;
		} else if ((c[x] == 0x1B) && (c[y] == 0x28) && (c[z] == 0x49)) {
			// ^[(I 半角カタカナ(7bit?)
			i+=2; m=2;
		} else if (c[x] == 0x0E) {
			b=m; m=2;
		} else if (c[x] == 0x0F) {
			m=b; b=0;
		} else if (m == 1) {
			// JISローマ字(ANK?)
			d[j++] = c[x];
		} else if (m == 2) {
			// 半角カタカナ
			d[j++] = 0x80 | c[x];
		} else if (m == 3) {
			// 漢字
			if (j<limit-2) {
				u=94*(c[x]-0x21)+(c[y]-0x21);
				t1=(u/188)+0x81;t2=(u%188)+0x40;
				if (t1 >= 0xA0) t1+=0x40;
				if (t2 >= 0x7F) t2++;
				d[j++]=t1;d[j++]=t2;
			}
			i++;
		} else if ((c[x] < 0x80) && (c[x] != 27)) {
			d[j++] = c[x];
		}
		// 改行の場所で漢字やカタカナだったらマイナスポイントにしようか
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
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能,b1=0:通常,=1:スライドモード)
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

int formatcheck(unsigned char *c, unsigned int size)
{
	int type=FT_TXT;
	if ((size >= 32) && (c[0] == 0xFF) && (c[1] == 0xD8) && (c[2] == 0xFF)) {
		type = FT_JPG;
	} else if ((size >= 16) && (c[0] == 0x89) && (c[1] == 0x50) && (c[2] == 0x4E) && (c[3] == 0x47) && (c[4] == 0x0D) && (c[5] == 0x0A)) {
		type = FT_PNG;
	} else if ((size >= 8) && (c[0] == 0x47) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x38) && (c[4] == 0x39) && (c[5] == 0x61)) {
		type = FT_GIF;
	} else if ((size >= 14) && (c[0] == 0x42) && (c[1] == 0x4D) && (c[6] == 0x00) && (c[7] == 0x00) && (c[8] == 0x00) && (c[9] == 0x00)) {
		type = FT_BMP;
	} else if ((size >= 20) && (c[0] == 0) && (c[1] == 0) && (c[2] == 1) && (c[3] == 0) && (c[4] < 16) && (c[5] == 0) && (c[6] == 0) && (c[7] == 0) && (c[9] == 0) && (c[10] == 0) && (c[11] == 0)) {
		type = FT_P2T;
	} else if ((size >= 256) && (c[0] == 0x53) && (c[1] == 0x43) && (c[2] > 0x10) && (c[2] < 0x20) && (c[3] > 0) && (c[3] < 16)) {
		type = FT_PS1;
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
	} else if ((size >= 24) && (c[0] == 0x4D) && (c[1] == 0x54) && (c[2] == 0x68) && (c[3] == 0x64)) {
		type = FT_MID;
	} else if ((size >= 44) && (c[0] == 0x52) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x46) && (c[8] == 0x57) && (c[9] == 0x41) && (c[10] == 0x56) && (c[11] == 0x45)) {
		type = FT_PCM;
	} else if ((size >= 64) && ( ((c[0] == 0xFF) && ((c[1] & 0xE0) == 0xE0) && (c[4] == 0x00) && (c[5] == 0x00)) || ((c[0] == 0x49) && (c[1] == 0x44) && (c[2] == 0x33)) )) {
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
	} else if ((size >= 80) && (c[0] == 0x00) && ((c[1] == 0x02) || (c[1] == 0x03)) && (c[2]+256*c[3]+65536*c[4] == size) && (c[5] == 0x00)) {
		type = FT_FNT;
	} else if ((size >= 32) && (c[0] == 0x4D) && (c[1] == 0x5A)) {
		type = FT_EXE;
	}
	// BIN/ELF/JPG/PNG/GIF/BMP/MP3/AAC/AC3/PCM/TXT/ZIP/RAR/LZH/TEK/GZ/7Z
	// todo: OGG SAR PSU PS1(SC/MC)
	return type;
}
int formatcheckfile(char *file)
{
	int fd;
	char *p;
	unsigned char buffer[512];
	unsigned int size;
	char fullpath[MAX_PATH], tmp[MAX_PATH];
	
	if (!strncmp(file, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &file[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		//fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			//fileXioUmount("pfs0:");
			return -1;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		if (size > 512) size = 512;
		fileXioLseek(fd, 0, SEEK_SET);
		fileXioRead(fd, buffer, size);
		fileXioClose(fd);
		//fileXioUmount("pfs0:");
	} else {
		strcpy(fullpath, file);
		fd = fioOpen(fullpath, O_RDONLY);
		if (fd<0)
			return -1;
		size = fioLseek(fd, 0, SEEK_END);
		if (size > 512) size = 512;
		fioLseek(fd, 0, SEEK_SET);
		fioRead(fd, buffer, size);
		fioClose(fd);
	}
	return formatcheck(buffer, size);
};

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
	int dsize=size,ret=-11,dither;
	int *env=NULL;
	unsigned char *buffer=NULL, *decoded=NULL;
	tvmode = setting->tvmode;
	if (gsregs[tvmode].loaded != 1) tvmode = ITO_VMODE_AUTO-1;
	bpp = setting->screen_depth[tvmode] > 0 ? (setting->screen_depth[tvmode]-1):4-gsregs[tvmode].psm;
	dither = setting->screen_dither[tvmode] > 0 ? (setting->screen_dither[tvmode]-1):gsregs[tvmode].dither;
	if ((bpp == 2) && dither) bpp = 4;
	if (setting->txt_autodecode && ((dsize = tek_getsize(c)) >= 0)) {
		decoded = (unsigned char*)malloc(dsize);
		if (decoded != NULL) {
			if (tek_decomp(c, decoded, size)<0) {
				free(decoded);
				decoded=NULL;
				printf("viewer: tek auto decode failed\n");
			} else {
				printf("viewer: decoded tek compression\n");
			}
		}
	}
	if (decoded == NULL) {
		dsize = size;
		decoded = c;
	}
	drawMsg(lang->gen_decoding);
	type = formatcheck(decoded, dsize);
	// 簡易版ファイル形式判別
	for (i=0;i<FT_TYPES;i++) {
		if (ft_type[i] == 0) {
			printf("viewer: filetype: BINARY\n");
			break;
		} else if (ft_type[i] == type) {
			printf("viewer: filetype: %s\n", ft_char[i]);
			break;
		}
	}
	
	if (!(mode & 2) && (paddata & PAD_R2)) {
		// テキストビューアで開く
		ret = txtedit(mode | 0x0010, file, decoded, dsize);
	} else if (!(mode & 2) && (paddata & PAD_R1)) {
		// バイナリビューアで開く
		ret = txtedit(mode | 0x0020, file, decoded, dsize);
	} else {
		// 普通に開く
		alphablend = FALSE;
		switch(type){
			case FT_JPG:
			{
				env = malloc(16384*sizeof(int));
				if (env == NULL) {
					ret=-2;
					break;
				}
				env[0] = 0; env[1] = 0;
				i = info_JPEG(env, info, dsize, decoded);
				printf("viewer: info_JPEG returned: %d (env: %d,%d)\n", i, env[0], env[1]);
				printf("viewer: info data: %d,%d,%dx%d\n", info[0], info[1], info[2], info[3]);
				if ((info[0] != 0x0002) || (info[2] * info[3] == 0)) {
					//free(env);
					break;
				}
				buffer = malloc(info[2] * info[3] * bpp);
				if (buffer == NULL) {
					if ((bpp <= 2) || ((bpp>2) && ((bpp=2)==2) && ((buffer = malloc(info[2] * info[3] * bpp))==NULL))) {
						//free(env);
						ret=-2;
						break;
					}
				}
				i = decode0_JPEG(env, dsize, decoded, bpp, buffer, 0);
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
	*/			ret = imgview(mode, file, buffer, info[2], info[3], bpp);
				//free(buffer);
				//free(env);
				//if ((decoded != NULL) && (decoded != c)) free(decoded);
				//return i;
				break;
			}
			case FT_BMP:
			{
				i = info_BMP(info, decoded, dsize);
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
						//if ((decoded != NULL) && (decoded != c)) free(decoded);
						//return -2;
						ret = -2;
						break;
					}
				}
				i = decode_BMP(buffer, decoded, dsize, bpp);
				printf("viewer: decode_BMP(%dbpp) returned: %d\n", bpp*8, i);
				ret = imgview(mode, file, buffer, info[2], info[3], bpp);
				//free(buffer);
				//if ((decoded != NULL) && (decoded != c)) free(decoded);
				//return i;
				break;
			}
			case FT_GIF:
			{
				int bpp0;
				alphablend = TRUE;
				i = info_GIF(info, decoded, dsize);
				//bpp = bpp0 = 1;
				bpp0 = bpp;
				printf("viewer: info_GIF returned: %d\n", i);
				printf("viewer: info data: %d,%dx%d(%dbpp),%d image(s)\n", info[0], info[2], info[3], info[1], info[4]);
				//i = 24576 +16 + info[2] * info[3] +16;
				i = info[2] * info[3] +16;
				if ((info[0] != 0x0008) || (info[2] * info[3] == 0))
					break;
				if ((info[5] == dsize) && (info[4] > 1)) {
					buffer = malloc(info[2] * info[3] * info[4] * bpp +i);
					if (!buffer && (bpp > 2)) {
						bpp = bpp0 = 2;
						buffer = malloc(info[2] * info[3] * info[4] * bpp +i);
					}
					if (!buffer && (bpp > 1)) {
						bpp = bpp0 = 1;
						buffer = malloc(info[2] * info[3] * info[4] * bpp +i);
					}
					if (buffer) {
						unsigned char *ttttemp;
						bpp0 = bpp | (0x0100 * info[4]);
						ttttemp = realloc(buffer, info[2] * info[3] * info[4] * bpp);
						if (ttttemp) buffer = ttttemp;
					} else {
						unsigned char tmps[64];
						sprintf(tmps, " [%d/%d]", info[4], info[4]);
						strcat(file, tmps);
					}
				}// else buffer = NULL;
				if (buffer == NULL)
					buffer = malloc(info[2] * info[3] * bpp);
				if (buffer == NULL) {
					//if ((decoded != NULL) && (decoded != c)) free(decoded);
					//return -2;
					ret = -2;
					break;
				}
				i = decode_GIF(buffer, decoded, dsize, bpp0);
				printf("viewer: decode_GIF(%dbpp) returned: %d\n", bpp*8, i);
			//	if (info[5] != dsize) info_GIF(info, decoded, dsize);
			//	if ((info[5] == dsize) && (info[4] > 1)) {
			//		unsigned char tmps[64];
			//		sprintf(tmps, " [%d/%d]", info[4], info[4]);
			//		strcat(file, tmps);
			//	}
				ret = imgview(mode, file, buffer, info[2], info[3], bpp0);
				//free(buffer);
				//if ((decoded != NULL) && (decoded != c)) free(decoded);
				//return i;
				break;
			}
			case FT_P2T:
			{
				i = info_PS2ICO(info, decoded, dsize);
				printf("viewer: info_PS2ICO returned: %d\n", i);
				printf("viewer: info data: %d,%dx%d(%dbpp)\n", info[0], info[2], info[3], info[1]);
				if ((info[0] != 0x7009) || (info[2] * info[3] == 0))
					break;
				buffer = malloc(info[2] * info[3] * 2);
				if (buffer == NULL) {
					ret = -2;
					break;
				}
				i = decode_PS2ICO(buffer, decoded, dsize, 2);
				printf("viewer: decode_PS2ICO(%dbpp) returned: %d\n", 16, i);
				ret = imgview(mode, file, buffer, info[2], info[3], 2);
				break;
			}
			case FT_PS1:
			{
				i = info_PS1ICO(info, decoded, dsize);
				bpp = 1;
				printf("viewer: info_PS1ICO returned: %d\n", i);
				printf("viewer: info data: %d,%dx%d(%dbpp),%d images\n", info[0], info[2], info[3], info[1], info[4]);
				if ((info[0] != 0x700A) || (info[2] * info[3] * info[4] == 0))
					break;
				buffer = (char*)malloc(info[2] * info[3] * info[4]);
				if (buffer == NULL) {
					ret = -2;
					break;
				}
				if (info[4] > 1) bpp |= info[4] << 8;
				i = decode_PS1ICO(buffer, decoded, dsize, bpp);
				printf("viewer: decode_PS1ICO(%dbpp) returned: %d\n", 8, i);
				ret = imgview(mode, file, buffer, info[2], info[3], bpp);
				break;
			}
			case FT_FNT:
			{	// フォントビューアを起動
				//ret = fntview(mode, file, buffer, dsize);
				break;
			}
			case FT_MP3:
			case FT_PCM:
			{
				break;
			}
		}
	}
	if (env != NULL) free(env);
	if (buffer != NULL) free(buffer);
	if ((mode & 2) && (ret == -11)) ret = -1;
	if (ret == -11)
		ret = txtedit(mode, file, decoded, dsize);
	if ((decoded != NULL) && (decoded != c)) free(decoded);
	return ret;
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
		c = buffer + (y*w+x)*(bpp & 15);
		switch(bpp) {
			case 1: // 256色
			case 17:
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
			case 19:
			case 4: // TrueColor
			{
			/*	if (c[3] != 0) {
					//printf("viewer: memory error at %08X\n", (int) &c[0]);
					if (!memoryerror) memoryerror = (int) &c[0];
					return 0;
				}
			*/	return ((uint64) c[0] << 16)|((uint64) c[1] << 8)|(uint64) c[2];
			}
			case 18: // HighColor
			{
				i = (short *) c;
				return ((uint64) (i[0] & 0x7C00) << 9)|((uint64) (i[0] & 0x03E0) << 6)|((uint64) (i[0] & 0x001F) << 3)|((uint64) (i[0] & 0x8000) << 16);
			}
			case 20: // TrueColor
			{
			/*	if (c[3] != 0) {
					//printf("viewer: memory error at %08X\n", (int) &c[0]);
					if (!memoryerror) memoryerror = (int) &c[0];
					return 0;
				}
			*/	return ((uint64) c[2] << 16)|((uint64) c[1] << 8)|(uint64) c[0] | ((uint64) c[3] << 24);
			}
		}
	}
	return 0;
}
void itoPoint2c(uint64 color0, uint16 x0, uint16 y0, uint64 color1, uint16 x1, uint16 y1);
int imgview(int mode, char *file, unsigned char *buffer0, int w, int h, int bppf)
{
	int redraw=fieldbuffers,redi=0,bpp,bppb,ani,q,cls=0;
	//int sl=0,st=0,sw=1,sh=1; // カーソル位置
	int tl,tt,tw,th,x,y,ff;
	int vl,vt,vw,vh; // ビューポート(描画可能範囲)
	int dl,dt,dw,dh; // 描画範囲用
	int k,gx,gy,gz;
	int tmpx[2048], tmpy[1088];
	static int bilinear=0;
	//uint64 color;
	//unsigned char *d;
	char msg0[MAX_PATH], msg1[MAX_PATH];
	unsigned char *buffer=buffer0;
	double mx,my,dx,dy,gw,gh;	// 倍率
	strcpy(msg0, file);
	ani = bppf >> 8;
	bpp = bppf & 0x00ff;
	bppb = bpp | (alphablend << 4);
	q = ani -1;
	if (ani) {
		buffer = buffer0 + w * h * bpp * q;
		if (alphablend) {
			if (!(pget(buffer, 0, 0, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, w-1, 0, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, 0, h-1, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, w-1, h-1, w, h, bppb) & 0xFF000000ull)
			) cls = 1; else cls = 0;
		}
	}
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
			if (new_pad & PAD_SQUARE) {
				redraw = fieldbuffers;
				resizer = (resizer +1) % 2;
			}
			if (!ffmode && (new_pad & PAD_R2)) {
				redraw = fieldbuffers;
				bilinear ^= 1;
			}
			if (mode & 2) {
				if (new_pad & PAD_L1) return 1;
				if (new_pad & PAD_R1) return 2;
				if (new_pad & PAD_UP) return 1;
				if (new_pad & PAD_DOWN) return 2;
			}
			if (ani) {
				if (new_pad & PAD_LEFT) {
					if (q > 0) q--; else q = ani -1;
					buffer = buffer0 + w * h * bpp * q;
					redraw = fieldbuffers;
					if (!cls && ((framebuffers == 1) || (fieldbuffers == 2))) redi = fieldbuffers;
				}
				if (new_pad & PAD_RIGHT) {
					if (q < ani -1) q++; else q = 0;
					buffer = buffer0 + w * h * bpp * q;
					redraw = fieldbuffers;
					if (!cls && ((framebuffers == 1) || (fieldbuffers == 2))) redi = fieldbuffers;
				}
			}
		}
	
		if (redraw) {
			if (!redi)	clrScr(setting->color[COLOR_BACKGROUND]);
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
			}
			// リサイズ後のサイズを求める
			if (resizer) {
				if (!ffmode && bilinear)
					ff = 1;
				else
					ff = 0;
				// イメージ1ドット進むごとに画面はbx,byピクセル進む(=拡大率)
				mx = (double) vw * gx / (w -ff);
				my = (double) vh * gy * (ffmode+1) / (h -ff);
				gw = (double) vw / w;
				gh = (double) vh * (ffmode+1) / h;
				// 画面1ピクセル進むごとにイメージはbx,byドット進む(=縮小率)
				//dx = (double) w / vw;
				//dy = (double) h / vh;
				if (mx < my) my = mx; else mx = my;
				if (gw < gh) gh = gw; else gw = gh;
				dx = 1/mx*gx;
				dy = 1/my*gy;
				//printf("screen: %dx%d, ps: %.3fx%.3f, %.3fx%.3f\n", vw, vh, dx, dy, mx, my);
				tw = w * mx / gx; th = h * my / (ffmode+1) / gy;
			} else {
				dx = dy = mx = my = gw = gh = 1;
				tw = w; th = h;
				if (ffmode) {
					th >>= 1;
				}
			}
			tl = (vw - tw) / 2;
			tt = (vh - th) / 2;
			if (tw > vw) dw = vw; else dw = tw;
			if (th > vh) dh = vh; else dh = th;
			if (tl < 0) dl = vl; else dl = tl+vl;
			if (tt < 0) dt = vt; else dt = tt+vt;
			if (tw > vw) tw = vw;
			if (th > vh) th = vh;
			if (!fullscreen) {
				if (mode & 2) {
					if (ani) 
						sprintf(msg1, lang->editor_image_help4, w, h, tw, th);
					else
						sprintf(msg1, lang->editor_image_help2, w, h, tw, th);
				} else {
					if (ani)
						sprintf(msg1, lang->editor_image_help3, w, h, tw, th);
					else
						sprintf(msg1, lang->editor_image_help, w, h, tw, th);
				}
				if (ani) sprintf(msg0, "%s [%d/%d]", file, q+1, ani);
				setScrTmp(msg0, msg1);
			}
			
			// イメージの描画
			// (テクスチャ読み込みをしてテクスチャを貼った方が良いかも知れない、特に極小サイズのイメージの場合)
			if (alphablend)	{itoPrimAlphaBlending(TRUE);}
			if (ffmode) {
				k=itoGetActiveFrameBuffer();
				if (gw > 1.2) {
					k^=1;
					// 座標のキャッシュ - 高速化を期待してみる
					gw = mx/gx;	gh = my/gy;
					for (x=0;x<=w;x++)	tmpx[x] = x*gw+dl;
					for (y=0;y<=h;y++)	tmpy[y] = (y*gh+k)/2+dt;
					// 描画
					for (y=0;y<h;y++)
						for (x=0;x<w;)
							itoSprite(pget(buffer,x,y,w,h,bppb), tmpx[x++], tmpy[y], tmpx[x], tmpy[y+1], 0);
				} else if (gw == 1) {
					for (y=0;y<dh;y++)
						for (x=0;x<dw;x+=2)
							itoPoint2c(pget(buffer,x,y*2+k,w,h,bppb), x+dl, y+dt, pget(buffer,x+1,y*2+k,w,h,bppb), x+dl+1, y+dt);
				} else {
					// 座標のキャッシュ - 高速化を期待してみる
					for (x=0;x<dw;x++)	tmpx[x] = x*dx;
					for (y=0;y<dh;y++)	tmpy[y] = (y*2+k)*dy;
					// 描画
					for (y=0;y<dh;y++)
						for (x=0;x<dw;x++)
							itoPoint(pget(buffer,tmpx[x],tmpy[y],w,h,bppb), x+dl, y+dt, 0);
					//	itoPoint(pget(buffer,x*dx,(y*2+k)*dy,w,h,bpp), x+dl, y+dt, 0);
				}
			} else {
				//printf("window: (%d,%d)-(%d,%d) %dx%d\n", dl, dt, dl+dw-1, dt+dh-1, dw, dh);
				//printf("floats: m=%6.3lf,%6.3lf d=%6.3lf,%6.3lf g=%6.3lf,%6.3lf mg=%d,%d\n", mx, my, dx, dy, gw, gh, gx, gy);
				if (gw > 1.2) {
					// 座標のキャッシュ - 高速化を期待してみる
					gw = mx/gx;	gh = my/gy;
					for (x=0;x<=w;x++)	tmpx[x] = x*gw+dl;
					for (y=0;y<=h;y++)	tmpy[y] = y*gh+dt;
					// 描画
					if (!bilinear) {
						for (y=0;y<h;y++)
							for (x=0;x<w;)
								itoSprite(pget(buffer,x,y,w,h,bppb), tmpx[x], tmpy[y], tmpx[++x], tmpy[y+1], 0);
						//	itoSprite(pget(buffer,x,y,w,h,bpp), x*gw+dl, y*gh+dt, (x+1)*gw+dl, (y+1)*gh+dt, 0);
					} else {
						// バイリニアテスト
						itoPrimShade( ITO_PRIM_SHADE_GOURAUD );
						for (y=0;y<h-1;y++) {
							itoTriangleStrip(	pget(buffer,0,y  ,w,h,bppb), tmpx[0], tmpy[y]  , 0,
												pget(buffer,0,y+1,w,h,bppb), tmpx[0], tmpy[y+1], 0,
												pget(buffer,1,y  ,w,h,bppb), tmpx[1], tmpy[y]  , 0);
							itoAddVertex(		pget(buffer,1,y+1,w,h,bppb), tmpx[1], tmpy[y+1], 0);
							//void itoAddVertex2(uint64 color1, uint16 x1, uint16 y1, uint64 color2, uint16 x2, uint16 y2);
							for (x=2;x<w;x++) //{
								itoAddVertex2(	pget(buffer,x,y  ,w,h,bppb), tmpx[x], tmpy[y  ],
												pget(buffer,x,y+1,w,h,bppb), tmpx[x], tmpy[y+1]);
							//	itoAddVertex(	pget(buffer,x,y  ,w,h,bpp), tmpx[x], tmpy[y  ], 0);
							//	itoAddVertex(	pget(buffer,x,y+1,w,h,bpp), tmpx[x], tmpy[y+1], 0);
							//}
							itoEndVertex();
							itoGsFinish();
						}
						// 結果: ピクセルが ↓ のようにスムージングされてしまう
						//                 ／￣|
						//                |＿／
						itoPrimShade( ITO_PRIM_SHADE_FLAT );
					}
				} else if (gw == 1) {
					for (y=0;y<dh;y++)
						for (x=0;x<dw;x+=2)
							itoPoint2c(pget(buffer,x,y,w,h,bppb), x+dl, y+dt, pget(buffer,x+1,y,w,h,bppb), x+dl+1, y+dt);
				} else {
					// 座標のキャッシュ - 高速化を期待してみる
					for (x=0;x<dw;x++)	tmpx[x] = x*dx;
					for (y=0;y<dh;y++)	tmpy[y] = y*dy;
					// 描画
					for (y=0;y<dh;y++)
						for (x=0;x<dw;x++)
							itoPoint(pget(buffer,tmpx[x],tmpy[y],w,h,bppb), x+dl, y+dt, 0);
				}
			}
			if (alphablend)	itoPrimAlphaBlending(FALSE);
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
			if (redi) redi--;
			redraw--;
		} else {
			itoVSync();
		}
	}
	return 0;
}

int set_viewerconfig(int linedisp, int tabspaces, int chardisp, int screenmode, int textwrap, int drawtype)
{
	linenum = linedisp;
	tabmode = tabspaces;
	tabdisp = nldisp = chardisp;
	fullscreen = screenmode;
	wordwrap = textwrap;
	resizer = drawtype;
	return 0;
}

int fntview(int mode, char *file, unsigned char *buffer, unsigned int size)
{
	// フォントビューア
	//	*buffer が対応しているフォント形式なら表示する (単一のファイルのみ)
	// 対応形式: FONTX2形式(半角/全角), MS-Win2.0(uLaunchELF互換)形式(半角かつビットマップフォントのみ)
	int ret=0,redraw=fieldbuffers, ref=fieldbuffers;
	char msg0[MAX_PATH], msg1[MAX_PATH];
	int cx,cy,ox,oy,cp,op,ty;
	
	cx=cy=cp=ox=oy=0; op=1;
	strcpy(msg0, file);
	while(1) {
		waitPadReady(0, 0);
		if(readpad()){
			if (new_pad & PAD_SELECT) break;
			if (new_pad & PAD_TRIANGLE) break;
			if (new_pad & PAD_UP) cy--;
			if (new_pad & PAD_DOWN) cy++;
			if (new_pad & PAD_LEFT) cx--;
			if (new_pad & PAD_RIGHT) cx++;
			
		}
		if ((cx != ox) || (cy != oy) || (cp != op)) redraw = fieldbuffers;
		if (ref) {
			clrScr(setting->color[COLOR_BACKGROUND]);
			setScrTmp(msg0, msg1);
			ref--;
		}
		if (redraw) {
			
			// LbFn v0.70.15                                                             
			// mass:/tmp.fnt                                                             
			// ─────────────────────────────────────
			//   種類: FONTX2(SBCS/ASCII), 8x16                                          
			//   種類: FONTX2(DBCS/KANJI), 16x16, 11280 (4435) 字形                      
			//   名称: KROM                                                              
			//   文字一覧:                                                               
			//   ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐  
			//   │  │０│１│２│３│４│５│６│７│８│９│Ａ│Ｂ│Ｃ│Ｄ│Ｅ│Ｆ│  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │０│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │１│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │２│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │３│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │４│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │５│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │６│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   ├─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┼─┤  
			//   │７│  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  │  
			//   └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘  
			// ─────────────────────────────────────
			//   ○/×:戻る                                                              
			
			for (ty=0; ty<16; ty++) {
				//itoLine(setting->color[COLOR_FRAME], tx*16, ty*24+SCREEN_TOP, tx*16+16, ty*24+24+SCREEN_TOP, 0);
			}
			drawScr();
			ox = cx; oy = cy; op = cp;
			redraw--;
		} else {
			itoVSync();
		}
	}
	return ret;
}

