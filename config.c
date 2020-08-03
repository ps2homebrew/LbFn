#include "launchelf.h"

//デフォルトの設定の値
enum
{
	DEF_TIMEOUT = 10,
	DEF_FILENAME = TRUE,
	DEF_FILEALL = TRUE,
	DEF_FLICKER_ALPHA = 0x4C,
	DEF_FLICKERCONTROL = TRUE,
	DEF_TVMODE = 0,	//0=auto 1=ntsc 2=pal 3=480p 4=1080i 5=720p
	DEF_DEFAULTTITLE = 0,
	DEF_DEFAULTDETAIL = 0,
	DEF_SORT_TYPE = 1,	//0=none 1=filename 2=extension 3=gametitle 4=size 5=timestamp
	DEF_SORT_DIR = TRUE,
	DEF_SORT_EXT = TRUE,

	DEF_CHAR_MARGIN = 2,
	DEF_LINE_MARGIN = 5,
	DEF_FONTBOLD = TRUE,
	DEF_ASCII_MARGINTOP = 0,
	DEF_ASCII_MARGINLEFT = 0,
	DEF_KANJI_MARGINTOP = 0,
	DEF_KANJI_MARGINLEFT = 0,

	DEF_DISCCONTROL = FALSE,
	DEF_FILEICON = TRUE,
	DEF_DISCPS2SAVECHECK = FALSE,
	DEF_DISCELFCHECK = FALSE,
	DEF_FILEPS2SAVECHECK = FALSE,
	DEF_FILEELFCHECK = FALSE,
	//DEF_LANGUAGE = LANG_ENGLISH,
#define	DEF_LANGUAGE	((*((char*)0x1FC7FF52))=='J'?LANG_JAPANESE:LANG_ENGLISH)
	DEF_USBMASS_FLAG = FALSE,	// 0:Inside Only
	DEF_USBD_FLAG = FALSE,
	DEF_USBM_DEVICES = 2,
	DEF_USBKBD_FLAG = FALSE,
	DEF_USBMOUSE_FLAG = FALSE,
};

//CONFIG
enum
{
	BUTTONSETTING=0,
	FILERSETTING,
	SCREENSETTING,
	COLORSETTING,
	FONTSETTING,
	DEVICESETTING,
	VIEWERSETTING,
	MISC,
	OK,
	CANCEL,
};

//GSEDIT
enum
{
	GSCFG_NAME=1,
	GSCFG_SCANMODE,
	GSCFG_SIZE,
	GSCFG_DEPTH,
	GSCFG_DITHER,
	GSCFG_INTERLACE,
	GSCFG_FFMODE,
	GSCFG_DEFAULT
};

//GSCONFIG
enum
{
	GS_EASYMODE,
	//GS_DETAILMODE,
	GS_ALLINIT,
	GS_OK,
	GS_CANCEL,
	GS_DETAILMODE,
};

//GSCONFIG/EASYMODE
enum
{
	GSE_NAME=1,
	GSE_VMODE,
	GSE_WIDTH,
	GSE_HEIGHT,
	GSE_DEPTH,
	GSE_DOUBLE,
	GSE_INFO,
	GSE_AUTOAPPLY,
	GSE_CONVERT,
	GSE_INIT,
};

//GSCONFIG/DETAILMODE
enum
{
	GSD_NUMBER=1,
	GSD_NAME,
	GSD_WIDTH,
	GSD_HEIGHT,
	GSD_LEFT,
	GSD_TOP,
	GSD_MAG_X,
	GSD_MAG_Y,
	GSD_DEPTH,
	GSD_X1,
	GSD_Y1,
	GSD_X2,
	GSD_Y2,
	GSD_ZLEFT,
	GSD_ZTOP,
	GSD_ZDEPTH,
	GSD_DITHER,
	GSD_INTERLACE,
	GSD_FFMODE,
	GSD_VMODE,
	GSD_VESA,
	GSD_DOUBLE,
	GSD_F0_LEFT,
	GSD_F0_TOP,
	GSD_F1_LEFT,
	GSD_F1_TOP,
	GSD_INIT,
};

//LAUNCHER SETTING
enum
{
	LAUNCH_BTNNUM=1,
	LAUNCH_NAME,
	LAUNCH_PADMSK,
	LAUNCH_ELFNUM,
	LAUNCH_PATH,
	LAUNCH_LIST,
	FILENAME,
	FILEALL,
	TIMEOUT,
	BUTTONINIT,
};

//FILER SETTING
enum
{
	FILEICON=1,
	PS2SAVECHECK,
	ELFCHECK,
	FILEPS2SAVECHECK,
	FILEELFCHECK,
	EXPORTDIR,
	DEFAULTTITLE,
	DEFAULTDETAIL,
	SORT_TYPE,
	SORT_DIR,
	SORT_EXT,
	FILERINIT,
};

//COLOR SETTING
enum
{
	COLOR1=1,
	COLOR2,
	COLOR3,
	COLOR4,
	COLOR5,
	COLOR6,
	COLOR7,
	COLOR8,
	COLOR9,
	COLOR10,
	COLOR11,
	COLOR12,
	FLICKER_ALPHA,
	PRESETCOLOR,
};

//SCREEN SETTING
enum
{
	TVMODE=1,
	FONTHALF,
	FONTVHALF,
	FONTSCALEMODE,
	FONTBOLDS,
	SCREEN_X,
	SCREEN_Y,
	FLICKERCONTROL,
	SCREENINIT,
};

//NETWORK SETTING
enum
{
	IPADDRESS,
	NETMASK,
	GATEWAY,
	NETWORKINIT,
	NETWORKSAVE,
	NETWORKCANCEL,
};

//FONT SETTING
enum
{
	ASCIIFONT=1,
	KANJIFONT,
	CHARMARGIN,
	LINEMARGIN,
	FONTBOLD,
	ASCIIMARGINTOP,
	ASCIIMARGINLEFT,
	KANJIMARGINTOP,
	KANJIMARGINLEFT,
	FONTINIT,
};

//DEVICE SETTING
enum
{
	USBD_FLAG=1,
	USBD_PATH,
	USBMASS_FLAG,
	USBMASS_PATH,
	USBM_DEVICES,
	USBKBD_FLAG,
	USBKBD_PATH,
	DEVICEINIT,
	USBMOUSE_FLAG,
	USBMOUSE_PATH,
};

//VIEWER SETTING
enum
{
	TXT_LINENUMBER=1,
	TXT_TABSPACES,
	TXT_CRLFTABDISP,
	IMG_FULLSCREEN,
	VIEWERINIT,
};

//MISC SETTING
enum
{
	LANG=1,
	DISCCONTROL,
	MISCINIT,
};

SETTING *setting=NULL, *tmpsetting;
extern int fonthalfmode;
char *splitcopy(const char *src, char *dist, int maxlen);
int explodeconfig(const char *src);
int strtogsreg(int num, char *src);
int gsregtostr(char *dst, int num);
static int tmpi[32];
static char tmps[32][MAX_PATH];
static uint64 clut[8][16] = 
{
	{	0x106040, 0xC0C0C0, 0x00C0FF, 0xFFFFFF, 0x00A0A0, 0x606060,
		0x00C000, 0x0000A0, 0xE08000, 0xA8A8A8, 0x0060C0, 0x000000,
	},{	0x321E1E, 0x504040, 0xC0C0C0, 0x808080, 0x008080, 0x808080,
		0x008000, 0x000080, 0xFF8000, 0x504040, 0x0060C0, 0x000000,
	},{	0x808080, 0x404040, 0x000060, 0x000000, 0x00A0A0, 0x505050,
		0x008000, 0x000080, 0xC06000, 0x404040, 0x0060C0, 0x000000,
	},{	0x181818, 0x404040, 0x0080FF, 0x909090, 0x008080, 0x808080,
		0x008000, 0x000080, 0xFF8000, 0x404040, 0x0060C0, 0x000000,
	},{	0x000000, 0x7A6450, 0x80FF80, 0xA0A0A0, 0x00A0A0, 0xA0A0A0,
		0x00A000, 0x2020A0, 0xFFA000, 0x303030, 0x0070C0, 0x100010,
	}
};

static int clutnum[16] = {
0,	COLOR_BACKGROUND,
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
	COLOR_OUTSIDE,
};

/*	画面モードメモ 
02	NTSC	 720x 480	60
03	PAL		 720x 576	50
82	NTSC	 720x 480	60
83	PAL		 720x 576	50
1A	VESA	 640x 480	60
1B	VESA	 640x 480	72
1C	VESA	 640x 480	75
1D	VESA	 640x 480	85
2A	VESA	 800x 600	56
2B	VESA	 800x 600	60
2C	VESA	 800x 600	72
2D	VESA	 800x 600	75
2E	VESA	 800x 600	85
3B	VESA	1024x 768	60
3C	VESA	1024x 768	70
3D	VESA	1024x 768	75
3E	VESA	1024x 768	85
4A	VESA	1280x1024	60
4B	VESA	1280x1024	75
50	480p	 720x 480	60
D0	480p	 720x 480	60
51	1080i	1920x1080	60
52	720p	1280x 720	60
	※スクイーズ信号出力をコントロール方法が不明
*/
//-------------------------------------------------
//スクリーンXY反映用
void SetScreenPosVM()
{
	int vmode;
	if ((vmode = setting->tvmode) == 0) vmode = ITO_VMODE_AUTO-1;
	SCREEN_LEFT = setting->screen_left[vmode];
	SCREEN_TOP = setting->screen_top[vmode];
	font_half = setting->font_half[vmode];
	font_vhalf = setting->font_vhalf[vmode];
	//font_bold = setting->font_bold[vmode];
	fonthalfmode = setting->font_scaler[vmode];
	flickerfilter = setting->flickerfilter[vmode];
	if (setting->font_bold[setting->tvmode] > 0)
		SetFontBold(setting->font_bold[setting->tvmode]-1);
	else
		SetFontBold(setting->FontBold);
}
//-------------------------------------------------
//スクリーンXY変更用
void SetScreenPosXY()
{
	int vmode;
	if ((vmode = setting->tvmode) == 0) vmode = ITO_VMODE_AUTO-1;
	setting->screen_left[vmode] = SCREEN_LEFT;
	setting->screen_top[vmode] = SCREEN_TOP;
	setting->font_half[vmode] = font_half;
	setting->font_vhalf[vmode] = font_vhalf;
	setting->font_scaler[vmode] = fonthalfmode;
	//setting->font_bold[tvmode] = font_bold;
	setting->flickerfilter[vmode] = flickerfilter;
}
//-------------------------------------------------
//メモリーカードの種類を取得
int GetMcType(int port, int slot)
{
	int type;

	mcGetInfo(port, slot, &type, NULL, NULL);
	mcSync(MC_WAIT, NULL, NULL);

	return type;
}

//-------------------------------------------------
// メモリーカードの状態をチェックする。
// 戻り値は有効なメモリーカードスロットの番号。
// メモリーカードが挿さっていない場合は-11を返す。
int CheckMC(void)
{
	int ret;
	//int ret=0,func=0,nret,cnt=0;
	//int i;
	//printf("mcSync[1]: %d:", mcSync(MC_NOWAIT, &func, &ret));
	//printf("func:%d, ret=%d\n", func, ret);
	//printf("debug::: 1\n");
	//printf("mcGetInfo: %d\n", mcGetInfo(0, 0, NULL, NULL, NULL));
	mcGetInfo(0, 0, NULL, NULL, NULL);
	//printf("debug::: 2\n");
	//while((nret=mcSync(MC_NOWAIT, &func, &ret))==0){
		mcSync(MC_WAIT, NULL, &ret);
	//	printf("mcSync(%d): %d: func:%d, ret=%d\n", ++cnt, nret, func, ret);
	//	if (cnt == 100) {
	//		printf("mcInit: %d\n", mcInit(MC_TYPE_MC));
	//	} else if (cnt == 200) {
	//		break;
	//	}
	//	for(i=0;i<1000000;i++){}
	//}
	//printf("mcSync(%d): %d: func:%d, ret=%d\n", ++cnt, nret, func, ret);
	//printf("\ndebug::: 3\n");

	if( -1 == ret || 0 == ret) return 0;

	mcGetInfo(1, 0, NULL, NULL, NULL);
	mcSync(MC_WAIT, NULL, &ret);

	if( -1 == ret || 0 == ret ) return 1;

	return -11;
}

//-------------------------------------------------
// GSプリセット値を初期化
void InitGSREG(void)
{// 480,576,96,48,275+48=323
	char src[15][128] = {
		//"\"NTSC\",640,224,1914,275,3,0,4,640,0,0,640,224,0,448,2,0,1,1,2,0,2,0,0,0,224",
		"\"NTSC\",640,448,720,480,0x02,1914,275,3,0,4,0,1,0,0,1,2",
		"\"PAL\",640,512,720,576,0x03,1914,328,3,0,4,0,1,0,0,1,2",
		"\"480p\",640,448,720,480,0x50,948,275,1,0,4,0,0,0,0,1,2",
		"\"1080i\",1820,1024,1920,1080,0x51,1194,578,0,0,2,1,1,0,0,0,2",
		"\"720p\",1216,684,1280,720,0x52,938,384,0,0,2,1,0,0,0,1,2",
		"\"CFG0 (NTSC,640x224)\",640,224,640,224,0x02,1914,275,3,0,4,0,1,1,0,1,2",
		"\"CFG1 (NTSC,832x448)\",832,448,832,448,0x02,1914,275,2,0,4,0,1,0,0,1,2",
		"\"CFG2 (PAL,832x512)\",832,512,832,512,0x03,1914,328,2,0,4,0,1,0,0,1,2",
		"\"CFG3 (480p,640x400)\",640,400,640,400,0x50,948,275,1,0,4,0,0,0,0,1,2",
		"\"CFG4 (1080i,832x512)\",832,512,832,512,0x51,1194,578,1,1,4,0,1,0,0,1,2",
		"\"CFG5 (1080i,1152x864)\",1152,864,1152,864,0x51,1195,578,0,0,2,1,1,0,0,1,2",
		"\"CFG6 (1080i,1280x768)\",1280,768,1280,768,0x51,1195,578,0,0,2,1,1,0,0,1,2",
		"\"CFG7 (1080i,1600x448)\",1600,448,1600,448,0x51,1195,578,0,0,2,1,1,1,0,1,2",
		"\"CFG8 (720p,960x540)\",960,540,960,540,0x52,938,384,0,0,4,0,0,0,0,1,2",
		"\"CFG9 (720p,1024x640)\",1024,640,1024,640,0x52,938,384,0,0,2,1,0,0,0,1,2",
	};
	int i;
	
	for (i=0;i<MAX_GSREG;i++) {
		gsregs[i].loaded = 0;
		gsregs[i].name[0] = 0;
		gsregs[i].width = 0;
		gsregs[i].height = 0;
	}
	for (i=0;i<15;i++) {
		if (src[i][0] == 0) break;
		strtogsreg(i+1,src[i]);
	}
	gsregs[0].vmode = ITO_VMODE_AUTO;
}

//-------------------------------------------------
// BUTTON SETTINGを初期化
void InitButtonSetting(void)
{
	int i,j;

	for(i=0;i<MAX_BUTTON;i++){
		for(j=0;j<MAX_ELF;j++)
			setting->dirElf[i].path[j][0]=0;
		setting->dirElf[i].name[0]=0;
		setting->dirElf[i].padmsk=0;
	}
	//strcpy(setting->dirElf[0].name, "Default");
	strcpy(setting->dirElf[1].path[0], "MISC/FileBrowser");
	strcpy(setting->dirElf[MAX_BUTTON-1].path[0], "MISC/CONFIG");
	setting->dirElf[1].padmsk = PAD_CIRCLE;
	setting->dirElf[MAX_BUTTON-1].padmsk = PAD_SELECT;
	setting->filename = DEF_FILENAME;
	setting->fileall = DEF_FILEALL;
	setting->timeout = DEF_TIMEOUT;
}

//-------------------------------------------------
// FILER SETTINGを初期化
void InitFilerSetting(void)
{
	setting->fileicon = DEF_FILEICON;
	setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
	setting->discELFCheck = DEF_DISCELFCHECK;
	setting->filePs2saveCheck = DEF_FILEPS2SAVECHECK;
	setting->fileELFCheck = DEF_FILEELFCHECK;
	setting->Exportdir[0] = 0;
	setting->defaulttitle = DEF_DEFAULTTITLE;
	setting->defaultdetail = DEF_DEFAULTDETAIL;
	setting->sort = DEF_SORT_TYPE;
	setting->sortdir = DEF_SORT_DIR;
	setting->sortext = DEF_SORT_EXT;
}

//-------------------------------------------------
// SCREEN SETTINGを初期化
void InitScreenSetting(void)
{
	int i;
	for (i=0;i<MAX_GSREG;i++){
		setting->screen_left[i]		= 0;
		setting->screen_top[i]		= 0;
		setting->screen_width[i]	= 0;
		setting->screen_height[i]	= 0;
		setting->screen_depth[i]	= 0;
		setting->screen_dither[i]	= 0;
		setting->screen_interlace[i]= 0;
		setting->screen_ffmode[i]	= 0;
		setting->screen_scan[i]		= 0;
		setting->font_half[i]		= 0;
		setting->font_vhalf[i]		= 0;
		setting->font_scaler[i]		= 0;
		setting->font_bold[i]		= 0;
		setting->flickerfilter[i]	= 0;
		if (i>0 && gsregs[i].interlace && !gsregs[i].ffmode && gsregs[i].magy==0)
			setting->flickerfilter[i]	= 1;
	}
	//setting->flickerControl = DEF_FLICKERCONTROL;
	setting->tvmode = DEF_TVMODE;
}

//-------------------------------------------------
// COLOR SETTINGを初期化
void InitColorSetting(void)
{
	setting->color[COLOR_BACKGROUND]	= clut[0][0];
	setting->color[COLOR_FRAME]			= clut[0][1];
	setting->color[COLOR_HIGHLIGHTTEXT]	= clut[0][2];
	setting->color[COLOR_TEXT]			= clut[0][3];
	setting->color[COLOR_DIR]			= clut[0][4];
	setting->color[COLOR_FILE]			= clut[0][5];
	setting->color[COLOR_PS2SAVE]		= clut[0][6];
	setting->color[COLOR_ELF]			= clut[0][7];
	setting->color[COLOR_PS1SAVE]		= clut[0][8];
	setting->color[COLOR_GRAYTEXT]		= clut[0][9];
	setting->color[COLOR_PSU]			= clut[0][10];
	setting->color[COLOR_OUTSIDE]		= clut[0][11];
	setting->flicker_alpha = DEF_FLICKER_ALPHA;
}

//-------------------------------------------------
// FONT SETTINGを初期化
void InitFontSetting(void)
{
	strcpy(setting->AsciiFont, "systemfont");
	strcpy(setting->KanjiFont, "systemfont");
	setting->CharMargin = DEF_CHAR_MARGIN;
	setting->LineMargin = DEF_LINE_MARGIN;
	setting->FontBold = DEF_FONTBOLD;
	//setting->font_half = DEF_FONTHALF;
	setting->AsciiMarginTop = DEF_ASCII_MARGINTOP;
	setting->AsciiMarginLeft = DEF_ASCII_MARGINLEFT;
	setting->KanjiMarginTop = DEF_KANJI_MARGINTOP;
	setting->KanjiMarginLeft = DEF_KANJI_MARGINLEFT;
}

//-------------------------------------------------
// DEVICES SETTINGを初期化
void InitDeviceSetting(void)
{
	setting->usbd_flag = DEF_USBD_FLAG;
	strcpy(setting->usbd_path, "mc:/SYS-CONF/USBD.IRX");
	setting->usbmass_flag = DEF_USBMASS_FLAG;
	strcpy(setting->usbmass_path, "mc:/SYS-CONF/USB_MASS.IRX");
	setting->usbmdevs = DEF_USBM_DEVICES;
	setting->usbkbd_flag = DEF_USBKBD_FLAG;
	strcpy(setting->usbkbd_path, "mc:/SYS-CONF/PS2KBD.IRX");
	setting->usbmouse_flag = DEF_USBMOUSE_FLAG;
	strcpy(setting->usbmouse_path, "mc:/SYS-CONF/PS2MOUSE.IRX");
}

//-------------------------------------------------
// VIEWER SETTINGを初期化
void InitViewerSetting(void)
{
	setting->txt_linenumber = 0;
	setting->txt_tabmode = 8;
	setting->txt_chardisp = 0;
	setting->img_fullscreen = 0;
	set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);
}

//-------------------------------------------------
// MISC SETTINGを初期化
void InitMiscSetting(void)
{
	setting->discControl = DEF_DISCCONTROL;
	setting->language = DEF_LANGUAGE;
}

//-------------------------------------------------
// 設定を初期化
void InitSetting(void)
{
	InitGSREG();
	InitButtonSetting();
	InitFilerSetting();
	InitScreenSetting();
	InitColorSetting();
	InitFontSetting();
	InitDeviceSetting();
	InitViewerSetting();
	InitMiscSetting();
}

//-------------------------------------------------
char *splitcopy(const char *src, char *dist, int maxlen)
{
	char *i,*s,*d,j=0;
	int c;
	s=(char *)src;
	d=dist;
	if (*s == 0x22) {
		s++;
		j=!0;
	}
	c=0;
	for (i=s;*i!=0;i++){
		if ((i[0] == 0x22) && (i[1] == 0x22)) {
			d[c++]=0x22;i++;
		} else if (*i == 0x22) {
			i++;
			break;
		} else {
			d[c++]=*i;
		}
		if (c>=maxlen-1) break;
	}
	d[c]=0;
	return i;
}

//-------------------------------------------------
// 値の分解
// 例) gsreg="NTSC",640,448,1914,35,3,0,4,0,0,0,0,0,640,448,0,1,0,2,0,2,0,0,0,448
int explodeconfig(const char *src)
{
	int argc;
	char *s,*p,*e,*t;
	s = (char *)src;
	t = strchr(s, 10);
	e = s + strlen(s);
	if ((t != NULL) && (e > t)) e = t;
	for (argc=0;argc<32;argc++){
		tmps[argc][0] = 0;
		tmpi[argc] = 0;
	}
	for (argc=0;argc<32;argc++){
		if (*s == 0x22) {
			s = splitcopy(s, tmps[argc], MAX_PATH);
			if (*s != ',') break;
			s++;
		} else {
			//if ((s[0] == 0x30) && ((s[1] == 0x58)||(s[1] == 0x78)))
				tmpi[argc] = strtol(s, NULL, 0);
			//else
			//	tmpi[argc] = atoi(s);
			p = strchr(s, ',');
			if (p == 0) break;
			s=p+1;
		}
		if (s >= e) break;
	}
	return argc+1;
}

//-------------------------------------------------
int strtogsreg(int num, char *src)
{
	int i,j;
	int psmtable[] = {ITO_CLUT4, ITO_CLUT8, ITO_RGBA16, ITO_RGB24, ITO_RGBA32};
	int zpsmtable[] = {ITO_ZBUF16, ITO_ZBUF16S, ITO_ZBUF16, ITO_ZBUF24, ITO_ZBUF32};
	int ffmodetable[] = {ITO_FIELD, ITO_FRAME};
	i = explodeconfig(src);
	
	if (i != 17){
		return 0;
	}
/*	printf("strtogsreg: %s",tmps[0]);
	for (j=1;j<i;j++)
		printf(",%d", tmpi[j]);
	printf("\n");
*/	j = 0;
	strcpy(gsregs[num].name, tmps[j++]);
	gsregs[num].width = tmpi[j++];
	gsregs[num].height = tmpi[j++];
	gsregs[num].defwidth = tmpi[j++];
	gsregs[num].defheight = tmpi[j++];
	
	gsregs[num].vmode = tmpi[j++];
	gsregs[num].left = tmpi[j++];
	gsregs[num].top = tmpi[j++];
	gsregs[num].magx = tmpi[j++];
	gsregs[num].magy = tmpi[j++];
	
	gsregs[num].psm = psmtable[tmpi[j++]];
	gsregs[num].dither = tmpi[j++];
	gsregs[num].interlace = tmpi[j++];
	gsregs[num].ffmode = ffmodetable[tmpi[j++]];
	gsregs[num].vesa = tmpi[j++];

	gsregs[num].doublebuffer = tmpi[j++];
	gsregs[num].zpsm = zpsmtable[tmpi[j++]];
	
	if (gsregs[num].width*gsregs[num].height == 0) return 0;
	if (gsregs[num].name[0] == 0) return 0;
	gsregs[num].loaded = TRUE;
	return 1;
}

int gsregtostr(char *dst, int num)
{
	int psmtable[] = {4,3,2,1,0};
	int zpsmtable[] = {4,3,2,0};
	int ffmodetable[] = {0,1};
	*dst = 0;
	if (!gsregs[num].loaded) return 0;
	if (!gsregs[num].name[0]) return 0;
	if (gsregs[num].width*gsregs[num].height == 0) return 0;
	sprintf(dst, "\"%s\",%d,%d,%d,%d,0x%02X,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	
	gsregs[num].name,
	gsregs[num].width,
	gsregs[num].height,
	gsregs[num].defwidth,
	gsregs[num].defheight,

	gsregs[num].vmode,
	gsregs[num].left,
	gsregs[num].top,
	gsregs[num].magx,
	gsregs[num].magy,

	psmtable[gsregs[num].psm & 15],
	gsregs[num].dither,
	gsregs[num].interlace,
	ffmodetable[(int) gsregs[num].ffmode],
	gsregs[num].vesa,

	gsregs[num].doublebuffer,
	zpsmtable[gsregs[num].zpsm & 3]);
	return 1;
}

int strtogscfg(int num, char *src)
{
	int i,j;
	i = explodeconfig(src);
	if (i<14) return 0;
	j = 0;
	setting->screen_left[num]		= tmpi[j++];
	setting->screen_top[num]		= tmpi[j++];
	setting->screen_width[num]		= tmpi[j++];
	setting->screen_height[num]		= tmpi[j++];
	setting->screen_depth[num]		= tmpi[j++];
	setting->screen_dither[num]		= tmpi[j++];
	setting->screen_interlace[num]	= tmpi[j++];
	setting->screen_ffmode[num]		= tmpi[j++];
	setting->screen_scan[num]		= tmpi[j++];
	setting->font_half[num]			= tmpi[j++];
	setting->font_vhalf[num]		= tmpi[j++];
	setting->font_scaler[num]		= tmpi[j++];
	setting->font_bold[num]			= tmpi[j++];
	setting->flickerfilter[num]		= tmpi[j++];
	return 1;
}

int gscfgtostr(char *dst, int num)
{
	*dst = 0;
	sprintf(dst, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	
	setting->screen_left[num],
	setting->screen_top[num],
	setting->screen_width[num],
	setting->screen_height[num],
	
	setting->screen_depth[num],
	setting->screen_dither[num],
	setting->screen_interlace[num],
	setting->screen_ffmode[num],
	
	setting->screen_scan[num],
	setting->font_half[num],
	setting->font_vhalf[num],
	setting->font_scaler[num],
	
	setting->font_bold[num],
	setting->flickerfilter[num]);
	
	return 1;
}

int strtoelfcfg(int num, char *src)
{//	buttonX="TITLE",0x0003,"mc:/BOOT/BOOT.ELF","mass:/BOOT.ELF","host:BOOT.ELF"
	int i,j,k;
	i = explodeconfig(src);
	if (i < 2) return 0;
	j = 0;
	strcpy(setting->dirElf[num].name, tmps[j++]);
	setting->dirElf[num].padmsk = tmpi[j++];
	for(k=j;j<i;j++){
		if(j-k>=MAX_ELF) break;
		strcpy(setting->dirElf[num].path[j-k], tmps[j]);
	}
	return j-k+1;
}

int elfcfgtostr(char *dst, int num)
{
	char tmp[MAX_PATH];
	int i;
	*dst = 0;
	if ((setting->dirElf[num].path[0][0] == 0) && (setting->dirElf[num].name[0] == 0)) return 0;
	if (setting->dirElf[num].padmsk)
		sprintf(dst, "\"%s\",0x%08x", setting->dirElf[num].name, setting->dirElf[num].padmsk);
	else
		sprintf(dst, "\"%s\",0", setting->dirElf[num].name);
	for(i=0;i<MAX_ELF;i++)
		if(setting->dirElf[num].path[i][0] != 0){
			sprintf(tmp, ",\"%s\"", setting->dirElf[num].path[i]);
			strcat(dst, tmp);
		}
	return i;
}

void padmsktostr(char *dst, int padmsk, char *def)
{
	char c[MAX_PATH];
	c[0] = 0;
	if (padmsk) {
		if (padmsk & PAD_CIRCLE)	strcat(c, "+○");
		if (padmsk & PAD_CROSS)		strcat(c, "+×");
		if (padmsk & PAD_TRIANGLE)	strcat(c, "+△");
		if (padmsk & PAD_SQUARE)	strcat(c, "+□");
		if (padmsk & PAD_L1)		strcat(c, "+L1");
		if (padmsk & PAD_R1)		strcat(c, "+R1");
		if (padmsk & PAD_L2)		strcat(c, "+L2");
		if (padmsk & PAD_R2)		strcat(c, "+R2");
		if (padmsk & PAD_L3)		strcat(c, "+L3");
		if (padmsk & PAD_R3)		strcat(c, "+R3");
		if (padmsk & PAD_START)		strcat(c, "+START");
		if (padmsk & PAD_SELECT)	strcat(c, "+SELECT");
		strcpy(dst, c+1);
	} else {
		strcpy(dst, def);
	}
}

//-------------------------------------------------
void settingcheck(int *dist, char *s, int min, int max, int def)
{
	int src=atoi(s);
	if (src<min)
		*dist = def;
	else if (src>max)
		*dist = def;
	else
		*dist = src;
}

//-------------------------------------------------
void saveConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	char temp[MAX_PATH];
	int ret;
	int i;

	//cdから起動しているときは、設定ファイルを保存しない
	if(boot==CD_BOOT){
		mainMsg[0] = 0;
		return;
	}
	path[0]=0;
	//cnfファイルのパス
	//LaunchELFが実行されたパスから設定ファイルを開く
	if(boot!=HOST_BOOT){
		sprintf(path, "%sLBFN.INI", LaunchElfDir);
		fd = fioOpen(path, O_RDONLY);
		if(fd >= 0)
			fioClose(fd);
		else
			path[0]=0;
	}
	//開けなかったら、SYS-CONFの設定ファイルを開く
	if(path[0]==0){
		if(boot==MC_BOOT)
			mcport = LaunchElfDir[2]-'0';
		else
			mcport = CheckMC();
		if(mcport==0||mcport==1){
			sprintf(path, "mc%d:/SYS-CONF/LBFN.INI", mcport);
			fd = fioOpen(path, O_RDONLY);
			if(fd >= 0)
				fioClose(fd);
			else
				path[0]=0;
		}
	}

	cnf_init();

	//cnfファイルオープン
	if(cnf_load(path)==FALSE)
		path[0]=0;

	//version
	sprintf(tmp, "%d", 3);
	if(cnf_setstr("cnf_version", tmp)<0) goto error;
	//Launcher
	for(i=0;i<MAX_BUTTON;i++){
		sprintf(temp, "button%d", i);
		elfcfgtostr(tmp, i);
		if(cnf_setstr(temp, tmp)<0) goto error;
	}
	//color
	sprintf(tmp, "%08lX", setting->color[COLOR_BACKGROUND]);
	if(cnf_setstr("color_background", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_FRAME]);
	if(cnf_setstr("color_fream", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_HIGHLIGHTTEXT]);
	if(cnf_setstr("color_highlight_text", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_TEXT]);
	if(cnf_setstr("color_normal_text", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_DIR]);
	if(cnf_setstr("color_folder", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_FILE]);
	if(cnf_setstr("color_file", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_PS2SAVE]);
	if(cnf_setstr("color_ps2_save", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_ELF]);
	if(cnf_setstr("color_elf_file", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_PS1SAVE]);
	if(cnf_setstr("color_ps1_save", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_GRAYTEXT]);
	if(cnf_setstr("color_disable_text", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_PSU]);
	if(cnf_setstr("color_psu_file", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[COLOR_OUTSIDE]);
	if(cnf_setstr("color_outside", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->flicker_alpha);
	if(cnf_setstr("flicker_alpha", tmp)<0) goto error;
	//font
	strcpy(tmp, setting->AsciiFont);
	if(cnf_setstr("ascii_font", tmp)<0) goto error;
	strcpy(tmp, setting->KanjiFont);
	if(cnf_setstr("kanji_font", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->CharMargin);
	if(cnf_setstr("char_margin", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->LineMargin);
	if(cnf_setstr("line_margin", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontBold);
	if(cnf_setstr("font_bold", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->AsciiMarginTop);
	if(cnf_setstr("ascii_margin_top", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->AsciiMarginLeft);
	if(cnf_setstr("ascii_margin_left", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->KanjiMarginTop);
	if(cnf_setstr("kanji_margin_top", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->KanjiMarginLeft);
	if(cnf_setstr("kanji_margin_left", tmp)<0) goto error;
	//
	sprintf(tmp, "%d", setting->language);
	if(cnf_setstr("language", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->timeout);
	if(cnf_setstr("timeout", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->discControl);
	if(cnf_setstr("disc_control", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->filename);
	if(cnf_setstr("only_filename", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->fileall);
	if(cnf_setstr("displayallelf", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->fileicon);
	if(cnf_setstr("file_icon", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->discPs2saveCheck);
	if(cnf_setstr("ps2save_check", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->discELFCheck);
	if(cnf_setstr("elf_check", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->filePs2saveCheck);
	if(cnf_setstr("file_ps2save_check", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->fileELFCheck);
	if(cnf_setstr("file_elf_check", tmp)<0) goto error;
	strcpy(tmp, setting->Exportdir);
	if(cnf_setstr("export_dir", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->tvmode);
	if(cnf_setstr("tvmode", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->defaulttitle);
	if(cnf_setstr("default_title", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->defaultdetail);
	if(cnf_setstr("default_detail", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->sort);
	if(cnf_setstr("sort_type", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->sortdir);
	if(cnf_setstr("sort_dir", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->sortext);
	if(cnf_setstr("sort_ext", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbd_flag);
	if(cnf_setstr("usbd", tmp)<0) goto error;
	strcpy(tmp, setting->usbd_path);
	if(cnf_setstr("usbdfile", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbmass_flag);
	if(cnf_setstr("usbm", tmp)<0) goto error;
	strcpy(tmp, setting->usbmass_path);
	if(cnf_setstr("usbmfile", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbmdevs);
	if(cnf_setstr("usbmdevs", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbkbd_flag);
	if(cnf_setstr("usbkbd", tmp)<0) goto error;
	if(cnf_setstr("usbkbdfile", setting->usbkbd_path)<0) goto error;
	sprintf(tmp, "%d", setting->usbmouse_flag);
	if(cnf_setstr("usbms", tmp)<0) goto error;
	if(cnf_setstr("usbmsfile", setting->usbmouse_path)<0) goto error;
	sprintf(tmp, "%d", setting->txt_linenumber);
	if(cnf_setstr("linenumber", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->txt_tabmode);
	if(cnf_setstr("tabmode", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->txt_chardisp);
	if(cnf_setstr("chardisp", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->img_fullscreen);
	if(cnf_setstr("fullscreen", tmp)<0) goto error;
	
	ret = 0;
	for(i=6;i<MAX_GSREG;i++){
		sprintf(temp, "gsreg%d", i);
		if (gsregs[i].loaded) {
			gsregtostr(tmp, i);
			ret++;
			if(cnf_setstr(temp, tmp)<0) goto error;
		} else {
			strcpy(tmp, "");
			if (i<16) cnf_setstr(temp, tmp);
		}
	}
	//sprintf(tmp, "%d", ret);
	//if(cnf_setstr("gsregs", tmp)<0) goto error;
	for(i=1;i<MAX_GSREG;i++){
		sprintf(temp, "gscfg%d", i);
		if (gsregs[i].loaded) {
			gscfgtostr(tmp, i);
			if(cnf_setstr(temp, tmp)<0) goto error;
		}
	}
	
	goto no_error;

error:
	//エラーがあった
	sprintf(mainMsg, "%s", lang->conf_savefailed);
	cnf_free();
	return;

no_error:
	//cnfファイルのパス
	//LaunchELFのディレクトリにCNFがあったらLaunchELFのディレクトリにセーブ
	if(boot!=HOST_BOOT){
		sprintf(path, "%sLBFN.INI", LaunchElfDir);
		fd = fioOpen(path, O_RDONLY);
		if(fd >= 0)
			fioClose(fd);
		else
			path[0]=0;
	}
	//なかったら、SYS-CONFにセーブ
	if(path[0]==0){
		//SYS-CONFフォルダがあったらSYS-CONFにセーブ
		if(boot==MC_BOOT)
			mcport = LaunchElfDir[2]-'0';
		else
			mcport = CheckMC();
		sprintf(path, "mc%d:/SYS-CONF", mcport);
		fd = fioDopen(path);	//フォルダをオープンしてみる
		if(fd >= 0){
			//SYS-CONFフォルダがある
			fioDclose(fd);
			strcat(path, "/LBFN.INI");
		}
		else{
			//SYS-CONFがなかったらLaunchELFのディレクトリにセーブ
			sprintf(path, "%sLBFN.INI", LaunchElfDir);
		}
	}

	//cnf保存
	if(!strncmp(path, "mc", 2)){
		if(GetMcType(path[2]-'0', 0)==MC_TYPE_PS2)
			ret = cnf_save(path);
		else
			ret=-1;
	}
	else{
		ret = cnf_save(path);
	}

	if(ret<0)
		sprintf(mainMsg, "%s (%s)", lang->conf_savefailed, path);
	else
		sprintf(mainMsg, "%s (%s)", lang->conf_saveconfig, path);

	cnf_free();
	return;
}

//-------------------------------------------------
void loadConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	char gsregstr[MAX_PATH];
	int cnf_version=0;
	int ret=0;
	//int nchk=-1;
	int i;//, j;

	//printf("debug:: 1\n");
	path[0]=0;
	//printf("debug:: 2\n");

	//cnfファイルのパス
	//LaunchELFが実行されたパスから設定ファイルを開く
	if(boot!=HOST_BOOT){
	//	printf("debug:: 3\n");
		sprintf(path, "%sLBFN.INI", LaunchElfDir);
		if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
		printf("LoadConfig: path: %s\n", path);
		fd = fioOpen(path, O_RDONLY);
		if(fd >= 0)
			fioClose(fd);
		else
			path[0]=0;
	}
	//printf("debug:: 4\n");
	//開けなかったら、SYS-CONFの設定ファイルを開く
	if(path[0]==0){
	//	printf("debug:: 5\n");
		if(boot==MC_BOOT)
			mcport = LaunchElfDir[2]-'0';
		else
			mcport = CheckMC();
	//printf("debug:: 6\n");
		if(mcport==0||mcport==1){
	//printf("debug:: 7\n");
			sprintf(path, "mc%d:/SYS-CONF/LBFN.INI", mcport);
			printf("LoadConfig: path: %s\n", path);
			fd = fioOpen(path, O_RDONLY);
			if(fd >= 0)
				fioClose(fd);
			else
				path[0]=0;
		}
	}

	//printf("debug:: 8\n");
	//設定を初期化する
	InitSetting();

	cnf_init();

	//cnfファイルオープン
	if(cnf_load(path)<0){
		ret=0;
	}
	else{
		ret=1;
		cnf_version = 0;
		//version
		if(cnf_getstr("cnf_version", tmp, "")>=0)
			cnf_version = atoi(tmp);
		//バージョンチェック
		if(cnf_version!=3){
			//Setting初期化
			InitSetting();
			if(cnf_getstr("language", tmp, "")>=0)
				settingcheck(&setting->language, tmp, 0, NUM_LANG, DEF_LANGUAGE);
			ret=2;
		}
		else{
			//Launcher
			for (i=0; i<MAX_BUTTON; i++) {
				sprintf(gsregstr, "button%d", i);
				if(cnf_getstr(gsregstr, tmp, "")>=0)
					strtoelfcfg(i, tmp);
			}
			//color
			if(cnf_getstr("color_background", tmp, "")>=0)
				setting->color[COLOR_BACKGROUND] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_fream", tmp, "")>=0)
				setting->color[COLOR_FRAME] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_highlight_text", tmp, "")>=0)
				setting->color[COLOR_HIGHLIGHTTEXT] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_normal_text", tmp, "")>=0)
				setting->color[COLOR_TEXT] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_folder", tmp, "")>=0)
				setting->color[COLOR_DIR] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_file", tmp, "")>=0)
				setting->color[COLOR_FILE] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_ps2_save", tmp, "")>=0)
				setting->color[COLOR_PS2SAVE] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_elf_file", tmp, "")>=0)
				setting->color[COLOR_ELF] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_ps1_save", tmp, "")>=0)
				setting->color[COLOR_PS1SAVE] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_disable_text", tmp, "")>=0)
				setting->color[COLOR_GRAYTEXT] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_psu_file", tmp, "")>=0)
				setting->color[COLOR_PSU] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_outside", tmp, "")>=0)
				setting->color[COLOR_OUTSIDE] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("flicker_alpha", tmp, "")>=0)
				settingcheck(&setting->flicker_alpha, tmp, 0, 128, DEF_FLICKER_ALPHA);
			//font
			if(cnf_getstr("ascii_font", tmp, "")>=0)
				strcpy(setting->AsciiFont, tmp);
			if(cnf_getstr("kanji_font", tmp, "")>=0)
				strcpy(setting->KanjiFont, tmp);
			if(cnf_getstr("char_margin", tmp, "")>=0)
				setting->CharMargin = atoi(tmp);
			if(cnf_getstr("line_margin", tmp, "")>=0)
				setting->LineMargin = atoi(tmp);
			if(cnf_getstr("font_bold", tmp, "")>=0)
				settingcheck(&setting->FontBold, tmp, 0, 1, DEF_FONTBOLD);
			if(cnf_getstr("ascii_margin_top", tmp, "")>=0)
				setting->AsciiMarginTop = atoi(tmp);
			if(cnf_getstr("ascii_margin_left", tmp, "")>=0)
				setting->AsciiMarginLeft = atoi(tmp);
			if(cnf_getstr("kanji_margin_top", tmp, "")>=0)
				setting->KanjiMarginTop = atoi(tmp);
			if(cnf_getstr("kanji_margin_left", tmp, "")>=0)
				setting->KanjiMarginLeft = atoi(tmp);
			//
			if(cnf_getstr("language", tmp, "")>=0)
				settingcheck(&setting->language, tmp, 0, NUM_LANG, DEF_LANGUAGE);
			if(cnf_getstr("timeout", tmp, "")>=0)
				settingcheck(&setting->timeout, tmp, 0, 1, DEF_TIMEOUT);
			if(cnf_getstr("disc_control", tmp, "")>=0)
				settingcheck(&setting->discControl, tmp, 0, 1, DEF_DISCCONTROL);
			if(cnf_getstr("only_filename", tmp, "")>=0)
				settingcheck(&setting->filename, tmp, 0, 1, DEF_FILENAME);
			if(cnf_getstr("displayallelf", tmp, "")>=0)
				settingcheck(&setting->fileall, tmp, 0, 1, DEF_FILEALL);
			if(cnf_getstr("file_icon", tmp, "")>=0)
				settingcheck(&setting->fileicon, tmp, 0, 1, DEF_FILEICON);
			if(cnf_getstr("ps2save_check", tmp, "")>=0)
				settingcheck(&setting->discPs2saveCheck, tmp, 0, 1, DEF_DISCPS2SAVECHECK);
			if(cnf_getstr("elf_check", tmp, "")>=0)
				settingcheck(&setting->discELFCheck, tmp, 0, 1, DEF_DISCELFCHECK);
			if(cnf_getstr("file_ps2save_check", tmp, "")>=0)
				settingcheck(&setting->filePs2saveCheck, tmp, 0, 1, DEF_FILEPS2SAVECHECK);
			if(cnf_getstr("file_elf_check", tmp, "")>=0)
				settingcheck(&setting->fileELFCheck, tmp, 0, 1, DEF_FILEELFCHECK);
			if(cnf_getstr("export_dir", tmp, "")>=0)
				strcpy(setting->Exportdir, tmp);
			if(cnf_getstr("tvmode", tmp, "")>=0)
				settingcheck(&setting->tvmode, tmp, 0, 15, DEF_TVMODE);
			if(cnf_getstr("default_title", tmp, "")>=0)
				settingcheck(&setting->defaulttitle, tmp, 0, 1, DEF_DEFAULTTITLE);
			if(cnf_getstr("default_detail", tmp, "")>=0)
				settingcheck(&setting->defaultdetail, tmp, 0, 3, DEF_DEFAULTDETAIL);
			if(cnf_getstr("sort_type", tmp, "")>=0)
				settingcheck(&setting->sort, tmp, 0, 5, DEF_SORT_TYPE);
			if(cnf_getstr("sort_dir", tmp, "")>=0)
				settingcheck(&setting->sortdir, tmp, 0, 1, DEF_SORT_DIR);
			if(cnf_getstr("sort_ext", tmp, "")>=0)
				settingcheck(&setting->sortext, tmp, 0, 1, DEF_SORT_EXT);
			if(cnf_getstr("usbd", tmp, "")>=0)
				settingcheck(&setting->usbd_flag, tmp, 0, 1, DEF_USBD_FLAG);
			if(cnf_getstr("usbdfile", tmp, "")>=0)
				strcpy(setting->usbd_path, tmp);
			if(cnf_getstr("usbm", tmp, "")>=0)
				settingcheck(&setting->usbmass_flag, tmp, 0, 1, DEF_USBMASS_FLAG);
			if(cnf_getstr("usbmfile", tmp, "")>=0)
				strcpy(setting->usbmass_path, tmp);
			if(cnf_getstr("usbmdevs", tmp, "")>=0)
				settingcheck(&setting->usbmdevs, tmp, 0, 10, DEF_USBM_DEVICES);
			if(cnf_getstr("usbkbd", tmp, "")>=0)
				settingcheck(&setting->usbkbd_flag, tmp, 0, 1, DEF_USBKBD_FLAG);
			if(cnf_getstr("usbkbdfile", tmp, "")>=0)
				strcpy(setting->usbkbd_path, tmp);
			if(cnf_getstr("usbms", tmp, "")>=0)
				settingcheck(&setting->usbmouse_flag, tmp, 0, 1, DEF_USBMOUSE_FLAG);
			if(cnf_getstr("usbmsfile", tmp, "")>=0)
				strcpy(setting->usbmouse_path, tmp);
			if(cnf_getstr("linenumber", tmp, "")>=0)
				settingcheck(&setting->txt_linenumber, tmp, 0, 1, 0);
			if(cnf_getstr("tabmode", tmp, "")>=0)
				settingcheck(&setting->txt_tabmode, tmp, 2, 12, 8);
			if(cnf_getstr("chardisp", tmp, "")>=0)
				settingcheck(&setting->txt_chardisp, tmp, 0, 1, 0);
			if(cnf_getstr("fullscreen", tmp, "")>=0)
				settingcheck(&setting->img_fullscreen, tmp, 0, 1, 0);
			set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);

			for (i=1; i<MAX_GSREG; i++) {
				sprintf(gsregstr, "gsreg%d", i);
				if(cnf_getstr(gsregstr, tmp, "")>=0)
					strtogsreg(i, tmp);
				sprintf(gsregstr, "gscfg%d", i);
				if(cnf_getstr(gsregstr, tmp, "")>=0)
					strtogscfg(i, tmp);
			}
		}
	}

	SetLanguage(setting->language);

	if(ret==0){
		//設定ファイル開けなかった
		mainMsg[0] = 0;
	}
	else if(ret==1){
		//ロード成功
		sprintf(mainMsg, "%s (%s)", lang->conf_loadconfig, path);
	}
	else if(ret==2){
		//CNFのバージョンが古い
		sprintf(mainMsg, "%s (%s)", lang->conf_initializeconfig, path);
	}
	cnf_free();
	return;
}

//-------------------------------------------------
//IP設定
void ipconfig(char *mainMsg)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	int fd, mcport;
	char tmp[16*3], temp[16];
	char path[MAX_PATH];

	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmpip[16];
	char tmpnetmask[16];
	char tmpgw[16];

	strcpy(tmpip,ip);
	strcpy(tmpnetmask,netmask);
	strcpy(tmpgw,gw);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				sel=NETWORKSAVE;
			else if(new_pad & PAD_SELECT)
				sel=NETWORKCANCEL;
			else if(new_pad & PAD_CIRCLE){
				if(sel==IPADDRESS){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,ip);
					if(keyboard(tmp, 15)>=0) strcpy(ip,tmp);
				}
				else if(sel==NETMASK){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,netmask);
					if(keyboard(tmp, 15)>=0) strcpy(netmask,tmp);
				}
				else if(sel==GATEWAY){
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,gw);
					if(keyboard(tmp, 15)>=0) strcpy(gw,tmp);
				}
				else if(sel==NETWORKINIT){
					//init
					strcpy(ip, "192.168.0.10");
					strcpy(netmask, "255.255.255.0");
					strcpy(gw, "192.168.0.1");
					//sprintf(msg0, "%s", "Initialize Network Setting");
					//pushed = FALSE;
				}
				else if(sel==NETWORKSAVE){
					if(boot==MC_BOOT)
						mcport = LaunchElfDir[2]-'0';
					else
						mcport = CheckMC();
					if(mcport<0||mcport>1)
						mcport = 0;
					sprintf(path, "mc%d:/SYS-CONF/IPCONFIG.DAT", mcport);
					sprintf(temp, "mc%d:/", mcport);
					//メモリーカードの種類を取得
					sprintf(mainMsg, "%s %s", path, lang->conf_ipsavefailed);
					if(GetMcType(mcport, 0)==MC_TYPE_PS2){
						//save
						sprintf(tmp, "%s %s %s", ip, netmask, gw);
						//フォルダ作成
						newdir(temp, "SYS-CONF");
						// 書き込み
						fd = fioOpen(path, O_CREAT|O_WRONLY|O_TRUNC);
						if(fd >= 0){
							fioWrite(fd, tmp, strlen(tmp));
							fioClose(fd);
							sprintf(mainMsg, "%s %s", path, lang->conf_ipsaved);	//成功
						}
					}
					break;
				}
				else if(sel==NETWORKCANCEL){
					strcpy(ip,tmpip);
					strcpy(netmask,tmpnetmask);
					strcpy(gw,tmpgw);
					mainMsg[0] = 0;
					break;
				}
			}
		}

		//
		if (redraw) {
			for(i=0;i<=NETWORKCANCEL;i++){
				if(i==IPADDRESS){	//IPADDRESS
					sprintf(config[i], "%s: %s", lang->conf_ipaddress, ip);
				}
				else if(i==NETMASK){	//NETMASK
					sprintf(config[i], "%s: %s", lang->conf_netmask, netmask);
				}
				else if(i==GATEWAY){	//GATEWAY
					sprintf(config[i], "%s: %s", lang->conf_gateway, gw);
				}
				else if(i==NETWORKINIT){	//NETWORKINIT
					strcpy(config[i],lang->conf_ipsettinginit);
				}
				else if(i==NETWORKSAVE){	//NETWORKSAVE
					strcpy(config[i],lang->conf_ok);
				}
				else if(i==NETWORKCANCEL){
					strcpy(config[i],lang->conf_cancel);
				}
			}
			nList=i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) strcpy(msg0, "IPCONFIG");
			// 操作説明
			if(sel>=IPADDRESS && sel<=GATEWAY)
				sprintf(msg1, "○:%s", lang->conf_edit);
			else
				sprintf(msg1, "○:%s", lang->gen_ok);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//画面モードリスト
void config_screen_mode(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i, ret=-1, oldvmode;
	char config[MAX_GSREG+2][MAX_PATH];

	oldvmode = setting->tvmode;
	sel = oldvmode+1;
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE){
				ret = -1;
				break;
			}
			else if(new_pad & PAD_CIRCLE){
				if (sel==0) {
					ret = -1;
					break;
				} else if (sel == oldvmode+1) {
					ret = -1;
					break;
				} else if ((sel == 1) || ((gsregs[sel-1].loaded == 1) && drawDarks(1) && (MessageBox(lang->conf_screenmodemsg1, LBF_VER, MB_OKCANCEL) == IDOK))){
					setting->tvmode = sel-1;
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
					clrScr(setting->color[COLOR_BACKGROUND]);
					setScrTmp(msg0, msg1);
					drawDark();
					drawScr();
					clrScr(setting->color[COLOR_BACKGROUND]);
					setScrTmp(msg0, msg1);
					drawDark();
					drawScr();
					if ((sel == 1) || (MessageBox(lang->conf_screenmodemsg2, LBF_VER, MB_OKCANCEL|MB_DEFBUTTON2|MB_USETIMEOUT) == IDOK)) {
						ret = sel-1;
						oldvmode = setting->tvmode;
						break;
					}
					setting->tvmode = oldvmode;
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
				}
			}
			else if(new_pad & PAD_CROSS){	//×
			
			}
		}

		//
		if (redraw) {
			strcpy(config[0], "..");
			strcpy(config[1], "AUTO");
			for(i=1,nList=2;i<=MAX_GSREG;i++){
				if (gsregs[i].loaded==1)
					strcpy(config[nList++], gsregs[i].name);
			}

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s/%s", lang->conf_setting_screen, lang->conf_tvmode);
			// 操作説明
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	//return ret;
	return;
}

//-------------------------------------------------
//画面モード簡易編集
void config_screen_edit(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i, tvmode;
	char config[16][MAX_PATH];
	char pst[5][8] = {	"4bpp", "8bpp", "16bpp", "24bpp", "32bpp"};
	char fft[2][8] = {"FIELD", "FRAME"};
	char *psmtable[6] = {lang->conf_default, pst[0], pst[1], pst[2], pst[3], pst[4]};
	char *fftable[3] = {lang->conf_default, fft[0], fft[1]};
	char *onoff[3] = {lang->conf_default, lang->conf_off, lang->conf_on};
	char *onof[2] = {lang->conf_screen_scan_crop, lang->conf_screen_scan_full};
	
	tvmode =  setting->tvmode;
	if (tvmode == 0) tvmode = ITO_VMODE_AUTO-1;
	if (gsregs[tvmode].loaded != 1) tvmode = ITO_VMODE_AUTO-1;
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if (sel==0) break;
				if ((sel==GSCFG_SCANMODE) && (tvmode <= 5)) {
					setting->screen_scan[tvmode]^=1;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
				else if (sel==GSCFG_DEPTH) {
					if (setting->screen_depth[tvmode] == 0)
						setting->screen_depth[tvmode] = 3;
					else
						setting->screen_depth[tvmode] = (setting->screen_depth[tvmode] +2) % 7;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
				else if (sel==GSCFG_DITHER) {
					setting->screen_dither[tvmode] = (setting->screen_dither[tvmode] +1) % 3;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
				else if (sel==GSCFG_INTERLACE) {
					setting->screen_interlace[tvmode] = (setting->screen_interlace[tvmode] +1) % 3;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
				else if (sel==GSCFG_FFMODE) {
					setting->screen_ffmode[tvmode] = (setting->screen_ffmode[tvmode] +1) % 3;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
				else if (sel==GSCFG_DEFAULT) {
					setting->screen_width[tvmode]		= 0;
					setting->screen_height[tvmode]		= 0;
					setting->screen_depth[tvmode]		= 0;
					setting->screen_dither[tvmode]		= 0;
					setting->screen_interlace[tvmode]	= 0;
					setting->screen_ffmode[tvmode]		= 0;
					setting->screen_scan[tvmode]		= 0;
					SetScreenPosVM();
					setupito(setting->tvmode);
				}
			}
		}

		//
		if (redraw) {
			strcpy(config[0], "..");
			for(i=1;i<=GSCFG_DEFAULT;i++){
				if (i==GSCFG_NAME)
					sprintf(config[i], "%s: %s", lang->conf_displayname, gsregs[tvmode].name);
				else if (i==GSCFG_SCANMODE)
					sprintf(config[i], "%s: %s", lang->conf_screen_scan, onof[(int)setting->screen_scan[tvmode]]);
				else if (i==GSCFG_SIZE) {
					if (setting->screen_scan[tvmode])
						sprintf(config[i], "%s: %dx%d", lang->conf_resolution, gsregs[tvmode].defwidth, gsregs[tvmode].defheight);
					//	sprintf(config[i], "%s: %dx%d", lang->conf_resolution, setting->screen_width[tvmode], setting->screen_height[tvmode]);
					else
						sprintf(config[i], "%s: %dx%d", lang->conf_resolution, gsregs[tvmode].width, gsregs[tvmode].height);
				}
				else if (i==GSCFG_DEPTH)
					sprintf(config[i], "%s: %s", lang->conf_depth, psmtable[(int)setting->screen_depth[tvmode]]);
				else if (i==GSCFG_DITHER)
					sprintf(config[i], "%s: %s", lang->conf_dither, onoff[(int)setting->screen_dither[tvmode]]);
				else if (i==GSCFG_INTERLACE)
					sprintf(config[i], "%s: %s", lang->conf_interlace, onoff[(int)setting->screen_interlace[tvmode]]);
				else if (i==GSCFG_FFMODE)
					sprintf(config[i], "%s: %s", lang->conf_ffmode, fftable[(int)setting->screen_ffmode[tvmode]]);
				else if (i==GSCFG_DEFAULT)
					strcpy(config[i], lang->conf_gsedit_default);
			}
			nList = i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s/%s", lang->conf_setting_screen, lang->conf_edit);
			// 操作説明
			if ((sel == GSCFG_NAME) || (sel == GSCFG_SIZE))
				sprintf(msg1, "△:%s", lang->conf_up);
			else if (sel == GSCFG_DEFAULT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	//return ret;
	return;
}

//-------------------------------------------------
// GSCONFIG
static int autoapply=0;
void gsconfig_easy(GSREG *gsregs)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i, num=6, tvmode=ITO_VMODE_AUTO-1;
	int width=640,height=448,depth=ITO_RGBA32,dbl=TRUE;
	char config[16][MAX_PATH];
	char tmp[MAX_PATH];
	char psmtable[5][6] = {	"32bpp", "24bpp", "16bpp", "8bpp", "4bpp"};
	char *onoff[2] = {lang->conf_off, lang->conf_on};
	char name[64] = "";
	int maxwidth,maxheight;
	int totalsize,changed=0,vmoded=0;
	
	maxwidth = gsregs[tvmode].defwidth * (gsregs[tvmode].magx +1);
	maxheight = gsregs[tvmode].defheight * (gsregs[tvmode].magy +1);
	if (maxwidth>2048) maxwidth = 2048;
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_L1) {
				if (sel==GSE_CONVERT)
					if (num > 6) num--;
			}
			else if(new_pad & PAD_R1) {
				if (sel==GSE_CONVERT)
					if (num < 15) num++;
			}
			else if(new_pad & PAD_CIRCLE){
				if (sel==0) break;
				if (sel==GSE_NAME) {
					strcpy(tmp, name);
					if(keyboard(tmp, 64)>=0)
						strcpy(name, tmp);
				} else if (sel == GSE_VMODE) {
					if (tvmode < 5) tvmode++; else tvmode = 1;
					maxwidth = gsregs[tvmode].defwidth * (gsregs[tvmode].magx +1);
					maxheight = gsregs[tvmode].defheight * (gsregs[tvmode].magy +1);
					if (maxwidth>2048) maxwidth = 2048;
					if (width>maxwidth) width = maxwidth;
					if (height>maxheight) height = maxheight;
					changed = TRUE;
				} else if (sel == GSE_WIDTH) {
					if (paddata & PAD_SQUARE)
						width += 64;
					else
						width += 4;
					if (width>maxwidth) width = 128;
					changed = TRUE;
				} else if (sel == GSE_HEIGHT) {
					if (paddata & PAD_SQUARE)
						height += 32;
					else
						height += 4;
					if (height>maxheight) height = 128;
					changed = TRUE;
				} else if (sel == GSE_DEPTH) {
					depth = depth == ITO_RGBA32 ? ITO_RGBA16:ITO_RGBA32;
					changed = TRUE;
				} else if (sel == GSE_DOUBLE) {
					dbl = (dbl +1) % 2;
					changed = TRUE;
				} else if (sel == GSE_AUTOAPPLY) {
					autoapply ^= 1;
				} else if (sel == GSE_CONVERT) {
					gsregs[num] = gsregs[0];
					if (gsregs[num].name[0] == 0)
						sprintf(gsregs[num].name, "CFG (%s,%dx%d)", gsregs[tvmode].name, width, height);
					gsregs[num].loaded = TRUE;
				} else if (sel == GSE_INIT) {
					//tvmode=ITO_VMODE_AUTO-1;
					width=gsregs[tvmode].width;
					height=gsregs[tvmode].height;
					depth=gsregs[tvmode].psm;
					dbl=gsregs[tvmode].doublebuffer;
					changed = TRUE;
				}
			}
			else if(new_pad & PAD_CROSS){
				if (sel==GSE_NAME) {
					name[0] = 0;
				} else if (sel==GSE_VMODE) {
					if (tvmode > 1) tvmode--; else tvmode = 5;
					maxwidth = gsregs[tvmode].defwidth * (gsregs[tvmode].magx +1);
					maxheight = gsregs[tvmode].defheight * (gsregs[tvmode].magy +1);
					if (maxwidth>2048) maxwidth = 2048;
					if (width>maxwidth) width = maxwidth;
					if (height>maxheight) height = maxheight;
					changed = TRUE;
				} else if (sel==GSE_WIDTH) {
					if (paddata & PAD_SQUARE)
						width -= 64;
					else
						width -= 4;
					if (width<128) width = maxwidth;
					changed = TRUE;
				} else if (sel==GSE_HEIGHT) {
					if (paddata & PAD_SQUARE)
						height -= 32;
					else
						height -= 4;
					if (height<128) height = maxheight;
					changed = TRUE;
				} else if (sel == GSE_INIT) {
					width=gsregs[tvmode].defwidth;
					height=gsregs[tvmode].defheight;
					depth=gsregs[tvmode].psm;
					dbl=gsregs[tvmode].doublebuffer;
					changed = TRUE;
				}
			}
			else if(new_pad & PAD_SQUARE) {
				/*	if (sel==GSE_WIDTH) {
					width = gsregs[tvmode].width;
					changed = TRUE;
				} else if (sel==GSE_HEIGHT) {
					height = gsregs[tvmode].height;
					changed = TRUE;
				} else */if (sel==GSE_INIT) {
					if ((gsregs[num].loaded == 1) && (gsregs[num].width * gsregs[num].height)) {
						for(i=1;i<=5;i++)
							if (gsregs[i].vmode == gsregs[num].vmode) {
								tvmode = i;
								break;
							}
						width=gsregs[num].width;
						height=gsregs[num].height;
						depth=gsregs[num].psm;
						dbl=gsregs[num].doublebuffer;
						changed = TRUE;
						maxwidth = gsregs[tvmode].defwidth * (gsregs[tvmode].magx +1);
						maxheight = gsregs[tvmode].defheight * (gsregs[tvmode].magy +1);
						if (maxwidth>2048) maxwidth = 2048;
						if (width>maxwidth) width = maxwidth;
						if (height>maxheight) height = maxheight;
					}
				}
			}
		}
		// 画面反映
		if (changed) {
			gsregs[0] 				= gsregs[tvmode];
			gsregs[0].width			= width;
			gsregs[0].height		= height;
			i						= (gsregs[tvmode].defwidth * (gsregs[0].magx +1)) / width;
			if (i < 1) i = 1;
			if (i > 16) i = 16;
			gsregs[0].magx			= --i;
			i						= (gsregs[tvmode].defheight * (gsregs[0].magy +1)) / height;
			if (i < 1) i = 1;
			if (i > 4) i = 4;
			gsregs[0].magy			= --i;
			gsregs[0].psm			= depth;
			gsregs[0].dither		= 0;
			gsregs[0].doublebuffer	= dbl;
			gsregs[0].defwidth		= width;
			gsregs[0].defheight		= height;
			gsregs[0].loaded		= TRUE;
			strcpy(gsregs[0].name, name);
			if (autoapply) {
				if (!vmoded) {
					SCREEN_LEFT = 0;
					SCREEN_TOP = 0;
					font_half = 0;
					font_vhalf = 0;
					fonthalfmode = 0;
					flickerfilter = 0;
					SetFontBold(setting->FontBold);
					vmoded = TRUE;
				}
				changed = FALSE;
				setupito(0);
			}
		}
		if (!autoapply && vmoded) {
			SetScreenPosVM();
			if (setting->tvmode == 0)
				setupito(ITO_VMODE_AUTO-1);
			else
				setupito(setting->tvmode);
			vmoded = FALSE;
			changed = TRUE;
		}
		totalsize = ((((width+63)&-64) * height +8191) & -8192) * (dbl+1) * ((depth == 0)*2+2);

		//
		if (redraw) {
			strcpy(config[0], "..");
			for(i=1;i<=GSE_INIT;i++){
				if (i==GSE_NAME)
					sprintf(config[i], "%s: %s", lang->gs_name, name);
				else if (i==GSE_VMODE)
					sprintf(config[i], "%s: %s", lang->gs_vmode, gsregs[tvmode].name);
				else if (i==GSE_WIDTH)
					sprintf(config[i], "%s:%4d", lang->gs_width, width);
				else if (i==GSE_HEIGHT)
					sprintf(config[i], "%s:%4d", lang->gs_height, height);
				else if (i==GSE_DEPTH)
					sprintf(config[i], "%s: %s", lang->gs_depth, psmtable[depth]);
				else if (i==GSE_DOUBLE)
					sprintf(config[i], "%s: %s", lang->gs_double, onoff[dbl]);
				else if (i==GSE_INFO)
					sprintf(config[i], "%s: %4dKB/4096KB ", lang->gs_vramsize, (totalsize+1023)>>10);
				else if (i==GSE_CONVERT) {
					sprintf(config[i+1], "[%2d:%s]", num, gsregs[num].name);
					sprintf(config[i], lang->gse_convert, config[i+1]);
				}
				else if (i==GSE_AUTOAPPLY)
					sprintf(config[i], "%s: %s", lang->gs_autoapply, onoff[autoapply]);
				else if (i==GSE_INIT)
					strcpy(config[i], lang->gs_init);
			}
			nList = i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "GSCONFIG/%s", lang->gs_easymode);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->conf_up, lang->conf_up);
			else if (sel==GSE_NAME)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if (sel==GSE_VMODE)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->gs_next, lang->gs_prev, lang->conf_up);
			else if (sel==GSE_WIDTH || sel==GSE_HEIGHT)
				sprintf(msg1, "○:%s ×:%s +□:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_fast, lang->conf_up);
			else if (sel==GSE_DEPTH || sel==GSE_DOUBLE)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if (sel==GSE_INFO)
				sprintf(msg1, "△:%s", lang->conf_up);
			else if (sel==GSE_CONVERT)
				sprintf(msg1, "○:%s L1:%s R1:%s △:%s", lang->conf_change, lang->gs_prev, lang->gs_next, lang->conf_up);
			else if (sel==GSE_AUTOAPPLY)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if (sel==GSE_INIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else
				msg1[0] = 0;
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	//return ret;
	gsregs[0].loaded = FALSE;
	if (vmoded) {
		SetScreenPosVM();
		setupito(setting->tvmode);
		vmoded = FALSE;
		changed = TRUE;
	}
	return;
}

void gsconfig_detail(GSREG *gsregs)
{
	return;
}

void gsconfig(char *mainMsg)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[8][64];
	GSREG old_gsregs[MAX_GSREG];
	for (i=0;i<MAX_GSREG;i++)
		old_gsregs[i] = gsregs[i];
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				sel = GS_OK;
			else if(new_pad & PAD_SELECT)
				sel = GS_CANCEL;
			else if(new_pad & PAD_CIRCLE){
				if (sel==GS_EASYMODE) {
					gsconfig_easy(gsregs);
				} else if (sel==GS_DETAILMODE) {
					gsconfig_detail(gsregs);
				} else if (sel==GS_ALLINIT) {
					InitGSREG();
				} else if (sel==GS_OK) {
					saveConfig(mainMsg);
					if (!autoapply && setting->tvmode > 5)
						setupito(setting->tvmode);
					break;
				} else if (sel==GS_CANCEL) {
					for (i=0;i<MAX_GSREG;i++)
						gsregs[i] = old_gsregs[i];
					setupito(setting->tvmode);
					mainMsg[0] = 0;
					break;
				}
			}
		}
		//
		if (redraw) {
			for(i=0;i<=GS_CANCEL;i++)
				if (i==GS_EASYMODE) strcpy(config[i], lang->gs_easymode);
				else if (i==GS_DETAILMODE) strcpy(config[i], lang->gs_detailmode);
				else if (i==GS_ALLINIT) strcpy(config[i], lang->gs_gsinit);
				else if (i==GS_OK) strcpy(config[i], lang->gs_ok);
				else if (i==GS_CANCEL) strcpy(config[i], lang->gs_cancel);
			nList = i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) strcpy(msg0, "GSCONFIG");
			// 操作説明
			sprintf(msg1, "○:%s", lang->gen_ok);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	//return ret;
	return;
}

//-------------------------------------------------
//FreeMcBOOT Configture
void fmcb_cfg(char *mainMsg)
{
	cnf_init();
	cnf_load("mc0:/SYS-CONF/FREEMCB.CNF");
	/*	「Free McBoot Configurator 1.3 beta 6」 based
				Load CNF from: 
				Configutre launcher buttons...
					..
					BUTTON: 
					PATH1:
					PATH2:
					PATH3:
					CHECK ALL LAUNCHER SETTINGS
				Configutre OSDSYS options...
					..
					Hacked OSDSYS:
					Configutre Item xx:
						..
						Name:
						Path1:
						Path2:
						Path3:
					Configutre Scrolling Options...
						..
						Scroll Menu:
						Displayed Items:
						Menu y:
						Cursor Max Velocity:
						Cursor Acceleration:
						Left Cursor:
						Right Cursor:
						Top Delimiter:
						Bottom Delimiter:
					Video Mode:
					Skip MC update check:
					Skip HDD update check:
					Skip Disc Boot:
					Skip Sony Logo:
					Go to Browser:
					Selected Color:		R:00  G:00  B:00  A:00  ■
					Unselected Color:	R:00  G:00  B:00  A:00  ■
					Menu X:
					Enter:				X:        Y:
					Version:			X:        Y:
				Configutre ESR path...
					..
					Path1:
					Path2:
					Path3:
				FastBoot:
				Debug Screen:
				Pad Delay:
				Save CNF to:
				Return

CNF_version
Debug_Screen
FastBoot
ESR_Path_E1
ESR_Path_E2
ESR_Path_E3
pad_delay
LK_Auto_E1
LK_Circle_E1
LK_Cross_E1
LK_Square_E1
LK_Triangle_E1
LK_L1_E1
LK_R1_E1
LK_L2_E1
LK_R2_E1
LK_L3_E1
LK_R3_E1
LK_Up_E1
LK_Down_E1
LK_Left_E1
LK_Right_E1
LK_Start_E1
LK_Select_E1
hacked_OSDSYS
OSDSYS_video_mode
OSDSYS_Skip_Disc
OSDSYS_Skip_Logo
OSDSYS_Inner_Browser
OSDSYS_selected_color
OSDSYS_unselected_color
OSDSYS_scroll_menu
OSDSYS_menu_x
OSDSYS_menu_y
OSDSYS_enter_x
OSDSYS_enter_y
OSDSYS_version_x
OSDSYS_version_y
OSDSYS_cursor_max_velocity
OSDSYS_cursor_acceleration
OSDSYS_left_cursor
OSDSYS_right_cursor
OSDSYS_menu_top_delimiter
OSDSYS_menu_bottom_delimiter
OSDSYS_num_displayed_items
OSDSYS_Skip_MC
OSDSYS_Skip_HDD
name_OSDSYS_ITEM_?
path?_OSDSYS_ITEM_?

	*/
	cnf_free();
	return;
}

//-------------------------------------------------
//ランチャー設定
void config_button(SETTING *setting)
{
	char c[MAX_PATH];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i,j,k;
	char config[16][MAX_PATH];
	char *textbuffer=0;
	int textsize=0;
	int btn=0, elf=0, timeout;
	char none[2][16] = {"(none)", "DEFAULT"};
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT){
				if ((sel == LAUNCH_BTNNUM) || (sel == LAUNCH_NAME)){
					if (btn > 0)
						btn--;
				} else if ((sel == LAUNCH_ELFNUM) || (sel == LAUNCH_PATH)){
					if (elf > 0)
						elf--;
				} else if (sel >= LAUNCH_LIST) {
					sel-=MAX_ROWS/2;
					//if (sel==LAUNCH_BTNNUM) sel--;
					//if (sel==LAUNCH_ELFNUM) sel--;
					if ((sel>0) && (sel<LAUNCH_LIST)) sel=0;
				}
			} else if(new_pad & PAD_RIGHT) {
				if ((sel == LAUNCH_BTNNUM) || (sel == LAUNCH_NAME)){
					if (btn < MAX_BUTTON-1)
						btn++;
				} else if ((sel == LAUNCH_ELFNUM) || (sel == LAUNCH_PATH)){
					if (elf < MAX_ELF-1)
						elf++;
				} else if ((sel == 0) || (sel >= LAUNCH_LIST)) {
					sel+=MAX_ROWS/2;
					//if (sel==LAUNCH_BTNNUM) sel++;
					//if (sel==LAUNCH_ELFNUM) sel++;
					if (sel<LAUNCH_LIST) sel = LAUNCH_LIST;
				}
			} else if(new_pad & PAD_L1) {
				//if (sel == LAUNCH_PATH)
					if (btn>0) btn--;
			} else if(new_pad & PAD_R1) {
				//if (sel == LAUNCH_PATH)
					if (btn<MAX_BUTTON-1) btn++;
			} else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0)
					break;
				else if (sel == LAUNCH_BTNNUM) {
					if (btn < MAX_BUTTON-1)
						btn++;
				} else if (sel == LAUNCH_NAME) {
					strcpy(c, setting->dirElf[btn].name);
					if (keyboard(c, MAX_TITLE)>=0) strcpy(setting->dirElf[btn].name, c);
				} else if (sel == LAUNCH_PADMSK) {
					if (btn != 0) {
						drawDark();
						drawMsg(lang->conf_launch_pad0);
						timeout=-1;
						i=0;
						while(timeout != 0){
							itoVSync();
							waitPadReady(0, 0);
							if (readpad()) {
								if(new_pad) {
									i|=new_pad;
									timeout=8;
								};
								if(new_pad & (PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN))
									break;
							}
							if (timeout > 0) timeout--;
						}
						if (!(i & (PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN)))
							setting->dirElf[btn].padmsk=i;
					} else {
						pushed = FALSE;
						strcpy(msg0, lang->conf_launch_pad2);
					}
				} else if (sel == LAUNCH_ELFNUM) {
					if (elf < MAX_ELF-1)
						elf++;
				} else if (sel == LAUNCH_PATH) {
					getFilePath(setting->dirElf[btn].path[elf], ELF_FILE);
				}
				else if(sel==LAUNCH_LIST) {
					// [ 1] TITLE___________________ BUTTON_____ ELFS...
					textsize=0;
					for(i=0;i<MAX_BUTTON;i++)
						if(setting->dirElf[i].path[0][0]){
							textsize+=4+1+24+1+12-1;
								for(j=0;j<MAX_ELF;j++)
									if(setting->dirElf[i].path[j][0])
										textsize+=2+strlen(setting->dirElf[i].path[j]);
									else
										break;
							textsize+=2;
						}
					textbuffer = (char*)malloc(textsize);
					*textbuffer = 0; k = 0;
					for(i=0;i<MAX_BUTTON;i++)
						if (setting->dirElf[i].path[0][0]) {
							sprintf(c, "[%2d] ", i);
							strcat(textbuffer, c);
							strncpy(c, setting->dirElf[i].name, 24);
								c[24] = 0;
							strcat(c, "                         ");
							c[25] = 0;
							strcat(textbuffer, c);
							padmsktostr(c, setting->dirElf[i].padmsk, none[i==0]);
							if (strlen(c) < 12)
								strcat(c, "            ");
							c[12] = 32;
							c[13] = 0;
							strcat(textbuffer, c);
							c[0] = 0;
							for(j=0;j<MAX_ELF;j++){
								if (setting->dirElf[i].path[j][0]) {
									strcat(c, ", ");
									strcat(c, setting->dirElf[i].path[j]);
								} else
									break;
							}
							strcat(textbuffer, c+2);
							strcat(textbuffer, "\r\n");
						}
					textsize-=2;
					sprintf(c, "%s/%s", msg0, lang->conf_launch_list);
					set_viewerconfig(0, 0, 0, 0);
					txtedit(0, c, textbuffer, textsize);
					set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);
				}
				else if(sel==FILENAME)
					setting->filename = !setting->filename;
				else if(sel==FILEALL)
					setting->fileall = !setting->fileall;
				else if(sel==TIMEOUT) {
					if (paddata & PAD_SQUARE)
						setting->timeout+=60;
					else
						setting->timeout++;
					if (setting->timeout > 3600)
						setting->timeout = 3600;
				}
				else if(sel==BUTTONINIT){
					InitButtonSetting();
					//sprintf(msg0, "%s", "Initialize Button Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				//if(sel>=DEFAULT && sel<=LAUNCHER12)
				if (sel == LAUNCH_PATH)
					setting->dirElf[btn].path[elf][0]=0;
				else if (sel == LAUNCH_NAME)
					setting->dirElf[btn].name[0]=0;
				else if (sel == LAUNCH_BTNNUM){
					if (btn > 0)
						btn--;
				} else if (sel == LAUNCH_ELFNUM){
					if (elf > 0)
						elf--;
				}
				else if(sel==TIMEOUT) {
					if (paddata & PAD_SQUARE)
						setting->timeout-=60;
					else
						setting->timeout--;
					if (setting->timeout < 1)
						setting->timeout = 1;
				}
			}
			else if(new_pad & PAD_SQUARE){
				if (sel == LAUNCH_PATH)
					if(!strncmp(setting->dirElf[btn].path[elf], "mc", 2) && (setting->dirElf[btn].path[elf][3] == ':')){
						sprintf(c, "mc%s", &setting->dirElf[btn].path[elf][3]);
						strcpy(setting->dirElf[btn].path[elf], c);
					}
			}
		}
		if (redraw) {
			for(i=0;i<=BUTTONINIT;i++){
				if(i==0){
					strcpy(config[i], "..");
				}
				else if(i==LAUNCH_BTNNUM)
					if (btn == 0)
						sprintf(config[i], "%s: DEFAULT", lang->conf_launch_btnnum);
					else
						sprintf(config[i], "%s: %d", lang->conf_launch_btnnum, btn);
				else if(i==LAUNCH_NAME)
					sprintf(config[i], "%s: %s", lang->conf_launch_name, setting->dirElf[btn].name);
				else if(i==LAUNCH_PADMSK) {
					padmsktostr(c, setting->dirElf[btn].padmsk, "(none)");
					sprintf(config[i], "%s: %s", lang->conf_launch_padmsk, c);
				}
				else if(i==LAUNCH_ELFNUM)
					sprintf(config[i], "%s: %d", lang->conf_launch_elfnum, elf+1);
				else if(i==LAUNCH_PATH)
					sprintf(config[i], "%s: %s", lang->conf_launch_path, setting->dirElf[btn].path[elf]);
				else if(i==LAUNCH_LIST)
					strcpy(config[i], lang->conf_launch_list);
				else if(i==FILENAME){	//PRINT ONLY FILENAME
					sprintf(config[i], "%s: " ,lang->conf_print_only_filename);
					if(setting->filename)
						strcat(config[i], lang->conf_on);
					else
						strcat(config[i], lang->conf_off);
				}
				else if(i==FILEALL){	// DISPLAY ALL FILES
					sprintf(config[i], "%s: ", lang->conf_print_all_filename);
					if(setting->fileall)
						strcat(config[i], lang->conf_on);
					else
						strcat(config[i], lang->conf_off);
				}
				else if(i==TIMEOUT){	//TIMEOUT
					sprintf(config[i], "%s: %d", lang->conf_timeout, setting->timeout);
				}
				else if(i==BUTTONINIT){
					strcpy(config[i], lang->conf_buttonsettinginit);
				}
			}
			nList=i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_button);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if((sel==LAUNCH_BTNNUM)||(sel==LAUNCH_ELFNUM))
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
			else if(sel==LAUNCH_NAME)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==LAUNCH_PATH)
				if ((!strncmp(setting->dirElf[btn].path[elf], "mc", 2)) && (setting->dirElf[btn].path[elf][3] == ':'))
					sprintf(msg1, "○:%s ×:%s □:mc0,1->mc △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				else
					sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==LAUNCH_PADMSK)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==LAUNCH_LIST)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==FILENAME)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FILEALL)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==TIMEOUT)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
			else if(sel==BUTTONINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}

	for(i=0;i<MAX_BUTTON;i++){
		if(setting->dirElf[i].path[0][0]) return;
	}
	//ランチャー設定が何もないときSELECTにCONFIGをセット
	strcpy(setting->dirElf[MAX_BUTTON-1].path[0], "MISC/CONFIG");
	setting->dirElf[MAX_BUTTON-1].padmsk = PAD_SELECT;
	return;
}

//-------------------------------------------------
//配色設定
void config_color(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0 ,sel_x=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	int r,g,b;
	int colorid=0;

	int font_h;

	font_h = FONT_HEIGHT - GetFontMargin(LINE_MARGIN);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT){
				if(sel>=COLOR1 && sel<=COLOR12){
					sel_x--;
					if(sel_x<0){
						sel_x=2;
						sel--;
					}
				}
				else if (sel>FLICKER_ALPHA)
					sel=FLICKER_ALPHA;
				else
					sel=0;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel>=COLOR1 && sel<=COLOR12){
					sel_x++;
					if(sel_x>2){
						sel_x=0;
						sel++;
					}
				}
				else if (sel == 0)
					sel=FLICKER_ALPHA;
				else
					sel=PRESETCOLOR;
			}
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if((new_pad & PAD_CIRCLE)&&(sel != PRESETCOLOR)){
				if(sel==0) break;
				if(paddata & PAD_SQUARE) {i=8;} else {i=1;}
				if(sel>=COLOR1 && sel<=COLOR12){
					colorid = clutnum[sel];
					r = setting->color[colorid] & 0xFF;
					g = setting->color[colorid] >> 8 & 0xFF;
					b = setting->color[colorid] >> 16 & 0xFF;
					if(sel_x==0) {r+=i; if(r>255) r=255;}
					if(sel_x==1) {g+=i; if(g>255) g=255;}
					if(sel_x==2) {b+=i; if(b>255) b=255;}
					setting->color[colorid] = ITO_RGBA(r, g, b, 0);
					if (sel==COLOR12) itoSetBgColor(setting->color[COLOR_OUTSIDE]);
				} else if (sel == FLICKER_ALPHA) {
					setting->flicker_alpha+=i;
					if (setting->flicker_alpha>128)
						setting->flicker_alpha=128;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(paddata & PAD_SQUARE) {i=8;} else {i=1;}
				if(sel>=COLOR1 && sel<=COLOR12){
					colorid = clutnum[sel];
					r = setting->color[colorid] & 0xFF;
					g = setting->color[colorid] >> 8 & 0xFF;
					b = setting->color[colorid] >> 16 & 0xFF;
					if(sel_x==0) {r-=i; if(r<0) r=0;}
					if(sel_x==1) {g-=i; if(g<0) g=0;}
					if(sel_x==2) {b-=i; if(b<0) b=0;}
					setting->color[colorid] = ITO_RGBA(r, g, b, 0);
					if (sel==COLOR12) itoSetBgColor(setting->color[COLOR_OUTSIDE]);
				} else if (sel == FLICKER_ALPHA) {
					setting->flicker_alpha-=i;
					if (setting->flicker_alpha<0)
						setting->flicker_alpha=0;
				}
			}
			else if((new_pad & PAD_L3)||((new_pad & PAD_CIRCLE)&&(sel=PRESETCOLOR))){
				static int preset=0;
				setting->color[COLOR_BACKGROUND]	= clut[preset][0];
				setting->color[COLOR_FRAME]			= clut[preset][1];
				setting->color[COLOR_HIGHLIGHTTEXT]	= clut[preset][2];
				setting->color[COLOR_TEXT]			= clut[preset][3];
				setting->color[COLOR_DIR]			= clut[preset][4];
				setting->color[COLOR_FILE]			= clut[preset][5];
				setting->color[COLOR_PS2SAVE]		= clut[preset][6];
				setting->color[COLOR_ELF]			= clut[preset][7];
				setting->color[COLOR_PS1SAVE]		= clut[preset][8];
				setting->color[COLOR_GRAYTEXT]		= clut[preset][9];
				setting->color[COLOR_PSU]			= clut[preset][10];
				setting->color[COLOR_OUTSIDE]		= clut[preset][11];
				preset++;
				if(preset>=5) preset=0;
				itoSetBgColor(setting->color[COLOR_OUTSIDE]);
			}
		}

		//
		if (redraw) {
			for(i=0;i<=PRESETCOLOR;i++){
				if(i==0){
					sprintf(config[i], "..");
				}
				else if(i>=COLOR1 && i<=COLOR12){	//COLOR
					colorid = clutnum[i];
					r = setting->color[colorid] & 0xFF;
					g = setting->color[colorid] >> 8 & 0xFF;
					b = setting->color[colorid] >> 16 & 0xFF;

					if(i==COLOR1) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_background, r, g, b);
					if(i==COLOR2) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_frame, r, g, b);
					if(i==COLOR3) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_normaltext, r, g, b);
					if(i==COLOR4) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_highlighttext, r, g, b);
					if(i==COLOR5) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_disabletext, r, g, b);
					if(i==COLOR6) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_folder, r, g, b);
					if(i==COLOR7) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_file, r, g, b);
					if(i==COLOR8) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_ps2save, r, g, b);
					if(i==COLOR9) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_ps1save, r, g, b);
					if(i==COLOR10) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_elffile, r, g, b);
					if(i==COLOR11) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_psufile, r, g, b);
					if(i==COLOR12) sprintf(config[i], "%s:   R:%02X   G:%02X   B:%02X", lang->conf_outside, r, g, b);
				}
				else if(i==FLICKER_ALPHA)
					sprintf(config[i], "%s: %02X", lang->conf_flicker_alpha, setting->flicker_alpha);
				else if(i==PRESETCOLOR)	//INIT
					strcpy(config[i], lang->conf_presetcolor);
			}
			nList=15;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel){
					if(sel>=COLOR1 && sel<=COLOR12)
						printXY(">", FONT_WIDTH*21 + FONT_WIDTH*sel_x*7, y, color, TRUE);
					else
						printXY(">", x, y, color, TRUE);
				}
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				//色のプレビュー
				if(top+i>=COLOR1 && top+i<=COLOR12){
					colorid = clutnum[top+i];
					itoSprite(setting->color[colorid],
						x+FONT_WIDTH*42, y,
						x+FONT_WIDTH*42+font_h, y+font_h, 0);
				}
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_screen);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel>=COLOR1 && sel<=FLICKER_ALPHA)
				sprintf(msg1, "○:%s ×:%s +□:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_fast, lang->conf_up);
			else if(sel==PRESETCOLOR)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//画面設定
void config_screen(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;// ,sel_x=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	int font_h;

	font_h = FONT_HEIGHT - GetFontMargin(LINE_MARGIN);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				else if(sel==TVMODE){	//TVMODE
					//tvmode変更
					config_screen_mode(setting);
				}
				else if(sel==FONTHALF){	// fonthalf
					if (GetFontHalf() > -7) {
						SetFontHalf(GetFontHalf()-1);
						SetScreenPosXY();
						SetHeight();
					}
				}
				else if(sel==FONTVHALF){	// fontvhalf
					if (GetFontVHalf() > -7) {
						SetFontVHalf(GetFontVHalf()-1);
						SetScreenPosXY();
						SetHeight();
					}
				}
				else if(sel==FONTSCALEMODE){	// fonthalfmode
					fonthalfmode = (fonthalfmode +1) % 2;
					SetScreenPosXY();
				}
				else if(sel==FONTBOLDS){
					setting->font_bold[setting->tvmode] = (setting->font_bold[setting->tvmode]+1)%3;
					if (setting->font_bold[setting->tvmode] > 0)
						SetFontBold(setting->font_bold[setting->tvmode]-1);
					else
						SetFontBold(setting->FontBold);
				}
				else if(sel==SCREEN_X){	//SCREEN X
					if (SCREEN_LEFT < 256){
						SCREEN_LEFT++;
						screen_env.screen.x++;
						SetScreenPosXY();
						itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
					}
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					if (SCREEN_TOP < 256){
						SCREEN_TOP++;
						screen_env.screen.y++;
						SetScreenPosXY();
						itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
					}
				}
				else if(sel==FLICKERCONTROL){	//フリッカーコントロール
					//setting->flickerControl = !setting->flickerControl;
					flickerfilter = !flickerfilter;
					SetScreenPosXY();
				}
				else if(sel==SCREENINIT){	//SCREEN SETTING INIT
					//init
					InitScreenSetting();
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
					//sprintf(msg0, "%s", "Initialize Screen Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==FONTHALF){	// fonthalf
					if (GetFontHalf() < 7) {
						SetFontHalf(GetFontHalf()+1);
						SetScreenPosXY();
						SetHeight();
					}
				}
				else if(sel==FONTVHALF){	// fontvhalf
					if (GetFontVHalf() < 7) {
						SetFontVHalf(GetFontVHalf()+1);
						SetScreenPosXY();
						SetHeight();
					}
				}
				else if(sel==SCREEN_X){	//SCREEN X
					if((SCREEN_LEFT > -256) && (screen_env.screen.x > 0)){
						SCREEN_LEFT--;
						screen_env.screen.x--;
						SetScreenPosXY();
						itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
					}
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					if((SCREEN_TOP > -256) && (screen_env.screen.y > 0)){
						SCREEN_TOP--;
						screen_env.screen.y--;
						SetScreenPosXY();
						itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
					}
				}
			}
			else if(new_pad & PAD_SQUARE){
				if(sel==TVMODE){	//TVMODE
					config_screen_edit(setting);
				}
				else if(sel==FONTHALF){	// fonthalf
					SetFontHalf(0);
					SetScreenPosXY();
					SetHeight();
				}
				else if(sel==FONTVHALF){	// fontvhalf
					SetFontVHalf(0);
					SetScreenPosXY();
					SetHeight();
				}
				else if(sel==SCREEN_X){
					screen_env.screen.x-=SCREEN_LEFT;
					SCREEN_LEFT = 0;
					SetScreenPosXY();
					itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
				}
				else if(sel==SCREEN_Y){
					screen_env.screen.y-=SCREEN_TOP;
					SCREEN_TOP = 0;
					SetScreenPosXY();
					itoSetScreenPos(screen_env.screen.x, screen_env.screen.y);
				}
			}
		}

		//
		if (redraw) {
			for(i=0;i<=SCREENINIT;i++){
				if(i==0){
					sprintf(config[i], "..");
				}
				else if(i==TVMODE){	//TVMODE
					if(setting->tvmode==0)
						sprintf(config[i],"%s: %s", lang->conf_tvmode, "AUTO");
					else if(setting->tvmode>0)
						sprintf(config[i],"%s: %s", lang->conf_tvmode, gsregs[setting->tvmode].name);
				}
				else if(i==FONTHALF){	// FONTHALF
					if (font_half < 0) {
						sprintf(config[i], "%s: x%d", lang->conf_FontHalf, -font_half+1);
					} else if (font_half > 0) {
						sprintf(config[i], "%s: 1/%d", lang->conf_FontHalf, font_half+1);
					} else {
						sprintf(config[i], "%s: %s", lang->conf_FontHalf, lang->conf_off);
					}
				}
				else if(i==FONTVHALF){	// FONTVHALF
					if (font_vhalf < 0) {
						sprintf(config[i], "%s: x%d", lang->conf_FontVHalf, -font_vhalf+1);
					} else if (font_vhalf > 0) {
						sprintf(config[i], "%s: 1/%d", lang->conf_FontVHalf, font_vhalf+1);
					} else {
						sprintf(config[i], "%s: %s", lang->conf_FontVHalf, lang->conf_off);
					}
				}
				else if(i==FONTSCALEMODE){	//FONTSCALEMODE
					sprintf(config[i], "%s: ", lang->conf_FontScaler);
					if (fonthalfmode == 0)
						strcat(config[i], lang->conf_FontScaler_A);
					else if (fonthalfmode == 1)
						strcat(config[i], lang->conf_FontScaler_B);
					else if (fonthalfmode == 2)
						strcat(config[i], lang->conf_FontScaler_C);
				}
				else if(i==FONTBOLDS){	//FONTBOLD
					char *fbtable[3] = {lang->conf_default, lang->conf_off, lang->conf_on};
					sprintf(config[i], "%s: %s", lang->conf_FontBold, fbtable[(int) setting->font_bold[setting->tvmode]]);
				}
				else if(i==SCREEN_X){	//SCREEN X
					sprintf(config[i],"%s:%+4d", lang->conf_screen_x, SCREEN_LEFT);
				}
				else if(i==SCREEN_Y){	//SCREEN Y
					sprintf(config[i],"%s:%+4d", lang->conf_screen_y, SCREEN_TOP);
				}
				else if(i==FLICKERCONTROL){	//FLICKER CONTROL
					sprintf(config[i],"%s: ", lang->conf_flickercontrol);
					if(flickerfilter)
						strcat(config[i], lang->conf_on);
					else
						strcat(config[i], lang->conf_off);
				}
				else if(i==SCREENINIT){	//INIT
					strcpy(config[i], lang->conf_screensettinginit);
				}
			}
			nList=i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_screen);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==TVMODE)
				sprintf(msg1, "○:%s □:%s △:%s", lang->conf_change, lang->conf_detail, lang->conf_up);
			else if(sel==FONTHALF)
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_off, lang->conf_up);
			else if(sel==FONTVHALF)
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_off, lang->conf_up);
			else if(sel==FONTSCALEMODE)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FONTBOLDS)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==SCREEN_X)
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_default, lang->conf_up);
			else if(sel==SCREEN_Y)
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_default, lang->conf_up);
			else if(sel==FLICKERCONTROL)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==SCREENINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//フォント設定
void config_font(SETTING *setting)
{
	char c[MAX_PATH];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char newFontName[MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==ASCIIFONT){
					getFilePath(newFontName, FNT_FILE);
					//フォントを適用してみる
					if(InitFontAscii(newFontName)<0){
						//失敗したとき元に戻す
						if(InitFontAscii(setting->AsciiFont)<0){
							//元に戻すのを失敗したとき
							strcpy(setting->AsciiFont, "systemfont");
							InitFontAscii(setting->AsciiFont);
						}
					}
					else{
						//成功したとき
						strcpy(setting->AsciiFont, newFontName);
						if(!strncmp(setting->AsciiFont, "mc", 2)){
							sprintf(c, "mc%s", &setting->AsciiFont[3]);
							strcpy(setting->AsciiFont, c);
						}
					}
				}
				else if(sel==KANJIFONT){
					getFilePath(newFontName, FNT_FILE);
					//フォントを適用してみる
					if(InitFontKnaji(newFontName)<0){
						//失敗したとき元に戻す
						if(InitFontKnaji(setting->KanjiFont)<0){
							//元に戻すのを失敗したとき
							strcpy(setting->KanjiFont, "systemfont");
							InitFontKnaji(setting->KanjiFont);
						}
					}
					else{
						//成功したとき
						strcpy(setting->KanjiFont, newFontName);
						if(!strncmp(setting->KanjiFont, "mc", 2)){
							sprintf(c, "mc%s", &setting->KanjiFont[3]);
							strcpy(setting->KanjiFont, c);
						}
					}
				}
				else if(sel==CHARMARGIN){
					setting->CharMargin++;
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
				}
				else if(sel==LINEMARGIN){
					setting->LineMargin++;
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
				}
				else if(sel==FONTBOLD){
					setting->FontBold = !setting->FontBold ;
					if (setting->font_bold[setting->tvmode] > 0)
						SetFontBold(setting->font_bold[setting->tvmode]-1);
					else
						SetFontBold(setting->FontBold);
				}
				else if(sel==ASCIIMARGINTOP){
					setting->AsciiMarginTop++;
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
				}
				else if(sel==ASCIIMARGINLEFT){
					setting->AsciiMarginLeft++;
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
				}
				else if(sel==KANJIMARGINTOP){
					setting->KanjiMarginTop++;
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
				}
				else if(sel==KANJIMARGINLEFT){
					setting->KanjiMarginLeft++;
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
				}
				else if(sel==FONTINIT){
					//init
					InitFontSetting();
					InitFontAscii(setting->AsciiFont);
					InitFontKnaji(setting->KanjiFont);
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
					//sprintf(msg0, "%s", "Initialize Font Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==ASCIIFONT){
					strcpy(setting->AsciiFont, "systemfont");
					InitFontAscii(setting->AsciiFont);
				}
				else if(sel==KANJIFONT){
					strcpy(setting->KanjiFont, "systemfont");
					InitFontKnaji(setting->KanjiFont);
				}
				else if(sel==CHARMARGIN){
					setting->CharMargin--;
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
				}
				else if(sel==LINEMARGIN){
					if(FONT_HEIGHT>1) setting->LineMargin--;
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
				}
				else if(sel==ASCIIMARGINTOP){
					setting->AsciiMarginTop--;
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
				}
				else if(sel==ASCIIMARGINLEFT){
					setting->AsciiMarginLeft--;
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
				}
				else if(sel==KANJIMARGINTOP){
					setting->KanjiMarginTop--;
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
				}
				else if(sel==KANJIMARGINLEFT){
					setting->KanjiMarginLeft--;
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
				}
			}
			else if(new_pad & PAD_SQUARE){
				if(sel==CHARMARGIN){
					setting->CharMargin=DEF_CHAR_MARGIN;
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
				}
				else if(sel==LINEMARGIN){
					setting->LineMargin=DEF_LINE_MARGIN;
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
				}
				else if(sel==ASCIIMARGINTOP){
					setting->AsciiMarginTop=DEF_ASCII_MARGINTOP;
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
				}
				else if(sel==ASCIIMARGINLEFT){
					setting->AsciiMarginLeft=DEF_ASCII_MARGINLEFT;
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
				}
				else if(sel==KANJIMARGINTOP){
					setting->KanjiMarginTop=DEF_KANJI_MARGINTOP;
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
				}
				else if(sel==KANJIMARGINLEFT){
					setting->KanjiMarginLeft=DEF_KANJI_MARGINLEFT;
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
				}
			}
		}

		//
		if (redraw) {
			for(i=0;i<=FONTINIT;i++){
				if(i==0){
					sprintf(config[i], "..");
				}
				else if(i==ASCIIFONT){	//ASCIIFONT
					sprintf(config[i], "%s: %s", lang->conf_AsciiFont, setting->AsciiFont);
				}
				else if(i==KANJIFONT){	//KANJIFONT
					sprintf(config[i], "%s: %s", lang->conf_KanjiFont, setting->KanjiFont);
				}
				else if(i==CHARMARGIN){	//CHARMARGIN
					sprintf(config[i], "%s: %d", lang->conf_CharMargin, setting->CharMargin);
				}
				else if(i==LINEMARGIN){	//LINEMARGIN
					sprintf(config[i], "%s: %d", lang->conf_LineMargin, setting->LineMargin);
				}
				else if(i==FONTBOLD){	//FONTBOLD
					sprintf(config[i], "%s: ", lang->conf_FontBold);
					if(setting->FontBold)
						strcat(config[i], lang->conf_on);
					else
						strcat(config[i], lang->conf_off);
				}
				else if(i==ASCIIMARGINTOP){	//ASCIIMARGINTOP
					sprintf(config[i], "%s: %d", lang->conf_AsciiMarginTop, setting->AsciiMarginTop);
				}
				else if(i==ASCIIMARGINLEFT){	//ASCIIMARGINLEFT
					sprintf(config[i], "%s: %d", lang->conf_AsciiMarginLeft, setting->AsciiMarginLeft);
				}
				else if(i==KANJIMARGINTOP){	//KANJIMARGINTOP
					sprintf(config[i], "%s: %d", lang->conf_KanjiMarginTop, setting->KanjiMarginTop);
				}
				else if(i==KANJIMARGINLEFT){	//KANJIMARGINLEFT
					sprintf(config[i], "%s: %d", lang->conf_KanjiMarginLeft, setting->KanjiMarginLeft);
				}
				else if(i==FONTINIT){	//FONT INIT
					strcpy(config[i], lang->conf_fontsettinginit);
				}
			}
			nList=i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_font);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==ASCIIFONT)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==KANJIFONT)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==FONTBOLD)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FONTINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_default, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//ビューアの設定
void config_viewer(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char *onoff[2] = {lang->conf_off, lang->conf_on};

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				else if(sel==TXT_LINENUMBER){
					setting->txt_linenumber = !setting->txt_linenumber;
				}
				else if(sel==TXT_TABSPACES){
					setting->txt_tabmode = (setting->txt_tabmode % 12) + 2;
				}
				else if(sel==TXT_CRLFTABDISP){
					setting->txt_chardisp = !setting->txt_chardisp;
				}
				else if(sel==IMG_FULLSCREEN){
					setting->img_fullscreen = !setting->img_fullscreen;
				}
				else if(sel==VIEWERINIT){
					//init
					InitViewerSetting();
				}
				set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==TXT_TABSPACES){
					if (setting->txt_tabmode > 2)
						setting->txt_tabmode = setting->txt_tabmode - 2;
					else
						setting->txt_tabmode = 12;
					set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);
				}
			}
		}

		//
		if (redraw) {
			for(i=0;i<=VIEWERINIT;i++){
				if(i==0){
					sprintf(config[i], "..");
				}
				else if(i==TXT_LINENUMBER){
					sprintf(config[i], "%s: %s", lang->conf_linenumber, onoff[setting->txt_linenumber & 1]);
				}
				else if(i==TXT_TABSPACES){
					sprintf(config[i], "%s: %2d", lang->conf_tabspaces, setting->txt_tabmode);
				}
				else if(i==TXT_CRLFTABDISP){
					sprintf(config[i], "%s: %s", lang->conf_chardisp, onoff[setting->txt_chardisp & 1]);
				}
				else if(i==IMG_FULLSCREEN){
					sprintf(config[i], "%s: %s", lang->conf_fullscreen, onoff[setting->img_fullscreen & 1]);
				}
				else if(i==VIEWERINIT){
					strcpy(config[i], lang->conf_viewerinit);
				}
			}
			nList=i;

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
			if(top < 0)			top=0;
			if(sel >= nList)		sel=nList-1;
			if(sel < 0)			sel=0;
			if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
			if(sel < top)			top=sel;

			// 画面描画開始
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_view);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==TXT_LINENUMBER)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==TXT_TABSPACES)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
			else if(sel==TXT_CRLFTABDISP)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==IMG_FULLSCREEN)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==VIEWERINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//その他設定
void config_misc(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[8][MAX_PATH];
	char *onoff[2] = {lang->conf_off, lang->conf_on};

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==LANG){
					setting->language++;
					if(setting->language==NUM_LANG) setting->language=LANG_ENGLISH;
					SetLanguage(setting->language);
				}
				else if(sel==DISCCONTROL)
					setting->discControl = !setting->discControl;
				else if(sel==MISCINIT){
					//init
					InitMiscSetting();
					SetLanguage(setting->language);
					//sprintf(msg0, "%s", "Initialize Misc Setting");
					//pushed = FALSE;
				}
			}
		}

		//
		for(i=0;i<=MISCINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==LANG){	//LANG
				sprintf(config[i], "%s: ", lang->conf_language);
				if(setting->language==LANG_ENGLISH)
					strcat(config[i], lang->conf_language_us);
				else if(setting->language==LANG_JAPANESE)
					strcat(config[i], lang->conf_language_jp);
			}
			else if(i==DISCCONTROL){	//DISC CONTROL
				sprintf(config[i], "%s: %s" ,lang->conf_disc_control, onoff[setting->discControl != 0]);
			}
			else if(i==MISCINIT){	//INIT
				strcpy(config[i], lang->conf_miscsettinginit);
			}
		}
		nList=i;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==LANG)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==DISCCONTROL)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==MISCINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//ファイラーの設定
void config_filer(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char *onoff[2] = {lang->conf_off, lang->conf_on};

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel==FILEICON)
					setting->fileicon = !setting->fileicon;
				else if(sel==PS2SAVECHECK)
					setting->discPs2saveCheck = !setting->discPs2saveCheck;
				else if(sel==ELFCHECK)
					setting->discELFCheck = !setting->discELFCheck;
				else if(sel==FILEPS2SAVECHECK)
					setting->filePs2saveCheck = !setting->filePs2saveCheck;
				else if(sel==FILEELFCHECK)
					setting->fileELFCheck = !setting->fileELFCheck;
				else if(sel==EXPORTDIR){
					getFilePath(setting->Exportdir, DIR);
					if(!strncmp(setting->Exportdir, "cdfs", 2))
						setting->Exportdir[0]='\0';
				}
				else if(sel==DEFAULTTITLE)
					setting->defaulttitle = !setting->defaulttitle;
				else if(sel==DEFAULTDETAIL){
					setting->defaultdetail++;
					if(setting->defaultdetail>3) setting->defaultdetail = 0;
				}
				else if(sel==SORT_TYPE){
					setting->sort++;
					if(setting->sort > 5) setting->sort = 0;
				}
				else if(sel==SORT_DIR){
					setting->sortdir^=1;
				}
				else if(sel==SORT_EXT){
					setting->sortext^=1;
				}
				else if(sel==FILERINIT){
					//init
					InitFilerSetting();
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==EXPORTDIR)
					setting->Exportdir[0]='\0';
				if(sel==SORT_TYPE){
					if(setting->sort <= 0) setting->sort = 6;
					setting->sort--;
				}
			}
		}

		//
		for(i=0;i<=FILERINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==FILEICON){	//FILEICON
				sprintf(config[i], "%s: %s" ,lang->conf_fileicon, onoff[setting->fileicon != 0]);
			}
			else if(i==PS2SAVECHECK){	//DISC PS2SAVE CHECK
				sprintf(config[i], "%s: %s" ,lang->conf_disc_ps2save_check, onoff[setting->discPs2saveCheck != 0]);
			}
			else if(i==ELFCHECK){	//DISC ELF CHECK
				sprintf(config[i], "%s: %s" ,lang->conf_disc_elf_check, onoff[setting->discELFCheck != 0]);
			}
			else if(i==FILEPS2SAVECHECK){	//FILE PS2SAVE CHECK
				sprintf(config[i], "%s: %s" ,lang->conf_file_ps2save_check, onoff[setting->filePs2saveCheck != 0]);
			}
			else if(i==FILEELFCHECK){	// FILE ELF CHECK
				sprintf(config[i], "%s: %s", lang->conf_file_elf_check, onoff[setting->fileELFCheck != 0]);
			}
			else if(i==EXPORTDIR){	//EXPORT DIR
				sprintf(config[i], "%s: %s", lang->conf_export_dir, setting->Exportdir);
			}
			else if(i==DEFAULTTITLE){	//DEFAULTTITLE
				sprintf(config[i], "%s: %s", lang->conf_defaulttitle, onoff[setting->defaulttitle != 0]);
			}
			else if(i==DEFAULTDETAIL){	//DEFAULTDETAIL
				sprintf(config[i], "%s: ", lang->conf_defaultdetail);
				if(setting->defaultdetail==0)
					strcat(config[i], lang->conf_defaultdetail_none);
				else if(setting->defaultdetail==1)
					strcat(config[i], lang->conf_defaultdetail_size);
				else if(setting->defaultdetail==2)
					strcat(config[i], lang->conf_defaultdetail_modifytime);
				else if(setting->defaultdetail==3)
					strcat(config[i], lang->conf_defaultdetail_both);
			}
			else if(i==SORT_TYPE){
				//char *sorttypes[4] = {lang->conf_sort_none, lang->conf_sort_file, lang->conf_sort_game, lang->conf_sort_time};
				//sprintf(config[i], "%s: %s", lang->conf_sort_type, sorttypes[setting->sort]);
				sprintf(config[i], "%s: %s", lang->conf_sort_type, lang->conf_sort_types[setting->sort & 7]);
			}
			else if(i==SORT_DIR){
				sprintf(config[i], "%s: %s", lang->conf_sort_dir, onoff[setting->sortdir != 0]);
			}
			else if(i==SORT_EXT){
				sprintf(config[i], "%s: %s", lang->conf_sort_ext, onoff[setting->sortext != 0]);
			}
			else if(i==FILERINIT){	//INIT
				strcpy(config[i], lang->conf_filersettinginit);
			}
		}
		nList=i;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==FILEICON)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==PS2SAVECHECK)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==ELFCHECK)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FILEPS2SAVECHECK)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FILEELFCHECK)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==EXPORTDIR)
				sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==DEFAULTTITLE)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==DEFAULTDETAIL)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==SORT_TYPE)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==SORT_DIR)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==SORT_EXT)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==FILERINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//デバイス設定
void config_device(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0, redraw=fieldbuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char *onoff[2] = {lang->conf_off, lang->conf_on};

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = fieldbuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				else if(sel==USBD_FLAG)
					setting->usbd_flag = !setting->usbd_flag;
				else if(sel==USBD_PATH){
					getFilePath(setting->usbd_path, IRX_FILE);
					if(!strncmp(setting->usbd_path, "mass", 4))
						setting->usbd_path[0]='\0';
				}
				else if(sel==USBMASS_FLAG)
					setting->usbmass_flag = !setting->usbmass_flag;
				else if(sel==USBMASS_PATH){
					getFilePath(setting->usbmass_path, IRX_FILE);
					if(!strncmp(setting->usbmass_path, "mass", 4))
						setting->usbmass_path[0]='\0';
				}
				else if(sel==USBM_DEVICES){
					if (setting->usbmdevs < 10) setting->usbmdevs++;
					if (setting->usbmdevs == 1) setting->usbmdevs++;
				}
				else if(sel==USBKBD_FLAG)
					setting->usbkbd_flag = !setting->usbkbd_flag;
				else if(sel==USBKBD_PATH){
					getFilePath(setting->usbkbd_path, IRX_FILE);
				}
				else if(sel==USBMOUSE_FLAG)
					setting->usbmouse_flag = !setting->usbmouse_flag;
				else if(sel==USBMOUSE_PATH){
					getFilePath(setting->usbmouse_path, IRX_FILE);
				}
				else if(sel==DEVICEINIT){
					//init
					InitDeviceSetting();
				}
			}
			else if(new_pad & PAD_SQUARE){	//□
				if((sel==USBD_PATH)&&(!strncmp(setting->usbd_path, "mc", 2))&&(setting->usbd_path[3] == ':'))
					strcpy(setting->usbd_path+2, setting->usbd_path+3);
				if((sel==USBMASS_PATH)&&(!strncmp(setting->usbmass_path, "mc", 2))&&(setting->usbmass_path[3] == ':'))
					strcpy(setting->usbmass_path+2, setting->usbmass_path+3);
				if(sel==USBM_DEVICES)
					setting->usbmdevs = DEF_USBM_DEVICES;
				if((sel==USBKBD_PATH)&&(!strncmp(setting->usbkbd_path, "mc", 2))&&(setting->usbkbd_path[3] == ':'))
					strcpy(setting->usbkbd_path+2, setting->usbkbd_path+3);
				if((sel==USBMOUSE_PATH)&&(!strncmp(setting->usbmouse_path, "mc", 2))&&(setting->usbmouse_path[3] == ':'))
					strcpy(setting->usbmouse_path+2, setting->usbmouse_path+3);
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==USBD_PATH)
					setting->usbd_path[0]='\0';
				if(sel==USBMASS_PATH)
					setting->usbmass_path[0]='\0';
				if(sel==USBM_DEVICES){
					if (setting->usbmdevs > 0) setting->usbmdevs--;
					if (setting->usbmdevs == 1) setting->usbmdevs--;
				}
				if(sel==USBKBD_PATH)
					setting->usbkbd_path[0]='\0';
				if(sel==USBMOUSE_PATH)
					setting->usbmouse_path[0]='\0';
			}
		}

		//
		for(i=0;i<=DEVICEINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==USBD_FLAG){	//USBD_USE
				sprintf(config[i], "%s: %s", lang->conf_usbd_use, onoff[setting->usbd_flag != 0]);
			}
			else if(i==USBD_PATH)	//USBD_PATH
				sprintf(config[i], "%s: %s", lang->conf_usbd_path, setting->usbd_path);
			else if(i==USBMASS_FLAG){	//USBMASS_USE
				sprintf(config[i], "%s: %s", lang->conf_usbmass_use, onoff[setting->usbmass_flag != 0]);
			}
			else if(i==USBMASS_PATH)	//USBMASS_PATH
				sprintf(config[i], "%s: %s", lang->conf_usbmass_path, setting->usbmass_path);
			else if(i==USBM_DEVICES){
				if (setting->usbmdevs==1)
					sprintf(config[i], "%s: AUTO", lang->conf_usbmass_devs);
				else if (setting->usbmdevs)
					sprintf(config[i], "%s: %d", lang->conf_usbmass_devs, setting->usbmdevs);
				else
					sprintf(config[i], "%s: %s", lang->conf_usbmass_devs, lang->conf_off);
			}
			else if(i==USBKBD_FLAG)
				sprintf(config[i], "%s: %s", lang->conf_usbkbd_use, onoff[setting->usbkbd_flag != 0]);
			else if(i==USBKBD_PATH)
				sprintf(config[i], "%s: %s", lang->conf_usbkbd_path, setting->usbkbd_path);
			else if(i==USBMOUSE_FLAG)
				sprintf(config[i], "%s: %s", lang->conf_usbmouse_use, onoff[setting->usbmouse_flag != 0]);
			else if(i==USBMOUSE_PATH)
				sprintf(config[i], "%s: %s", lang->conf_usbmouse_path, setting->usbmouse_path);
			else if(i==DEVICEINIT){	//INIT
				strcpy(config[i], lang->conf_devicesettinginit);
			}
		}
		nList=i;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
			// 操作説明
			if(sel==0)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			else if(sel==USBD_FLAG)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==USBD_PATH)
				if ((!strncmp(setting->usbd_path, "mc", 2)) && (setting->usbd_path[3] == ':'))
					sprintf(msg1, "○:%s ×:%s □:mcx->mc △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				else
					sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==USBMASS_FLAG)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==USBMASS_PATH)
				if ((!strncmp(setting->usbmass_path, "mc", 2)) && (setting->usbmass_path[3] == ':'))
					sprintf(msg1, "○:%s ×:%s □:mcx->mc △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				else
					sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
			else if(sel==USBM_DEVICES)
				sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_default, lang->conf_up);
			else if(sel==USBKBD_FLAG)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==USBKBD_PATH)
				if ((!strncmp(setting->usbkbd_path, "mc", 2)) && (setting->usbkbd_path[3] == ':'))
					sprintf(msg1, "○:%s ×:%s □:mcx->mc △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				else
					sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				
			else if(sel==USBMOUSE_FLAG)
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
			else if(sel==USBMOUSE_PATH)
				if ((!strncmp(setting->usbmouse_path, "mc", 2)) && (setting->usbmouse_path[3] == ':'))
					sprintf(msg1, "○:%s ×:%s □:mcx->mc △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				else
					sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
				
			else if(sel==DEVICEINIT)
				sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}

//-------------------------------------------------
//設定
void config(char *mainMsg)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList, sel=0, top=0, redraw=framebuffers;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	tmpsetting = setting;
	setting = (SETTING*)malloc(sizeof(SETTING));
	*setting = *tmpsetting;

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) redraw=fieldbuffers;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				sel=OK;
			else if(new_pad & PAD_CIRCLE){
				if(sel==BUTTONSETTING) config_button(setting);
				if(sel==FILERSETTING) config_filer(setting);
				if(sel==SCREENSETTING) config_screen(setting);
				if(sel==COLORSETTING) config_color(setting);
				if(sel==FONTSETTING) config_font(setting);
				if(sel==DEVICESETTING) config_device(setting);
				if(sel==VIEWERSETTING) config_viewer(setting);
				if(sel==MISC) config_misc(setting);
				if(sel==OK){
					free(tmpsetting);
					saveConfig(mainMsg);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					break;
				}
				if(sel==CANCEL){	//cansel
					free(setting);
					setting = tmpsetting;
					SetLanguage(setting->language);
					if(InitFontAscii(setting->AsciiFont)<0){
						strcpy(setting->AsciiFont, "systemfont");
						InitFontAscii(setting->AsciiFont);
					}
					if(InitFontKnaji(setting->KanjiFont)<0){
						strcpy(setting->KanjiFont, "systemfont");
						InitFontKnaji(setting->KanjiFont);
					}
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontBold(setting->FontBold);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
					SetScreenPosVM();
					SetFontHalf(font_half);
					SetFontVHalf(font_vhalf);
					SetHeight();
					itoVSync();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
					set_viewerconfig(setting->txt_linenumber, setting->txt_tabmode, setting->txt_chardisp, setting->img_fullscreen);
					mainMsg[0] = 0;
					break;
				}
				redraw=fieldbuffers;
			}
			else if(new_pad & PAD_START)
				sel=OK;
			else if(new_pad & PAD_SELECT)
				sel=CANCEL;
		}
		//
		strcpy(config[0], lang->conf_setting_button);
		strcpy(config[1], lang->conf_setting_filer);
		strcpy(config[2], lang->conf_setting_screen);
		strcpy(config[3], lang->conf_setting_color);
		strcpy(config[4], lang->conf_setting_font);
		strcpy(config[5], lang->conf_setting_device);
		strcpy(config[6], lang->conf_setting_view);
		strcpy(config[7], lang->conf_setting_misc);
		strcpy(config[8], lang->conf_ok);
		strcpy(config[9], lang->conf_cancel);
		nList=10;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);

			// リスト
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				//カーソル表示
				if(top+i == sel)
					printXY(">", x, y, color, TRUE);
				//リスト表示
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
				y += FONT_HEIGHT;
			}

			// スクロールバー
			if(nList > MAX_ROWS){
				drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
					(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[COLOR_FRAME]);
				y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
				y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
				itoSprite(setting->color[COLOR_FRAME],
					(MAX_ROWS_X+8)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y0,
					(MAX_ROWS_X+9)*FONT_WIDTH,
					SCREEN_MARGIN+FONT_HEIGHT*3+y1,
					0);
			}
			// メッセージ
			strcpy(msg0, "CONFIG/");
			// 操作説明
			sprintf(msg1, "○:%s", lang->gen_ok);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return;
}
