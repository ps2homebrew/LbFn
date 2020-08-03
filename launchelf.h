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
#include <debug.h>
#include <ito.h>
#include <cdvd_rpc.h>
#include "cd.h"
#include "mass_rpc.h"

#include "language.h"

// バージョン
#define LBF_VER "LbF v0.46"

// 垂直スキャンレート
#define SCANRATE (ITO_VMODE_AUTO==ITO_VMODE_NTSC ? 60:50)

enum
{
	SCREEN_WIDTH = 640,
	SCREEN_HEIGHT = 448,
	SCREEN_MARGIN = 14,
	FONT_WIDTH = 10,
	FONT_HEIGHT = 20,
	LINE_THICKNESS = 2,
	
	MAX_NAME = 256,
	MAX_PATH = 1025,
	MAX_ENTRY = 2048,
	MAX_ROWS = 16,
	MAX_PARTITIONS=100
};

//getFilePathのモード
enum
{
	ANY_FILE = 0,
	ELF_FILE,
	DIR
};
	
typedef struct
{
	char dirElf[13][MAX_PATH];
	int timeout;
	int filename;
	uint64 color[8];
	int screen_x;
	int screen_y;
	int discControl;
	int flickerControl;
	int fileicon;
	int discPs2saveCheck;
	int discELFCheck;
	char Exportdir[MAX_PATH];
	int language;
} SETTING;

extern char LaunchElfDir[MAX_PATH], LastDir[MAX_NAME];

void loadCdModules(void);
void loadUsbModules(void);
void loadHddModules(void);

/* elf.c */
int checkELFheader(const char *filename);
void RunLoaderElf(char *filename, char *);

/* draw.c */
extern itoGsEnv screen_env;
int InitBIOSFont(void);
void FreeBIOSFont(void);
void drawDark(void);
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2);
void setScrTmp(const char *msg0, const char *msg1);
void drawMsg(const char *msg);
void setupito(void);
void clrScr(uint64 color);
void drawScr(void);
void drawFrame(int x1, int y1, int x2, int y2, uint64 color);
void drawChar(unsigned char c, int x, int y, uint64 colour);
int printXY(const unsigned char *s, int x, int y, uint64 colour, int);

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
unsigned char *elisaFnt;
void MessageDialog(const char *message);
int newdir(const char *path, const char *name);
int keyboard(char *out, int max);
void getFilePath(char *out, const int cnfmode);

/* language.c */
extern LANGUAGE *lang;
void InitLanguage(void);
void FreeLanguage(void);
void SetLanguage(const int langID);

#endif
