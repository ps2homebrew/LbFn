#include "launchelf.h"

typedef struct{
	char name[256];
	char title[16*4+1];
	unsigned short attr;
	int type;
} FILEINFO;

// psuファイルヘッダ構造体
typedef struct { // 512 bytes
	unsigned int  attr;
	unsigned int  size;	//file size, 0 for directory
	unsigned char createtime[8];	//0x00:sec:min:hour:day:month:year
	unsigned int unknown1;
	unsigned int unknown2;
	unsigned char modifytime[8];	//0x00:sec:min:hour:day:month:year
	unsigned char unknown3[32];
	unsigned char name[32];
	unsigned char unknown4[416];
} PSU_HEADER;

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

enum
{
	TYPE_OTHER,
	TYPE_DIR,
	TYPE_FILE,
	TYPE_PS2SAVE,
	TYPE_ELF
};

unsigned char *elisaFnt=NULL;
size_t freeSpace;
int mcfreeSpace;
int vfreeSpace;
int cut;
int nclipFiles, nmarks, nparties;
int title;
char mountedParty[2][MAX_NAME];
char parties[MAX_PARTITIONS][MAX_NAME];
char clipPath[MAX_PATH], LastDir[MAX_NAME], marks[MAX_ENTRY];
FILEINFO clipFiles[MAX_ENTRY];
int fileMode =  FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
// 確認ダイアログ
int ynDialog(const char *message)
{
	char msg[512];
	int dh, dw, dx, dy;
	int sel=0, a=6, b=4, c=2, n, tw;
	int i, x, len, ret;
	
	strcpy(msg, message);
	
	for(i=0,n=1; msg[i]!=0; i++){
		if(msg[i]=='\n'){
			msg[i]=0;
			n++;
		}
	}
	//文字列の表示
	for(i=len=tw=0; i<n; i++){
		ret = printXY(&msg[len], 0, 0, 0, FALSE);
		if(ret>tw) tw=ret;
		len=strlen(&msg[len])+1;
	}

//	if(tw<108) tw=108;
	if(tw<130) tw=130;

	dh = 16*(n+1)+2*2+a+b+c;	//ダイアログの高さ
	dw = 2*2+a*2+tw;			//ダイアログの幅
	dx = (640-dw)/2;			//ダイアログのx
	dy = (432-dh)/2;			//ダイアログのy
	//printf("tw=%d\ndh=%d\ndw=%d\ndx=%d\ndy=%d\n", tw,dh,dw,dx,dy);
	
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
		drawDialogTmp(dx-2, dy-2,
			dx+dw+2, dy+dh+4,
			setting->color[0], setting->color[1]);
		for(i=len=0; i<n; i++){
			printXY(&msg[len], dx+2+a,(dy+a+2+i*16), setting->color[3],TRUE);
			len=strlen(&msg[len])+1;
		}
		x=(tw-96)/2;
		printXY(" OK   CANCEL", dx+a+x, (dy+a+b+2+n*16), setting->color[3],TRUE);
		if(sel==0)
			drawChar('>', dx+a+x,(dy+a+b+2+n*16), setting->color[3]);
		else
			drawChar('>',dx+a+x+50,(dy+a+b+2+n*16),setting->color[3]);
		drawScr();
	}
	x=(tw-96)/2;
	drawChar(' ', dx+a+x,(dy+a+b+2+n*16), setting->color[3]);
	drawChar(' ',dx+a+x+50,(dy+a+b+2+n*16),setting->color[3]);
	return ret;
}

////////////////////////////////////////////////////////////////////////
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
void sort(FILEINFO *a, int left, int right) {
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

////////////////////////////////////////////////////////////////////////
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
		(!strcmp(mcDir[i].name,".") || !strcmp(mcDir[i].name,"..")))
			continue;
		strcpy(info[j].name, mcDir[i].name);
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		j++;
	}
	
	return j;
}

////////////////////////////////////////////////////////////////////////
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
		 (!strcmp(TocEntryList[i].filename,".") ||
		  !strcmp(TocEntryList[i].filename,"..")))
			continue;
		strcpy(info[j].name, TocEntryList[i].filename);
		if(TocEntryList[i].fileProperties & 0x02)
			info[j].attr = FIO_S_IFDIR;
		else
			info[j].attr = FIO_S_IFREG;
		j++;
	}
	
	return j;
}

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
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
		}
		return nparties;
	}
	
	getHddParty(path,NULL,party,dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';
	
	if((fd=fileXioDopen(dir)) < 0) return 0;
	
	while(fileXioDread(fd, &dirbuf)){
		if(dirbuf.stat.mode & FIO_S_IFDIR &&
		(!strcmp(dirbuf.name,".") || !strcmp(dirbuf.name,"..")))
			continue;
		
		info[i].attr = dirbuf.stat.mode;
		strcpy(info[i].name, dirbuf.name);
		i++;
		if(i==max) break;
	}
	
	fileXioDclose(fd);
	
	return i;
}

////////////////////////////////////////////////////////////////////////
// USBマスストレージ読み込み
int readMASS(const char *path, FILEINFO *info, int max)
{
	fat_dir_record record;
	int ret, n=0;
	
	loadUsbModules();
	
	ret = usb_mass_getFirstDirentry((char*)path+5, &record);
	while(ret > 0){
		if(record.attr & 0x10 &&
		(!strcmp(record.name,".") || !strcmp(record.name,".."))){
			ret = usb_mass_getNextDirentry(&record);
			continue;
		}
		
		strcpy(info[n].name, record.name);
		if(record.attr & 0x10)
			info[n].attr = FIO_S_IFDIR;
		else
			info[n].attr = FIO_S_IFREG;
		n++;
		ret = usb_mass_getNextDirentry(&record);
	}
	
	return n;
}

////////////////////////////////////////////////////////////////////////
// ファイルリスト取得
int getDir(const char *path, FILEINFO *info)
{
	int max=MAX_ENTRY-2;
	int n;
	
	if(!strncmp(path, "mc", 2))			n=readMC(path, info, max);
	else if(!strncmp(path, "hdd", 3))	n=readHDD(path, info, max);
	else if(!strncmp(path, "mass", 4))	n=readMASS(path, info, max);
	else if(!strncmp(path, "cdfs", 4))	n=readCD(path, info, max);
	else return 0;
	
	return n;
}

///////////////////////////////////////////////////////////////////////////
// セーブデータタイトルの取得
int getGameTitle(const char *path, const FILEINFO *file, char *out)
{
	iox_dirent_t dirEnt;
	char party[MAX_NAME], dir[MAX_PATH];
	int fd=-1, dirfd=-1, size, hddin=FALSE, ret;
	
	if(file->attr & FIO_S_IFREG) return -1;
	if(path[0]==0 || !strcmp(path,"hdd0:/")) return -1;
	
	if(!strncmp(path, "hdd", 3)){
		ret = getHddParty(path, file, party, dir);
		if(mountParty(party)<0) return -1;
		dir[3]=ret+'0';
		hddin=TRUE;
	}else
		sprintf(dir, "%s%s/", path, file->name);
	
	ret = -1;
	if(hddin){
		if((dirfd=fileXioDopen(dir)) < 0) goto error;
		while(fileXioDread(dirfd, &dirEnt)){
			if(dirEnt.stat.mode & FIO_S_IFREG &&
			 !strcmp(dirEnt.name,"icon.sys")){
				strcat(dir, "icon.sys");
				if((fd=fileXioOpen(dir, O_RDONLY, fileMode)) < 0)
					goto error;
				if((size=fileXioLseek(fd,0,SEEK_END)) <= 0x100)
					goto error;
				fileXioLseek(fd,0xC0,SEEK_SET);
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
		if((size=fioLseek(fd,0,SEEK_END)) <= 0x100) goto error;
		fioLseek(fd,0xC0,SEEK_SET);
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

////////////////////////////////////////////////////////////////////////
// メニュー
int menu(const char *path, const char *file)
{
	uint64 color;
	char enable[NUM_MENU], tmp[64];
	int x, y, i, sel;
	
	int menu_x = FONT_WIDTH*47;
	int menu_y = FONT_HEIGHT*4;
	int menu_w = FONT_WIDTH*13;
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
	if(!strncmp(path,"cdfs",4)){
		enable[CUT] = FALSE;
		enable[PASTE] = FALSE;
		enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		enable[NEWDIR] = FALSE;
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}
	if(!strncmp(path, "mass", 4)){
		//enable[CUT] = FALSE;
		//enable[PASTE] = FALSE;
		//enable[DELETE] = FALSE;
		enable[RENAME] = FALSE;
		//enable[NEWDIR] = FALSE;
	}
	if(!strncmp(path, "mc", 2))
		enable[RENAME] = FALSE;
	if(!strncmp(path, "hdd", 3)){
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}

	if(nmarks==0){	//マークしたファイルがない
		if(!strcmp(file, "..")){	//ファイルが".."
			enable[COPY] = FALSE;
			enable[CUT] = FALSE;
			enable[DELETE] = FALSE;
			enable[RENAME] = FALSE;
			enable[GETSIZE] = FALSE;
			enable[EXPORT] = FALSE;
			enable[IMPORT] = FALSE;
		}
	}
	else{	//マークしたファイルがある
		enable[RENAME] = FALSE;
		enable[EXPORT] = FALSE;
		enable[IMPORT] = FALSE;
	}

	if(nclipFiles==0)	//クリップボードに記憶したファイルがない
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
		}
		
		// 描画開始
		drawDialogTmp(menu_x, menu_y, menu_x+menu_w, menu_y+menu_h, setting->color[0], setting->color[1]);
		for(i=0,y=74; i<NUM_MENU; i++){
			if(i==COPY)			strcpy(tmp, "Copy");
			else if(i==CUT)		strcpy(tmp, "Cut");
			else if(i==PASTE)	strcpy(tmp, "Paste");
			else if(i==DELETE)	strcpy(tmp, "Delete");
			else if(i==RENAME)	strcpy(tmp, "Rename");
			else if(i==NEWDIR)	strcpy(tmp, "New Dir");
			else if(i==GETSIZE) strcpy(tmp, "Get Size");
			else if(i==EXPORT) strcpy(tmp, "Export psu");
			else if(i==IMPORT) strcpy(tmp, "Import psu");
			
			if(enable[i])
				color = setting->color[2];
			else
				color = setting->color[3];
			
			printXY(tmp, menu_x+FONT_WIDTH*2, menu_y+FONT_HEIGHT/2+i*FONT_HEIGHT, color, TRUE);
			y+=FONT_HEIGHT;
		}
		if(sel<NUM_MENU)
			drawChar('>', menu_x+FONT_WIDTH, menu_y+FONT_HEIGHT/2+sel*FONT_HEIGHT, setting->color[2]);
		
		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY("○:OK ×:Cancel", x, y, setting->color[3], TRUE);
		drawScr();
	}
	
	return sel;
}

//////////////////////////////////////////////////////////////////////////
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
			fioClose(fd);;
		}
	}
	return size;
}

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// ファイルコピー
int copy(const char *outPath, const char *inPath, FILEINFO file, int n)
{
	FILEINFO files[MAX_ENTRY];
	char out[MAX_PATH], in[MAX_PATH], tmp[MAX_PATH],
		*buff=NULL, inParty[MAX_NAME], outParty[MAX_NAME];
	int hddout=FALSE, hddin=FALSE, nfiles, i;
	size_t size, outsize;
	int ret=-1, pfsout=-1, pfsin=-1, in_fd=-1, out_fd=-1, buffSize;
	
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
	}else
		sprintf(in, "%s%s", inPath, file.name);
	//入力パスがHDDのときマウント
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
	}else
		sprintf(out, "%s%s", outPath, file.name);
	
	// フォルダの場合
	if(file.attr & FIO_S_IFDIR){
		// フォルダ作成
		ret = newdir(outPath, file.name);
		if(ret == -17){
			drawDark();
			itoSwitchFrameBuffers();
			drawDark();
			ret=-1;
			if(title) ret=getGameTitle(outPath, &file, tmp);
			if(ret<0) sprintf(tmp, "%s%s/", outPath, file.name);
			strcat(tmp, "\nOverwrite?");
			if(ynDialog(tmp)<0) return -1;
			drawMsg("Pasting...");
		} else if(ret < 0)
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
	}else{
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
	}else{
		out_fd=fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
		if(out_fd<0) goto error;
	}

	// メモリに一度で読み込めるファイルサイズだった場合
	buff = (char*)malloc(size);
	if(buff==NULL){
		buff = (char*)malloc(32768);
		buffSize = 32768;
	}else
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
		}else{
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

////////////////////////////////////////////////////////////////////////
// ペースト
int paste(const char *path)
{
	char tmp[MAX_PATH];
	int i, ret=-1;
	
	if(!strcmp(path,clipPath)) return -1;
	
	for(i=0; i<nclipFiles; i++){
		strcpy(tmp, clipFiles[i].name);
		if(clipFiles[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
		strcat(tmp, " pasting");
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

///////////////////////////////////////////////////////////////////////////
// psuファイルからインポート
int psuImport(const char *path, const FILEINFO *file)
{
	int dialog_x;				//ダイアログx位置
	int dialog_y;				//ダイアログy位置
	int dialog_width;			//ダイアログ幅 
	int dialog_height;			//ダイアログ高さ  
	char name[MAX_PATH];		//選択されたフォルダまたはファイル名
	char fullpath[MAX_PATH];	//選択されたフォルダまたはファイルのフルパス
	char title[16*4+1];		//ゲームタイトル
	char tmp[2048];
	int x,y,scroll;

	int ret = -1;
	int in_fd = -1;
	int out_fd = -1;
	int n = 0;
	int i;
	char outdir[MAX_PATH];	//出力フォルダ名
	char out[MAX_PATH];		//出力ファイル名のフルパス
	char *buff=NULL;
	int seek;
	size_t outsize;
	int fileSize;	//ファイルのサイズ
	int psuSize;
	PSU_HEADER psu_header[MAX_ENTRY];

	//フォルダのときは、psuからインポートできない
	if(file->attr & FIO_S_IFDIR){
		ret=-1;
		return ret;
	}

	//選択されたファイルの情報
	strcpy(name, file->name);
	strcpy(title, file->title);

	//選択されたフォルダまたはファイルのフルパス
	sprintf(fullpath, "%s%s", path, name);

	//psuファイルオープン
	in_fd = fioOpen(fullpath, O_RDONLY);
	if(in_fd<0){
		ret=-2;
		goto error;
	}
	psuSize = fioLseek(in_fd, 0, SEEK_END);	//psuファイルサイズ取得
	fioLseek(in_fd, 0, SEEK_SET);	//シークを0に戻す

	//psuヘッダ読み込む
	if(psuSize<sizeof(PSU_HEADER)){
		ret=-3;
		goto error;
	}
	//psuヘッダを読み込むのにpsu_header[0]を一時的に使う
	memset(&psu_header[0], 0, sizeof(PSU_HEADER));
	fioRead(in_fd, &psu_header[0], sizeof(PSU_HEADER));
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
		fioRead(in_fd, &psu_header[i], sizeof(PSU_HEADER));
		seek += sizeof(PSU_HEADER);
		if(psu_header[i].size>0){
			fileSize = (((psu_header[i].size-1)/0x400)+1)*0x400;
			if(psuSize<seek + fileSize){
				ret=-5;
				goto error;
			}
			seek += fileSize;
			fioLseek(in_fd, seek, SEEK_SET);
		}
	}
	//psuファイルクローズ
	fioClose(in_fd);

	dialog_width = FONT_WIDTH*48;
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
			else if(new_pad & PAD_CROSS){
				ret=-6;
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
		drawFrame(dialog_x+5, dialog_y+FONT_HEIGHT*4,
			dialog_x+dialog_width-5, dialog_y+FONT_HEIGHT*14, setting->color[1]);
		// psuファイルの情報を表示
		x = dialog_x+FONT_WIDTH;
		y = dialog_y+FONT_HEIGHT*0.5;
		strcpy(tmp, fullpath);
		if(strlen(tmp)>44){
			tmp[42]='.';
			tmp[43]='.';
			tmp[44]='.';
			tmp[45]=0;
		}
		printXY(tmp, x, y, setting->color[3], TRUE);
		y +=FONT_HEIGHT*2;
		sprintf(tmp, "%2d files", n);
		printXY(tmp, x, y, setting->color[3], TRUE);
		y +=FONT_HEIGHT*2;
		printXY(" num: attr:     size: filename", x, y, setting->color[3], TRUE);
		y +=FONT_HEIGHT;
		for(i=0;i<8;i++){
			sprintf(tmp, "%4d:", i+scroll);
			if(i+scroll<n)
				sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, psu_header[i+scroll].attr, psu_header[i+scroll].size, psu_header[i+scroll].name);
				if(strlen(tmp)>44){
					tmp[42]='.';
					tmp[43]='.';
					tmp[44]='.';
					tmp[45]=0;
				}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y +=FONT_HEIGHT;
		}
		y += FONT_HEIGHT;
		printXY("○:Import ×:Cancel", x, y, setting->color[3], TRUE);
		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY("○:Import ×:Cancel", x, y, setting->color[3], TRUE);
		drawScr();
	}

	//インポート開始
	// フォルダ作成
	ret = newdir(path, outdir);
	if(ret == -17){	//フォルダがすでにあるとき上書きを確認する
		drawDark();
		itoSwitchFrameBuffers();
		drawDark();
		sprintf(tmp, "%s%s/", path, outdir);
		strcat(tmp, "\nOverwrite?");
		if(ynDialog(tmp)<0){	//キャンセル
			ret = -7;
			goto error;
		}
	}
	else if(ret < 0){//フォルダ作成失敗
		ret = -8;
		goto error;
	}
	// フォルダの中身に全コピー
	//psuファイルオープン
	in_fd = fioOpen(fullpath, O_RDONLY);
	if(in_fd<0){
		ret=-9;
		goto error;
	}

	// 描画開始
	dialog_width = FONT_WIDTH*32;
	dialog_height = FONT_HEIGHT*1.5;
	dialog_x = (SCREEN_WIDTH-dialog_width)/2;
	dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
	drawDark();
	itoSwitchFrameBuffers();
	drawDark();
	seek = sizeof(PSU_HEADER);
	for(i=0;i<n;i++){
		// プログレスバー
		drawDialogTmp(dialog_x, dialog_y,
			dialog_x+dialog_width, dialog_y+dialog_height,
			setting->color[0], setting->color[1]);
		itoSprite(setting->color[1],
			dialog_x+6, dialog_y+6,
			dialog_x+6+(dialog_width-12)*(i*100/n)/100, dialog_y+dialog_height-5, 0);
		//
		sprintf(tmp, "%2d / %2d", i, n);
		printXY(tmp, dialog_x+120, dialog_y+7, setting->color[3], TRUE);
		drawScr();
		//
		seek += sizeof(PSU_HEADER);
		if(psu_header[i].size>0){
			fioLseek(in_fd, seek, SEEK_SET);	//シーク
			//書き込むデータのメモリを確保
			buff = (char*)malloc(psu_header[i].size);
			if(buff==NULL){
				ret=-10;
				goto error;
			}
			memset(buff, 0, psu_header[i].size);
			//psuファイルから読み込む
			fioRead(in_fd, buff, psu_header[i].size);

			//出力するファイルオープン
			sprintf(out, "%s%s/%s", path, outdir, psu_header[i].name);
			out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
			if(out_fd<0){
				ret=-11;
				goto error;
			}
			outsize = fioWrite(out_fd, buff, psu_header[i].size);
			if(outsize!=psu_header[i].size){
				ret=-12;
				goto error;
			}
			fioLseek(in_fd, seek, SEEK_SET);	//シークを読み込み開始位置に戻す		
			seek += (((psu_header[i].size-1)/0x400)+1)*0x400;
			fioLseek(in_fd, seek, SEEK_SET);	//シークを読み込み開始位置に戻す		
			//出力するファイルクローズ
			fioClose(out_fd);
			free(buff);
		}
	}
	//psuファイルをクローズ
	fioClose(in_fd);
	ret=0;
error:
	free(buff);
	if(out_fd>0) fioClose(out_fd);
	if(in_fd>0) fioClose(in_fd);

	return ret;
}

///////////////////////////////////////////////////////////////////////////
// psuファイルにエクスポート
int psuExport(const char *path, const FILEINFO *file)
{
	int dialog_x;				//ダイアログx位置
	int dialog_y;				//ダイアログy位置
	int dialog_width;			//ダイアログ幅
	int dialog_height;			//ダイアログ高さ
	char name[MAX_PATH];		//選択されたフォルダまたはファイル名
	char fullpath[MAX_PATH];	//選択されたフォルダまたはファイルのフルパス
	char title[16*4+1];		//ゲームタイトル
	char tmp[2048];				//表示用
	char Pattern[MAX_PATH];		//列挙用パターン
	char tmppath[MAX_PATH];
	int x,y,scroll;

	mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	int mcret;
	fat_dir_record record[MAX_ENTRY];
	int massret;

	int ret = -1;
	int out_fd = -1;
	int in_fd = -1;
	int fd;
	int n = 0;
	int i;
	char out[MAX_PATH];	//出力するpsuファイル名
	char *buff=NULL;
	size_t outsize;
	int writeSize;

	PSU_HEADER psu_header;

	//選択されたファイルの情報
	strcpy(name, file->name);
	strcpy(title, file->title);
	if(strlen(title)>46){	//titleが長いときに短くする
		title[42] = '.';
		title[43] = '.';
		title[44] = '.';
		title[45] = 0;
	}

	//ファイルのときは、psuにエクスポートできない
	if(file->attr & FIO_S_IFREG){	//ファイル
		ret = -1;
		return ret;
	}

	//選択されたフォルダまたはファイルのフルパス
	sprintf(fullpath, "%s%s", path, name);

	//リスト読み込み
	if(!strncmp(path, "mc", 2)){
		sprintf(Pattern, "%s/*", &fullpath[4]);
		mcSync(0, NULL, &mcret);
		mcGetDir(fullpath[2]-'0', 0, Pattern, 0, MAX_ENTRY-2, mcDir);
		mcSync(0, NULL, &n);	//ファイル数
	}
	else if(!strncmp(path, "mass", 4)){
		sprintf(Pattern,"%s/", &fullpath[5]);
		massret = usb_mass_getFirstDirentry(Pattern, &record[0]);
		//fat_dir_record構造体のファイルサイズが取得できないので調べる
		record[0].size = 0;
		while(massret>0){
			n++;	//ファイル数
			massret = usb_mass_getNextDirentry(&record[n]);
			if(record[n].attr & 0x10){
				record[n].size = 0;
			}
			else{
				sprintf(tmppath,"%s/%s", fullpath, record[n].name);
				fd = fioOpen(tmppath, O_RDONLY);
				record[n].size = fioLseek(fd,0,SEEK_END);
				fioClose(fd);
			}
		}
	}
	else{
		ret=-2;
		return ret;
	}

	dialog_width = FONT_WIDTH*48;
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
			else if(new_pad & PAD_CROSS){
				ret=-1;
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
		drawFrame(dialog_x+5, dialog_y+FONT_HEIGHT*4,
			dialog_x+dialog_width-5, dialog_y+FONT_HEIGHT*14, setting->color[1]);
		//
		x = dialog_x+FONT_WIDTH;
		y = dialog_y+FONT_HEIGHT*0.5;
		sprintf(tmp, "%s/", fullpath);
		if(strlen(tmp)>44){
			tmp[42]='.';
			tmp[43]='.';
			tmp[44]='.';
			tmp[45]=0;
		}
		printXY(tmp, x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		printXY(title, x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		sprintf(tmp, "%d files", n);
		printXY(tmp, x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT*2;
		printXY(" num: attr:     size: filename", x, y, setting->color[3], TRUE);
		y += FONT_HEIGHT;
		for(i=0;i<8;i++){
			sprintf(tmp, "%4d:", i+scroll);
			if(!strncmp(path, "mc", 2)){
				if(i+scroll<n)
					sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, mcDir[i+scroll].attrFile, mcDir[i+scroll].fileSizeByte, mcDir[i+scroll].name);
					if(strlen(tmp)>44){
						tmp[42]='.';
						tmp[43]='.';
						tmp[44]='.';
						tmp[45]=0;
					}
			}
			else if(!strncmp(path, "mass", 4)){
				if(i+scroll<n)
				sprintf(tmp, "%4d: %4X: %8d: %s", i+scroll, record[i+scroll].attr, record[i+scroll].size, record[i+scroll].name);
				if(strlen(tmp)>44){
					tmp[42]='.';
					tmp[43]='.';
					tmp[44]='.';
					tmp[45]=0;
				}
			}
			printXY(tmp, x, y, setting->color[3], TRUE);
			y += FONT_HEIGHT;
		}
		y += FONT_HEIGHT;
		printXY("○:Export ×:Cancel", x, y, setting->color[3], TRUE);
		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY("○:Export ×:Cancel", x, y, setting->color[3], TRUE);

		drawScr();
	}

	//出力するpsuファイル名
	sprintf(out, "%s%s.psu", path, name);

	//オープン
	out_fd = fioOpen(out, O_WRONLY | O_TRUNC | O_CREAT);
	if(out_fd<0){
		ret=-2;
		goto error;
	}
	//psuヘッダ書き込み
	memset(&psu_header, 0, sizeof(PSU_HEADER));
	psu_header.attr = 0x8427;
	psu_header.size = n;
	strcpy(psu_header.name, name);
	outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
	if(outsize!=sizeof(PSU_HEADER)){
		ret=-3;
		goto error;
	}

	//ファイルヘッダとファイル書き込み
	dialog_width = FONT_WIDTH*32;
	dialog_height = FONT_HEIGHT*1.5;
	dialog_x = (SCREEN_WIDTH-dialog_width)/2;
	dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
	drawDark();
	itoSwitchFrameBuffers();
	drawDark();
	for(i=0;i<n;i++){
		// 描画開始
		drawDialogTmp(dialog_x, dialog_y,
			dialog_x+dialog_width, dialog_y+dialog_height,
			setting->color[0], setting->color[1]);
		// プログレスバー
		itoSprite(setting->color[1],
			dialog_x+6, dialog_y+6,
			dialog_x+6+(dialog_width-12)*(i*100/n)/100, dialog_y+dialog_height-5, 0);
		sprintf(tmp, "%2d / %2d", i, n);
		printXY(tmp, dialog_x+120, dialog_y+7, setting->color[3], TRUE);
		drawScr();
		//ファイルヘッダを作成
		memset(&psu_header, 0, sizeof(PSU_HEADER));
		if(!strncmp(path, "mc", 2)){
			psu_header.attr = mcDir[i].attrFile;	//ファイル属性はメモリーカードと同じにする
			psu_header.size = mcDir[i].fileSizeByte;
			strcpy(psu_header.name, mcDir[i].name);
		}
		else if(!strncmp(path, "mass", 4)){
			//usbメモリではファイル属性が正確に取れない
			if(record[i].attr & 0x10)
				psu_header.attr = 0x8427;	//フォルダ
			else
				psu_header.attr = 0x8417;	//フォルダ以外
			psu_header.size = record[i].size;
			strcpy(psu_header.name, record[i].name);
		}
		//ファイルヘッダ書き込み
		outsize = fioWrite(out_fd, &psu_header, sizeof(PSU_HEADER));
		if(outsize!=sizeof(PSU_HEADER)){
			ret=-4;
			goto error;
		}
		//ファイル書き込み
		if(!strncmp(path, "mc", 2)){
			if(mcDir[i].fileSizeByte>0){
				sprintf(tmppath, "%s/%s", fullpath, mcDir[i].name);
				writeSize = (((mcDir[i].fileSizeByte-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				if(buff==NULL){
					ret=-5;
					goto error;
				}
				memset(buff, 0, writeSize);
				//読み込むファイルオープン
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-6;
					goto error;
				}
				//読み込む
				fioRead(in_fd, buff, mcDir[i].fileSizeByte);
				//psuファイルに書き込み
				outsize = fioWrite(out_fd, buff, writeSize);
				if(outsize!=writeSize){
					ret=-7;
					goto error;
				}
				//読み込むファイルクローズ
				fioClose(in_fd);
				free(buff);
			}
		}
		else if(!strncmp(path, "mass", 4)){
			if(record[i].size>0){
				sprintf(tmppath, "%s/%s", fullpath, record[i].name);
				writeSize = (((record[i].size-1)/0x400)+1)*0x400;
				buff = (char*)malloc(writeSize);
				memset(buff, 0, writeSize);
				if(buff==NULL){
					ret=-5;
					goto error;
				}
				//読み込むファイルオープン
				in_fd = fioOpen(tmppath, O_RDONLY);
				if(in_fd<0){
					ret=-6;
					goto error;
				}
				//読み込む
				fioRead(in_fd, buff, record[i].size);
				//psuファイルに書き込み
				outsize = fioWrite(out_fd, buff, writeSize);
				if(outsize!=writeSize){
					ret=-7;
					goto error;
				}
				//読み込むファイルクローズ
				fioClose(in_fd);
				free(buff);
			}
		}
	}
	ret=0;
error:
	free(buff);
	if(out_fd>0) fioClose(out_fd);
	if(in_fd>0) fioClose(in_fd);

	if(ret<0){
		// エクスポート失敗したときpsuファイルを削除
		if(!strncmp(out, "mc", 2)){
			mcSync(0,NULL,NULL);
			mcDelete(out[2]-'0', 0, &out[4]);
			mcSync(0, NULL, &mcret);
		}
		else if(!strncmp(out, "mass", 4)){
			massret = fioRemove(out);
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////
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
	char tmp[256];//, *p;

	WFONTS=13;
	HFONTS=7;
	KEY_W=(WFONTS*3+2)*FONT_WIDTH;
	KEY_H=(HFONTS+3)*FONT_HEIGHT;
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
				if(sel<=WFONTS*HFONTS){
					if(sel>=WFONTS) sel-=WFONTS;
				}else{
					sel-=4;
				}
			}
			else if(new_pad & PAD_DOWN){
				if(sel/WFONTS == HFONTS-1){
					if(sel%WFONTS < 5)	sel=WFONTS*HFONTS;
					else				sel=WFONTS*HFONTS+1;
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
		drawFrame(	//枠
			KEY_X+4, KEY_Y+4,
			KEY_X+KEY_W-4, KEY_Y+FONT_HEIGHT*1.5-4, setting->color[1]);
		printXY(out,	//入力中の文字列の表示
			KEY_X+4+FONT_WIDTH,
			KEY_Y+8,
			setting->color[3], TRUE);
		t++;
		if(t<SCANRATE/2){
			printXY("|",	//キャレット
				KEY_X+1+(cur+1)*FONT_WIDTH,
				KEY_Y+8,
				setting->color[3], TRUE);
		}else{
			if(t==SCANRATE) t=0;
		}
		//キーボード表示
		for(i=0; i<KEY_LEN; i++)
			drawChar(KEY[i],
				KEY_X+FONT_WIDTH*2 + (i%WFONTS)*FONT_WIDTH*3,
				KEY_Y+FONT_HEIGHT*2 + (i/WFONTS)*FONT_HEIGHT,
				setting->color[3]);
		//OKとCANCEL表示
		printXY("OK                            CANCEL",
			KEY_X+FONT_WIDTH*2,
			KEY_Y+FONT_HEIGHT*9,
			setting->color[3], TRUE);
		//カーソル表示
		if(sel<=WFONTS*HFONTS)
			x = KEY_X+FONT_WIDTH + (sel%WFONTS)*FONT_WIDTH*3;	//CANCEL以外
		else
			x = KEY_X+FONT_WIDTH*31;	//CANCEL
		y = KEY_Y+FONT_HEIGHT*2 + (sel/WFONTS)*FONT_HEIGHT;
		drawChar('>', x, y, setting->color[3]);
		
		// 操作説明
		x = FONT_WIDTH*2;
		y = SCREEN_MARGIN+FONT_HEIGHT*20;
		itoSprite(setting->color[0],
			0, y,
			SCREEN_WIDTH, y+FONT_HEIGHT, 0);
		printXY("○:OK ×:Back L1:Left R1:Right START:Enter",
			x, y, setting->color[3], TRUE);
		drawScr();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// ファイルリスト設定
int setFileList(const char *path, const char *ext, FILEINFO *files, int cnfmode)
{
	char *p;
	int nfiles, i, j, ret=0;
	char fullpath[MAX_PATH];

	int checkELFret;
	FILEINFO file;
	char party[MAX_NAME], dir[MAX_PATH];

	// ファイルリスト設定
	if(path[0]==0){
		strcpy(files[0].name, "mc0:");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		strcpy(files[1].name, "mc1:");
		files[1].attr = FIO_S_IFDIR;
		files[1].type=TYPE_OTHER;
		strcpy(files[2].name, "hdd0:");
		files[2].attr = FIO_S_IFDIR;
		files[2].type=TYPE_OTHER;
		strcpy(files[3].name, "cdfs:");
		files[3].attr = FIO_S_IFDIR;
		files[3].type=TYPE_OTHER;
		strcpy(files[4].name, "mass:");
		files[4].attr = FIO_S_IFDIR;
		files[4].type=TYPE_OTHER;
		nfiles = 5;
		for(i=0; i<nfiles; i++)
			files[i].title[0]=0;
		if(cnfmode){
			strcpy(files[nfiles].name, "MISC");
			files[nfiles].attr = FIO_S_IFDIR;
			files[nfiles].type=TYPE_OTHER;
			nfiles++;
		}
		vfreeSpace=FALSE;
	}
	else if(!strcmp(path, "MISC/")){
		strcpy(files[0].name, "..");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		strcpy(files[1].name, "FileBrowser");
		files[1].attr = FIO_S_IFREG;
		files[1].type=TYPE_OTHER;
		strcpy(files[2].name, "PS2Browser");
		files[2].attr = FIO_S_IFREG;
		files[2].type=TYPE_OTHER;
		strcpy(files[3].name, "PS2Disc");
		files[3].attr = FIO_S_IFREG;
		files[3].type=TYPE_OTHER;
		strcpy(files[4].name, "PS2Net");	//PS2Net uLaunchELF3.60
		files[4].attr = FIO_S_IFREG;
		files[4].type=TYPE_OTHER;
		nfiles = 5;
		for(i=0; i<nfiles; i++)
			files[i].title[0]=0;
	}
	else{
		strcpy(files[0].name, "..");
		files[0].attr = FIO_S_IFDIR;
		files[0].type=TYPE_OTHER;
		nfiles = getDir(path, &files[1]) + 1;
		if(strcmp(ext,"*")){	//ファイルマスク
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
		//ゲームタイトルとファイルタイプ取得
		for(i=1; i<nfiles; i++){
			//ゲームタイトル取得
			if( !strncmp(path, "cdfs", 4)){
				//cdfs
				if(setting->discPs2saveCheck){
					ret = getGameTitle(path, &files[i], files[i].title);
					if(ret<0) files[i].title[0]=0;
				}
				else{
					ret=-1;
					files[i].title[0]=0;
				}
			}
			else{
				//mcとhddとmass
				ret = getGameTitle(path, &files[i], files[i].title);
				if(ret<0) files[i].title[0]=0;
			}

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
				else if( !strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/") ){
					checkELFret = checkELFheader(fullpath); 	//checkELFheader
					mountedParty[0][0]=0;
					if(checkELFret<0)
						files[i].type=TYPE_FILE;
					else
						files[i].type=TYPE_ELF;
					//HDDのとき再マウント
					strcpy(file.name,files[i].name);
					strcpy(file.title,files[i].title);
					file.attr=files[i].attr;
					file.type=files[i].type;
					//
					getHddParty(path,&file,party,dir);
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
		if(!strcmp(path, "hdd0:/"))
			vfreeSpace=FALSE;
		else if(nfiles>1)
			sort(&files[1], 0, nfiles-2);
	}
	
	return nfiles;
}

////////////////////////////////////////////////////////////////////////
// 任意のファイルパスを返す
void getFilePath(char *out, int cnfmode)
{
	char path[MAX_PATH], oldFolder[MAX_PATH],
		msg0[MAX_PATH], msg1[MAX_PATH],
		tmp[MAX_PATH], ext[8], *p;
	uint64 color,iconcolor=0;
	FILEINFO files[MAX_ENTRY];
	int nfiles=0, sel=0, top=0, rows=MAX_ROWS;
	int cd=TRUE, up=FALSE, pushed=TRUE;
	int nofnt=FALSE;
	int x, y, y0, y1;
	int i, ret;//, fd;
	size_t size;

	if(cnfmode) strcpy(ext, "elf");
	else		strcpy(ext, "*");
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
				sel-=rows/2;
			else if(new_pad & PAD_RIGHT)
				sel+=rows/2;
			else if(new_pad & PAD_TRIANGLE)
				up=TRUE;
			else if(new_pad & PAD_CIRCLE){
				if(files[sel].attr & FIO_S_IFDIR){
					if(!strcmp(files[sel].name,".."))
						up=TRUE;
					else{
						strcat(path, files[sel].name);
						strcat(path, "/");
						cd=TRUE;
					}
				}
				else{
					sprintf(out, "%s%s", path, files[sel].name);
					ret=checkELFheader(out);
					mountedParty[0][0]=0;
					if(ret<0){
						pushed=FALSE;
						sprintf(msg0, "This file isn't ELF.");
						out[0] = 0;
					}
					else{
						strcpy(LastDir, path);
						break;
					}
				}
			}
			if(cnfmode){	//コンフィグモード
				if(new_pad & PAD_SQUARE) {	// ファイルマスク切り替え
					if(!strcmp(ext,"*")) strcpy(ext, "elf");
					else				 strcpy(ext, "*");
					cd=TRUE;
				}
				else if(new_pad & PAD_CROSS){	// メインメニューに戻る
					if(mountedParty[0][0]!=0) fileXioUmount("pfs0:");
					return;
				}
			}
			else{	//ファイラーモード
				// メニュー
				if(new_pad & PAD_R1){
					drawDark();
					itoSwitchFrameBuffers();
					drawDark();
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
						sprintf(msg0, "Copied to the Clipboard");
						pushed=FALSE;
						if(ret==CUT)	cut=TRUE;
						else			cut=FALSE;
					}
					else if(ret==DELETE){	// デリート
						drawDark();
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
							strcat(tmp, "\nDelete?");
							ret = ynDialog(tmp);
						}
						else
							ret = ynDialog("Mark Files Delete?");
						
						if(ret>0){
							if(nmarks==0){
								strcpy(tmp, files[sel].name);
								if(files[sel].attr & FIO_S_IFDIR) strcat(tmp,"/");
								strcat(tmp, " deleting");
								drawMsg(tmp);
								ret=delete(path, &files[sel]);
							}
							else{
								for(i=0; i<nfiles; i++){
									if(marks[i]){
										strcpy(tmp, files[i].name);
										if(files[i].attr & FIO_S_IFDIR) strcat(tmp,"/");
										strcat(tmp, " deleting");
										drawMsg(tmp);
										ret=delete(path, &files[i]);
										if(ret<0) break;
									}
								}
							}
							if(ret>=0)
								cd=TRUE;
							else{
								strcpy(msg0, "Delete Failed");
								pushed = FALSE;
							}
						}
					}
					else if(ret==RENAME){	// リネーム
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						strcpy(tmp, files[sel].name);
						if(keyboard(tmp, 36)>=0){
							if(Rename(path, &files[sel], tmp)<0){
								pushed=FALSE;
								strcpy(msg0, "Rename Failed");
							}
							else
								cd=TRUE;
						}
					}
					else if(ret==PASTE){	// クリップボードからペースト
						drawMsg("Pasting...");
						ret=paste(path);
						if(ret < 0){
							strcpy(msg0, "Paste Failed");
							pushed = FALSE;
						}else
							if(cut) nclipFiles=0;
						cd=TRUE;
					}
					else if(ret==NEWDIR){	// 新規フォルダ作成
						tmp[0]=0;
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						if(keyboard(tmp, 36)>=0){
							ret = newdir(path, tmp);
							if(ret == -17){
								strcpy(msg0, "directory already exists");
								pushed=FALSE;
							}
							else if(ret < 0){
								strcpy(msg0, "NewDir Failed");
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
						drawMsg("Checking Size...");
						if(nmarks==0){
							size=getFileSize(path, &files[sel]);
						}
						else{
							for(i=size=0; i<nfiles; i++){
								if(marks[i])
									size+=getFileSize(path, &files[i]);
								if(size<0) size=-1;
							}
						}
						if(size<0){
							strcpy(msg0, "Paste Failed");
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
						itoSwitchFrameBuffers();
						drawDark();
						ret = psuExport(path, &files[sel]);
						if(ret<0){
							sprintf(msg0, "Export psu Failed %d", ret);
							pushed = FALSE;
						}
						else{
							cd = TRUE;
						}
					}
					else if(ret==IMPORT){	// psuファイルからインポート
						drawDark();
						itoSwitchFrameBuffers();
						drawDark();
						ret = psuImport(path, &files[sel]);
						if(ret<0){
							sprintf(msg0, "Import psu Failed %d", ret);
							pushed = FALSE;
						}
						else{
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
				else if(new_pad & PAD_L1) {	// タイトル表示切り替え
					title = !title;
					nofnt = TRUE;
					cd=TRUE;
				}
				else if(new_pad & PAD_SELECT){	// メインメニューに戻る
					if(mountedParty[0][0]!=0) fileXioUmount("pfs0:");
					if(mountedParty[1][0]!=0) fileXioUmount("pfs1:");
					return;
				}
				else if(new_pad & PAD_R2){	//コンフィグ
					config(msg0);
					pushed = FALSE;
					if(setting->discControl)
						loadCdModules();
					if(setting->fileicon)
						cd=TRUE;
				}
				else if(new_pad & PAD_R3){
					//FILEICON
					setting->fileicon = !setting->fileicon;
					if(setting->fileicon)
						cd=TRUE;
				}
				else if(new_pad & PAD_L3){
					//FILEICON
					setting->flickerControl = !setting->flickerControl;
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
		// フォルダ移動
		if(cd){
			nfiles = setFileList(path, ext, files, cnfmode);
			// 空き容量取得
			if(!cnfmode){
				if(!strncmp(path, "mc", 2)){
					mcGetInfo(path[2]-'0', 0, NULL, &mcfreeSpace, NULL);
				}
				else if(!strncmp(path,"hdd",3)&&strcmp(path,"hdd0:/")){
					freeSpace = 
					fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_FREE,NULL,0,NULL,0)*fileXioDevctl("pfs0:",PFSCTL_GET_ZONE_SIZE,NULL,0,NULL,0);
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
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
		// 画面描画開始
		clrScr(setting->color[0]);
		// ファイルリスト
		x = FONT_WIDTH*3;
		y = SCREEN_MARGIN+FONT_HEIGHT*3;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles)
				break;
			if(top+i == sel){
				color = setting->color[2];
				drawChar('>', x, y, color);	//カーソル表示
			}
			else
				color = setting->color[3];

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
			if(strlen(tmp)>52){	//ファイル名が長いときは、短くする
				tmp[50]='.';
				tmp[51]='.';
				tmp[52]='.';
				tmp[53]=0;
			}

			//マーク表示
			if(marks[top+i]){
				//drawChar('*', x+FONT_WIDTH, y, setting->color[3]);
				//アルファブレンド有効
				itoPrimAlphaBlending( TRUE );
				itoSprite(setting->color[2]|0x10000000,
					x+FONT_WIDTH, y,
					x+FONT_WIDTH*57, y+FONT_HEIGHT-2, 0);
				//アルファブレンド無効
				itoPrimAlphaBlending(FALSE);
			}

			//
			if(!setting->fileicon)
				printXY(tmp, x+FONT_WIDTH*2, y, color, TRUE);	//ファイル名表示
			else{
				if(files[top+i].type!=TYPE_OTHER){
					if(files[top+i].type==TYPE_DIR) iconcolor=setting->color[4];
					else if(files[top+i].type==TYPE_FILE) iconcolor=setting->color[5];
					else if(files[top+i].type==TYPE_PS2SAVE) iconcolor=setting->color[6];
					else if(files[top+i].type==TYPE_ELF) iconcolor=setting->color[7];
					printXY("■", x+FONT_WIDTH*2, y, iconcolor, TRUE);	//アイコン
				}
				printXY(tmp, x+FONT_WIDTH*4, y, color, TRUE);	//ファイル名表示
			}
			y += FONT_HEIGHT;
		}
		// スクロールバー
		if(nfiles > rows){
			drawFrame(FONT_WIDTH*61, SCREEN_MARGIN+FONT_HEIGHT*3,
				FONT_WIDTH*62, SCREEN_MARGIN+FONT_HEIGHT*19,setting->color[1]);
			y0=FONT_HEIGHT*16*((double)top/nfiles);
			y1=FONT_HEIGHT*16*((double)(top+rows)/nfiles);
			itoSprite(setting->color[1],
				FONT_WIDTH*61,
				SCREEN_MARGIN+FONT_HEIGHT*3+y0,
				FONT_WIDTH*62,
				SCREEN_MARGIN+FONT_HEIGHT*3+y1,
				0);
		}
		// メッセージ
		if(pushed) sprintf(msg0, "Path: %s", path);
		// 操作説明
		if(cnfmode){
			if(!strcmp(ext, "*"))
				sprintf(msg1, "○:OK ×:Cancel △:Up □:*->ELF");
			else
				sprintf(msg1, "○:OK ×:Cancel △:Up □:ELF->*");
		}
		else{
			if(title)
				sprintf(msg1, "○:OK △:Up ×:Mark □:RevMark L1:TitleOFF R1:Menu R2:Config");
			else
				sprintf(msg1, "○:OK △:Up ×:Mark □:RevMark L1:TitleON  R1:Menu R2:Config");
		}
		setScrTmp(msg0, msg1);

		// フリースペース表示
		if(!strncmp(path, "mc", 2) && !vfreeSpace && !cnfmode){
			if(mcSync(1,NULL,NULL)!=0){
				freeSpace = mcfreeSpace*1024;
				vfreeSpace=TRUE;
			}
		}
		if(vfreeSpace){
			if(freeSpace >= 1024*1024)
				sprintf(tmp, "[%.1fMB free]", (double)freeSpace/1024/1024);
			else if(freeSpace >= 1024)
				sprintf(tmp, "[%.1fKB free]", (double)freeSpace/1024);
			else
				sprintf(tmp, "[%dB free]", freeSpace);
			ret=strlen(tmp);
			itoSprite(setting->color[0],
				FONT_WIDTH*62-FONT_WIDTH*ret,
				SCREEN_MARGIN+FONT_HEIGHT,
				FONT_WIDTH*62,
				SCREEN_MARGIN+FONT_HEIGHT*2, 0);
			printXY(tmp,
				FONT_WIDTH*62-FONT_WIDTH*ret,
				SCREEN_MARGIN+FONT_HEIGHT,
				setting->color[3], TRUE);
		}
		drawScr();
	}
	
	if(mountedParty[0][0]!=0) fileXioUmount("pfs0:");
	if(mountedParty[1][0]!=0) fileXioUmount("pfs1:");
	return;
}
