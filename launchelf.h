#ifndef LAUNCHELF_H
#define LAUNCHELF_H

#include <stdio.h>
#include <tamtypes.h>
#include <sifcmd.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <string.h>
#include <malloc.h>
#include <libhdd.h>
#include <libmc.h>
#include <libpad.h>
#include <fileio.h>
#include <sys/stat.h>
#include <iopheap.h>
#include <errno.h>
#include <fileXio_rpc.h>
#include <iopcontrol.h>
#include <stdarg.h>
#include <sbv_patches.h>
#include <sys/fcntl.h>
#include <ito.h>
#include <cdvd_rpc.h>
#include <smod.h>

#include "cd.h"
#include "language.h"
#include <libkbd.h>
#include <libmouse.h>

// バージョン
#define LBFN_VER "LbFn v0.70.18"

// 垂直スキャンレート
//#define SCANRATE (ITO_VMODE_AUTO==ITO_VMODE_PAL ? 50:60)
//#define	SCANRATE	(gsregs[setting->tvmode].vmode == ITO_VMODE_PAL ? 50:60)

#define UNK_BOOT 0
#define CD_BOOT 1
#define MC_BOOT 2
#define HOST_BOOT 3
#define PFS_BOOT 4
#define VFS_BOOT 5
#define HDD_BOOT 6
#define MASS_BOOT 7
#define PCSX_BOOT 8

enum
{
	MAX_NAME = 256,
	MAX_TITLE = 64,
	MAX_PATH = 512,
	MAX_ENTRY = 2048,
	MAX_PARTITIONS=100,
	MAX_BUTTON = 32,
	MAX_ELF = 5,
	MAX_GSREG = 32,
	MAX_SKIN = 3,
};

enum
{
	COLOR_BACKGROUND,
	COLOR_FRAME,
	COLOR_TEXT,
	COLOR_HIGHLIGHTTEXT,
//	COLOR_CONTROLTEXT,
	COLOR_GRAYTEXT,
	COLOR_SHADOWTEXT,
	COLOR_DIR,
	COLOR_FILE,
	COLOR_PS2SAVE,
	COLOR_PS1SAVE,
	COLOR_ELF,
	COLOR_PSU,
	COLOR_TXT,
	COLOR_OUTSIDE,
	NUM_COLOR,
	
	HTML_BACK=0,
	HTML_TEXT,
	HTML_GRAYTEXT,
	HTML_LINK,
	HTML_ALINK,
	HTML_VLINK,
	HTML_NONE,
	HTML_FOUND,
	HTML_TRIP,
	HTML_COLORS,
};

enum
{
	TXT_AUTO,
	TXT_BINARY=TXT_AUTO,
	TXT_LANG=TXT_AUTO,
	TXT_ASCII,
	TXT_SJIS,
	TXT_EUCJP,
	TXT_JIS,
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
	FT_P2S,
	FT_P2T,
	FT_PS1,
	FT_ICO,
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
	FT_MID,
	FT_EXE,
	FT_TYPES,
};

typedef struct
{
	char name[MAX_TITLE];
	char path[MAX_ELF][MAX_PATH];
	int padmsk;
} DIRELF;

typedef struct
{
	void *tex;
	int mode;
	int dp;
	int tx;
	int ty;
	int tw;
	int th;
} SKINDATA;

typedef struct
{
	int flag;
	int clipmode;
	int brightness;
	int contrast;
	int sl,st,sw,sh;	// src/nouse
	int dl,dt,dw,dh;	// dst/nouse
} WALLPAPER;

enum{
	WP_SCREEN, 
	WP_WINDOW, 
	wp_has
};

typedef struct
{
	DIRELF dirElf[MAX_BUTTON];
	int timeout;
	int filename;
	int fileall;
	uint64 clr[HTML_COLORS];
	uint64 color[NUM_COLOR];
	int flicker_alpha;
	int discControl;
	int tvmode;
	int fileicon;
	int discPs2saveCheck;
	int discELFCheck;
	int filePs2saveCheck;
	int fileELFCheck;
	int getsizecrc32;
	int exportname;
	char Exportdir[MAX_PATH];
	int defaulttitle;
	int defaultdetail;
	int sort;
	int sortdir;
	int sortext;
	int language;
//	char lang_path[MAX_PATH];
	char AsciiFont[MAX_PATH];
	char KanjiFont[MAX_PATH];
//	char LangFont[MAX_PATH];
	int fontcache;
	int disablectrl;
	int CharMargin;
	int LineMargin;
	int FontBold;
	int AsciiMarginTop;
	int AsciiMarginLeft;
	int KanjiMarginTop;
	int KanjiMarginLeft;
//	int	LangMarginTop;
//	int	LangMarginLeft;
	int usbd_flag;
	char usbd_path[MAX_PATH];
	int usbmass_flag;
	char usbmass_path[MAX_PATH];
	int usbkbd_flag;
	char usbkbd_path[MAX_PATH];
	int usbmouse_flag;
	char usbmouse_path[MAX_PATH];
	int usbmdevs;
	int vmc_flag;
	char vmc_path[MAX_PATH];
//	int mcraw_flag;
//	char mcdump_path[MAX_PATH];
//	char mcstore_path[MAX_PATH];
//	char kbd_sbcspath[MAX_PATH];
//	char kbd_dbcspath[MAX_PATH];
//	char kbd_dicpath[MAX_PATH];
	int kbd_update;
	int screenshotbutton;
	int screenshotformat;
	int screenshotenable;
	int screenshotconfig[16][4];
	char screenshotpath[MAX_PATH];
	char downloadpath[MAX_PATH];
	char wallpaperpath[MAX_PATH];	// source image
	WALLPAPER wallpaper[wp_has];// fullscreen and popup window
//	char temphtmlpath[MAX_PATH];
//	char sav_db_path[MAX_PATH];
//	int sav_ps1save;
//	int sav_ps2save;
//	int sav_pspsave;
//	int sav_psusave;
//	int sav_startup;
//	int sav_mode;
	int cnf_compress;
	int txt_linenumber;
	int txt_tabmode;
	int txt_chardisp;
	int txt_wordwrap;
	int img_fullscreen;
	int img_resize;
	int txt_autodecode;
	int img_aniauto;
	int img_position;
	int snd_bgplay;
	int snd_volume;
	int snd_repeat;
	int img_sdtv_aspect;
	int img_pixel_aspect;
	short screen_left[MAX_GSREG];
	short screen_top[MAX_GSREG];
	short screen_width[MAX_GSREG];
	short screen_height[MAX_GSREG];
	char screen_depth[MAX_GSREG];
	char screen_dither[MAX_GSREG];
	char screen_interlace[MAX_GSREG];
	char screen_ffmode[MAX_GSREG];
	char screen_scan[MAX_GSREG];
	char font_half[MAX_GSREG];
	char font_vhalf[MAX_GSREG];
	char font_scaler[MAX_GSREG];
	char font_bold[MAX_GSREG];
	char flickerfilter[MAX_GSREG];
//	char skin_image[MAX_PATH];
//	SKINDATA skin[MAX_SKIN];
} SETTING;

/* main.c */
extern char LaunchElfDir[MAX_PATH], LastDir[MAX_NAME];
extern int boot;
extern int usbd,usbmass,usbkbd,usbmouse;
void loadCdModules(void);
void loadUsbModules(void);
void loadUsbMassModules(void);
void loadUsbKbdModules(void);
void loadUsbMouseModules(void);
void loadHddModules(void);
void loadHTTPModules(void);
int loadSoundModules(void);
void delay(int count);

/* elf.c */
int checkELFheader(const char *filename);
void RunLoaderElf(char *filename, char *);

/* draw.c */
enum	//SetFontMargin GetFontMargin
{
	CHAR_MARGIN = 0,
	LINE_MARGIN,
	ASCII_FONT_MARGIN_TOP,
	ASCII_FONT_MARGIN_LEFT,
	KANJI_FONT_MARGIN_TOP,
	KANJI_FONT_MARGIN_LEFT
};

enum	//GetFontSize
{
	ASCII_FONT_WIDTH = 0,
	ASCII_FONT_HEIGHT,
	KANJI_FONT_WIDTH,
	KANJI_FONT_HEIGHT
};

/*
enum	//GetCurrentPos
{
	CURRENTPOS_X = 0,
	CURRENTPOS_Y
};
*/
//"NTSC",640,448,1914,35,3,0,4,0,0,0,0,0,640,448,0,1,0,2,0,2,0,0,0,448
/*
typedef struct
{
	int	loaded;
	char name[64];
	short width;
	short height;
	short left;
	short top;
	char magx;
	char magy;
	char psm;
	short bufferwidth;
	short x1;
	short y1;
	short x2;
	short y2;
	short zleft;
	short ztop;
	char zpsm;
	char dither;
	char interlace;
	char ffmode;
	char vmode;
	char vesa;
	char frames;
	short f0_left;
	short f0_top;
	short f1_left;
	short f1_top;
	short defwidth;
	short defheight;
} GSREG;*/
typedef struct
{
	char name[65];
	char loaded;
	char magx;
	char magy;
	char psm;
	char dither;
	char interlace;
	char ffmode;
	unsigned char vmode;
	char vesa;
	char doublebuffer;
	char zpsm;
	short left;
	short top;
	short width;
	short height;
	short defwidth;
	short defheight;
} GSREG;
extern itoGsEnv screen_env;
extern GSREG gsregs[];
extern int SCREEN_LEFT;
extern int SCREEN_TOP;
extern int font_half;
extern int font_vhalf;
extern int font_bold;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int SCREEN_MARGIN;
extern int FONT_WIDTH;
extern int FONT_HEIGHT;
extern int MAX_ROWS;
extern int MAX_ROWS_X;
extern int flickerfilter;
extern int framebuffers;
extern int fieldbuffers;
extern int ffmode, interlace;
extern int fieldnow;
extern int screen_depth;
extern int SCANRATE;
extern int wallpaper;
extern char *font_ascii, *font_kanji;
extern unsigned int font_ascii_size, font_kanji_size;
extern uint64 totalcount;
void setup_vsync();
void drawDark(void);
int drawDarks(int ret);
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2);
void setScrTmp(const char *msg0, const char *msg1);
void drawMsg(const char *msg);
void drawBar(int x1, int y1, int x2, int y2, uint64 color, int ofs, int len, int size);
void setupito(int tvmode);
void clrScr(uint64 color);
void drawScr(void);
void SetHeight(void);
void drawFrame(int x1, int y1, int x2, int y2, uint64 color);
int InitFontAscii(const char *path);
int InitFontKnaji(const char *path);
void FreeFontAscii(void);
void FreeFontKanji(void);
int SetFontMargin(int type, int Margin);
int GetFontMargin(int type);
int GetFontSize(int type);
void SetFontBold(int flag);
int GetFontBold(void);
void SetFontHalf(int flag);
int GetFontHalf(void);
void SetFontVHalf(int flag);
int GetFontVHalf(void);
int checkMSWinheader(const char *path);
int checkFONTX2header(const char *path);
//void drawChar(unsigned char c, int x, int y, uint64 colour);
void drawChar_JIS(unsigned int c, int x, int y, uint64 fcol, uint64 scol, unsigned char *ctrlchars);
int printXY(const unsigned char *s, int x, int y, uint64 color, int draw);
int drawString(const unsigned char *s, int charset, int x, int y, uint64 fcol, uint64 scol, unsigned char *ctrlchars);
int drawStringLimit(const unsigned char *s, int charset, int sx, int sy, uint64 fcol, uint64 scol, unsigned char *ctrlchars, int right);
int drawStringAAS(const unsigned char *s, int sx, int sy, uint64 *col, int sl, int sr);
int drawStringWindow(const unsigned char *s, int charset, int sx, int sy, uint64 fcol, int sl, int sr);
int mkfontcache(int c, void *dist, int ofs, int limit);
int mkfontcaches(int start, int chars, void *dist, int ofs, int limit);
int mkfontcacheset(void);
void mkfontcacheclear(void);
int mkfontcachereset(void);

#ifdef ENABLE_ICON
void loadIcon(void);
int drawIcon(int x, int y, int w, int h, int id);
#endif
//int SetCurrentPos(int x, int y);
//int GetCurrentPos(int type);
//int printXY2(const unsigned char *s, uint64 color, int draw);

/* pad.c */
extern struct padButtonStatus buttons;
extern u32 new_pad;
extern u32 paddata;
int setupPad(void);
int readpad(void);
void waitPadReady(int port, int slot);

/* config.c */
extern SETTING *setting;
void SetScreenPosVM(void);
void InitScreenSetting(void);
void loadConfig(char *);
void ipconfig(char *);
void gsconfig(char *);
void fmcb_cfg(char *);
void config(char *);
void padmsktostr(char *dist, int mask, char *def);
int CheckMC(void);

/* filer.c */
enum	//getFilePath
{
	ANY_FILE = 0,
	ELF_FILE,
	DIR,
	FNT_FILE,
	IRX_FILE,
	JPG_FILE,
	TXT_FILE,
	FMB_FILE,
};
/* MessageBox */
enum{
	MB_OK           = 0x00000000, 
	MB_OKCANCEL     = 0x00000001, 
	MB_YESNOCANCEL  = 0x00000003, 
	MB_YESNO        = 0x00000004, 
	MB_MC0MC1CANCEL = 0x0000000F, 
	MB_DEFBUTTON1   = 0x00000000, 
	MB_DEFBUTTON2   = 0x00000100, 
	MB_DEFBUTTON3   = 0x00000200, 
	MB_USETRIANGLE  = 0x00100000, 
	MB_USESQUARE    = 0x00200000, 
	MB_USETIMEOUT   = 0x00400000, 
	IDOK         = 0x0001, 
	IDCANCEL     = 0x0002, 
	IDYES        = 0x0006, 
	IDNO         = 0x0007, 
	IDMC0        = 0x0010, 
	IDMC1        = 0x0011, 
	IDTRIANGLE   = 0x0100, 
	IDSQUARE     = 0x0200, 
	IDTIMEOUT    = 0x0400, 
};
int MessageBox(const char *Text, const char *Caption, int type);
char* getExtension(const char *path);
int newdir(const char *path, const char *name);
#ifdef ENABLE_PSB
int psb(const char *psbpath);
#endif
void getFilePath(char *out, const int cnfmode);
//PS2TIME uLaunchELF
typedef struct
{
	unsigned char unknown;
	unsigned char sec;	// date/time (second)
	unsigned char min;	// date/time (minute)
	unsigned char hour;	// date/time (hour)
	unsigned char day;	// date/time (day)
	unsigned char month;	// date/time (month)
	unsigned short year;	// date/time (year)
} PS2TIME __attribute__((aligned (2)));
typedef struct{
	PS2TIME createtime;
	PS2TIME modifytime;
	unsigned long fileSizeByte;
	unsigned short attr;
	char title[16*4+1];
	char name[256];
	int type;
	unsigned int timestamp;
	int num;
} FILEINFO;

/* language.c */
//extern char *lang[];
extern LANGUAGE *lang;
void InitLanguage(void);
void FreeLanguage(void);
void SetLanguage(const int langID);

/* cnf.c */
int cnf_init(void);
void cnf_free(void);
int cnf_inited(void);
int cnf_load(char* path);
int cnf_save(char* path);
int cnf_bload(char *buffer, size_t limit);
size_t cnf_bsave(char *buff, int maxbytes);
int cnf_getstr(const char* key, char *str, const char* def);
int cnf_setstr(const char* key, char *str);
int cnf_delkey(const char* key);
int cnf_session(const char *name);
int cnf_mode(int mode);

/* tek.c */
int tek_getsize(unsigned char *p);
int tek_decomp(unsigned char *p, char *q, int size);

/* tek_comp.c */
int tek_compfile(int type, char *infile, char *outfile);
int tek_comp(int type, char *src, int size, char *dst, int limit);

/* viewer.c */
int txteditfile(int mode, char *file);
int txtedit(int mode, char *file, unsigned char *buffer, unsigned int size);
int bineditfile(int mode, char *file);
int binedit(int mode, char *file, unsigned char *buffer, unsigned int size);
int imgviewfile(int mode, char *file);
int imgview(int mode, char *file, unsigned char *buffer, int w, int h, int bpp);
int imgdraw(unsigned char *buffer, int sw, int sh, int bpp, int dx, int dy, int dw, int dh);
int viewer_file(int mode, char *file);
int viewer(int mode, char *file, unsigned char *buffer, unsigned int size);
int fntview_file(int mode, char *file);
int fntview(int mode, char *file, unsigned char *buffer, unsigned int size);
int bbsview(int mode, char *file, unsigned char *c, unsigned int size);
int sndview(int mode, char *file, unsigned char *c, unsigned int size);
int pcmpause(void);
int pcmplay(void);
int pcmclear(void);
int pcmconfig(int mode, int rate, int channels, int bits);
int pcmspeed(int rate);
int pcmadd(char *buffer, int size);
int pcmadds(char *buffer, int sample, int rate, int ch, int bits, int type);
int pcmvolume(int volume);
int pcmregist(void (*callback)(void));
int pcmdelete(void);
int pcmquit(void);
int is_psu(unsigned char *buff, unsigned int size);
int is_video(unsigned char *buff, unsigned int size);
int is_audio(unsigned char *buff, unsigned int size);
int is_image(unsigned char *buff, unsigned int size);
int is_video_file(char *file);
int is_audio_file(char *file);
int is_image_file(char *file);
int formatcheck(unsigned char *buff, unsigned int size);
int formatcheckfile(char *file);
//int set_viewerconfig(int linedisp, int tabspaces, int chardisp, int screenmode, int textwrap, int drawtype);
int set_viewerconfig(int *conf);
void X_itoVSync(void);
void X_itoSprite(int left, int top, int right, int bottom, int type);
void X_clrScr(void);
void itoNoVSync(void);
int wallpapercache(int type);	// (bpp=0:only filecache)
int wallpaperfree(void);
void wallpapersetup(int reload);
int stretchblt(char *dist, char *src, 
				int ds, int ss, int db, int sb, int dp, int sp,
				int dl, int dt, int sl, int st, 
				int dw, int dh, int sw, int sh, 
				int ff, int dither, int filter);
int bitblt(char *dist, char *src, 
				int ds, int ss, int db, int sb, int dp, int sp,
				int left, int top, int width, int height,
				int ffmode, int field, int dither);
extern int sndview_redraw, pcmnplay;
extern unsigned char sndview_totaltime[16];

/* misc.c */
enum{
	SKBD_ALL=0,	
	SKBD_IP,
	SKBD_FILE,
	SKBD_TITLE,
};
extern char LBF_VER[];
int keyboard(int type, char *buff, int limit);
int NetworkDownload(char* msg0);
unsigned int CRC32Check(unsigned char *buff, unsigned int size);
unsigned int CRC32file(char *file);

/* deflate.c */
int inflate_chunk_add(u8 *src, u32 size);			// データチャンク追加用
void inflate_chunk_clr(void);						// データチャンクリスト初期化
void inflate_chunk_dst(u8 *dst, u32 limit);			// 展開先指定用
int inflate_chunk_exe(void);		// deflate圧縮のデコード開始
int gz_getsize(u8 *src, u32 size);					// 展開後のサイズ取得(ファイル全体)
int gzdecode(u8 *dst, u32 limit, u8 *src, u32 size);// deflate圧縮のデコード

/* fileio.c */
int nopen(char *path, int attr);
int nclose(int fd);
int nquit(char *path);
int nremove(char *path);
int nmkdir(char *path, char *dir, int attr);
int nrmdir(char *path);
int nseek(int fd, signed long ofs, int mode);
int ndopen(char *path);
int ndclose(int fd);
int ndread(int fd, void *dst);
unsigned int ngetc(int fd, char data);
unsigned int nputc(int fd, char data);
unsigned int ngets(int fd, void *dst, unsigned int limit);
unsigned int nread(int fd, void *dst, unsigned int size);
unsigned int nwrite(int fd, void *src, unsigned int size);

/* Libito0.2.1modify/itogsprims.c */
void itoPoint2c(uint64 color0, uint16 x0, uint16 y0, uint64 color1, uint16 x1, uint16 y1);
void itoAddVertex2(uint64 color1, uint16 x1, uint16 y1, uint64 color2, uint16 x2, uint16 y2);
void itoPoint2(uint64 color, uint16 x0, uint16 y0, uint16 x1, uint16 y1);
void itoLineX(uint64 color, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
void itoPointX(uint64 color, uint16 x, uint16 y);
void itoPoint2X(uint64 color, uint16 x0, uint16 y0, uint16 x1, uint16 y1);
void itoSpriteX(uint64 color, uint16 x1, uint16 y1, uint16 x2, uint16 y2);

#define	itoVSync	X_itoVSync
#endif
