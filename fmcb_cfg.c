// ◯ 
#include "launchelf.h"

//-------------------------------------------------
//FreeMcBOOT Configture
int fmcb_init(void);
int fmcb_default(void);
int fmcb_load(char *path);
int fmcb_save(char *path);

//デフォルトの設定の値
enum
{
	MAX_OSDSYS_ITEMS = 100,
	MAX_BOOT_BUTTONS = 17,
	MAX_REGIST_ELF = 3,
	MAX_PATH_LENGTH = 256,
	MAX_TITLE_LENGTH = 1024,
	MAX_CURSOR_LENGTH = 10,
	MAX_DELIMITER_LENGTH = 80,
};

enum{
	// general settings //
	cnfversion=0,
	
	// top level //
	fastboot,
	debugscreen,
	paddelay,
	
	// OSDSYS options //
	hackedosdsys,
	skipdiscboot,
	skipsonylogo,
	gotobrowser,
	selectedcolor,
	unselectedcolor,
	scrollmenu,
	menux,
	menuy,
	enterx,
	entery,
	versionx,
	versiony,
	cursormaxvelocity,
	cursoracceleration,
	displayeditems,
	skipmcupdatecheck,
	skiphddupdatecheck,
	
	MAX_INT_KEYS,
};

enum{
	leftcursor=0,
	rightcursor,
	topdelimiter,
	bottomdelimiter,
	
	MAX_CHAR_KEYS,
};

typedef struct {
	char path[MAX_REGIST_ELF][MAX_PATH_LENGTH];
} FMCB_PATH;

typedef struct {
	char title[MAX_TITLE_LENGTH];
	char path[MAX_REGIST_ELF][MAX_PATH_LENGTH];
} FMCB_ITEM;
/*
typedef struct {
	// general settings //
	int cnfversion;
	
	// launcher buttons //
	FMCB_PATH launch[MAX_BOOT_BUTTONS];
	
	// OSDSYS options //
	int hackedosdsys;
	FMCB_ITEM osditem[MAX_OSDSYS_ITEMS];
	int scrollmenu;
	int displayeditems;
	int menuy;
	int cursormaxvelocity;
	int cursoracceleration;
	char leftcursor[MAX_TITLE_LENGTH];
	char rightcursor[MAX_TITLE_LENGTH];
	char topdelimiter[MAX_TITLE_LENGTH];
	char bottomdelimiter[MAX_TITLE_LENGTH];
	int videomode;
	int skipmcupdatecheck;
	int skiphddupdatecheck;
	int skipdiscboot;
	int skipsonylogo;
	int gotobrowser;
	unsigned int selectedcolor;
	unsigned int unselectedcolor;
	int menux;
	int enterx;
	int entery;
	int versionx;
	int versiony;
	
	// ESR path //
	FMCB_PATH esr;
	
	// top level //
	int fastboot;
	int debugscreen;
	int paddelay;
} FMCB_SETTING;
*/
typedef struct {
	// general settings //
	int i[MAX_INT_KEYS];
	char c[MAX_CHAR_KEYS][MAX_TITLE_LENGTH];
	// launcher buttons //
	FMCB_PATH launch[MAX_BOOT_BUTTONS];
	// OSDSYS options //
	FMCB_ITEM osditem[MAX_OSDSYS_ITEMS];
	int videomode;					// "AUTO", "NTSC", "PAL"
	unsigned int selectedcolor;		// "0x00,0x00,0x00,0x00"
	unsigned int unselectedcolor;	// "0x00,0x00,0x00,0x00"
	// ESR path //
	FMCB_PATH esr;
	
	// temporaly data //
	int numr;
	int numw;
	int numi;
} FMCB_SETTING;

struct {char title[16]; char key[16]; } fmcb_buttons[MAX_BOOT_BUTTONS] = {
	{ .title = "AUTO",    .key = "Auto",     /* .pad = 0            */ },
	{ .title = "\x81\x9B",.key = "Circle",   /* .pad = PAD_CIRCLE   */ },
	{ .title = "\x81\x7E",.key = "Cross",    /* .pad = PAD_CROSS    */ },
	{ .title = "\x81\xA0",.key = "Square",   /* .pad = PAD_SQUARE   */ },
	{ .title = "\x81\xA2",.key = "Triangle", /* .pad = PAD_TRIANGLE */ },
	{ .title = "L1",      .key = "L1",       /* .pad = PAD_L1       */ },
	{ .title = "R1",      .key = "R1",       /* .pad = PAD_R1       */ },
	{ .title = "L2",      .key = "L2",       /* .pad = PAD_L2       */ },
	{ .title = "R2",      .key = "R2",       /* .pad = PAD_R2       */ },
	{ .title = "L3",      .key = "L3",       /* .pad = PAD_L3       */ },
	{ .title = "R3",      .key = "R3",       /* .pad = PAD_R3       */ },
	{ .title = "\x81\xAA",.key = "Up",       /* .pad = PAD_UP       */ },
	{ .title = "\x81\xAB",.key = "Down",     /* .pad = PAD_DOWN     */ },
	{ .title = "\x81\xA9",.key = "Left",     /* .pad = PAD_LEFT     */ },
	{ .title = "\x81\xA8",.key = "Right",    /* .pad = PAD_RIGHT    */ },
	{ .title = "START",   .key = "Start",    /* .pad = PAD_START    */ },
	{ .title = "SELECT",  .key = "Select",   /* .pad = PAD_SELECT   */ },
};

char fmcbpath[8][32] = {
	"mass:/FREEMCB.CNF",
	"mc0:/SYS-CONF/FREEMCB.CNF",
	"mc1:/SYS-CONF/FREEMCB.CNF",
	"host:/FREEMCB.CNF",
};

char fmcbdefault1[4][20] = {
	"uLaunchELF",          
	"ESR",                 
	"HD Loader",           
	"Simple Media System", 
};
char fmcbdefault2[4][16] = {
	"BOOT.ELF",     
	"ESR.ELF",      
	"HDLOADER.ELF", 
	"SMS.ELF",      
};
char fmcbdefault3[3][20] = {
	"mass:/BOOT/",
	"mc?:/BOOT/",
	"mc?:/B?DATA-SYSTEM/",
};//"mc?:/SYS-CONF/FMCB_CFG.ELF";
char fmcbdefault4[4][16] = {
	"HDLOADER.ELF",
	"LAUNCHELF.ELF",
	"SMS.ELF",
	"ESR.ELF",
};//"FMCB_CFG.ELF";

char fmcbkeys_int[MAX_INT_KEYS][32] = {
	"CNF_version",
	"FastBoot",
	"Debug_Screen",
	"pad_delay",
	"hacked_OSDSYS",
	"OSDSYS_Skip_Disc",
	"OSDSYS_Skip_Logo",
	"OSDSYS_Inner_Browser",
	"OSDSYS_selected_color",
	"OSDSYS_unselected_color",
	"OSDSYS_scroll_menu",
	"OSDSYS_menu_x",
	"OSDSYS_menu_y",
	"OSDSYS_enter_x",
	"OSDSYS_enter_y",
	"OSDSYS_version_x",
	"OSDSYS_version_y",
	"OSDSYS_cursor_max_velocity",
	"OSDSYS_cursor_acceleration",
	"OSDSYS_num_displayed_items",
	"OSDSYS_Skip_MC",
	"OSDSYS_Skip_HDD",
};

char fmcbkeys_char[MAX_CHAR_KEYS][32] = {
	"OSDSYS_left_cursor",
	"OSDSYS_right_cursor",
	"OSDSYS_menu_top_delimiter",
	"OSDSYS_menu_bottom_delimiter",
};

int fmcbinit_int[MAX_INT_KEYS] = {
	1, 1, 0, 0,
	1, 1, 1, 0, 0x80E08010, 0x80333333,
	1, 320, 110, -1, -1, -1, -1, 1000, 100, 7, 1, 1,
};

char fmcbinit_char[MAX_CHAR_KEYS][80] = {
	"o009", "o008",
	"y-99Free McBoot              c1[r0.80Version 1.8r0.00]y-00",
	"c0r0.60y+99Use o006/o007 to browse listy-00r0.00",
};

char fmcbvmode[4][8] = {"AUTO", "NTSC", "PAL", "0"};

FMCB_SETTING *fmcb = NULL;
FMCB_SETTING *fmcb_backup = NULL;

int explodeconfig(const char *src);
extern int tmpi[];
extern char tmps[][MAX_PATH];

enum{
	type_end=0,
	type_sub,	// サブメニュー
	type_bol,	// boolean (ON/OFF)
	type_chr,	// 文字列データ
	type_int,	// 数値データ
	type_pos,	// 座標データ
	type_pal,	// カラーコード
	type_sel,	// 文字列インデックス
	type_cnf,	// CNF選択用
	type_dir,	// ELFファイル指定
	type_flt,	// 時間データ
	type_ret,	// 戻る
};
typedef void usr;
typedef struct {
	int lang;	// メッセージ番号
	int type;	// メニューの種類
	void *ptr;	// 変数へのポインタ
	int prms[4];// 各種パラメータ
	int help;
	usr (*func)(char*,void*,int);
} SUBMENU;
usr mnu_fmcb_cnf(char* msg0, SUBMENU* data, int num);
usr mnu_fmcb_launcher(char* msg0, SUBMENU* data, int num);
usr mnu_fmcb_osditem(char* msg0, SUBMENU* data, int num);
usr mnu_fmcb_esr(char* msg0, SUBMENU* data, int num);
SUBMENU mnu_fmcb[56] = {
	// トップメニュー
	{.lang= 1, .help=27, .func=(void*)&mnu_fmcb_cnf, .type=type_cnf, .ptr=&tmpi[0], .prms={0,2,(int)&fmcbpath[0][0],(int)(&fmcbpath[1][0]-&fmcbpath[0][0])}},
	{.lang= 3, .help=60, .func=(void*)&mnu_fmcb_launcher, .type=type_sub, .prms={10,52},},				// [ 1]
	{.lang= 4, .help=60, .func=NULL, .type=type_sub, .prms={16,53},},
	{.lang= 5, .help=60, .func=(void*)&mnu_fmcb_esr, .type=type_sub, .prms={50,56},},
	{.lang= 6, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)fastboot,},
	{.lang= 7, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)debugscreen,},
	{.lang= 8, .help=59, .func=NULL, .type=type_flt, .ptr=(void*)paddelay, .prms={0,10000,100,1000}},
	{.lang= 2, .help=28, .func=(void*)&mnu_fmcb_cnf, .type=type_cnf, .ptr=&tmpi[4], .prms={0,2,(int)&fmcbpath[0][0],(int)(&fmcbpath[1][0]-&fmcbpath[0][0])}},
	{.lang= 9, .help=60, .func=NULL, .type=type_ret,},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},
	// ランチャーの設定
	{.lang=39, .help=60, .func=NULL, .type=type_ret,},							// [10]
	{.lang=10, .help=46, .func=(void*)&mnu_fmcb_launcher, .type=type_sel, .ptr=&tmpi[1], .prms={0,MAX_BOOT_BUTTONS-1,(int)&fmcb_buttons[0].title[0],(int)(&fmcb_buttons[1].title[0]-&fmcb_buttons[0].title[0])}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_launcher, .type=type_dir, .ptr=&tmps[1], .prms={1}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_launcher, .type=type_dir, .ptr=&tmps[2], .prms={2}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_launcher, .type=type_dir, .ptr=&tmps[3], .prms={3}},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},
	// OSDSYSの設定 {39,13,14,15,16,17,18,19,20,21,22,23,25,26,0};
	{.lang=39, .help=60, .func=NULL, .type=type_ret,},							// [16]
	{.lang=12, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)hackedosdsys,},
	{.lang=13, .help=60, .func=(void*)&mnu_fmcb_osditem, .type=type_sub, .prms={32,54},},
	{.lang=14, .help=60, .func=NULL, .type=type_sub, .prms={39,55},},
	{.lang=15, .help=62, .func=NULL, .type=type_sel, .ptr=&tmpi[2], .prms={0,2,(int)&fmcbvmode[0][0],(int)(&fmcbvmode[1][0]-&fmcbvmode[0][0])}},
	{.lang=16, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)skipmcupdatecheck,},
	{.lang=17, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)skiphddupdatecheck,},
	{.lang=18, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)skipdiscboot,},
	{.lang=19, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)skipsonylogo,},
	{.lang=20, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)gotobrowser,},
	{.lang=21, .help=63, .func=NULL, .type=type_pal, .ptr=(void*)selectedcolor,},
	{.lang=22, .help=63, .func=NULL, .type=type_pal, .ptr=(void*)unselectedcolor,},
	{.lang=23, .help=59, .func=NULL, .type=type_int, .ptr=(void*)menux, .prms={0,640,1,10}},
	{.lang=25, .help=63, .func=NULL, .type=type_pos, .ptr=(void*)enterx, .prms={-1,640,-1,256}},
	{.lang=26, .help=63, .func=NULL, .type=type_pos, .ptr=(void*)versionx, .prms={-1,640,-1,256}},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},
	// アイテムの設定 {39,40,29,30,30,30,0};
	{.lang=39, .help=60, .func=NULL, .type=type_ret,},							// [32]
	{.lang=40, .help=46, .func=(void*)&mnu_fmcb_osditem, .type=type_int, .ptr=&tmpi[3], .prms={1,100,1,5}},
	{.lang=29, .help=58, .func=(void*)&mnu_fmcb_osditem, .type=type_chr, .ptr=&tmps[0], .prms={MAX_TITLE_LENGTH}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_osditem, .type=type_dir, .ptr=&tmps[1], .prms={1}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_osditem, .type=type_dir, .ptr=&tmps[2], .prms={2}},
	{.lang=30, .help=57, .func=(void*)&mnu_fmcb_osditem, .type=type_dir, .ptr=&tmps[3], .prms={3}},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},
	// スクロールメニューの設定 {39,31,32,24,33,34,35,36,37,38,0};
	{.lang=39, .help=60, .func=NULL, .type=type_ret,},							// [39]
	{.lang=31, .help=62, .func=NULL, .type=type_bol, .ptr=(void*)scrollmenu,},
	{.lang=32, .help=47, .func=NULL, .type=type_int, .ptr=(void*)displayeditems, .prms={1,15,1,2}},
	{.lang=24, .help=59, .func=NULL, .type=type_int, .ptr=(void*)menuy, .prms={0,220,1,5}},
	{.lang=33, .help=59, .func=NULL, .type=type_int, .ptr=(void*)cursormaxvelocity, .prms={0,300000,100,1000}},
	{.lang=34, .help=59, .func=NULL, .type=type_int, .ptr=(void*)cursoracceleration, .prms={0,30000,10,100}},
	{.lang=35, .help=61, .func=NULL, .type=type_chr, .ptr=(void*)leftcursor, .prms={MAX_CURSOR_LENGTH}},
	{.lang=36, .help=61, .func=NULL, .type=type_chr, .ptr=(void*)rightcursor, .prms={MAX_CURSOR_LENGTH}},
	{.lang=37, .help=61, .func=NULL, .type=type_chr, .ptr=(void*)topdelimiter, .prms={MAX_DELIMITER_LENGTH}},
	{.lang=38, .help=61, .func=NULL, .type=type_chr, .ptr=(void*)bottomdelimiter, .prms={MAX_DELIMITER_LENGTH}},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},
	// ESRの設定
	{.lang=39, .help=60, .func=NULL, .type=type_ret,},							// [50]
	{.lang=30, .help=61, .func=(void*)&mnu_fmcb_esr, .type=type_dir, .ptr=&tmps[1], .prms={1}},
	{.lang=30, .help=61, .func=(void*)&mnu_fmcb_esr, .type=type_dir, .ptr=&tmps[2], .prms={2}},
	{.lang=30, .help=61, .func=(void*)&mnu_fmcb_esr, .type=type_dir, .ptr=&tmps[3], .prms={3}},
	{.lang= 0, .help= 0, .func=NULL, .type=type_end,},							// [54]
};
#define	push(m)	stack[st++]=m
#define pop(m)	m=stack[--st]
int menu_player(SUBMENU* data, char *mainMsg, int title)
{
	int i,a,b,z,ret=0,redraw=framebuffers;
	char msg0[128],msg1[128],tmp[256], menutitle[128];
	int cs=0,co=0,cx=0,cy=0,cd=0,cz=0,cr=0,ml=0,cc=0;
	int stack[64],st=0;
	int x,y; uint64 color;
	unsigned char *c; int *d;
	//char item[32][64];
	strcpy(menutitle, lang->fmcb[title]);
	strcat(menutitle, "/");
	ml = strlen(menutitle);
	strcpy(msg0, mainMsg);
	while(1) {
		if ((cd==0)||(cs!=co)) {
			// メニュー変更
			cr=0;
			for(i=cs,cd=0; data[i].type!=type_end; i++) {
				if (data[i].type==type_ret)
					cr=i;
				cd++;
			}
			co=cs;
			redraw=framebuffers;
		}
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {
				redraw=framebuffers;
						strcpy(msg0, menutitle);
			}
			if (new_pad & (PAD_TRIANGLE|PAD_SELECT|PAD_START)) {
				if (cx) {
					cx = 0;
				} else if (cs) {
					new_pad|=PAD_CIRCLE;
					cy=0;
				} else {
					cy=cr-cs;
				}
			}
			cc = cs+cy;
			if ((int)data[cc].ptr < 256)
				c = (char*)&fmcb->c[(int)data[cc].ptr];
			else
				c = (char*)data[cc].ptr;
			if ((int)data[cc].ptr < 256)
				d = (int*)&fmcb->i[(int)data[cc].ptr];
			else
				d = (int*)data[cc].ptr;
			if (paddata & PAD_SQUARE) i=3; else i=2;
			if (new_pad & PAD_CIRCLE) {
				if ((data[cc].type==type_ret)&&(st<=0)) break;
				switch(data[cc].type) {
					case type_ret:
						pop(cz); 
						pop(cd); 
						pop(cy); 
						pop(cx); 
						pop(cs); 
						pop(cr); 
						pop(ml);
						menutitle[ml] = 0;
						strcpy(msg0, menutitle);
						co=cs;
						break;
					case type_sub:
						push(ml); 
						push(cr); 
						push(cs); 
						push(cx); 
						push(cy); 
						push(cd); 
						push(cz);
						strcat(menutitle, lang->fmcb[data[cc].prms[1]]);
						strcat(menutitle, "/");
						ml=strlen(menutitle);
						strcpy(msg0, menutitle);
						cs=data[cc].prms[0];
						cx=cy=cz=0;
						if (data[cc].func) {
							data[cc].func(msg0, data, cc);
						}
						break;
					case type_bol:
						*d ^= 1;
						break;
					case type_chr:
						keyboard(SKBD_ALL, c, data[cc].prms[0]);
						new_pad = PAD_CIRCLE;
						break;
					case type_flt:
					case type_int:
						if (*d < data[cc].prms[1])
							*d+= data[cc].prms[i];
						else
							*d = data[cc].prms[0];
						if (*d > data[cc].prms[1])
							*d = data[cc].prms[1];
						break;
					case type_pos:
						if (cx) {
							z = cx-1;
							if (d[z] < data[cc].prms[z*2+1])
								d[z] += (i-2)*9+1;
							else
								d[z] = data[cc].prms[z*2];
							if (d[z] > data[cc].prms[z*2+1])
								d[z] = data[cc].prms[z*2+1];
						} else {
							cx++;
						}
						break;
					case type_pal:
						if (cx) {
							if ((int)data[cc].ptr < 256)
								c = (char*)&fmcb->i[(int)data[cc].ptr];
							else
								c = (char*)data[cc].ptr;
							c[cx-1]+=(i-2)*7+1;
							if ((cx == 4) && (c[cx-1]>0x80)) c[cx-1] = 0;
						} else {
							cx++;
						}
						break;
					case type_sel:
						if (*d < data[cc].prms[1])
							*d+=1;
						else
							*d = data[cc].prms[0];
						break;
					case type_dir:
						getFilePath(c, FMB_FILE);
						if ((strlen(c)>=17)&&(c[0]==0x6D)&&(c[1]==0x63)&&(c[3]==0x3A)&&(c[4]==0x2F)&&(c[11]==0x2D))
							c[6]=0x3F;
						new_pad = PAD_CIRCLE;
						break;
				}
				if (co!=cs) continue;
			}
			if (new_pad & PAD_CROSS) {
				switch(data[cc].type) {
					case type_bol:
						*d ^= 1;
						break;
					case type_chr:
						c[0] = 0;
						break;
					case type_flt:
					case type_int:
						if (*d > data[cc].prms[0])
							*d-= data[cc].prms[i];
						else
							*d = data[cc].prms[1];
						if (*d < data[cc].prms[0])
							*d = data[cc].prms[0];
						break;
					case type_pos:
						if (cx) {
							z = cx-1;
							if (d[z] > data[cc].prms[z*2])
								d[z] -= (i-2)*9+1;
							else
								d[z] = data[cc].prms[z*2+1];
							if (d[z] < data[cc].prms[z*2])
								d[z] = data[cc].prms[z*2];
						} else {
							cx++;
						}
						break;
					case type_pal:
						if (cx) {
							if ((int)data[cc].ptr < 256)
								c = (char*)&fmcb->i[(int)data[cc].ptr];
							else
								c = (char*)data[cc].ptr;
							c[cx-1]-=(i-2)*7+1;
							if ((cx == 4) && (c[cx-1]>0x80)) c[cx-1] = 0x80;
						} else {
							cx++;
						}
						break;
					case type_sel:
						if (*d > data[cc].prms[0])
							*d-=1;
						else
							*d = data[cc].prms[1];
						break;
					case type_dir:
						c[0] = 0;
						break;
				}
			}
			if (new_pad & PAD_UP) {
				cy--;
				cx=0;
			} if (new_pad & PAD_DOWN) {
				cy++;
				cx=0;
			} if (new_pad & PAD_LEFT) {
				if (cx == 0)
					cy-=(MAX_ROWS+1)/2;
				else
					cx--;
			} if (new_pad & PAD_RIGHT) {
				if (cx == 0)
					cy+=(MAX_ROWS+1)/2;
				else if (data[cc].type==type_pal)
					cx++;
				else if (data[cc].type==type_pos) {
					cx++;
					if (cx > 2) cx = 2;
				}
			}
			if ((new_pad & (-(PAD_UP|PAD_DOWN|PAD_LEFT|PAD_RIGHT)-1)) && (data[cc].type != type_sub) && (data[cc].func)) {
			//if (new_pad && data[cc].func) {
				data[cc].func(msg0, data, cc);
			}
		}
		// リスト表示用変数の正規化
		if (cz > cd -MAX_ROWS)	cz = cd -MAX_ROWS;
		if (cz < 0)				cz = 0;
		if (cy >= cd)			cy = cd -1;
		if (cy < 0)				cy = 0;
		if (cy >= cz +MAX_ROWS)	cz = cy -MAX_ROWS +1;
		if (cy < cz)			cz = cy;
		if (cx > 4)				cx = 4;
		if (cx < 0)				cx = 0;
		
		// 画面描画開始
		if (redraw) {
			clrScr(setting->color[COLOR_BACKGROUND]);
			
			if (cd > MAX_ROWS) {
				void drawBar(int x1, int y1, int x2, int y2, uint64 color, int ofs, int len, int size);
				x = (MAX_ROWS_X+8)*FONT_WIDTH; y = SCREEN_MARGIN+FONT_HEIGHT*3;
				drawBar(x, y, x+FONT_WIDTH, y+FONT_HEIGHT*MAX_ROWS, setting->color[COLOR_FRAME], cz, MAX_ROWS, cd);
			}
			x = FONT_WIDTH *3;
			y = SCREEN_MARGIN +FONT_HEIGHT *3;
			for(i=0; i<MAX_ROWS; i++) {
				if (cz+i >= cd) break;
				if (!cx && cz+i == cy){
					color = setting->color[COLOR_HIGHLIGHTTEXT];
					printXY(">", x, y, color, TRUE);
				} else {
					color = setting->color[COLOR_TEXT];
				}
				z = cs+cz+i; a=0; b=0;
				if ((int)data[z].ptr < 256)
					c = (char*)&fmcb->i[(int)data[z].ptr];
				else
					c = (char*)data[z].ptr;
				if ((int)data[z].ptr < 256)
					d = (int*)&fmcb->i[(int)data[z].ptr];
				else
					d = (int*)data[z].ptr;
				switch(data[z].type) {
					case type_cnf:
					case type_sel:
						a = data[z].prms[2] + data[z].prms[3] * ((int*)data[z].ptr)[0];
						break;
					case type_bol:
						if ((int)data[z].ptr < 256)
							a = fmcb->i[(int)data[z].ptr];
						else
							a = ((int*)data[z].ptr)[0];
						if (a) a = (int)&lang->conf_on[0]; else a = (int)&lang->conf_off[0];
						break;
					case type_int:
						if ((int)data[z].ptr < 256)
							a = fmcb->i[(int)data[z].ptr];
						else
							a = ((int*)data[z].ptr)[0];
						break;
					case type_chr:
						if ((int)data[z].ptr < 256)
							a = (int)&fmcb->c[(int)data[z].ptr];
						else
							a = (int)data[z].ptr;
						break;
					case type_pos:
					case type_pal:
						break;
					case type_dir:
						a = data[z].prms[0];
						b = (int)data[z].ptr;
						break;
					case type_flt:
						if ((int)data[z].ptr < 256)
							a = fmcb->i[(int)data[z].ptr];
						else
							a = ((int*)data[z].ptr)[0];
						b = (a % 1000) / 100;
						a /= 1000;
					//case type_sub:
					//default:
				}
				sprintf(tmp, lang->fmcb[data[z].lang], a, b);
				printXY(tmp, x+FONT_WIDTH*2, y, color, TRUE);
				if (data[z].type == type_pos) {
					a = x+FONT_WIDTH*strlen(tmp) +FONT_WIDTH*2;
					for (b=0;b<2;b++){
						sprintf(tmp, "%c%c:%3d ", (cz+i==cy&&cx==b+1)?0x3E:0x20, 0x58+b, d[b]);
						printXY(tmp, a, y, (cz+i==cy&&cx==b+1)?setting->color[COLOR_HIGHLIGHTTEXT]:color, TRUE);
						a+=strlen(tmp)*FONT_WIDTH;
					}
				} else if (data[z].type == type_pal) {
					char pal[4] = "RGBA";
					b = x+FONT_WIDTH*strlen(tmp) +FONT_WIDTH*2;
					for (a=0;a<4;a++){
						sprintf(tmp, "%c%c:%02X ", (cy==cz+i&&cx==a+1)?0x3E:0x20, pal[a], c[a]);
						printXY(tmp, b, y, (cy==cz+i&&cx==a+1)?setting->color[COLOR_HIGHLIGHTTEXT]:color, TRUE);
						b+=strlen(tmp)*FONT_WIDTH;
					}
					itoSprite(*d, b, y, b+FONT_WIDTH, y+FONT_HEIGHT, 0);
					itoPrimAlphaBlending( TRUE );
					itoSprite(*d, b+FONT_WIDTH, y, b+FONT_WIDTH*2, y+FONT_HEIGHT, 0);
					itoPrimAlphaBlending( FALSE );
				}
				y += FONT_HEIGHT;
			}
			strcpy(msg1, lang->fmcb[data[cs+cy].help]);
			if (cx > 0) strcpy(msg1, lang->fmcb[59]);
			if (cs != 0) {
				strcat(msg1, " \x81\xA2:");
				strcat(msg1, lang->conf_up);
			}
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	return ret;
}

usr mnu_fmcb_cnf(char* msg0, SUBMENU* data, int num)
{
	int i,m,l,b[5];
	if (num < 4) m=0; else m=4;
	if (new_pad & PAD_L1) {
		tmpi[m]--;
		if (tmpi[m] < data[num].prms[0]) tmpi[m] = data[num].prms[1];
	} if (new_pad & PAD_R1) {
		tmpi[m]++;
		if (tmpi[m] > data[num].prms[1]) tmpi[m] = data[num].prms[0];
	} if (new_pad & PAD_CIRCLE) {
		for(i=0;i<5;i++) b[i]=tmpi[i];
		if (m) l=fmcb_save(fmcbpath[b[m]]); else l=fmcb_load(fmcbpath[b[m]]);
		for(i=0;i<5;i++) tmpi[i]=b[i];
		if (l>=0) {
			if (m) l=43; else l=41;
		} else {
			if (m) l=44; else l=42;
		}
		sprintf(msg0, lang->fmcb[l], fmcbpath[tmpi[m]]);
	}
	if (!m) {
		if (new_pad & PAD_L2) {
			fmcb_init();
			strcpy(msg0, lang->fmcb[49]);
		} if (new_pad & PAD_R2) {
			fmcb_init();
			fmcb_default();
			strcpy(msg0, lang->fmcb[50]);
		}
	}
}

usr mnu_fmcb_launcher(char* msg0, SUBMENU* data, int num)
{
	int i;
	char *c,*s;
	if (data[num].type == type_sub) {
		for(i=0;i<3;i++) strcpy(tmps[i+1], fmcb->launch[tmpi[1]].path[i]);
	} else if (data[num].type == type_dir) {
		// L1:MC変更 R1:機能登録 L2:コピー R2:ペースト (×:削除 ○:編集)
		//printf("mnu_fmcb_launcher: set path%d=%s\n", data[num].prms[0], tmps[data[num].prms[0]]);
		c = (char*)&tmps[data[num].prms[0]];
		s = (char*)&fmcb->launch[tmpi[1]].path[data[num].prms[0]-1];
		if (new_pad & PAD_L1) {
			if ((c[0]==0x6D)&&(c[1]==0x63)) {
				c[2]++;
				if (c[2] == 0x40) c[2]=0x30;
				if (c[2] > 0x31) c[2]=0x3F;
			}
		} else if (new_pad & PAD_R1) {
			if ((c[0]==0x4F)&&(c[3]==0x53)) {
				strcpy(c, "FASTBOOT");
			} else if (c[0] == 0x46) {
				strcpy(c, "OSDMENU");
			} else {
				strcpy(c, "OSDSYS");
			}
		} else if (new_pad & PAD_L2) {
			//strcpy(tmps[data[num].prms[0]+4], c);
			strcpy(tmps[4], c);
			strcpy(msg0, lang->conf_button_copied);
		} else if (new_pad & PAD_R2) {
			strcpy(c, tmps[4]);
			strcpy(msg0, lang->conf_button_pasted);
		}
		strcpy(s, c);
	} else {
		if (new_pad & PAD_L1) {
			if (tmpi[1] > 0) tmpi[1]--;
			else tmpi[1] = MAX_BOOT_BUTTONS-1;
		} if (new_pad & PAD_R1) {
			if (tmpi[1] < MAX_BOOT_BUTTONS-1) tmpi[1]++;
			else tmpi[1] = 0;
		}
		if (new_pad & PAD_L2) {
			for(i=0;i<3;i++)
				strcpy(tmps[i+5], tmps[i+1]);
			strcpy(msg0, lang->conf_button_copied);
		} if (new_pad & PAD_R2) {
			for(i=0;i<3;i++){
				strcpy(tmps[i+1], tmps[i+5]);
				strcpy(fmcb->launch[tmpi[1]].path[i], tmps[i+5]);
			}
			strcpy(msg0, lang->conf_button_pasted);
		} 
		if (new_pad&PAD_CIRCLE||new_pad&PAD_CROSS||new_pad&PAD_L1||new_pad&PAD_R1) {
		//if (new_pad&PAD_CIRCLE||new_pad&PAD_CROSS) {
			for(i=0;i<3;i++) strcpy(tmps[i+1], fmcb->launch[tmpi[1]].path[i]);
		}
	}
}

usr mnu_fmcb_osditem(char* msg0, SUBMENU* data, int num)
{
	int i;
	char *c,*s;
	if (data[num].type == type_sub) {
		strcpy(tmps[0], fmcb->osditem[tmpi[3]-1].title);
		for(i=0;i<3;i++) strcpy(tmps[i+1], fmcb->osditem[tmpi[3]-1].path[i]);
	} else if (data[num].type == type_dir) {
		// L1:MC変更 R1:機能登録 L2:コピー R2:ペースト (×:削除 ○:編集)
		//printf("mnu_fmcb_launcher: set path%d=%s\n", data[num].prms[0], tmps[data[num].prms[0]]);
		c = (char*)&tmps[data[num].prms[0]];
		s = (char*)&fmcb->osditem[tmpi[3]-1].path[data[num].prms[0]-1];
		if (new_pad & PAD_L1) {
			if ((c[0]==0x6D)&&(c[1]==0x63)) {
				c[2]++;
				if (c[2] == 0x40) c[2]=0x30;
				if (c[2] > 0x31) c[2]=0x3F;
			}
		} else if (new_pad & PAD_R1) {
			if (c[0]==0x4F) {
				strcpy(c, "FASTBOOT");
			} else {
				strcpy(c, "OSDSYS");
			}
		} else if (new_pad & PAD_L2) {
			//strcpy(tmps[data[num].prms[0]+4], c);
			strcpy(tmps[4], c);
			strcpy(msg0, lang->conf_button_copied);
		} else if (new_pad & PAD_R2) {
			strcpy(c, tmps[4]);
			strcpy(msg0, lang->conf_button_pasted);
		}
		strcpy(s, c);
	} else if (data[num].type == type_chr) {
		if (new_pad & PAD_L2) {
			strcpy(tmps[12], tmps[0]);
			strcpy(msg0, lang->conf_button_copied);
		} if (new_pad & PAD_R2) {
			strcpy(tmps[0], tmps[12]);
			strcpy(msg0, lang->conf_button_pasted);
		}
		strcpy(fmcb->osditem[tmpi[3]-1].title, tmps[0]);
	} else {
		if (new_pad & PAD_L1) {
			if (tmpi[3] > 1) tmpi[3]--;
			else tmpi[3] = MAX_OSDSYS_ITEMS;
		} if (new_pad & PAD_R1) {
			if (tmpi[3] < MAX_OSDSYS_ITEMS) tmpi[3]++;
			else tmpi[3] = 1;
		}
		if (new_pad & PAD_L2) {
			for(i=0;i<4;i++)
				strcpy(tmps[i+8], tmps[i]);
			strcpy(msg0, lang->conf_button_copied);
		} if (new_pad & PAD_R2) {
			strcpy(tmps[0], tmps[8]);
			strcpy(fmcb->osditem[tmpi[3]-1].title, tmps[8]);
			for(i=0;i<3;i++) {
				strcpy(tmps[i+1], tmps[i+9]);
				strcpy(fmcb->osditem[tmpi[3]-1].path[i], tmps[i+9]);
			}
			strcpy(msg0, lang->conf_button_pasted);
		} 
		if (new_pad&PAD_CIRCLE||new_pad&PAD_CROSS||new_pad&PAD_L1||new_pad&PAD_R1) {
		//if (new_pad&PAD_CIRCLE||new_pad&PAD_CROSS) {
			strcpy(tmps[0], fmcb->osditem[tmpi[3]-1].title);
			for(i=0;i<3;i++) strcpy(tmps[i+1], fmcb->osditem[tmpi[3]-1].path[i]);
		}
	}
}

usr mnu_fmcb_esr(char* msg0, SUBMENU* data, int num)
{
	int i;
	char *c;
	if (data[num].type == type_sub) {
		for(i=0;i<3;i++) strcpy(tmps[i+1], fmcb->esr.path[i]);
	} else if (data[num].type == type_dir) {
		c = (char*)&tmps[data[num].prms[0]];
		if (new_pad & PAD_SQUARE || new_pad & PAD_L1) {
			if ((c[0]==0x6D)&&(c[1]==0x63)) {
				c[2]++;
				if (c[2] == 0x40) c[2]=0x30;
				if (c[2] > 0x31) c[2]=0x3F;
			}
		} else if (new_pad & PAD_L2) {
			strcpy(tmps[13], c);
			strcpy(msg0, lang->conf_button_copied);
		} else if (new_pad & PAD_R2) {
			strcpy(c, tmps[13]);
			strcpy(msg0, lang->conf_button_pasted);
		}
		strcpy(fmcb->esr.path[data[num].prms[0]-1], c);
	}
	return (usr) 0;
}

int fmcb_init()
{	// 初期値の読み込み
	int i,k;
	if (fmcb == NULL) fmcb=(FMCB_SETTING*)malloc(sizeof(FMCB_SETTING));
	if (fmcb == NULL) return -1;
	
	for (i=0; i<MAX_INT_KEYS; i++) fmcb->i[i] = fmcbinit_int[i];
	for (i=0; i<MAX_CHAR_KEYS; i++) strcpy(fmcb->c[i], fmcbinit_char[i]);
	fmcb->videomode = 0;
	for (i=0; i<MAX_BOOT_BUTTONS; i++)
		for (k=0; k<MAX_REGIST_ELF; k++)
			fmcb->launch[i].path[k][0] = 0;
	for (i=0; i<MAX_OSDSYS_ITEMS; i++) {
		fmcb->osditem[i].title[0] = 0;
		for (k=0; k<MAX_REGIST_ELF; k++)
			fmcb->osditem[i].path[k][0] = 0;
	}
	for (i=0; i<MAX_REGIST_ELF; i++)
		fmcb->esr.path[i][0] = 0;
	return 0;
}
int fmcb_default()
{
	int i,k;
	if (fmcb == NULL) fmcb=(FMCB_SETTING*)malloc(sizeof(FMCB_SETTING));
	if (fmcb == NULL) return -1;
	
	// LAUNCHER SETTINGS
	for (i=0; i<MAX_BOOT_BUTTONS; i++) {
		strcpy(fmcb->launch[i].path[0], "OSDSYS");
		for (k=1; k<MAX_REGIST_ELF; k++)
			fmcb->launch[i].path[k][0] = 0;
	}
	for (i=0; i<4; i++) {
		for (k=0; k<3; k++) {
			//strcpy(fmcb->launch[i+5].path[k], fmcbdefault3[k]);
			//strcat(fmcb->launch[i+5].path[k], fmcbdefault4[i]);
			sprintf(fmcb->launch[i+5].path[k], "%s%s", fmcbdefault3[k], fmcbdefault4[i]);
			//printf("i=%d,k=%d:%-40s (%-20s,%-16s)\n", i, k, fmcb->launch[i+5].path[k], fmcbdefault3[k], fmcbdefault4[i]);
		}
	}
	strcpy(fmcb->launch[15].path[0], "mc?:/SYS-CONF/FMCB_CFG.ELF");
	strcpy(fmcb->launch[15].path[1], "OSDSYS");
	
	// OSDSYS ITEM
	for (i=0; i<4; i++) {
		strcpy(fmcb->osditem[i].title, fmcbdefault1[i]);
		for (k=0; k<3; k++) {
			//strcpy(fmcb->osditem[i].path[k], fmcbdefault3[k]);
			//strcat(fmcb->osditem[i].path[k], fmcbdefault2[i]);
			sprintf(fmcb->osditem[i].path[k], "%s%s", fmcbdefault3[k], fmcbdefault2[i]);
		}
	}
	strcpy(fmcb->osditem[MAX_OSDSYS_ITEMS-1].title, "Free McBoot Configurator");
	strcpy(fmcb->osditem[MAX_OSDSYS_ITEMS-1].path[0], "mc?:/SYS-CONF/FMCB_CFG.ELF");
	
	// ESR
	for (i=0; i<3; i++) {
		//strcpy(fmcb->esr.path[i], fmcbdefault3[i]);
		//strcat(fmcb->esr.path[i], fmcbdefault2[1]);
		sprintf(fmcb->esr.path[i], "%s%s", fmcbdefault3[i], fmcbdefault2[1]);
	}
	
	return 0;
}

int cnf_setkey(const char *key, char *str)
{
	if (str[0] == 0) {
		return cnf_delkey(key);
	} else {
		return cnf_setstr(key, str);
	}
}

int fmcb_load(char *path)
{	// FREEMCB.CNFの読み込み
	// fmcbkeys_int => fmcb->i[];
	// fmcbkeys_char => fmcb->c[];
	
	int i,k,ret;
	char tmp[1024];
	if (fmcb == NULL) fmcb=(FMCB_SETTING*)malloc(sizeof(FMCB_SETTING));
	if (fmcb == NULL) return -1;
	cnf_init();
	cnf_mode(1);
	fmcb_init();
	if ((ret=cnf_load(path))>=0) {
		// int型のデータ
		for(i=0; i<MAX_INT_KEYS; i++) {
			cnf_getstr(fmcbkeys_int[i], tmp, fmcbvmode[3]);
			if (i==selectedcolor || i==unselectedcolor) {
				explodeconfig(tmp);
				//fmcb->i[i] = (tmpi[--k]<<24)|(tmpi[--k]<<16)|(tmpi[--k]<<8)|tmpi[--k];
				fmcb->i[i] = (tmpi[3]<<24)|(tmpi[2]<<16)|(tmpi[1]<<8)|tmpi[0];
				printf("(int)%3d:%-32s=0x%08X(%3d,%3d,%3d,%3d)\n", i, fmcbkeys_int[i], fmcb->i[i], tmpi[0], tmpi[1], tmpi[2], tmpi[3]);
			} else {
				fmcb->i[i] = strtol(tmp, NULL, 0);
				printf("(int)%3d:%-32s=%10d\n", i, fmcbkeys_int[i], fmcb->i[i]);
			}
		}
		// char型のデータ
		for(i=0; i<MAX_CHAR_KEYS; i++) {
			cnf_getstr(fmcbkeys_char[i], fmcb->c[i], "");
			printf("(str)%3d:%-32s=%s\n", i, fmcbkeys_char[i], fmcb->c[i]);
		}
		// 映像出力
		cnf_getstr("OSDSYS_video_mode", tmp, "");
		for(i=0;i<3;i++) {
			if (strcmp(tmp, fmcbvmode[i]) == 0) {
				fmcb->videomode = i;
				break;
			}
		}
		printf("(chr) videomode=%d (%s -> %s)\n", fmcb->videomode, tmp, fmcbvmode[fmcb->videomode]);
		// ランチャーの設定
		for(i=0;i<MAX_BOOT_BUTTONS;i++) {
			//	fmcb_buttons[]->key
			for(k=0;k<MAX_REGIST_ELF;k++) {
				sprintf(tmp, "LK_%s_E%d", fmcb_buttons[i].key, k+1);
				cnf_getstr(tmp, fmcb->launch[i].path[k], "");
			}
			printf("(pad) %-8s: %s,%s,%s\n", fmcb_buttons[i].key, fmcb->launch[i].path[0], fmcb->launch[i].path[1], fmcb->launch[i].path[2]);
		}
		// ESRの設定
		for(i=0;i<MAX_REGIST_ELF;i++) {
			sprintf(tmp, "ESR_Path_E%d", i+1);
			cnf_getstr(tmp, fmcb->esr.path[i], "");
		}
		printf("(esr) %s,%s,%s\n", fmcb->esr.path[0], fmcb->esr.path[1], fmcb->esr.path[2]);
		// メニューアイテムの設定
		for(i=0;i<MAX_OSDSYS_ITEMS;i++) {
			sprintf(tmp, "name_OSDSYS_ITEM_%d", i+1);
			cnf_getstr(tmp, fmcb->osditem[i].title, "");
			for(k=0;k<MAX_REGIST_ELF;k++) {
				sprintf(tmp, "path%d_OSDSYS_ITEM_%d", k+1, i+1);
				cnf_getstr(tmp, fmcb->osditem[i].path[k], "");
			}
			printf("(osd)%3d [%s] %s,%s,%s\n", i, fmcb->osditem[i].title, fmcb->osditem[i].path[0], fmcb->osditem[i].path[1], fmcb->osditem[i].path[2]);
		}
	}
	cnf_free();
	return ret;
}

int fmcb_save(char *path)
{	// FREEMCB.CNFへの書き込み
	// 戻り値=0:成功,=-1:失敗
	int i,k;
	char tmp[1024], tmpval[1024];
	if (fmcb == NULL) fmcb=(FMCB_SETTING*)malloc(sizeof(FMCB_SETTING));
	if (fmcb == NULL) return -1;
	
	cnf_init();
	cnf_mode(1);
	cnf_load(path);
	
	// cnf_setstr(key, str)	空の文字列でもキーを削除しない
	// cnf_setkey(key, str)	空の文字列ならキーを削除する
	// cnf_delkey(key)		キーを削除する
	
	// int型データ
	for(i=0; i<MAX_INT_KEYS; i++) {
		if (i==selectedcolor || i==unselectedcolor) {
			sprintf(tmpval, "0x%02X,0x%02X,0x%02X,0x%02X",
			 		fmcb->i[i] & 0xFF,
			 		(fmcb->i[i] >> 8) & 0xFF,
			 		(fmcb->i[i] >>16) & 0xFF,
			 		(fmcb->i[i] >>24) & 0xFF);
		} else {
			sprintf(tmpval, "%d", fmcb->i[i]);
		}
		cnf_setstr(fmcbkeys_int[i], tmpval);
		printf("(int)%3d:%s = %s\n", i, fmcbkeys_int[i], tmpval);
	}
	// char型データ
	for(i=0; i<MAX_CHAR_KEYS; i++) {
		cnf_setstr(fmcbkeys_char[i], fmcb->c[i]);
		printf("(str)%3d:%s = %s\n", i, fmcbkeys_char[i], fmcb->c[i]);
	}
	// 映像出力
	cnf_setstr("OSDSYS_video_mode", fmcbvmode[fmcb->videomode & 3]);
	// ランチャー設定
	for(i=0;i<MAX_BOOT_BUTTONS;i++) {
		for(k=0;k<MAX_REGIST_ELF;k++) {
			sprintf(tmp, "LK_%s_E%d", fmcb_buttons[i].key, k+1);
			if (i>=5&&i<=8)
				cnf_setstr(tmp, fmcb->launch[i].path[k]);
			else
				cnf_setkey(tmp, fmcb->launch[i].path[k]);
		}
	}
	// メニュー設定
	for(i=0;i<MAX_OSDSYS_ITEMS;i++) {
		sprintf(tmp, "name_OSDSYS_ITEM_%d", i+1);
		cnf_setkey(tmp, fmcb->osditem[i].title);
		for(k=0;k<MAX_REGIST_ELF;k++) {
			sprintf(tmp, "path%d_OSDSYS_ITEM_%d", k+1, i+1);
			cnf_setkey(tmp, fmcb->osditem[i].path[k]);
		}
	}
	
	// ESR設定
	for(i=0;i<MAX_REGIST_ELF;i++) {
		sprintf(tmp, "ESR_Path_E%d", i+1);
		cnf_setstr(tmp, fmcb->esr.path[i]);
	}
	
	k=cnf_save(path);
	cnf_free();
	return k;
}

void fmcb_cfg(char *mainMsg)
{
	int i,k;
	
	// メモリ確保
	if (fmcb == NULL) fmcb=(FMCB_SETTING*)malloc(sizeof(FMCB_SETTING));
	if (fmcb == NULL) {
		strcpy(mainMsg, lang->editor_viewer_error2);
		return;
	}
	// アプリ名変更
	strcpy(LBF_VER, "FMCB Configurator in " LBFN_VER);

	// FREEMCB.CNFの自動読み込み
	for(i=0,k=0;i<3+(boot==HOST_BOOT);i++){
		k=fmcb_load(fmcbpath[i]);
		if (k==0) break;
	}
	if (!k) {
		sprintf(mainMsg, lang->fmcb[41], fmcbpath[i]);
	}
	
	if (boot == HOST_BOOT) {
		mnu_fmcb[0].prms[1] = 3;
		mnu_fmcb[7].prms[1] = 3;
	}
	// 初期値のコピー
	// tmpi:
	//	[0] == セーブ/ロードフォルダ選択
	//	[1] == ボタン
	//	[2] == 映像出力
	//	[3] == アイテム番号
	// tmps:
	//	[0] == ランチャー登録1〜3
	//	[3] == アイテム表示名
	//	[4] == アイテム登録1〜3
	//	[7] == ESR登録1〜3
	tmpi[0] = tmpi[4] = 1;
	tmpi[1] = 0;
	tmpi[2] = fmcb->videomode;
	tmpi[3] = 1;
	
	// メニュー
	i = menu_player(mnu_fmcb, mainMsg, 51);
	
	// メモリ開放
	free(fmcb);	fmcb = NULL;
	if (fmcb_backup != NULL) {
		free(fmcb_backup);
		fmcb_backup = NULL;
	}
	// アプリ名復帰
	strcpy(LBF_VER, LBFN_VER);
	mainMsg[0]=0;
	return;
}
