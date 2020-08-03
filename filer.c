#include "launchelf.h"

//PS2TIME LaunchELF 3.80
typedef struct
{
	unsigned char unknown;
	unsigned char sec;      // date/time (second)
	unsigned char min;      // date/time (minute)
	unsigned char hour;     // date/time (hour)
	unsigned char day;      // date/time (day)
	unsigned char month;    // date/time (month)
	unsigned short year;    // date/time (year)
} PS2TIME __attribute__((aligned (2)));

typedef struct{
	PS2TIME modifyTime;
    unsigned fileSizeByte;
	unsigned short attr;
	char title[16*4+1];
	char name[256];
	int type;
} FILEINFO;

// psuファイルヘッダ構造体
typedef struct { // 512 bytes
	unsigned short  attr;
	unsigned short  unknown1;
	unsigned int  size;	//file size, 0 for directory
	unsigned char createtime[8];	//0x00:sec:min:hour:day:month:year
	unsigned int unknown2;
	unsigned int unknown3;
	unsigned char modifytime[8];	//0x00:sec:min:hour:day:month:year
	unsigned char unknown4[32];
	unsigned char name[32];
	unsigned char unknown5[416];
} PSU_HEADER;

//FILEINFO type
enum
{
	TYPE_OTHER,
	TYPE_DIR,
	TYPE_FILE,
	TYPE_PS2SAVE,
	TYPE_ELF
};

//menu
enum
{
	COPY,
	CUT,
	PASTE,
	DELETE,
	RENAME,
	NEWDIR,
	GETSIZE,
	EXPORT,
	IMPORT,
	NUM_MENU
};

// ASCIIとSJISの変換用配列
const unsigned char sjis_lookup_81[256] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x00
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x10
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x20
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x30
  ' ', ',', '.', ',', '.', 0xFF,':', ';', '?', '!', 0xFF,0xFF,'ｴ', '`', 0xFF,'^',   // 0x40
  0xFF,'_', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'0', 0xFF,'-', '-', 0xFF,0xFF,  // 0x50
  0xFF,0xFF,0xFF,0xFF,0xFF,'\'','\'','"', '"', '(', ')', 0xFF,0xFF,'[', ']', '{',   // 0x60
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'+', '-', 0xFF,'*', 0xFF,  // 0x70
  '/', '=', 0xFF,'<', '>', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'ｰ', 0xFF,0xFF,'ｰ', 0xFF,  // 0x80
  '$', 0xFF,0xFF,'%', '#', '&', '*', '@', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x90
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xA0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xB0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'&', '|', '!', 0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xC0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xD0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xE0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xF0
};
const unsigned char sjis_lookup_82[256] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x00
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x10
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x20
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x30
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,'0',   // 0x40
  '1', '2', '3', '4', '5', '6', '7', '8', '9', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x50
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',   // 0x60
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x70
  0xFF,'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',   // 0x80
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0xFF,0xFF,0xFF,0xFF,0xFF,  // 0x90
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xA0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xB0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xC0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xD0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xE0
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,  // 0xF0
};

int cut;
int nclipFiles, nmarks, nparties;
int title;
char mountedParty[2][MAX_NAME];
char parties[MAX_PARTITIONS][MAX_NAME];
char clipPath[MAX_PATH], LastDir[MAX_NAME], marks[MAX_ENTRY];
FILEINFO clipFiles[MAX_ENTRY];
int fileMode =  FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

#define MAX_ARGC 3
int psb_argc;
char psb_argv[MAX_ARGC][MAX_PATH+2];

//-------------------------------------------------
//拡張子を取得
char* getExtension(const char *path)
{
	return strrchr(path,'.');
}

//-------------------------------------------------
// HDDのパーティションとパスを取得
int getHddParty(const char *path, const FILEINFO *file, char *party, char *dir)
{
	char fullpath[MAX_PATH], *p;
	
	if(strncmp(path,"hdd",3)) return -1;
	
	strcpy(fullpath, path);
	if(file!=NULL){
		strcat(fullpath, file->name);
		if(file->attr & FIO_S_IFDIR) strcat(fullpath,"/");
	}
	if((p=strchr(&fullpath[6], '/'))==NULL) return -1;
	if(dir!=NULL) sprintf(dir, "pfs0:%s", p);
	*p=0;
	if(party!=NULL) sprintf(party, "hdd0:%s", &fullpath[6]);
	
	return 0;
}

//-------------------------------------------------
// パーティションのマウント
int mountParty(const char *party)
{
	if(!strcmp(party, mountedParty[0]))
		return 0;
	else if(!strcmp(party, mountedParty[1]))
		return 1;
	
	fileXioUmount("pfs0:"); mountedParty[0][0]=0;
	if(fileXioMount("pfs0:", party, FIO_MT_RDWR) < 0) return -1;
	strcpy(mountedParty[0], party);
	return 0;
}

//-------------------------------------------------
// 確認ダイアログ
int ynDialog(const char *message, int defaultsel)
{
	char msg[2048];
	int dh, dw, dx, dy;
	int sel=0, n, tw;//, a=6, b=4, c=2;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	sel=defaultsel;
	strcpy(msg, message);

	//\n区切りを\0区切りに変換 n:改行の数
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//表示する文字列の最大の幅を調べる tw:幅
	for(i=len=tw=0; i<n; i++){
		ret = printXY(&msg[len], 0, 0, 0, FALSE);	//表示する文字列の幅を調べる
		if(ret>tw) tw=ret;
		len=strlen(&msg[len])+1;
	}

	if(tw<FONT_WIDTH*22) tw=FONT_WIDTH*22;	//幅の最小値

	x_margin = FONT_WIDTH;				//左右のマージン
	y_margin = FONT_HEIGHT/2;			//上下のマージン
	dw = tw+x_margin*2;				//ダイアログの幅
	dh = FONT_HEIGHT*(n+2)+y_margin*2;	//ダイアログの高さ
	dx = (SCREEN_WIDTH-dw)/2;			//ダイアログのx
	dy = (SCREEN_HEIGHT-dh)/2;			//ダイアログのy
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_LEFT){
				sel=0;	//OK
			}else if(new_pad & PAD_RIGHT){
				sel=1;	//CANCEL
			}else if(new_pad & PAD_CROSS){
				ret=-1;
				break;
			}else if(new_pad & PAD_CIRCLE){
				if(sel==0) ret=1;
				else	   ret=-1;
				break;
			}
			else if(new_pad & PAD_SELECT){
				sel=1;	//CANCEL
			}
			else if(new_pad & PAD_START){
				sel=0;	//OK
			}
		}
		//描画開始
		//メッセージ消す
		itoSprite(setting->color[0],
			0, SCREEN_MARGIN+FONT_HEIGHT,
			SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
		//背景
		drawDialogTmp(dx, dy,
			dx+dw, dy+dh,
			setting->color[0], setting->color[1]);
		//メッセージ
		for(i=len=0; i<n; i++){
			printXY(&msg[len], dx+x_margin, (dy+y_margin+i*FONT_HEIGHT), setting->color[3],TRUE);
			len=strlen(&msg[len])+1;
		}
		//OKとCANCEL
		x=(tw-FONT_WIDTH*22)/2;
		sprintf(tmp, " %-10s %-10s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3],TRUE);
		//カーソル
		if(sel==0)
			printXY(">", dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3], TRUE);
		else
			printXY(">",dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3], TRUE);
		drawScr();
	}
/*
	//
	x=(tw-FONT_WIDTH*12)/2;
	drawChar(' ', dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
	drawChar(' ',dx+x_margin+x+FONT_WIDTH*11, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3]);
*/
	return ret;
}

//-------------------------------------------------
// メッセージダイアログ
void MessageDialog(const char *message)
{
	char msg[2048];
	int dh, dw, dx, dy;
	int n, tw;
	int i, x, len, ret;
	int x_margin;
	int y_margin;
	char tmp[MAX_PATH];

	strcpy(msg, message);

	//\n区切りを\0区切りに変換 n:改行の数
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//表示する文字列の最大の幅を調べる tw:幅
	for(i=len=tw=0; i<n; i++){
		ret = printXY(&msg[len], 0, 0, 0, FALSE);	//表示する文字列の幅を調べる
		if(ret>tw) tw=ret;
		len=strlen(&msg[len])+1;
	}

	if(tw<FONT_WIDTH*10) tw=FONT_WIDTH*10;	//幅の最小値

	x_margin = FONT_WIDTH;				//左右のマージン
	y_margin = FONT_HEIGHT/2;			//上下のマージン
	dw = tw+x_margin*2;				//ダイアログの幅
	dh = FONT_HEIGHT*(n+2)+y_margin*2;	//ダイアログの高さ
	dx = (SCREEN_WIDTH-dw)/2;			//ダイアログのx
	dy = (SCREEN_HEIGHT-dh)/2;			//ダイアログのy
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_CIRCLE){
				break;
			}
		}
		//描画開始
		//背景
		drawDialogTmp(dx, dy,
			dx+dw, dy+dh,
			setting->color[0], setting->color[1]);
		//メッセージ
		for(i=len=0; i<n; i++){
			printXY(&msg[len], dx+x_margin, (dy+y_margin+i*FONT_HEIGHT), setting->color[3],TRUE);
			len=strlen(&msg[len])+1;
		}
		x=(tw-FONT_WIDTH*10)/2;
		//OK
		sprintf(tmp, "○: %s", lang->gen_ok);
		printXY(tmp, dx+x_margin+x, (dy+y_margin+(n+1)*FONT_HEIGHT), setting->color[3],TRUE);
		drawScr();
	}
	return;
}

//-------------------------------------------------
// クイックソート
int cmpFile(FILEINFO *a, FILEINFO *b)
{
	unsigned char *p, ca, cb;
	int i, n, ret, aElf=FALSE, bElf=FALSE, t=title;
	
	if(a->attr==b->attr){
		if(a->attr & FIO_S_IFREG){
			p = strrchr(a->name, '.');
			if(p!=NULL && !stricmp(p+1, "ELF")) aElf=TRUE;
			p = strrchr(b->name, '.');
			if(p!=NULL && !stricmp(p+1, "ELF")) bElf=TRUE;
			if(aElf && !bElf)		return -1;
			else if(!aElf && bElf)	return 1;
			t=FALSE;
		}
		if(t){
			if(a->title[0]!=0 && b->title[0]==0) return -1;
			else if(a->title[0]==0 && b->title[0]!=0) return 1;
			else if(a->title[0]==0 && b->title[0]==0) t=FALSE;
		}
		if(t) n=strlen(a->title);
		else  n=strlen(a->name);
		for(i=0; i<=n; i++){
			if(t){
				ca=a->title[i]; cb=b->title[i];
			}else{
				ca=a->name[i]; cb=b->name[i];
				if(ca>='a' && ca<='z') ca-=0x20;
				if(cb>='a' && cb<='z') cb-=0x20;
			}
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->attr & FIO_S_IFDIR)	return -1;
	else						return 1;
}
void sort(FILEINFO *a, int left, int right)
{
	FILEINFO tmp, pivot;
	int i, p;
	
	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

//-------------------------------------------------
// メモリーカード読み込み
int readMC(const char *path, FILEINFO *info, int max)
{
	static mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	char dir[MAX_PATH];
	int i, j, ret;
	
	mcSync(0,NULL,NULL);
	
	strcpy(dir, &path[4]); strcat(dir, "*");
	mcGetDir(path[2]-'0', 0, dir, 0, MAX_ENTRY-2, mcDir);
	mcSync(0, NULL, &ret);
	
	for(i=j=0; i<ret; i++)
	{
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR &&
		(!strcmp(mcDir[i].name, ".") || !strcmp(mcDir[i].name, "..")))
			continue;
		strcpy(info[j].name, mcDir[i].name);
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		info[j].fileSizeByte = mcDir[i].fileSizeByte;
		info[j].modifyTime.unknown = mcDir[i]._modify.unknown2;
		info[j].modifyTime.sec = mcDir[i]._modify.sec;
		info[j].modifyTime.min = mcDir[i]._modify.min;
		info[j].modifyTime.hour = mcDir[i]._modify.hour;
		info[j].modifyTime.day = mcDir[i]._modify.day;
		info[j].modifyTime.month = mcDir[i]._modify.month;
		info[j].modifyTime.year = mcDir[i]._modify.year;
		j++;
	}
	
	return j;
}

//-------------------------------------------------
// CD読み込み
int readCD(const char *path, FILEINFO *info, int max)
{
	static struct TocEntry TocEntryList[MAX_ENTRY];
	char dir[MAX_PATH];
	int i, j, n;
	
	loadCdModules();
	
	strcpy(dir, &path[5]);
	CDVD_FlushCache();
	n = CDVD_GetDir(dir, NULL, CDVD_GET_FILES_AND_DIRS, TocEntryList, MAX_ENTRY, dir);
	
	for(i=j=0; i<n; i++)
	{
		if(TocEntryList[i].fileProperties & 0x02 &&
		 (!strcmp(TocEntryList[i].filename, ".") ||
		  !strcmp(TocEntryList[i].filename, "..")))
			continue;
		strcpy(info[j].name, TocEntryList[i].filename);
		if(TocEntryList[i].fileProperties & 0x02)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		info[j].fileSizeByte = TocEntryList[i].fileSize;
		memset(&info[j].modifyTime, 0, sizeof(PS2TIME)); //取得できない
		j++;
	}
	
	return j;
}

//-------------------------------------------------
// パーティションリスト設定
void setPartyList(void)
{
	iox_dirent_t dirEnt;
	int hddFd;
	
	nparties=0;
	
	if((hddFd=fileXioDopen("hdd0:")) < 0)
		return;
	while(fileXioDread(hddFd, &dirEnt) > 0)
	{
		if(nparties >= MAX_PARTITIONS)
			break;
		if((dirEnt.stat.attr & ATTR_SUB_PARTITION) 
				|| (dirEnt.stat.mode == FS_TYPE_EMPTY))
			continue;
		if(!strncmp(dirEnt.name, "PP.HDL.", 7))
			continue;
		if(!strncmp(dirEnt.name, "__", 2) && strcmp(dirEnt.name, "__boot"))
			continue;
		
		strcpy(parties[nparties++], dirEnt.name);
	}
	fileXioDclose(hddFd);
}

//-------------------------------------------------
// HDD読み込み
int readHDD(const char *path, FILEINFO *info, int max)
{
	iox_dirent_t dirbuf;
	char party[MAX_PATH], dir[MAX_PATH];
	int i=0, fd, ret;
	
	if(nparties==0){
		loadHddModules();
		setPartyList();
	}
	
	if(!strcmp(path, "hdd0:/")){
		for(i=0; i<nparties; i++){
			strcpy(info[i].name, parties[i]);
			info[i].attr = FIO_S_IFDIR;
			info[i].fileSizeByte = 0;
			memset(&info[i].modifyTime, 0, sizeof(PS2TIME));
		}
		return nparties;
	}
	
	getHddParty(path, NULL, party, dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';
	
	if((fd=fileXioDopen(dir)) < 0) return 0;
	
	while(fileXioDread(fd, &dirbuf)){
		if(dirbuf.stat.mode & FIO_S_IFDIR &&
		(!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;
		
		info[i].attr = dirbuf.stat.mode;
		strcpy(info[i].name, dirbuf.name);
		info[i].fileSizeByte = dirbuf.stat.size;
		info[i].modifyTime.unknown = dirbuf.stat.mtime[0];
		info[i].modifyTime.sec = dirbuf.stat.mtime[1];
		info[i].modifyTime.min = dirbuf.stat.mtime[2];
		info[i].modifyTime.hour = dirbuf.stat.mtime[3];
		info[i].modifyTime.day = dirbuf.stat.mtime[4];
		info[i].modifyTime.month = dirbuf.stat.mtime[5];
		info[i].modifyTime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
		i++;
		if(i==max) break;
	}
	
	fileXioDclose(fd);
	
	return i;
}

//-------------------------------------------------
// USBマスストレージ読み込み
int readMASS(const char *path, FILEINFO *info, int max)
{
	fio_dirent_t record;
	int n=0, dd=-1;

	loadUsbModules();
	
	if ((dd = fioDopen(path)) < 0) goto exit;

	while(fioDread(dd, &record) > 0){
		if((FIO_SO_ISDIR(record.stat.mode))
			&& (!strcmp(record.name,".") || !strcmp(record.name,".."))
		) continue;

		strcpy(info[n].name, record.name);
		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = FIO_S_IFDIR;
		}
		else if(FIO_SO_ISREG(record.stat.mode)){
			info[n].attr = FIO_S_IFREG;
			info[n].fileSizeByte = record.stat.size;
		}
		else
			continue;
		strncpy(info[n].name, info[n].name, 32);
		info[n].modifyTime.unknown = 0;
		info[n].modifyTime.sec = record.stat.mtime[1];
		info[n].modifyTime.min = record.stat.mtime[2];
		info[n].modifyTime.hour = record.stat.mtime[3];
		info[n].modifyTime.day = record.stat.mtime[4];
		info[n].modifyTime.month = record.stat.mtime[5];
		info[n].modifyTime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
		n++;
		if(n==max) break;
	}

exit:
	if(dd >= 0) fioDclose(dd);
	return n;
}

//-------------------------------------------------
// ファイルリスト取得
int getDir(const char *path, FILEINFO *info)
{
	int max=MAX_ENTRY-2;
	int n;
	
	if(!strncmp(path, "mc", 2))
		n=readMC(path, info, max);
	else if(!strncmp(path, "hdd", 3))
		n=readHDD(path, info, max);
	else if(!strncmp(path, "mass", 4))
		n=readMASS(path, info, max);
	else if(!strncmp(path, "cdfs", 4))
		n=readCD(path, info, max);
	else
		return 0;
	
	return n;
}

//-------------------------------------------------
// セーブデータタイトルの取得
int getGameTitle(const char *path, const FILEINFO *file, char *out)
{
	iox_dirent_t dirEnt;
	char party[MAX_NAME], dir[MAX_PATH];
	int fd=-1, dirfd=-1, size, hddin=FALSE, ret;
	
	if(file->attr & FIO_S_IFREG) return -1;
	if(path[0]==0 || !strcmp(path, "hdd0:/")) return -1;
	
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path, file, party, dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3]=ret+'0';
		hddin=TRUE;
	}else
		sprintf(dir, "%s%s/", path, file->name);
	
	ret = -1;
	if(hddin){
		if((dirfd=fileXioDopen(dir)) < 0) goto error;
		while(fileXioDread(dirfd, &dirEnt)){
			if(dirEnt.stat.mode & FIO_S_IFREG &&
			 !strcmp(dirEnt.name, "icon.sys")){
				strcat(dir, "icon.sys");
				if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) < 0)
					goto error;
				if((size=fileXioLseek(fd, 0, SEEK_END)) <= 0x100)
					goto error;
				fileXioLseek(fd, 0xC0, SEEK_SET);
				fileXioRead(fd, out, 16*4);
				out[16*4] = 0;
				fileXioClose(fd); fd=-1;
				ret=0;
				break;
			}
		}
		fileXioDclose(dirfd); dirfd=-1;
	}
	else{
		strcat(dir, "icon.sys");
		if((fd=fioOpen(dir, O_RDONLY)) < 0) goto error;
		if((size=fioLseek(fd, 0, SEEK_END)) <= 0x100) goto error;
		fioLseek(fd, 0xC0, SEEK_SET);
		fioRead(fd, out, 16*4);
		out[16*4] = 0;
		fioClose(fd); fd=-1;
		ret=0;
	}
error:
	if(fd>=0){
		if(hddin) fileXioClose(fd);
		else	  fioClose(fd);
	}
	if(dirfd>=0) fileXioDclose(dirfd);
	return ret;
}

//-------------------------------------------------
// メニュー
int menu(const char *path, const char *file)
{
	uint64 color;
	char enable[NUM_MENU], tmp[MAX_PATH];
	int x, y, i, sel;
	
	int menu_x = SCREEN_WIDTH-FONT_WIDTH*19;
	int menu_y = FONT_HEIGHT*4;
	int menu_w = FONT_WIDTH*15;
	int menu_h = FONT_HEIGHT*(NUM_MENU+1);

	// メニュー項目有効・無効設定
	memset(enable, TRUE, NUM_MENU);	//全部TRUEにする

	if(!strcmp(path,"hdd0:/") || path[0]==0){
		enable[COPY] = FALSE;
		enable[CUT] = FALSE;
		enable[PASTE] = FALSE;
		enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		enable[NEWDIR] = FALSE;
		enable[GETSIZE] = FALSE;
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}

	if(!strncmp(path, "mc", 2))
		enable[RENAME] = FALSE;

	if(!strncmp(path, "hdd", 3))
		enable[EXPORT] = FALSE;

	if(!strncmp(path,"cdfs",4)){
		enable[CUT] = FALSE;
		enable[PASTE] = FALSE;
		enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		enable[NEWDIR] = FALSE;
		enable[EXPORT] = FALSE;
	}
	if(!strncmp(path, "mass", 4)){
		enable[RENAME] = FALSE;
		enable[EXPORT] = FALSE;
	}

	//マークしたファイルがない
	if(nmarks==0){
		//R1ボタンを押したときのカーソルの位置が".."のとき
		if(!strcmp(file, "..")){
			enable[COPY] = FALSE;
			enable[CUT] = FALSE;
			enable[DELETE] = FALSE;
			enable[RENAME] = FALSE;
			enable[GETSIZE] = FALSE;
			enable[EXPORT] = FALSE;
			enable[IMPORT] = FALSE;
		}
	}
	else{
		//マークしたファイルがある
		enable[RENAME] = FALSE;
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}

	//クリップボードに記憶したファイルがない
	if(nclipFiles==0)
		enable[PASTE] = FALSE;

	// 初期選択項設定
	for(sel=0; sel<NUM_MENU; sel++)
		if(enable[sel]==TRUE) break;
	
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP && sel<NUM_MENU){
				do{
					sel--;
					if(sel<0) sel=NUM_MENU-1;
				}while(!enable[sel]);
			}else if(new_pad & PAD_DOWN && sel<NUM_MENU){
				do{
					sel++;
					if(sel==NUM_MENU) sel=0;
				}while(!enable[sel]);
			}else if(new_pad & PAD_CROSS)
				return -1;
			else if(new_pad & PAD_CIRCLE)
				break;
			else if(new_pad & PAD_SQUARE){
				if(enable[COPY]){
					sel=COPY;
					break;
				}
			}
			else if(new_pad & PAD_TRIANGLE){
				if(enable[PASTE]){
					sel=PASTE;
					break;
				}
			}
		}
		
		// 描画開始
		drawDialogTmp(menu_x, menu_y, menu_x+menu_w, menu_y+menu_h, setting->color[0], setting->color[1]);
		for(i=0,y=74; i<NUM_MENU; i++){
			if(i==COPY)          sprintf(tmp, "%s(□)", lang->filer_menu_copy);
			else if(i==CUT)     strcpy(tmp, lang->filer_menu_cut);
			else if(i==PASTE)   sprintf(tmp, "%s(△)", lang->filer_menu_paste);
			else if(i==DELETE)  strcpy(tmp, lang->filer_menu_delete);
			else if(i==RENAME)  strcpy(tmp, lang->filer_menu_rename);
			else if(i==NEWDIR)  strcpy(tmp, lang->filer_menu_newdir);
			else if(i==GETSIZE) strcpy(tmp, lang->filer_menu_getsize);
			else if(i==EXPORT)  strcpy(tmp, lang->filer_menu_exportpsu);
			else if(i==IMPORT)  strcpy(tmp, lang->filer_menu_importpsu);

			if(enable[i])
				color = setting->color[2];
			else
				color = setting->color[3];
			
			printXY(tmp, menu_x+FONT_WIDTH*2, menu_y+FONT_HEIGHT/2+i*FONT_HEIGHT, color, TRUE);
			y+=FONT_HEIGHT;
		}
		if(sel<NUM_MENU)
			printXY(">", menu_x+FONT_WIDTH, menu_y+FONT_HEIGHT/2+sel*FONT_HEIGHT, setting->color[2], TRUE);

		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		sprintf(tmp,"○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
		printXY(tmp, x, y, setting->color[3], TRUE);
		drawScr();
	}
	
	return sel;
}

//-------------------------------------------------
// ファイルサイズ取得
size_t getFileSize(const char *path, const FILEINFO *file)
{
	size_t size;
	FILEINFO files[MAX_ENTRY];
	char dir[MAX_PATH], party[MAX_NAME];
	int nfiles, i, ret, fd;
	
	if(file->attr & FIO_S_IFDIR){
		sprintf(dir, "%s%s/", path, file->name);
		// 対象フォルダ内の全ファイル・フォルダサイズを合計
		nfiles = getDir(dir, files);
		for(i=size=0; i<nfiles; i++){
			ret=getFileSize(dir, &files[i]);
			if(ret < 0) size = -1;
			else		size+=ret;
		}
	}
	else{
		// パーティションマウント
		if(!strncmp(path, "hdd", 3)){
			getHddParty(path,file,party,dir);
			ret = mountParty(party);
			if(ret<0) return 0;
			dir[3] = ret+'0';
		}else
			sprintf(dir, "%s%s", path, file->name);
		// ファイルサイズ取得
		if(!strncmp(path, "hdd", 3)){
			fd = fileXioOpen(dir, O_RDONLY, fileMode);
			size = fileXioLseek(fd,0,SEEK_END);
			fileXioClose(fd);
		}else{
			fd = fioOpen(dir, O_RDONLY);
			size = fioLseek(fd,0,SEEK_END);
			fioClose(fd);
		}
	}
	return size;
}

//-------------------------------------------------
// ファイル・フォルダ削除
int delete(const char *path, const FILEINFO *file)
{
	FILEINFO files[MAX_ENTRY];
	char party[MAX_NAME], dir[MAX_PATH], hdddir[MAX_PATH];
	int nfiles, i, ret;
	
	// パーティションマウント
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path,file,party,hdddir);
		ret = mountParty(party);
		if(ret<0) return 0;
		hdddir[3] = ret+'0';
	}
	sprintf(dir, "%s%s", path, file->name);
	
	if(file->attr & FIO_S_IFDIR){
		strcat(dir,"/");
		// 対象フォルダ内の全ファイル・フォルダを削除
		nfiles = getDir(dir, files);
		for(i=0; i<nfiles; i++){
			ret=delete(dir, &files[i]);
			if(ret < 0) return -1;
		}
		// 対象フォルダを削除
		if(!strncmp(dir, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(0, NULL, &ret);
		}else if(!strncmp(path, "hdd", 3)){
			ret = fileXioRmdir(hdddir);
		}else if(!strncmp(path, "mass", 4)){
			sprintf(dir, "mass0:%s%s", &path[5], file->name);
			ret = fioRmdir(dir);
			if (ret < 0){
				dir[4] = 1 + '0';
				ret = fioRmdir(dir);
			}
		}
	} else {
		// 対象ファイルを削除
		if(!strncmp(path, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(dir[2]-'0', 0, &dir[4]);
			mcSync(0, NULL, &ret);
		}else if(!strncmp(path, "hdd", 3)){
			ret = fileXioRemove(hdddir);
		}else if(!strncmp(path, "mass", 4)){
			ret = fioRemove(dir);
		}
	}
	return ret;
}

//-------------------------------------------------
// ファイル・フォルダリネーム
int Rename(const char *path, const FILEINFO *file, const char *name)
{
	char party[MAX_NAME], oldPath[MAX_PATH], newPath[MAX_PATH];
	int ret=0;
	
	if(!strncmp(path, "hdd", 3)){
		sprintf(party, "hdd0:%s", &path[6]);
		*strchr(party, '/')=0;
		sprintf(oldPath, "pfs0:%s", strchr(&path[6], '/')+1);
		sprintf(newPath, "%s%s", oldPath, name);
		strcat(oldPath, file->name);
		
		ret = mountParty(party);
		if(ret<0) return -1;
		oldPath[3] = newPath[3] = ret+'0';
		
		ret=fileXioRename(oldPath, newPath);
	}else
		return -1;
	
	return ret;
}

//-------------------------------------------------
// 新規フォルダ作成
int newdir(const char *path, const char *name)
{
	char party[MAX_NAME], dir[MAX_PATH];
	int ret=0;
	
	if(!strncmp(path, "hdd", 3)){
		getHddParty(path,NULL,party,dir);
		ret = mountParty(party);
		if(ret<0) return -1;
		dir[3] = ret+'0';
		//fileXioChdir(dir);
		strcat(dir, name);
		ret = fileXioMkdir(dir, fileMode);
	}else if(!strncmp(path, "mc", 2)){
		sprintf(dir, "%s%s", path+4, name);
		mcSync(0,NULL,NULL);
		mcMkDir(path[2]-'0', 0, dir);
		mcSync(0, NULL, &ret);
		if(ret == -4)
			ret = -17;
	}else if(!strncmp(path, "mass", 4)){
		strcpy(dir, path);
		strcat(dir, name);
		ret = fioMkdir(dir);
	}
	
	return ret;
}

//-------------------------------------------------
// ファイルコピー
int copy(const char *outPath, const char *inPath, FILEINFO file, int n)
{
	FILEINFO files[MAX_ENTRY];
	char out[MAX_PATH], in[MAX_PATH], tmp[MAX_PATH],
		*buff=NULL, inParty[MAX_NAME], outParty[MAX_NAME];
	int hddout=FALSE, hddin=FALSE, nfiles, i;
	size_t size, outsize;
	int ret=-1, pfsout=-1, pfsin=-1, in_fd=-1, out_fd=-1, buffSize;

	//フォルダ名またはファイル名の文字数
	if(!strncmp(outPath, "mc", 2)){
		if(strlen(file.name)>32){
			return -1;
		}
	}
	else if(!strncmp(outPath, "mass", 4)){
		if(strlen(file.name)>128){
			return -1;
		}
	}
	else if(!strncmp(outPath, "hdd", 3)){
		if(strlen(file.name)>256){
			return -1;
		}
	}
	sprintf(out, "%s%s", outPath, file.name);
	sprintf(in, "%s%s", inPath, file.name);
	
	// 入力パスの設定とパーティションのマウント。
	if(!strncmp(inPath, "hdd", 3)){
		hddin = TRUE;
		getHddParty(inPath, &file, inParty, in);
		if(!strcmp(inParty, mountedParty[0]))
			pfsin=0;
		else if(!strcmp(inParty, mountedParty[1]))
			pfsin=1;
		else
			pfsin=-1;
	}
	// 出力パスの設定とパーティションのマウント。
	if(!strncmp(outPath, "hdd", 3)){
		hddout = TRUE;
		getHddParty(outPath, &file, outParty, out);
		if(!strcmp(outParty, mountedParty[0]))
			pfsout=0;
		else if(!strcmp(outParty, mountedParty[1]))
			pfsout=1;
		else
			pfsout=-1;
	}
	//入力パスがHDDのときマウント
	if(hddin){
		if(pfsin<0){
			if(pfsout==0) pfsin=1;
			else		  pfsin=0;
			sprintf(tmp, "pfs%d:", pfsin);
			if(mountedParty[pfsin][0]!=0)
				fileXioUmount(tmp); mountedParty[pfsin][0]=0;
			printf("%s mounting\n", inParty);
			if(fileXioMount(tmp, inParty, FIO_MT_RDWR) < 0) return -1;
			strcpy(mountedParty[pfsin], inParty);
		}
		in[3]=pfsin+'0';
	}
	else
		sprintf(in, "%s%s", inPath, file.name);
	//出力パスがHDDのときマウント
	if(hddout){
		if(pfsout<0){
			if(pfsin==0) pfsout=1;
			else		 pfsout=0;
			sprintf(tmp, "pfs%d:", pfsout);
			if(mountedParty[pfsout][0]!=0)
				fileXioUmount(tmp); mountedParty[pfsout][0]=0;
			if(fileXioMount(tmp, outParty, FIO_MT_RDWR) < 0) return -1;
			printf("%s mounting\n", outParty);
			strcpy(mountedParty[pfsout], outParty);
		}
		out[3]=pfsout+'0';
	}
	else
		sprintf(out, "%s%s", outPath, file.name);
	
	// フォルダの場合
	if(file.attr & FIO_S_IFDIR){
		// フォルダ作成
		ret = newdir(outPath, file.name);
		if(ret == -17){
			drawDark();
			itoGsFinish();
			itoSwitchFrameBuffers();
			drawDark();
			ret=-1;
			if(title) ret=getGameTitle(outPath, &file, tmp);
			if(ret<0) sprintf(tmp, "%s%s/", outPath, file.name);
			strcat(tmp, "\n");
			strcat(tmp, lang->filer_overwrite);
			if(ynDialog(tmp,0)<0) return -1;
			drawMsg(lang->filer_pasting);
		}
		else if(ret < 0)
			return -1;
		// フォルダの中身を全コピー
		sprintf(out, "%s%s/", outPath, file.name);
		sprintf(in, "%s%s/", inPath, file.name);
		nfiles = getDir(in, files);
		for(i=0; i<nfiles; i++)
			if(copy(out, in, files[i], n+1) < 0) return -1;
		return 0;
	}

	// 入力ファイルオープンとファイルサイズ取得
	if(hddin){
		in_fd = fileXioOpen(in, O_RDONLY, fileMode);
		if(in_fd<0) goto error;
		size = fileXioLseek(in_fd,0,SEEK_END);
		fileXioLseek(in_fd,0,SEEK_SET);
	}
	else{
		in_fd = fioOpen(in, O_RDONLY);
		if(in_fd<0) goto error;
		size = fioLseek(in_fd,0,SEEK_END);
		fioLseek(in_fd,0,SEEK_SET);
	}
	// 出力ファイルオープン
	if(hddout){
		// O_TRUNC が利かないため、オープン前にファイル削除
		fileXioRemove(out);
		out_fd = fileXioOpen(out,O_WRONLY|O_TRUNC|O_CREAT,fileMode);
		if(out_fd<0) goto error;
	}
	else{
		out_fd=fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
		if(out_fd<0) goto error;
	}

	// メモリに一度で読み込めるファイルサイズだった場合
	buff = (char*)malloc(size);
	if(buff==NULL){
		buff = (char*)malloc(32768);
		buffSize = 32768;
	}
	else
		buffSize = size;

	while(size>0){
		// 入力
		if(hddin) buffSize = fileXioRead(in_fd, buff, buffSize);
		else	  buffSize = fioRead(in_fd, buff, buffSize);
		// 出力
		if(hddout){
			outsize = fileXioWrite(out_fd,buff,buffSize);
			if(buffSize!=outsize){
				fileXioClose(out_fd); out_fd=-1;
				fileXioRemove(out);
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd,buff,buffSize);
			if(buffSize!=outsize){
				fioClose(out_fd); out_fd=-1;
				mcSync(0,NULL,NULL);
				mcDelete(out[2]-'0', 0, &out[4]);
				mcSync(0, NULL, NULL);
				goto error;
			}
		}
		size -= buffSize;
	}
	ret=0;
error:
	free(buff);
	if(in_fd>0){
		if(hddin) fileXioClose(in_fd);
		else	  fioClose(in_fd);
	}
	if(out_fd>0){	//修正した
		if(hddout) fileXioClose(out_fd);
		else	  fioClose(out_fd);
	}
	return ret;
}

//-------------------------------------------------
// ペースト
int paste(const char *path)
{
	char tmp[MAX_PATH];
	int i, ret=-1;
	
	if(!strcmp(path,clipPath)) return -1;
	
	for(i=0; i<nclipFiles; i++){
		strcpy(tmp, clipFiles[i].name);
		if(clipFiles[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
		strcat(tmp, " ");
		strcat(tmp, lang->filer_pasting);
		drawMsg(tmp);
		ret=copy(path, clipPath, clipFiles[i], 0);
		if(ret < 0) break;
		if(cut){
			ret=delete(clipPath, &clipFiles[i]);
			if(ret<0) break;
		}
	}
	
	if(mountedParty[0][0]!=0){
		fileXioUmount("pfs0:"); mountedParty[0][0]=0;
	}
	
	if(mountedParty[1][0]!=0){
		fileXioUmount("pfs1:"); mountedParty[1][0]=0;
	}
	
	return ret;
}

//-------------------------------------------------
//psbCommand psbコマンド実行
int psbCommand(void)
{
	char path[2][MAX_PATH];
	char dir[MAX_PATH];
	char pathtmp[MAX_PATH];
	int len;
	FILEINFO file;
	char *p;
	int fd;
	int ret=0;
	char message[2048];
	FILEINFO clipFilesBackup;
	int nclipFilesBackup;

	//引数が足りない
	if(psb_argc<2)
		return 0;

	strcpy(path[0], psb_argv[1]);
	strcpy(path[1], psb_argv[2]);

	//コメント
	if(!strnicmp(psb_argv[0], "rem",3))
		return 0;

	//Module
	if(!strncmp(path[0], "cdfs", 4)||!strncmp(path[1], "cdfs", 4))
		loadCdModules();
	else if(!strncmp(path[0], "hdd", 3)||!strncmp(path[1], "hdd", 3)){
		if(nparties==0){
			loadHddModules();
			setPartyList();
		}
	}
	else if(!strncmp(path[0], "mass", 4)||!strncmp(path[1], "mass", 4))
		loadUsbModules();

	//psb_argv[1]の最後がスラッシュのとき削除
	len = strlen(path[0]);
	if(len>0){
		if(path[0][len-1]=='/')
			path[0][len-1]='\0';
	}
	if(psb_argc>1){
		//psb_argv[2]の最後にスラッシュ無いときつける
		len = strlen(path[1]);
		if(len>0){
			if(path[1][len-1]!='/')
				strcat(path[1], "/");
		}
	}
	//
	strcpy(dir, path[0]);
	if((p=strrchr(dir, '/'))){
		p++;
		*p=0;
	}

	//psb_argv[1]からFILEINFOを作成する
	//file.name
	if((p = strrchr(path[0], '/')))
		strcpy(file.name, p+1);
	//file.attr
	if(!strncmp(path[0], "mc", 2)){
		mcTable mcDir __attribute__((aligned(64)));
		int mcret;
		//フォルダとしてオープンしてみる
		strcpy(pathtmp, path[0]+4); strcat(pathtmp,"/*");
		mcGetDir(path[0][2]-'0', 0, pathtmp, 0, 1, &mcDir);
		mcSync(0, NULL, &mcret);
		if(mcret<0){
			//失敗したらファイルとしてオープンしてみる
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				file.attr=0;	//失敗
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//ファイル
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//フォルダ
		}
	}
	else if(!strncmp(path[0], "cdfs", 4)){
		//フォルダとしてオープンしてみる
		struct TocEntry TocEntryList;
		char cdfsdir[MAX_PATH];
		int n;
		strcpy(cdfsdir, path[0]+5);
		CDVD_FlushCache();
		n = CDVD_GetDir(cdfsdir, NULL, CDVD_GET_FILES_AND_DIRS, &TocEntryList, 1, cdfsdir);
		if(n<0){
			//失敗したらファイルとしてオープンしてみる
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				file.attr=0;	//失敗
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//ファイル
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//フォルダ
		}
		if(setting->discControl) CDVD_Stop();
	}
	else if(!strncmp(path[0], "hdd", 3)){
		char party[MAX_NAME];
		int r;
		//ファイルがhddにあるときパスを変更
		getHddParty(path[0], NULL, party, pathtmp);
		//マウント
		r = mountParty(party);
		pathtmp[3] = r+'0';

		//フォルダとしてオープンしてみる
		fd = fileXioDopen(pathtmp);
		if(fd<0){
			//失敗したらファイルとしてオープンしてみる
			fd = fileXioOpen(pathtmp, O_RDONLY, fileMode);
			if(fd<0){
				file.attr=0;	//失敗
			}
			else{
				fileXioClose(fd);
				file.attr=FIO_S_IFREG;	//ファイル
			}
		}
		else{
			fileXioDclose(fd);
			file.attr=FIO_S_IFDIR;
		}
	}
	else if(!strncmp(path[0], "mass", 4)){
		//フォルダとしてオープンしてみる
		strcpy(pathtmp, path[0]);
		fd = fioDopen(pathtmp);
		if(fd<0){
			//失敗したらファイルとしてオープンしてみる
			fd = fioOpen(path[0], O_RDONLY);
			if(fd<0)
				file.attr=0;	//失敗
			else{
				fioClose(fd);
				file.attr=FIO_S_IFREG;	//ファイル
			}
		}
		else{
		
			file.attr=FIO_S_IFDIR;	//フォルダ
			fioDclose(fd);
		}
	}

	//コマンド
	if(!stricmp(psb_argv[0], "copy")){	//コピー
		if(psb_argc<3){
			ret=-1;
		}
		else{
			if(file.attr==0){
				//コピーするものが無かった
				ret=-1;
			}
			else{
				//バックアップ
				clipFilesBackup = clipFiles[0];
				nclipFilesBackup = nclipFiles;
				//
				clipFiles[0]=file;
				nclipFiles = 1;
				//コピー元のディレクトリ名(clipPath)
				strcpy(clipPath, dir);
				//
				cut=FALSE;	//コピー
				//ペースト開始
				ret=paste(path[1]);
				//元に戻す
				clipFiles[0] = clipFilesBackup;
				nclipFiles = nclipFilesBackup;
			}
		}
		if(ret) MessageDialog("copy Failed");
	}
	else if(!stricmp(psb_argv[0], "move")){	//移動
		if(psb_argc<3){
			ret=-1;
		}
		else{
			if(file.attr==0){
				//コピーするものが無かった
				ret=-1;
			}
			else{
				//バックアップ
				clipFilesBackup = clipFiles[0];
				nclipFilesBackup = nclipFiles;
				//
				clipFiles[0]=file;
				nclipFiles = 1;
				//コピー元のディレクトリ名(clipPath)
				strcpy(clipPath, dir);
				//
				cut=TRUE;	//移動
				//ペースト開始
				ret=paste(path[1]);
				//元に戻す
				clipFiles[0] = clipFilesBackup;
				nclipFiles = nclipFilesBackup;
			}
		}
		if(ret) MessageDialog("move Failed");
	}
	else if(!stricmp(psb_argv[0], "del")){	//削除
		if(file.attr==0){
			//削除するものが無かった
			ret=-1;
		}
		else{
			int ynret;
			sprintf(message, "%s", file.name);
			if(file.attr & FIO_S_IFDIR) strcat(message, "/");
			strcat(message, "\n");
			strcat(message, lang->filer_delete);
			ynret = ynDialog(message,0);
			if(ynret>0){
				//削除開始
				ret=delete(dir, &file);
			}
		}
		if(ret) MessageDialog("del Failed");
	}
	else if(!stricmp(psb_argv[0], "mkdir")){	//フォルダ作成
		//作成開始
		ret=newdir(dir, file.name);
		if(ret) MessageDialog("mkdir Failed");
	}
	else if(!stricmp(psb_argv[0], "rmdir")){	//フォルダ削除
		if(file.attr==0){
			//削除するものが無かった
			ret=-1;
		}
		else{
			int ynret;
			//ディレクトリ削除
			if(file.attr==FIO_S_IFDIR){
				sprintf(message, "%s", file.name);
				if(file.attr & FIO_S_IFDIR) strcat(message, "/");
				strcat(message, "\n");
				strcat(message, lang->filer_delete);
				ynret = ynDialog(message,0);
				if(ynret>0){
					//削除開始
					ret=delete(dir, &file);
				}
			}
			else{
				ret=-1;
			}
		}
		if(ret) MessageDialog("rmdir Failed");
	}
	return ret;
}

//-------------------------------------------------
//psbParse psbファイルパース
void psbParse(const char *str)
{
	char *p;
	int len;
	int i;
	int flag;
	int l;
	char strtmp[(MAX_PATH+2)*MAX_ARGC];
	char tmp[MAX_PATH+2];

	strcpy(strtmp, str);

	//初期化
	psb_argc=0;
	memset(psb_argv, 0, sizeof(psb_argv));

	//改行コードを消す
	p=strrchr(strtmp,'\n');
	if(p!=NULL) *p='\0';
	p=strrchr(strtmp,'\r');
	if(p!=NULL) *p='\0';

	len=strlen(strtmp);

	//
	if(len==0) return;

	//スペースを\0に変換
	flag=0;
	for(i=0;i<len;i++){
		if(strtmp[i]=='\"')	//ダブルクォーテーションフラグ
			flag=1-flag;
		else if((strtmp[i]==' ')&&(flag==0))
			strtmp[i]='\0';
	}

	//\0区切りを読みとる
	for(i=0;i<len;i++){
		if(strtmp[i]=='\0'){
			//何もしない
		}
		else{
			l=strlen(strtmp+i);
			if(psb_argc<MAX_ARGC){
				if(l<MAX_PATH+2)
					strcpy(psb_argv[psb_argc], strtmp+i);
				psb_argc++;
			}
			i+=l;
		}
	}

	//ダブルクォーテーションを消す
	for(i=0;i<psb_argc;i++){
		if(psb_argv[i][0]=='\"'){
			//前のダブルクォーテーションを消す
			strcpy(tmp, &psb_argv[i][1]);
			strcpy(psb_argv[i],tmp);
			//後ろのダブルクォーテーションを消す
			p=strrchr(psb_argv[i],'\"');
			if(p!=NULL) *p='\0';
		}
	}
}

//-------------------------------------------------
//psbファイル実行
//戻り値     0:成功
//          -1:ファイルオープン失敗
//       0以上:エラーが出た行番号
int psb(const char *psbpath)
{
	int fd;
	char buffer[(MAX_PATH+2)*MAX_ARGC];
	int lineno;
	int ret;

	//
	if(!strncmp(psbpath, "hdd", 3)){
		if(nparties==0){
			loadHddModules();
			setPartyList();
		}
	}
	else if(!strncmp(psbpath, "cdfs", 4))
		loadCdModules();
	else if(!strncmp(psbpath, "mass", 4))
		loadUsbModules();

	//
	fd=fioOpen(psbpath, O_RDONLY);
	if(fd<0) return -1;

	lineno=0;
	while(1){
		memset(buffer, 0, sizeof(buffer));
		if(fioGets(fd, buffer, (MAX_PATH+2)*MAX_ARGC)==0)
			break;
		if(buffer[0]=='\n'){
		}
		else{
			lineno++;
			//パース
			psbParse(buffer);
			//実行
			if(psbCommand()<0){
				ret=lineno;
				goto psberror;	//エラーでたら停止
			}
		}
	}
	ret=0;	//エラーなし
psberror:
	fioClose(fd);
	return ret;
}

//-------------------------------------------------
//ゲームタイトルをファイル名に変換
void title2filename(const unsigned char *in, unsigned char *out)
{
	int i=0;
	int code;

	strcpy(out,in);
	code=in[i];
	while(in[i]){
		code=in[i];
		//windowsでファイル名に使えない文字は「_」に変換
		if(code==0x22) out[i]='_';	// '"'
		if(code==0x2A) out[i]='_';	// '*'
		if(code==0x2C) out[i]='_';	// ','
		if(code==0x2F) out[i]='_';	// '/'
		if(code==0x3A) out[i]='_';	// ':'
		if(code==0x3B) out[i]='_';	// ';'
		if(code==0x3C) out[i]='_';	// '<'
		if(code==0x3E) out[i]='_';	// '>'
		if(code==0x3F) out[i]='_';	// '?'
		if(code==0x5c) out[i]='_';	// '\'
		if(code==0x7C) out[i]='_';	// '|'
		i++;
	}
}

//-------------------------------------------------
//ゲームタイトルのsjisの英数字と記号をASCIIに変換
void sjis2ascii(const unsigned char *in, unsigned char *out)
{
	int i=0;
	int code;
	unsigned char ascii;
	int n=0;

	while(in[i]){
		if(in[i] & 0x80){
			// SJISコードの生成
			code = in[i++];
			code = (code<<8) + in[i++];

			ascii=0xFF;
			if(code>>8==0x81)
				ascii = sjis_lookup_81[code & 0x00FF];
			else if(code>>8==0x82)
				ascii = sjis_lookup_82[code & 0x00FF];

			if(ascii!=0xFF){
				out[n]=ascii;
				n++;
			}
			else{
				//ASCIIに変換できない文字
				out[n]=code>>8&0xFF;
				out[n+1]=code&0xFF;
				n=n+2;
			}
		}
		else{
			out[n]=in[i];
			n++;
			i++;
		}
	}
}

//-------------------------------------------------
// psuファイルからインポート
// 戻り値
// 0以下 :失敗
// 0     :mc0にインポート
// 1     :mc1にインポート
int psuImport(const char *path, const FILEINFO *file)
{
	//
	int ret = -1;	//戻り値
	int n = 0;
	PSU_HEADER psu_header[MAX_ENTRY];
	char outdir[MAX_PATH];		//セーブデータのフォルダ名
	char title[16*4+1]="";
	char *buff=NULL;
	int outmc=0;	//インポート先のmc番号

	int in_fd = -1, out_fd = -1;
	int hddin = FALSE;
	int i;

	int dialog_x;		//ダイアログx位置
	int dialog_y;		//ダイアログy位置
	int dialog_width;	//ダイアログ幅 
	int dialog_height;	//ダイアログ高さ  

	//フォルダのときは、psuからインポートできない
	if(file->attr & FIO_S_IFDIR){
		ret=-1;
		return ret;
	}

	//step1 psuヘッダ読み込み
	{
		char inpath[MAX_PATH];	//選択されたフォルダまたはファイルのフルパス
		char tmp[2048];		//雑用
		char party[MAX_NAME];
		int r;
		int psuSize;
		int seek;
		int fileSize;	//ファイルのサイズ

		//psuファイルがhddのあるときパスを変更
		if(!strncmp(path, "hdd", 3)){
			hddin = TRUE;
			getHddParty(path, NULL, party, inpath);
			//pfs0にマウント
			r = mountParty(party);
			if(r<0) return 0;
			inpath[3] = r+'0';
			//psuファイルのフルパス
			strcat(inpath, file->name);
		}
		else{
			//psuファイルのフルパス
			sprintf(inpath, "%s%s", path, file->name);
		}

		//psuファイルオープンとサイズ取得
		if(hddin){
			in_fd = fileXioOpen(inpath, O_RDONLY, fileMode);
			if(in_fd<0){
				ret=-2;
				goto error;
			}
			psuSize = fileXioLseek(in_fd, 0, SEEK_END);
			fileXioLseek(in_fd, 0, SEEK_SET);
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-2;
				goto error;
			}
			psuSize = fioLseek(in_fd, 0, SEEK_END);
			fioLseek(in_fd, 0, SEEK_SET);
		}
	
		//psuヘッダ読み込む
		if(psuSize<sizeof(PSU_HEADER)){
			ret=-3;
			goto error;
		}
		//psuヘッダを読み込むのにpsu_header[0]を一時的に使う
		memset(&psu_header[0], 0, sizeof(PSU_HEADER));
		if(hddin) fileXioRead(in_fd, (char*)&psu_header[0], sizeof(PSU_HEADER));
		else fioRead(in_fd, &psu_header[0], sizeof(PSU_HEADER));
		n = psu_header[0].size;	//ファイル数
		strcpy(outdir, psu_header[0].name);	//出力するフォルダ名
		seek = sizeof(PSU_HEADER);	//ファイルのシーク
	
		//psu_header[0]から読み込む
		for(i=0;i<n;i++){
			//ファイルヘッダ読み込む
			if(psuSize<seek+sizeof(PSU_HEADER)){
				ret=-4;
				goto error;
			}
			memset(&psu_header[i], 0, sizeof(PSU_HEADER));
			if(hddin) fileXioRead(in_fd, (char*)&psu_header[i], sizeof(PSU_HEADER));
			else fioRead(in_fd, &psu_header[i], sizeof(PSU_HEADER));
			seek += sizeof(PSU_HEADER);
			//ゲームタイトル
			if(!strcmp(psu_header[i].name,"icon.sys")){
				if(hddin){
					fileXioLseek(in_fd, seek+0xC0, SEEK_SET);
					fileXioRead(in_fd, tmp, 64);
					title[64]=0;
					fileXioLseek(in_fd, seek, SEEK_SET);
				}
				else{
					fioLseek(in_fd, seek+0xC0, SEEK_SET);
					fioRead(in_fd, tmp, 64);
					title[64]=0;
					fioLseek(in_fd, seek, SEEK_SET);
				}
				sjis2ascii(tmp, title);
			}
			//
			if(psu_header[i].size>0){
				fileSize = (((psu_header[i].size-1)/0x400)+1)*0x400;
				if(psuSize<seek + fileSize){
					ret=-5;
					goto error;
				}
				seek += fileSize;
				if(hddin) fileXioLseek(in_fd, seek, SEEK_SET);
				else fioLseek(in_fd, seek, SEEK_SET);
			}
		}
		//psuファイルクローズ
		if(hddin){
			hddin = FALSE;
			fileXioClose(in_fd);
		}
		else
			fioClose(in_fd);
		in_fd = -1;
	}

	//step2 情報の表示 
	{
		char tmp[2048];
		int x, y, scroll;
		char fullpath[MAX_PATH];	//選択されたフォルダまたはファイルのフルパス

		//psuファイルのフルパス
		sprintf(fullpath, "%s%s", path, file->name);

		dialog_width = FONT_WIDTH*50;
		dialog_height = FONT_HEIGHT*16;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		scroll = 0;
		while(1){
			waitPadReady(0, 0);
			if(readpad()){
				if(new_pad & PAD_UP){
					scroll -= 8;
					if(scroll<0) scroll += MAX_ENTRY;
				}
				else if(new_pad & PAD_DOWN){
					scroll += 8;
					if(scroll>=MAX_ENTRY) scroll -= MAX_ENTRY;
				}
				else if(new_pad & PAD_LEFT){
					outmc --;
					if(outmc<0) outmc = 2;
				}
				else if(new_pad & PAD_RIGHT){
					outmc ++;
					if(outmc>2) outmc = 0;
				}
				else if(new_pad & PAD_CROSS){
					ret=-6;
					return ret;
				}
				else if(new_pad & PAD_CIRCLE){
					if(outmc==2){
						ret=-6;
						return ret;
					}
					break;
				}
				else if(new_pad & PAD_SELECT){
					//キャンセル
					outmc=2;
				}
			}

			// 描画開始
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			drawFrame(dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_HEIGHT*14, setting->color[1]);
			// psuファイルの情報を表示
			x = dialog_x+FONT_WIDTH*2;
			y = dialog_y+FONT_HEIGHT*0.5;
			strcpy(tmp, fullpath);
			if(strlen(tmp)>46){
				tmp[42]=0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			strcpy(tmp, title);
			if(strlen(tmp)>46){	//titleが長いときに短くする
				tmp[42] = 0;
				strcat(tmp,"...");

			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			sprintf(tmp, "%2d %s", n, lang->filer_import_files);
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT*2;
			printXY(lang->filer_import_header, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
			for(i=0;i<8;i++){
				sprintf(tmp, "%4d:", i+scroll);
				if(i+scroll<n)
					sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, psu_header[i+scroll].attr, psu_header[i+scroll].size, psu_header[i+scroll].name);
					if(strlen(tmp)>46){
						tmp[42]=0;
						strcat(tmp,"...");
					}
				printXY(tmp, x, y, setting->color[3], TRUE);
				y +=FONT_HEIGHT;
			}
			y += FONT_HEIGHT;
			sprintf(tmp,"   mc0:/    mc1:/    %s", lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			printXY(">", x+FONT_WIDTH+FONT_WIDTH*9*outmc, y, setting->color[3], TRUE);
			// 操作説明
			x = FONT_WIDTH*2;
			y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[0],
				0, y,
				SCREEN_WIDTH, y+FONT_HEIGHT, 0);
			sprintf(tmp,"○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			drawScr();
		}
	}

	//step3 インポート開始
	{
		char inpath[MAX_PATH];	//psuファイルのフルパス
		char outpath[MAX_PATH];//セーブデータのフォルダを出力するフォルダのフルパス
		int seek;
		char tmp[2048];		//雑用 表示用
		char out[MAX_PATH];	//セーブデータのフォルダの中のファイルのフルパス
		size_t outsize;
		char party[MAX_NAME];
		int r;

		//セーブデータのフォルダを出力するフォルダのフルパス
		if(!outmc)
			strcpy(outpath, "mc0:/");
		else
			strcpy(outpath, "mc1:/");

		//psuファイル
		if(!strncmp(path, "hdd", 3)){
			hddin = TRUE;
			getHddParty(path, NULL, party, inpath);
			//pfs0にマウント
			r = mountParty(party);
			if(r<0) return 0;
			inpath[3] = r+'0';
			//psuファイルのフルパス
			strcat(inpath, file->name);
		}
		else{
			//psuファイルのフルパス
			sprintf(inpath, "%s%s", path, file->name);
		}

		//セーブデータのフォルダ作成
		r = newdir(outpath, outdir);
		if(r == -17){	//フォルダがすでにあるとき上書きを確認する
			drawDark();
			itoGsFinish();
			itoSwitchFrameBuffers();
			drawDark();
			sprintf(tmp, "%s%s/\n%s", outpath, outdir, lang->filer_overwrite);
			if(ynDialog(tmp,0)<0){	//キャンセル
				ret = -7;
				goto error;
			}
		}
		else if(r < 0){//フォルダ作成失敗
			ret = -8;
			goto error;
		}

		//psuファイルオープン
		if(hddin){
			in_fd = fileXioOpen(inpath, O_RDONLY, fileMode);
			if(in_fd<0){
				ret=-2;
				goto error;
			}
		}
		else{
			in_fd = fioOpen(inpath, O_RDONLY);
			if(in_fd<0){
				ret=-9;
				goto error;
			}
		}

		// 描画開始
		dialog_width = FONT_WIDTH*32;
		dialog_height = FONT_HEIGHT*2;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();
		seek = sizeof(PSU_HEADER);
		for(i=0;i<n;i++){
			// プログレスバー
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			itoSprite(setting->color[1],
				dialog_x+FONT_HEIGHT/2, dialog_y+FONT_WIDTH/2,
				dialog_x+FONT_HEIGHT/2+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+dialog_height-FONT_WIDTH/2, 0);
			//
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+FONT_HEIGHT/2, setting->color[3], TRUE);
			drawScr();
			//
			seek += sizeof(PSU_HEADER);
			if(psu_header[i].size>0){
				//書き込むデータのメモリを確保
				buff = (char*)malloc(psu_header[i].size);
				if(buff==NULL){
					ret=-10;
					goto error;
				}
				//出力するファイルオープン
				sprintf(out, "%s%s/%s", outpath, outdir, psu_header[i].name);
				out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
				if(out_fd<0){
					ret=-11;
					goto error;
				}
				//読み込み
				memset(buff, 0, psu_header[i].size);
				if(hddin){
					fileXioLseek(in_fd, seek, SEEK_SET);
					fileXioRead(in_fd, buff, psu_header[i].size);
				}
				else{
					fioLseek(in_fd, seek, SEEK_SET);
					fioRead(in_fd, buff, psu_header[i].size);
				}
				//書き込み
				outsize = fioWrite(out_fd, buff, psu_header[i].size);
				if(outsize!=psu_header[i].size){
					ret=-12;
					goto error;
				}
				//クローズ
				fioClose(out_fd); out_fd=-1;
				free(buff);
				//
				fioLseek(in_fd, seek, SEEK_SET);	//シークをファイルの先頭にに戻す
				seek += (((psu_header[i].size-1)/0x400)+1)*0x400;
				fioLseek(in_fd, seek, SEEK_SET);	//シークを次のファイルヘッダの先頭に移動
			}
		}
		//psuファイルをクローズ
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
		in_fd=-1;
	}
	//
	ret=outmc;
error:
	free(buff);
	if(in_fd>=0){
		if(hddin) fileXioClose(in_fd);
		else fioClose(in_fd);
	}
	if(out_fd>=0){
		fioClose(out_fd);
	}

	return ret;
}

//-------------------------------------------------
// psuファイルにエクスポート
int psuExport(const char *path, const FILEINFO *file)
{
	int ret = -1;	//戻り値
	int n = 0;

	mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	int mcret;
	int r;

	int sjisout = FALSE;		//psuファイルをsjisで出力
	char outpath[MAX_PATH];	//出力するpsuファイル名
	char *buff=NULL;
	int out_fd = -1;
	int in_fd = -1;
	int hddout = FALSE;

	int dialog_x;		//ダイアログx位置
	int dialog_y;		//ダイアログy位置
	int dialog_width;	//ダイアログ幅
	int dialog_height;	//ダイアログ高さ

	//ファイルのときは、psuにエクスポートできない
	if(file->attr & FIO_S_IFREG){	//ファイル
		ret = -1;
		return ret;
	}

	//step1 エクスポートするセーブデータを調べる
	{
		char inpath[MAX_PATH];		//選択されたフォルダのフルパス
		char Pattern[MAX_PATH];	//列挙用パターン

		//選択されたフォルダのフルパス
		sprintf(inpath, "%s%s", path, file->name);

		//リスト読み込み
		sprintf(Pattern, "%s/*", &inpath[4]);
		mcSync(0, NULL, &mcret);
		mcGetDir(inpath[2]-'0', 0, Pattern, 0, MAX_ENTRY-2, mcDir);
		mcSync(0, NULL, &n);	//ファイル数
		//mcDir[0]の情報
		mcDir[0].fileSizeByte=0;
		mcDir[0].attrFile=0x8427;
		strcpy(mcDir[0].name,".");
	}

	//step2 情報の表示
	{
		int x,y,scroll;
		char inpath[MAX_PATH];	//選択されたフォルダのフルパス
		char tmp[2048];		//表示用
		int i;

		//選択されたフォルダのフルパス
		sprintf(inpath, "%s%s", path, file->name);

		dialog_width = FONT_WIDTH*50;
		dialog_height = FONT_HEIGHT*16;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		scroll = 0;
		while(1){
			waitPadReady(0, 0);
			if(readpad()){
				if(new_pad & PAD_UP){
					scroll -= 8;
					if(scroll<0) scroll += MAX_ENTRY;
				}
				else if(new_pad & PAD_DOWN){
					scroll += 8;
					if(scroll>=MAX_ENTRY) scroll -= MAX_ENTRY;
				}
				else if(new_pad & PAD_TRIANGLE){
					sjisout=TRUE;
					break;
				}
				else if(new_pad & PAD_CROSS){
					ret=-201;
					return ret;
				}
				else if(new_pad & PAD_CIRCLE){
					break;
				}
			}
	
			// 描画開始
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			drawFrame(dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*4,
				dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_HEIGHT*14, setting->color[1]);
			//
			x = dialog_x+FONT_WIDTH*2;
			y = dialog_y+FONT_HEIGHT*0.5;
			sprintf(tmp, "%s/", inpath);
			if(strlen(tmp)>46){
				tmp[42]=0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(tmp, "%s", file->title);
			if(strlen(tmp)>46){	//titleが長いときに短くする
				tmp[42] = 0;
				strcat(tmp,"...");
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			sprintf(tmp, "%d %s", n, lang->filer_export_files);
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT*2;
			printXY(lang->filer_export_header, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
			for(i=0;i<8;i++){
				sprintf(tmp, "%4d:", i+scroll);
				if(i+scroll<n)
					sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, mcDir[i+scroll].attrFile, mcDir[i+scroll].fileSizeByte, mcDir[i+scroll].name);
				if(strlen(tmp)>46){
					tmp[42]=0;
					strcat(tmp,"...");
				}
				printXY(tmp, x, y, setting->color[3], TRUE);
				y += FONT_HEIGHT;
			}
			y += FONT_HEIGHT;
			sprintf(tmp,"○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
			// 操作説明
			x = FONT_WIDTH*2;
			y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[0],
				0, y,
				SCREEN_WIDTH, y+FONT_HEIGHT, 0);
			sprintf(tmp,"○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[3], TRUE);
	
			drawScr();
		}
	}

	//step3
	{
		char inpath[MAX_PATH];		//選択されたフォルダのフルパス
		char party[MAX_NAME];
		int r;
		int i;
		size_t outsize;
		int readSize;
		int writeSize;
		unsigned char tmp[2048]="";	//表示用
		int code;
		char tmppath[MAX_PATH];
		PSU_HEADER psu_header;

		//選択されたフォルダのフルパス
		sprintf(inpath, "%s%s", path, file->name);

		//出力するフォルダ名
		if(setting->Exportdir[0])
			strcpy(outpath, setting->Exportdir);
		else
			strcpy(outpath, path);

		//出力するpsuファイル名
		strcpy(tmp,file->name);
		if(sjisout){
			if(file->title[0]){
				//ファイル名に使えない文字を変換
				title2filename(file->title, tmp);
			}
		}

		//出力先がmcのときにファイル名の文字数を調べる
		if(!strncmp(outpath, "mc", 2)){
			//ファイル名の最大 mc:32byte mass:128byte hdd:256byte
			if(strlen(tmp)>28){	//ファイル名が長いときに短くする
				tmp[28] = 0;
				code=tmp[27];
				//2byte文字の1byte目だったら消す
				if( (code>=0x81)&&(code<=0x9F) ) tmp[27] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[27] = 0;
			}
		}

		//出力するpsuファイルのフルパス
		strcat(outpath, tmp);
		strcat(outpath, ".psu");

		//出力するpsuファイルがhddのときパスを変更
		if(!strncmp(outpath, "hdd", 3)){
			if(nparties==0){
				loadHddModules();
				setPartyList();
			}
			hddout = TRUE;
			getHddParty(outpath, NULL, party, tmp);
			//pfs0にマウント
			r = mountParty(party);
			if(r<0) return -301;
			strcpy(outpath, tmp);
			outpath[3] = r+'0';
		}
		else if(!strncmp(outpath, "cdfs", 2)){
			ret=-302;
			goto error;
		}

		//psuファイルオープン 新規作成
		if(hddout){
			// O_TRUNC が利かないため、オープン前にファイル削除
			fileXioRemove(outpath);
			out_fd = fileXioOpen(outpath, O_WRONLY|O_TRUNC|O_CREAT, fileMode);
			if(out_fd<0){
				ret=-303;
				goto error;
			}
		}
		else{	//mc mass
			out_fd = fioOpen(outpath, O_WRONLY | O_TRUNC | O_CREAT);
			if(out_fd<0){
				ret=-304;
				goto error;
			}
		}

		//psuヘッダ書き込み
		memset(&psu_header, 0, sizeof(PSU_HEADER));
		psu_header.attr = 0x8427;
		psu_header.size = n;
		strcpy(psu_header.name, file->name);
		if(hddout){
			outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-305;
				goto error;
			}
		}
		else{
			outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
			if(outsize!=sizeof(PSU_HEADER)){
				ret=-306;
				goto error;
			}
		}
	
		//ファイルヘッダとファイル書き込み
		dialog_width = FONT_WIDTH*32;
		dialog_height = FONT_HEIGHT*2;
		dialog_x = (SCREEN_WIDTH-dialog_width)/2;
		dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
		drawDark();
		itoGsFinish();
		itoSwitchFrameBuffers();
		drawDark();
		for(i=0;i<n;i++){
			// 描画開始
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[0], setting->color[1]);
			// プログレスバー
			itoSprite(setting->color[1],
				dialog_x+FONT_HEIGHT/2, dialog_y+FONT_WIDTH/2,
				dialog_x+FONT_HEIGHT/2+(dialog_width-FONT_WIDTH)*(i*100/n)/100, dialog_y+dialog_height-FONT_WIDTH/2, 0);
			sprintf(tmp, "%2d / %2d", i, n);
			printXY(tmp, dialog_x+120, dialog_y+FONT_HEIGHT/2, setting->color[3], TRUE);
			drawScr();
			//ファイルヘッダを作成
			memset(&psu_header, 0, sizeof(PSU_HEADER));
			psu_header.attr = mcDir[i].attrFile;	//ファイル属性はメモリーカードと同じにする
			psu_header.size = mcDir[i].fileSizeByte;
			strncpy(psu_header.name, mcDir[i].name,32);
			//ファイルヘッダ書き込み
			if(hddout){
				outsize = fileXioWrite(out_fd, (char*)&psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-307;
					goto error;
				}
			}
			else{
				outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
				if(outsize!=sizeof(PSU_HEADER)){
					ret=-308;
					goto error;
				}
			}
			//ファイル書き込み
			if(mcDir[i].fileSizeByte>0){
				sprintf(tmppath, "%s/%s", inpath, mcDir[i].name);
				writeSize = (((mcDir[i].fileSizeByte-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				if(buff==NULL){
					ret=-309;
					goto error;
				}
				memset(buff, 0, writeSize);
				//ファイルオープン
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-310;
					goto error;
				}
				//読み込む
				readSize = fioRead(in_fd, buff, mcDir[i].fileSizeByte);
				if(readSize!=mcDir[i].fileSizeByte){
					ret=-311;
					goto error;
				}
				//クローズ
				fioClose(in_fd); in_fd=-1;
				//psuファイルに書き込み
				if(hddout){
					outsize = fileXioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-312;
						goto error;
					}
				}
				else{
					outsize = fioWrite(out_fd, buff, writeSize);
					if(outsize!=writeSize){
						ret=-313;
						goto error;
					}
				}
				free(buff);
			}
		}
	}
	//psuファイルクローズ
	if(hddout)fileXioClose(out_fd);
	else fioClose(out_fd);
	out_fd=-1;
	ret=0;
error:
	free(buff);
	if(in_fd>=0) fioClose(in_fd);
	if(out_fd>=0){
		if(hddout) fileXioClose(out_fd);
		else fioClose(out_fd);
	}

	if(ret<0){
		// エクスポート失敗したときpsuファイルを削除
		if(!strncmp(outpath, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(outpath[2]-'0', 0, &outpath[4]);
			mcSync(0, NULL, &r);
		}
		else if(!strncmp(outpath, "pfs", 3)){
			r = fileXioRemove(outpath);
		}
		else if(!strncmp(outpath, "mass", 4)){
			r = fioRemove(outpath);
		}
	}
	return ret;
}

//-------------------------------------------------
// スクリーンキーボード
/*
■ 使用不可文字
 : * " | < > \ / ?
■ レイアウト
A B C D E F G H I J K L M
N O P Q R S T U V W X Y Z
a b c d e f g h i j k l m
n o p q r s t u v w x y z
0 1 2 3 4 5 6 7 8 9      
( ) [ ] ! # $ % & @ ;    
= + - ' ^ . , _          
OK                  CANCEL
*/
int keyboard(char *out, int max)
{
	int	WFONTS,	//キーボードの横の文字数
		HFONTS,	//キーボードの縦の文字数
		KEY_W,	//キーボードの横のサイズ
		KEY_H,	//キーボードの縦のサイズ
		KEY_X,	//キーボードのx座標
		KEY_Y;	//キーボードのy座標
	char *KEY="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789   ()[]!#$%&@;  =+-'^.,_     ";
	int KEY_LEN;
	int cur=0, sel=0, i, x, y, t=0;
	char tmp[MAX_PATH];//, *p;

	WFONTS=13;
	HFONTS=7;
	KEY_W=(WFONTS*3+4)*FONT_WIDTH;
	KEY_H=(HFONTS+4.5)*FONT_HEIGHT;
	KEY_X=(SCREEN_WIDTH-KEY_W)/2;
	KEY_Y=(SCREEN_HEIGHT-KEY_H)/2;
	
/*
	//キャレットを拡張子の前に移動
	p=strrchr(out, '.');
	if(p==NULL)
		cur=strlen(out);
	else
		cur=(int)(p-out);
*/
	//キャレットを文字列の先頭に移動
	cur=0;
	KEY_LEN = strlen(KEY);

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad & PAD_UP){
				if(sel<WFONTS*HFONTS){
					if(sel>=WFONTS) sel-=WFONTS;
				}
				else{
					if(sel==WFONTS*HFONTS) sel=82;	//カーソルがOKにあるときに上を押した
					else sel=86;					//カーソルガCANCELにあるときに上を押した
				}
			}
			else if(new_pad & PAD_DOWN){
				if(sel/WFONTS == HFONTS-1){
					if(sel%WFONTS < 6)		//カーソルが中心より左にあるときOKに移動
						sel=WFONTS*HFONTS;
					else					//カーソルが中心より右にあるときCANCELに移動
						sel=WFONTS*HFONTS+1;
				}else if(sel/WFONTS <= HFONTS-2)
					sel+=WFONTS;
			}
			else if(new_pad & PAD_LEFT){
				if(sel>0) sel--;
			}
			else if(new_pad & PAD_RIGHT){
				if(sel<=WFONTS*HFONTS) sel++;
			}
			else if(new_pad & PAD_START){
				sel = WFONTS*HFONTS;
			}
			else if(new_pad & PAD_SELECT){
				sel = WFONTS*HFONTS+1;
			}
			else if(new_pad & PAD_L1){
				if(cur>0) cur--;
				t=0;
			}
			else if(new_pad & PAD_R1){
				if(cur<strlen(out)) cur++;
				t=0;
			}
			else if(new_pad & PAD_CROSS){
				if(cur>0){
					strcpy(tmp, out);
					out[cur-1]=0;
					strcat(out, &tmp[cur]);
					cur--;
					t=0;
				}
			}
			else if(new_pad & PAD_CIRCLE){
				i=strlen(out);
				if(sel < WFONTS*HFONTS){
					if(i<max && i<33){
						strcpy(tmp, out);
						out[cur]=KEY[sel];
						out[cur+1]=0;
						strcat(out, &tmp[cur]);
						cur++;
						t=0;
					}
				}else if(sel == WFONTS*HFONTS && i>0){
					break;
				}else
					return -1;
			}
		}
		// 描画開始
		drawDialogTmp(KEY_X, KEY_Y, KEY_X+KEY_W, KEY_Y+KEY_H, setting->color[0], setting->color[1]);
		//キーボード内側の枠
		drawFrame(KEY_X+FONT_WIDTH, KEY_Y+FONT_HEIGHT*1.5,
			KEY_X+KEY_W-FONT_WIDTH, KEY_Y+FONT_HEIGHT*9.5, setting->color[1]);
		//入力中の文字列の表示
		printXY(out,
			KEY_X+FONT_WIDTH*2, KEY_Y+FONT_HEIGHT*0.5,
			setting->color[3], TRUE);
		t++;
		//キャレット
		if(t<SCANRATE/2){
			printXY("|",
				KEY_X+FONT_WIDTH*0.5+(cur+1)*FONT_WIDTH,
				KEY_Y+FONT_HEIGHT*0.5,
				setting->color[3], TRUE);
		}
		else{
			if(t==SCANRATE) t=0;
		}

		//カーソル表示
		//アルファブレンド有効
		itoPrimAlphaBlending( TRUE );
		if(sel<WFONTS*HFONTS){	//OKとCANCEL以外
			x = KEY_X+FONT_WIDTH*2 + (sel%WFONTS)*FONT_WIDTH*3;
			y = KEY_Y+FONT_HEIGHT*2 + (sel/WFONTS)*FONT_HEIGHT;
			itoSprite(setting->color[2]|0x10000000,
				x, y,
				x+FONT_WIDTH*3, y+FONT_HEIGHT, 0);
		}
		else{
			if(sel==WFONTS*HFONTS)
				x = KEY_X+KEY_W/4;	//OK
			else
				x = KEY_X+KEY_W/2;	//CANCEL
			y = KEY_Y+FONT_HEIGHT*10;
			itoSprite(setting->color[2]|0x10000000,
				x, y,
				x+KEY_W/4, y+FONT_HEIGHT, 0);
		}
		//アルファブレンド無効
		itoPrimAlphaBlending(FALSE);

		//キーボード表示
		for(i=0; i<KEY_LEN; i++){
			sprintf(tmp,"%c",KEY[i]);
			printXY(tmp,
				KEY_X+FONT_WIDTH*3 + (i%WFONTS)*FONT_WIDTH*3,
				KEY_Y+FONT_HEIGHT*2 + (i/WFONTS)*FONT_HEIGHT,
				setting->color[3], TRUE);
		}
		//OK表示
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_ok))/2;
		sprintf(tmp, "%s",lang->gen_ok);
		printXY(tmp, KEY_X+KEY_W/4+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
		//CANCEL表示
		x=((KEY_W/4)-FONT_WIDTH*strlen(lang->gen_cancel))/2;
		sprintf(tmp, "%s",lang->gen_cancel);
		printXY(tmp, KEY_X+KEY_W/2+x, KEY_Y+FONT_HEIGHT*10, setting->color[3], TRUE);
		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY(lang->filer_keyboard_hint, x, y, setting->color[3], TRUE);
		drawScr();
	}
	return 0;
}

//-------------------------------------------------
// ファイルリスト設定
int setFileList(const char *path, const char *ext, FILEINFO *files, int cnfmode)
{
	char *p;
	int nfiles, i, j, ret=0;
	char fullpath[MAX_PATH];

	int checkELFret;
	FILEINFO file;
	char party[MAX_NAME], dir[MAX_PATH];

	char tmp[16*4+1];

	// ファイルリスト設定
	if(path[0]==0){
		for(i=0;i<5;i++){
			memset(&files[i].modifyTime,0,sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			files[i].attr = FIO_S_IFDIR;
			if(i==0) strcpy(files[i].name, "mc0:");
			if(i==1) strcpy(files[i].name, "mc1:");
			if(i==2) strcpy(files[i].name, "hdd0:");
			if(i==3) strcpy(files[i].name, "cdfs:");
			if(i==4) strcpy(files[i].name, "mass:");
			files[i].title[0] = 0;
			files[i].type = TYPE_OTHER;
		}
		nfiles = 5;
		if(cnfmode==ELF_FILE){
			memset(&files[5].modifyTime,0,sizeof(PS2TIME));
			files[5].fileSizeByte = 0;
			files[5].attr = FIO_S_IFDIR;
			strcpy(files[5].name, "MISC");
			files[5].type = TYPE_OTHER;
			files[5].title[0] = 0;
			nfiles = 6;
		}
	}
	else if(!strcmp(path, "MISC/")){
		for(i=0;i<7;i++){
			memset(&files[i].modifyTime,0,sizeof(PS2TIME));
			files[i].fileSizeByte = 0;
			if(i==0)
				files[i].attr = FIO_S_IFDIR;
			else
				files[i].attr = FIO_S_IFREG;
			if(i==0) strcpy(files[i].name, "..");
			if(i==1) strcpy(files[i].name, "FileBrowser");
			if(i==2) strcpy(files[i].name, "PS2Browser");
			if(i==3) strcpy(files[i].name, "PS2Disc");
			if(i==4) strcpy(files[i].name, "PS2Net");	//PS2Net uLaunchELF3.60
			if(i==5) strcpy(files[i].name, "INFO");
			if(i==6) strcpy(files[i].name, "CONFIG");
			files[i].title[0] = 0;
			files[i].type = TYPE_OTHER;
		}
		nfiles = 7;
	}
	else{
		//files[0]を初期化
		memset(&files[0].modifyTime, 0, sizeof(PS2TIME));
		files[0].fileSizeByte = 0;
		files[0].attr = FIO_S_IFDIR;
		strcpy(files[0].name, "..");
		files[0].title[0] = 0;
		files[0].type=TYPE_OTHER;

		//ファイルリストとファイル数を取得
		nfiles = getDir(path, &files[1]) + 1;
		if(strcmp(ext, "*")){	//ファイルマスク
			for(i=j=1; i<nfiles; i++){
				if(files[i].attr & FIO_S_IFDIR)
					files[j++] = files[i];
				else{
					p = strrchr(files[i].name, '.');
					if(p!=NULL && !stricmp(ext,p+1))
						files[j++] = files[i];
				}
			}
			nfiles = j;
		}

		//ゲームタイトルとファイルタイプを取得
		for(i=1; i<nfiles; i++){
			//ゲームタイトル取得
			if( !strncmp(path, "cdfs", 4)){
				//cdfs
				if(setting->discPs2saveCheck){
					ret = getGameTitle(path, &files[i], tmp);
					if(ret<0) tmp[0]=0;
				}
				else{
					ret=-1;
					tmp[0]=0;
				}
			}
			else{
				//mcとhddとmass
				ret = getGameTitle(path, &files[i], tmp);
				if(ret<0) tmp[0]=0;
			}
			//sjisの英数字と記号をASCIIに変換
			memset(files[i].title, 0, 65);
			sjis2ascii(tmp, files[i].title);

			//タイプ取得
			if(files[i].attr & FIO_S_IFDIR){	//フォルダ
				if(ret<0)
					files[i].type=TYPE_DIR;
				else
					files[i].type=TYPE_PS2SAVE;	//PS2SAVE
			}
			else if(files[i].attr & FIO_S_IFREG){	//ファイル
				sprintf(fullpath, "%s%s", path, files[i].name);
				//ELFヘッダを調べる
				if(!strncmp(path, "mc", 2) || !strncmp(path, "mass", 4)){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
				}
				else if( !strncmp(path, "hdd", 3)&&strcmp(path, "hdd0:/") ){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					mountedParty[0][0]=0;
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
					//HDDのとき再マウント
					strcpy(file.name, files[i].name);
					strcpy(file.title, files[i].title);
					file.attr=files[i].attr;
					file.type=files[i].type;
					//
					getHddParty(path, &file, party, dir);
					mountParty(party);
				}
				else if( !strncmp(path, "cdfs", 4)){
					if(setting->discELFCheck){
						checkELFret = checkELFheader(fullpath); 	//checkELFheader
						if(checkELFret<0)
							files[i].type=TYPE_FILE;
						else
							files[i].type=TYPE_ELF;
					}
					else{
						files[i].type=TYPE_FILE;
					}
				}
			}
		}
		//ソート
		if(nfiles>1)
			sort(&files[1], 0, nfiles-2);
	}
	
	return nfiles;
}

//-------------------------------------------------
// 任意のファイルパスを返す
void getFilePath(char *out, int cnfmode)
{
	char path[MAX_PATH], oldFolder[MAX_PATH],
		msg0[MAX_PATH], msg1[MAX_PATH],
		tmp[MAX_PATH], ext[8], *p;
	uint64 color,iconcolor=0;
	FILEINFO files[MAX_ENTRY];
	int nfiles=0, sel=0, top=0;
	int cd=TRUE, up=FALSE, pushed=TRUE;
	int x, y, y0, y1;
	int i, ret;//, fd;
	size_t size;
	int code;
	int detail=0;	//詳細表示 0:なし 1:サイズ 2:更新日時
	size_t freeSpace=0;
	int mcfreeSpace=0;
	int vfreeSpace=FALSE;	//空き容量表示フラグ

	if(cnfmode==ANY_FILE)
		strcpy(ext, "*");
	else if(cnfmode==ELF_FILE)
		strcpy(ext, "elf");
	else if(cnfmode==DIR)
		strcpy(ext, "");
	else if(cnfmode==FNT_FILE)
		strcpy(ext, "fnt");

	strcpy(path, LastDir);
	mountedParty[0][0]=0;
	mountedParty[1][0]=0;
	clipPath[0] = 0;
	nclipFiles = 0;
	cut = 0;
	title=FALSE;
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
				up=TRUE;
			else if(new_pad & PAD_CIRCLE){	//change dir
				if(files[sel].attr & FIO_S_IFDIR){
					if(!strcmp(files[sel].name,".."))
						up=TRUE;
					else{
						strcat(path, files[sel].name);
						strcat(path, "/");
						cd=TRUE;
					}
				}
			}
			else if(new_pad & PAD_SELECT){	//戻る
				break;
			}
			else if(new_pad & PAD_L1) {	// タイトル表示切り替え
				title = !title;
				cd=TRUE;
			}
			else if(new_pad & PAD_R2){	//GETSIZE
				if(path[0]==0 || !strcmp(path,"hdd0:/") || !strcmp(path,"MISC/")){
				}
				else if(nmarks==0 && !strcmp(files[sel].name, "..")){
				}
				else{
					if(nmarks==0){
						drawMsg("SIZE =");
						size=getFileSize(path, &files[sel]);
					}
					else{
						drawMsg(lang->filer_checkingsize);
						for(i=size=0; i<nfiles; i++){
							if(marks[i])
								size+=getFileSize(path, &files[i]);
							if(size<0) size=-1;
						}
					}
					//
					if(size<0){
						strcpy(msg0, lang->filer_getsizefailed);
					}
					else{
						if(size >= 1024*1024)
							sprintf(msg0, "SIZE = %.1f MByte", (double)size/1024/1024);
						else if(size >= 1024)
							sprintf(msg0, "SIZE = %.1f KByte", (double)size/1024);
						else
							sprintf(msg0, "SIZE = %d Byte", size);
					}
					pushed = FALSE;
				}
			}
			else if(new_pad & PAD_R3){	//FILEICON
				setting->fileicon = !setting->fileicon;
				if(setting->fileicon) cd=TRUE;
			}
			else if(new_pad & PAD_L3){	//FLICKERCONTROL
				setting->flickerControl = !setting->flickerControl;
			}
			else if(new_pad & PAD_L2) {	//詳細表示
				detail++;
				if(detail==3) detail=0;
			}
			//ELF_FILE ELF選択時
			if(cnfmode==ELF_FILE){
				if(new_pad & PAD_CIRCLE) {	//ファイルを決定
					if(files[sel].attr & FIO_S_IFREG){
						int ret;
						sprintf(out, "%s%s", path, files[sel].name);
						ret = checkELFheader(out);	//ヘッダチェック
						mountedParty[0][0]=0;
						if(ret==1){
							//ELFファイル選択
							strcpy(LastDir, path);
							break;
						}
						else{
							//ELFファイルではないとき
							char *extension;
							extension = getExtension(files[sel].name);
							if(extension!=NULL){
								if(!stricmp(extension, ".psb")){
									strcpy(LastDir, path);
									break;
								}
							}
						}
						pushed=FALSE;
						sprintf(msg0, lang->filer_not_elf);
						out[0] = 0;
					}
				}
				else if(new_pad & PAD_SQUARE) {	// ファイルマスク切り替え
					if(!strcmp(ext,"*")) strcpy(ext, "elf");
					else				 strcpy(ext, "*");
					cd=TRUE;
				}
				else if(new_pad & PAD_CROSS){	//戻る
					break;
				}
			}
			//FNT_FILE FNT選択時
			else if(cnfmode==FNT_FILE){
				if(new_pad & PAD_CIRCLE) {//FNTファイルを決定
					if(files[sel].attr & FIO_S_IFREG){
						sprintf(out, "%s%s", path, files[sel].name);
						//ヘッダチェック
						if(checkFONTX2header(out)<0){
							mountedParty[0][0]=0;
							pushed=FALSE;
							sprintf(msg0, lang->filer_not_fnt);
							out[0] = 0;
						}
						else{
							strcpy(LastDir, path);
							break;
						}
					}
				}
				else if(new_pad & PAD_SQUARE) {	// ファイルマスク切り替え
					if(!strcmp(ext,"*")) strcpy(ext, "fnt");
					else				 strcpy(ext, "*");
					cd=TRUE;
				}
				else if(new_pad & PAD_CROSS){	//戻る
					break;
				}
			}
			//DIR ディレクトリ選択時
			else if(cnfmode==DIR){
				if(new_pad & PAD_START) {	//ディレクトリを決定
					if( path[0]!=0 && strcmp(path, "hdd0:/")!=0 && strncmp(path, "cdfs", 4)!=0 ){
						strcpy(out, path);
						break;
					}
				}
				else if(new_pad & PAD_CROSS){	//戻る
					break;
				}
			}
			//ANY_FILE	ファイラーモード	すべてのファイルが対象
			else if(cnfmode==ANY_FILE){
				if(new_pad & PAD_CIRCLE) {
					if(files[sel].attr & FIO_S_IFREG){	//ファイル
						char fullpath[MAX_PATH];
						int ret;
						sprintf(fullpath, "%s%s", path, files[sel].name);
						ret = checkELFheader(fullpath);
						mountedParty[0][0]=0;
						if(ret==1){
							//ELFファイル選択
							strcpy(out, fullpath);
							strcpy(LastDir, path);
							break;
						}
						else{
							//ELFファイルではないとき
							char *extension;
							pushed=FALSE;
							sprintf(msg0, lang->filer_not_elf);
							extension = getExtension(fullpath);
							if(extension!=NULL){
								if(!stricmp(extension, ".psb")){	//psbファイルを実行
									int ynret;
									int psbret;
									ynret = ynDialog(lang->filer_execute_psb, 0);
									if(ynret>0){
										psbret = psb(fullpath);
										if(psbret==0){
											pushed=TRUE;
											cd=TRUE;	//空きスペース再計算
										}
										else if(psbret>0){
											sprintf(msg0, "error line no = %d", psbret);
										}
										else if(psbret<0){
											strcpy(msg0, "psb open error");
										}
									}
								}
							}
						}
					}
				}
				else if(new_pad & PAD_R1){	// メニュー
					drawDark();
					itoGsFinish();
					itoSwitchFrameBuffers();
					drawDark();

					//メニュー
					ret = menu(path, files[sel].name);

					if(ret==COPY || ret==CUT){	// クリップボードにコピー
						strcpy(clipPath, path);
						if(nmarks>0){
							for(i=nclipFiles=0; i<nfiles; i++)
								if(marks[i])
									clipFiles[nclipFiles++]=files[i];
						}
						else{
							clipFiles[0]=files[sel];
							nclipFiles = 1;
						}
						sprintf(msg0, lang->filer_copy_to_clip);
						pushed=FALSE;
						if(ret==CUT)	cut=TRUE;
						else			cut=FALSE;
					}
					else if(ret==DELETE){	// デリート
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();
						if(nmarks==0){
							if(title && files[sel].title[0])
								sprintf(tmp,"%s",files[sel].title);
							else{
								sprintf(tmp,"%s",files[sel].name);
								if(files[sel].attr & FIO_S_IFDIR)
									strcat(tmp,"/");
							}
							strcat(tmp, "\n");
							strcat(tmp, lang->filer_delete);
							ret = ynDialog(tmp,0);
						}
						else
							ret = ynDialog(lang->filer_deletemarkfiles,0);

						if(ret>0){
							if(nmarks==0){
								strcpy(tmp, files[sel].name);
								if(files[sel].attr & FIO_S_IFDIR) strcat(tmp,"/");
								strcat(tmp, " ");
								strcat(tmp, lang->filer_deleting);
								drawMsg(tmp);
								ret=delete(path, &files[sel]);
							}
							else{
								for(i=0; i<nfiles; i++){
									if(marks[i]){
										strcpy(tmp, files[i].name);
										if(files[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
										strcat(tmp, " ");
										strcat(tmp, lang->filer_deleting);
										drawMsg(tmp);
										ret=delete(path, &files[i]);
										if(ret<0) break;
									}
								}
							}
							if(ret>=0){
								cd=TRUE;	//空きスペース再計算
							}
							else{
								strcpy(msg0, lang->filer_deletefailed);
								pushed = FALSE;
							}
						}
					}
					else if(ret==RENAME){	// リネーム
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();
						strcpy(tmp, files[sel].name);
						if(keyboard(tmp, 36)>=0){
							if(Rename(path, &files[sel], tmp)<0){
								pushed=FALSE;
								strcpy(msg0, lang->filer_renamefailed);
							}
							else
								cd=TRUE;
						}
					}
					else if(ret==PASTE){	// クリップボードからペースト
						drawMsg(lang->filer_pasting);
						ret=paste(path);
						if(ret < 0){
							strcpy(msg0, lang->filer_pastefailed);
							pushed = FALSE;
						}
						else{
							if(cut) nclipFiles=0;
						}
						cd=TRUE;
					}
					else if(ret==NEWDIR){	// 新規フォルダ作成
						tmp[0]=0;
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();
						if(keyboard(tmp, 36)>=0){
							ret = newdir(path, tmp);
							if(ret == -17){
								strcpy(msg0, lang->filer_direxists);
								pushed=FALSE;
							}
							else if(ret < 0){
								strcpy(msg0, lang->filer_newdirfailed);
								pushed=FALSE;
							}
							else{
								strcat(path, tmp);
								strcat(path, "/");
								cd=TRUE;
							}
						}
					}
					else if(ret==GETSIZE){	// サイズ表示
						if(nmarks==0){
							drawMsg("SIZE =");
							size=getFileSize(path, &files[sel]);
						}
						else{
							drawMsg(lang->filer_checkingsize);
							for(i=size=0; i<nfiles; i++){
								if(marks[i])
									size+=getFileSize(path, &files[i]);
								if(size<0) size=-1;
							}
						}
						//
						if(size<0){
							strcpy(msg0, lang->filer_getsizefailed);
						}
						else{
 							if(size >= 1024*1024)
								sprintf(msg0, "SIZE = %.1f MByte", (double)size/1024/1024);
							else if(size >= 1024)
								sprintf(msg0, "SIZE = %.1f KByte", (double)size/1024);
							else
								sprintf(msg0, "SIZE = %d Byte", size);
						}
						pushed = FALSE;
					}
					else if(ret==EXPORT){	// psuファイルにエクスポート
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();

						ret = psuExport(path, &files[sel]);
						if(ret<0){
							sprintf(msg0, "%s %d", lang->filer_exportfailed, ret);
							pushed = FALSE;
						}
						else{
							if(setting->Exportdir[0])
								strcpy(tmp,setting->Exportdir);
							else
								strcpy(tmp,path);
							sprintf(msg0, "%s %s", lang->filer_exportto, tmp);
							pushed = FALSE;
							cd = TRUE;
						}
					}
					else if(ret==IMPORT){	// psuファイルからインポート
						drawDark();
						itoGsFinish();
						itoSwitchFrameBuffers();
						drawDark();

						ret = psuImport(path, &files[sel]);
						if(ret<0){
							sprintf(msg0, "%s %d", lang->filer_importfailed, ret);
							pushed = FALSE;
						}
						else{
							if(ret==0) strcpy(tmp,"mc0:/");
							else strcpy(tmp,"mc1:/");
							sprintf(msg0, "%s %s", lang->filer_importto, tmp);
							pushed = FALSE;
							cd = TRUE;
						}
					}
				}
				else if(new_pad & PAD_CROSS) {	// マーク
					if(sel!=0 && path[0]!=0 && strcmp(path,"hdd0:/")){
						if(marks[sel]){
							marks[sel]=FALSE;
							nmarks--;
						}
						else{
							marks[sel]=TRUE;
							nmarks++;
						}
					}
					sel++;
				}
				else if(new_pad & PAD_SQUARE) {	// マーク反転
					if(path[0]!=0 && strcmp(path,"hdd0:/")){
						for(i=1; i<nfiles; i++){
							if(marks[i]){
								marks[i]=FALSE;
								nmarks--;
							}
							else{
								marks[i]=TRUE;
								nmarks++;
							}
						}
					}
				}
			}
		}
		// 上位フォルダ移動
		if(up){
			if((p=strrchr(path, '/'))!=NULL)
				*p = 0;
			if((p=strrchr(path, '/'))!=NULL){
				p++;
				strcpy(oldFolder, p);
				*p = 0;
			}
			else{
				strcpy(oldFolder, path);
				path[0] = 0;
			}
			cd=TRUE;
		}
		//フォルダ移動（移動先のフォルダが現在のフォルダと同じときはファイルリストを更新）
		if(cd){
			nfiles = setFileList(path, ext, files, cnfmode);
			// 空き容量取得
			vfreeSpace=FALSE;	//空き容量表示フラグ
			if(cnfmode==ANY_FILE){
				if(!strncmp(path, "mc", 2)){
					mcGetInfo(path[2]-'0', 0, NULL, &mcfreeSpace, NULL);
					mcSync(0,NULL,NULL);
					freeSpace = mcfreeSpace*1024;
					vfreeSpace=TRUE;
				}
				else if(!strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/")){
					freeSpace = fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_FREE,NULL,0,NULL,0)*fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_SIZE,NULL,0,NULL,0);
					vfreeSpace=TRUE;
				}
			}
			// 変数初期化
			sel=0;
			top=0;
			if(up){
				for(i=0; i<nfiles; i++){
					if(!strcmp(oldFolder, files[i].name)){
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			nmarks = 0;
			memset(marks, 0, MAX_ENTRY);
			cd=FALSE;
			up=FALSE;
		}
		if(strncmp(path,"cdfs",4) && setting->discControl)
			CDVD_Stop();
		// ファイルリスト表示用変数の正規化
		if(top > nfiles-MAX_ROWS)	top=nfiles-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		clrScr(setting->color[0]);
		// ファイルリスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<MAX_ROWS; i++){
			if(top+i >= nfiles)
				break;
			if(top+i == sel){
				color = setting->color[2];
				printXY(">", x, y, color, TRUE);	//カーソル表示
			}
			else
				color = setting->color[3];

			//マーク表示
			if(marks[top+i]){
				printXY("*", x+FONT_WIDTH, y, setting->color[3], TRUE);
			}

			//ファイルリスト表示
			if(files[top+i].attr & FIO_S_IFDIR){	//フォルダのとき
				if(!strcmp(files[top+i].name,".."))
					strcpy(tmp,"..");
				else if(title && files[top+i].title[0]!=0)
					strcpy(tmp,files[top+i].title);	//ゲームタイトル
				else
					sprintf(tmp, "%s/", files[top+i].name);	//フォルダ名
			}
			else
				strcpy(tmp,files[top+i].name);	//ファイル名

			//ファイル名が長いときは、短くする
			if(strlen(tmp)>52){
				tmp[53]=0;
				code=tmp[52];
				if( (code>=0x81)&&(code<=0x9F) ) tmp[52] = 0;
				if( (code>=0xE0)&&(code<=0xFF) ) tmp[52] = 0;
				strcat(tmp,"...");
			}

			//
			if(setting->fileicon){
				//ファイル名とアイコンを表示
				if(files[top+i].type!=TYPE_OTHER){
					if(files[top+i].type==TYPE_DIR) iconcolor=setting->color[4];
					else if(files[top+i].type==TYPE_FILE) iconcolor=setting->color[5];
					else if(files[top+i].type==TYPE_PS2SAVE) iconcolor=setting->color[6];
					else if(files[top+i].type==TYPE_ELF) iconcolor=setting->color[7];
					//アイコン
					itoSprite(iconcolor,
						x+FONT_WIDTH*2, y,
						x+FONT_WIDTH*2+FONT_WIDTH, y+(FONT_HEIGHT - GetFontMargin(LINE_MARGIN)), 0);
				}
				//ファイル名表示
				printXY(tmp, x+FONT_WIDTH*4, y, color, TRUE);
			}
			else{
				//ファイル名のみ表示
				printXY(tmp, x+FONT_WIDTH*2, y, color, TRUE);
			}

			//詳細表示
			if(path[0]==0 || !strcmp(path,"hdd0:/") || !strcmp(path,"MISC/")){
				//何もしない
			}
			else{
				if(detail==1){	//ファイルサイズ表示
					int len;
					if(files[top+i].attr & FIO_S_IFDIR)
						sprintf(tmp,"<DIR>");
					else{
						if(files[top+i].fileSizeByte >= 1024*1024)
							sprintf(tmp, "%.1f MB", (double)files[top+i].fileSizeByte/1024/1024);
						else if(files[top+i].fileSizeByte >= 1024)
							sprintf(tmp, "%.1f KB", (double)files[top+i].fileSizeByte/1024);
						else
							sprintf(tmp,"%d B ",files[top+i].fileSizeByte);
					}
					len=strlen(tmp);
					if(strcmp(files[top+i].name,"..")){
						itoSprite(setting->color[0],
							SCREEN_WIDTH-FONT_WIDTH*14, y,
							SCREEN_WIDTH-FONT_WIDTH*4, y+FONT_HEIGHT, 0);
						itoLine(setting->color[1], SCREEN_WIDTH-FONT_WIDTH*13.5, y, 0,
							setting->color[1], SCREEN_WIDTH-FONT_WIDTH*13.5, y+FONT_HEIGHT, 0);	
						printXY(tmp, SCREEN_WIDTH-FONT_WIDTH*(4+len), y, color, TRUE);
					}
				}
				else if(detail==2){	//更新日時表示
					int len;
					//cdfsは、更新日時を取得できない
					if(!strncmp(path,"cdfs",4)){
						strcpy(tmp,"----/--/-- --:--:--");
					}
					else{
						sprintf(tmp,"%04d/%02d/%02d %02d:%02d:%02d",
							files[top+i].modifyTime.year,
							files[top+i].modifyTime.month,
							files[top+i].modifyTime.day,
							files[top+i].modifyTime.hour,
							files[top+i].modifyTime.min,
							files[top+i].modifyTime.sec);
					}
					len=strlen(tmp);
					if(strcmp(files[top+i].name,"..")){
						itoSprite(setting->color[0],
							SCREEN_WIDTH-FONT_WIDTH*24, y,
							SCREEN_WIDTH-FONT_WIDTH*4, y+FONT_HEIGHT, 0);
						itoLine(setting->color[1], SCREEN_WIDTH-FONT_WIDTH*23.5, y, 0,
							setting->color[1], SCREEN_WIDTH-FONT_WIDTH*23.5, y+FONT_HEIGHT, 0);	
						printXY(tmp, SCREEN_WIDTH-FONT_WIDTH*(4+len), y, color, TRUE);
					}
				}
			}
			y += FONT_HEIGHT;
		}
		// スクロールバー
		if(nfiles > MAX_ROWS){
			drawFrame(SCREEN_WIDTH-FONT_WIDTH*3, SCREEN_MARGIN+FONT_HEIGHT*3,
				SCREEN_WIDTH-FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3),setting->color[1]);
			y0=FONT_HEIGHT*MAX_ROWS*((double)top/nfiles);
			y1=FONT_HEIGHT*MAX_ROWS*((double)(top+MAX_ROWS)/nfiles);
			itoSprite(setting->color[1],
				SCREEN_WIDTH-FONT_WIDTH*3,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				SCREEN_WIDTH-FONT_WIDTH*2,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "Path: %s", path);
		// 操作説明
		if(cnfmode==ANY_FILE){
			if(title)
				sprintf(msg1, lang->filer_anyfile_hint1);
			else
				sprintf(msg1, lang->filer_anyfile_hint2);
		}
		else if(cnfmode==ELF_FILE){
			if(!strcmp(ext, "*"))
				sprintf(msg1, lang->filer_elffile_hint1);
			else
				sprintf(msg1, lang->filer_elffile_hint2);
		}
		else if(cnfmode==FNT_FILE){
			if(!strcmp(ext, "*"))
				sprintf(msg1, lang->filer_fntfile_hint1);
			else
				sprintf(msg1, lang->filer_fntfile_hint2);
		}
		else if(cnfmode==DIR){
			sprintf(msg1, lang->filer_dir_hint);
		}
		setScrTmp(msg0, msg1);

		// フリースペース表示
		if(vfreeSpace){
			if(freeSpace >= 1024*1024)
				sprintf(tmp, "[%.1fMB free]", (double)freeSpace/1024/1024);
			else if(freeSpace >= 1024)
				sprintf(tmp, "[%.1fKB free]", (double)freeSpace/1024);
			else
				sprintf(tmp, "[%dB free]", freeSpace);
			ret=strlen(tmp);
			//
			printXY(tmp,
				SCREEN_WIDTH-FONT_WIDTH*(ret+2), SCREEN_MARGIN,
				setting->color[3], TRUE);
		}
		drawScr();
	}
	
	if(mountedParty[0][0]!=0){
		fileXioUmount("pfs0:");
		mountedParty[0][0]=0;
	}
	if(mountedParty[1][0]!=0){
		fileXioUmount("pfs1:");
		mountedParty[1][0]=0;
	}
	return;
}
