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
#define LBFN_VER "LbFn v0.70.13"

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
	COLOR_GRAYTEXT,
	COLOR_DIR,
	COLOR_FILE,
	COLOR_PS2SAVE,
	COLOR_PS1SAVE,
	COLOR_ELF,
	COLOR_PSU,
	COLOR_TXT,
	COLOR_OUTSIDE,
	NUM_COLOR
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
	DIRELF dirElf[MAX_BUTTON];
	int timeout;
	int filename;
	int fileall;
	uint64 color[NUM_COLOR];
	int flicker_alpha;
	int discControl;
	int tvmode;
	int fileicon;
	int discPs2saveCheck;
	int discELFCheck;
	int filePs2saveCheck;
	int fileELFCheck;
	char Exportdir[MAX_PATH];
	int defaulttitle;
	int defaultdetail;
	int sort;
	int sortdir;
	int sortext;
	int language;
	char lang_path[MAX_PATH];
	char AsciiFont[MAX_PATH];
	char KanjiFont[MAX_PATH];
	char LangFont[MAX_PATH];
	int CharMargin;
	int LineMargin;
	int FontBold;
	int AsciiMarginTop;
	int AsciiMarginLeft;
	int KanjiMarginTop;
	int KanjiMarginLeft;
	int	LangMarginTop;
	int	LangMarginLeft;
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
	int mcraw_flag;
	char mcdump_path[MAX_PATH];
	char mcstore_path[MAX_PATH];
	char kbd_sbcspath[MAX_PATH];
	char kbd_dbcspath[MAX_PATH];
	char kbd_extpath[MAX_PATH];
	char kbd_histpath[MAX_PATH];
	char kbd_dicpath[MAX_PATH];
	char sav_db_path[MAX_PATH];
	int sav_ps1save;
	int sav_ps2save;
	int sav_pspsave;
	int sav_psusave;
	int sav_startup;
	int sav_mode;
	int cnf_compress;
	int txt_linenumber;
	int txt_tabmode;
	int txt_chardisp;
	int txt_wordwrap;
	int img_fullscreen;
	int img_resize;
	int txt_autodecode;
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
	char skin_image[MAX_PATH];
	SKINDATA skin[MAX_SKIN];
	unsigned short *skbd_history;
	unsigned char *skbd_dictionary;
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
extern int SCANRATE;
void setup_vsync();
void drawDark(void);
int drawDarks(int ret);
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2);
void setScrTmp(const char *msg0, const char *msg1);
void drawMsg(const char *msg);
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
int checkFONTX2header(const char *path);
void drawChar(unsigned char c, int x, int y, uint64 colour);
int printXY(const unsigned char *s, int x, int y, uint64 colour, int draw);
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
#define MB_OK           0x00000000
#define MB_OKCANCEL     0x00000001
#define MB_YESNOCANCEL  0x00000003
#define MB_YESNO        0x00000004
#define MB_MC0MC1CANCEL 0x0000000F
#define MB_DEFBUTTON1   0x00000000
#define MB_DEFBUTTON2   0x00000100
#define MB_DEFBUTTON3   0x00000200
#define MB_USETRIANGLE  0x00100000
#define MB_USESQUARE    0x00200000
#define	MB_USETIMEOUT	0x00400000
#define IDOK         0x0001
#define IDCANCEL     0x0002
#define IDYES        0x0006
#define IDNO         0x0007
#define IDMC0        0x0010
#define IDMC1        0x0011
#define IDTRIANGLE   0x0100
#define IDSQUARE     0x0200
#define	IDTIMEOUT    0x0400
int MessageBox(const char *Text, const char *Caption, int type);
char* getExtension(const char *path);
int newdir(const char *path, const char *name);
#ifdef ENABLE_PSB
int psb(const char *psbpath);
#endif
int keyboard(char *out, int max);
void getFilePath(char *out, const int cnfmode);

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
int formatcheck(unsigned char *buff, unsigned int size);
int formatcheckfile(char *file);
int set_viewerconfig(int linedisp, int tabspaces, int chardisp, int screenmode, int textwrap, int drawtype);
//int set_viewerconfig(int *conf);

/* misc.c */
extern char LBF_VER[];
#endif
