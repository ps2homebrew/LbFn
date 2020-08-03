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
#include "cnf.h"

// バージョン
#define LBF_VER "LbFn v0.70.10"

// 垂直スキャンレート
#define SCANRATE (ITO_VMODE_AUTO==ITO_VMODE_NTSC ? 60:50)

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
	MAX_PATH = 1025,
	MAX_ENTRY = 2048,
	MAX_PARTITIONS=100
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
	NUM_COLOR
};

typedef struct
{
	char dirElf[13][MAX_PATH];
	int timeout;
	int filename;
	uint64 color[NUM_COLOR];
	int flicker_alpha;
	int screen_x_480i;
	int screen_y_480i;
	int screen_x_480p;
	int screen_y_480p;
	int screen_x_1080i;
	int screen_y_1080i;
	int screen_x_720p;
	int screen_y_720p;
	int discControl;
	int flickerControl;
	int tvmode;
	int interlace;
	int ffmode_480i;
	int ffmode_1080i;
	int screen_scan_480i;
	int screen_scan_480p;
	int screen_scan_1080i;
	int screen_scan_720p;
	int fullhd_width;
	int fileicon;
	int discPs2saveCheck;
	int discELFCheck;
	int filePs2saveCheck;
	int fileELFCheck;
	char Exportdir[MAX_PATH];
	int defaulttitle;
	int defaultdetail;
	int language;
	char AsciiFont[MAX_PATH];
	char KanjiFont[MAX_PATH];
	int CharMargin;
	int LineMargin;
	int FontBold;
	int FontHalf_480i;
	int FontHalf_480p;
	int FontHalf_720p;
	int FontHalf_1080i;
	int FontVHalf_480i;
	int FontVHalf_480p;
	int FontVHalf_720p;
	int FontVHalf_1080i;
	int FontScaler_480i;
	int FontScaler_480p;
	int FontScaler_720p;
	int FontScaler_1080i;
	int AsciiMarginTop;
	int AsciiMarginLeft;
	int KanjiMarginTop;
	int KanjiMarginLeft;
	int usbd_flag;
	char usbd_path[MAX_PATH];
	int usbmass_flag;
	char usbmass_path[MAX_PATH];
	int usbkbd_flag;
	char usbkbd_path[MAX_PATH];
	int usbmouse_flag;
	char usbmouse_path[MAX_PATH];
} SETTING;

/* main.c */
extern char LaunchElfDir[MAX_PATH], LastDir[MAX_NAME];
extern int boot;
void loadCdModules(void);
void loadUsbModules(void);
void loadUsbMassModules(void);
void loadUsbKbdModules(void);
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
extern itoGsEnv screen_env;
extern int SCREEN_LEFT;
extern int SCREEN_TOP;
extern int interlace;
extern int ffmode;
extern int screenscan;
extern int font_half;
extern int font_vhalf;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int SCREEN_MARGIN;
extern int FONT_WIDTH;
extern int FONT_HEIGHT;
extern int MAX_ROWS;
extern int MAX_ROWS_X;
extern int framebuffers;
void drawDark(void);
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
void SetScreenPosVM();
void InitScreenSetting(void);
void loadConfig(char *);
void config(char *);

/* filer.c */
enum	//getFilePath
{
	ANY_FILE = 0,
	ELF_FILE,
	DIR,
	FNT_FILE,
	IRX_FILE,
	JPG_FILE,
	TXT_FILE
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
#define IDOK         0x0001
#define IDCANCEL     0x0002
#define IDYES        0x0006
#define IDNO         0x0007
#define IDMC0        0x0010
#define IDMC1        0x0011
#define IDTRIANGLE   0x0100
#define IDSQUARE     0x0200
int MessageBox(const char *Text, const char *Caption, int type);
char* getExtension(const char *path);
int newdir(const char *path, const char *name);
#ifdef ENABLE_PSB
int psb(const char *psbpath);
#endif
int keyboard(char *out, int max);
void getFilePath(char *out, const int cnfmode);

/* language.c */
extern LANGUAGE *lang;
void InitLanguage(void);
void FreeLanguage(void);
void SetLanguage(const int langID);

/* tek.c */
int tek_getsize(unsigned char *p);
int tek_decomp(unsigned char *p, char *q, int size);

#endif
