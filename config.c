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
	DEF_DISCELFCHECK = FALSE
};

enum{	
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
	COLORINIT,

	IPADDRESS=0,
	NETMASK,
	GATEWAY,
	NETWORKSAVE,
	NETWORKINIT,

	TIMEOUT=0,
	DISCCONTROL,
	FILENAME,
	FILEICON,
	PS2SAVECHECK,
	ELFCHECK,
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
// 設定を初期化
void InitConfig(void)
{
	int i;

	for(i=0; i<12; i++) setting->dirElf[i][0] = 0;

	strcpy(setting->dirElf[1], "MISC/FileBrowser");
	setting->timeout = DEF_TIMEOUT;
	setting->filename = DEF_FILENAME;
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
	setting->discControl = DEF_DISCCONTROL;
	setting->flickerControl = DEF_FLICKERCONTROL;
	setting->fileicon = DEF_FILEICON;
	setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
	setting->discELFCheck = DEF_DISCELFCHECK;
}

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF に設定を保存する
void saveConfig(char *mainMsg)
{
	int i, ret, fd, size;
	const char LF[3]={0x0D, 0x0A, 0};
	char c[MAX_PATH], tmp[30][MAX_PATH];
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
	
	p = (char*)malloc(sizeof(SETTING));
	p[0]=0;
	size=0;
	for(i=0; i<30; i++){
		strcpy(c, tmp[i]);
		strcat(c, LF);
		strcpy(p+size, c);
		size += strlen(c);
	}
	// LaunchELFのディレクトリにCNFがあったらLaunchELFのディレクトリにセーブ
	strcpy(c, LaunchElfDir);
	strcat(c, "LbF.CNF");
	if((fd=fioOpen(c, O_RDONLY)) >= 0)
		fioClose(fd);
	else{
		if(!strncmp(LaunchElfDir, "mc", 2))
			sprintf(c, "mc%d:/SYS-CONF", LaunchElfDir[2]-'0');
		else
			sprintf(c, "mc%d:/SYS-CONF", CheckMC());
		// SYS-CONFがあったらSYS-CONFにセーブ
		if((fd=fioDopen(c)) >= 0){
			fioDclose(fd);
			strcat(c, "/LbF.CNF");
		// SYS-CONFがなかったらLaunchELFのディレクトリにセーブ
		}else{
			strcpy(c, LaunchElfDir);
			strcat(c, "LbF.CNF");
		}
	}
	strcpy(mainMsg,"Save Failed");
	// 書き込み
	if((fd=fioOpen(c,O_CREAT|O_WRONLY|O_TRUNC)) < 0){
		return;
	}
	ret = fioWrite(fd,p,size);
	if(ret==size) sprintf(mainMsg, "Save Config (%s)", c);
	fioClose(fd);
	free(p);
}

////////////////////////////////////////////////////////////////////////
// LAUNCHELF.CNF から設定を読み込む
void loadConfig(char *mainMsg)
{
	int i, j, k, fd, len, mcport;
	size_t size;
	char path[MAX_PATH], tmp[30][MAX_PATH], *p;
	char cnf_version[16];
	
	setting = (SETTING*)malloc(sizeof(SETTING));
	// LaunchELFが実行されたパスから設定ファイルを開く
	sprintf(path, "%s%s", LaunchElfDir, "LbF.CNF");
	if(!strncmp(path, "cdrom", 5)) strcat(path, ";1");
	fd = fioOpen(path, O_RDONLY);
	// 開けなかったら、SYS-CONFの設定ファイルを開く
	if(fd<0){
		if(!strncmp(LaunchElfDir, "mc", 2))
			mcport = LaunchElfDir[2]-'0';
		else
			mcport = CheckMC();
		if(mcport==1 || mcport==0){
			sprintf(path, "mc%d:/SYS-CONF/LbF.CNF", mcport);
			fd = fioOpen(path, O_RDONLY);
		}
	}
	// どのファイルも開けなかった場合、設定を初期化する
	if(fd<0){
		InitConfig();
		mainMsg[0] = 0;
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
		// 27行目はdiscPs2saveCheck
		// 28行目はdiscELFCheck
		for(i=j=k=0; i<size; i++){
			if(p[i]==0x0D && p[i+1]==0x0A){
				if(i-k<MAX_PATH){
					p[i]=0;
					strcpy(tmp[j++], &p[k]);
				}
				else
					break;
				if(j>=30)
					break;
				k=i+2;
			}
		}
		while(j<30)
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

			//
			sprintf(mainMsg, "Load Config (%s)", path);
		}
		else{
			//CNF初期化
			InitConfig();
			sprintf(mainMsg, "Initialize Config (%s)", path);
		}

		free(p);
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
					if(s==OK-1) s=COLORINIT;
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
					if(s==BUTTONINIT+1) s=OK;	//OKまで移動
				}
				else if(page==SCREENSETTING){
					if(s==COLORINIT+1) s=OK;		//OKまで移動
				}
				else if(page==NETWORK){
					if(s==NETWORKINIT+1) s=OK;		//OKまで移動
				}
				else if(page==MISC){
					if(s==MISCINIT+1) s=OK;		//OKまで移動
				}
			}
			else if(new_pad & PAD_LEFT){	//左
				if(page==BUTTONSETTING){
					s=DEFAULT;	//DEFAULTまで移動
				}
				else if(page==SCREENSETTING){
					if(s<8){
						s_x--;
						if(s_x<0) s_x=2;
					}
					else
						s=COLOR1;		//COLOR1まで移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;		//IP ADDRESSまで移動
				}
				else if(page==MISC){
					s=TIMEOUT;		//TIMEOUTまで移動
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
						if(s == 0) itoSetBgColor(setting->color[0]);
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
				}
			}
			else if(new_pad & PAD_CIRCLE){	//○
				if(page==BUTTONSETTING){
					if(s<BUTTONINIT){
						getFilePath(setting->dirElf[s], TRUE);
						if(!strncmp(setting->dirElf[s], "mc", 2)){
							sprintf(c, "mc%s", &setting->dirElf[s][3]);
							strcpy(setting->dirElf[s], c);
						}
					}
					else if(s==BUTTONINIT){
						for(i=0; i<12; i++) setting->dirElf[i][0] = 0;
						strcpy(setting->dirElf[1], "MISC/FileBrowser");
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
						if(s == 0) itoSetBgColor(setting->color[0]);
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
					else if(s==COLORINIT){	//COLOR SETTING INIT
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
						//
						screen_env.screen.x = setting->screen_x;
						screen_env.screen.y = setting->screen_y;
						screen_env.interlace = ITO_INTERLACE;
						itoGsReset();
						itoGsEnvSubmit(&screen_env);
						itoSetBgColor(setting->color[0]);
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
						// 書き込み
						if((fd=fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT",O_CREAT|O_WRONLY|O_TRUNC)) >= 0){
							fioWrite(fd, tmp, strlen(tmp));
							fioClose(fd);
						}
					}
					if(s==NETWORKINIT){
						//init
						strcpy(ip, "192.168.0.10");
						strcpy(netmask, "255.255.255.0");
						strcpy(gw, "192.168.0.1");
					}
				}
				else if(page==MISC){
					if(s==TIMEOUT)
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
					else if(s==MISCINIT){
						setting->timeout = DEF_TIMEOUT;
						setting->filename = DEF_FILENAME;
						setting->discControl = DEF_DISCCONTROL;
						setting->fileicon = DEF_FILEICON;
						setting->discPs2saveCheck = DEF_DISCPS2SAVECHECK;
						setting->discELFCheck = DEF_DISCELFCHECK;
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
					screen_env.screen.x = setting->screen_x;
					screen_env.screen.y = setting->screen_y;
					screen_env.interlace = ITO_INTERLACE;	//setting->interlace;
					itoGsReset();
					itoGsEnvSubmit(&screen_env);
					itoSetBgColor(setting->color[0]);
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
					s=DEFAULT;	//DEFAULTまで移動
				}
				else if(page==SCREENSETTING){
					s=COLOR1;		//COLOR1まで移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;		//IPADDRESSまで移動
				}
				else if(page==MISC){
					s=TIMEOUT;		//TIMEOUTまで移動
				}
			}
			else if(new_pad & PAD_R1){
				page++;
				s_x=0;
				if(page>MISC) page=BUTTONSETTING;
				if(page==BUTTONSETTING){
					s=DEFAULT;	//DEFAULTまで移動
				}
				else if(page==SCREENSETTING){
					s=COLOR1;		//COLOR1まで移動
				}
				else if(page==NETWORK){
					s=IPADDRESS;		//IPADDRESSまで移動
				}
				else if(page==MISC){
					s=TIMEOUT;		//TIMEOUTまで移動
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

		//ページカーソルの枠
		drawFrame(FONT_WIDTH*1.5, SCREEN_MARGIN+FONT_HEIGHT,
			FONT_WIDTH*62.5, SCREEN_MARGIN+FONT_HEIGHT*2,
			setting->color[1]);

		//ページカーソル
		itoPrimAlphaBlending( TRUE );	//アルファブレンド有効
		itoSprite(setting->color[2]|0x10000000,
			FONT_WIDTH*10+FONT_WIDTH*11*page, SCREEN_MARGIN+FONT_HEIGHT+1,
			FONT_WIDTH*10+FONT_WIDTH*11+FONT_WIDTH*11*page, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
		itoPrimAlphaBlending(FALSE);	//アルファブレンド無効
		//
		printXY("<L1     BUTTON     SCREEN     NETWORK     MISC       R1>",
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
				printXY(c, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			printXY("BUTTON SETTING INIT", x, y, setting->color[3], TRUE);
		}
		//COLOR SETTING
		if(page==SCREENSETTING){
			for(i=0;i<8;i++){
				r = setting->color[i] & 0xFF;
				g = setting->color[i] >> 8 & 0xFF;
				b = setting->color[i] >> 16 & 0xFF;
				switch(i){
				case 0:
					sprintf(c, "BACK GROUND   :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 1:
					sprintf(c, "FRAME         :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 2:
					sprintf(c, "HIGHLIGHT TEXT:   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 3:
					sprintf(c, "NORMAL TEXT   :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 4:
					sprintf(c, "FOLDER        :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 5:
					sprintf(c, "FILE          :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 6:
					sprintf(c, "PS2 SAVE      :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				case 7:
					sprintf(c, "ELF FILE      :   R:%3d   G:%3d   B:%3d", r, g, b);
					break;
				}
				printXY(c, x, y, setting->color[3], TRUE);

				sprintf(c, "■");	//色のプレビュー
				printXY(c, x+FONT_WIDTH*42, y, setting->color[i], TRUE);
				y += FONT_HEIGHT;
			}
			sprintf(c, "SCREEN X: %3d", setting->screen_x );
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			sprintf(c, "SCREEN Y: %3d", setting->screen_y );
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			if(setting->flickerControl)
				sprintf(c, "FLICKER CONTROL: ON");
			else
				sprintf(c, "FLICKER CONTROL: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			printXY("COLOR SETTING INIT", x, y, setting->color[3], TRUE);
		}
		//NETWORK
		if(page==NETWORK){
			sprintf(c, "IP ADDRESS: %s", ip);
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			sprintf(c, "NETMASK   : %s", netmask);
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			sprintf(c, "GATEWAY   : %s", gw);
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			printXY("SAVE", x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;

			printXY("NETWORK SETTING INIT", x, y, setting->color[3], TRUE);
		}
		//MISC
		if(page==MISC){
			sprintf(c, "TIMEOUT: %d", setting->timeout);
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			
			if(setting->discControl)
				sprintf(c, "DISC CONTROL: ON");
			else
				sprintf(c, "DISC CONTROL: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			
			if(setting->filename)
				sprintf(c, "PRINT ONLY FILENAME: ON");
			else
				sprintf(c, "PRINT ONLY FILENAME: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			
			if(setting->fileicon)
				sprintf(c, "FILEICON: ON");
			else
				sprintf(c, "FILEICON: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//
			if(setting->discPs2saveCheck)
				sprintf(c, "DISC PS2SAVE CHECK: ON");
			else
				sprintf(c, "DISC PS2SAVE CHECK: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			//
			if(setting->discELFCheck)
				sprintf(c, "DISC ELF CHECK: ON");
			else
				sprintf(c, "DISC ELF CHECK: OFF");
			printXY(c, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			printXY("MISC INIT", x, y, setting->color[3], TRUE);
		}

		//OKとCANCEL
		x = FONT_WIDTH*5;
		y = SCREEN_MARGIN+FONT_HEIGHT*17;
		printXY("OK", x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		printXY("CANCEL", x, y, setting->color[3], TRUE);

		//カーソル
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3+s*FONT_HEIGHT;
		if(page==SCREENSETTING)
			if(s<8) x = FONT_WIDTH*22 + FONT_WIDTH*s_x*8;
		drawChar('>', x, y, setting->color[3]);

		//操作説明
		if(page==BUTTONSETTING){
			if (s < BUTTONINIT)
				sprintf(c, "○:Edit ×:Clear");
			else
				sprintf(c, "○:OK");
		}
		if(page==SCREENSETTING){
			if(s<FLICKERCONTROL)
				sprintf(c, "○:Add ×:Away");
			else if(s==FLICKERCONTROL)
				sprintf(c, "○:Change");
			else
				sprintf(c, "○:OK");
		}
		if(page==NETWORK){
			if(s==IPADDRESS)
				sprintf(c, "○:Edit");
			else if(s==NETMASK)
				sprintf(c, "○:Edit");
			else if(s==GATEWAY)
				sprintf(c, "○:Edit");
			else
				sprintf(c, "○:OK");
		}
		if(page==MISC){
			if(s==TIMEOUT)
				sprintf(c, "○:Add ×:Away");
			else if(s==FILENAME)
				sprintf(c, "○:Change");
			else if(s==DISCCONTROL)
				sprintf(c, "○:Change");
			else if(s==FILEICON)
				sprintf(c, "○:Change");
			else if(s==PS2SAVECHECK)
				sprintf(c, "○:Change");
			else if(s==ELFCHECK)
				sprintf(c, "○:Change");
			else
				sprintf(c, "○:OK");
		}
		setScrTmp("", c);
		drawScr();
	}
	
	return;
}
