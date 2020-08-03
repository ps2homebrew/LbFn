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
	DEF_SCREEN_X = 160,
	DEF_SCREEN_Y = 55,
	DEF_FLICKERCONTROL = TRUE,
	DEF_TVMODE = 0,	//0=auto 1=ntsc 2=pal 3=480p 4=720p
	DEF_INTERLACE = TRUE,	//FALSE=ITO_NON_INTERLACE TRUE=ITO_INTERLACE
	DEF_FFMODE = FALSE,	//FALSE=ITO_FIELD TRUE=ITO_FRAME

	DEF_CHAR_MARGIN = 2,
	DEF_LINE_MARGIN = 5,
	DEF_FONTBOLD = TRUE,
	DEF_ASCII_MARGINTOP = 0,
	DEF_ASCII_MARGINLEFT = 0,
	DEF_KANJI_MARGINTOP = 0,
	DEF_KANJI_MARGINLEFT = 0,

	DEF_DISCCONTROL = TRUE,
	DEF_FILEICON = TRUE,
	DEF_DISCPS2SAVECHECK = FALSE,
	DEF_DISCELFCHECK = FALSE,
	DEF_LANGUAGE = LANG_ENGLISH,
};

//CONFIG
enum
{
	BUTTONSETTING=0,
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

//SCREEN SETTING
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
	TVMODE,
	INTERLACE,
	FFMODE,
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
	EXPORTDIR,
	MISCINIT,
};

SETTING *setting, *tmpsetting;

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
// SCREEN SETTINGを初期化
void InitScreenSetting(void)
{
	setting->color[0] = DEF_COLOR1;
	setting->color[1] = DEF_COLOR2;
	setting->color[2] = DEF_COLOR3;
	setting->color[3] = DEF_COLOR4;
	setting->color[4] = DEF_COLOR5;
	setting->color[5] = DEF_COLOR6;
	setting->color[6] = DEF_COLOR7;
	setting->color[7] = DEF_COLOR8;
	setting->color[8] = DEF_COLOR9;
	setting->color[9] = DEF_COLOR10;
	setting->screen_x = DEF_SCREEN_X;
	setting->screen_y = DEF_SCREEN_Y;
	setting->flickerControl = DEF_FLICKERCONTROL;
	setting->tvmode = DEF_TVMODE;
	setting->interlace = DEF_INTERLACE;
	setting->ffmode = DEF_FFMODE;
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
	setting->Exportdir[0] = 0;
	setting->language = DEF_LANGUAGE;
}

//-------------------------------------------------
// 設定を初期化
void InitSetting(void)
{
	InitButtonSetting();
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
	sprintf(tmp, "%08lX", setting->color[0]);
	if(cnf_setstr("color_background", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[1]);
	if(cnf_setstr("color_fream", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[2]);
	if(cnf_setstr("color_highlight_text", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[3]);
	if(cnf_setstr("color_normal_text", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[4]);
	if(cnf_setstr("color_folder", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[5]);
	if(cnf_setstr("color_file", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[6]);
	if(cnf_setstr("color_ps2_save", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[7]);
	if(cnf_setstr("color_elf_file", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[8]);
	if(cnf_setstr("color_ps1_save", tmp)<0) goto error;
	sprintf(tmp, "%08lX", setting->color[9]);
	if(cnf_setstr("color_disable_text", tmp)<0) goto error;
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
	sprintf(tmp, "%d", setting->screen_x);
	if(cnf_setstr("screen_pos_x", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->screen_y);
	if(cnf_setstr("screen_pos_y", tmp)<0) goto error;
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
	strcpy(tmp, setting->Exportdir);
	if(cnf_setstr("export_dir", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->tvmode);
	if(cnf_setstr("tvmode", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->interlace);
	if(cnf_setstr("interlace", tmp)<0) goto error;
	sprintf(tmp, "%d", setting->ffmode);
	if(cnf_setstr("ffmode", tmp)<0) goto error;

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
				setting->color[0] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_fream", tmp, "")>=0)
				setting->color[1] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_highlight_text", tmp, "")>=0)
				setting->color[2] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_normal_text", tmp, "")>=0)
				setting->color[3] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_folder", tmp, "")>=0)
				setting->color[4] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_file", tmp, "")>=0)
				setting->color[5] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_ps2_save", tmp, "")>=0)
				setting->color[6] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_elf_file", tmp, "")>=0)
				setting->color[7] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_ps1_save", tmp, "")>=0)
				setting->color[8] = strtoul(tmp, NULL, 16);
			if(cnf_getstr("color_disable_text", tmp, "")>=0)
				setting->color[9] = strtoul(tmp, NULL, 16);
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
				setting->screen_x = atoi(tmp);
			if(cnf_getstr("screen_pos_y", tmp, "")>=0)
				setting->screen_y = atoi(tmp);
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
			if(cnf_getstr("export_dir", tmp, "")>=0)
				strcpy(setting->Exportdir, tmp);
			if(cnf_getstr("tvmode", tmp, "")>=0){
				setting->tvmode = atoi(tmp);
				if(setting->tvmode<0 || setting->tvmode>4)
					setting->tvmode = DEF_TVMODE;
			}
			if(cnf_getstr("interlace", tmp, "")>=0){
				setting->interlace = atoi(tmp);
				if(setting->interlace<0 || setting->interlace>1)
					setting->interlace = DEF_INTERLACE;
			}
			if(cnf_getstr("ffmode", tmp, "")>=0){
				setting->ffmode = atoi(tmp);
				if(setting->ffmode<0 || setting->ffmode>1)
					setting->ffmode = DEF_FFMODE;
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
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
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
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
//画面設定
void config_screen(SETTING *setting)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0 ,sel_x=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;
	char config[32][MAX_PATH];
	int r,g,b;

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
				if(sel>=COLOR1 && sel<=COLOR10){
					sel_x--;
					if(sel_x<0){
						sel_x=2;
						sel--;
					}
				}
				else if(sel>TVMODE)
					sel=TVMODE;
				else
					sel=0;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel>=COLOR1 && sel<=COLOR10){
					sel_x++;
					if(sel_x>2){
						sel_x=0;
						sel++;
					}
				}
				else if(sel==0)
					sel=TVMODE;
				else
					sel+=MAX_ROWS/2;
			}
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel>=COLOR1 && sel<=COLOR10){
					r = setting->color[sel-1] & 0xFF;
					g = setting->color[sel-1] >> 8 & 0xFF;
					b = setting->color[sel-1] >> 16 & 0xFF;
					if(sel_x==0 && r<255) r++;
					if(sel_x==1 && g<255) g++;
					if(sel_x==2 && b<255) b++;
					setting->color[sel-1] = ITO_RGBA(r, g, b, 0);
				}
				else if(sel==TVMODE){	//TVMODE
					//tvmode変更
					setting->tvmode++;
					if(setting->tvmode==2){	//NTSCからPALへ変更
						setting->screen_x += 5;
						setting->screen_y += 10;
					}
					//
					if(setting->tvmode==3){	//PALから480pへ変更
						if(setting->interlace)
							setting->screen_y -= 30;
						setting->ffmode = ITO_FIELD;
						setting->interlace = ITO_NON_INTERLACE;
						setting->screen_y += 15;
					}
					if(setting->tvmode==4)	//480pから720pへ変更
						setting->screen_x += 170;
					if(setting->tvmode==5){	//720pからautoへ変更
						setting->screen_x -= 175;
						setting->screen_y += 5;
						setting->tvmode = 0;
						setting->interlace = ITO_INTERLACE;
					}
					//
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
				}
				else if(sel==INTERLACE){	//インターレース
					if(setting->tvmode<3){
						if(setting->interlace)
							setting->screen_y-=30;
						else
							setting->screen_y+=30;
						setting->interlace = !setting->interlace;
						//
						itoGsReset();
						setupito(setting->tvmode);
						SetHeight();
					}
				}
				else if(sel==FFMODE){	//ffmode
					if(setting->tvmode<3){
						setting->ffmode = !setting->ffmode;
						itoGsReset();
						setupito(setting->tvmode);
						SetHeight();
					}
				}
				else if(sel==SCREEN_X){	//SCREEN X
					setting->screen_x++;
					screen_env.screen.x = setting->screen_x;
					itoSetScreenPos(setting->screen_x, setting->screen_y);
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					setting->screen_y++;
					screen_env.screen.y = setting->screen_y;
					itoSetScreenPos(setting->screen_x, setting->screen_y);
				}
				else if(sel==FLICKERCONTROL)	//フリッカーコントロール
					setting->flickerControl = !setting->flickerControl;
				else if(sel==SCREENINIT){	//SCREEN SETTING INIT
					//init
					InitScreenSetting();
					itoGsReset();
					setupito(setting->tvmode);
					SetHeight();
					//sprintf(msg0, "%s", "Initialize Screen Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel>=COLOR1 && sel<=COLOR10){
					r = setting->color[sel-1] & 0xFF;
					g = setting->color[sel-1] >> 8 & 0xFF;
					b = setting->color[sel-1] >> 16 & 0xFF;
					if(sel_x==0 && r>0) r--;
					if(sel_x==1 && g>0) g--;
					if(sel_x==2 && b>0) b--;
					setting->color[sel-1] = ITO_RGBA(r, g, b, 0);
				}
				else if(sel==SCREEN_X){	//SCREEN X
					if(setting->screen_x > 0){
						setting->screen_x--;
						screen_env.screen.x = setting->screen_x;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
				}
				else if(sel==SCREEN_Y){	//SCREEN Y
					if(setting->screen_y > 0){
						setting->screen_y--;
						screen_env.screen.y = setting->screen_y;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
				}
			}
			else if(new_pad & PAD_L3){
				static int preset=0;
				//デフォルト
				if(preset==0){
					setting->color[0] = DEF_COLOR1;
					setting->color[1] = DEF_COLOR2;
					setting->color[2] = DEF_COLOR3;
					setting->color[3] = DEF_COLOR4;
					setting->color[4] = DEF_COLOR5;
					setting->color[5] = DEF_COLOR6;
					setting->color[6] = DEF_COLOR7;
					setting->color[7] = DEF_COLOR8;
					setting->color[8] = DEF_COLOR9;
					setting->color[9] = DEF_COLOR10;
				}
				//Unofficial LaunchELF
				if(preset==1){
					setting->color[0] = ITO_RGBA(128,128,128,0);
					setting->color[1] = ITO_RGBA(64,64,64,0);
					setting->color[2] = ITO_RGBA(96,0,0,0);
					setting->color[3] = ITO_RGBA(0,0,0,0);
					setting->color[4] = DEF_COLOR5;
					setting->color[5] = ITO_RGBA(96,96,96,0);
					setting->color[6] = DEF_COLOR7;
					setting->color[7] = DEF_COLOR8;
					setting->color[8] = ITO_RGBA(0,96,192,0);
					setting->color[9] = ITO_RGBA(64,64,64,0);
				}
				//黒い背景
				if(preset==2){
					setting->color[0] = ITO_RGBA(24,24,24,0);
					setting->color[1] = ITO_RGBA(64,64,64,0);
					setting->color[2] = ITO_RGBA(255,128,0,0);
					setting->color[3] = ITO_RGBA(144,144,144,0);
					setting->color[4] = DEF_COLOR5;
					setting->color[5] = DEF_COLOR6;
					setting->color[6] = DEF_COLOR7;
					setting->color[7] = DEF_COLOR8;
					setting->color[8] = DEF_COLOR9;
					setting->color[9] = ITO_RGBA(64,64,64,0);
				}
				preset++;
				if(preset>2) preset=0;
			}
		}

		//
		for(i=0;i<=SCREENINIT;i++){
			if(i==0){
				sprintf(config[i], "..");
			}
			else if(i>=COLOR1 && i<=COLOR10){	//COLOR
				r = setting->color[i-1] & 0xFF;
				g = setting->color[i-1] >> 8 & 0xFF;
				b = setting->color[i-1] >> 16 & 0xFF;
				if(i==COLOR1) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_background, r, g, b);
				if(i==COLOR2) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_frame, r, g, b);
				if(i==COLOR3) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_highlighttext, r, g, b);
				if(i==COLOR4) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_normaltext, r, g, b);
				if(i==COLOR5) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_folder, r, g, b);
				if(i==COLOR6) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_file, r, g, b);
				if(i==COLOR7) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_ps2save, r, g, b);
				if(i==COLOR8) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_elffile, r, g, b);
				if(i==COLOR9) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_ps1save, r, g, b);
				if(i==COLOR10) sprintf(config[i], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_disabletext, r, g, b);
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
			}
			else if(i==INTERLACE){	//INTERLACE
				sprintf(config[i], "%s: ", lang->conf_interlace);
				if(setting->interlace)
					strcat(config[i], lang->conf_on);
				else
					strcat(config[i], lang->conf_off);
			}
			else if(i==FFMODE){	//FFMODE
				sprintf(config[i],"%s: ", lang->conf_ffmode);
				if(setting->ffmode)
					strcat(config[i], lang->conf_ffmode_frame);
				else
					strcat(config[i], lang->conf_ffmode_field);
			}
			else if(i==SCREEN_X){	//SCREEN X
				sprintf(config[i],"%s: %3d", lang->conf_screen_x, setting->screen_x);
			}
			else if(i==SCREEN_Y){	//SCREEN Y
				sprintf(config[i],"%s: %3d", lang->conf_screen_y, setting->screen_y);
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
		nList=18;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
			//カーソル表示
			if(top+i == sel){
				if(sel>=COLOR1 && sel<=COLOR10)
					printXY(">", FONT_WIDTH*21 + FONT_WIDTH*sel_x*8, y, color, TRUE);
				else
					printXY(">", x, y, color, TRUE);
			}
			//リスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			//色のプレビュー
			if(top+i>=COLOR1 && top+i<=COLOR10){
				itoSprite(setting->color[top+i-1],
					x+FONT_WIDTH*42, y,
					x+FONT_WIDTH*42+font_h, y+font_h, 0);
			}
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame((MAX_ROWS_X+8)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*3,
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
		else if(sel>=COLOR1 && sel<=COLOR10)
			sprintf(msg1, "○:%s ×:%s △:%s", lang->conf_add, lang->conf_away, lang->conf_up);
		else if(sel==TVMODE)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==INTERLACE)
			sprintf(msg1, "○:%s △:%s", lang->conf_change, lang->conf_up);
		else if(sel==FFMODE)
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
					MessageDialog(tmp);
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
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
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
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
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
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
		else if(sel==FONTINIT)
			sprintf(msg1, "○:%s △:%s", lang->gen_ok, lang->conf_up);
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
				else if(sel==EXPORTDIR){
					getFilePath(setting->Exportdir, DIR);
					if(!strncmp(setting->Exportdir, "cdfs", 2))
						setting->Exportdir[0]='\0';
				}
				else if(sel==MISCINIT){
					//init
					InitMiscSetting();
					SetLanguage(setting->language);
					//sprintf(msg0, "%s", "Initialize Misc Setting");
					//pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==TIMEOUT)
					setting->timeout--;
				if(sel==EXPORTDIR)
					setting->Exportdir[0]='\0';
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
				sprintf(config[6], "%s: " ,lang->conf_disc_ps2save_check);
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
			else if(i==EXPORTDIR){	//EXPORT DIR
				sprintf(config[i], "%s: %s", lang->conf_export_dir, setting->Exportdir);
			}
			else if(i==MISCINIT){	//INIT
				strcpy(config[i], lang->conf_miscsettinginit);
			}
		}
		nList=10;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
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
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
		else if(sel==EXPORTDIR)
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
		strcpy(config[1], lang->conf_setting_screen);
		strcpy(config[2], lang->conf_setting_network);
		strcpy(config[3], lang->conf_setting_font);
		strcpy(config[4], lang->conf_setting_misc);
		strcpy(config[5], lang->conf_ok);
		strcpy(config[6], lang->conf_cancel);
		nList=7;

		// リスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// リスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nList) break;
			//色
			if(top+i == sel)
				color = setting->color[2];
			else
				color = setting->color[3];
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
				(MAX_ROWS_X+9)*FONT_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
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
