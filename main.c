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
//extern u8 *loader_elf;
//extern int size_loader_elf;
extern u8 *iopmod_irx;
extern int size_iopmod_irx;
extern u8 *usbd_irx;
extern int size_usbd_irx;
extern void usbhdfsd_irx;
extern int size_usbhdfsd_irx;
extern u8 *cdvd_irx;
extern int size_cdvd_irx;
extern u8 *ps2ip_irx;
extern int size_ps2ip_irx;
extern u8 *ps2smap_irx;
extern int size_ps2smap_irx;
extern u8 *ps2ftpd_irx;
extern int size_ps2ftpd_irx;

//#define DEBUG
#ifdef DEBUG
#include <debug.h>
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
char LaunchElfDir[MAX_PATH], mainMsg[MAX_PATH];
int boot;

int reset=FALSE;
int usbmass=0;

//--------------------------------------------------------------
#define IPCONF_MAX_LEN  (3*16)
char if_conf[IPCONF_MAX_LEN];
int if_conf_len;

char ip[16]      = "192.168.0.10";
char netmask[16] = "255.255.255.0";
char gw[16]      = "192.168.0.1";

char netConfig[IPCONF_MAX_LEN+64];	//Adjust size as needed

//--------------------------------------------------------------
//original source uLaunchELF
void PS2Browser(void) 
{
	__asm__ __volatile__(
	"li $3, 0x04;"
	"syscall;"
	"nop;");
}

//--------------------------------------------------------------
// FormatMemoryCard
void FormatMemoryCard(void)
{
	char tmp[2048];
	int ret;
	int type;
	char dir[MAX_PATH];
	char path[MAX_PATH];
	char romver[16];
	int fd;
	int size_bootelf=0;
	char *bootelf=NULL;
	int size_titledb=0;
	char *titledb=NULL;
	int y;
	int n;
	char log[8][MAX_PATH];
	int i;

	y=SCREEN_MARGIN;
	n=0;
	clrScr(setting->color[COLOR_BACKGROUND]);
	drawScr();
	clrScr(setting->color[COLOR_BACKGROUND]);
	drawScr();

	sprintf(tmp, "format mc0:/ OK?       ");
	ret = MessageBox(tmp, "FormatMemoryCard", MB_YESNO|MB_DEFBUTTON2);
	if(ret!=IDYES){
		//キャンセル
		return;
	}
	sprintf(tmp, "format mc0:/ OK Really?");
	ret = MessageBox(tmp, "FormatMemoryCard", MB_YESNO|MB_DEFBUTTON2);
	if(ret!=IDYES){
		//キャンセル
		return;
	}

	//メモリーカードの種類を取得
	mcGetInfo(0, 0, &type, NULL, NULL);	//mc0
	mcSync(MC_WAIT, NULL, NULL);

	//メモリーカードの種類
	if(type==MC_TYPE_NONE){
		strcpy(log[n], "MC_TYPE_NONE"); n++;
	}
	else if(type==MC_TYPE_PSX){
		strcpy(log[n], "MC_TYPE_PSX"); n++;
	}
	else if(type==MC_TYPE_PS2){
		strcpy(log[n], "MC_TYPE_PS2"); n++;
	}
	else if(type==MC_TYPE_POCKET){
		strcpy(log[n], "MC_TYPE_POCKET"); n++;
	}

	strcpy(log[n], "format start"); n++;

	//ログ表示
	clrScr(setting->color[COLOR_BACKGROUND]);
	for(i=0;i<n;i++)
		printXY(log[i], FONT_WIDTH*2, SCREEN_MARGIN+i*FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);
	drawScr();

	//バックアップ
	//ROM Version
	fd = fioOpen("rom0:ROMVER", O_RDONLY);
	fioRead(fd, romver, sizeof(romver));
	fioClose(fd);
	romver[15] = 0;
	//BxDATASYSTEM
	if(romver[4]=='E')
		strcpy(dir, "mc0:/BEDATA-SYSTEM");/* europe */
	else if(romver[4]=='J')
		strcpy(dir, "mc0:/BIDATA-SYSTEM");/* japan */
	else
		strcpy(dir, "mc0:/BADATA-SYSTEM");/* us */

	//フォルダオープンしてみる
	fd = fioDopen(dir);
	if(fd<0){
		dir[0]='\0';
	}
	else{
		fioDclose(fd);
		//BOOT.ELF
		sprintf(path, "%s/BOOT.ELF", dir);
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0){
			size_bootelf = fioLseek(fd, 0, SEEK_END);
			fioLseek(fd, 0, SEEK_SET);
			bootelf = (char*)malloc(size_bootelf);
			fioRead(fd, bootelf, size_bootelf);
			fioClose(fd);
			sprintf(log[n], "backup %s", path); n++;
		}
		//TITLE.DB
		sprintf(path, "%s/TITLE.DB", dir);
		fd = fioOpen(path, O_RDONLY);
		if(fd>=0){
			size_titledb = fioLseek(fd, 0, SEEK_END);
			fioLseek(fd, 0, SEEK_SET);
			titledb = (char*)malloc(size_titledb);
			fioRead(fd, titledb, size_titledb);
			fioClose(fd);
			sprintf(log[n], "backup %s", path); n++;
		}
	}

	//ログ表示
	strcpy(log[n], "Initialize..."); n++;
	clrScr(setting->color[COLOR_BACKGROUND]);
	for(i=0;i<n;i++)
		printXY(log[i], FONT_WIDTH*2, SCREEN_MARGIN+i*FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);
	drawScr();

	//未フォーマットにする
	if(type==MC_TYPE_PS2){
		mcUnformat(0, 0);
		mcSync(MC_WAIT, NULL, NULL);
	}

	//ログ表示
	strcpy(log[n], "format..."); n++;
	clrScr(setting->color[COLOR_BACKGROUND]);
	for(i=0;i<n;i++)
		printXY(log[i], FONT_WIDTH*2, SCREEN_MARGIN+i*FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);
	drawScr();

	//フォーマット開始
	mcFormat(0, 0);
	mcSync(MC_WAIT, NULL, NULL);

	//元に戻す
	if(dir){
		//BxDATASYSTEM
		fioMkdir(dir);
		//
		if(bootelf!=NULL){
			sprintf(path, "%s/BOOT.ELF", dir);
			fd = fioOpen(path, O_WRONLY | O_CREAT);
			if(fd>=0){
				fioWrite(fd, bootelf, size_bootelf);
				free(bootelf);
				fioClose(fd);
				sprintf(log[n], "restore %s", path); n++;
			}
		}
		//
		if(titledb!=NULL){
			sprintf(path, "%s/TITLE.DB", dir);
			fd = fioOpen(path, O_WRONLY | O_CREAT);
			if(fd>=0){
				fioWrite(fd, titledb, size_titledb);
				free(titledb);
				fioClose(fd);
				sprintf(log[n], "restore %s", path); n++;
			}
		}
	}

	//ログ表示
	strcpy(log[n], "format end"); n++;
	clrScr(setting->color[COLOR_BACKGROUND]);
	for(i=0;i<n;i++)
		printXY(log[i], FONT_WIDTH*2, SCREEN_MARGIN+i*FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);
	drawScr();
	clrScr(setting->color[COLOR_BACKGROUND]);
	for(i=0;i<n;i++)
		printXY(log[i], FONT_WIDTH*2, SCREEN_MARGIN+i*FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);
	drawScr();
	MessageBox("formated mc0:/", "FormatMemoryCard", MB_OK);
	return;
}

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
void initsbv_patches(void)
{
	static int SbvPatchesInited=FALSE;
	
	if(!SbvPatchesInited){
		sbv_patch_enable_lmb();
		sbv_patch_disable_prefix_check();
		SbvPatchesInited=TRUE;
	}
}

//--------------------------------------------------------------
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
//IRXファイル読み込み
int load_irxfile(char *filename, int arg_len, char *args)
{
	int fd, size;
	char path[MAX_PATH];
	int ret=-1;

	if(boot == HOST_BOOT) goto mc;
	if(boot == MASS_BOOT) goto mc;

	sprintf(path, "%s%s", LaunchElfDir, filename);
	fd = fioOpen(path, O_RDONLY);
	if(fd>=0){
		size = fioLseek(fd, 0, SEEK_END);
		fioClose(fd);
		if(size>=0){
			ret = SifLoadModule(path, arg_len, args);
			if(ret>=0) return 1;
		}
	}

mc:
	sprintf(path, "mc0:/SYS-CONF/%s", filename);
	fd = fioOpen(path, O_RDONLY);
	if(fd>=0){
		size = fioLseek(fd, 0, SEEK_END);
		fioClose(fd);
		if(size>=0){
			ret = SifLoadModule(path, arg_len, args);
			if(ret>=0) return 2;
		}
	}

	sprintf(path, "mc1:/SYS-CONF/%s", filename);
	fd = fioOpen(path, O_RDONLY);
	if(fd>=0){
		size = fioLseek(fd, 0, SEEK_END);
		fioClose(fd);
		if(size>=0){
			ret = SifLoadModule(path, arg_len, args);
			if(ret>=0) return 3;
		}
	}
	return -1;
}

//--------------------------------------------------------------
void	load_iomanx(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "IOX/File_Manager", &mod_t )==0)
			SifExecModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_filexio(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "IOX/File_Manager_Rpc", &mod_t )==0)
			SifExecModuleBuffer(&filexio_irx, size_filexio_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2dev9(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "dev9", &mod_t )==0)
			SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2atad(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "atad", &mod_t )==0)
			SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2hdd(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";

	if(!loaded){
		if(smod_get_mod_by_name( "hdd", &mod_t )==0)
			SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2fs(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";

	if(!loaded){
		if(smod_get_mod_by_name( "pfs_driver", &mod_t )==0)
			SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, sizeof(pfsarg), pfsarg, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2ip(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "TCP/IP Stack", &mod_t )==0)
			SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2smap(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){	
		if(smod_get_mod_by_name( "INET_SMAP_driver", &mod_t )==0)
			SifExecModuleBuffer(&ps2smap_irx, size_ps2smap_irx, if_conf_len, &if_conf[0], &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_ps2ftpd(void)
{
	int ret;
	static int loaded=FALSE;
//	smod_mod_info_t	mod_t;
	int arglen;
	char* arg_p;

	arg_p = "-anonymous";
	arglen = strlen(arg_p);

	if(!loaded){
		//if( smod_get_mod_by_name( "???" )==0 )
			SifExecModuleBuffer(&ps2ftpd_irx, size_ps2ftpd_irx, arglen, arg_p, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void	load_poweroff(void)
{
	int ret;
	static int loaded=FALSE;
	smod_mod_info_t	mod_t;

	if(!loaded){
		if(smod_get_mod_by_name( "Poweroff_Handler", &mod_t )==0)
			SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &ret);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
void PowerOff(void)
{
	char filepath[MAX_PATH] = "xyz:/imaginary/hypothetical/doesn't.exist";
	FILE *File;

	hddPowerOff();
	delay(1);
	File = fopen( filepath, "r" );
	if( File != NULL ) fclose( File );
}

//--------------------------------------------------------------
void setupPowerOff(void)
{
	static int loaded=FALSE;

	if(!loaded){
		hddPreparePoweroff();
		load_poweroff();
		load_iomanx();
		load_filexio();
		load_ps2dev9();
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
//loadHddModules
void loadHddModules(void)
{
	static int loaded=FALSE;
	
	if(!loaded){
		drawMsg(lang->main_loadhddmod);
		setupPowerOff();
//		load_iomanx();
//		load_filexio();
//		load_ps2dev9();
		load_ps2atad();
		load_ps2hdd();
		load_ps2fs();
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
//FTPD
void loadFtpdModules(void)
{
	static int loaded=FALSE;

	if(!loaded){
		loadHddModules();
		loadUsbModules();
		drawMsg(lang->main_loadftpmod);
//		load_iomanx();
//		load_ps2dev9();
		load_ps2ip();
		load_ps2smap();
		load_ps2ftpd();
		loaded=TRUE;
	}
	strcpy(mainMsg, netConfig);
}

//--------------------------------------------------------------
//loadUsbModules
void loadUsbModules(void)
{
	static int loaded=FALSE;
	int ret;

	if(!loaded){
		//usbd.irx
		SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
		//usbhdfsd.irx
		ret = load_irxfile("USB_MASS.IRX", 0, NULL);
		usbmass=ret;
		if(ret<0)
			SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &ret);
		delay(3);
		loaded=TRUE;
	}
}

//--------------------------------------------------------------
//loadCdModules
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
//loadModules
void loadModules(void)
{
	smod_mod_info_t	mod_t;

	if(smod_get_mod_by_name( "sio2man", &mod_t )==0)
		SifLoadModule("rom0:SIO2MAN", 0, NULL);

	if(smod_get_mod_by_name( "mcman", &mod_t )==0)
		SifLoadModule("rom0:MCMAN", 0, NULL);

	if(smod_get_mod_by_name( "mcserv", &mod_t )==0)
		SifLoadModule("rom0:MCSERV", 0, NULL);

	if(smod_get_mod_by_name( "padman", &mod_t )==0)
		SifLoadModule("rom0:PADMAN", 0, NULL);
}

//--------------------------------------------------------------
void showinfo(void)
{
	char msg0[MAX_PATH], msg1[MAX_PATH];
	uint64 color;
	int nList=0, sel=0, top=0;
	int pushed=TRUE;
	int x, y, y0, y1;
	int i;

	char info[5][1024];
	char bootdevice[256];
	int fd;
	char romver[16];

/*
	//build info
	strcpy(info[nList], "BUILD INFO  : ");
#ifdef ENABLE_PSB
	strcat(info[nList], "PSB ");
#endif
#ifdef ENABLE_ICON
	strcat(info[nList], "ICON ");
#endif
	nList++;
*/
	//ROM Version
	fd = fioOpen("rom0:ROMVER", O_RDONLY);
	fioRead(fd, romver, sizeof(romver));
	fioClose(fd);
	romver[14] = 0;
	sprintf(info[nList], "ROM VERSION : %s", romver);
	nList++;
	//LaunchElfDir
	sprintf(info[nList], "ELF DIR     : %s", LaunchElfDir);
	nList++;
	//boot
	if(boot==UNK_BOOT)
		strcpy(bootdevice, "UNK_BOOT");
	else if(boot==CD_BOOT)
		strcpy(bootdevice, "CD_BOOT");
	else if(boot==MC_BOOT)
		strcpy(bootdevice, "MC_BOOT");
	else if(boot==HOST_BOOT)
		strcpy(bootdevice, "HOST_BOOT");
	else if(boot==PFS_BOOT)
		strcpy(bootdevice, "PFS_BOOT");
	else if(boot==VFS_BOOT)
		strcpy(bootdevice, "VFS_BOOT");
	else if(boot==HDD_BOOT)
		strcpy(bootdevice, "HDD_BOOT");
	else if(boot==MASS_BOOT)
		strcpy(bootdevice, "MASS_BOOT");
	sprintf(info[nList], "BOOT        : %s", bootdevice);
	nList++;
	//reset
	strcpy(info[nList], "RESET IOP   : ");
	if(reset)
		strcat(info[nList], "YES");
	else
		strcat(info[nList], "NO");
	nList++;

	//
	if(usbmass){
		strcpy(info[nList], "USB_MASS.IRX: ");
		if(usbmass==-1) strcat(info[nList], "loaded default USBHDFSD.IRX");
		if(usbmass==1){
			strcat(info[nList], LaunchElfDir);
			strcat(info[nList], "USB_MASS.IRX");
		}
		if(usbmass==2) strcat(info[nList], "loaded mc0:/SYS-CONF/USB_MASS.IRX");
		if(usbmass==3) strcat(info[nList], "loaded mc1:/SYS-CONF/USB_MASS.IRX");
		nList++;
	}

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
			else if(new_pad & PAD_SELECT)
				break;
			else if(new_pad & PAD_CROSS)
				break;
			else if(new_pad & PAD_CIRCLE)
				break;
			else if(new_pad & PAD_TRIANGLE)
				break;
		}

		//リスト表示用変数の正規化
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
			printXY(info[top+i], x+FONT_WIDTH*2, y, color, TRUE);
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
		if(pushed) sprintf(msg0, "INFO");
		// 操作説明
		sprintf(msg1, "○:%s ×:%s", lang->gen_cancel, lang->gen_cancel);
		setScrTmp(msg0, msg1);
		drawScr();
	}
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
	
	loadCdModules();

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

#ifdef ENABLE_PSB
	char *extension;
	int ret;
	//psb実行
	extension = getExtension(path);
	if(extension!=NULL){
		if(!stricmp(extension, ".psb")){
			char msgtmp[2048];
			mainMsg[0] = 0;
			ret = psb(path);
			if(ret<0){
				strcpy(msgtmp, "psb open error");
				MessageBox(msgtmp, LBF_VER, MB_OK);
			}
			if(ret>0){
				sprintf(msgtmp, "error line no = %d", ret);
				MessageBox(msgtmp, LBF_VER, MB_OK);
			}
			return;
		}
	}
#endif
	//ELF実行
	if(!strncmp(path, "mc", 2)){
		if(path[2]==':'){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkELFheader(fullpath)!=1){
				fullpath[2]='1';
				if(checkELFheader(fullpath)!=1){
					sprintf(mainMsg, "%s%s", path, lang->main_notfound);
					return;
				}
			}
		}
		else{
			strcpy(fullpath, path);
			if(checkELFheader(fullpath)!=1){
				sprintf(mainMsg, "%s%s", path, lang->main_notfound);
				return;
			}
		}
	}
	else if(!strncmp(path, "hdd0:/", 6)){
		loadHddModules();
		sprintf(party, "hdd0:%s", path+6);
		p = strchr(party, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		if(checkELFheader(path)!=1){
			sprintf(mainMsg, "%s%s", path, lang->main_notfound);
			return;
		}
	}
	else if(!strncmp(path, "cdfs", 4)){
		party[0] = 0;
		strcpy(fullpath, path);
		CDVD_FlushCache();
		CDVD_DiskReady(0);
	}
	else if(!strncmp(path, "mass", 4)){
		loadUsbModules();
		party[0] = 0;
		strcpy(fullpath, "mass:");
		strcat(fullpath, path+6);
		if(checkELFheader(fullpath)!=1){
			sprintf(mainMsg, "%s%s", path, lang->main_notfound);
			return;
		}
	}
	else if(!strncmp(path, "MISC", 4)){
		if(!stricmp(path, "MISC/FileBrowser")){
			mainMsg[0] = 0;
			tmp[0] = 0;
			LastDir[0] = 0;
			getFilePath(tmp, ANY_FILE);
			if(tmp[0]) RunElf(tmp);
			else return;
		}
		else if(!stricmp(path, "MISC/PS2Browser")){
			//party[0]=0;
			//strcpy(fullpath, "rom0:OSDSYS");
			PS2Browser();
		}
		else if(!stricmp(path, "MISC/PS2Disc")){
			drawMsg(lang->main_readsystemcnf);
			strcpy(mainMsg, lang->main_failed);
			party[0]=0;
			trayopen=FALSE;
			if(!ReadCNF(fullpath)) return;
			//strcpy(mainMsg, "Succece!"); return;
		}
		else if(!stricmp(path, "MISC/PS2Ftpd")){
			getIpConfig();	//リロード
			mainMsg[0] = 0;
			loadFtpdModules();
			return;
		}
		else if(!stricmp(path, "MISC/DiscStop")){
			loadCdModules();
			CDVD_Stop();
			strcpy(mainMsg, lang->main_stopdisc);
			return;
		}
		else if(!stricmp(path, "MISC/McFormat")){
			FormatMemoryCard();
			return;
		}
		else if(!stricmp(path, "MISC/PowerOff")){
			setupPowerOff();
			PowerOff();
			return;
		}
		else if(!stricmp(path, "MISC/INFO")){
			showinfo();
			return;
		}
		else if(!stricmp(path, "MISC/CONFIG")){
			config(mainMsg);
			if(setting->discControl)
				loadCdModules();
			return;
		}
		else{
			return;
		}
	}
/*	else if(!strncmp(path, "rom", 3)){
		party[0] = 0;
		strcpy(fullpath, path);
	}*/
	
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	clrScr(ITO_RGBA(0x00, 0x00, 0x00, 0));
	drawScr();
	FreeFontAscii();	//フォントを終了
	FreeFontKanji();
	FreeLanguage();
	free(setting);
	padPortClose(0,0);
	RunLoaderElf(fullpath, party);
}

//--------------------------------------------------------------
// reboot IOP (original source by Hermes in BOOT.c - cogswaploader)
// uLaunchELF
void Reset()
{
	reset=TRUE;

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
void LaunchMain(void)
{
	int timeout=0;
	int cancel=FALSE;
	int mode=BUTTON;
	CdvdDiscType_t cdmode;
	int use_default;
	int i;
	int mode_changed;

	uint64 color;
	char tmp[MAX_PATH+8], name[MAX_PATH];
	char *p;
	char dummyElf[MAX_PATH];

	char list[16][MAX_PATH];
	char elfpath[16][MAX_PATH];
	int nList=0, sel=0, top=0;
	int x, y, y0, y1;

	timeout = (setting->timeout+1)*SCANRATE;

	while(1){
		mode_changed=FALSE;
		use_default=FALSE;
		if(setting->dirElf[0][0]) use_default=TRUE;
		//discControl
		if(setting->discControl){
			CDVD_Stop();
			cdmode = cdGetDiscType();
			if(cdmode==CDVD_TYPE_NODISK){
				trayopen = TRUE;
				strcpy(mainMsg, lang->main_nodisc);
			}
			else if(cdmode>=CDVD_TYPE_DETECT && cdmode<=CDVD_TYPE_DETECT_DVDDUAL){
				strcpy(mainMsg, lang->main_detectingdisc);
			}
			else if(cdmode>=CDVD_TYPE_UNKNOWN){
				if(trayopen==TRUE){
					char Message[MAX_PATH];
					trayopen=FALSE;
					strcpy(Message, lang->main_stopdisc);
					if(cdmode==CDVD_TYPE_PS1CD||cdmode==CDVD_TYPE_PS1CDDA)
						strcat(Message,"(PS1CD)");
					else if(cdmode==CDVD_TYPE_PS2CD||cdmode==CDVD_TYPE_PS2CDDA)
						strcat(Message,"(PS2CD)");
					else if(cdmode==CDVD_TYPE_PS2DVD)
						strcat(Message,"(PS2DVD)");
					strcpy(mainMsg, Message);
				}
			}
		}

		//表示するリストとELFのパスのリスト作成
		for(i=0; i<16; i++){
			list[i][0]='\0';
			elfpath[i][0]='\0';
		}
		if(mode==BUTTON || mode==DPAD){
			nList=0;
			//DEFAULT
			if(setting->dirElf[0][0]){
				if(mode==BUTTON){
					if(cancel==FALSE)
						sprintf(tmp, "TIMEOUT: %d", timeout/SCANRATE);
					else
						sprintf(tmp, "TIMEOUT: -");
				}
				else if(mode==DPAD)
					sprintf(tmp, "TIMEOUT: -");
				strcpy(list[nList], tmp);
				nList++;
			}
			//BUTTON
			for(i=0; i<13; i++){
				if(setting->dirElf[i][0]){
					if(i==0)  strcpy(tmp, "DEFAULT: ");
					if(i==1)  strcpy(tmp, "     ○: ");
					if(i==2)  strcpy(tmp, "     ×: ");
					if(i==3)  strcpy(tmp, "     □: ");
					if(i==4)  strcpy(tmp, "     △: ");
					if(i==5)  strcpy(tmp, "     L1: ");
					if(i==6)  strcpy(tmp, "     R1: ");
					if(i==7)  strcpy(tmp, "     L2: ");
					if(i==8)  strcpy(tmp, "     R2: ");
					if(i==9)  strcpy(tmp, "     L3: ");
					if(i==10) strcpy(tmp, "     R3: ");
					if(i==11) strcpy(tmp, "  START: ");
					if(i==12) strcpy(tmp, " SELECT: ");
					//ELFのパスのリスト
					strcpy(elfpath[nList], setting->dirElf[i]);
					//表示するファイル名
					if(setting->filename){
						if((p=strrchr(setting->dirElf[i], '/')))
							strcpy(name, p+1);
						else
							strcpy(name, setting->dirElf[i]);
						if((p=strrchr(name, '.')))
							*p = 0;
					}
					else{
						strcpy(name, setting->dirElf[i]);
					}
					strcat(tmp, name);
					strcpy(list[nList], tmp);
					nList++;
				}
			}
		}
		else{	//mode==DPAD_MISC
			nList=9;
			for(i=0; i<nList; i++){
				strcpy(tmp, "         ");
				if(i==0) strcpy(dummyElf, "MISC/FileBrowser");
				if(i==1) strcpy(dummyElf, "MISC/PS2Browser");
				if(i==2) strcpy(dummyElf, "MISC/PS2Disc");
				if(i==3) strcpy(dummyElf, "MISC/PS2Ftpd");
				if(i==4) strcpy(dummyElf, "MISC/DiscStop");
				if(i==5) strcpy(dummyElf, "MISC/McFormat");
				if(i==6) strcpy(dummyElf, "MISC/PowerOff");
				if(i==7) strcpy(dummyElf, "MISC/INFO");
				if(i==8) strcpy(dummyElf, "MISC/CONFIG");
				//ELFのパスのリスト
				strcpy(elfpath[i], dummyElf);
				//表示するファイル名
				if(setting->filename){
					if((p=strrchr(dummyElf, '/')))
						strcpy(name, p+1);
					else
						strcpy(name, dummyElf);
				}
				else{
					strcpy(name, dummyElf);
				}
				strcat(tmp, name);
				strcpy(list[i], tmp);
			}
		}

		//キー入力
		waitPadReady(0,0);
		if(readpad()){
			if(new_pad) cancel=TRUE;
			if(mode==BUTTON){
				if(new_pad & PAD_UP){
					sel=nList-1;
					mode=DPAD;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_DOWN){
					sel=0;
					if(use_default) sel=1; 
					mode=DPAD;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT){
					sel=0;
					mode=DPAD_MISC;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_CIRCLE)
					RunElf(setting->dirElf[1]);
				else if(new_pad & PAD_CROSS)
					RunElf(setting->dirElf[2]);
				else if(new_pad & PAD_SQUARE)
					RunElf(setting->dirElf[3]);
				else if(new_pad & PAD_TRIANGLE)
					RunElf(setting->dirElf[4]);
				else if(new_pad & PAD_L1)
					RunElf(setting->dirElf[5]);
				else if(new_pad & PAD_R1)
					RunElf(setting->dirElf[6]);
				else if(new_pad & PAD_L2)
					RunElf(setting->dirElf[7]);
				else if(new_pad & PAD_R2)
					RunElf(setting->dirElf[8]);
				else if(new_pad & PAD_L3)
					RunElf(setting->dirElf[9]);
				else if(new_pad & PAD_R3)
					RunElf(setting->dirElf[10]);
				else if(new_pad & PAD_START)
					RunElf(setting->dirElf[11]);
				else if(new_pad & PAD_SELECT)
					RunElf(setting->dirElf[12]);
			}
			else if(mode==DPAD){
				if(new_pad & PAD_UP){
					sel--;
					if(sel==0 && use_default) sel=nList-1;
					if(sel<0) sel=nList-1;
				}
				else if(new_pad & PAD_DOWN){
					sel++;
					if(sel>=nList){
						sel=0;
						if(use_default) sel++;
					}
					
				}
				else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT){
					sel=0;
					mode=DPAD_MISC;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_CROSS){
					mode=BUTTON;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_CIRCLE){
					RunElf(elfpath[sel]);	//ランチャー
				}
/*
				else if(new_pad & PAD_R1){	//デバッグ
				}
*/
			}
			else if(mode==DPAD_MISC){
				if(new_pad & PAD_UP){
					sel--;
					if(sel<0) sel=nList-1;
				}
				else if(new_pad & PAD_DOWN){
					sel++;
					if(sel>=nList) sel=0;
				}
				else if(new_pad & PAD_LEFT || new_pad & PAD_RIGHT || new_pad & PAD_CROSS){
					sel=0;
					mode=BUTTON;
					mode_changed=TRUE;
				}
				else if(new_pad & PAD_CIRCLE){
					RunElf(elfpath[sel]);
				}
			}
		}

		//画面描画開始
		if(!mode_changed){
			clrScr(setting->color[COLOR_BACKGROUND]);

/*
			//モード表示
			if(mode==BUTTON)
				strcpy(tmp, "MODE:BUTTON");
			else if(mode==DPAD)
				strcpy(tmp, "MODE:DPAD  ");
			else if(mode==DPAD_MISC)
				strcpy(tmp, "MODE:MISC  ");
			printXY(tmp,
				(MAX_ROWS_X-1)*FONT_WIDTH, SCREEN_MARGIN,
				setting->color[COLOR_TEXT], TRUE);
*/

			// リスト表示用変数の正規化
			if(top > nList-MAX_ROWS) top=nList-MAX_ROWS;
			if(top < 0)              top=0;
			if(sel >= nList)         sel=nList-1;
			if(sel < 0)              sel=0;
			if(sel >= top+MAX_ROWS)  top=sel-MAX_ROWS+1;
			if(sel < top)            top=sel;

			//
			x = FONT_WIDTH*3;
			y = SCREEN_MARGIN+FONT_HEIGHT*3;
			for(i=0; i<MAX_ROWS; i++){
				if(top+i >= nList) break;
				//色
				if(top+i == sel)
					color = setting->color[COLOR_HIGHLIGHTTEXT];
				else
					color = setting->color[COLOR_TEXT];
				if(mode==BUTTON)
					color = setting->color[COLOR_TEXT];
				//リスト表示
				printXY(list[top+i], x+FONT_WIDTH*2, y, color, TRUE);
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
			// 操作説明
			if(mode==BUTTON)
				strcpy(tmp, lang->main_launch_hint);
			else
				sprintf(tmp, "○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
	
			setScrTmp(mainMsg, tmp);
			drawScr();
		}
		//AutoRun
		if(timeout/SCANRATE==0 && use_default && cancel==FALSE){
			RunElf(setting->dirElf[0]);
			cancel=TRUE;
		}
		//timeout
		if(cancel==FALSE) timeout--;
	}
}

//--------------------------------------------------------------
// main
int main(int argc, char *argv[])
{
	char *p;
	smod_mod_info_t	mod_t;
	struct padButtonStatus buttons;
	u32 paddata;
	int ret;

	//ブートフォルダ名	original source altimit
	if (argc == 0){
		strcpy(LaunchElfDir,"host:"); // Naplink
	}
	else if (argc != 1){
		strcpy(LaunchElfDir,"mc0:/BWLINUX/");
	}
	else{
		//argc==1
		strcpy(LaunchElfDir,argv[0]);
		p = strrchr(LaunchElfDir,'/');
		if (p == NULL){
			p = strrchr(LaunchElfDir,'\\');
			if (p == NULL){
				p = strrchr(LaunchElfDir,':');
				if (p == NULL){
					SleepThread();
				}
			}
		}
		if (p){
			p++;
			*p = '\0';
		}
	}

	//SWAPMAGIC
	if(!strcmp(LaunchElfDir, "mass0:\\SWAPMAGIC\\"))
		strcpy(LaunchElfDir, "mass:SWAPMAGIC/");

	//ブートしたデバイス	original source altimit
	boot = UNK_BOOT;
	if(!strncmp(LaunchElfDir, "cd", 2))
		boot = CD_BOOT;
	else if(!strncmp(LaunchElfDir, "mc", 2))
		boot = MC_BOOT;
	else if(!strncmp(LaunchElfDir, "host", 4)){
		if(smod_get_mod_by_name( "fakehost", &mod_t )!=0)
			boot = HDD_BOOT;
		else
			boot = HOST_BOOT;
	}
	else if(!strncmp(LaunchElfDir, "mass", 4))
		boot = MASS_BOOT;

	SifInitRpc(0);

	//host以外から起動したときRESET
	if(boot!=HOST_BOOT)
		Reset();

	initsbv_patches();
	loadModules();

	//
	mcInit(MC_TYPE_MC);
	setupPad();

	//cd
	if(boot==CD_BOOT)
		loadCdModules();
	//mass
	if(boot==MASS_BOOT)
		loadUsbModules();

	//CNFファイルを読み込む前に初期化
	InitLanguage();

	//設定をロード
	loadConfig(mainMsg);
	SetScreenPosVM();

	//discControl
	if(setting->discControl)
		loadCdModules();

	//アスキーフォント
	if(!strncmp(setting->AsciiFont, "cdfs", 4))
		loadCdModules();
	if(!strncmp(setting->AsciiFont, "mass", 4))
		loadUsbModules();
	if(InitFontAscii(setting->AsciiFont)<0){
		strcpy(setting->AsciiFont, "systemfont");
		InitFontAscii(setting->AsciiFont);
	}
	//漢字フォント
	if(!strncmp(setting->KanjiFont, "cdfs", 4))
		loadCdModules();
	if(!strncmp(setting->KanjiFont, "mass", 4))
		loadUsbModules();
	if(InitFontKnaji(setting->KanjiFont)<0){
		strcpy(setting->KanjiFont, "systemfont");
		InitFontKnaji(setting->KanjiFont);
	}
	//
	SetFontMargin(CHAR_MARGIN, setting->CharMargin);
	SetFontMargin(LINE_MARGIN, setting->LineMargin);
	SetFontBold(setting->FontBold);
	SetFontMargin(ASCII_FONT_MARGIN_TOP, setting->AsciiMarginTop);
	SetFontMargin(ASCII_FONT_MARGIN_LEFT, setting->AsciiMarginLeft);
	SetFontMargin(KANJI_FONT_MARGIN_TOP, setting->KanjiMarginTop);
	SetFontMargin(KANJI_FONT_MARGIN_LEFT, setting->KanjiMarginLeft);

	getIpConfig();

	itoInit();
	setupito(setting->tvmode);

	LastDir[0] = 0;

	//format mc0:/
	ret=0;
	while(ret==0){
		ret = padRead(0, 0, &buttons);
	}
	paddata = 0xffff ^ buttons.btns;
	if((paddata&PAD_L2)&&(paddata&PAD_R2)&&(paddata&PAD_L1)&&(paddata&PAD_R1)){
		//L2 R2 L1 R1 ボタン押しながら起動したとき、メモリーカードをフォーマット
		FormatMemoryCard();
	}
	if(paddata&PAD_SELECT){
		//SELECTボタン押しながら起動したとき、SCREEN SETTINGを初期化
		InitScreenSetting();
		SetScreenPosVM();
		SetHeight();
		setupito(setting->tvmode);
		clrScr(setting->color[COLOR_BACKGROUND]);
		drawScr();
		clrScr(setting->color[COLOR_BACKGROUND]);
		drawScr();
		MessageBox("Screen Setting Initialize", LBF_VER, MB_OK);
	}

	//ランチャーメイン
	LaunchMain();

	return 0;
}
