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

//psbファイル実行機能の有無
//#define ENABLE_PSB

#include "cd.h"
#include "language.h"
#include "cnf.h"

// バージョン
#define LBF_VER "LbF v0.62"

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

typedef struct
{
	char dirElf[13][MAX_PATH];
	int timeout;
	int filename;
	uint64 color[9];
	int screen_x;
	int screen_y;
	int discControl;
	int flickerControl;
	int tvmode;
	int interlace;
	int ffmode;
	int fileicon;
	int discPs2saveCheck;
	int discELFCheck;
	char Exportdir[MAX_PATH];
	int language;
	char AsciiFont[MAX_PATH];
	char KanjiFont[MAX_PATH];
	int CharMargin;
	int LineMargin;
	int FontBold;
	int AsciiMarginTop;
	int AsciiMarginLeft;
	int KanjiMarginTop;
	int KanjiMarginLeft;
} SETTING;

/* main.c */
extern char LaunchElfDir[MAX_PATH], LastDir[MAX_NAME];
extern int boot;
void loadCdModules(void);
void loadUsbModules(void);
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

enum	//GetCurrentPos
{
	CURRENTPOS_X = 0,
	CURRENTPOS_Y
};

enum	//GetFontSize
{
	ASCII_FONT_WIDTH = 0,
	ASCII_FONT_HEIGHT,
	KANJI_FONT_WIDTH,
	KANJI_FONT_HEIGHT
};

extern itoGsEnv screen_env;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int SCREEN_MARGIN;
extern int FONT_WIDTH;
extern int FONT_HEIGHT;
extern int MAX_ROWS;
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
int checkFONTX2header(const char *path);
void drawChar(unsigned char c, int x, int y, uint64 colour);
int printXY(const unsigned char *s, int x, int y, uint64 colour, int draw);
//int SetCurrentPos(int x, int y);
//int GetCurrentPos(int type);
//int printXY2(const unsigned char *s, uint64 color, int draw);

/* pad.c */
extern u32 new_pad;
int setupPad(void);
int readpad(void);
void waitPadReady(int port, int slot);

/* config.c */
extern SETTING *setting;
void loadConfig(char *);
void config(char *);

/* filer.c */
enum	//getFilePath
{
	ANY_FILE = 0,
	ELF_FILE,
	DIR,
	FNT_FILE
};
char* getExtension(const char *path);
int ynDialog(const char *message, int defaultsel);
void MessageDialog(const char *message);
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

#endif
