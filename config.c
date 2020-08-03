#include "launchelf.h"

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
	DEF_DISCCONTROL = TRUE,
	DEF_FLICKERCONTROL = TRUE,
	DEF_FILEICON = TRUE,
	DEF_DISCPS2SAVECHECK = FALSE,
	DEF_DISCELFCHECK = FALSE,
	DEF_LANGUAGE = LANG_ENGLISH
};

enum
{
	BUTTONSETTING=0,
	SCREENSETTING,
	NETWORK,
	MISC,
	OK=14,
	CANCEL,

	DEFAULT=0,
	BUTTONINIT=12,

	COLOR1=0,
	SCREEN_X=8,
	SCREEN_Y,
	FLICKERCONTROL,
	SCREENINIT,

	IPADDRESS=0,
	NETMASK,
	GATEWAY,
	NETWORKSAVE,
	NETWORKINIT,

	LANG=0,
	TIMEOUT,
	DISCCONTROL,
	FILENAME,
	FILEICON,
	PS2SAVECHECK,
	ELFCHECK,
	EXPORTDIR,
	MISCINIT
};

SETTING *setting, *tmpsetting;

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// BUTTON SETTINGを初期化
void InitButtonSetting(SETTING *setting)
{
	int i;

	for(i=0; i<12; i++) setting->dirElf[i][0] = 0;

	strcpy(setting->dirElf[1], "MISC/FileBrowser");
}

////////////////////////////////////////////////////////////////////////
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
}

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// 設定を初期化
void InitSetting(SETTING *setting)
{
	InitButtonSetting(setting);
	InitScreenSetting(setting);
	InitMiscSetting(setting);
}

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF に設定を保存する
void saveConfig(char *mainMsg)
{
	int i, ret, fd, size;
	const char LF[3]={0x0D, 0x0A, 0};
	char c[MAX_PATH], tmp[32][MAX_PATH];
	char* p;
	
	// 設定をメモリに格納
	strcpy(tmp[0], "CNF_VERSION_1");
	for(i=0; i<12; i++)
		strcpy(tmp[i+1], setting->dirElf[i]);
	sprintf(tmp[13], "%d", setting->timeout);
	sprintf(tmp[14], "%d", setting->filename);
	for(i=0; i<8; i++)
		sprintf(tmp[15+i], "%lu", setting->color[i]);
	sprintf(tmp[23], "%d", setting->screen_x);
	sprintf(tmp[24], "%d", setting->screen_y);
	sprintf(tmp[25], "%d", setting->discControl);
	sprintf(tmp[26], "%d", setting->flickerControl);
	sprintf(tmp[27], "%d", setting->fileicon);
	sprintf(tmp[28], "%d", setting->discPs2saveCheck);
	sprintf(tmp[29], "%d", setting->discELFCheck);
	strcpy(tmp[30], setting->Exportdir);
	sprintf(tmp[31], "%d", setting->language);
	
	p = (char*)malloc(sizeof(SETTING));
	p[0]=0;
	size=0;
	for(i=0; i<32; i++){
		strcpy(c, tmp[i]);
		strcat(c, LF);
		strcpy(p+size, c);
		size += strlen(c);
	}

	//hostから起動しているか調べる
	if(!strncmp(LaunchElfDir, "host", 4)){
		//有効なmcのSYS-CONFフォルダ
		sprintf(c, "mc%d:/SYS-CONF", CheckMC());

		// SYS-CONFフォルダをオープンしてみる
		if((fd=fioDopen(c)) >= 0){
			// SYS-CONFフォルダがあったらSYS-CONFにセーブ
			fioDclose(fd);
			strcat(c, "/LBF.CNF");
		}
		else
			// SYS-CONFフォルダが無いときは保存しない
			c[0]=0;
	}
	else{ 
		//CNFファイルのパス
		strcpy(c, LaunchElfDir);
		strcat(c, "LBF.CNF");

		// LaunchELFのディレクトリにCNFがあったらLaunchELFのディレクトリにセーブ
		if((fd=fioOpen(c, O_RDONLY)) >= 0)
			fioClose(fd);
		// LaunchELFのディレクトリにCNFが無いとき
		else{
			//mcから起動しているか調べる
			if(!strncmp(LaunchElfDir, "mc", 2))
				//mcのとき
				sprintf(c, "mc%d:/SYS-CONF", LaunchElfDir[2]-'0');
			else
				//mc以外のときは有効なmcのSYS-CONFフォルダ
				sprintf(c, "mc%d:/SYS-CONF", CheckMC());
	
			// SYS-CONFフォルダをオープンしてみる
			if((fd=fioDopen(c)) >= 0){
				// SYS-CONFフォルダがあったらSYS-CONFにセーブ
				fioDclose(fd);
				strcat(c, "/LBF.CNF");
			}
			else{
				// SYS-CONFフォルダがなかったらLaunchELFのディレクトリにセーブ
				strcpy(c, LaunchElfDir);
				strcat(c, "LBF.CNF");
			}
		}
	}
	strcpy(mainMsg,lang->conf_savefailed);

	// 書き込み
	if((fd=fioOpen(c,O_CREAT|O_WRONLY|O_TRUNC)) < 0){
		return;
	}
	ret = fioWrite(fd,p,size);
	if(ret==size) sprintf(mainMsg, "%s (%s)", lang->conf_saveconfig, c);
	fioClose(fd);
	free(p);
}

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF から設定を読み込む
void loadConfig(char *mainMsg)
{
	int i, j, k, fd, len, mcport;
	size_t size;
	char path[MAX_PATH], tmp[31][MAX_PATH], *p;
	char cnf_version[16];
	int ret;
	
	setting = (SETTING*)malloc(sizeof(SETTING));

	//hostから起動しているか調べる
	if(!strncmp(LaunchElfDir, "host", 4)){
		//有効なmcのSYS-CONFフォルダ
		sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", CheckMC());
		fd = fioOpen(path, O_RDONLY);
	}
	else{
		// LaunchELFが実行されたパスから設定ファイルを開く
		sprintf(path, "%s%s", LaunchElfDir, "LBF.CNF");
		if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
		fd = fioOpen(path, O_RDONLY);
		// 開けなかったら、SYS-CONFの設定ファイルを開く
		if(fd<0){
			//mcから起動しているか調べる
			if(!strncmp(LaunchElfDir, "mc", 2))
				//mcのとき
				mcport = LaunchElfDir[2]-'0';
			else
				//mc以外のときは有効なmcのSYS-CONFフォルダ
				mcport = CheckMC();
			if(mcport==1 || mcport==0){
				sprintf(path, "mc%d:/SYS-CONF/LBF.CNF", mcport);
				fd = fioOpen(path, O_RDONLY);
			}
		}
	}
	// どのファイルも開けなかった場合、設定を初期化する
	if(fd<0){
		InitSetting(setting);
		mainMsg[0] = 0;
		ret=0;
	}
	else{
		// 設定ファイルをメモリに読み込み
		size = fioLseek(fd, 0, SEEK_END);
		printf("size=%d\n", size);
		fioLseek(fd, 0, SEEK_SET);
		p = (char*)malloc(sizeof(size));
		fioRead(fd, p, size);
		fioClose(fd);
		
		// 計28行のテキストを読み込む
		// 1行目はCNF_VERSION_1
		// 2行目から13行目まではボタンセッティング
		// 14行目は TIMEOUT 値
		// 15行目は PRINT ONLY FILENAME の設定値
		// 16行目から23行目まではカラーセッティング
		// 24行目はスクリーンX
		// 25行目はスクリーンY
		// 26行目はディスクコントロール
		// 27行目はインターレース
		// 28行目はアイコン表示
		// 29行目はdiscPs2saveCheck
		// 30行目はdiscELFCheck
		// 31行目はExportdir
		// 32行目はlanguage
		for(i=j=k=0; i<size; i++){
			if(p[i]==0x0D && p[i+1]==0x0A){
				if(i-k<MAX_PATH){
					p[i]=0;
					strcpy(tmp[j++], &p[k]);
				}
				else
					break;
				if(j>=32)
					break;
				k=i+2;
			}
		}
		while(j<32)
			tmp[j++][0] = 0;

		//cnf version check
		//CNFファイルの互換
		//LbFv0.30まで、LaunchELFのCNFファイルと同じ
		//LbFv0.40から、互換性がありません
		strcpy(cnf_version, tmp[0]);
		if(!strcmp(cnf_version, "CNF_VERSION_1")){
			// ボタンセッティング
			for(i=0; i<12; i++)
				strcpy(setting->dirElf[i], tmp[i+1]);
			// TIMEOUT値の設定
			if(tmp[13][0]){
				setting->timeout = 0;
				len = strlen(tmp[13]);
				i = 1;
				while(len-- != 0){
					setting->timeout += (tmp[13][len]-'0') * i;
					i *= 10;
				}
			}
			else
				setting->timeout = DEF_TIMEOUT;
			// PRINT ONLY FILENAME の設定
			if(tmp[14][0])
				setting->filename = tmp[14][0]-'0';
			else
				setting->filename = DEF_FILENAME;
			// カラー1から4の設定
			if(tmp[15][0]){
				for(i=0; i<8; i++){
					setting->color[i] = 0;
					len = strlen(tmp[15+i]);
					j = 1;
					while(len-- != 0){
						setting->color[i] += (tmp[15+i][len]-'0') * j;
						j *= 10;
					}
				}
			}
			else{
				setting->color[0] = DEF_COLOR1;
				setting->color[1] = DEF_COLOR2;
				setting->color[2] = DEF_COLOR3;
				setting->color[3] = DEF_COLOR4;
				setting->color[4] = DEF_COLOR5;
				setting->color[5] = DEF_COLOR6;
				setting->color[6] = DEF_COLOR7;
				setting->color[7] = DEF_COLOR8;
			}
			// スクリーンXの設定
			if(tmp[23][0]){
				setting->screen_x = 0;
				j = strlen(tmp[23]);
				for(i=1; j; i*=10)
					setting->screen_x += (tmp[23][--j]-'0')*i;
			}
			else
				setting->screen_x = DEF_SCREEN_X;
			// スクリーンYの設定
			if(tmp[24][0]){
				setting->screen_y = 0;
				j = strlen(tmp[24]);
				for(i=1; j; i*=10)
					setting->screen_y += (tmp[24][--j]-'0')*i;
			}
			else
				setting->screen_y = DEF_SCREEN_Y;
			// ディスクコントロールの設定
			if(tmp[25][0])
				setting->discControl = tmp[25][0]-'0';
			else
				setting->discControl = DEF_DISCCONTROL;
			// インターレースの設定
			if(tmp[26][0])
				setting->flickerControl = tmp[26][0]-'0';
			else
				setting->flickerControl = DEF_FLICKERCONTROL;
			// アイコン表示の設定
			if(tmp[27][0])
				setting->fileicon = tmp[27][0]-'0';
			else
				setting->fileicon = DEF_FILEICON;
			// discPs2saveCheck
			if(tmp[28][0])
				setting->discPs2saveCheck = tmp[28][0]-'0';
			else
				setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
			// discELFCheck
			if(tmp[29][0])
				setting->discELFCheck = tmp[29][0]-'0';
			else
				setting->discELFCheck = DEF_DISCELFCHECK;
			strcpy(setting->Exportdir, tmp[30]);
			// language
			if(tmp[31][0])
				setting->language = tmp[31][0]-'0';
			else
				setting->language = DEF_LANGUAGE;

			ret=1;
		}
		else{
			//Setting初期化
			InitSetting(setting);
			ret=2;
		}

		free(p);
	}

	SetLanguage(setting->language);
	if(ret==0){
		//設定ファイル開けなかった
	}
	else if(ret==1){
		//ロード成功
		sprintf(mainMsg, "%s (%s)", lang->conf_loadconfig, path);
	}
	else if(ret==2){
		//CNFのバージョンが古い
		sprintf(mainMsg, "%s (%s)", lang->conf_initializeconfig, path);
	}
	return;
}

////////////////////////////////////////////////////////////////////////
// Config画面
void config(char *mainMsg)
{
	char c[MAX_PATH];
	int i;
	int s;	//select_y
	int x, y;
	int page=BUTTONSETTING;
	int r,g,b;
	int s_x=0;	//select_x

	int fd;
	extern char ip[16];
	extern char netmask[16];
	extern char gw[16];
	char tmpip[16];
	char tmpnetmask[16];
	char tmpgw[16];
	char tmp[16*3];

	tmpsetting = setting;
	setting = (SETTING*)malloc(sizeof(SETTING));
	*setting = *tmpsetting;

	strcpy(tmpip,ip);
	strcpy(tmpnetmask,netmask);
	strcpy(tmpgw,gw);

	s=0;
	while(1)
	{
		// 操作
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP){	//上
				s--;
				if(s<0) s=CANCEL;
				if(page==BUTTONSETTING){
					if(s==OK-1) s=BUTTONINIT;
				}
				else if(page==SCREENSETTING){
					if(s==OK-1) s=SCREENINIT;
				}
				else if(page==NETWORK){
					if(s==OK-1) s=NETWORKINIT;
				}
				else if(page==MISC){
					if(s==OK-1) s=MISCINIT;
				}
			}
			else if(new_pad & PAD_DOWN){	//下
				s++;
				if(s>CANCEL) s=0;
				if(page==BUTTONSETTING){
					if(s==BUTTONINIT+1) s=OK;	//OKに移動
				}
				else if(page==SCREENSETTING){
					if(s==SCREENINIT+1) s=OK;	//OKに移動
				}
				else if(page==NETWORK){
					if(s==NETWORKINIT+1) s=OK;	//OKに移動
				}
				else if(page==MISC){
					if(s==MISCINIT+1) s=OK;	//OKに移動
				}
			}
			else if(new_pad & PAD_LEFT){	//左
				if(page==BUTTONSETTING){
					s=DEFAULT;	//DEFAULTに移動
				}
				else if(page==SCREENSETTING){
					if(s<8){
						s_x--;
						if(s_x<0) s_x=2;
					}
					else
						s=COLOR1;		//COLOR1に移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;		//IP ADDRESSに移動
				}
				else if(page==MISC){
					s=LANG;		//LANGに移動
				}
			}
			else if(new_pad & PAD_RIGHT){	//右
				if(page==SCREENSETTING){
					if(s<8){
						s_x++;
						if(s_x>2) s_x=0;
					}
					else
						s=OK;	//OK
				}
				else
					s=OK;	//OK
			}
			else if(new_pad & PAD_CROSS){	//×
				if(page==BUTTONSETTING){
					if(s<12)
					setting->dirElf[s][0]=0;
				}
				else if(page==SCREENSETTING){
					if(s<8){
						r = setting->color[s] & 0xFF;
						g = setting->color[s] >> 8 & 0xFF;
						b = setting->color[s] >> 16 & 0xFF;
						if(s_x==0){
							if(r>0) r--;
						}
						if(s_x==1){
							if(g>0) g--;
						}
						if(s_x==2){
							if(b>0) b--;
						}
						setting->color[s] = ITO_RGBA(r, g, b, 0);
						//if(s == 0) itoSetBgColor(setting->color[0]);
					}
					else if(s==SCREEN_X){	//SCREEN X
						if(setting->screen_x > 0){
							setting->screen_x--;
							screen_env.screen.x = setting->screen_x;
							itoSetScreenPos(setting->screen_x, setting->screen_y);
						}
					}
					else if(s==SCREEN_Y){	//SCREEN Y
						if(setting->screen_y > 0){
							setting->screen_y--;
							screen_env.screen.y = setting->screen_y;
							itoSetScreenPos(setting->screen_x, setting->screen_y);
						}
					}
				}
				else if(page==MISC){
					if(s==TIMEOUT){
						if(setting->timeout > 0) setting->timeout--;
					}
					if(s==EXPORTDIR){
						setting->Exportdir[0]=0;
					}
				}
			}
			else if(new_pad & PAD_CIRCLE){	//○
				if(page==BUTTONSETTING){
					if(s<BUTTONINIT){
						getFilePath(setting->dirElf[s], ELF_FILE);
						if(!strncmp(setting->dirElf[s], "mc", 2)){
							sprintf(c, "mc%s", &setting->dirElf[s][3]);
							strcpy(setting->dirElf[s], c);
						}
					}
					else if(s==BUTTONINIT){
						InitButtonSetting(setting);
					}
				}
				else if(page==SCREENSETTING){
					if(s<8){
						r = setting->color[s] & 0xFF;
						g = setting->color[s] >> 8 & 0xFF;
						b = setting->color[s] >> 16 & 0xFF;
						if(s_x==0){
							if(r<255) r++;
						}
						else if(s_x==1){
							if(g<255) g++;
						}
						else if(s_x==2){
							if(b<255) b++;
						}
						setting->color[s] = ITO_RGBA(r, g, b, 0);
						//if(s == 0) itoSetBgColor(setting->color[0]);
					}
					else if(s==SCREEN_X){	//SCREEN X
						setting->screen_x++;
						screen_env.screen.x = setting->screen_x;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
					else if(s==SCREEN_Y){	//SCREEN Y
						setting->screen_y++;
						screen_env.screen.y = setting->screen_y;
						itoSetScreenPos(setting->screen_x, setting->screen_y);
					}
					else if(s==FLICKERCONTROL)	//フリッカーコントロール
						setting->flickerControl = !setting->flickerControl;
					else if(s==SCREENINIT){	//SCREEN SETTING INIT
						//init
						InitScreenSetting(setting);

						screen_env.screen.x = setting->screen_x;
						screen_env.screen.y = setting->screen_y;
						screen_env.interlace = ITO_INTERLACE;
						itoGsReset();
						itoGsEnvSubmit(&screen_env);
						//itoSetBgColor(setting->color[0]);
						//アルファブレンド
						itoSetAlphaBlending(
							ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
							ITO_ALPHA_COLOR_DST, // B = COLOR DEST
							ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
							ITO_ALPHA_COLOR_DST, // C = COLOR DEST
							0x80);				 // Fixed Value
					}
				}
				else if(page==NETWORK){
					if(s==IPADDRESS){
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						strcpy(tmp,ip);
						if(keyboard(tmp, 15)>=0) strcpy(ip,tmp);
					}
					if(s==NETMASK){
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						strcpy(tmp,netmask);
						if(keyboard(tmp, 15)>=0) strcpy(netmask,tmp);
					}
					if(s==GATEWAY){
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						strcpy(tmp,gw);
						if(keyboard(tmp, 15)>=0) strcpy(gw,tmp);
					}
					if(s==NETWORKSAVE){
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
					if(s==NETWORKINIT){
						//init
						strcpy(ip, "192.168.0.10");
						strcpy(netmask, "255.255.255.0");
						strcpy(gw, "192.168.0.1");
					}
				}
				else if(page==MISC){
					if(s==LANG){
						setting->language++;
						if(setting->language==NUM_LANG) setting->language=LANG_ENGLISH;
						SetLanguage(setting->language);
					}
					else if(s==TIMEOUT)
						setting->timeout++;
					else if(s==FILENAME)
						setting->filename = !setting->filename;
					else if(s==DISCCONTROL)
						setting->discControl = !setting->discControl;
					else if(s==FILEICON)
						setting->fileicon = !setting->fileicon;
					else if(s==PS2SAVECHECK)
							setting->discPs2saveCheck = !setting->discPs2saveCheck;
					else if(s==ELFCHECK)
							setting->discELFCheck = !setting->discELFCheck;
					else if(s==EXPORTDIR)
						getFilePath(setting->Exportdir, DIR);
					else if(s==MISCINIT){
						//init
						InitMiscSetting(setting);
						SetLanguage(setting->language);
					}
				}
				//
				if(s==OK){
					free(tmpsetting);
					saveConfig(mainMsg);
					break;
				}
				if(s==CANCEL){
					free(setting);
					setting = tmpsetting;
					strcpy(ip,tmpip);
					strcpy(netmask,tmpnetmask);
					strcpy(gw,tmpgw);
					SetLanguage(setting->language);
					screen_env.screen.x = setting->screen_x;
					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = ITO_INTERLACE;	//setting->interlace;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					//itoSetBgColor(setting->color[0]);
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
			else if(new_pad & PAD_L1){
				page--;
				s_x=0;
				if(page<BUTTONSETTING) page=MISC;
				if(page==BUTTONSETTING){
					s=DEFAULT;	//DEFAULTに移動
				}
				else if(page==SCREENSETTING){
					s=COLOR1;	//COLOR1に移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;	//IPADDRESSに移動
				}
				else if(page==MISC){
					s=LANG;		//LANGに移動
				}
			}
			else if(new_pad & PAD_R1){
				page++;
				s_x=0;
				if(page>MISC) page=BUTTONSETTING;
				if(page==BUTTONSETTING){
					s=DEFAULT;	//DEFAULTに移動
				}
				else if(page==SCREENSETTING){
					s=COLOR1;	//COLOR1に移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;	//IPADDRESSに移動
				}
				else if(page==MISC){
					s=LANG;		//TIMEOUTに移動
				}
			}
			else if(new_pad & PAD_SELECT){
				s=CANCEL;	//CANCEL
			}
			else if(new_pad & PAD_START){
				s=OK;	//OK
			}
		}
		
		// 画面描画開始
		clrScr(setting->color[0]);

/*
		//ページカーソルの枠
		drawFrame(FONT_WIDTH*1.5, SCREEN_MARGIN+FONT_HEIGHT,
			FONT_WIDTH*62.5, SCREEN_MARGIN+FONT_HEIGHT*2,
			setting->color[1]);
*/

		//ページカーソル
		itoPrimAlphaBlending( TRUE );	//アルファブレンド有効
		itoSprite(setting->color[2]|0x10000000,
			FONT_WIDTH*8+FONT_WIDTH*12*page, SCREEN_MARGIN+FONT_HEIGHT+1,
			FONT_WIDTH*8+FONT_WIDTH*12+FONT_WIDTH*12*page, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
		itoPrimAlphaBlending(FALSE);	//アルファブレンド無効
		//
		printXY(lang->conf_menu,
			FONT_WIDTH*4, SCREEN_MARGIN+FONT_HEIGHT+2, setting->color[3], TRUE);

		// 枠の中
		x = FONT_WIDTH*5;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;

		//BUTTON SETTING
		if(page==BUTTONSETTING){
			for(i=0; i<12; i++){
				switch(i){
				case 0:
					strcpy(c,"DEFAULT: ");
					break;
				case 1:
					strcpy(c,"○     : ");
					break;
				case 2:
					strcpy(c,"×     : ");
					break;
				case 3:
					strcpy(c,"□     : ");
					break;
				case 4:
					strcpy(c,"△     : ");
					break;
				case 5:
					strcpy(c,"L1     : ");
					break;
				case 6:
					strcpy(c,"R1     : ");
					break;
				case 7:
					strcpy(c,"L2     : ");
					break;
				case 8:
					strcpy(c,"R2     : ");
					break;
				case 9:
					strcpy(c,"L3     : ");
					break;
				case 10:
					strcpy(c,"R3     : ");
					break;
				case 11:
					strcpy(c,"START  : ");
					break;
				}
				strcat(c, setting->dirElf[i]);
				if(s==i)
					printXY(c, x, y, setting->color[2], TRUE);
				else
					printXY(c, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			strcpy(c,lang->conf_buttonsettinginit);
			if(s==i)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
		}
		//SCREEN SETTING
		if(page==SCREENSETTING){
			for(i=0;i<8;i++){
				r = setting->color[i] & 0xFF;
				g = setting->color[i] >> 8 & 0xFF;
				b = setting->color[i] >> 16 & 0xFF;
				switch(i){
				case 0:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_background, r, g, b);
					break;
				case 1:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_frame, r, g, b);
					break;
				case 2:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_highlighttext, r, g, b);
					break;
				case 3:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_normaltext, r, g, b);
					break;
				case 4:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_folder, r, g, b);
					break;
				case 5:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_file, r, g, b);
					break;
				case 6:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_ps2save, r, g, b);
					break;
				case 7:
					sprintf(c, "%s:   R:%3d   G:%3d   B:%3d", lang->conf_elffile, r, g, b);
					break;
				}
				if(s==i)
					printXY(c, x, y, setting->color[2], TRUE);
				else
					printXY(c, x, y, setting->color[3], TRUE);

				//色のプレビュー
				//printXY("■", x+FONT_WIDTH*42, y, setting->color[i], TRUE);
				itoSprite(setting->color[i],
					x+FONT_WIDTH*42, y,
					x+FONT_WIDTH*42+16, y+16, 0);
				y += FONT_HEIGHT;
			}
			//SCREEN_X
			sprintf(c, "%s: %3d", lang->conf_screen_x, setting->screen_x );
			if(s==SCREEN_X)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//SCREEN_Y
			sprintf(c, "%s: %3d", lang->conf_screen_y, setting->screen_y );
			if(s==SCREEN_Y)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//FLICKERCONTROL
			if(setting->flickerControl)
				sprintf(c, "%s: %s", lang->conf_flickercontrol, lang->conf_on);
			else
				sprintf(c, "%s: %s", lang->conf_flickercontrol, lang->conf_off);
			if(s==FLICKERCONTROL)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//SCREENINIT
			strcpy(c,lang->conf_screensettinginit);
			if(s==SCREENINIT)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
		}
		//NETWORK
		if(page==NETWORK){
			//IPADDRESS
			sprintf(c, "%s: %s", lang->conf_ipaddress, ip);
			if(s==IPADDRESS)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//NETMASK
			sprintf(c, "%s: %s", lang->conf_netmask, netmask);
			if(s==NETMASK)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//GATEWAY
			sprintf(c, "%s: %s", lang->conf_gateway, gw);
			if(s==GATEWAY)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//NETWORKSAVE
			strcpy(c,lang->conf_ipoverwrite);
			if(s==NETWORKSAVE)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//NETWORKINIT
			strcpy(c,lang->conf_ipsettinginit);
			if(s==NETWORKINIT)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
		}
		//MISC
		if(page==MISC){
			//LANG
			sprintf(c, "%s: ", lang->conf_language);
			if(setting->language==LANG_ENGLISH)
				strcat(c, lang->conf_language_us);
			else if(setting->language==LANG_JAPANESE)
				strcat(c, lang->conf_language_jp);
			if(s==LANG)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//TIMEOUT
			sprintf(c, "%s: %d", lang->conf_timeout, setting->timeout);
			if(s==TIMEOUT)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//DISC CONTROL
			sprintf(c, "%s: " ,lang->conf_disc_control);
			if(setting->discControl)
				strcat(c, lang->conf_on);
			else
				strcat(c, lang->conf_off);
			if(s==DISCCONTROL)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//PRINT ONLY FILENAME
			sprintf(c, "%s: " ,lang->conf_print_only_filename);
			if(setting->filename)
				strcat(c, lang->conf_on);
			else
				strcat(c, lang->conf_off);
			if(s==FILENAME)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//FILEICON
			sprintf(c, "%s: " ,lang->conf_fileicon);
			if(setting->fileicon)
				strcat(c, lang->conf_on);
			else
				strcat(c, lang->conf_off);
			if(s==FILEICON)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//DISC PS2SAVE CHECK
			sprintf(c, "%s: " ,lang->conf_disc_ps2save_check);
			if(setting->discPs2saveCheck)
				strcat(c, lang->conf_on);
			else
				strcat(c, lang->conf_off);
			if(s==PS2SAVECHECK)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//DISC ELF CHECK
			sprintf(c, "%s: " ,lang->conf_disc_elf_check);
			if(setting->discELFCheck)
				strcat(c, lang->conf_on);
			else
				strcat(c, lang->conf_off);
			if(s==ELFCHECK)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//EXPORT DIR
			sprintf(c, "%s: %s", lang->conf_export_dir, setting->Exportdir);
			if(s==EXPORTDIR)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//
			strcpy(c,lang->conf_miscsettinginit);
			if(s==MISCINIT)
				printXY(c, x, y, setting->color[2], TRUE);
			else
				printXY(c, x, y, setting->color[3], TRUE);
		}

		//OK
		x = FONT_WIDTH*5;
		y = SCREEN_MARGIN+FONT_HEIGHT*17;
		strcpy(c,lang->conf_ok);
		if(s==OK)
			printXY(c, x, y, setting->color[2], TRUE);
		else
			printXY(c, x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		//CANCEL
		strcpy(c,lang->conf_cancel);
		if(s==CANCEL)
			printXY(c, x, y, setting->color[2], TRUE);
		else
			printXY(c, x, y, setting->color[3], TRUE);

		//カーソル
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3+s*FONT_HEIGHT;
		if(page==SCREENSETTING)
			if(s<8) x = FONT_WIDTH*21 + FONT_WIDTH*s_x*8;
		printXY(">", x, y, setting->color[2], TRUE);

		//操作説明
		if(page==BUTTONSETTING){
			if (s < BUTTONINIT)
				sprintf(c, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
			else
				sprintf(c, "○:%s", lang->gen_ok);
		}
		if(page==SCREENSETTING){
			if(s<FLICKERCONTROL)
				sprintf(c, "○:%s ×:%s", lang->conf_add, lang->conf_away);
			else if(s==FLICKERCONTROL)
				sprintf(c, "○:%s", lang->conf_change);
			else
				sprintf(c, "○:%s", lang->gen_ok);
		}
		if(page==NETWORK){
			if(s==IPADDRESS)
				sprintf(c, "○:%s", lang->conf_edit);
			else if(s==NETMASK)
				sprintf(c, "○:%s", lang->conf_edit);
			else if(s==GATEWAY)
				sprintf(c, "○:%s", lang->conf_edit);
			else
				sprintf(c, "○:%s", lang->gen_ok);
		}
		if(page==MISC){
			if(s==LANG)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==TIMEOUT)
				sprintf(c, "○:%s ×:%s", lang->conf_add, lang->conf_away);
			else if(s==FILENAME)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==DISCCONTROL)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==FILEICON)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==PS2SAVECHECK)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==ELFCHECK)
				sprintf(c, "○:%s", lang->conf_change);
			else if(s==EXPORTDIR)
				sprintf(c, "○:%s ×:%s", lang->conf_edit, lang->conf_clear);
			else
				sprintf(c, "○:%s", lang->gen_ok);
		}
		setScrTmp("", c);
		drawScr();
	}
	
	return;
}
