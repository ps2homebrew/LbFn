#include "launchelf.h"

extern u8 *iomanx_irx;
extern int size_iomanx_irx;
extern u8 *filexio_irx;
extern int size_filexio_irx;
extern u8 *ps2dev9_irx;
extern int size_ps2dev9_irx;
extern u8 *ps2atad_irx;
extern int size_ps2atad_irx;
extern u8 *ps2hdd_irx;
extern int size_ps2hdd_irx;
extern u8 *ps2fs_irx;
extern int size_ps2fs_irx;
extern u8 *poweroff_irx;
extern int size_poweroff_irx;
extern u8 *loader_elf;
extern int size_loader_elf;
extern u8 *ps2netfs_irx;
extern int size_ps2netfs_irx;
extern u8 *iopmod_irx;
extern int size_iopmod_irx;
extern u8 *usbd_irx;
extern int size_usbd_irx;
extern u8 *usb_mass_irx;
extern int size_usb_mass_irx;
extern u8 *cdvd_irx;
extern int size_cdvd_irx;

//PS2Net uLaunchELF3.60
extern u8 *ps2ip_irx;
extern int size_ps2ip_irx;
extern u8 *ps2smap_irx;
extern int size_ps2smap_irx;
extern u8 *ps2ftpd_irx;
extern int size_ps2ftpd_irx;

//#define DEBUG
#ifdef DEBUG
#define dbgprintf(args...) scr_printf(args)
#define dbginit_scr() init_scr()
#else
#define dbgprintf(args...) do { } while(0)
#define dbginit_scr() do { } while(0)
#endif

enum
{
	BUTTON,
	DPAD,
	DPAD_MISC
};

int trayopen=FALSE;
int selected=0;
int timeout=0;
int cancel=FALSE;
int mode=BUTTON;
char LaunchElfDir[MAX_PATH], mainMsg[MAX_PATH];

//--------------------------------------------------------------
//PS2Net uLaunchELF3.60
#define IPCONF_MAX_LEN  (3*16)
char if_conf[IPCONF_MAX_LEN];
int if_conf_len;

char ip[16]      = "192.168.0.10";
char netmask[16] = "255.255.255.0";
char gw[16]      = "192.168.0.1";

char netConfig[IPCONF_MAX_LEN+64];	//Adjust size as needed

//--------------------------------------------------------------
// Parse network configuration from IPCONFIG.DAT
// Now completely rewritten to fix some problems
//------------------------------
static void getIpConfig(void)
{
	int fd;
	int i;
	int len;
	char c;
	char buf[IPCONF_MAX_LEN];

	fd = fioOpen("mc0:/SYS-CONF/IPCONFIG.DAT", O_RDONLY);
	if (fd >= 0) 
	{	bzero(buf, IPCONF_MAX_LEN);
		len = fioRead(fd, buf, IPCONF_MAX_LEN - 1); //Save a byte for termination
		fioClose(fd);
	}

	if	((fd > 0) && (len > 0))
	{	buf[len] = '\0'; //Ensure string termination, regardless of file content
		for	(i=0; ((c = buf[i]) != '\0'); i++) //Clear out spaces and any CR/LF
			if	((c == ' ') || (c == '\r') || (c == '\n'))
				buf[i] = '\0';
		strncpy(ip, buf, 15);
		i = strlen(ip)+1;
		strncpy(netmask, buf+i, 15);
		i += strlen(netmask)+1;
		strncpy(gw, buf+i, 15);
	}

	bzero(if_conf, IPCONF_MAX_LEN);
	strncpy(if_conf, ip, 15);
	i = strlen(ip) + 1;
	strncpy(if_conf+i, netmask, 15);
	i += strlen(netmask) + 1;
	strncpy(if_conf+i, gw, 15);
	i += strlen(gw) + 1;
	if_conf_len = i;
	sprintf(netConfig, "Net Config:  %-15s %-15s %-15s", ip, netmask, gw);

}

//--------------------------------------------------------------
// loadModules
void delay(int count)
{
	int i;
	int ret;
	for (i  = 0; i < count; i++) {
	        ret = 0x01000000;
		while(ret--) asm("nop\nnop\nnop\nnop");
	}
}

//--------------------------------------------------------------
void initsbv_patches(void)
{
	static int SbvPatchesInited=FALSE;
	
	if(!SbvPatchesInited){
		dbgprintf("Init MrBrown sbv_patches\n");
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		SbvPatchesInited=TRUE;
	}
}

//--------------------------------------------------------------
void	load_iomanx(void)
{
	int ret;
	static int loaded=FALSE;

	if(!loaded)
	{	SifExecModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_filexio(void)
{
	int ret;
	static int loaded=FALSE;

	if(!loaded)
	{	SifExecModuleBuffer(&filexio_irx, size_filexio_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2dev9(void)
{
	int ret;
	static int loaded=FALSE;

	load_iomanx();
	if(!loaded)
	{	SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2ip(void)
{
	int ret;
	static int loaded=FALSE;

	load_ps2dev9();
	if(!loaded)
	{	SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, &ret);
		SifExecModuleBuffer(&ps2smap_irx, size_ps2smap_irx, if_conf_len, &if_conf[0], &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2atad(void)
{
	int ret;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";
	static int loaded=FALSE;

	load_ps2dev9();
	if(!loaded)
	{	SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, &ret);
		SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &ret);
		SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, sizeof(pfsarg), pfsarg, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2ftpd(void)
{
	int 	ret;
	int		arglen;
	char* arg_p;
	static int loaded=FALSE;

	arg_p = "-anonymous";
	arglen = strlen(arg_p);

	load_ps2ip();
	if(!loaded)
	{	SifExecModuleBuffer(&ps2ftpd_irx, size_ps2ftpd_irx, arglen, arg_p, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2netfs(void)
{
	static int loaded=FALSE;
	int ret;

	load_ps2ip();
	if(!loaded)
	{	SifExecModuleBuffer(&ps2netfs_irx, size_ps2netfs_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void loadModules(void)
{
	dbgprintf("Loading SIO2MAN\n");
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	dbgprintf("Loading MCMAN\n");
	SifLoadModule("rom0:MCMAN", 0, NULL);
	dbgprintf("Loading MCSERV\n");
	SifLoadModule("rom0:MCSERV", 0, NULL);
	dbgprintf("Loading PADMAN\n");
	SifLoadModule("rom0:PADMAN", 0, NULL);
}

//--------------------------------------------------------------
void loadCdModules(void)
{
	static int loaded=FALSE;
	int ret;
	
	if(!loaded){
		initsbv_patches();
		SifExecModuleBuffer(&cdvd_irx, size_cdvd_irx, 0, NULL, &ret);
		cdInit(CDVD_INIT_INIT);
		CDVD_Init();
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
//
int load_irxfile(char *filename)
{
	int fd, size;
	char path[MAX_PATH];
	int ret=-1;

	if(!strncmp(LaunchElfDir, "host", 4)){
		//hostから起動したときHOSTからIRXをロードしない
	}
	else{
		sprintf(path, "%s%s", LaunchElfDir, filename);
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0){
			size = fioLseek(fd, 0, SEEK_END);
			fioClose(fd);
			if(size>=0){
				ret = SifLoadModule(path, 0, NULL);
				if(ret>=0) return 0;
			}
		}
	}

	sprintf(path, "mc0:/SYS-CONF/%s", filename);
	fd = fioOpen(path, O_RDONLY);
	if(fd>=0){
		size = fioLseek(fd, 0, SEEK_END);
		fioClose(fd);
		if(size>=0){
			ret = SifLoadModule(path, 0, NULL);
			if(ret>=0) return 0;
		}
	}

	sprintf(path, "mc1:/SYS-CONF/%s", filename);
	fd = fioOpen(path, O_RDONLY);
	if(fd>=0){
		size = fioLseek(fd, 0, SEEK_END);
		fioClose(fd);
		if(size>=0){
			ret = SifLoadModule(path, 0, NULL);
			if(ret>=0) return 0;
		}
	}
	return -1;
}

//--------------------------------------------------------------
void loadUsbModules(void)
{
	static int loaded=FALSE;
	int ret;

	if(!loaded){
		initsbv_patches();
		if(!strncmp(LaunchElfDir, "mass", 4)){
			//massから起動したときLbFに組み込んだIRXをロードする
			//USBD.IRX
			SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
			//USB_MASS.IRX
			SifExecModuleBuffer(&usb_mass_irx, size_usb_mass_irx, 0, NULL, &ret);
		}
		else{
			//USBD.IRX
			ret = load_irxfile("USBD.IRX");
			if (ret<0) SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
			//USB_MASS.IRX
			ret = load_irxfile("USB_MASS.IRX");
			if (ret<0) SifExecModuleBuffer(&usb_mass_irx, size_usb_mass_irx, 0, NULL, &ret);
		}

		delay(3);
		ret = usb_mass_bindRpc();
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void poweroffHandler(int i)
{
	hddPowerOff();
}

//--------------------------------------------------------------
void loadHddModules(void)
{
	static int loaded=FALSE;
	int ret;
	int i=0;
	
	if(!loaded)
	{	drawMsg(lang->main_loadhddmod);
		hddPreparePoweroff();
		hddSetUserPoweroffCallback((void *)poweroffHandler,(void *)i);
		SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);

		load_iomanx();
		load_filexio();
		load_ps2dev9();
		load_ps2atad(); //also loads ps2hdd & ps2fs
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
// Load Network modules by EP (modified by RA)
//------------------------------
void loadNetModules(void)
{
	static int loaded=FALSE;

	if(!loaded){
		loadHddModules();
		loadUsbModules();
		drawMsg(lang->main_loadftpmod);
		
		// getIpConfig(); //RA NB: I always get that info, early in init
		// Also, my module checking makes some other tests redundant
		load_ps2netfs(); // loads ps2netfs from internal buffer
		load_ps2ftpd();  // loads ps2dftpd from internal buffer
		loaded=TRUE;
	}
	strcpy(mainMsg, netConfig);
}

//--------------------------------------------------------------
// SYSTEM.CNFの読み取り
int ReadCNF(char *direlf)
{
	char *systemcnf;
	int fd;
	int size;
	int n;
	int i;
	
	/*
	loadCdModules();
	CDVD_FlushCache();
	CDVD_DiskReady(0);
	*/
	i = 0x10000;
	while(i--) asm("nop\nnop\nnop\nnop");
	fd = fioOpen("cdrom0:\\SYSTEM.CNF;1",1);
	if(fd>=0) {
		size = fioLseek(fd,0,SEEK_END);
		fioLseek(fd,0,SEEK_SET);
		systemcnf = (char*)malloc(size+1);
		fioRead(fd, systemcnf, size);
		systemcnf[size+1]=0;
		for(n=0; systemcnf[n]!=0; n++){
			if(!strncmp(&systemcnf[n], "BOOT2", 5)) {
				n+=5;
				break;
			}
		}
		while(systemcnf[n]!=0 && systemcnf[n]==' ') n++;
		if(systemcnf[n]!=0 ) n++; // salta '='
		while(systemcnf[n]!=0 && systemcnf[n]==' ') n++;
		if(systemcnf[n]==0){
			free(systemcnf);
			return 0;
		}
		
		for(i=0; systemcnf[n+i]!=0; i++) {
			direlf[i] = systemcnf[n+i];
			if(i>2)
				if(!strncmp(&direlf[i-1], ";1", 2)) {
					direlf[i+1]=0;
					break;
				}
		}
		fioClose(fd);
		free(systemcnf);
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------
// ELFのテストと実行
void RunElf(const char *path)
{
	char tmp[MAX_PATH];
	static char fullpath[MAX_PATH];
	static char party[40];
	char *p;
	
	if(path[0]==0) return;
	
	if(!strncmp(path, "hdd0:/", 6)){
		loadHddModules();
		sprintf(party, "hdd0:%s", path+6);
		p = strchr(party, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
	}
	else if(!strncmp(path, "mc", 2)){
		party[0] = 0;
		if(path[2]==':'){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkELFheader(fullpath)<0){
				fullpath[2]='1';
				if(checkELFheader(fullpath)<0){
					sprintf(mainMsg, "%s%s", path, lang->main_notfound);
					return;
				}
			}
		} else {
			strcpy(fullpath, path);
			if(checkELFheader(fullpath)<0){
				sprintf(mainMsg, "%s%s", path, lang->main_notfound);
				return;
			}
		}
	}
	else if(!strncmp(path, "mass", 4)){
		loadUsbModules();
		party[0] = 0;
		strcpy(fullpath, "mass:");
		strcat(fullpath, path+6);
		if(checkELFheader(fullpath)<0){
			sprintf(mainMsg, "%s%s", path, lang->main_notfound);
			return;
		}
	}
	else if(!stricmp(path, "MISC/PS2Disc")){
		drawMsg(lang->main_readsystemcnf);
		strcpy(mainMsg, lang->main_failed);
		party[0]=0;
		trayopen=FALSE;
		if(!ReadCNF(fullpath)) return;
		//strcpy(mainMsg, "Succece!"); return;
	}
	else if(!stricmp(path, "MISC/FileBrowser")){
		mainMsg[0] = 0;
		tmp[0] = 0;
		LastDir[0] = 0;
		getFilePath(tmp, ANY_FILE);
		if(tmp[0]) RunElf(tmp);
		else return;
	}
	else if(!stricmp(path, "MISC/PS2Browser")){
		party[0]=0;
		strcpy(fullpath,"rom0:OSDSYS");
	}
	else if(!stricmp(path, "MISC/PS2Net")){	//PS2Net uLaunchELF3.60
		getIpConfig();	//リロード
		mainMsg[0] = 0;
		loadNetModules();
		return;
	}
	else if(!strncmp(path, "cdfs", 4)){
		party[0] = 0;
		strcpy(fullpath, path);
		CDVD_FlushCache();
		CDVD_DiskReady(0);
	}
	else if(!strncmp(path, "rom", 3)){
		party[0] = 0;
		strcpy(fullpath, path);
	}
	
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	FreeFontAscii();	//フォントを終了
	FreeFontKnaji();
	FreeLanguage();
	free(setting);
	padPortClose(0,0);
	RunLoaderElf(fullpath, party);
}

//--------------------------------------------------------------
// 方向キーで選択されたELFの実行
void RunSelectedElf(void)
{
	int n=0;
	int i;
	
	for(i=0; i<12; i++){
		if(setting->dirElf[i][0] && n++==selected){
			RunElf(setting->dirElf[i]);
			break;
		}
	}
}

//--------------------------------------------------------------
// reboot IOP (original source by Hermes in BOOT.c - cogswaploader)
// uLaunchELF3.60
void Reset()
{
	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
	while(SifIopSync());
	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();

	SifInitRpc(0);
	FlushCache(0);
	FlushCache(2);
}

//--------------------------------------------------------------
// メイン画面の描画
int drawMainScreen(void)
{
	int nElfs=0;
	int i;
	int x, y;
	uint64 color;
	char c[MAX_PATH+8], f[MAX_PATH];
	char *p;
	char dummyElf[MAX_PATH];

	strcpy(setting->dirElf[12], "CONFIG");
	
	clrScr(setting->color[0]);
	
	// 枠の中
	x = FONT_WIDTH*5;
	y = SCREEN_MARGIN + FONT_HEIGHT*3;

	//DEFAULT
	if(setting->dirElf[0][0]){
		if(mode==BUTTON){
			if(cancel==FALSE)
				sprintf(c, "TIMEOUT: %d", timeout/SCANRATE);
			else
				sprintf(c, "TIMEOUT: -");
		}
		else
			sprintf(c, "TIMEOUT: -");
		printXY(c, x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
	}

	if(mode!=DPAD_MISC){
		for(i=0; i<13; i++){
			if(setting->dirElf[i][0]){
				switch(i){
				case 0:
					strcpy(c,"DEFAULT: ");
					break;
				case 1:
					strcpy(c,"     ○: ");
					break;
				case 2:
					strcpy(c,"     ×: ");
					break;
				case 3:
					strcpy(c,"     □: ");
					break;
				case 4:
					strcpy(c,"     △: ");
					break;
				case 5:
					strcpy(c,"     L1: ");
					break;
				case 6:
					strcpy(c,"     R1: ");
					break;
				case 7:
					strcpy(c,"     L2: ");
					break;
				case 8:
					strcpy(c,"     R2: ");
					break;
				case 9:
					strcpy(c,"     L3: ");
					break;
				case 10:
					strcpy(c,"     R3: ");
					break;
				case 11:
					strcpy(c,"  START: ");
					break;
				case 12:
					strcpy(c," SELECT: ");
					break;
				}
				//
				if(setting->filename){
					if((p=strrchr(setting->dirElf[i], '/')))
						strcpy(f, p+1);
					else
						strcpy(f, setting->dirElf[i]);
					if((p=strrchr(f, '.')))
						*p = 0;
				}
				else{
					strcpy(f, setting->dirElf[i]);
				}
				strcat(c, f);
				//文字列の色
				if(nElfs++==selected && mode==DPAD)
					color = setting->color[2];
				else
					color = setting->color[3];
				printXY(c, x, y, color, TRUE);
				y += FONT_HEIGHT;
			}
		}
	}
	else{	//mode==DPAD_MISC
		for(i=0; i<5; i++){
			strcpy(c,"         ");
			switch(i){
			case 0:
				strcpy(dummyElf,"MISC/FileBrowser");
				break;
			case 1:
				strcpy(dummyElf,"MISC/PS2Browser");
				break;
			case 2:
				strcpy(dummyElf,"MISC/PS2Disc");
				break;
			case 3:
				strcpy(dummyElf,"MISC/PS2Net");
				break;
			case 4:
				strcpy(dummyElf,"CONFIG");
				break;
			}
			//
			if(setting->filename){
				if((p=strrchr(dummyElf, '/')))
					strcpy(f, p+1);
				else
					strcpy(f, dummyElf);
			}
			else{
				strcpy(f, dummyElf);
			}
			strcat(c, f);
			//文字列の色
			if(nElfs++==selected)
				color = setting->color[2];
			else
				color = setting->color[3];
			printXY(c, x, y, color, TRUE);
			y += FONT_HEIGHT;
		}
	}
	// 操作説明
	if(mode==BUTTON)
		strcpy(c, lang->main_launch_hint);
	else
		sprintf(c, "○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
	
	setScrTmp(mainMsg, c);
	drawScr();
	
	return nElfs;
}

//--------------------------------------------------------------
// main
int main(int argc, char *argv[])
{
	char *p;
	int nElfs;
	CdvdDiscType_t cdmode;

	//ELFのフォルダ名 uLaunchELF3.60
	strcpy(LaunchElfDir, argv[0]);
	if	( ((p=strrchr(LaunchElfDir, '/'))==NULL)&&((p=strrchr(LaunchElfDir, '\\'))==NULL) )
		p=strrchr(LaunchElfDir, ':');
	if	(p!=NULL)
		*(p+1)=0;

	SifInitRpc(0);

	//RESET IOP
	if(!strncmp(LaunchElfDir, "host", 4)){
		//hostから起動したときリセットしない
	}
	else{
		//host以外から起動したときリセット
		Reset();
	}

	//
	loadModules();
	mcInit(MC_TYPE_MC);
	setupPad();
	initsbv_patches();
	//mass
	if(!strncmp(LaunchElfDir, "mass:", 5)) loadUsbModules();


	//CNFファイルを読み込む前に初期化
	InitLanguage();

	//設定をロード
	loadConfig(mainMsg);

	//discControl
	if(setting->discControl)
		loadCdModules();

	//フォント
	if(InitFontAscii(setting->AsciiFont)<0)
		InitFontAscii("systemfont");
	if(InitFontKnaji(setting->KanjiFont)<0)
		InitFontKnaji("systemfont");
	SetFontMargin(CHAR_MARGIN, setting->CharMargin);
	SetFontMargin(LINE_MARGIN, setting->LineMargin);
	SetFontBold(setting->FontBold);
	SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
	SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
	SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
	SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);

	getIpConfig();

	setupito();

	LastDir[0] = 0;

	timeout = (setting->timeout+1)*SCANRATE;
	while(1){
		if(setting->discControl){
			CDVD_Stop();
			cdmode = cdGetDiscType();
			if(cdmode==CDVD_TYPE_NODISK){
				trayopen = TRUE;
				strcpy(mainMsg, lang->main_nodisc);
			}else if(cdmode>=0x01 && cdmode<=0x04){
				strcpy(mainMsg, lang->main_detectingdisc);
			}else if(trayopen==TRUE){
				trayopen=FALSE;
				strcpy(mainMsg, lang->main_stopdisc);
			}
		}
		
		if(cancel==FALSE) timeout--;
		nElfs = drawMainScreen();

		waitPadReady(0,0);
		if(readpad()){
			switch(mode){
				case BUTTON:
				{
					if(new_pad & PAD_CIRCLE){
						cancel=TRUE;
						RunElf(setting->dirElf[1]);
					}
					else if(new_pad & PAD_CROSS){
						cancel=TRUE;
						RunElf(setting->dirElf[2]);
					}
					else if(new_pad & PAD_SQUARE){
						cancel=TRUE;
						RunElf(setting->dirElf[3]);
					}
					else if(new_pad & PAD_TRIANGLE){
						cancel=TRUE;
						RunElf(setting->dirElf[4]);
					}
					else if(new_pad & PAD_L1){
						cancel=TRUE;
						RunElf(setting->dirElf[5]);
					}
					else if(new_pad & PAD_R1){
						cancel=TRUE;
						RunElf(setting->dirElf[6]);
					}
					else if(new_pad & PAD_L2){
						cancel=TRUE;
						RunElf(setting->dirElf[7]);
					}
					else if(new_pad & PAD_R2){
						cancel=TRUE;
						RunElf(setting->dirElf[8]);
					}
					else if(new_pad & PAD_L3){
						cancel=TRUE;
						RunElf(setting->dirElf[9]);
					}
					else if(new_pad & PAD_R3){
						cancel=TRUE;
						RunElf(setting->dirElf[10]);
					}
					else if(new_pad & PAD_START){
						cancel=TRUE;
						RunElf(setting->dirElf[11]);
					}
					else if(new_pad & PAD_SELECT){
						cancel=TRUE;
						config(mainMsg);
						if(setting->discControl)
							loadCdModules();
					}
					else if(new_pad & PAD_UP || new_pad & PAD_DOWN){
						cancel=TRUE;
						selected=0;
						mode=DPAD;
					}
					else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT){
						cancel=TRUE;
						selected=0;
						mode=DPAD_MISC;
					}
					break;
				}
				case DPAD:
				{
					if(new_pad & PAD_UP){
						selected--;
						if(selected<0)
							selected=nElfs-1;
					}
					else if(new_pad & PAD_DOWN){
						selected++;
						if(selected>=nElfs)
							selected=0;
					}
					else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT){
						selected=0;
						mode=DPAD_MISC;
					}
					else if(new_pad & PAD_CROSS){
						mode=BUTTON;
					}
					else if(new_pad & PAD_CIRCLE){
						if(selected==nElfs-1){		//CONFIG
							mode=BUTTON;
							config(mainMsg);
							if(setting->discControl)
								loadCdModules();
						}
						else{
							if(mode==DPAD)
								RunSelectedElf();		//ランチャー
						}
					}
/*
					//デバッグ
					else if(new_pad & PAD_R1){
					}
*/
					break;
				}
				case DPAD_MISC:
				{
					if(new_pad & PAD_UP){
						selected--;
						if(selected<0)
							selected=nElfs-1;
					}
					else if(new_pad & PAD_DOWN){
						selected++;
						if(selected>=nElfs)
							selected=0;
					}
					else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT || new_pad & PAD_CROSS){
						selected=0;
						mode=BUTTON;
					}
					else if(new_pad & PAD_CIRCLE){
						switch(selected){
						case 0:
							RunElf("MISC/FileBrowser");
							break;
						case 1:
							RunElf("MISC/PS2Browser");
							break;
						case 2:
							RunElf("MISC/PS2Disc");
							break;
						case 3:
							RunElf("MISC/PS2Net");
							break;
						case 4:
							config(mainMsg);
							if(setting->discControl) loadCdModules();
						}
					}
					break;
				}
			}
		}
		if(timeout/SCANRATE==0 && setting->dirElf[0][0] && mode==BUTTON && cancel==FALSE){
			RunElf(setting->dirElf[0]);
		}
	}
}
