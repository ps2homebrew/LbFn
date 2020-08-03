#include "launchelf.h"

//デフォルトの設定の値
enum
{
	DEF_TIMEOUT = 10,
	DEF_FILENAME = TRUE,
	DEF_COLOR1 = ITO_RGBA(30,30,50,0),		//背景
	DEF_COLOR2 = ITO_RGBA(64,64,80,0),		//枠
	DEF_COLOR3 = ITO_RGBA(192,192,192,0),	//強調の文字色
	DEF_COLOR4 = ITO_RGBA(128,128,128,0),	//通常の文字色
	DEF_COLOR5 = ITO_RGBA(128,128,0,0),	//フォルダ
	DEF_COLOR6 = ITO_RGBA(128,128,128,0),	//ファイル
	DEF_COLOR7 = ITO_RGBA(0,128,0,0),		//PS2saveフォルダ
	DEF_COLOR8 = ITO_RGBA(128,0,0,0),		//ELFファイル
	DEF_COLOR9 = ITO_RGBA(0,128,255,0),		//PS1saveフォルダ
	DEF_COLOR10 = ITO_RGBA(64,64,80,0),		//無効の文字色
	DEF_COLOR11 = ITO_RGBA(192,96,0,0),		//psuファイル
	DEF_FLICKER_ALPHA = 0x4C,
	DEF_SCREEN_X_D1 = 639,
	DEF_SCREEN_Y_D1 = 50,
	DEF_SCREEN_X_D2 = 310,
	DEF_SCREEN_Y_D2 = 50,
	DEF_SCREEN_X_D3 = 283,
	DEF_SCREEN_Y_D3 = 66,
	DEF_SCREEN_X_D4 = 331,
	DEF_SCREEN_Y_D4 = 42,
	DEF_SCREEN_SCAN = FALSE,	//FALSE=NORMAL TRUE=FULL
	DEF_FULLHD_WIDTH = 960,
	DEF_FLICKERCONTROL = TRUE,
	DEF_TVMODE = 0,	//0=auto 1=ntsc 2=pal 3=480p 4=720p 5=1080i
	DEF_INTERLACE = TRUE,	//FALSE=ITO_NON_INTERLACE TRUE=ITO_INTERLACE
	DEF_FFMODE = FALSE,	//FALSE=ITO_FIELD TRUE=ITO_FRAME
	DEF_DEFAULTTITLE = 0,
	DEF_DEFAULTDETAIL = 0,

	DEF_CHAR_MARGIN = 2,
	DEF_LINE_MARGIN = 5,
	DEF_FONTBOLD = TRUE,
	DEF_FONTHALF_480i = 0,	// 0: 標準, -1: 2倍拡大, +1: 1/2縮小
	DEF_FONTHALF_480p = 0,
	DEF_FONTHALF_720p = 0,
	DEF_FONTHALF_1080i = 1,
	DEF_FONTVHALF_480i = 0,
	DEF_FONTVHALF_480p = 0,
	DEF_FONTVHALF_720p = 0,
	DEF_FONTVHALF_1080i = 0,
	DEF_ASCII_MARGINTOP = 0,
	DEF_ASCII_MARGINLEFT = 0,
	DEF_KANJI_MARGINTOP = 0,
	DEF_KANJI_MARGINLEFT = 0,

	DEF_DISCCONTROL = FALSE,
	DEF_FILEICON = TRUE,
	DEF_DISCPS2SAVECHECK = FALSE,
	DEF_DISCELFCHECK = FALSE,
	DEF_FILEPS2SAVECHECK = TRUE,
	DEF_FILEELFCHECK = TRUE,
	DEF_LANGUAGE = LANG_ENGLISH,
	DEF_USBMASS_FLAG = FALSE,	// 0:Inside Only
	DEF_USBD_FLAG = FALSE,
};

//CONFIG
enum
{
	BUTTONSETTING=0,
	COLORSETTING,
	SCREENSETTING,
	NETWORK,
	FONTSETTING,
	MISC,
	OK,
	CANCEL,
};

//BUTTON SETTING
enum
{
	DEFAULT=1,
	LAUNCHER1,
	LAUNCHER2,
	LAUNCHER3,
	LAUNCHER4,
	LAUNCHER5,
	LAUNCHER6,
	LAUNCHER7,
	LAUNCHER8,
	LAUNCHER9,
	LAUNCHER10,
	LAUNCHER11,
	LAUNCHER12,
	BUTTONINIT,
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
	FLICKER_ALPHA,
	PRESETCOLOR,
};

//SCREEN SETTING
enum
{
	TVMODE=1,
	SCREENSIZE,
	INTERLACE,
	FFMODE,
	FONTHALF,
	FONTVHALF,
	SCREEN_X,
	SCREEN_Y,
	FLICKERCONTROL,
	SCREENINIT,
};

//NETWORK SETTING
enum
{
	IPADDRESS=1,
	NETMASK,
	GATEWAY,
	NETWORKSAVE,
	NETWORKINIT,
};

//FONT SETTING
enum
{
	ASCIIFONT=1,
	KANJIFONT,
	CHARMARGIN,
	LINEMARGIN,
	FONTBOLD,
	//FONTHALF,
	ASCIIMARGINTOP,
	ASCIIMARGINLEFT,
	KANJIMARGINTOP,
	KANJIMARGINLEFT,
	FONTINIT,
};

//MISC SETTING
enum
{
	LANG=1,
	TIMEOUT,
	DISCCONTROL,
	FILENAME,
	FILEICON,
	PS2SAVECHECK,
	ELFCHECK,
	FILEPS2SAVECHECK,
	FILEELFCHECK,
	EXPORTDIR,
	DEFAULTTITLE,
	DEFAULTDETAIL,
	USBD_FLAG,
	USBD_PATH,
	USBMASS_FLAG,
	USBMASS_PATH,
	MISCINIT,
};

SETTING *setting, *tmpsetting;

//-------------------------------------------------
//スクリーンXY反映用
void SetScreenPosVM()
{
	switch(setting->tvmode)
	{
		case 0:	//AUTO
		case 1:
		case 2:
		{
			SCREEN_LEFT = setting->screen_x_480i;
			SCREEN_TOP = setting->screen_y_480i;
			interlace = setting->interlace;
			ffmode = setting->ffmode_480i;
			screenscan = setting->screen_scan_480i;
			font_half = setting->FontHalf_480i;
			font_vhalf = setting->FontVHalf_480i;
			break;
		}
		case 3:	//480p
		{
			SCREEN_LEFT = setting->screen_x_480p;
			SCREEN_TOP = setting->screen_y_480p;
			interlace = ITO_NON_INTERLACE;
			ffmode = ITO_FIELD;
			screenscan = setting->screen_scan_480p;
			font_half = setting->FontHalf_480p;
			font_vhalf = setting->FontVHalf_480p;
			break;
		}
		case 4:	//720p
		{
			SCREEN_LEFT = setting->screen_x_720p;
			SCREEN_TOP = setting->screen_y_720p;
			interlace = ITO_NON_INTERLACE;
			ffmode = ITO_FIELD;
			screenscan = setting->screen_scan_720p;
			font_half = setting->FontHalf_720p;
			font_vhalf = setting->FontVHalf_720p;
			break;
		}
		case 5:	//1080i
		{
			SCREEN_LEFT = setting->screen_x_1080i;
			SCREEN_TOP = setting->screen_y_1080i;
			interlace = ITO_INTERLACE;
			ffmode = setting->ffmode_1080i;
			screenscan = setting->screen_scan_1080i;
			font_half = setting->FontHalf_1080i;
			font_vhalf = setting->FontVHalf_1080i;
			break;
		}
		case 6:	//1080p(1080/30p)
		{
			SCREEN_LEFT = setting->screen_x_1080i;
			SCREEN_TOP = setting->screen_y_1080i >> 1;
			interlace = ITO_NON_INTERLACE;
			ffmode = ITO_FIELD;
			screenscan = setting->screen_scan_1080i;
			font_half = setting->FontHalf_1080i;
			font_vhalf = setting->FontVHalf_1080i;
			break;
		}
	}
}
//-------------------------------------------------
//スクリーンXY変更用
void SetScreenPosXY()
{
	switch(setting->tvmode)
	{
		case 0:	//AUTO
		case 1:
		case 2:
		{
			setting->screen_x_480i = SCREEN_LEFT;
			setting->screen_y_480i = SCREEN_TOP;
			setting->screen_scan_480i = screenscan;
			setting->FontHalf_480i = font_half;
			setting->FontVHalf_480i = font_vhalf;
			break;
		}
		case 3:	//480p
		{
			setting->screen_x_480p = SCREEN_LEFT;
			setting->screen_y_480p = SCREEN_TOP;
			setting->screen_scan_480p = screenscan;
			setting->FontHalf_480p = font_half;
			setting->FontVHalf_480p = font_vhalf;
			break;
		}
		case 4:	//720p
		{
			setting->screen_x_720p = SCREEN_LEFT;
			setting->screen_y_720p = SCREEN_TOP;
			setting->screen_scan_720p = screenscan;
			setting->FontHalf_720p = font_half;
			setting->FontVHalf_720p = font_vhalf;
			break;
		}
		case 5:	//1080i
		case 6:
		{
			setting->screen_x_1080i = SCREEN_LEFT;
			setting->screen_y_1080i = SCREEN_TOP;
			setting->screen_scan_1080i = screenscan;
			setting->FontHalf_1080i = font_half;
			setting->FontVHalf_1080i = font_vhalf;
			break;
		}
	}
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
	
	mcGetInfo(0, 0, NULL, NULL, NULL);
	mcSync(MC_WAIT, NULL, &ret);

	if( -1 == ret || 0 == ret) return 0;

	mcGetInfo(1, 0, NULL, NULL, NULL);
	mcSync(MC_WAIT, NULL, &ret);

	if( -1 == ret || 0 == ret ) return 1;

	return -11;
}

//-------------------------------------------------
// BUTTON SETTINGを初期化
void InitButtonSetting(void)
{
	int i;

	for(i=0; i<12; i++) setting->dirElf[i][0] = 0;

	strcpy(setting->dirElf[1], "MISC/FileBrowser");
	strcpy(setting->dirElf[12], "MISC/CONFIG");
}

//-------------------------------------------------
// COLOR SETTINGを初期化
void InitColorSetting(void)
{
	setting->color[COLOR_BACKGROUND] = DEF_COLOR1;
	setting->color[COLOR_FRAME] = DEF_COLOR2;
	setting->color[COLOR_HIGHLIGHTTEXT] = DEF_COLOR3;
	setting->color[COLOR_TEXT] = DEF_COLOR4;
	setting->color[COLOR_DIR] = DEF_COLOR5;
	setting->color[COLOR_FILE] = DEF_COLOR6;
	setting->color[COLOR_PS2SAVE] = DEF_COLOR7;
	setting->color[COLOR_ELF] = DEF_COLOR8;
	setting->color[COLOR_PS1SAVE] = DEF_COLOR9;
	setting->color[COLOR_GRAYTEXT] = DEF_COLOR10;
	setting->color[COLOR_PSU] = DEF_COLOR11;
	setting->flicker_alpha = DEF_FLICKER_ALPHA;
}

//-------------------------------------------------
// SCREEN SETTINGを初期化
void InitScreenSetting(void)
{
	setting->screen_x_480i = DEF_SCREEN_X_D1;
	setting->screen_y_480i = DEF_SCREEN_Y_D1;
	setting->screen_x_480p = DEF_SCREEN_X_D2;
	setting->screen_y_480p = DEF_SCREEN_Y_D2;
	setting->screen_x_720p = DEF_SCREEN_X_D4;
	setting->screen_y_720p = DEF_SCREEN_Y_D4;
	setting->screen_x_1080i = DEF_SCREEN_X_D3;
	setting->screen_y_1080i = DEF_SCREEN_Y_D3;
	setting->screen_scan_480i = DEF_SCREEN_SCAN;
	setting->screen_scan_480p = DEF_SCREEN_SCAN;
	setting->screen_scan_720p = DEF_SCREEN_SCAN;
	setting->screen_scan_1080i = DEF_SCREEN_SCAN;
	setting->FontHalf_480i = DEF_FONTHALF_480i;
	setting->FontHalf_480p = DEF_FONTHALF_480p;
	setting->FontHalf_720p = DEF_FONTHALF_720p;
	setting->FontHalf_1080i = DEF_FONTHALF_1080i;
	setting->flickerControl = DEF_FLICKERCONTROL;
	setting->tvmode = DEF_TVMODE;
	setting->interlace = DEF_INTERLACE;
	setting->ffmode_480i = DEF_FFMODE;
	setting->ffmode_1080i = DEF_FFMODE;
	setting->fullhd_width = DEF_FULLHD_WIDTH;
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
// MISC SETTINGを初期化
void InitMiscSetting(void)
{
	setting->timeout = DEF_TIMEOUT;
	setting->filename = DEF_FILENAME;
	setting->discControl = DEF_DISCCONTROL;
	setting->fileicon = DEF_FILEICON;
	setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
	setting->discELFCheck = DEF_DISCELFCHECK;
	setting->filePs2saveCheck = DEF_FILEPS2SAVECHECK;
	setting->fileELFCheck = DEF_FILEELFCHECK;
	setting->Exportdir[0] = 0;
	setting->language = DEF_LANGUAGE;
	setting->defaulttitle = DEF_DEFAULTTITLE;
	setting->defaultdetail = DEF_DEFAULTDETAIL;
	setting->usbd_flag = DEF_USBD_FLAG;
	strcpy(setting->usbd_path, "mc:/SYS-CONF/USBD.IRX");
	setting->usbmass_flag = DEF_USBMASS_FLAG;
	strcpy(setting->usbmass_path, "mc:/SYS-CONF/USB_MASS.IRX");
}

//-------------------------------------------------
// 設定を初期化
void InitSetting(void)
{
	InitButtonSetting();
	InitColorSetting();
	InitScreenSetting();
	InitFontSetting();
	InitMiscSetting();
}

//-------------------------------------------------
void saveConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	int ret;

	//cdから起動しているときは、設定ファイルを保存しない
	if(boot==CD_BOOT){
		mainMsg[0] = 0;
		return;
	}

	//cnfファイルのパス
	//LaunchELFが実行されたパスから設定ファイルを開く
	if(boot!=HOST_BOOT){
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
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
			sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
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
	sprintf(tmp, "%d", 2);
	if(cnf_setstr("cnf_version", tmp)<0) goto error;
	//Launcher
	strcpy(tmp, setting->dirElf[0]);
	if(cnf_setstr("DEFAULT", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[1]);
	if(cnf_setstr("CIRCLE", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[2]);
	if(cnf_setstr("CROSS", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[3]);
	if(cnf_setstr("SQUARE", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[4]);
	if(cnf_setstr("TRIANGLE", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[5]);
	if(cnf_setstr("L1", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[6]);
	if(cnf_setstr("R1", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[7]);
	if(cnf_setstr("L2", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[8]);
	if(cnf_setstr("R2", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[9]);
	if(cnf_setstr("L3", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[10]);
	if(cnf_setstr("R3", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[11]);
	if(cnf_setstr("START", tmp)<0) goto error;
	strcpy(tmp, setting->dirElf[12]);
	if(cnf_setstr("SELECT", tmp)<0) goto error;
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
	sprintf(tmp, "%d", setting->FontHalf_480i);
	if(cnf_setstr("font_half_480i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontHalf_480p);
	if(cnf_setstr("font_half_480p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontHalf_720p);
	if(cnf_setstr("font_half_720p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontHalf_1080i);
	if(cnf_setstr("font_half_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontVHalf_480i);
	if(cnf_setstr("font_vhalf_480i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontVHalf_480p);
	if(cnf_setstr("font_vhalf_480p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontVHalf_720p);
	if(cnf_setstr("font_vhalf_720p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->FontVHalf_1080i);
	if(cnf_setstr("font_vhalf_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->AsciiMarginTop);
	if(cnf_setstr("ascii_margin_top", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->AsciiMarginLeft);
	if(cnf_setstr("ascii_margin_left", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->KanjiMarginTop);
	if(cnf_setstr("kanji_margin_top", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->KanjiMarginLeft);
	if(cnf_setstr("kanji_margin_left", tmp)<0) goto error;
	//
	sprintf(tmp, "%d", setting->screen_x_480i);
	if(cnf_setstr("screen_pos_x_480i", tmp)<0) goto error;
	sprintf(tmp, "%d", (setting->screen_x_480i+2)>>2);
	if(cnf_setstr("screen_pos_x", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_y_480i);
	if(cnf_setstr("screen_pos_y", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_x_480p);
	if(cnf_setstr("screen_pos_x_480p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_y_480p);
	if(cnf_setstr("screen_pos_y_480p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_x_720p);
	if(cnf_setstr("screen_pos_x_720p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_y_720p);
	if(cnf_setstr("screen_pos_y_720p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_x_1080i);
	if(cnf_setstr("screen_pos_x_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_y_1080i);
	if(cnf_setstr("screen_pos_y_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_scan_480i);
	if(cnf_setstr("screen_scan_480i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_scan_480p);
	if(cnf_setstr("screen_scan_480p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_scan_720p);
	if(cnf_setstr("screen_scan_720p", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_scan_1080i);
	if(cnf_setstr("screen_scan_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->flickerControl);
	if(cnf_setstr("flicker_control", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->language);
	if(cnf_setstr("language", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->timeout);
	if(cnf_setstr("timeout", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->discControl);
	if(cnf_setstr("disc_control", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->filename);
	if(cnf_setstr("only_filename", tmp)<0) goto error;
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
	sprintf(tmp, "%d", setting->interlace);
	if(cnf_setstr("interlace", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->fullhd_width);
	if(cnf_setstr("fullhd_width", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->ffmode_480i);
	if(cnf_setstr("ffmode", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->ffmode_1080i);
	if(cnf_setstr("ffmode_1080i", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->defaulttitle);
	if(cnf_setstr("default_title", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->defaultdetail);
	if(cnf_setstr("default_detail", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbd_flag);
	if(cnf_setstr("usbd_use_ext", tmp)<0) goto error;
	strcpy(tmp, setting->usbd_path);
	if(cnf_setstr("usbd_path", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->usbmass_flag);
	if(cnf_setstr("usbmass_use_ext", tmp)<0) goto error;
	strcpy(tmp, setting->usbmass_path);
	if(cnf_setstr("usbmass_path", tmp)<0) goto error;
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
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
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
			strcat(path, "/LBF.CNF");
		}
		else{
			//SYS-CONFがなかったらLaunchELFのディレクトリにセーブ
			sprintf(path, "%sLBF.CNF", LaunchElfDir);
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
	int cnf_version=0;
	int ret=0;
	int nchk=-1;
//	int i;

	setting = (SETTING*)malloc(sizeof(SETTING));

	//cnfファイルのパス
	//LaunchELFが実行されたパスから設定ファイルを開く
	if(boot!=HOST_BOOT){
		sprintf(path, "%sLBF.CNF", LaunchElfDir);
		if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
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
			sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
			fd = fioOpen(path, O_RDONLY);
			if(fd >= 0)
				fioClose(fd);
			else
				path[0]=0;
		}
	}

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
		if(cnf_version!=2){
			//Setting初期化
			InitSetting();
			//ファイルサイズを0にする
			fd = fioOpen(path, O_WRONLY | O_TRUNC | O_CREAT);
			fioClose(fd);
			ret=2;
		}
		else{
			//Launcher
			if(cnf_getstr("DEFAULT", tmp, "")>=0)
				strcpy(setting->dirElf[0], tmp);
			if(cnf_getstr("CIRCLE", tmp, "")>=0)
				strcpy(setting->dirElf[1], tmp);
			if(cnf_getstr("CROSS", tmp, "")>=0)
				strcpy(setting->dirElf[2], tmp);
			if(cnf_getstr("SQUARE", tmp, "")>=0)
				strcpy(setting->dirElf[3], tmp);
			if(cnf_getstr("TRIANGLE", tmp, "")>=0)
				strcpy(setting->dirElf[4], tmp);
			if(cnf_getstr("L1", tmp, "")>=0)
				strcpy(setting->dirElf[5], tmp);
			if(cnf_getstr("R1", tmp, "")>=0)
				strcpy(setting->dirElf[6], tmp);
			if(cnf_getstr("L2", tmp, "")>=0)
				strcpy(setting->dirElf[7], tmp);
			if(cnf_getstr("R2", tmp, "")>=0)
				strcpy(setting->dirElf[8], tmp);
			if(cnf_getstr("L3", tmp, "")>=0)
				strcpy(setting->dirElf[9], tmp);
			if(cnf_getstr("R3", tmp, "")>=0)
				strcpy(setting->dirElf[10], tmp);
			if(cnf_getstr("START", tmp, "")>=0)
				strcpy(setting->dirElf[11], tmp);
			if(cnf_getstr("SELECT", tmp, "")>=0)
				strcpy(setting->dirElf[12], tmp);
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
			if(cnf_getstr("flicker_alpha", tmp, "")>=0){
				setting->flicker_alpha = atoi(tmp);
				if(setting->flicker_alpha<0 || setting->flicker_alpha>255)
					setting->flicker_alpha = DEF_FLICKER_ALPHA;
			}
			//font
			if(cnf_getstr("ascii_font", tmp, "")>=0)
				strcpy(setting->AsciiFont, tmp);
			if(cnf_getstr("kanji_font", tmp, "")>=0)
				strcpy(setting->KanjiFont, tmp);
			if(cnf_getstr("char_margin", tmp, "")>=0)
				setting->CharMargin = atoi(tmp);
			if(cnf_getstr("line_margin", tmp, "")>=0)
				setting->LineMargin = atoi(tmp);
			if(cnf_getstr("font_bold", tmp, "")>=0){
				setting->FontBold = atoi(tmp);
				if(setting->FontBold<0 || setting->FontBold>1)
					setting->FontBold = DEF_FONTBOLD;
			}
			if(cnf_getstr("font_half_480i", tmp, "")>=0){
				setting->FontHalf_480i = atoi(tmp);
				if(setting->FontHalf_480i<-7 || setting->FontHalf_480i>7)
					setting->FontHalf_480i = DEF_FONTHALF_480i;
			}
			if(cnf_getstr("font_half_480p", tmp, "")>=0){
				setting->FontHalf_480p = atoi(tmp);
				if(setting->FontHalf_480p<-7 || setting->FontHalf_480p>7)
					setting->FontHalf_480p = DEF_FONTHALF_480p;
			}
			if(cnf_getstr("font_half_720p", tmp, "")>=0){
				setting->FontHalf_720p = atoi(tmp);
				if(setting->FontHalf_720p<-7 || setting->FontHalf_720p>7)
					setting->FontHalf_720p = DEF_FONTHALF_720p;
			}
			if(cnf_getstr("font_half_1080i", tmp, "")>=0){
				setting->FontHalf_1080i = atoi(tmp);
				if(setting->FontHalf_1080i<-7 || setting->FontHalf_1080i>7)
					setting->FontHalf_1080i = DEF_FONTHALF_1080i;
			}
			if(cnf_getstr("font_vhalf_480i", tmp, "")>=0){
				setting->FontVHalf_480i = atoi(tmp);
				if(setting->FontVHalf_480i<-7 || setting->FontVHalf_480i>7)
					setting->FontVHalf_480i = DEF_FONTVHALF_480i;
			}
			if(cnf_getstr("font_vhalf_480p", tmp, "")>=0){
				setting->FontVHalf_480p = atoi(tmp);
				if(setting->FontVHalf_480p<-7 || setting->FontVHalf_480p>7)
					setting->FontVHalf_480p = DEF_FONTVHALF_480p;
			}
			if(cnf_getstr("font_vhalf_720p", tmp, "")>=0){
				setting->FontVHalf_720p = atoi(tmp);
				if(setting->FontVHalf_720p<-7 || setting->FontVHalf_720p>7)
					setting->FontVHalf_720p = DEF_FONTVHALF_720p;
			}
			if(cnf_getstr("font_vhalf_1080i", tmp, "")>=0){
				setting->FontVHalf_1080i = atoi(tmp);
				if(setting->FontVHalf_1080i<-7 || setting->FontVHalf_1080i>7)
					setting->FontVHalf_1080i = DEF_FONTVHALF_1080i;
			}
			if(cnf_getstr("ascii_margin_top", tmp, "")>=0)
				setting->AsciiMarginTop = atoi(tmp);
			if(cnf_getstr("ascii_margin_left", tmp, "")>=0)
				setting->AsciiMarginLeft = atoi(tmp);
			if(cnf_getstr("kanji_margin_top", tmp, "")>=0)
				setting->KanjiMarginTop = atoi(tmp);
			if(cnf_getstr("kanji_margin_left", tmp, "")>=0)
				setting->KanjiMarginLeft = atoi(tmp);
			//
			if(cnf_getstr("screen_pos_x", tmp, "")>=0)
				setting->screen_x_480i = atoi(tmp)<<2;
			if(cnf_getstr("screen_pos_x_480i", tmp, "")>=0)
				nchk = atoi(tmp);
			if(cnf_getstr("screen_pos_y", tmp, "")>=0)
				setting->screen_y_480i = atoi(tmp);
			if(cnf_getstr("screen_pos_x_480p", tmp, "")>=0)
				setting->screen_x_480p = atoi(tmp);
			if(cnf_getstr("screen_pos_y_480p", tmp, "")>=0)
				setting->screen_y_480p = atoi(tmp);
			if(cnf_getstr("screen_pos_x_720p", tmp, "")>=0)
				setting->screen_x_720p = atoi(tmp);
			if(cnf_getstr("screen_pos_y_720p", tmp, "")>=0)
				setting->screen_y_720p = atoi(tmp);
			if(cnf_getstr("screen_pos_x_1080i", tmp, "")>=0)
				setting->screen_x_1080i = atoi(tmp);
			if(cnf_getstr("screen_pos_y_1080i", tmp, "")>=0)
				setting->screen_y_1080i = atoi(tmp);
			if(cnf_getstr("screen_scan_480i", tmp, "")>=0)
				setting->screen_scan_480i = atoi(tmp);
			if(cnf_getstr("screen_scan_480p", tmp, "")>=0)
				setting->screen_scan_480p = atoi(tmp);
			if(cnf_getstr("screen_scan_720p", tmp, "")>=0)
				setting->screen_scan_720p = atoi(tmp);
			if(cnf_getstr("screen_scan_1080i", tmp, "")>=0)
				setting->screen_scan_1080i = atoi(tmp);
			if(cnf_getstr("flicker_control", tmp, "")>=0){
				setting->flickerControl = atoi(tmp);
				if(setting->flickerControl<0 || setting->flickerControl>1)
					setting->flickerControl = DEF_FLICKERCONTROL;
			}
			if(cnf_getstr("language", tmp, "")>=0){
				setting->language = atoi(tmp);
				if(setting->language<0 || setting->flickerControl>=NUM_LANG)
					setting->language = DEF_LANGUAGE;
			}
			if(cnf_getstr("timeout", tmp, "")>=0){
				setting->timeout = atoi(tmp);
				if(setting->timeout<0)
					setting->timeout = DEF_TIMEOUT;
			}
			if(cnf_getstr("disc_control", tmp, "")>=0){
				setting->discControl = atoi(tmp);
				if(setting->discControl<0 || setting->discControl>1)
					setting->discControl = DEF_DISCCONTROL;
			}
			if(cnf_getstr("only_filename", tmp, "")>=0){
				setting->filename = atoi(tmp);
				if(setting->filename<0 || setting->filename>1)
					setting->filename = DEF_FILENAME;
			}
			if(cnf_getstr("file_icon", tmp, "")>=0){
				setting->fileicon = atoi(tmp);
				if(setting->fileicon<0 || setting->fileicon>1)
					setting->fileicon = DEF_FILEICON;
			}
			if(cnf_getstr("ps2save_check", tmp, "")>=0){
				setting->discPs2saveCheck = atoi(tmp);
				if(setting->discPs2saveCheck<0 || setting->discPs2saveCheck>1)
					setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
			}
			if(cnf_getstr("elf_check", tmp, "")>=0){
				setting->discELFCheck = atoi(tmp);
				if(setting->discELFCheck<0 || setting->discELFCheck>1)
					setting->discELFCheck = DEF_DISCELFCHECK;
			}
			if(cnf_getstr("file_ps2save_check", tmp, "")>=0){
				setting->filePs2saveCheck = atoi(tmp);
				if(setting->filePs2saveCheck<0 || setting->filePs2saveCheck>1)
					setting->filePs2saveCheck = DEF_FILEPS2SAVECHECK;
			}
			if(cnf_getstr("file_elf_check", tmp, "")>=0){
				setting->fileELFCheck = atoi(tmp);
				if(setting->fileELFCheck<0 || setting->fileELFCheck>1)
					setting->fileELFCheck = DEF_FILEELFCHECK;
			}
			if(cnf_getstr("export_dir", tmp, "")>=0)
				strcpy(setting->Exportdir, tmp);
			if(cnf_getstr("tvmode", tmp, "")>=0){
				setting->tvmode = atoi(tmp);
				if(setting->tvmode<0 || setting->tvmode>5)
					setting->tvmode = DEF_TVMODE;
			}
			if(cnf_getstr("interlace", tmp, "")>=0){
				setting->interlace = atoi(tmp);
				if(setting->interlace<0 || setting->interlace>1)
					setting->interlace = DEF_INTERLACE;
			}
			if(cnf_getstr("ffmode", tmp, "")>=0){
				setting->ffmode_480i = atoi(tmp);
				if(setting->ffmode_480i<0 || setting->ffmode_480i>1)
					setting->ffmode_480i = DEF_FFMODE;
			}
			if(cnf_getstr("ffmode_1080i", tmp, "")>=0){
				setting->ffmode_1080i = atoi(tmp);
				if(setting->ffmode_1080i<0 || setting->ffmode_1080i>1)
					setting->ffmode_1080i = DEF_FFMODE;
			}
			if(cnf_getstr("fullhd_width", tmp, "")>=0){
				setting->fullhd_width = atoi(tmp);
				if(setting->fullhd_width<320 || setting->fullhd_width>1920)
					setting->fullhd_width = DEF_FULLHD_WIDTH;
			}
			if(cnf_getstr("default_title", tmp, "")>=0){
				setting->defaulttitle = atoi(tmp);
				if(setting->defaulttitle<0 || setting->defaulttitle>1)
					setting->defaulttitle = DEF_DEFAULTTITLE;
			}
			if(cnf_getstr("default_detail", tmp, "")>=0){
				setting->defaultdetail = atoi(tmp);
				if(setting->defaultdetail<0 || setting->defaultdetail>2)
					setting->defaultdetail = DEF_DEFAULTDETAIL;
			}
			if(cnf_getstr("usbd_use_ext", tmp, "")>=0){
				setting->usbd_flag = atoi(tmp);
				if(setting->usbd_flag<0 || setting->usbd_flag>1)
					setting->usbd_flag = DEF_USBD_FLAG;
			}
			if(cnf_getstr("usbd_path", tmp, "")>=0)
				strcpy(setting->usbd_path, tmp);
			if(cnf_getstr("usbmass_use_ext", tmp, "")>=0){
				setting->usbmass_flag = atoi(tmp);
				if(setting->usbmass_flag<0 || setting->usbmass_flag>1)
					setting->usbmass_flag = DEF_USBMASS_FLAG;
			}
			if(cnf_getstr("usbmass_path", tmp, "")>=0)
				strcpy(setting->usbmass_path, tmp);
			
		}
	}

	SetLanguage(setting->language);

	if (nchk>=0) {
		setting->screen_x_480i = nchk;
	} else if (setting->screen_x_480i >= 1600) {
		setting->screen_x_480i = setting->screen_x_480i >> 2;
	}
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
//ランチャー設定
void config_button(SETTING *setting)
{
	char c[MAX_PATH];
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
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
				if(sel==0)
					break;
				if(sel>=DEFAULT && sel<=LAUNCHER12){
					getFilePath(setting->dirElf[sel-1], ELF_FILE);
					if(!strncmp(setting->dirElf[sel-1], "mc", 2)){
						sprintf(c, "mc%s", &setting->dirElf[sel-1][3]);
						strcpy(setting->dirElf[sel-1], c);
					}
				}
				else if(sel==BUTTONINIT){
					InitButtonSetting();
					//sprintf(msg0, "%s", "Initialize Button Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel>=DEFAULT && sel<=LAUNCHER12)
					setting->dirElf[sel-1][0]=0;
			}
		}
		for(i=0;i<=BUTTONINIT;i++){
			if(i==0){
				strcpy(config[i], "..");
			}
			else if(i==DEFAULT){
				sprintf(config[i], "DEFAULT: %s", setting->dirElf[0]);
			}
			else if(i==LAUNCHER1){
				sprintf(config[i], "○     : %s", setting->dirElf[1]);
			}
			else if(i==LAUNCHER2){
				sprintf(config[i], "×     : %s", setting->dirElf[2]);
			}
			else if(i==LAUNCHER3){
				sprintf(config[i], "□     : %s", setting->dirElf[3]);
			}
			else if(i==LAUNCHER4){
				sprintf(config[i], "△     : %s", setting->dirElf[4]);
			}
			else if(i==LAUNCHER5){
				sprintf(config[i], "L1     : %s", setting->dirElf[5]);
			}
			else if(i==LAUNCHER6){
				sprintf(config[i], "R1     : %s", setting->dirElf[6]);
			}
			else if(i==LAUNCHER7){
				sprintf(config[i], "L2     : %s", setting->dirElf[7]);
			}
			else if(i==LAUNCHER8){
				sprintf(config[i], "R2     : %s", setting->dirElf[8]);
			}
			else if(i==LAUNCHER9){
				sprintf(config[i], "L3     : %s", setting->dirElf[9]);
			}
			else if(i==LAUNCHER10){
				sprintf(config[i], "R3     : %s", setting->dirElf[10]);
			}
			else if(i==LAUNCHER11){
				sprintf(config[i], "START  : %s", setting->dirElf[11]);
			}
			else if(i==LAUNCHER12){
				sprintf(config[i], "SELECT : %s", setting->dirElf[12]);
			}
			else if(i==BUTTONINIT){
				strcpy(config[i], lang->conf_buttonsettinginit);
			}
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
		else if (sel>=DEFAULT && sel<=LAUNCHER12)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_edit, lang->conf_clear, lang->conf_up);
		else if(sel==BUTTONINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}

	for(i=0;i<=12;i++){
		if(setting->dirElf[i][0]) return;
	}
	//ランチャー設定が何もないときSELECTにCONFIGをセット
	strcpy(setting->dirElf[12], "MISC/CONFIG");
	return;
}

//-------------------------------------------------
//配色設定
void config_color(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0 ,sel_x=0;
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
			if(new_pad) pushed=TRUE;
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT){
				if(sel>=COLOR1 && sel<=COLOR11){
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
				if(sel>=COLOR1 && sel<=COLOR11){
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
				if(sel>=COLOR1 && sel<=COLOR11){
					switch(sel){
						case COLOR1: colorid=COLOR_BACKGROUND; break;
						case COLOR2: colorid=COLOR_FRAME; break;
						case COLOR3: colorid=COLOR_TEXT; break;
						case COLOR4: colorid=COLOR_HIGHLIGHTTEXT; break;
						case COLOR5: colorid=COLOR_GRAYTEXT; break;
						case COLOR6: colorid=COLOR_DIR; break;
						case COLOR7: colorid=COLOR_FILE; break;
						case COLOR8: colorid=COLOR_PS2SAVE; break;
						case COLOR9: colorid=COLOR_PS1SAVE; break;
						case COLOR10: colorid=COLOR_ELF; break;
						case COLOR11: colorid=COLOR_PSU; break;
					}
					r = setting->color[colorid] & 0xFF;
					g = setting->color[colorid] >> 8 & 0xFF;
					b = setting->color[colorid] >> 16 & 0xFF;
					if(sel_x==0 && r<255) r++;
					if(sel_x==1 && g<255) g++;
					if(sel_x==2 && b<255) b++;
					setting->color[colorid] = ITO_RGBA(r, g, b, 0);
				} else if (sel == FLICKER_ALPHA) {
					if (paddata & PAD_SQUARE)
						setting->flicker_alpha+=16;
					else
						setting->flicker_alpha++;
					if (setting->flicker_alpha>128)
						setting->flicker_alpha=128;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel>=COLOR1 && sel<=COLOR11){
					switch(sel){
						case COLOR1: colorid=COLOR_BACKGROUND; break;
						case COLOR2: colorid=COLOR_FRAME; break;
						case COLOR3: colorid=COLOR_TEXT; break;
						case COLOR4: colorid=COLOR_HIGHLIGHTTEXT; break;
						case COLOR5: colorid=COLOR_GRAYTEXT; break;
						case COLOR6: colorid=COLOR_DIR; break;
						case COLOR7: colorid=COLOR_FILE; break;
						case COLOR8: colorid=COLOR_PS2SAVE; break;
						case COLOR9: colorid=COLOR_PS1SAVE; break;
						case COLOR10: colorid=COLOR_ELF; break;
						case COLOR11: colorid=COLOR_PSU; break;
					}
					r = setting->color[colorid] & 0xFF;
					g = setting->color[colorid] >> 8 & 0xFF;
					b = setting->color[colorid] >> 16 & 0xFF;
					if(sel_x==0 && r>0) r--;
					if(sel_x==1 && g>0) g--;
					if(sel_x==2 && b>0) b--;
					setting->color[colorid] = ITO_RGBA(r, g, b, 0);
				} else if (sel == FLICKER_ALPHA) {
					if (paddata & PAD_SQUARE)
						setting->flicker_alpha-=16;
					else
						setting->flicker_alpha--;
					if (setting->flicker_alpha<0)
						setting->flicker_alpha=0;
				}
			}
			else if((new_pad & PAD_L3)||((new_pad & PAD_CIRCLE)&&(sel=PRESETCOLOR))){
				static int preset=0;
				//デフォルト
				if(preset==0){
					setting->color[COLOR_BACKGROUND] = DEF_COLOR1;
					setting->color[COLOR_FRAME] = DEF_COLOR2;
					setting->color[COLOR_HIGHLIGHTTEXT] = DEF_COLOR3;
					setting->color[COLOR_TEXT] = DEF_COLOR4;
					setting->color[COLOR_DIR] = DEF_COLOR5;
					setting->color[COLOR_FILE] = DEF_COLOR6;
					setting->color[COLOR_PS2SAVE] = DEF_COLOR7;
					setting->color[COLOR_ELF] = DEF_COLOR8;
					setting->color[COLOR_PS1SAVE] = DEF_COLOR9;
					setting->color[COLOR_GRAYTEXT] = DEF_COLOR10;
					setting->color[COLOR_PSU] = DEF_COLOR11;
				}
				//Unofficial LaunchELF
				if(preset==1){
					setting->color[COLOR_BACKGROUND] = ITO_RGBA(128,128,128,0);
					setting->color[COLOR_FRAME] = ITO_RGBA(64,64,64,0);
					setting->color[COLOR_HIGHLIGHTTEXT] = ITO_RGBA(96,0,0,0);
					setting->color[COLOR_TEXT] = ITO_RGBA(0,0,0,0);
					setting->color[COLOR_DIR] = ITO_RGBA(160,160,0,0);
					setting->color[COLOR_FILE] = ITO_RGBA(80,80,80,0);
					setting->color[COLOR_PS2SAVE] = DEF_COLOR7;
					setting->color[COLOR_ELF] = DEF_COLOR8;
					setting->color[COLOR_PS1SAVE] = ITO_RGBA(0,96,192,0);
					setting->color[COLOR_GRAYTEXT] = ITO_RGBA(64,64,64,0);
					setting->color[COLOR_PSU] = DEF_COLOR11;
				}
				//黒い背景
				if(preset==2){
					setting->color[COLOR_BACKGROUND] = ITO_RGBA(24,24,24,0);
					setting->color[COLOR_FRAME] = ITO_RGBA(64,64,64,0);
					setting->color[COLOR_HIGHLIGHTTEXT] = ITO_RGBA(255,128,0,0);
					setting->color[COLOR_TEXT] = ITO_RGBA(144,144,144,0);
					setting->color[COLOR_DIR] = DEF_COLOR5;
					setting->color[COLOR_FILE] = DEF_COLOR6;
					setting->color[COLOR_PS2SAVE] = DEF_COLOR7;
					setting->color[COLOR_ELF] = DEF_COLOR8;
					setting->color[COLOR_PS1SAVE] = DEF_COLOR9;
					setting->color[COLOR_GRAYTEXT] = ITO_RGBA(64,64,64,0);
					setting->color[COLOR_PSU] = DEF_COLOR11;
				}
				preset++;
				if(preset>2) preset=0;
			}
		}

		//
		for(i=0;i<=PRESETCOLOR;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i>=COLOR1 && i<=COLOR11){	//COLOR
				switch(i){
					case COLOR1: colorid=COLOR_BACKGROUND; break;
					case COLOR2: colorid=COLOR_FRAME; break;
					case COLOR3: colorid=COLOR_TEXT; break;
					case COLOR4: colorid=COLOR_HIGHLIGHTTEXT; break;
					case COLOR5: colorid=COLOR_GRAYTEXT; break;
					case COLOR6: colorid=COLOR_DIR; break;
					case COLOR7: colorid=COLOR_FILE; break;
					case COLOR8: colorid=COLOR_PS2SAVE; break;
					case COLOR9: colorid=COLOR_PS1SAVE; break;
					case COLOR10: colorid=COLOR_ELF; break;
					case COLOR11: colorid=COLOR_PSU; break;
				}
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
			}
			else if(i==FLICKER_ALPHA)
				sprintf(config[i], "%s: %02X", lang->conf_flicker_alpha, setting->flicker_alpha);
			else if(i==PRESETCOLOR)	//INIT
				strcpy(config[i], lang->conf_presetcolor);
		}
		nList=14;

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
				if(sel>=COLOR1 && sel<=COLOR11)
					printXY(">", FONT_WIDTH*21 + FONT_WIDTH*sel_x*7, y, color, TRUE);
				else
					printXY(">", x, y, color, TRUE);
			}
			//リスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			//色のプレビュー
			if(top+i>=COLOR1 && top+i<=COLOR11){
				switch(top+i){
					case COLOR1: colorid=COLOR_BACKGROUND; break;
					case COLOR2: colorid=COLOR_FRAME; break;
					case COLOR3: colorid=COLOR_TEXT; break;
					case COLOR4: colorid=COLOR_HIGHLIGHTTEXT; break;
					case COLOR5: colorid=COLOR_GRAYTEXT; break;
					case COLOR6: colorid=COLOR_DIR; break;
					case COLOR7: colorid=COLOR_FILE; break;
					case COLOR8: colorid=COLOR_PS2SAVE; break;
					case COLOR9: colorid=COLOR_PS1SAVE; break;
					case COLOR10: colorid=COLOR_ELF; break;
					case COLOR11: colorid=COLOR_PSU; break;
				}
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
		else if(sel>=COLOR1 && sel<=COLOR11)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==FLICKER_ALPHA)
			sprintf(msg1, "○:%s ×:%s +□:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_fast, lang->conf_up);
		else if(sel==PRESETCOLOR)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//画面設定
void config_screen(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;// ,sel_x=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	int font_h;

	font_h = FONT_HEIGHT - GetFontMargin(LINE_MARGIN);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
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
					setting->tvmode++;
					if(setting->tvmode>5)
						setting->tvmode = 0;
					//
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
				}
				else if(sel==INTERLACE){	//インターレース
					if(setting->tvmode<3){
						if(setting->interlace)
							SCREEN_TOP = SCREEN_TOP>>1;
						else
							SCREEN_TOP = SCREEN_TOP<<1;
						setting->interlace = !setting->interlace;
						interlace = !interlace;
						SetScreenPosXY();
						//
						itoGsReset();
						setupito(setting->tvmode);
						SetHeight();
					}
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
				else if(sel==FFMODE){	//ffmode
					if((setting->tvmode<3)||(setting->tvmode==5)){
						if(setting->tvmode<3)
							setting->ffmode_480i = !setting->ffmode_480i;
						else
							setting->ffmode_1080i = !setting->ffmode_1080i;
						ffmode = !ffmode;
						itoGsReset();
						setupito(setting->tvmode);
						SetHeight();
					}
				}
				else if(sel==SCREEN_X){	//SCREEN X
					SCREEN_LEFT++;
					screen_env.screen.x = SCREEN_LEFT;
					SetScreenPosXY();
					itoSetScreenPos(SCREEN_LEFT, SCREEN_TOP);
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					SCREEN_TOP++;
					screen_env.screen.y = SCREEN_TOP;
					SetScreenPosXY();
					itoSetScreenPos(SCREEN_LEFT, SCREEN_TOP);
				}
				else if(sel==SCREENSIZE){	//画面サイズ
					screenscan = !screenscan;
					SetScreenPosXY();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
				}
				else if(sel==FLICKERCONTROL)	//フリッカーコントロール
					setting->flickerControl = !setting->flickerControl;
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
				if(sel==TVMODE){ // TVMODE
					switch(setting->fullhd_width)
					{
						case 1920: {setting->fullhd_width = 960; break;}
						case  960: {setting->fullhd_width = 640; break;}
						case  640: {setting->fullhd_width = 480; break;}
						case  480: {setting->fullhd_width = 384; break;}
						case  384: {setting->fullhd_width = 320; break;}
						default: {setting->fullhd_width = 1920; break;}
					}
					if ((setting->fullhd_width == 1920) && ((setting->tvmode == 5) || (setting->tvmode == 6) || (setting->tvmode == 9) || (setting->tvmode == 10))) {
						setting->ffmode_1080i = ITO_FRAME;
						ffmode = ITO_FRAME;
					}
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
				}
				else if(sel==FONTHALF){	// fonthalf
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
					if(SCREEN_LEFT > 0){
						SCREEN_LEFT--;
						screen_env.screen.x = SCREEN_LEFT;
						SetScreenPosXY();
						itoSetScreenPos(SCREEN_LEFT, SCREEN_TOP);
					}
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					if(SCREEN_TOP > 0){
						SCREEN_TOP--;
						screen_env.screen.y = SCREEN_TOP;
						SetScreenPosXY();
						itoSetScreenPos(SCREEN_LEFT, SCREEN_TOP);
					}
				}
			}
			else if(new_pad & PAD_SQUARE){
				if(sel==TVMODE){	//TVMODE
					//tvmode変更
					setting->tvmode++;
					if(setting->tvmode==7)
						setting->tvmode = 0;
					//
					SetScreenPosVM();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
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
			}
		}

		//
		for(i=0;i<=SCREENINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==TVMODE){	//TVMODE
				if(setting->tvmode==0)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "AUTO");
				else if(setting->tvmode==1)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "NTSC");
				else if(setting->tvmode==2)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "PAL");
				else if(setting->tvmode==3)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "480p");
				else if(setting->tvmode==4)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "720p");
				else if(setting->tvmode==5)
					sprintf(config[i],"%s: %dx%s", lang->conf_tvmode, setting->fullhd_width, "1080i");
				else if(setting->tvmode==6)
					sprintf(config[i],"%s: %dx%s", lang->conf_tvmode, setting->fullhd_width, "1080p 29.97Hz");
				else if(setting->tvmode==11)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "CFG1");
				else if(setting->tvmode==12)
					sprintf(config[i],"%s: %s", lang->conf_tvmode, "CFG2");
				
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
			else if(i==INTERLACE){	//INTERLACE
				sprintf(config[i], "%s: ", lang->conf_interlace);
				if(interlace)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FFMODE){	//FFMODE
				sprintf(config[i],"%s: ", lang->conf_ffmode);
				if(ffmode)
					strcat(config[i], lang->conf_ffmode_frame);
				else
					strcat(config[i], lang->conf_ffmode_field);
			}
			else if(i==SCREEN_X){	//SCREEN X
				sprintf(config[i],"%s: %3d", lang->conf_screen_x, SCREEN_LEFT);
			}
			else if(i==SCREEN_Y){	//SCREEN Y
				sprintf(config[i],"%s: %3d", lang->conf_screen_y, SCREEN_TOP);
			}
			else if(i==SCREENSIZE){	//SCREEN SIZE
				sprintf(config[i],"%s: ", lang->conf_screen_scan);
				if(screenscan)
					strcat(config[i], lang->conf_screen_scan_full);
				else
					strcat(config[i], lang->conf_screen_scan_crop);
			}
			else if(i==FLICKERCONTROL){	//FLICKER CONTROL
				sprintf(config[i],"%s: ", lang->conf_flickercontrol);
				if(setting->flickerControl)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==SCREENINIT){	//INIT
				strcpy(config[i], lang->conf_screensettinginit);
			}
		}
		nList=11;

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
			if ((setting->tvmode == 5) || (setting->tvmode == 6) || (setting->tvmode == 9) || (setting->tvmode == 10))
				sprintf(msg1, "○:%s ×:%s%s △:%s", lang->conf_change, lang->conf_horizontalresolution, lang->conf_change, lang->conf_up);
			else
				sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==INTERLACE)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==FONTHALF)
			sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_off, lang->conf_up);
		else if(sel==FONTVHALF)
			sprintf(msg1, "○:%s ×:%s □:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_off, lang->conf_up);
		else if(sel==FFMODE)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==SCREENSIZE)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==FLICKERCONTROL)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==SCREENINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//IP設定
void config_network(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	int fd;
	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmp[16*3];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
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
				else if(sel==NETWORKSAVE){
					//
					sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s", lang->conf_ipsavefailed);
					//メモリーカードの種類を取得
					if(GetMcType(0, 0)==MC_TYPE_PS2){
						//save
						sprintf(tmp, "%s %s %s", ip, netmask, gw);
						//フォルダ作成
						newdir("mc0:/", "SYS-CONF");
						// 書き込み
						fd = fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT", O_CREAT|O_WRONLY|O_TRUNC);
						if(fd >= 0){
							fioWrite(fd, tmp, strlen(tmp));
							fioClose(fd);
							sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s", lang->conf_ipsaved);	//成功
						}
					}
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();
					MessageBox(tmp, LBF_VER, MB_OK);
				}
				else if(sel==NETWORKINIT){
					//init
					strcpy(ip, "192.168.0.10");
					strcpy(netmask, "255.255.255.0");
					strcpy(gw, "192.168.0.1");
					//sprintf(msg0, "%s", "Initialize Network Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
			}
		}

		//
		for(i=0;i<=NETWORKINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i==IPADDRESS){	//IPADDRESS
				sprintf(config[i], "%s: %s", lang->conf_ipaddress, ip);
			}
			else if(i==NETMASK){	//NETMASK
				sprintf(config[i], "%s: %s", lang->conf_netmask, netmask);
			}
			else if(i==GATEWAY){	//GATEWAY
				sprintf(config[i], "%s: %s", lang->conf_gateway, gw);
			}
			else if(i==NETWORKSAVE){	//NETWORKSAVE
				strcpy(config[i],lang->conf_ipoverwrite);
			}
			else if(i==NETWORKINIT){	//NETWORKINIT
				strcpy(config[i],lang->conf_ipsettinginit);
			}
		}
		nList=6;

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
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_network);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		else if(sel>=IPADDRESS && sel<=GATEWAY)
			sprintf(msg1, "○:%s △:%s", lang->conf_edit, lang->conf_up);
		else if(sel==NETWORKINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
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
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	char newFontName[MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
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
					SetFontBold(setting->FontBold);
					SetFontHalf(font_half);
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
		}

		//
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
		nList=11;

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
		else if(sel==CHARMARGIN)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==LINEMARGIN)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==FONTBOLD)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==ASCIIMARGINTOP)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==ASCIIMARGINLEFT)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==KANJIMARGINTOP)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==KANJIMARGINLEFT)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==FONTINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//その他設定
void config_misc(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) pushed=TRUE;
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
				else if(sel==TIMEOUT)
					setting->timeout++;
				else if(sel==DISCCONTROL)
					setting->discControl = !setting->discControl;
				else if(sel==FILENAME)
					setting->filename = !setting->filename;
				else if(sel==FILEICON)
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
					if(setting->defaultdetail>2) setting->defaultdetail = 0;
				}
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
				else if(sel==MISCINIT){
					//init
					InitMiscSetting();
					SetLanguage(setting->language);
					//sprintf(msg0, "%s", "Initialize Misc Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_SQUARE){	//□
				if((sel==USBD_PATH)&&(!strncmp(setting->usbd_path, "mc", 2))&&(setting->usbd_path[3] == ':'))
					strcpy(setting->usbd_path+2, setting->usbd_path+3);
				if((sel==USBMASS_PATH)&&(!strncmp(setting->usbmass_path, "mc", 2))&&(setting->usbmass_path[3] == ':'))
					strcpy(setting->usbmass_path+2, setting->usbmass_path+3);
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==TIMEOUT)
					setting->timeout--;
				if(sel==EXPORTDIR)
					setting->Exportdir[0]='\0';
				if(sel==USBD_PATH)
					setting->usbd_path[0]='\0';
				if(sel==USBMASS_PATH)
					setting->usbmass_path[0]='\0';
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
			else if(i==TIMEOUT){	//TIMEOUT
				sprintf(config[i], "%s: %d", lang->conf_timeout, setting->timeout);
			}
			else if(i==DISCCONTROL){	//DISC CONTROL
				sprintf(config[i], "%s: " ,lang->conf_disc_control);
				if(setting->discControl)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILENAME){	//PRINT ONLY FILENAME
				sprintf(config[i], "%s: " ,lang->conf_print_only_filename);
				if(setting->filename)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILEICON){	//FILEICON
				sprintf(config[i], "%s: " ,lang->conf_fileicon);
				if(setting->fileicon)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==PS2SAVECHECK){	//DISC PS2SAVE CHECK
				sprintf(config[i], "%s: " ,lang->conf_disc_ps2save_check);
				if(setting->discPs2saveCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==ELFCHECK){	//DISC ELF CHECK
				sprintf(config[i], "%s: " ,lang->conf_disc_elf_check);
				if(setting->discELFCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILEPS2SAVECHECK){	//FILE PS2SAVE CHECK
				sprintf(config[i], "%s: " ,lang->conf_file_ps2save_check);
				if(setting->filePs2saveCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FILEELFCHECK){	// FILE ELF CHECK
				sprintf(config[i], "%s: ", lang->conf_file_elf_check);
				if(setting->fileELFCheck)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==EXPORTDIR){	//EXPORT DIR
				sprintf(config[i], "%s: %s", lang->conf_export_dir, setting->Exportdir);
			}
			else if(i==DEFAULTTITLE){	//DEFAULTTITLE
				sprintf(config[i], "%s: ", lang->conf_defaulttitle);
				if(setting->defaulttitle)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==DEFAULTDETAIL){	//DEFAULTDETAIL
				sprintf(config[i], "%s: ", lang->conf_defaultdetail);
				if(setting->defaultdetail==0)
					strcat(config[i], lang->conf_defaultdetail_none);
				else if(setting->defaultdetail==1)
					strcat(config[i], lang->conf_defaultdetail_size);
				else if(setting->defaultdetail==2)
					strcat(config[i], lang->conf_defaultdetail_modifytime);
			}
			else if(i==USBD_FLAG){	//USBD_USE
				sprintf(config[i], "%s: ", lang->conf_usbd_use);
				if(setting->usbd_flag)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==USBD_PATH)	//USBD_PATH
				sprintf(config[i], "%s: %s", lang->conf_usbd_path, setting->usbd_path);
			else if(i==USBMASS_FLAG){	//USBMASS_USE
				sprintf(config[i], "%s: ", lang->conf_usbmass_use);
				if(setting->usbmass_flag)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==USBMASS_PATH)	//USBMASS_PATH
				sprintf(config[i], "%s: %s", lang->conf_usbmass_path, setting->usbmass_path);
			else if(i==MISCINIT){	//INIT
				strcpy(config[i], lang->conf_miscsettinginit);
			}
		}
		nList=18;

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
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		else if(sel==LANG)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==TIMEOUT)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==DISCCONTROL)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==FILENAME)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
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
		else if(sel==MISCINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}

//-------------------------------------------------
//設定
void config(char *mainMsg)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList, sel=0, top=0;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];

	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmpip[16];
	char tmpnetmask[16];
	char tmpgw[16];

	tmpsetting = setting;
	setting = (SETTING*)malloc(sizeof(SETTING));
	*setting = *tmpsetting;

	strcpy(tmpip,ip);
	strcpy(tmpnetmask,netmask);
	strcpy(tmpgw,gw);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
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
				if(sel==COLORSETTING) config_color(setting);
				if(sel==SCREENSETTING) config_screen(setting);
				if(sel==NETWORK) config_network(setting);
				if(sel==FONTSETTING) config_font(setting);
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
					strcpy(ip,tmpip);
					strcpy(netmask,tmpnetmask);
					strcpy(gw,tmpgw);
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
					itoGsReset();
					setupito(setting->tvmode);
					mainMsg[0] = 0;
					break;
				}
			}
			else if(new_pad & PAD_START)
				sel=OK;
			else if(new_pad & PAD_SELECT)
				sel=CANCEL;
		}
		//
		strcpy(config[0], lang->conf_setting_button);
		strcpy(config[1], lang->conf_setting_color);
		strcpy(config[2], lang->conf_setting_screen);
		strcpy(config[3], lang->conf_setting_network);
		strcpy(config[4], lang->conf_setting_font);
		strcpy(config[5], lang->conf_setting_misc);
		strcpy(config[6], lang->conf_ok);
		strcpy(config[7], lang->conf_cancel);
		nList=8;

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
		strcpy(msg0, "CONFIG/");
		// 操作説明
		sprintf(msg1, "○:%s", lang->gen_ok);
		setScrTmp(msg0, msg1);
		drawScr();
	}
	return;
}
