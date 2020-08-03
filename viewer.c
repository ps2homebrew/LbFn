#include "libgs.h"
#include "launchelf.h"
#include <audsrv.h>
#include "audsrv2.h"
#define	MAX_LINES	32768
#define	MAX_COLS	16384
#define	MAX_UNICODE	0x010000
//#define	waitPadReady(...)
//#define	BBSVIEWER

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
enum{
	TTP_TEXT,
	TTP_BINARY,
	TTP_BBS,
};
enum{
	BBS_TEMPSIZE = 65536,
};
//static unsigned char *clipbuffer=NULL;
//static unsigned char editline[2][MAX_COLS];
static unsigned char displine[MAX_COLS];
static int redraw=2, charset=0, fullscreen=0, resizer=1;
int linenum=0, defaultcharset=0, tabmode=8, tabdisp=0, nldisp=0, aniauto=1, imgpos=4;
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
extern unsigned char sndview_filename[MAX_PATH], sndview_format[32], sndview_totaltime[16], *sndview_head, *sndview_body;
extern int nobgmpos;
extern unsigned int sndview_size;
int scrnsize=0; char *scrnbuff=NULL;
//*/
extern unsigned char monafontwidth[11536];
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
int decode_PNG(char *dist, char *src, int size, int bpp);
int info_PNG(int *info, char *buff, int size);

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
int txt_detect(unsigned char *c, unsigned int size, int *type)
{
	int w,x,y,z,u,charset,cp,co;
	int ascii=0,jis=0,sjis=0,eucjp=0,utf8=0,text=0,binary=0,bbs=0;
	int jisb=0,jisf=0,jism=0,sjisf=0,eucjpf=0,utf8f=0,bbsf=0;
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
		if ((c[x] == 0x3C) && (c[y] == 0x3E)) bbsf++;
		else if (c[x] == 10) {
			if (bbsf != 4) bbs -= 5; else bbs += 30;
			bbsf = 0;
		}
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
	co = TTP_TEXT; cp = text;
	if (binary > cp)	{co = TTP_BINARY; cp = binary;}
	if (bbs > cp)		{co = TTP_BBS; cp = bbs;}
	if (type != NULL)	{*type = co;}
	printf("textdetect:%d txt=%d,bin=%d,2ch=%d,\n	asc=%d,sjis=%d,euc=%d,jis=%d,utf8=%d pts.\n", charset, text, binary, bbs, ascii, sjis, eucjp, jis, utf8);
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
	int i, j, type;
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
	charset = txt_detect(c, size, &type);
	if (!(paddata & PAD_R2) || paddata & PAD_L1) {
		if (type == TTP_BBS || paddata & PAD_L1) {
			return bbsview(mode, file, c, size);
		}
	}
	cs = TXT_AUTO; cp=0;
	redraw=fieldbuffers;
	maxbytes=MAX_COLS-1;
	makectrlchars();
	
	while(1){
	//	waitPadReady(0, 0);
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
/*
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
//*/
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
//	printf("viewer: ucstableinit start\n");
	for (c = 0; c < MAX_UNICODE; c++)
		ucstable[c] = 0x3F;	// question mark
	// making ASCII <-> Unicode convert table
	for (c = 0; c < 128; c++)
		ucstable[c] = c;	// Unicode => ANK (it is init, override the next steps)
	// making ANK(HALF WIDTH KATAKANA) <-> Unicode convert table
	for (c = 1; c < 64; c++)
		ucstable[c + 0xFF60] = c + 0xA0;
	// making Japanese <-> Unicode convert table
	for (c = 0; c < 11280; c++) {
		u = sjistable[c];
		if ((u > 0x10FFFF) || ((u >= 0xD800) && (u <= 0xDFFF)) || (u == 0)) u = 0x3F;
		if (u >= MAX_UNICODE) u = 0x3F;
		sjistable[c] = u;	// jis codepoint number -> Unicode
		if ((u != 0x3F) && (ucstable[u] == 0x3F))
			ucstable[u] = c + 256;	// Unicode -> codepoint number + 256
	}
	ucstabled = !0;
//	printf("viewer: ucstableinited: %d\n", ucstabled);
	return !0;
}

////////////////////////////////
// 適切な形式で表示(ファイル指定版)
//	in:	mode	編集モード(b0=0:表示のみ,=1:編集可能,b1=0:通常,=1:スライドモード)
//  	file	ファイル名
int viewer_file(int mode, char *file)
{
	int fd,ret;
	unsigned char *buffer;
	unsigned int size;
	
	if (strcmp(file, sndview_filename)==0) {
		buffer = sndview_head;
		size = sndview_size;
	} else {
		printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		fd = nopen(file, O_RDONLY);
		if (fd < 0)
			return -1;
		size = nseek(fd, 0, SEEK_END);
		printf("viewer: size: %d\n", size);
		nseek(fd, 0, SEEK_SET);
		buffer = (char*)malloc(size);
		if (buffer != NULL) {
			drawMsg(lang->gen_loading);
			nread(fd, buffer, size);
		}
		nclose(fd);
	}
	if (buffer == NULL)
		return -2;
	ret = viewer(mode, file, buffer, size);
	if (sndview_head != buffer)
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
	} else if ((size >= 8) && (c[0] == 0x47) && (c[1] == 0x49) && (c[2] == 0x46) && (c[3] == 0x38) && ((c[4] == 0x37) || (c[4] == 0x39))) {
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
	} else if ((size >= 32) && (c[28] == 0x3C) && !c[29] && !c[30] && !c[31] && (c[12] | c[13] | c[14] | c[15]) && (c[24] | c[25])) {
		type = FT_PCM;
	//} else if ((size >= 64) && ( ((c[0] == 0xFF) && ((c[1] & 0xE0) == 0xE0) && (c[4] == 0x00) && (c[5] == 0x00)) || ((c[0] == 0x49) && (c[1] == 0x44) && (c[2] == 0x33)) )) {
	} else if ((size >= 64) && ( ((c[0] == 0xFF) && (c[1] >= 0xE0) && (c[1] & 0x06) && ((c[2] & 0xF0)!=0xF0) && ((c[2] & 0x0C)!=0x0C)) || ((c[0] == 0x49) && (c[1] == 0x44) && (c[2] == 0x33)) || ((c[0] == 0x46) && (c[1] == 0x4C) && (c[2] == 0x56) && (c[3] == 0x01)) )) {
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
	unsigned char buffer[512];
	unsigned int size;
	
	fd = nopen(file, O_RDONLY);
	if (fd < 0)
		return -1;
	size = nseek(fd, 0, SEEK_END);
	if (size > 512) size = 512;
	nseek(fd, 0, SEEK_SET);
	nread(fd, buffer, size);
	nclose(fd);
	return formatcheck(buffer, size);
};
int is_psu(unsigned char *buff, unsigned int size) 
{
	int s,i,n;
	int tmp[] = {0x10, 8, 0x20, 32, 0x60, 416, 0, 0};
	if (!buff[0x40] || !(buff[4]+buff[5])) return 0;
	for(s=0,n=0;tmp[s];s+=2){
		for(i=tmp[s];i<tmp[s]+tmp[s+1];i++) {
			if (buff[i]) n++;
		}
	}
	return !n;
}
int imagetypes[] = {FT_JPG,FT_BMP,FT_GIF,FT_PNG,FT_P2T,FT_PS1,FT_ICO,-1};
int audiotypes[] = {FT_PCM,FT_MP3,FT_AC3,FT_AAC,-1};
int videotypes[] = {FT_MPG,FT_AVI,FT_MP4,-1};
int is_image(unsigned char *buff, unsigned int size) {
	int type,i;
	type = formatcheck(buff, size);
	for(i=0;imagetypes[i]>=0;i++) if (imagetypes[i] == type) return 1;
	return 0;
}
int is_image_file(char *file) {
	int type,i;
	type = formatcheckfile(file);
	for(i=0;imagetypes[i]>=0;i++) if (imagetypes[i] == type) return 1;
	return 0;
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
	} else if (setting->txt_autodecode && ((dsize = gz_getsize(c, size)) >= 0)) {
		decoded = (unsigned char*)malloc(dsize);
		if (decoded != NULL) {
			if (gzdecode(decoded, dsize, c, size)<0) {
				free(decoded);
				decoded=NULL;
				printf("viewer: gzip auto decode failed\n");
			} else {
				printf("viewer: decoded gzip compression\n");
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
				int bpp0,b;
				//	alphablend = TRUE;
				if (bpp>2 && dither) bpp = 2;
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
					b = info[4] * sizeof(int) + 4;
					buffer = malloc(info[2] * info[3] * info[4] * bpp +i +b);
					if (!buffer && (bpp > 2)) {
						bpp = bpp0 = 2;
						buffer = malloc(info[2] * info[3] * info[4] * bpp +i +b);
					}
					if (!buffer && (bpp > 1)) {
						bpp = bpp0 = 1;
						buffer = malloc(info[2] * info[3] * info[4] * bpp +i +b);
					}
					if (buffer) {
						unsigned char *ttttemp;
						bpp0 = bpp | (0x0100 * info[4]);
						ttttemp = realloc(buffer, info[2] * info[3] * info[4] * bpp +b);
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
				{
					int e=info[2]*info[3];
					if (bpp0 >> 8) e *= bpp0 >> 8;
					if (bpp > 1) 
						for(i=0;i<e;i+=bpp){
							if (!(buffer[i+bpp-1] & 0x80)) {
								alphablend = TRUE;
								break;
							}
						}
					else
						for(i=0;i<e;i++){
							if (!(activeclut[buffer[i]] & 0x80000000)) {
								alphablend = TRUE;
								break;
							}
						}
				}
			//	if (info[5] != dsize) info_GIF(info, decoded, dsize);
			//	if ((info[5] == dsize) && (info[4] > 1)) {
			//		unsigned char tmps[64];
			//		sprintf(tmps, " [%d/%d]", info[4], info[4]);
			//		strcat(file, tmps);
			//	}
				ret = imgview(mode|(8*((bpp0>>8)!=0)), file, buffer, info[2], info[3], bpp0);
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
				buffer = (char*)malloc(info[2] * info[3] * info[4] + info[4] * sizeof(int));
				if (buffer == NULL) {
					ret = -2;
					break;
				}
				if (info[4] > 1) bpp |= info[4] << 8;
				i = decode_PS1ICO(buffer, decoded, dsize, bpp);
				printf("viewer: decode_PS1ICO(%dbpp) returned: %d\n", 8, i);
				ret = imgview(mode|8, file, buffer, info[2], info[3], bpp);
				break;
			}
			case FT_PNG:
			{
				int bpp0,b=0;
				i = info_PNG(info, decoded, dsize);
				if (info[5] & 1) {
					bpp = 1;	// 3	(clut)
				} else if (info[5] & 4) {
					bpp = bpp;	// 4,6	(alpha)
				} else if (info[5] & 2) {
					bpp = bpp;	// 2	(color)
				//	if (bpp > ((info[1] +7)>>3))
				//		bpp = (info[1] +7)>>3;
				} else {
					bpp = 1;	// 0	(gray)
				}
				printf("viewer: info_PNG returned: %d\n", i);
				printf("viewer: info_data: %d, %dx%d (%dbpp)\n", info[0], info[2], info[3], info[1]);
				if ((info[0] != 0x000b) || (info[2] * info[3] == 0))
					break;
				if (bpp == 3) bpp++;
				bpp0 = bpp;
				b = info[7] +32;
				if (info[6]) {
					i = info[2] * info[3] * bpp * 7;
					buffer = malloc(i +b);
					if (buffer != NULL) {
						bpp0 |= 0x700;
					} else {
						unsigned char tmps[64];
						sprintf(tmps, " [%d/%d]", 7, 7);
						strcat(file, tmps);
					}
				} else buffer = NULL;
				if (buffer == NULL) {
					i = info[2] * info[3] * bpp;
					buffer = malloc(i +b);
				}
				if (buffer == NULL) {
					if ((bpp <= 2) || ((bpp>2) && ((bpp=2)==2) && ((buffer = malloc((i=info[2] * info[3] * bpp)+b))==NULL))) {
						//if ((decoded != NULL) && (decoded != c)) free(decoded);
						//return -2;
						ret = -2;
						break;
					}
					bpp0 = bpp;
				}
				if (info[5] & 4) alphablend = TRUE;
				{
					unsigned char *ttttemp;
					ttttemp = realloc(buffer, i);
					if (ttttemp) buffer = ttttemp;
				}
				i = decode_PNG(buffer, decoded, dsize, bpp0);
				printf("viewer: decode_PNG(%dbpp) returned: %d\n", bpp*8, i);
				if (i < 0) {
					ret = i;
					break;
				}
				if (info[5] & 4) {
					int e=info[2]*info[3]*bpp;//,p=0;
					alphablend = FALSE;
					if (bpp0 >> 8) e *= bpp0 >> 8;
					if (bpp > 2) 
						for(i=0;i<e;i+=bpp){
							if (buffer[i+bpp-1] < 0x80) {
								alphablend = TRUE;
								//break;
				//				p++;
							}
						}
					else if (bpp > 1) 
						for(i=0;i<e;i+=bpp){
							if (!(buffer[i+bpp-1] & 0x80)) {
								alphablend = TRUE;
								//break;
				//				p++;
							}
						}
					else
						for(i=0;i<e;i++){
							if (((activeclut[buffer[i]] >> 24) & 255) < 0x80) {
								alphablend = TRUE;
								//break;
				//				p++;
							}
						}
				//	printf("viewer: alphablend: %d / %d ( %3d\% )\n", p, e/bpp, p * 100 / (e/bpp));
				}
				ret = imgview(mode, file, buffer, info[2], info[3], bpp0);
				//free(buffer);
				//if ((decoded != NULL) && (decoded != c)) free(decoded);
				//return i;
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
				ret = sndview(mode, file, decoded, dsize);
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
			case 20: // TrueColor
			{
			/*	if (c[3] != 0) {
					//printf("viewer: memory error at %08X\n", (int) &c[0]);
					if (!memoryerror) memoryerror = (int) &c[0];
					return 0;
				}
			*/	//return ((uint64) c[0] << 16)|((uint64) c[1] << 8)|(uint64) c[2];
			//	return ((uint64) c[2] << 16)|((uint64) c[1] << 8)|(uint64) c[0];
				return *(int *)c;
			}
			case 18: // HighColor
			{
				i = (short *) c;
				return ((uint64) (i[0] & 0x7C00) << 9)|((uint64) (i[0] & 0x03E0) << 6)|((uint64) (i[0] & 0x001F) << 3)|((uint64) (i[0] & 0x8000) << 16);
			}
			{
			/*	if (c[3] != 0) {
					//printf("viewer: memory error at %08X\n", (int) &c[0]);
					if (!memoryerror) memoryerror = (int) &c[0];
					return 0;
				}
			*/	//return ((uint64) c[2] << 16)|((uint64) c[1] << 8)|(uint64) c[0] | ((uint64) c[3] << 24);
			}
		}
	}
	return 0;
}
int imgview(int mode, char *file, unsigned char *buffer0, int w, int h, int bppf)
{	// イメージビューア
	// mode:	b1	=1:スライドモード
	//			b3	=1:アニメーションウェイト対応
	int redraw=fieldbuffers,redi=0,bpp,bppb,ani,oq,q,cls=0;
	//int sl=0,st=0,sw=1,sh=1; // カーソル位置
	int tl,tt,tw,th,x,y,ff,fm;
	int vl,vt,vw,vh; // ビューポート(描画可能範囲)
	int dl,dt,dw,dh; // 描画範囲用
	int k,gx,gy,gz;
	int tmpx[2048], tmpy[1088], *wait, owait, await;
	uint64 oldcount=0;
	static int bilinear=0;
	//uint64 color;
	//unsigned char *d;
	//	SAR			PAR
	//			NTSC	PAL
	//	 4:3	10:11	12:11	
	//	16:9	40:33	16:11	
	char msg0[MAX_PATH], msg1[MAX_PATH];
	unsigned char *buffer=NULL,*tmpbuf=NULL;
	double mx,my,dx,dy,gw,gh,pw;	// 倍率
	int cx,cy,ox,oy,cz,oz;
	int vmode,dither,tbb,oresizer=!resizer;
	vmode = setting->tvmode;
	if (!gsregs[vmode].loaded) vmode = (ITO_VMODE_AUTO)-1;
	dither=setting->screen_dither[vmode] > 0 ? (setting->screen_dither[vmode]-1):gsregs[vmode].dither;
	strcpy(msg0, file);
	ani = bppf >> 8;
	bpp = bppf & 0x00ff; tbb = bpp;
	bppb = bpp | (alphablend << 4);
	fm = ffmode != 0;
	q = ani -1;
	wait = NULL; owait = await = 0;
	if (ani) {
		if (mode & 8) {
			wait = (int*)((((int)&buffer0[w * h * bpp * ani]) +3) & ~3);
			for(oq=0;oq<ani;oq++){
				printf("viewer: delay[%3d]=%8d\n", oq, wait[oq]);
			}
		}
		buffer = buffer0 + w * h * bpp * q;
		if (alphablend) {
			if (!(pget(buffer, 0, 0, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, w-1, 0, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, 0, h-1, w, h, bppb) & 0xFF000000ull)
			 || !(pget(buffer, w-1, h-1, w, h, bppb) & 0xFF000000ull)
			) cls = 1; else cls = 0;
		}
		if (wait && aniauto) {
			buffer = buffer0;
			owait = wait[0];
			q = 0;
		}
		await = (2000000 / SCANRATE +1) >> 1;
		oldcount = totalcount;
		printf("viewer: delay_scrn=%8d\n", await);
	} else if (bpp != 4-screen_depth && !alphablend) {
		// ディザリングが有効だとテクスチャロードによる高速化が期待できないので対策してみる
		tmpbuf = (char*)malloc(w * h * (4-screen_depth));
		if (tmpbuf) {
			buffer = tmpbuf;
			stretchblt(tmpbuf, buffer0, w, w, h, h, 4-screen_depth, bpp, 0, 0, 0, 0, w, h, w, h, 0, 1, 0);
			//	imagedump_bmp(buffer0, w, h, bpp, 0, "host:/imagesrc.bmp");
			//	imagedump_bmp(tmpbuf , w, h, 4-screen_depth, 0, "host:/imagedst.bmp");
		}
	}
	printf("vi: bpp=%d, dither=%d, screen_depth:%d (%d) size=%4dx%4d\n", bpp, dither, screen_depth, 4-screen_depth, w, h);
	if (!buffer) buffer = buffer0;
	oq = q;
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
	if ((gsregs[gz].vmode == 80) || (gsregs[gz].vmode == 83))		gx /= 2;
	else if ((gsregs[gz].vmode >= 16) && (gsregs[gz].vmode <  60))	gx /= 2;
	else if ((gsregs[gz].vmode ==208) || (gsregs[gz].vmode ==211))	gx /= 2;
	else if ((gsregs[gz].vmode ==  2) || (gsregs[gz].vmode ==  3))	gx /= 4;
	else if ((gsregs[gz].vmode ==130) || (gsregs[gz].vmode ==131))	gx /= 4;
	printf("viewer: gx=%d, gy=%d, magx=%d, magy=%d\n", gx, gy, gsregs[gz].magx, gsregs[gz].magy);
//	printf("sdtv_aspect: %d\npixelaspect: %d\n", setting->img_sdtv_aspect, setting->img_pixel_aspect);
	if (ffmode & 2) {
		pw = 1.0;
		gx = 4; gy = 4;
	} else
	if (setting->img_pixel_aspect) {	// PAR適用
		if ((gsregs[gz].vmode == 2) || (gsregs[gz].vmode == 80) || (gsregs[gz].vmode == 130) || (gsregs[gz].vmode == 208)) { // NTSC/480p
			// 4:3 => 10:11
			//16:9 => 40:33
			if (!setting->img_sdtv_aspect)	pw = 11.0 / 10.0;	// 704x480 => 640x480 : 0.9091
			else							pw = 33.0 / 40.0;	// 704x480 => 853x480 : 1.2121
		} else if ((gsregs[gz].vmode == 3) || (gsregs[gz].vmode == 83) || (gsregs[gz].vmode == 131) || (gsregs[gz].vmode == 211)) {	// PAL/576p
			// 4:3 => 12:11
			//16:9 => 16:11
			if (!setting->img_sdtv_aspect)	pw = 11.0 / 12.0;	// 704x576 => 768x576 : 1.0909
			else							pw = 11.0 / 16.0;	// 704x576 =>1024x576 : 1.4545
		} else
			pw = 1.0;
	} else if (setting->img_sdtv_aspect && ((gsregs[gz].vmode == 2) || (gsregs[gz].vmode == 3) || (gsregs[gz].vmode == 80) || (gsregs[gz].vmode == 83) || (gsregs[gz].vmode == 130) || (gsregs[gz].vmode == 131) || (gsregs[gz].vmode == 208))) {
		// =0:4:3, =1:16:9
		// 1024x576 => 768x576 : 0.75
		pw = 0.75;
	} else {
		pw = 1.0;
	}
	cx = cy = cz = 0; oz = 1;
	if (imgpos) {
		int xx,yy,hh,ff;	
		ff = (ffmode != 0) +1;
		xx = imgpos % 3; yy = imgpos / 3;
		if (fullscreen)	hh = SCREEN_HEIGHT;
		else hh = MAX_ROWS*FONT_HEIGHT+FONT_HEIGHT-1;
		hh *= ff;
		switch(xx) {
			case 0:	// left
				cx = 0;
				break;
			case 1:	// center
				if (w > SCREEN_WIDTH) cx = (w - SCREEN_WIDTH) >> 1;
				else cx = 0;
				break;
			case 2:	// right
				if (w <= SCREEN_WIDTH) cx = 0;
				else cx = w -SCREEN_WIDTH;
				break;
		}
		switch(yy) {
			case 0:	// top
				cy = 0;
				break;
			case 1:	// center
				if (h <= hh) cy = 0;
				else cy = (h - hh) >> 1;
				break;
			case 2:	// bottom
				if (h > hh) cy = h - hh;
				else cy = 0;
				break;
		}
		//if (ffmode) cy >>= 1;
		printf("viewer: imgpos=%d(x=%d,y=%d), fullscreen=%d, hh=%d, ofs=(%d,%d), size=%dx%d\n", imgpos,xx,yy,fullscreen,hh,cx,cy,w,h);
	}
	ox = cx; oy = cy;
	static int entered=0;
	//if (entered) padExitPressMode(--entered, 0);
	if (!entered) padEnterPressMode(entered++, 0);
	while(1){
	//	waitPadReady(0, 0);
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
			if (!(ffmode & 1) && (new_pad & PAD_R2)) {
				redraw = fieldbuffers;
				bilinear ^= 1;
			}
			if (new_pad & PAD_START) {
				aniauto ^= 1;
				if (aniauto && wait) {
					owait = wait[q];
					oldcount = totalcount;
				}
			}
			if (mode & 2) {
				if (new_pad & PAD_L1) return 1;
				if (new_pad & PAD_R1) return 2;
				if (!(paddata & PAD_L2)) {
					if (new_pad & PAD_UP) return 1;
					if (new_pad & PAD_DOWN) return 2;
				}
			}
			if (ani) {
				if (!(paddata & PAD_L2)) {
					if (new_pad & PAD_LEFT) {
						if (q > 0) q--; else q = ani -1;
					}
					if (new_pad & PAD_RIGHT) {
						if (q < ani -1) q++; else q = 0;
					}
				}
			}
			if (paddata & PAD_L2) {
				if (paddata & PAD_UP)	{if (buttons.up_p   ) cy -= (totalcount -oldcount) * (buttons.up_p    +2) / 3; else cy -= 4 * (totalcount -oldcount) * oz;}
				if (paddata & PAD_DOWN) {if (buttons.down_p ) cy += (totalcount -oldcount) * (buttons.down_p  +2) / 3; else cy += 4 * (totalcount -oldcount) * oz;}
				if (paddata & PAD_LEFT) {if (buttons.left_p ) cx -= (totalcount -oldcount) * (buttons.left_p  +2) / 3; else cx -= 4 * (totalcount -oldcount) * oz;}
				if (paddata & PAD_RIGHT){if (buttons.right_p) cx += (totalcount -oldcount) * (buttons.right_p +2) / 3; else cx += 4 * (totalcount -oldcount) * oz;}
			//	if (paddata & PAD_UP)	cy -= 4 * (totalcount -oldcount) * oz;
			//	if (paddata & PAD_DOWN) cy += 4 * (totalcount -oldcount) * oz;
			//	if (paddata & PAD_LEFT) cx -= 4 * (totalcount -oldcount) * oz;
			//	if (paddata & PAD_RIGHT)cx += 4 * (totalcount -oldcount) * oz;
				if ((cx!=ox)||(cy!=oy)) cz+=totalcount-oldcount; else cz=0;
				if (cz < SCANRATE * 1) 		oz = 1;
				else if (cz < SCANRATE * 3)	oz = 2;
				else						oz = 5;
			//	printf("pad: Lj=%3d,%3d Rj=%3d,%3d UDLR=%3d,%3d,%3d,%3d\r", 
			//		buttons.ljoy_h, buttons.ljoy_v,
			//		buttons.rjoy_h, buttons.rjoy_v,
			//		buttons.up_p, buttons.down_p, buttons.left_p, buttons.right_p
			//	);
			}
			if (cx+SCREEN_WIDTH > w) cx = w-SCREEN_WIDTH;
			ff = (ffmode && interlace) +1;
			if (fullscreen) {
				if (cy+SCREEN_HEIGHT*ff > h) cy = h-SCREEN_HEIGHT*ff;
			} else {
				if (cy+(MAX_ROWS*FONT_HEIGHT+FONT_HEIGHT-1)*ff>h)
					cy = h-(MAX_ROWS*FONT_HEIGHT+FONT_HEIGHT-1)*ff;
			}
			if (cx < 0) cx = 0;
			if (cy < 0) cy = 0;
			if ((cx!=ox)||(cy!=oy)) {
				if (!resizer && !(wait && aniauto && (owait < await * (totalcount -oldcount))))
					redraw = fieldbuffers;
				//	printf("viewer: offset:%3d,%3d\r", cx, cy);
				redi = fieldbuffers;
				ox = cx; oy = cy;
			}
		}
		if (wait && aniauto) {
			if ((q == oq) && (redraw < fieldbuffers)) {
				k=0;
				owait -= await * (totalcount - oldcount);
				do {
					if (owait <= 0) {
						if (q < ani -1) q++; else q = 0;
						owait += wait[q];
						if (wait[q] < 0) owait += 3000000;
						else if (!wait[q]) {
							if (q == ani -1)
								owait += 1000000;
							else
								owait += await * 2;
						}
					}
					if (++k > 60) break;
				} while(owait <= 0);
			} else {
				owait = await * 30 + wait[q];
			}
		} else if (tmpbuf && resizer != oresizer) {
			if (resizer) {
				buffer = buffer0;
				bpp = tbb;
			} else {
				buffer = tmpbuf;
				bpp = 4-screen_depth;
			}
			oresizer = resizer;
		}
		oldcount = totalcount;
		if (q != oq) {
			buffer = buffer0 + w * h * bpp * q;
			redraw = fieldbuffers;
			if (!cls && ((framebuffers == 1) || (fieldbuffers == 2))) redi = fieldbuffers;
			oq = q;
		}
	
		if (redraw) {
			if (new_pad & (PAD_CIRCLE | PAD_SQUARE)) redi = 0;
			if (!redi || bppb&16)	clrScr(setting->color[COLOR_BACKGROUND]);
			nobgmpos = fullscreen;
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
				mx = (double) vw * gx / pw / (w -ff);
				my = (double) vh * gy * (fm+1) / (h -ff);
				gw = (double) vw / w;
				gh = (double) vh * (fm+1) / h;
				// 画面1ピクセル進むごとにイメージはbx,byドット進む(=縮小率)
				//dx = (double) w / vw;
				//dy = (double) h / vh;
				if (mx < my) my = mx; else mx = my;
				if (gw < gh) gh = gw; else gw = gh;
				dx = 1/mx*gx/pw;
				dy = 1/my*gy;
				//printf("screen: %dx%d, ps: %.3fx%.3f, %.3fx%.3f\n", vw, vh, dx, dy, mx, my);
				tw = w * mx / gx * pw; th = h * my / (fm+1) / gy;
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
				//if (gw > 1.2) {
				if ((dx < 0.67) || (dy < 0.85)) {
					k^=1;
					// 座標のキャッシュ - 高速化を期待してみる
					gw = mx/gx*pw;	gh = my/gy;
					for (x=0;x<=w;x++)	tmpx[x] = x*gw+dl;
					for (y=0;y<=h;y++)	tmpy[y] = (y*gh+k)/2+dt;
					// 描画
					if ((ffmode & 1) || !bilinear) {
						for (y=0;y<h;y++)
							for (x=0;x<w;)
								itoSprite(pget(buffer,x,y,w,h,bppb), tmpx[x++], tmpy[y], tmpx[x], tmpy[y+1], 0);
					} else {
						// グーローシェーディングテスト
						itoPrimShade( ITO_PRIM_SHADE_GOURAUD );
						for (y=0;y<h-1;y++) {
							itoTriangleStrip(	pget(buffer,0,y  ,w,h,bppb), tmpx[0], tmpy[y]  , 0,
												pget(buffer,0,y+1,w,h,bppb), tmpx[0], tmpy[y+1], 0,
												pget(buffer,1,y  ,w,h,bppb), tmpx[1], tmpy[y]  , 0);
							itoAddVertex(		pget(buffer,1,y+1,w,h,bppb), tmpx[1], tmpy[y+1], 0);
							for (x=2;x<w;x++) //{
								itoAddVertex2(	pget(buffer,x,y  ,w,h,bppb), tmpx[x], tmpy[y  ],
												pget(buffer,x,y+1,w,h,bppb), tmpx[x], tmpy[y+1]);
							itoEndVertex();
							itoGsFinish();
						}
						itoPrimShade( ITO_PRIM_SHADE_FLAT );
					}
			//	} else if (gh == 1) {
				} else if (dx == 1.0 && dy == 1.0) {
					if ((4-screen_depth == bpp) && !alphablend) {
						// テクスチャ読み込みテスト
						int base,src,ww;
						base = itoGetFrameBufferBase(itoGetActiveFrameBuffer())>>8;
						ww = w; if (ww > dw) ww = dw;
						itoGsFinish();
						src = bpp * (cx + w * cy);
						src+= (base!=0)*bpp*w;
						for (y=0;y<dh;y++,src+=2*bpp*w) {
							GsLoadImage(&buffer[src], &(GS_IMAGE){.x=dl,.y=dt+y,.width=ww,.height=1,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-bpp});
						}
					} else
						for (y=0;y<dh;y++)
							for (x=0;x<dw;x+=2)
								itoPoint2c(pget(buffer,x+cx,y*2+k+cy,w,h,bppb), x+dl, y+dt, pget(buffer,x+cx+1,y*2+k+cy,w,h,bppb), x+dl+1, y+dt);
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
				if ((dx < 0.67) || (dy < 0.83)) {
					// 座標のキャッシュ - 高速化を期待してみる
					gw = mx/gx*pw;	gh = my/gy;
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
				} else if (dx == 1.0 && dy == 1.0) {
					if ((4-screen_depth == bpp) && !alphablend) {
						// テクスチャ読み込みテスト
						int base,src,ww;
						base = itoGetFrameBufferBase(itoGetActiveFrameBuffer())>>8;
						ww = w; if (ww > dw) ww = dw;
						itoGsFinish();
						src = bpp * (cx + w * cy);
						for (y=0;y<dh;y++,src+=bpp*w) {
							GsLoadImage(&buffer[src], &(GS_IMAGE){.x=dl,.y=dt+y,.width=ww,.height=1,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-bpp});
						}
					} else
					for (y=0;y<dh;y++)
						for (x=0;x<dw;x+=2)
							itoPoint2c(pget(buffer,x+cx,y+cy,w,h,bppb), x+dl, y+dt, pget(buffer,x+cx+1,y+cy,w,h,bppb), x+dl+1, y+dt);
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
			redraw--;
			if (redi && !redraw) redi=0;
		} else {
			itoVSync();
		}
	}
//	padExitPressMode(0, 0); 
//	waitPadReady(0, 0);
	if (tmpbuf != NULL) free(tmpbuf);
	nobgmpos = 0;
	return 0;
}

//int set_viewerconfig(int linedisp, int tabspaces, int chardisp, int screenmode, int textwrap, int drawtype)
int set_viewerconfig(int *data)
{
	int i=0;
	linenum = data[i++];
	tabmode = data[i++];
	tabdisp = nldisp = data[i++];
	fullscreen = data[i++];
	wordwrap = data[i++];
	resizer = !data[i++];
	aniauto = data[i++];
	imgpos = data[i++];
	return i;
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
	//	waitPadReady(0, 0);
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

int strpos(char *target, int limit, char *src, int start){
	// 文字列の検索
	int i,cl; u8 cc,*t=NULL;
	int p=start-1;
	cl = strlen(src); cc = src[0];
	while(p < limit) {
		//	t = strchr(t, cc);
		for(i=p+1,t=NULL;i<limit;i++){
			if (target[i] == cc) {
				t = &target[i];
				break;
			}
		}
		if (t) {
			p = (int)t - (int)target;
			if (p + cl > limit) {t = NULL; break;}
			if (!strncmp(t, src, cl)) {
				break;
			}
		//	t++;
		} else break;
	}
	if (t) return p; else return -1;
//	if (t) return p;
//	return t;
}
#ifdef BBSVIEWER
////////////////////////////////
// ２ちゃんねる互換ログビューア(オンメモリバッファ版)
//	in:	mode	編集モード(b0=0:表示のみ)
//  	*file	ファイル名(相対パス対応用)
//  	*buffer	ログデータ
//  	size	ログサイズ
// データフォーマット
//	名前欄<>メール欄<>投稿日時・ID・Be等<> 本文 <> タイトル [<> その他のデータ ]\n
// 表示フォーマット(html版)
//	<P><center><big>タイトル</big></center></P><hr size="2" color="white" width="80%"><dl>
//	<dt>[レス番号] 名前欄 [メール欄] : 投稿日時等<dd>本文
//	</dl>
// 表示フォーマット(テキスト版)
//  [レス番号] 名前欄 [メール欄] : 投稿日時等
// 	>	本文
// 表示例)
//	                            アマガミSS　第21話「ハッケン」★7
//			─────────────────────────────────────
//	[442] 渡る世間は名無しばかり[sage] : 2010/11/26(金) 03:24:43.11 ID:H/q6v/Z/ \n
//	>	修羅場ｷﾀ━━━━━━(ﾟ∀ﾟ)━━━━━━!!!! \n
//	[443] ...
// LbFn表示)
//	LbFn v0.70.XX
//	host:/dat/livetbs/kako/o1290/1290708806.dat [ 115 KB ]
//								アマガミSS　第21話「ハッケン」★7
//	─────────────────────────────────────────────
//	[442] 渡る世間は名無しばかり[sage] : 2010/11/26(金) 03:24:43.11 ID:H/q6v/Z/
//	>	修羅場ｷﾀ━━━━━━(ﾟ∀ﾟ)━━━━━━!!!!
// 表示色)
//			osask		2ch風
//	back:	#406010		#efefef
//	text:	#ffffff		#000000
//	link:	#00e0e0		#0000ff
//	alink:	#10c080		#ff0000	(access-ing?)
//	vlink:	#10c080		#660099	(visited?)
//	none:	#ffffe0		#008000
//	found:	#ffff00		#0000ff
//	trips:	#ff8484		#90ee90
// 注意点)
//	・&gt;などのhtmlspecialcharsに注意する
//	・アンカーリンクのタグは除去する
enum{
	BBS_NAME,
	BBS_MAIL,
	BBS_DATE,
	BBS_TEXT,
	BBS_TITLE,
};
static char teri[] = "<>";
int explode(char **d, int *dsize, int max, char *s, int size, char *split){
	// 文字列の分割
	//	php)	(string[]) explode ( split-string , source-string );
	int i=0,p=0,l,cl;
	cl = strlen(split);
//	printf("explode: split=[%s]\n", s);
//	printf("s	=%09X\n", (int)s);
//	printf("p	=%4d\ncl	=%4d\n", p, cl);
	while(1) {
		// abcd<>efghijk<><>lmno
		// search "<>" =>
		//		p		= 0;
		//		cl		= 2	<= strlen("<>");
		//		l		= 4	<= strpos("abcd<>efghijk<><>lmno", 21, "<>", 0);
		//		d[0]	= 0;	(p)
		//		dsiz[0]	= 4;	(l - p)	//		 11111111112
		//		p		= 6;	(l + cl) //45678901234567890
		//		l		=13	<= strpos("abcd<>efghijk<><>lmno", 21, "<>", 6);
		//		d[1]	= 6;	(p)
		//		dsiz[1]	= 7;	(l - p)
		//		p		=15;	(l + cl)
		//		l		=15	<= strpos("abcd<>efghijk<><>lmno", 21, "<>",15);
		//		d[2]	=15;	(p)
		//		dsiz[2]	= 0;	(l - p)
		//		p		=17;	(l + cl)
		//		l		=-1	<= strpos("abcd<>efghijk<><>lmno", 21, "<>",17);
		//		d[3]	=17;	(p)
		//		dsiz[3]	= 4;	(size - p)
		l = strpos(s, size, split, p);
	//	printf("l	=%4d\n", l);
		if ((l < 0) || (i == max -1)) {
			d[i] = &s[p];
		//	printf("d[%d]	=%4d\n", i, p);
			dsize[i++] = size - p;
		//	printf("dsiz[%d]	=%4d\n", i-1, dsize[i-1]);
		//	u8 temp[size-p+1];
		//	temp[size-p] = 0;
		//	memcpy(temp, d[i-1], dsize[i-1]);
		//	printf("=>	[%s]\n", temp);
			break;
		} else {
			d[i] = &s[p];
		//	printf("d[%d]	=%4d\n", i, p);
			dsize[i++] = l - p;
		//	printf("dsiz[%d]	=%4d\n", i-1, dsize[i-1]);
		//	u8 temp[l-p+1];
		//	temp[l-p] = 0;
		//	memcpy(temp, d[i-1], dsize[i-1]);
		//	printf("=>	[%s]\n", temp);
			p = l + cl;
		//	printf("p	=%4d\n", p);
		}
	}
	return i;
}
//int ereg_replace(char *pattern, char *replacement, char *str, int size){
	// 正規表現による置換
	//対応文法:	(|)[^-].?+*
//}
int tagspace(char c) {
	return (c == 32) || (c == 9) || (c==0x3e) || (c == 13) || (c == 10);
}
int tagcheck(char *c, int size, u8 *tag, int space) {
	int l;
	l = strlen(tag);
	return (size > l) && !strnicmp(c, tag, l) && ((space && tagspace(c[l])) || !space);
}
int htmlspecialchars_decode_2ch(char *dist, int limit, char *s, int size) {
// htmlspecialchars)
//	変換対象となる文字は以下の通りです。 
//	・'&' (アンパサンド) は '&amp;' になります。 ※<A>タグのリンクの文字も対象
//	・ENT_NOQUOTESが設定されていない場合、'"' (ダブルクォート) は '&quot;'になります。 
//	・ENT_QUOTESが設定されている場合のみ、''' (シングルクオート) は '&#039;'になります。 
//	・'<' (小なり) は '&lt;' になります。 
//	・'>' (大なり) は '&gt;' になります。
	int r,w,nr,i,cc,ts=0,thru=0;
	u8 *tags[] = {"a", "font", "href=", "color=", "br", "b", "!--", "-->", "script"};
	u8 *colorname[] = {	// from Firefox 3.5.3
		"yellowgreen",			"yellow",				"whitesmoke",			"wheat",				"violet",				"turquoise",			
		"tomato",				"thistle",				"teal",					"steelblue",			"springgreen",			"snow",					
		"slategrey",			"slategray",			"slateblue",			"skyblue",				"silver",				"sienna",				
		"seashell",				"seagreen",				"sandybrown",			"salmon",				"saddlebrown",			"royalblue",			
		"rosybrown",			"red",					"purple",				"powderblue",			"plum",					"pink",					
		"peru",					"peachpuff",			"papayawhip",			"palevioletred",		"paleturquoise",		"palegreen",			
		"palegoldenrod",		"orchid",				"orangered",			"orange",				"olivedrab",			"olive",				
		"oldlace",				"navy",					"navajowhite",			"moccasin",				"mistyrose",			"mintcream",			
		"midnightblue",			"mediumvioletred",		"mediumturquoise",		"mediumspringgreen",	"mediumslateblue",		"mediumseagreen",		
		"mediumpurple",			"mediumorchid",			"mediumblue",			"mediumaquamarine",		"maroon",				"magenta",				
		"linen",				"limegreen",			"lime",					"lightyellow",			"lightsteelblue",		"lightslategrey",		
		"lightslategray",		"lightskyblue",			"lightseagreen",		"lightsalmon",			"lightpink",			"lightgrey",			
		"lightgreen",			"lightgray",			"lightgoldenrodyellow",	"lightcyan",			"lightcoral",			"lightblue",			
		"lemonchiffon",			"lawngreen",			"lavenderblush",		"lavender",				"khaki",				"ivory",				
		"indigo",				"indianred",			"hotpink",				"honeydew",				"greenyellow",			"green",				
		"grey",					"gray",					"goldenrod",			"gold",					"ghostwhite",			"gainsboro",			
		"fuchsia",				"forestgreen",			"floralwhite",			"firebrick",			"dodgerblue",			"dimgrey",				
		"dimgray",				"deepskyblue",			"deeppink",				"darkviolet",			"darkturquoise",		"darkslategrey",		
		"darkslategray",		"darkslateblue",		"darkseagreen",			"darksalmon",			"darkred",				"darkorchid",			
		"darkorange",			"darkolivegreen",		"darkmagenta",			"darkkhaki",			"darkgrey",				"darkgreen",			
		"darkgray",				"darkgoldenrod",		"darkcyan",				"darkblue",				"cyan",					"crimson",				
		"cornsilk",				"cornflowerblue",		"coral",				"chocolate",			"chartreuse",			"cadetblue",			
		"burlywood",			"brown",				"blueviolet",			"blue",					"blanchedalmond",		"bisque",				
		"beige",				"azure",				"aquamarine",			"aqua",					"antiquewhite",			"aliceblue",			
		"black",				"white",				0
	};
	uint64 colordata[] = {	// from Firefox 3.5.3
		0x9acd32, 0xffff00, 0xf5f5f5, 0xf5deb3, 0xee82ee, 0x40e0d0, 
		0xff6347, 0xd8bfd8, 0x008080, 0x4682b4, 0x00ff7f, 0xfffafa, 
		0x708090, 0x708090, 0x6a5acd, 0x87ceeb, 0xc0c0c0, 0xa0522d, 
		0xfff5ee, 0x2e8b57, 0xf4a460, 0xfa8072, 0x8b4513, 0x4169e1, 
		0xbc8f8f, 0xff0000, 0x800080, 0xb0e0e6, 0xdda0dd, 0xffc0cb, 
		0xcd853f, 0xffdab9, 0xffefd5, 0xdb7093, 0xafeeee, 0x98fb98, 
		0xeee8aa, 0xda70d6, 0xff4500, 0xffa500, 0x6b8e23, 0x808000, 
		0xfdf5e6, 0x000080, 0xffdead, 0xffe4b5, 0xffe4e1, 0xf5fffa, 
		0x191970, 0xc71585, 0x48d1cc, 0x00fa9a, 0x7b68ee, 0x3cb371, 
		0x9370db, 0xba55d3, 0x0000cd, 0x66cdaa, 0x800000, 0xff00ff, 
		0xfaf0e6, 0x32cd32, 0x00ff00, 0xffffe0, 0xb0c4de, 0x778899, 
		0x778899, 0x87cefa, 0x20b2aa, 0xffa07a, 0xffb6c1, 0xd3d3d3, 
		0x90ee90, 0xd3d3d3, 0xfafad2, 0xe0ffff, 0xf08080, 0xadd8e6, 
		0xfffacd, 0x7cfc00, 0xfff0f5, 0xe6e6fa, 0xf0e68c, 0xfffff0, 
		0x4b0082, 0xcd5c5c, 0xff69b4, 0xf0fff0, 0xadff2f, 0x008000, 
		0x808080, 0x808080, 0xdaa520, 0xffd700, 0xf8f8ff, 0xdcdcdc, 
		0xff00ff, 0x228b22, 0xfffaf0, 0xb22222, 0x1e90ff, 0x696969, 
		0x696969, 0x00bfff, 0xff1493, 0x9400d3, 0x00ced1, 0x2f4f4f, 
		0x2f4f4f, 0x483d8b, 0x8fbc8f, 0xe9967a, 0x8b0000, 0x9932cc, 
		0xff8c00, 0x556b2f, 0x8b008b, 0xbdb76b, 0xa9a9a9, 0x006400, 
		0xa9a9a9, 0xb8860b, 0x008b8b, 0x00008b, 0x00ffff, 0xdc143c, 
		0xfff8dc, 0x6495ed, 0xff7f50, 0xd2691e, 0x7fff00, 0x5f9ea0, 
		0xdeb887, 0xa52a2a, 0x8a2be2, 0x0000ff, 0xffebcd, 0xffe4c4, 
		0xf5f5dc, 0xf0ffff, 0x7fffd4, 0x00ffff, 0xfaebd7, 0xf0f8ff, 
		0x000000, 0xffffff, 
	};
	u32 fclr; //uint64 fclr;
	u8 *entities_src[] = {	// from [php5.2.13] get_html_translation_table(HTML_ENTITIES);
		"nbsp",    "iexcl",   "cent",    "pound",   "curren",  "yen",     "brvbar",  "sect",    
		"uml",     "copy",    "ordf",    "laquo",   "not",     "shy",     "reg",     "macr",    
		"deg",     "plusmn",  "sup2",    "sup3",    "acute",   "micro",   "para",    "middot",  
		"cedil",   "sup1",    "ordm",    "raquo",   "frac14",  "frac12",  "frac34",  "iquest",  
		"Agrave",  "Aacute",  "Acirc",   "Atilde",  "Auml",    "Aring",   "AElig",   "Ccedil",  
		"Egrave",  "Eacute",  "Ecirc",   "Euml",    "Igrave",  "Iacute",  "Icirc",   "Iuml",    
		"ETH",     "Ntilde",  "Ograve",  "Oacute",  "Ocirc",   "Otilde",  "Ouml",    "times",   
		"Oslash",  "Ugrave",  "Uacute",  "Ucirc",   "Uuml",    "Yacute",  "THORN",   "szlig",   
		"agrave",  "aacute",  "acirc",   "atilde",  "auml",    "aring",   "aelig",   "ccedil",  
		"egrave",  "eacute",  "ecirc",   "euml",    "igrave",  "iacute",  "icirc",   "iuml",    
		"eth",     "ntilde",  "ograve",  "oacute",  "ocirc",   "otilde",  "ouml",    "divide",  
		"oslash",  "ugrave",  "uacute",  "ucirc",   "uuml",    "yacute",  "thorn",   "yuml",    
		"quot",    "lt",      "gt",      "amp",     "thinsp",  "ensp",    "emsp",    "zwnj",	
		0
	};
	u32 entities_dst[] = {	// from [php5.2.13] get_html_translation_table(HTML_ENTITIES);
		0x0020, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 
		0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af, 
		0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
		0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf, 
		0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 
		0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 
		0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 
		0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df, 
		0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 
		0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 
		0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 
		0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff, 
		0x0022, 0x003c, 0x003e, 0x0026, 0xdb82, 0xdb88, 0xdb90, 0xdb80, 
		0
	};
	// &#8204(U+200C)=0dot, &#8202(U+200A)=1dot
	for(r=0,w=0;r<size&&w<limit-8;r++){
		if (s[r] == '&') {
			// 参照文字
			//	・htmlspcialchars_decode相当
			//	・有効なフォントパターンが無い参照文字や文字コードについては「?」と表示する
			//	todo:	[php] get_html_translation_table(HTML_ENTITIES) への対応
			int c,u;
			r++; nr=r;
			while(nr<size){
				if (nr >= r+10) break;
				if (s[nr++] == ';') break;
			}
			if (s[nr-1] != ';') {
				c = r;
				if (s[c] == '#') {
					c++;
					if ((s[c] == 'x') || (s[c] == 'X')) {s[c] = 'x'; c++;}
					if (s[c-1] == 'x') {
						while(c<size) {
							if (((s[c] >= 0x30) && (s[c] <= 0x39)) || ((s[c] > 0x40) && (s[c] < 0x48)) || ((s[c] > 0x60) && (s[c] < 0x68))) {
								c++;
								continue;
							}
							break;
						}
						nr = c;
					} else {
						while(c<size) {
							if ((s[c] >= 0x30) && (s[c] <= 0x39)) {
								c++;
								continue;
							}
							break;
						}
						nr = c;
					}
				} else {
					dist[w++] = '&';
					r--;
					continue;
				}
			}
			ucstableinit();
			if (s[r] == '#') {
				r++;
				if ((s[r] == 'x') || (s[r] == 'X')) {
					// 16進数
					//	&#x10ffff;
					r++;
					u = strtoul(&s[r], NULL, 16);
				} else {
					// 10進数
					//	&#1114111;
					u = atoi(&s[r]);
				}
			} else {
				u = -1;
				for(i=0;entities_src[i];i++){
					if (!strncmp(&s[r], entities_src[i], strlen(entities_src[i]))) {
						//	dist[w++] = entities_dst[i][0];
						u = entities_dst[i];
						break;
					}
				}
				if (u < 0) {
					for(i=0;entities_src[i];i++){
						if (!strnicmp(&s[r], entities_src[i], strlen(entities_src[i]))) {
							//	dist[w++] = entities_dst[i][0];
							u = entities_dst[i];
							break;
						}
					}
				}
			}
			if (u >= 0)	c = ucstable[u]; else c = 0x3F;
			if ((u >= 0xdb80) && (u < 0xdc00)) {
				dist[w++] = 27;
				dist[w++] = u - 0xdb80 + 128;
			} else
			if (c < 256) {
				if ((c == 0x3F) && (u != c)) {
					dist[w++] = 27;
					dist[w++] = 0x03;
				}
				if (u == 0xa0) c = u;
				dist[w++] = c;
			} else {
				c -= 256;
				dist[w++] = c / 188 + 0x81 + (c > 5827) * 0x40;
				dist[w++] = (c % 188) + 0x40 + ((c % 188) >= 0x7F);
			}
			r = nr -1;
		} else 
		if (s[r] == '<') {
			// htmlタグ
			//	→除去して反映
			r++; nr=r;
			while(nr<size){
				if (s[nr++] == '>') {
					break;
				}
			}
			if (s[r] == '/') {
				cc = 1;
				r++;
			} else if (s[nr-2] == '/') {
				cc = -1;
			} else
				cc = 0;
			if (tagcheck(&s[r], size-r, tags[6], 0)) {
				// <!-- comment -->
				r += 3; nr=r+2;
				while(nr<size){
					if ((s[nr++] == '>') && tagcheck(&s[nr-3], size-nr, tags[7], 0)) {
						break;
					}
				}
			} else
			if (tagcheck(&s[r], size-r, tags[8], 1)) {
				// script
				thru = !cc;
			} else
			if (tagcheck(&s[r], size-r, tags[0], 1)) {
				// a
				if (cc < 1) {
					r += 2;
					while(r < nr) {
						if (tagcheck(&s[r], size-r, tags[2], 0)) {
							// href=
							int q;
							r += 5;
							if ((q = s[r] == 0x22)) r++;
							// link target process to here
							if (q) {
								for(i=0;i<size-r;i++){
									if (s[r+i] == 0x22) {
										r += i +1;
										break;
									}
								}
							} else {
								for(i=0;i<size-r;i++){
									if (tagspace(s[r+i])) {
										r += i +1;
										break;
									}
								}
							}
						} else {
							// other: skip
							int q=0;
							while(r < nr) {
								if (s[r] == 0x22) {q=!q; r++; continue;}
								if (!q && tagspace(s[r])) {r++; break;}
								else r++;
							}
						}
					}
				}
			} else if (tagcheck(&s[r], size-r, tags[1], 1)) {
				// font
				if (cc < 1) {
					r += 5;
					while(r < nr) {
						if (tagcheck(&s[r], size-r, tags[3], 0)) {
							// color=
							int q,sp,ss=0;
							r += 5;
							if ((q = s[r] == 0x22)) r++;
							sp = r;
							if (q) {
								for(i=0;i<size-r;i++){
									if (s[r+i] == 0x22) {
										r += i +1;
										ss = i;
										break;
									}
								}
							} else {
								for(i=0;i<size-r;i++){
									if (tagspace(s[r+i])) {
										r += i +1;
										ss = i;
										break;
									}
								}
							}
							if (s[sp] == '#') {
								// 色指定(16進数6桁)
								fclr = strtoul(&s[sp], NULL, 16);
							//	printf("fontcolor=%08lx\n", strtoul(&s[sp], NULL, 16));
							} else {
								// 色指定(色彩名称)
								for(i=0,fclr=0;colorname[i];i++) {
									if (!strncmp(s, colorname[i], strlen(colorname[i]))) {
										fclr = colordata[i];
										break;
									}
								}
							}
							dist[w++] = 27;
							dist[w++] = 35;
							sprintf(&dist[w], "%06x;",  fclr);	
							w+=7;
							while((r < nr) && tagspace(s[r])) r++;
						} else {
							// other: skip
							int q=0;
							while(r < nr) {
								if (s[r] == 0x22) {q=!q; r++; continue;}
								if (!q && tagspace(s[r])) {r++; break;}
								else r++;
							}
						}
					}
				}
				if (cc) {
					dist[w++] = 27;
					dist[w++] = 64;
				}
			} else if (tagcheck(&s[r], size-r, tags[4], 1)) {
				// br
				if (dist[w-1] == 32) w--;
				dist[w++] = 10;
				if (s[nr] == 32) nr++;
				dist[w++] = 9;
			} else if (tagcheck(&s[r], size-r, tags[5], 1)) {
				// b
				if (cc) {
					// </b>
					dist[w++] = 27;
					dist[w++] = 0x37;
				} else {
					// <b>
					dist[w++] = 27;
					dist[w++] = 64;
				}
			}
			r = nr -1;
		} else if (!thru) {
			if (tagspace(s[r])) {
				if (!ts) {
					dist[w++] = 32;
				}
				ts = 1;
			} else {
				dist[w++] = s[r];
				ts = 0;
			}
		}
	}
	dist[w] = 0;
	return w;
}
// データフォーマット
//	名前欄<>メール欄<>投稿日時・ID・Be等<> 本文 <> タイトル [<> その他のデータ ]\n
// 表示フォーマット(テキスト版)
//  [レス番号] 名前欄 [メール欄] : 投稿日時等
// 	>	本文
// 表示フォーマット(html版)
//	<P><center><big>タイトル</big></center></P><hr size="2" color="white" width="80%"><dl>
//	<dt><a name="レス番号" />[レス番号] <font color="">名前欄</font>[<a href="mailto:メール欄">メール欄</a>] : 投稿日時等<dd> 本文
//	</dl>															~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
// 内部フォーマット
//	[レス番号] <font color="名前欄色"><font color="トリップ等色"></font></font>[<font color="メール欄色">メール欄</font> : 投稿日時等\n\t
//	本文(" <br> " → "\n\t" 化)
int dat2html(char *d, int limit, char *s, int size, int num) {
	// テキスト変換
	char *tmps[8]; int tmpi[8],p,i;
	u8 temp[3840];
	explode(tmps, tmpi, 8, s, size, teri);
	// レス番号
	sprintf(d, "[%d] ", num);
	p = strlen(d);
	// 名前欄
	if (tmpi[BBS_MAIL]) {
		d[p++] = 27;
		d[p++] = 0x36;
	} else {
		d[p++] = 27;
		d[p++] = 0x35;
	}
	i = htmlspecialchars_decode_2ch(temp, 3840, tmps[BBS_NAME], tmpi[BBS_NAME]);
	memcpy(&d[p], temp, i); p+=i;
//	memcpy(&d[p], tmps[BBS_NAME], tmpi[BBS_NAME]); p+=tmpi[BBS_NAME];
	d[p++] = 27;
	d[p++] = 64;
	// メール欄
	if (tmpi[BBS_MAIL]) {
		d[p++] = '[';
		d[p++] = 27;
		d[p++] = 0x32;
		d[p++] = 27;
		d[p++] = 0x75;
		memcpy(&d[p], tmps[BBS_MAIL], tmpi[BBS_MAIL]); p+=tmpi[BBS_MAIL];
		d[p++] = 27;
		d[p++] = 0x55;
		d[p++] = 27;
		d[p++] = 64;
		d[p++] = ']';
	}
	// 投稿日時等
	d[p++] = 32;
	d[p++] = 58;
	d[p++] = 32;
	memcpy(&d[p], tmps[BBS_DATE], tmpi[BBS_DATE]); p+=tmpi[BBS_DATE];
	d[p++] = 10;
	// 本文
	d[p++] = 9;
	i = tmps[BBS_TEXT][0] == 32;
	i = htmlspecialchars_decode_2ch(temp, 3840, tmps[BBS_TEXT]+i, tmpi[BBS_TEXT]-i);
	memcpy(&d[p], temp, i); p+=i;
	d[p] = 0;
//	printf("%s\n", d);
	return p;
}
int bbstitle(char *d, int limit, char *s, int size) {
	// スレッドタイトルの取得
	char *tmps[8]; int tmpi[8], i;
	if (!limit) return -1; else d[0] = 0;
	if (explode(tmps, tmpi, 8, s, size, teri) < BBS_TITLE) return -1;
	i = tmpi[BBS_TITLE];
	if (i >= limit) i = limit -1;
	memcpy(d, tmps[BBS_TITLE], i);
	d[i] = 0;
	if (d[i -1] == 10) d[--i] = 0;
	if (d[i -1] == 13) d[--i] = 0;
	printf("thread title: %s\n", d);
	return i;
}
int bbsview(int mode, char *file, unsigned char *c, unsigned int size)
{
	int x,y,z,w,ress,lines,tw,sz,p;
	//int crlf=0,cr=0,lf=0,lfcr=0,nlf=0,lines=0;
	int maxbytes=0;
	uint64 color1, color2, colors[8]={
		0x80106040,0x80ffffff,0x80e0e000,0x8080c010,
		0x8080c010,0x80e0ffff,0x8000ffff,0x808484ff
	};
	int sel=0, top=0, selx=0, oldselx=0, oldsel=0, msgf=0, oldmsgf=0;
	int i;//, type;
	int l2button=FALSE, oldl2=FALSE;
	int textwidth,scrnshot=0;
//	char *onoff[2] = {lang->conf_off, lang->conf_on};
	char msg0[MAX_PATH], msg1[MAX_PATH], tmp[MAX_PATH], title[MAX_PATH];
	ofscache datofs[MAX_LINES], txtofs[MAX_LINES];
	char *htmlbuff, tempbuff[4096];
	printf("tempbuff = 0x%08x\n", (int)&tempbuff);
	
//	colors[0] = 0x80406010;	//back
//	colors[1] = 0x80ffffff;	//text
//	colors[2] = 0x8000e0e0;	//link
//	colors[3] = 0x8010c080;	//alink
//	colors[4] = 0x8010c080;	//vlink
//	colors[5] = 0x80ffffe0;	//nomailname
//	colors[6] = 0x80ffff00;	//foundmailname
//	colors[7] = 0x80ff8484;	//tripname
	ress = txt_count(c, size, datofs, 0, MAX_LINES);
	if (ress == MAX_LINES)
		size = datofs[ress-1].offset + datofs[ress-1].bytes;
	if (datofs[ress-1].bytes < 2) ress--;
	bbstitle(title, MAX_PATH, c, datofs[0].bytes);
	tw = drawStringAAS(title, 0, 0, 0, 0, 0);
	if (title[0]) {
		//strcpy(msg0, title);
		msg0[0] = 0;
		msgf=oldmsgf=1;
	} else
		strcpy(msg0, file);
	for(i=0,sz=1;i<ress;i++){
		sz += dat2html(tempbuff, 4096, c +datofs[i].offset, datofs[i].bytes, i+1) +1;
	}
	htmlbuff = (char*)malloc(sz);
	if (htmlbuff) {
		for(i=0,p=0;i<ress;i++){
			p += dat2html(htmlbuff+p, sz-p, c +datofs[i].offset, datofs[i].bytes, i+1);
			htmlbuff[p++] = 10;
		}
		lines = txt_count(htmlbuff, sz-1, txtofs, 0, MAX_LINES);
		if (lines == MAX_LINES)
			size = txtofs[lines-1].offset + txtofs[lines-1].bytes;
		if (txtofs[lines-1].bytes < 2) lines--;
	} else
		return -2;
	redraw=fieldbuffers;
	maxbytes=MAX_COLS-1;
	
	while(1){
	//	waitPadReady(0, 0);
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
		top = sel;

		if((selx != oldselx) || (sel != oldsel)) {
			oldsel = sel;
			oldselx = selx;
			redraw = fieldbuffers;
		}
		if (oldmsgf != msgf) {
			oldmsgf = msgf;
			if (msgf && title[0]) {
				//strcpy(msg0, title);
				msg0[0] = 0;
			} else
				strcpy(msg0, file);
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
						w = txtofs[top+i].bytes;
						z = txtofs[top+i].offset;
					} else {
						w = 0;
						z = txtofs[MAX_LINES-1].offset;
					}
					w = txtdraw(htmlbuff+z, w, TXT_SJIS);
					if ((displine[0] != 0)&&(w>=selx))
						drawStringAAS(displine, x+linenum*6*FONT_WIDTH-FONT_WIDTH*selx, y, colors, linenum*6*FONT_WIDTH+FONT_WIDTH, (MAX_ROWS_X+8)*FONT_WIDTH);
					//	drawString(displine+selx, TXT_ASCII, x+linenum2*6*FONT_WIDTH, y, color1, color2, ctrlchars);
					//	drawString(displine+selx, TXT_SJIS, x+linenum2*6*FONT_WIDTH, y, color1, color2, ctrlchars);
				}
        
				y += FONT_HEIGHT;
			}
			if (linenum) {
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
				dialog_height = FONT_HEIGHT*3;
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
			//	sprintf(tmp, "△:%s [%s]", lang->editor_l2popup_charset, chartablename[cp]);
			//	printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
				sprintf(tmp, "×:%s", lang->editor_l2popup_flicker);
				printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			//	sprintf(tmp, "□:%s", lang->editor_l2popup_tabmode);
			//	printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			//	sprintf(tmp, "R2:%s [%s]", lang->editor_l2popup_wordwrap, onoff[wordwrap&1]);
			//	printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE); y+=FONT_HEIGHT;
			}
			setScrTmp(msg0, msg1);
			if (msgf)
				drawStringAAS(title, (SCREEN_WIDTH - tw) >> 1, SCREEN_MARGIN+FONT_HEIGHT, colors, 0, SCREEN_WIDTH);
			// 	drawStringLimit(msg0, TXT_SJIS, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], 0, SCREEN_WIDTH - FONT_WIDTH*2);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	if (htmlbuff) free(htmlbuff);
	return 0;
}
#else
int bbsview(int mode, char *file, unsigned char *c, unsigned int size) {return -1;}
#endif

//	#ifndef	itoVSync
//	#define	itoVSync	X_itoVSync
//	#endif
#ifdef	itoVSync
#undef	itoVSync
#endif
int ps2_screenshot_file( const char* pFilename,unsigned int VramAdress,
                         unsigned int Width, unsigned int Height, unsigned int Psm );
int ps2_screenshot( void* pTemp, unsigned int VramAdress,unsigned int x,unsigned int y,
                    unsigned int Width, unsigned int Height, unsigned int Psm );
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
void screen_tex_dump(char *dist, int size);
int screen_tex_size(void);
int screenshot_getsize(void);
int screenshot_tobuff(char *dist);
int screenshot_to_bmp(char *name);
int screenshot_frombuff(char *data);
int tw,th,tb;
void X_clrScr(void) {
	// 壁紙を描画する
	if (tb != 4-screen_depth) {
		itoSprite(setting->color[COLOR_BACKGROUND], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, -1);
		return;
	} else if (tw > 2048) {
		return X_itoSprite(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	}
	
	int base;
	itoGsFinish();
	
	base = itoGetFrameBufferBase(itoGetActiveFrameBuffer())>>8;
	if (ffmode && interlace) {
		GsLoadImage(&scrnbuff[(base!=0)*scrnsize/2], &(GS_IMAGE){.x=0,.y=0,.width=tw,.height=SCREEN_HEIGHT,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-tb});
	}  else  {
		GsLoadImage(scrnbuff, &(GS_IMAGE){.x=0,.y=0,.width=tw,.height=SCREEN_HEIGHT,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-tb});
	}
	
	base = setting->wallpaper[0].brightness;
	if (base == 0) return;
	
	itoPrimAlphaBlending(TRUE);
	if (base < 0) {
		// 暗くする
		itoSprite(0x000000 | (-base << 24), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	} else {
		// 明るくする
		itoSprite(0xFFFFFF | ( base << 24), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	}
	itoPrimAlphaBlending(FALSE);
}
void X_itoSprite(int left, int top, int right, int bottom, int window) {
	if (!wallpaper || (tb != 4-screen_depth)) {
		itoSprite(setting->color[COLOR_BACKGROUND], left, top, right, bottom, 0);
		return;
	}
	
	// 壁紙で塗りつぶす
	int base,src,width,y,tt;
	itoGsFinish();
	
	base = itoGetFrameBufferBase(itoGetActiveFrameBuffer())>>8;
	if (right > SCREEN_WIDTH) right = SCREEN_WIDTH;
	width = right-left;	if (width >= SCREEN_WIDTH) right = left+tw;
	src = tb*(tw*top+left); 
	if (ffmode && interlace) src += (base!=0)*scrnsize/2;
	if (width < tw || tw > 2048) {
		tt = width; if (tt > 2048) tt = 2048;
		for(y=0;y<bottom-top;y++,src+=tw*tb){
			GsLoadImage(&scrnbuff[src], &(GS_IMAGE){.x=left,.y=top+y,.width=tt,.height=1,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-tb});
		}
	} else {
		GsLoadImage(&scrnbuff[src], &(GS_IMAGE){.x=left,.y=top,.width=right-left,.height=bottom-top,.vram_addr=base,.vram_width=(SCREEN_WIDTH+63)/64,.pix_mode=4-tb});
	}
	
	// 明るさ調整
	y = setting->wallpaper[window].brightness;
	if (y == 0) return;
	itoPrimAlphaBlending(TRUE);
	if (y < 0) {
		// 暗くする
		itoSprite(0x000000 | (-y << 24), left, top, right, bottom, 0);
	} else {
		// 明るくする
		itoSprite(0xFFFFFF | ( y << 24), left, top, right, bottom, 0);
	}
	itoPrimAlphaBlending(FALSE);
}
//static char dithermatrix[2][8] = {{0,4,1,5,1,5,0,4},{6,2,7,3,7,3,6,2}};
//static char dithermatrixnormal[16] = {0,4,1,5,6,2,7,3,1,5,0,4,7,3,6,2};
static char dithermatrix3[2][8] = {{ 0,-4, 1,-3, 1,-3, 0,-4},{-2, 2,-1, 3,-1, 3,-2, 2}};
static char dithermatrixnormal3[4][4] = {{ 0,-4, 1,-3},{-2, 2,-1, 3},{ 1,-3, 0,-4},{-1, 3,-2, 2}};
static char dithermatrix4a[4][4] = {{ 0,-8, 2,-6},	{-4, 4,-2, 6},	{ 3,-5, 1,-7},	{-1, 7,-3, 5}};
static char dithermatrix4b[4][4] = {{ 0, 8, 2,10},	{12, 4,14, 6},	{ 3,11, 1, 9},	{15, 7,13, 5}};
static int ditherenable, ffframe;
static uint64 clut[256];
void setclut216(uint64 *clut, int ofs) {
	// 6階調216色パレットのロード
	int r,g,b,i;
	for(b=0,i=ofs;b<6;b++);
		for(g=0;g<6;g++);
			for(r=0;r<6;r++,i++);
				clut[i] = 0x80000000L | (r * 0x000033 + g * 0x003300 + b * 0x330000);
//	for(i=0;i<216;i++)	printf("clut[%3d]=%08lX\n", i+ofs, clut[i+ofs]);
}
void setclut16(uint64 *clut, int ofs) {
	// 16色固定パレットのロード
	int i;
	for(i=1;i<8;i++){
	//	clut[i+ofs+0] = 0x80000000L | ((((i & 1) * 0x040000 + (i & 2) * 0x000200 + (i & 4) * 0x000001) >> 2) * 0x80);
	//	clut[i+ofs+8] = 0x80000000L | ((((i & 1) * 0x040000 + (i & 2) * 0x000200 + (i & 4) * 0x000001) >> 2) * 0xFF);
		clut[i+ofs+0] = 0x80000000L | (((i & 1) * 0x000001 + (i & 2) * 0x000080 + (i & 4) * 0x004000) * 0x80);
		clut[i+ofs+8] = 0x80000000L | (((i & 1) * 0x000001 + (i & 2) * 0x000080 + (i & 4) * 0x004000) * 0xFF);
	}
	clut[ofs+0] = 0x80000000L;
	clut[ofs+8] = 0x80C0C0C0L;
//	for(i=0;i<16;i++)	printf("clut[%3d]=%08lX\n", i+ofs, clut[i+ofs]);
}
uint64 truetoclut(uint64 src, int x, int y) {
	// 256階調から6階調への減色
	int dither,a,b,c;
	if (ditherenable) {
		if (0) {
			// PS2GSと同じ減色法
			dither = dithermatrix4a[y&3][x&3];// & 15 -8;
			a = ((((src >> 16) & 0xFF) * 6 +3) / 16) +dither;
			b = ((((src >>  8) & 0xFF) * 6 +3) / 16) +dither;
			c = ((((src >>  0) & 0xFF) * 6 +3) / 16) +dither;
			if ((a < 0) || (a > 95)) a -= dither;
			if ((b < 0) || (b > 95)) b -= dither;
			if ((c < 0) || (c > 95)) c -= dither;
		} else {
			// 一般的(?)な減色法
			dither = dithermatrix4b[y&3][x&3] & 15;
			a = ((((src >> 16) & 0xFF) * 16) / 51) +dither;
			b = ((((src >>  8) & 0xFF) * 16) / 51) +dither;
			c = ((((src >>  0) & 0xFF) * 16) / 51) +dither;
		}
		a >>= 4; b >>= 4; c >>= 4;
	} else {
		// ディザリング無しの減色
		a = (((src >> 16) & 0xFF) * 6 +3) >> 8;
		b = (((src >>  8) & 0xFF) * 6 +3) >> 8;
		c = (((src >>  0) & 0xFF) * 6 +3) >> 8;
	}
	return a * 36 + b * 6 + c +32;
}
uint64 truetohigh(uint64 src, int x, int y) {
	if (ditherenable) {
		int dither;
#if 1
		if (ditherenable & 1)
			dither = dithermatrixnormal3[y&3][x&3];
		else
			dither = dithermatrix3[ffframe][(y&1)*4+(x&3)];
		src=(((src & 0x0000FF) + dither)& 0x0003FF)
		|	(((src & 0x00FF00) + dither * 0x000100) << 24) 
		|	(((src & 0xFF0000) + dither * 0x010000) & 0x03FF0000);
		if (src & 0x000000000100L) src -=                 dither;
		if (src & 0x000001000000L) src -=     0x010000L * dither;
		if (src & 0x010000000000L) src -= 0x0100000000L * dither;
		src = (src & 0xFF00FF) | ((src >> 24) & 0x00FF00);
#endif
#if 0
		if (ffmode&&interlace)
			dither = dithermatrix3[ffframe][(y&1)*4+(x&3)];
		else
			dither = dithermatrixnormal3[y&3][x&3];
		src=((src & 0xFF00FF) + dither * 0x010001) | (((src & 0x00FF00) + dither * 0x000100) << 24);
		if (src & 0x000000000100L) src -=                 dither;
		if (src & 0x000001000000L) src -=     0x010000L * dither;
		if (src & 0x010000000000L) src -= 0x0100000000L * dither;
		src = (src & 0xFF00FF) | ((src >> 24) & 0x00FF00);
#endif
#if 0
		if (ffmode&&interlace)
			src += dithermatrix3[ffframe][(y&1)*4+(x&3)] * 0x010101;
		else
			src += dithermatrixnormal3[y&3][x&3] * 0x010101;
#endif
	}
	return ((src & 0xF80000) >> 9) | ((src & 0x00F800) >> 6) | ((src & 0x0000F8) >> 3) | ((src & 0x80000000) >> 16);
	
}
uint64 hightotrue(uint64 src) {
	return 
		((((src & 0x001F) * 0x00000021) >> 2) & 0x000000FF)
	|	((((src & 0x03E0) * 0x00000042) >> 0) & 0x0000FF00)
	|	((((src & 0x7C00) * 0x00000210) >> 0) & 0x00FF0000)
	|	((src & 0x8000) << 16);
}
uint64 cluttotrue(uint64 src) {
	if (!activeclut) return 0L;
	return activeclut[src];
//	src = activeclut[src];
//	return (src & 0xFF00FF00L) | ((src >> 16) & 0x0000FF) | ((src << 16) & 0xFF0000);
}
uint64 cluttohigh(uint64 src) {
	if (!activeclut) return 0L;
	src = activeclut[src];
	return ((src & 0xF80000) >> 9) | ((src & 0x00F800) >> 6) | ((src & 0x0000F8) >> 3) | ((src & 0x80000000) >> 16);
//	return ((src & 0xF80000) >> 19) | ((src & 0x00F800) >> 6) | ((src & 0x0000F8) << 7) | ((src & 0x80000000) >> 16);
}
/* //
int stretchblt(char *scrnbuff, char *buffer, 
				int ds, int ss, int db, int sb, int dp, int sp,
				int dl, int dt, int sl, int st, 
				int dw, int dh, int sw, int sh, int ff, int de, int fl) {
	double fx,fy; int x,y,sx[dw+1],sy[dh+1],ys,ye,k;
	unsigned char lx[dw],ly[dh];
	// 255 x 255 + 255 x 0 = 
	unsigned short *uss=(unsigned short *)buffer, *usd=(unsigned short *)scrnbuff;
	unsigned int   *uis=(unsigned int   *)buffer, *uid=(unsigned int   *)scrnbuff;
	unsigned char  *ucs=(unsigned char  *)buffer, *ucd=(unsigned char  *)scrnbuff;
	fx = 1.0f * dw / sw;	ffframe = ff;	
	fy = 2.0f * dh / sh;	ditherenable = de;
	for(x=0;x<dw;x++){	k = 256 *  x       / fx; sx[x] = k / 256; lx[x] = k % 256;}
	for(y=0;y<dh;y++){	k = 256 * (y*2+ff) / fy; sy[y] = k / 256; ly[y] = k % 256;}
	sx[x] = sw; sy[y] = sh;
	printf("stretchblt: s:(%4d,%4d)-(%4dx%4d) buf=%4dx%4dx%2d f=%10.4fx%10.4f\n", sl, st, sw, sh, ss, sb, sp*8, fx, fy / 2 * (1+ff));
	printf("stretchbit: d:(%4d,%4d)-(%4dx%4d) buf=%4dx%4dx%2d (%4d,%4d)-(%4d,%4d)\n", dl, dt, dw, dh, ds, db, dp*8, sx[0], sy[0], sx[dw-1], sy[dh-1]);
	ys = 0; if (dt < 0) ys = -dt;
	ye = dh; if (dt+dh > db) ye = db - dt;
	for(y=ys;y<ye;y++){
		if ((sy[y] < 0) || (sy[y] >= sh)) continue;// || (y+dt >= dh
		if (sp==dp) {
			for(x=0;x<dw;x++){
				if ((sx[x] < 0) || (sx[x] >= sw) || (x+dl < 0) || (x+dl >= ds)) continue;
				if (dp>2)	uid[(y+dt)*ds+x+dl] = uis[(sy[y]+st)*ss+sx[x]+sl];
				else		usd[(y+dt)*ds+x+dl] = uss[(sy[y]+st)*ss+sx[x]+sl];
			}
		} else {
			// todo: 高速?＆最適化
			for(x=0;x<dw;x++){
				if ((sx[x] < 0) || (sx[x] >= sw) || (x+dl < 0) || (x+dl >= ds)) continue;
				if (sp>2)		k = uis[(sy[y]+st)*ss+sx[x]+sl];
				else if (sp==2)	k = uss[(sy[y]+st)*ss+sx[x]+sl];
				else			k = ucs[(sy[y]+st)*ss+sx[x]+sl];
				if (sp==2)		k = hightotrue(k);
				else if (sp<2)	k = cluttotrue(k);
				if (dp==2)		k = truetohigh(k,x+dl,y+dt);
				else if (dp<2)	k = truetoclut(k,x+dl,y+dt);
				if (dp>2)		uid[(y+dt)*ds+x+dl] = k;
				else if (dp==2)	usd[(y+dt)*ds+x+dl] = k;
				else			ucd[(y+dt)*ds+x+dl] = k;
			}
		}
	}
	return 0;
}//*/
int stretchblt(char *scrnbuff, char *buffer, 
				int ds, int ss, int db, int sb, int dp, int sp,
				int dl, int dt, int sl, int st, 
				int dw, int dh, int sw, int sh, int ff, int de, int fl) {
	double fx,fy; int x,y,sx[dw],sy[dh],ys,ye,k;
	unsigned char lx[dw],ly[dh];
	// 255 x 255 + 255 x 0 = 
	unsigned short *uss=(unsigned short *)buffer, *usd=(unsigned short *)scrnbuff;
	unsigned int   *uis=(unsigned int   *)buffer, *uid=(unsigned int   *)scrnbuff;
	unsigned char  *ucs=(unsigned char  *)buffer, *ucd=(unsigned char  *)scrnbuff;
	if (fl) {
		if (sw!=dw&&sw>1) sw--;
		if (sh!=dh&&sh!=dh*2&&sh>1) sh--;
	}
	fx = 1.0f * dw / sw;	ffframe = ff;	
	fy = 2.0f * dh / sh;	ditherenable = de;
	for(x=0;x<dw;x++){	k = 256 *  x       / fx; sx[x] = k / 256; lx[x] = k % 256;}
	for(y=0;y<dh;y++){	k = 256 * (y*2+ff) / fy; sy[y] = k / 256; ly[y] = k % 256;}
//	if (!ff && ss > 64) 
//	for(x=0;x<dw;x++){
//		printf("%4d.%02X ", sx[x], lx[x]);
//		if ((x&7)==7) printf("\n");
//	}	printf("\n");
//	sx[x] = sw; sy[y] = sh;
	printf("stretchblt: s:(%4d,%4d)-(%4dx%4d) buf=%4dx%4dx%2d f=%10.4fx%10.4f\n", sl, st, sw, sh, ss, sb, sp*8, fx, fy / 2 * (1+ff));
	printf("stretchbit: d:(%4d,%4d)-(%4dx%4d) buf=%4dx%4dx%2d (%4d,%4d)-(%4d,%4d)\n", dl, dt, dw, dh, ds, db, dp*8, sx[0], sy[0], sx[dw-1], sy[dh-1]);
	ys = 0; if (dt < 0) ys = -dt;
	ye = dh; if (dt+dh > db) ye = db - dt;
	for(y=ys;y<ye;y++){
		if ((sy[y] < 0) || (sy[y] >= sh)) continue;// || (y+dt >= dh
		if (sp==dp&&!alphablend&&sw==dw) {
			for(x=0;x<dw;x++){
				if ((sx[x] < 0) || (sx[x] >= sw) || (x+dl < 0) || (x+dl >= ds)) continue;
				if (dp>2)	uid[(y+dt)*ds+x+dl] = uis[(sy[y]+st)*ss+sx[x]+sl];
				else		usd[(y+dt)*ds+x+dl] = uss[(sy[y]+st)*ss+sx[x]+sl];
			}
		} else {
			// todo: 高速?＆最適化
			uint64 half(uint64 color1, uint64 color2, int blend);
			int pgeto(int cx, int cy) {
				int k;
				if (sp>2)		k = uis[cy*ss+cx];
				else if (sp==2)	k = uss[cy*ss+cx];
				else			k = ucs[cy*ss+cx];
				if (sp==2)		k = hightotrue(k);
				else if (sp<2)	k = cluttotrue(k);
				return k;
			}
			void pseto(int cx, int cy, int k) {
				if (alphablend) {
					int m=setting->color[COLOR_BACKGROUND];
				//	if (dp>2)		m = uid[cy*ds+cx];
				//	else if (dp==2)	m = hightotrue(usd[cy*ds+cx]);
				//	else			m = cluttotrue(ucd[cy*ds+cx]);
					k = half(m, k, (k >> 23) & 0x1FE);
				}
				if (dp==2)		k = truetohigh(k,cx,cy);
				else if (dp<2)	k = truetoclut(k,cx,cy);
				if (dp>2)		uid[cy*ds+cx] = k;
				else if (dp==2)	usd[cy*ds+cx] = k;
				else			ucd[cy*ds+cx] = k;
			}
			int pgets(int cx, int cy, int x, int y) {
				if ((!lx[x] && !ly[y])) return pgeto(cx, cy);
				int k;
				if (lx[x] && ly[y]) 
					k = half(half(pgeto(cx, cy), pgeto(cx+1, cy), lx[x]), half(pgeto(cx, cy+1), pgeto(cx+1, cy+1), lx[x]), ly[y]);
				else if (lx[x])
					k = half(pgeto(cx, cy), pgeto(cx+1, cy), lx[x]);
				else
					k = half(pgeto(cx, cy), pgeto(cx, cy+1), ly[y]);
				return k;
			}
			if (fl) {
				for(x=0;x<dw;x++){
					if ((sx[x] < 0) || (sx[x] >= sw) || (x+dl < 0) || (x+dl >= ds)) continue;
					pseto(x + dl, y + dt, pgets(sx[x] + sl, sy[y] + st, x, y));
				}
			} else {
				for(x=0;x<dw;x++){
					if ((sx[x] < 0) || (sx[x] >= sw) || (x+dl < 0) || (x+dl >= ds)) continue;
					pseto(x + dl, y + dt, pgeto(sx[x] + sl, sy[y] + st));
				}
			}
		}
	}
	return 0;
}
int info_image(int *info, char *decoded, int dsize, int type) {
	int ret=0;
	switch(type) {
		case FT_JPG:
		{
			char *env;
			env = (char*)malloc(16384*sizeof(int));
			if (env == NULL) return -2;
			env[0] = 0; env[1] = 0;
			ret = info_JPEG(env, info, dsize, decoded);
			free(env);
			break;
		}
		case FT_BMP:	ret = info_BMP(info, decoded, dsize);	break;
		case FT_GIF:	ret = info_GIF(info, decoded, dsize);	break;
		case FT_P2T:	ret = info_PS2ICO(info,decoded,dsize);	break;
		case FT_PS1:	ret = info_PS1ICO(info,decoded,dsize);	break;
		case FT_PNG:	ret = info_PNG(info, decoded, dsize);	break;
	}
	return ret;
}
int decode_image(char *buffer, char *decoded, int dsize, int bpp, int *info, int type) {
	int ret=-1;
	alphablend = FALSE;
	switch(type) {
		case FT_JPG:
		{
			char *env;
			env = (char*)malloc(16384*sizeof(int));
			if (env == NULL) return -2;
			env[0] = 0; env[1] = 0;
			ret = info_JPEG(env, info, dsize, decoded);
			ret = decode0_JPEG(env, dsize, decoded, bpp, buffer, 0);
			free(env);
			break;
		}
		case FT_BMP:	ret = decode_BMP(buffer, decoded, dsize, bpp);	break;
		case FT_GIF:	alphablend = TRUE;	
						ret = decode_GIF(buffer, decoded, dsize, bpp);	break;
		case FT_P2T:	ret = decode_PS2ICO(buffer,decoded,dsize,bpp);	break;
		case FT_PS1:	ret = decode_PS1ICO(buffer,decoded,dsize,bpp);	break;
		case FT_PNG:	if (info[5] & 4) alphablend = TRUE;	
						ret = decode_PNG(buffer, decoded, dsize, bpp);	break;
	}
	return ret;
}
void memset16(void *dst, int val, int size) {
	short *d=(short*)dst; int i;
	//	printf("memset16: dst:%08X, val:%04X, size:%7d\n", (int)dst, val, size);
	for(i=0;i*2<size;i++)	d[i] = val;
}
void memset32(void *dst, int val, int size) {
	int *d=(int*)dst; int i;
	for(i=0;i*4<size;i++)	d[i] = val;
}
void memsetdither(void *dst, uint64 val, int size) {
	short *d=(short*)dst; int i;
//	printf("	%08X: %016lX: \n", (int)dst, val);
//	for(i=0;i<4;i++) {
//		printf("%04X, ", (int)(val >> (1ul << ((i & 3) * 16))) & 0xFFFF);
//	}	printf("\n"); delay(1);
	for(i=0;i*2<size;i++)	d[i] = (val >> ((i & 3) * 16)) & 0xFFFF;
}
static char wpold[MAX_PATH]={0}, wppath[MAX_PATH]={0};
static char *wpbuff=NULL; 
static int wpsize=0,wptype=FT_BINARY,wpcmd=FT_BINARY,wpinfo[8],wpbpp,wpalpha,wpclip=-1,wpvmode=-1;
int imagedump_bmp(char *buff, int width, int height, int bpp, uint64 *clut, char *filename);
int wallpapercache(int type) {
	//	type	=0:filecache
	//			=1:imagecache
	wpsize = 1; wptype = FT_JPG;
	if (type) wptype = FT_BINARY;
	wpcmd = wptype;
	return 0;
}
int wallpaperfree(void) {
	if (wpbuff) {
		free(wpbuff); 
		printf("wallpapersetup: wallpaper cache removed\n");
		wpbuff = NULL;
		wpsize = 0;
	}
	return 0;
}
void wallpapersetup(void) {
	int i,fd,ff,type,size,dsize,bpp,tvmode,dither,info[8]; char *c,*decoded,*buffer;
	//wallpaper = 0;
	printf("wallpapersetup: flag:%d, src:%s\n", setting->wallpaper[0].flag, setting->wallpaperpath);
	tvmode = setting->tvmode;
	if (gsregs[tvmode].loaded != 1) tvmode = ITO_VMODE_AUTO-1;
	if (setting->wallpaper[0].flag && setting->wallpaperpath[0] && (wpclip<0 || wpclip!=setting->wallpaper[0].clipmode || wpvmode<0 || wpvmode!=tvmode)) {
		if (scrnsize != (i = screen_tex_size())) {
			scrnsize = i;
			if (scrnbuff != NULL) free(scrnbuff);
			scrnbuff = (char*)malloc(scrnsize);
			if (!scrnbuff) scrnsize = 0;
			wallpaper = 0;
		}
		printf("wallpapersetup: scrnbuff: %08X, scrnsize: %d\n", (int)scrnbuff, scrnsize);
		if (scrnbuff) {
			bpp = setting->screen_depth[tvmode] > 0 ? (setting->screen_depth[tvmode]-1):4-gsregs[tvmode].psm;
			dither = setting->screen_dither[tvmode] > 0 ? (setting->screen_dither[tvmode]-1):gsregs[tvmode].dither;
			if ((bpp == 2) && dither) bpp = 4;
			ff = (ffmode && interlace) +1;
			
			dsize = size = 0;
			decoded = c = buffer = NULL;
			if (wpbuff&&!strcmp(wppath,setting->wallpaperpath)) {
				if (wptype == FT_BINARY) {
					buffer = wpbuff;
					bpp = wpbpp;
					for(i=0;i<8;i++) info[i] = wpinfo[i];
					alphablend = wpalpha;
					printf("wallpapersetup: use old image cache\n");
				} else {
					dsize = size = wpsize;
					decoded = wpbuff;
					printf("wallpapersetup: use old file cache\n");
				}
			} else {
				if (wpbuff) {
					free(wpbuff);
					printf("wallpapersetup: old cache removed\n");
					wpbuff = NULL;
					wpsize = 1;
				}
				fd = nopen(setting->wallpaperpath, O_RDONLY);
				if (fd >= 0) {
					drawMsg(lang->conf_wallpaperload);
					dsize = size = nseek(fd, 0, SEEK_END);
					printf("wallpapersetup: size: %d\n", size);
					nseek(fd, 0, SEEK_SET);
					c = (char*)malloc(size);
					if (c != NULL) {
						//	drawMsg(lang->gen_loading);
						printf("wallpapersetup: loading %s\n", setting->wallpaperpath);
						nread(fd, c, size);
					}
					nclose(fd);
					if (c) {
						if (setting->txt_autodecode && ((dsize = tek_getsize(c)) >= 0)) {
							decoded = (char*)malloc(dsize);
							if (decoded != NULL) {
								if (tek_decomp(c, decoded, size)<0) {
									free(decoded);
									decoded=NULL;
									printf("viewer: tek auto decode failed\n");
								} else {
									printf("viewer: decoded tek compression\n");
								}
							}
						} else if (setting->txt_autodecode && ((dsize = gz_getsize(c, size)) >= 0)) {
							decoded = (char*)malloc(dsize);
							if (decoded != NULL) {
								if (gzdecode(decoded, dsize, c, size)<0) {
									free(decoded);
									decoded=NULL;
									printf("viewer: gzip auto decode failed\n");
								} else {
									printf("viewer: decoded gzip compression\n");
								}
							}
						}
						if (decoded == NULL) {
							dsize = size;
							decoded = c;
						} else {
							free(c);
						}
						c = NULL;
						size = 0;
					}
					if (decoded!=NULL && wpbuff==NULL && wpsize && wpcmd!=FT_BINARY) {
						strcpy(wppath, setting->wallpaperpath);
						wpbuff = decoded;
						wpsize = dsize;
						wptype = !FT_BINARY;
						printf("wallpapersetup: cached file: %s ( %d bytes )\n", wppath, wpsize);
					}
				}
			}
			if (decoded) {
				//	drawMsg(lang->gen_decoding);
				type = formatcheck(decoded, dsize);
				i = info_image(info, decoded, dsize, type);
				printf("wallpapersetup: info_image returned: %d\n", i);
				printf("wallpapersetup: info data: %d,%dx%d(%dbpp),%d image(s)\n", info[0], info[2], info[3], info[1], info[4]);
				if (i && info[2] * info[3]) {
					buffer = (char*)malloc(info[2] * info[3] * bpp);
					if (!buffer && bpp>2) {
						bpp = 2; 
						buffer = (char*)malloc(info[2] * info[3] * bpp);
					}
					if (!buffer && bpp>1) {
						bpp = 1;
						buffer = (char*)malloc(info[2] * info[3] * bpp);
					}
					if (buffer) {
						drawMsg(lang->conf_wallpaperdecode);
						i = decode_image(buffer, decoded, dsize, bpp, info, type);
						printf("wallpapersetup: decode_image(%dbpp) returned: %d\n", bpp*8, i);
						if (i < 0) {
							free(buffer);
							buffer = NULL;
						}
					}
					if (wpcmd==FT_BINARY && wpbuff==decoded) wpbuff = NULL;
					if (buffer!=NULL && wpbuff==NULL && wpsize && wpcmd==FT_BINARY) {
						if (bpp > 2) {
							strcpy(wppath, setting->wallpaperpath);
							wpbuff = buffer;
							wpsize = info[2] * info[3] * bpp;
							wpbpp = bpp;
							wpalpha = alphablend;
							wptype = FT_BINARY;
							for(i=0;i<8;i++)	wpinfo[i] = info[i];
							printf("wallpapersetup: cached image: %s\n", wppath);
						} else {
							strcpy(wppath, setting->wallpaperpath);
							wpbuff = decoded;
							wpsize = dsize;
							wptype = !FT_BINARY;
							printf("wallpapersetup: cached file: %s ( %d bytes )\n", wppath, wpsize);
						}
					}
				}
			}
			if (buffer) {
				if(strcmp(wpold, setting->wallpaperpath))
					drawMsg(lang->conf_wallpaperresize);
				else
					drawMsg(lang->conf_wallpaperreload);
				int y,x,z,ww,st,sl,dt,dl,w,h,sw,sh,dw,dh,sblt,tt;	float fx,fy; fx=fy=1.0f;
				if (dither) dither = (ffmode && interlace) +1;
				if (tb > 2)	memset32(scrnbuff, setting->color[COLOR_BACKGROUND], scrnsize);
				else if (!dither)
							memset16(scrnbuff, truetohigh(setting->color[COLOR_BACKGROUND],0,0), scrnsize);
				else {	
					int temp[4];
					uint64 tmpdst[4];
					temp[0] = temp[1] = temp[2] = temp[3] = (setting->color[COLOR_BACKGROUND] & 0xFFFFFF) | 0x80 << 24;
					stretchblt((char*)&tmpdst[0], (char*)temp, 4, 2, 4/ff, 2, 2, 4, 0, 0, 0, 0, 4, 4/ff, 2, 2, 0, dither, 0);
					if (ff - 1)	
						stretchblt((char*)&tmpdst[2], (char*)temp, 4, 2, 4/ff, 2, 2, 4, 0, 0, 0, 0, 4, 4/ff, 2, 2, 1, dither, 0);
					//	stretchblt(scrnbuff +(th/2*tw*tb), (char*)temp, tw, 2, th/ff, 2, tb, 4, 0, 0, 0, 0, tw, th, 2, 2, 1, dither, 0);
					//	for(y=0;y<4;y++)
					//		printf("	%016lx\n", tmpdst[y]);
					//	 0988,0567,0988,0567
					//	 0567,0988,0567,0988
					//	 0988,0567,0988,0567
					//	 0567,0988,0567,0988
					for(y=0,z=3/ff;y<SCREEN_HEIGHT;y++) {
						memsetdither(&scrnbuff[y*tw*tb], tmpdst[y&z], tw*tb);
						if(ff-1)
							memsetdither(&scrnbuff[(y+th/2)*tw*tb], tmpdst[(y&z)+2], tw*tb);
					}
				}
				w = info[2]; h = info[3];
				dw = SCREEN_WIDTH;	sw = w; sl = st = 0;
				dh = SCREEN_HEIGHT;	sh = h; dl = dt = 0;
				tt = tw;
				if (tt > 2048) tt = 2048;
				if (dw > 2048) dw = 2048;
				printf("wallpapersetup: clipmode:%d\n", setting->wallpaper[0].clipmode);
				sblt = 0;
				switch(setting->wallpaper[0].clipmode) {
					case 0:	// 中央に表示
						dl = (dw - w   ) / 2;
						dt = (dh - h/ff) / 2;
						printf("wallpapersetup: screen:(%d,%d)-(%dx%d), src:(%d,%d)-(%dx%d)\n", dl, dt, dw, dh, sl, st, sw, sh);
						if (tb == bpp) {
							if (dl < 0) {sl=-dl   ; dl=0;} else {sl=0; dw-=dl*2;}
							if (dt < 0) {st=-dt*ff; dt=0;} else {st=0; dh-=dt*2;}
						//	if (dl + sw    > dw) sw = dw - dl;
						//	if (dt + sh/ff > dh) sh = (dh - dt) * ff;
							if (ffmode&&interlace) {
								for(y=0;y<dh;y++){
									memcpy(&scrnbuff[((y+dt+   0)*tt+dl)*bpp], &buffer[((y*ff+0+st)*w+sl)*bpp], sw*bpp);
									memcpy(&scrnbuff[((y+dt+th/2)*tt+dl)*bpp], &buffer[((y*ff+1+st)*w+sl)*bpp], sw*bpp);
								}
							} else {
								for(y=0;y<dh;y++){
									memcpy(&scrnbuff[((y+dt)*tt+dl)*bpp], &buffer[((y+st)*w+sl)*bpp], sw*bpp);
								}
							}
						} else {
							dw = sw; dh = sh / ff; sl = st = 0; sblt = 1;
						}
						break;
					case 1:	// 並べて表示
						if (bpp == tb) {
							if (ffmode&&interlace) {
								for(z=0;z<dh;z+=sh/2){
									for(y=0;y<sh/2;y++){
										if (z+y >= dh) break;
										for(x=0;x<dw;x+=sw){
											ww = sw; if (x+ww > dw) ww = dw - x;
											memcpy(&scrnbuff[((z+     y+dt)*tt+x+dl)*bpp], &buffer[((y*2+0+st)*w+sl)*bpp], ww*bpp);
											memcpy(&scrnbuff[((z+th/2+y+dt)*tt+x+dl)*bpp], &buffer[((y*2+1+st)*w+sl)*bpp], ww*bpp);
										}
									}
								}
							} else {
								for(z=0;z<dh;z+=sh){
									for(y=0;y<sh;y++){
										if (z+y >= dh) break;
										for(x=0;x<dw;x+=sw){
											ww = sw; if (x+ww > dw) ww = dw - x;
											memcpy(&scrnbuff[((z+y+dt)*tt+x+dl)*bpp], &buffer[((y+st)*w+sl)*bpp], ww*bpp);
										}
									}
								}
							}
						} else {
							
							stretchblt(scrnbuff, buffer, tw, sw, th/ff, sh, tb, bpp, 0, 0, 0, 0, sw, sh/ff, sw, sh, 0, dither, 0);
							if (ffmode && interlace)
								stretchblt(scrnbuff +(th/2*tw*tb), buffer, tw, sw, th/ff, sh, tb, bpp, 0, 0, 0, 0, sw, sh/ff, sw, sh, 1, dither, 0);
							for(z=0;z<dh;z+=sh/ff){
								for(y=0;y<sh/ff;y++){
									if (z+y >= dh) break;
									for(x=0;x<dw;x+=sw){
										if(!x&&!z)continue;
										ww = sw; if (x+ww > dw) ww = dw - x;
										memcpy(&scrnbuff[((z+     y)*tt+x)*tb], &scrnbuff[((y     )*tt)*tb], ww*tb);
										if (ff-1)
											memcpy(&scrnbuff[((z+th/2+y)*tt+x)*tb], &scrnbuff[((y+th/2)*tt)*tb], ww*tb);
									}
								}
							}
						}
						break;
					case 2:	// 拡大(内側)
						fx = 1.0f * dw / sw;	sblt = 1;
						fy = 1.0f * dh / sh * ff;
						if (fx > fy) fx = fy; else fy = fx;
						break;
					case 3:	// 拡大(外側)
						fx = 1.0f * dw / sw;	sblt = 1;
						fy = 1.0f * dh / sh * ff;
						if (fx < fy) fx = fy; else fy = fx;
						break;
					case 4:	// 拡大(全体)
						fx = 1.0f * dw / sw;	sblt = 1;
						fy = 1.0f * dh / sh * ff;
						break;
				}
				if (sblt) {
					dw = fx * sw +.5;	dh = fy * sh / ff +.5;
					dl = (SCREEN_WIDTH - dw) >> 1;
					dt = (SCREEN_HEIGHT -dh) >> 1;
					printf("wallpapersetup: screen:(%d,%d)-(%dx%d), src:(%d,%d)-(%dx%d)\n", dl, dt, dw, dh, sl, st, sw, sh);
					stretchblt(scrnbuff, buffer, tw, sw, th/ff, sh, tb, bpp, dl, dt, sl, st, dw, dh, sw, sh, 0, dither, 1);
					if (ff - 1)	stretchblt(scrnbuff +(th/2*tw*tb), buffer, tw, sw, th/ff, sh, tb, bpp, dl, dt, sl, st, dw, dh, sw, sh, 1, dither, 1);
				}
				setting->wallpaper[0].sl = sl;
				setting->wallpaper[0].st = st;
				setting->wallpaper[0].sw = sw;
				setting->wallpaper[0].sh = sh;
				setting->wallpaper[0].dl = dl;
				setting->wallpaper[0].dt = dt;
				setting->wallpaper[0].dw = dw;
				setting->wallpaper[0].dh = dh;
				wallpaper = setting->wallpaper[0].flag;
				//	free(buffer);
				//	imagedump_bmp(buffer, sw, sh, bpp, activeclut, "host:/wpsrc.bmp");
				//	imagedump_bmp(scrnbuff, tw, th, tb, activeclut, "host:/wpscr.bmp");
				
				if (activeclut != clut) {
					activeclut = clut;
					setclut16(activeclut, 0);
					setclut216(activeclut, 32);
				}
				
				//	decoded = (char*)malloc(info[2]*info[3]);
				//	if (decoded) {
				//		stretchblt(decoded, buffer, info[2], info[2], info[3], info[3], 1, bpp, 0, 0, 0, 0, info[2], info[3], info[2], info[3], 0);
				//		imagedump_bmp(decoded, info[2], info[3], 1, activeclut, "host:/8bpp.bmp");
				//		free(decoded);
				//	}
				strcpy(wpold, setting->wallpaperpath);
				wpvmode = tvmode; wpclip = setting->wallpaper[0].clipmode;
			} else {
				free(scrnbuff);
				scrnbuff = NULL;
				scrnsize = 0;
				wallpaper = 0;
				wpold[0] = 0;
			}
			
			if (decoded == wpbuff)	decoded = NULL;
			if (decoded)			free(decoded);
			if (buffer == wpbuff)	buffer = NULL;
			if (buffer)				free(buffer);
		}
		if (!wpold[0])	drawMsg(lang->conf_wallpapererror);
		printf("wallpapersetup: result wallpaper:%d scrnbuff:%08X scrnsize:%d\n", wallpaper, (int)scrnbuff, scrnsize);
	}
}
void X_itoVSync(void)
{	// 転送処理
	//printf("%9d\r", totalcount);
//	enum{PAD_SCRNSHOT = PAD_L3 | PAD_R3};
//	enum{PAD_SCRNSHOT = PAD_R3};
//	enum{PAD_SCRNSHOT = PAD_R3, PAD_SCRNBACK = PAD_L3};
	enum{PAD_SCRNSHOT = PAD_R3, PAD_SCRNBACK = PAD_L3};
//	static char *scrnbuff=NULL; static int scrnsize=0;
#define	PAD_SCRNSHOT	setting->screenshotbutton
	itoNoVSync();
	if (setting->screenshotenable && (new_pad & PAD_SCRNSHOT) && ((paddata & PAD_SCRNSHOT) == PAD_SCRNSHOT)) {
		u8 temp[MAX_PATH];
		new_pad &= ~PAD_SCRNSHOT;
		sprintf(temp, "%s%s", setting->screenshotpath, "SCRNSHOT.BMP");
	//	printf("%s\n", temp);
		screenshot_to_bmp(temp);
	//	screenshot_to_bmp("host:SCRNSHOT.BMP");
	} else if ((new_pad & PAD_SCRNBACK) && ((paddata & PAD_SCRNBACK) == PAD_SCRNBACK)) {
		if (scrnbuff) wallpaper = (wallpaper +1) % 3;
	}
	itoVSync();
#undef	PAD_SCRNSHOT
}
void screen_tex_dump(char *scrnbuff, int size){//, int width, int height, int *bpp) {
//	tw = 1024; th = 1024; if (size > 2 * 1048576) tb = 4; else tb = 4 - screen_depth;
//	while(size <= tw*th*tb/2)	th>>=1;
	printf("screen_tex_dump: %7d => %4dx%4d ( %d bpp ) = %7d\n", size, tw, th, tb*8, tw*th*tb);
//	return;
	{
		int y,w,base;
		if (ffmode && interlace) {
			base = itoGetFrameBufferBase(1) >> 8;
			for(y=0,w=0;y<th>>1;y+=64,w+=tw*tb*64){
			//	printf("screen_tex_dump1: [%4d]: %7d / %7d\n", y, w, size);
				if (w >= size) break;
				ps2_screenshot(&scrnbuff[w],    0, 0, y, tw, 64, 4-tb);
			}
			for(y=0,w=size>>1;y<th>>1;y+=64,w+=tw*tb*64){
			//	printf("screen_tex_dump2: [%4d]: %7d / %7d\n", y, w, size);
				if (w >= size) break;
				ps2_screenshot(&scrnbuff[w], base, 0, y, tw, 64, 4-tb);
			}
		} else {
			// 0x200000
			base = itoGetFrameBufferBase(itoGetVisibleFrameBuffer()) >> 8;
			for(y=0,w=0;y<th;y+=64,w+=tw*tb*64){
			//	printf("screen_tex_dump0: [%4d]: %7d / %7d\n", y, w, size);
				if (w >= size) break;
				ps2_screenshot(&scrnbuff[w], base, 0, y, tw, 64, 4-tb);
			}
		}
	}
}
int screen_tex_size(void) {
	tw = (SCREEN_WIDTH +63) & ~63; th = (SCREEN_HEIGHT +63) & ~63; tb = 4 - screen_depth;
	th *= ((ffmode && interlace) +1);
	//	if (tw > 2048) tw = 2048;
	return tw * th * tb;
	int size=1048576*4,sz=SCREEN_WIDTH * SCREEN_HEIGHT * (4 - screen_depth) * ((ffmode && interlace) +1);
	while(sz <= size/2) size >>= 1;
	printf("screen_tex_size: %4dx%4d ( %d bpp ) = %7d ( %7d )\n", SCREEN_WIDTH, SCREEN_HEIGHT, (4-screen_depth)*8, size, sz);
	return size;
}
int screenshot_getsize(void) {
//	int vmode = setting->tvmode;	if (!gsregs[tvmode].loaded) vmode = (ITO_VMODE_AUTO)-1;
	return SCREEN_WIDTH * SCREEN_HEIGHT * (4 - screen_depth) * ((ffmode && interlace) +1);
}
int screenshot_temp(int fd, char *data) {
//	printf("ss_tobuff: 64line=%6d\n", ((SCREEN_WIDTH+63)&~63) * 64 * (4 - screen_depth));
	u8 *temp,*d=data,*tmpb;
	int width,height,bpp,base,y,i,w,z,sz=0;
	
	width = (SCREEN_WIDTH + 63) & ~63;
	height = SCREEN_HEIGHT * ((ffmode&&interlace)+1);
	bpp = 4 - screen_depth;
	if (width > 2048) {
		width = ((width >> 1) + 63) & ~63;
		height <<= 1;
	//	width = 2048;
	//	height = 2048 / bpp;
	}
	printf("screenshot: screen: %dx%d, %dbpp (psm:%d)\n", width, height, bpp*8, screen_depth);
	if (ffmode) {
		base = itoGetFrameBufferBase(1) >> 8;
	} else {
		base = itoGetFrameBufferBase(itoGetVisibleFrameBuffer()) >> 8;
	}
	
	temp = (char*)malloc(width * bpp * 66);
	if (temp == NULL) return -2;
	tmpb = temp + (width * bpp * 64);
	
	for(y=0,w=0;y<height;y+=64,w+=width*bpp*64){
		if (ffmode) {
			if (interlace) {
				for(i=0;i<32;i++){
					ps2_screenshot(temp + (width*bpp*(i*2+0)), 0   , 0, (y>>1)+i, width, 1, screen_depth);
					ps2_screenshot(temp + (width*bpp*(i*2+1)), base, 0, (y>>1)+i, width, 1, screen_depth);
				}
			} else {
				ps2_screenshot(temp, 0   , 0, y, width, 64, screen_depth);
			//	if (bpp > 2) {
				for(z=0;z<64;z++){
					if (y+z>=height) break;
					ps2_screenshot(tmpb, base, 0, y+z, width, 1, screen_depth);
					if (bpp > 2) {
						for(i=0;i<width*bpp;i++){
							temp[z*width*bpp+i] = ((int)temp[z*width*bpp+i] + tmpb[i] +1) >> 1;
						}
					} else {
						int a,b;
						for(i=0;i<width;i++){
							a = 1*temp[z*width*bpp+i*2+0] | 256*temp[z*width*bpp+i*2+1];
							b = 1*tmpb[i*2+0] | 256*tmpb[i*2+1];
							a = ((a & 0x7BDE) + (b & 0x7BDE)) >> 1;
							temp[z*width*bpp+i*2+0] = (a >> 0) & 255;
							temp[z*width*bpp+i*2+1] = (a >> 8) & 255;
						}
					}
				}
			//	} 
			}
		} else {
			ps2_screenshot(temp, base, 0, y, width, 64, screen_depth);
		}
		if (SCREEN_WIDTH & 63) {
			for(i=0;i<64;i++){
				if (y+i>=height) break;
				if (fd>=0)	nwrite(fd , &temp[i*width*bpp], SCREEN_WIDTH*bpp);
				else		memcpy(d+w, &temp[i*width*bpp], SCREEN_WIDTH*bpp);
				sz+=SCREEN_WIDTH*bpp;
			}
		} else {
			if (y+64 <= height) {
				if (fd>=0)	nwrite(fd , temp, width*64*bpp);
				else		memcpy(d+w, temp, width*64*bpp);
				sz += width*64*bpp;
			} else {
				if (fd>=0)	nwrite(fd , temp, width*(height & 63)*bpp);
				else		memcpy(d+w, temp, width*(height & 63)*bpp);
				sz += width*(height & 63)*bpp;
			}
		}
	}
//	if (fd) fioWrite(fd, 0, 0);
//	printf("size: %d\n", sz);
	free(temp);
	return 0;
}
int screenshot_tobuff(char *dist) {
	return screenshot_temp(-1, dist);
}
unsigned char bmpheader[54] = "BM____\x00\x00\x00\x00\x42\x00\x00\x00\x28\x00" "\x00\x00________\x01\x00\x08\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\xA0\x05\x00\x00\xA0\x05\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00";
u32 bf16[3]={0x001F,0x03E0,0x7C00},bf32[3]={0x0000FF,0x00FF00,0xFF0000};
int screenshot_to_bmp(char *filename) {
	int width,height,bpp;
	int fd;
	//width = (SCREEN_WIDTH + 63) & ~63;
	width = SCREEN_WIDTH;
	height = SCREEN_HEIGHT * ((ffmode&&interlace)+1);
	bpp = 4 - screen_depth;
	if (width > 2048) {
		width = ((width >> 1) + 63) & ~63;
		height <<= 1;
	}
	fd = nopen(filename, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		printf("viewer: target open error!\n");
	} else {
		seti32(bmpheader+2, width*height*bpp+54+12);
		seti32(bmpheader+10, 54+12);
		seti32(bmpheader+18, width);
		seti32(bmpheader+22, -height);
		seti16(bmpheader+28, bpp*8);
		seti32(bmpheader+30, 3);
		nwrite(fd, bmpheader, 54);
		if (bpp > 2)	nwrite(fd, bf32, 12);
		else			nwrite(fd, bf16, 12);
		screenshot_temp(fd, NULL);
		itoNoVSync();
		nclose(fd);
	}//*/
	return 0;
}
int imagedump_bmp(char *buff, int width, int height, int bpp, uint64 *clut, char *filename) {
	int fd,i;
	u32 bpal[256];
	fd = nopen(filename, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		printf("viewer: target open error!\n");
	} else {
		int cluts=(bpp==1)*1024+(bpp==0)*64,bitfields=(bpp>=2)*12;
		int bpl=(width*bpp+3) & ~3;
		seti32(bmpheader+2, height*bpl+14+40+bitfields+cluts);
		seti32(bmpheader+10, 14+40+bitfields+cluts);
		seti32(bmpheader+18, width);
		seti32(bmpheader+22, -height);
		seti16(bmpheader+28, bpp*8);
		seti32(bmpheader+30, bitfields/4);	// 0:raw 1-2:rle, 3:bitfield
		nwrite(fd, bmpheader, 54);
		if (bitfields) {
			if (bpp > 2)	nwrite(fd, bf32, 12);
			else			nwrite(fd, bf16, 12);
		} else if (cluts) {
			for(i=0;i<cluts/4;i++)
				bpal[i]=((clut[i] >> 16) & 0x0000FF)	//	bed
				|		( clut[i]        & 0x00FF00)	//	green
				|		((clut[i] << 16) & 0xFF0000)	//	red
				|		( clut[i]      & 0xFF000000);	//	reserved
			nwrite(fd, bpal, cluts);
		}
		if ((width*bpp) & 3) {
			for(i=0;i<height;i++){
				nwrite(fd, &buff[i*width*bpp], width*bpp);
				nwrite(fd, &bitfields, (width*bpp) & 3);
			}
		} else
			nwrite(fd, buff, width*height*bpp);
		nclose(fd);
	}//*/
	return 0;
};
