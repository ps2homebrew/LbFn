#include "launchelf.h"

#define NUM_CNF_KEY 43
const char *cnf_keyname[NUM_CNF_KEY] = 
{
	//version
	"cnf_version",
	//Launcher
	"DEFAULT",
	"CIRCLE",
	"CROSS",
	"SQUARE",
	"TRIANGLE",
	"L1",
	"R1",
	"L2",
	"R2",
	"L3",
	"R3",
	"START",
	//color
	"color_background",
	"color_fream",
	"color_highlight_text",
	"color_normal_text",
	"color_folder",
	"color_file",
	"color_ps2_save",
	"color_elf_file",
	//font
	"ascii_font",
	"kanji_font",
	"char_margin",
	"line_margin",
	"font_bold",
	"ascii_margin_top",
	"ascii_margin_left",
	"aanji_margin_top",
	"aanji_margin_left",
	//
	"screen_pos_x",
	"screen_pos_y",
	"flicker_control",
	"language",
	"timeout",
	"disc_control",
	"only_filename",
	"file_icon",
	"ps2save_check",
	"elf_check",
	"export_dir",
	"interlace",
	"ffmode",
};

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
	DEF_SCREEN_X = 160,
	DEF_SCREEN_Y = 55,
	DEF_FLICKERCONTROL = TRUE,
	DEF_INTERLACE = TRUE,
	DEF_FFMODE = FALSE,

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

enum
{
	BUTTONSETTING=0,
	SCREENSETTING,
	NETWORK,
	FONTSETTING,
	MISC,
	OK,
	CANCEL,

	DEFAULT=1,
	LAUNCHER1,
	LAUNCHER11=12,
	BUTTONINIT,

	COLOR1=1,
	COLOR8=8,
	INTERLACE,
	FFMODE,
	SCREEN_X,
	SCREEN_Y,
	FLICKERCONTROL,
	SCREENINIT,

	IPADDRESS=1,
	NETMASK,
	GATEWAY,
	NETWORKSAVE,
	NETWORKINIT,

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
// メモリーカードの状態をチェックする。
// 戻り値は有効なメモリーカードスロットの番号。
// メモリーカードが挿さっていない場合は-11を返す。
int CheckMC(void)
{
	int ret;
	
	mcGetInfo(0, 0, NULL, NULL, NULL);
	mcSync(0, NULL, &ret);

	if( -1 == ret || 0 == ret) return 0;

	mcGetInfo(0, 0, NULL, NULL, NULL);
	mcSync(0, NULL, &ret);

	if( -1 == ret || 0 == ret ) return 1;

	return -11;
}

//-------------------------------------------------
// BUTTON SETTINGを初期化
void InitButtonSetting(SETTING *setting)
{
	int i;

	for(i=0; i<12; i++) setting->dirElf[i][0] = 0;

	strcpy(setting->dirElf[1], "MISC/FileBrowser");
}

//-------------------------------------------------
// SCREEN SETTINGを初期化
void InitScreenSetting(SETTING *setting)
{
	setting->color[0] = DEF_COLOR1;
	setting->color[1] = DEF_COLOR2;
	setting->color[2] = DEF_COLOR3;
	setting->color[3] = DEF_COLOR4;
	setting->color[4] = DEF_COLOR5;
	setting->color[5] = DEF_COLOR6;
	setting->color[6] = DEF_COLOR7;
	setting->color[7] = DEF_COLOR8;
	setting->screen_x = DEF_SCREEN_X;
	setting->screen_y = DEF_SCREEN_Y;
	setting->flickerControl = DEF_FLICKERCONTROL;
	setting->interlace = DEF_INTERLACE;
	setting->ffmode = DEF_FFMODE;
}

//-------------------------------------------------
// FONT SETTINGを初期化
void InitFontSetting(SETTING *setting)
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
void InitMiscSetting(SETTING *setting)
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
void InitSetting(SETTING *setting)
{
	InitButtonSetting(setting);
	InitScreenSetting(setting);
	InitFontSetting(setting);
	InitMiscSetting(setting);
}

//-------------------------------------------------
void saveConfig(char *mainMsg)
{
	int fd, mcport;
	char path[MAX_PATH];
	char tmp[MAX_PATH];
	int i, ret, error;

	//cnfファイルのパス
	//hostから起動しているか調べる
	if(!strncmp(LaunchElfDir, "host", 4)){
		//有効なmcのSYS-CONFフォルダ
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
	}
	else{
		// LaunchELFが実行されたパスから設定ファイルを開く
		sprintf(path, "%s%s", LaunchElfDir, "LBF.CNF");
		if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0)
			fioClose(fd);
		// 開けなかったら、SYS-CONFの設定ファイルを開く
		else{
			//mcから起動しているか調べる
			if(!strncmp(LaunchElfDir, "mc", 2))
				//mcのとき
				mcport = LaunchElfDir[2]-'0';
			else{
				//mc以外のときは有効なmcのSYS-CONFフォルダ
				mcport = CheckMC();
				if(mcport==1 || mcport==0){
					sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
				}
				else{
					path[0]=0;
				}
			}
		}
	}

	cnf_init();

	//cnfファイルオープン
	if(cnf_load(path)==FALSE){
		path[0]=0;
	}

	error=FALSE;
	for(i=0;i<NUM_CNF_KEY;i++){
		//version
		if(i==0)
			sprintf(tmp, "%d", 2);
		//Launcher
		if(i>=1 && i<=12)
			strcpy(tmp, setting->dirElf[i-1]);
		//color
		if(i>=13 && i<=20)
			sprintf(tmp, "%08lX", setting->color[i-13]);
		//font
		if(i==21)
			strcpy(tmp, setting->AsciiFont);
		if(i==22)
			strcpy(tmp, setting->KanjiFont);
		if(i==23)
			sprintf(tmp, "%d", setting->CharMargin);
		if(i==24)
			sprintf(tmp, "%d", setting->LineMargin);
		if(i==25)
			sprintf(tmp, "%d", setting->FontBold);
		if(i==26)
			sprintf(tmp, "%d", setting->AsciiMarginTop);
		if(i==27)
			sprintf(tmp, "%d", setting->AsciiMarginLeft);
		if(i==28)
			sprintf(tmp, "%d", setting->KanjiMarginTop);
		if(i==29)
			sprintf(tmp, "%d", setting->KanjiMarginLeft);
		//
		if(i==30)
			sprintf(tmp, "%d", setting->screen_x);
		if(i==31)
			sprintf(tmp, "%d", setting->screen_y);
		if(i==32)
			sprintf(tmp, "%d", setting->flickerControl);
		if(i==33)
			sprintf(tmp, "%d", setting->language);
		if(i==34)
			sprintf(tmp, "%d", setting->timeout);
		if(i==35)
			sprintf(tmp, "%d", setting->discControl);
		if(i==36)
			sprintf(tmp, "%d", setting->filename);
		if(i==37)
			sprintf(tmp, "%d", setting->fileicon);
		if(i==38)
			sprintf(tmp, "%d", setting->discPs2saveCheck);
		if(i==39)
			sprintf(tmp, "%d", setting->discELFCheck);
		if(i==40)
			strcpy(tmp, setting->Exportdir);
		if(i==41)
			sprintf(tmp, "%d", setting->interlace);
		if(i==42)
			sprintf(tmp, "%d", setting->ffmode);
		//
		ret = cnf_setstr(cnf_keyname[i], tmp);
		if(ret<0){
			error=TRUE;
			break;
		}
	}

	//エラーがあった
	if(error==TRUE){
		sprintf(mainMsg, "%s", lang->conf_savefailed);
		cnf_free();
		return;
	}

	//cnfファイルのパス
	if(!strncmp(LaunchElfDir, "host", 4)){
		//有効なmcのSYS-CONFフォルダ
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
	}
	if(path[0]==0) sprintf(path, "%s%s", LaunchElfDir, "LBF.CNF");

	//cnf保存
	ret = cnf_save(path);
	if(ret<0)
		sprintf(mainMsg, "%s", lang->conf_savefailed);
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
	int i;

	setting = (SETTING*)malloc(sizeof(SETTING));

	//cnfファイルのパス
	//hostから起動しているか調べる
	if(!strncmp(LaunchElfDir, "host", 4)){
		//有効なmcのSYS-CONFフォルダ
		strcpy(path, "mc0:/SYS-CONF/LBF.CNF");
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0)
			fioClose(fd);
		else{
			strcpy(path, "mc1:/SYS-CONF/LBF.CNF");
			fd = fioOpen(path, O_RDONLY);
			if(fd>=0)
				fioClose(fd);
			else
				path[0] = 0;
		}
	}
	else{
		// LaunchELFが実行されたパスから設定ファイルを開く
		sprintf(path, "%s%s", LaunchElfDir, "LBF.CNF");
		if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0)
			fioClose(fd);
		// 開けなかったら、SYS-CONFの設定ファイルを開く
		else{
			//mcから起動しているか調べる
			if(!strncmp(LaunchElfDir, "mc", 2))
				//mcのとき
				mcport = LaunchElfDir[2]-'0';
			else
				//mc以外のときは有効なmcのSYS-CONFフォルダ
				mcport = CheckMC();
			if(mcport==1 || mcport==0){
				sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
			}
		}
	}

	//設定を初期化する
	InitSetting(setting);

	cnf_init();
	//cnfファイルオープン
	if(cnf_load(path)<0){
		ret=0;
	}
	else{
		ret=1;
		cnf_version = 0;
		for(i=0;i<NUM_CNF_KEY;i++){
			if(cnf_getstr(cnf_keyname[i], tmp, "")>=0){
				//version
				if(i==0)
					cnf_version = atoi(tmp);
				//Launcher
				if(i>=1 && i<=12)
					strcpy(setting->dirElf[i-1], tmp);
				//color
				if(i>=13 && i<=20)
					setting->color[i-13] = strtoul(tmp, NULL, 16);
				//font
				if(i==21)
					strcpy(setting->AsciiFont, tmp);
				if(i==22)
					strcpy(setting->KanjiFont, tmp);
				if(i==23)
					setting->CharMargin = atoi(tmp);
				if(i==24)
					setting->LineMargin = atoi(tmp);
				if(i==25)
					setting->FontBold = atoi(tmp);
					if(setting->FontBold<0 || setting->FontBold>1)
						setting->FontBold = DEF_FONTBOLD;
				if(i==26)
					setting->AsciiMarginTop = atoi(tmp);
				if(i==27)
					setting->AsciiMarginLeft = atoi(tmp);
				if(i==28)
					setting->KanjiMarginTop = atoi(tmp);
				if(i==29)
					setting->KanjiMarginLeft = atoi(tmp);
				//
				if(i==30)
					setting->screen_x = atoi(tmp);
				if(i==31)
					setting->screen_y = atoi(tmp);
				if(i==32){
					setting->flickerControl = atoi(tmp);
					if(setting->flickerControl<0 || setting->flickerControl>1)
						setting->flickerControl = DEF_FLICKERCONTROL;
				}
				if(i==33){
					setting->language = atoi(tmp);
					if(setting->language<0 || setting->flickerControl>=NUM_LANG)
						setting->language = DEF_LANGUAGE;
				}
				if(i==34){
					setting->timeout = atoi(tmp);
					if(setting->timeout<0)
						setting->timeout = DEF_TIMEOUT;
				}
				if(i==35){
					setting->discControl = atoi(tmp);
					if(setting->discControl<0 || setting->discControl>1)
						setting->discControl = DEF_DISCCONTROL;
				}
				if(i==36){
					setting->filename = atoi(tmp);
					if(setting->filename<0 || setting->filename>1)
						setting->filename = DEF_FILENAME;
				}
				if(i==37){
					setting->fileicon = atoi(tmp);
					if(setting->fileicon<0 || setting->fileicon>1)
						setting->fileicon = DEF_FILEICON;
				}
				if(i==38){
					setting->discPs2saveCheck = atoi(tmp);
					if(setting->discPs2saveCheck<0 || setting->discPs2saveCheck>1)
						setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
				}
				if(i==39){
					setting->discELFCheck = atoi(tmp);
					if(setting->discELFCheck<0 || setting->discELFCheck>1)
						setting->discELFCheck = DEF_DISCELFCHECK;
				}
				if(i==40)
					strcpy(setting->Exportdir, tmp);
				if(i==41){
					setting->interlace = atoi(tmp);
					if(setting->interlace<0 || setting->interlace>1)
						setting->interlace = DEF_INTERLACE;
				}
				if(i==42){
					setting->ffmode = atoi(tmp);
					if(setting->ffmode<0 || setting->ffmode>1)
						setting->ffmode = DEF_FFMODE;
				}
			}
		}
		//バージョンチェック
		if(cnf_version!=2){
			//Setting初期化
			InitSetting(setting);
			//ファイルサイズを0にする
			fd = fioOpen(path, O_WRONLY | O_TRUNC | O_CREAT);
			fioClose(fd);
			ret=2;
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
				if(sel==0) break;
				if(sel>=DEFAULT && sel<=LAUNCHER11){
					getFilePath(setting->dirElf[sel-1], ELF_FILE);
					if(!strncmp(setting->dirElf[sel-1], "mc", 2)){
						sprintf(c, "mc%s", &setting->dirElf[sel-1][3]);
						strcpy(setting->dirElf[sel-1], c);
					}
				}
				else if(sel==BUTTONINIT){
					InitButtonSetting(setting);
					sprintf(msg0, "%s", "Initialize Button Setting");
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel>=1 && sel<=12) setting->dirElf[sel-1][0]=0;
			}
		}
		//BUTTON SETTING
		strcpy(config[0], "..");
		strcpy(config[1], "DEFAULT: ");
		strcpy(config[2], "○     : ");
		strcpy(config[3], "×     : ");
		strcpy(config[4], "□     : ");
		strcpy(config[5], "△     : ");
		strcpy(config[6], "L1     : ");
		strcpy(config[7], "R1     : ");
		strcpy(config[8], "L2     : ");
		strcpy(config[9], "R2     : ");
		strcpy(config[10],"L3     : ");
		strcpy(config[11],"R3     : ");
		strcpy(config[12],"START  : ");
		strcpy(config[13],lang->conf_buttonsettinginit);
		for(i=0; i<12; i++)
			strcat(config[i+1], setting->dirElf[i]);
		nList=14;

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_button);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if (sel>=DEFAULT && sel<=LAUNCHER11)
			sprintf(msg1, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
		else if(sel==BUTTONINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
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
				if(sel>=COLOR1 && sel<=COLOR8) sel_x--;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel>=COLOR1 && sel<=COLOR8) sel_x++;
			}
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				if(sel==0) break;
				if(sel>=COLOR1 && sel<=COLOR8){
					r = setting->color[sel-1] & 0xFF;
					g = setting->color[sel-1] >> 8 & 0xFF;
					b = setting->color[sel-1] >> 16 & 0xFF;
					if(sel_x==0 && r<255) r++;
					if(sel_x==1 && g<255) g++;
					if(sel_x==2 && b<255) b++;
					setting->color[sel-1] = ITO_RGBA(r, g, b, 0);
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
				else if(sel==INTERLACE){	//インターレース
					setting->interlace = !setting->interlace;
					if(setting->interlace) setting->screen_y+=30;
					else setting->screen_y-=30;

					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = setting->interlace;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					//アルファブレンド
					itoSetAlphaBlending(
						ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
						ITO_ALPHA_COLOR_DST, // B = COLOR DEST
						ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
						ITO_ALPHA_COLOR_DST, // C = COLOR DEST
						0x80);				 // Fixed Value
					SetHeight();
				}
				else if(sel==FFMODE){	//ffmode
					setting->ffmode = !setting->ffmode;

					screen_env.ffmode = setting->ffmode;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					//アルファブレンド
					itoSetAlphaBlending(
						ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
						ITO_ALPHA_COLOR_DST, // B = COLOR DEST
						ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
						ITO_ALPHA_COLOR_DST, // C = COLOR DEST
						0x80);				 // Fixed Value
					SetHeight();
				}
				else if(sel==FLICKERCONTROL)	//フリッカーコントロール
					setting->flickerControl = !setting->flickerControl;
				else if(sel==SCREENINIT){	//SCREEN SETTING INIT
					//init
					InitScreenSetting(setting);

					screen_env.screen.x = setting->screen_x;
					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = setting->interlace;
					screen_env.ffmode = setting->ffmode;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					//アルファブレンド
					itoSetAlphaBlending(
						ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
						ITO_ALPHA_COLOR_DST, // B = COLOR DEST
						ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
						ITO_ALPHA_COLOR_DST, // C = COLOR DEST
						0x80);				 // Fixed Value
					SetHeight();
					sprintf(msg0, "%s", "Initialize Screen Setting");
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel>=COLOR1 && sel<=COLOR8){
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
		}
		//SCREEN SETTING
		sprintf(config[0], "..");
		for(i=0;i<8;i++){
			r = setting->color[i] & 0xFF;
			g = setting->color[i] >> 8 & 0xFF;
			b = setting->color[i] >> 16 & 0xFF;
			if(i==0) sprintf(config[1], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_background, r, g, b);
			if(i==1) sprintf(config[2], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_frame, r, g, b);
			if(i==2) sprintf(config[3], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_highlighttext, r, g, b);
			if(i==3) sprintf(config[4], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_normaltext, r, g, b);
			if(i==4) sprintf(config[5], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_folder, r, g, b);
			if(i==5) sprintf(config[6], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_file, r, g, b);
			if(i==6) sprintf(config[7], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_ps2save, r, g, b);
			if(i==7) sprintf(config[8], "%s:   R:%3d   G:%3d   B:%3d", lang->conf_elffile, r, g, b);
		}
		//INTERLACE
		sprintf(config[9], "%s: ", lang->conf_interlace);
		if(setting->interlace)
			strcat(config[9], lang->conf_on);
		else
			strcat(config[9], lang->conf_off);
		//FFMODE
		sprintf(config[10],"%s: ", lang->conf_ffmode);
		if(setting->ffmode)
			strcat(config[10], lang->conf_on);
		else
			strcat(config[10], lang->conf_off);
		//SCREEN X
		sprintf(config[11],"%s: %3d", lang->conf_screen_x, setting->screen_x);
		//SCREEN Y
		sprintf(config[12],"%s: %3d", lang->conf_screen_y, setting->screen_y);
		//FLICKER CONTROL
		sprintf(config[13],"%s: ", lang->conf_flickercontrol);
		if(setting->flickerControl)
			strcat(config[13], lang->conf_on);
		else
			strcat(config[13], lang->conf_off);
		//INIT
		strcpy(config[14], lang->conf_screensettinginit);
		nList=15;

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;
		if(sel_x < 0)			sel_x=2;
		if(sel_x > 2)			sel_x=0;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
				if(sel>=COLOR1 && sel<=COLOR8)
					printXY(">", FONT_WIDTH*21 + FONT_WIDTH*sel_x*8, y, color, TRUE);
				else
					printXY(">", x, y, color, TRUE);
			}
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			//色のプレビュー
			if(top+i>=COLOR1 && top+i<=COLOR8){
				itoSprite(setting->color[top+i-1],
					x+FONT_WIDTH*42, y,
					x+FONT_WIDTH*42+font_h, y+font_h, 0);
			}
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_screen);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if(sel>=COLOR1 && sel<=COLOR8)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==INTERLACE)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==FFMODE)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==FLICKERCONTROL)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==SCREENINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
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
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,ip);
					if(keyboard(tmp, 15)>=0) strcpy(ip,tmp);
				}
				else if(sel==NETMASK){
					drawDark();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,netmask);
					if(keyboard(tmp, 15)>=0) strcpy(netmask,tmp);
				}
				else if(sel==GATEWAY){
					drawDark();
					itoSwitchFrameBuffers();
					drawDark();
					strcpy(tmp,gw);
					if(keyboard(tmp, 15)>=0) strcpy(gw,tmp);
				}
				else if(sel==NETWORKSAVE){
					//save
					sprintf(tmp, "%s %s %s", ip, netmask, gw);
					//フォルダ作成
					newdir("mc0:/","SYS-CONF");
					// 書き込み
					if((fd=fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT",O_CREAT|O_WRONLY|O_TRUNC)) >= 0){
						fioWrite(fd, tmp, strlen(tmp));
						fioClose(fd);
						sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s",lang->conf_ipsaved);
					}
					else{
						sprintf(tmp, "mc0:/SYS-CONF/IPCONFIG.DAT\n%s",lang->conf_ipsavefailed);
					}
					drawDark();
					itoSwitchFrameBuffers();
					drawDark();
					MessageDialog(tmp);
				}
				else if(sel==NETWORKINIT){
					//init
					strcpy(ip, "192.168.0.10");
					strcpy(netmask, "255.255.255.0");
					strcpy(gw, "192.168.0.1");
					sprintf(msg0, "%s", "Initialize Network Setting");
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
			}
		}
		//MISC SETTING
		sprintf(config[0], "..");
		//IPADDRESS
		sprintf(config[1], "%s: %s", lang->conf_ipaddress, ip);
		//NETMASK
		sprintf(config[2], "%s: %s", lang->conf_netmask, netmask);
		//GATEWAY
		sprintf(config[3], "%s: %s", lang->conf_gateway, gw);
		//NETWORKSAVE
		strcpy(config[4],lang->conf_ipoverwrite);
		//NETWORKINIT
		strcpy(config[5],lang->conf_ipsettinginit);
		nList=6;

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_network);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if(sel>=IPADDRESS && sel<=GATEWAY)
			sprintf(msg1, "○:%s", lang->conf_edit);
		else if(sel==NETWORKINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
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
					getFilePath(setting->AsciiFont, FNT_FILE);
					if(!strncmp(setting->AsciiFont, "mc", 2)){
						sprintf(c, "mc%s", &setting->AsciiFont[3]);
						strcpy(setting->AsciiFont, c);
					}
					if(InitFontAscii(setting->AsciiFont)<0){
						sprintf(setting->AsciiFont, "systemfont");
						InitFontAscii(setting->AsciiFont);
					}
				}
				else if(sel==KANJIFONT){
					getFilePath(setting->KanjiFont, FNT_FILE);
					if(!strncmp(setting->KanjiFont, "mc", 2)){
						sprintf(c, "mc%s", &setting->KanjiFont[3]);
						strcpy(setting->KanjiFont, c);
					}
					if(InitFontKnaji(setting->KanjiFont)<0){
						sprintf(setting->KanjiFont, "systemfont");
						InitFontKnaji(setting->KanjiFont);
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
					InitFontSetting(setting);
					InitFontAscii(setting->AsciiFont);
					InitFontKnaji(setting->KanjiFont);
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontBold(setting->FontBold);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
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
					setting->LineMargin--;
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
		//FONT SETTING
		sprintf(config[0], "..");
		//ASCIIFONT
		sprintf(config[1], "%s: %s", lang->conf_AsciiFont, setting->AsciiFont);
		//KANJIFONT
		sprintf(config[2], "%s: %s", lang->conf_KanjiFont, setting->KanjiFont);
		//CHARMARGIN
		sprintf(config[3], "%s: %d", lang->conf_CharMargin, setting->CharMargin);
		//LINEMARGIN
		sprintf(config[4], "%s: %d", lang->conf_LineMargin, setting->LineMargin);
		//FONTBOLD
		sprintf(config[5], "%s: ", lang->conf_FontBold);
		if(setting->FontBold)
			strcat(config[5], lang->conf_on);
		else
			strcat(config[5], lang->conf_off);
		//ASCIIMARGINTOP
		sprintf(config[6], "%s: %d", lang->conf_AsciiMarginTop, setting->AsciiMarginTop);
		//ASCIIMARGINLEFT
		sprintf(config[7], "%s: %d", lang->conf_AsciiMarginLeft, setting->AsciiMarginLeft);
		//KANJIMARGINTOP
		sprintf(config[8], "%s: %d", lang->conf_KanjiMarginTop, setting->KanjiMarginTop);
		//KANJIMARGINLEFT
		sprintf(config[9], "%s: %d", lang->conf_KanjiMarginLeft, setting->KanjiMarginLeft);
		//FONT INIT
		strcpy(config[10], lang->conf_fontsettinginit);
		nList=11;

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_font);
		// 操作説明
		if(sel==ASCIIFONT)
			sprintf(msg1, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
		else if(sel==KANJIFONT)
			sprintf(msg1, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
		else if(sel==CHARMARGIN)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==LINEMARGIN)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==FONTBOLD)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==ASCIIMARGINTOP)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==ASCIIMARGINLEFT)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==KANJIMARGINTOP)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==KANJIMARGINLEFT)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==FONTINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if(sel==FONTINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if(sel==FONTINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
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
				else if(sel==FILENAME)
					setting->filename = !setting->filename;
				else if(sel==DISCCONTROL)
					setting->discControl = !setting->discControl;
				else if(sel==FILEICON)
					setting->fileicon = !setting->fileicon;
				else if(sel==PS2SAVECHECK)
						setting->discPs2saveCheck = !setting->discPs2saveCheck;
				else if(sel==ELFCHECK)
						setting->discELFCheck = !setting->discELFCheck;
				else if(sel==EXPORTDIR)
					getFilePath(setting->Exportdir, DIR);
				else if(sel==MISCINIT){
					//init
					InitMiscSetting(setting);
					SetLanguage(setting->language);
					sprintf(msg0, "%s", "Initialize Misc Setting");
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_CROSS){	//×
				if(sel==TIMEOUT)
					setting->timeout--;
				if(sel==EXPORTDIR)
					setting->Exportdir[0]=0;
			}
		}
		//MISC SETTING
		sprintf(config[0], "..");
		//LANG
		sprintf(config[1], "%s: ", lang->conf_language);
		if(setting->language==LANG_ENGLISH)
			strcat(config[1], lang->conf_language_us);
		else if(setting->language==LANG_JAPANESE)
			strcat(config[1], lang->conf_language_jp);
		//TIMEOUT
		sprintf(config[2], "%s: %d", lang->conf_timeout, setting->timeout);
		//DISC CONTROL
		sprintf(config[3], "%s: " ,lang->conf_disc_control);
		if(setting->discControl)
			strcat(config[3], lang->conf_on);
		else
			strcat(config[3], lang->conf_off);
		//PRINT ONLY FILENAME
		sprintf(config[4], "%s: " ,lang->conf_print_only_filename);
		if(setting->filename)
			strcat(config[4], lang->conf_on);
		else
			strcat(config[4], lang->conf_off);
		//FILEICON
		sprintf(config[5], "%s: " ,lang->conf_fileicon);
		if(setting->fileicon)
			strcat(config[5], lang->conf_on);
		else
			strcat(config[5], lang->conf_off);
		//DISC PS2SAVE CHECK
		sprintf(config[6], "%s: " ,lang->conf_disc_ps2save_check);
		if(setting->discPs2saveCheck)
			strcat(config[6], lang->conf_on);
		else
			strcat(config[6], lang->conf_off);
		//DISC ELF CHECK
		sprintf(config[7], "%s: " ,lang->conf_disc_elf_check);
		if(setting->discELFCheck)
			strcat(config[7], lang->conf_on);
		else
			strcat(config[7], lang->conf_off);
		//EXPORT DIR
		sprintf(config[8], "%s: %s", lang->conf_export_dir, setting->Exportdir);
		//INIT
		strcpy(config[9], lang->conf_miscsettinginit);
		nList=10;

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "CONFIG/%s", lang->conf_setting_misc);
		// 操作説明
		if(sel==0)
			sprintf(msg1, "○:%s", lang->gen_ok);
		else if(sel==LANG)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==TIMEOUT)
			sprintf(msg1, "○:%s ×:%s", lang->conf_add, lang->conf_away);
		else if(sel==DISCCONTROL)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==FILENAME)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==FILEICON)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==PS2SAVECHECK)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==ELFCHECK)
			sprintf(msg1, "○:%s", lang->conf_change);
		else if(sel==EXPORTDIR)
			sprintf(msg1, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
		else if(sel==MISCINIT)
			sprintf(msg1, "○:%s", lang->gen_ok);
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
					InitFontAscii(setting->AsciiFont);
					InitFontKnaji(setting->KanjiFont);
					SetFontMargin(CHAR_MARGIN, setting->CharMargin);
					SetFontMargin(LINE_MARGIN, setting->LineMargin);
					SetFontBold(setting->FontBold);
					SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
					SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
					SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
					SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);
					screen_env.screen.x = setting->screen_x;
					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = setting->interlace;
					screen_env.ffmode = setting->ffmode;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					//アルファブレンド
					itoSetAlphaBlending(
						ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
						ITO_ALPHA_COLOR_DST, // B = COLOR DEST
						ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
						ITO_ALPHA_COLOR_DST, // C = COLOR DEST
						0x80);				 // Fixed Value
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

		// ファイルリスト表示用変数の正規化
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);

		// ファイルリスト
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
			//ファイルリスト表示
			printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
			y += FONT_HEIGHT;
		}

		// スクロールバー
		if(nList > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nList);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nList);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
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
