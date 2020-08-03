#include "launchelf.h"

enum{fiofiles=32,fbufsize=8192};
int fioinited=0,fiolist[fiofiles];
//int fileMode = FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;
extern int fileMode;
extern char mountedParty[2][MAX_NAME];
extern int loaded_filexio;
extern int nclipFiles, nmarks, nparties;
extern char mountedParty[2][MAX_NAME];
extern char parties[MAX_PARTITIONS][MAX_NAME];
extern char clipPath[MAX_PATH], LastDir[MAX_NAME], marks[MAX_ENTRY];
extern FILEINFO clipFiles[MAX_ENTRY];
int getHddParty(const char *path, const FILEINFO *file, char *party, char *dir);
int mountParty(const char *party);
void itoNoVSync(void);
static void ninit(void) {
	if (fioinited) return;
	int i;
	for(i=1;i<fiofiles;i++)
		fiolist[i] = -1;
	fiolist[0] = 255;
	fioinited=1;
}
static int nempty(void) {
	int i;
	for(i=0;i<fiofiles;i++)
		if (fiolist[i] < 0)
			return i;
	return -1;
}
static int nhddmount(const char *party)
{	
	int n; char p[32];
	//マウントしていたら番号を返す
	if(!strcmp(party, mountedParty[0]))
		return 0;
	else if(!strcmp(party, mountedParty[1]))
		return 1;

	//マウントしていないとき空いているところにマウント
	for(n=0;n<2;n++){
		if (!mountedParty[n][0]) {
			sprintf(p, "pfs%d:", n);
			if(fileXioMount(p, party, FIO_MT_RDWR) >= 0) {
				strcpy(mountedParty[0], party);
				return n;
			}
		}
	}
	return -1;
}

static int nparty(char *path, char *dparty, char *fullpath) {
	char party[MAX_NAME], *p; int n=-1;
	if (!fullpath) return -1;
	if (!strncmp(path, "hdd0", 4)) {
		loadHddModules();
		// 	"hdd0:/+MP3/folder/file.mp3" 
		//=>"pfs0:/folder/file.mp3"
		p = strchr(&path[6], '/');
		if (p) {
			strcpy(party, "hdd0:");
			memcpy(&party[5], &path[6], p-&path[6]);
			party[5+(p-&path[6])] = 0;
			if (strcmp(party, mountedParty[0]) == 0) n = 0;
			else if (strcmp(party, mountedParty[1]) == 0) n = 1;
			else n = -2;
			if (n < 0) {
				n = mountParty(party);
			}
			sprintf(fullpath, "pfs%d:%s", n, p);
			printf("hddparty: %s\n", party);
			printf("hddpath: %s\n", fullpath);
			if (dparty) strcpy(dparty, party);
		}
	}
	return n;
}
int nopen(char *path, int attr) {
//	printf("nopen %s attr %x\n", path, attr);
	int fd,ret,n,t=0;
	char fullpath[MAX_PATH];
	if (!fioinited) ninit();
	strcpy(fullpath, path);
	if (!strncmp(path, "mass", 4)) loadUsbMassModules();
	if (!strncmp(path, "hdd0", 4)) {
		n = nparty(path, 0, fullpath);
		if (n >= 0 && (attr & O_WRONLY) && (attr & O_TRUNC) && (attr & O_CREAT)) t = 1;
		fd = -1;
	} else
		fd = fioOpen(fullpath, attr);
	n = 0;
	if (fd < 0) {
		if (loaded_filexio) {
			// O_TRUNC が利かないため、オープン前にファイル削除
			if (t) fileXioRemove(fullpath);
			fd = fileXioOpen(fullpath, attr, fileMode);
			if (fd >= 0) n = 1;
			//	printf("xopen fd = %d\n", fd);
		}
	}
	if (fd >= 0) {
		ret = nempty();
		if (ret < 0) {
			if (n)	fileXioClose(fd);
			else	fioClose(fd);
		} else
			fiolist[ret] = fd * 2 + n;
	} else ret = fd;
//	printf("nopen ret = %d, val = %d (fd = %d, xio = %d)\n", ret, fiolist[ret], fd, n);
	return ret;
}
int nclose(int fd) {
//	printf("nclose[%d] fd = %d\n", fiolist[fd], fd);
	int ret;
	if (fiolist[fd]&1)	ret = fileXioClose(fiolist[fd]>>1);
	else				ret = fioClose(fiolist[fd]>>1);
	fiolist[fd] = -1;
	return ret;
}
int nquit(char *path) {
	if (loaded_filexio) return fileXioUmount(path);
	return 0;
}
int nremove(char *path) {
	return -1;
}
int nmkdir(char *path, char *dir, int attr) {
	return 0;
}
int nrmdir(char *path) {
	return 0;
}
int nseek(int fd, signed long ofs, int mode) {
	if (fiolist[fd] & 1) return fileXioLseek(fiolist[fd]>>1, ofs, mode);
	return fioLseek(fiolist[fd]>>1, ofs, mode);
}
int ndopen(char *path) {
	int ret=-1,n,fd;
	char fullpath[MAX_PATH];
	strcpy(fullpath, path);
	if (!strncmp(path, "hdd", 3)) {
		loadHddModules();
		if (strcmp(path, "hdd0:")) n = nparty(path, 0, fullpath);
		fd = -1;
	} else if (!strncmp(path, "cd", 2)) {
		return -1;
	} else if (!strncmp(path, "vmc", 3)) {
		fd = -1;
	} else {
		fd = fioDopen(path);
	}
	n = 0;
	if (fd < 0) {
		if (loaded_filexio) {
			fd = fileXioDopen(fullpath);
			if (fd >= 0) n = 1;
			//	printf("xopen fd = %d\n", fd);
		}
	}
	if (fd >= 0) {
		ret = nempty();
		if (ret < 0) {
			if (n)	fileXioDclose(fd);
			else	fioDclose(fd);
		} else
			fiolist[ret] = fd * 2 + n;
	} else ret = fd;
	return ret;
}
int ndclose(int fd) {
	int ret;
	if (fiolist[fd]&1)	ret = fileXioDclose(fiolist[fd]>>1);
	else				ret = fioDclose(fiolist[fd]>>1);
	fiolist[fd] = -1;
	return ret;
}
int ndread(int fd, void *dst) {
	if (fiolist[fd] & 1) return fileXioDread(fiolist[fd]>>1, dst);
	return fioDread(fiolist[fd]>>1, dst);
}
static unsigned int nread0(int fd, void *dst, unsigned int size) {
	if (fiolist[fd] & 1) return fileXioRead(fiolist[fd]>>1, dst, size);
	return fioRead(fiolist[fd]>>1, dst, size);
}
static unsigned int nwrite0(int fd, void *src, unsigned int size) {
	if (fiolist[fd] & 1) return fileXioWrite(fiolist[fd]>>1, src, size);
	return fioWrite(fiolist[fd]>>1, src, size);
}
unsigned int nwrite(int fd, void *src, unsigned int size) {
	//	printf("nwrite: fd=%2d, src=%08X, size=%7d\n", fd, (int)src, size);
	if (size < fbufsize * 2) {
		itoNoVSync();
		return nwrite0(fd, src, size);
	}
	int ofs,siz=0,p=size&-fbufsize; uint64 o=totalcount;
	//	printf("nwrite: ofs=%7d/%7d, wb=%7d, p=%7d, fs=%d\n", ofs, size, siz, p, fbufsize);
	for(ofs=0;ofs<p;ofs+=fbufsize){
		siz += nwrite0(fd, (void*)&((char*)src)[ofs], fbufsize);
		if (o != totalcount) {
			itoNoVSync();
			o = totalcount;
		}
	}
	p = size & (fbufsize-1);
	if (p) siz += nwrite0(fd, (void*)&((char*)src)[ofs], p);
	itoNoVSync();
	return siz;
}
unsigned int nread(int fd, void *dst, unsigned int size) {
	if (size < fbufsize * 2) return nread0(fd, dst, size);
	int ofs,siz,p; uint64 o;
	for(ofs=0,siz=0,p=size&-fbufsize,o=totalcount;ofs<p;ofs+=fbufsize){
		siz += nread0(fd, (void*)&((char*)dst)[ofs], fbufsize);
		if (o != totalcount) {
			itoNoVSync();
			o = totalcount;
		}
	}
	p = size & (fbufsize-1);
	if (p) siz += nread0(fd, (void*)&((char*)dst)[ofs], p);
	itoNoVSync();
	return siz;
}
unsigned int ps2timetostamp(PS2TIME *src)
{
	// YYYY YYYm mmmd dddd HHHH Hiii iiiS SSSS
	//   28   24   20   16   12    8    4    0
	int Y;
	unsigned int tmp;
	Y = src[0].year;
	if (Y >= 2236) Y -= 256;
	if (Y >= 1980) Y -= 1980;
	if (Y < 0) Y = 0;
	if (Y > 127) Y = 127;
	tmp = ((unsigned int) Y << 25)|((int) src[0].month << 21)|((int) src[0].day << 16)|((int) src[0].hour << 11)|((int) src[0].min << 5)|((int) src[0].sec >> 1);
	//printf("filer: timestamp: %08X\n", tmp);
	return tmp;
}

//-------------------------------------------------
// メモリーカード読み込み
int readMC(const char *path, FILEINFO *info, int max)
{
	static mcTable mcDir[MAX_ENTRY] __attribute__((aligned(64)));
	char dir[MAX_PATH];
	int i, j, ret;

	mcSync(MC_WAIT, NULL, NULL);

	strcpy(dir, &path[4]); strcat(dir, "*");
	mcGetDir(path[2]-'0', 0, dir, 0, MAX_ENTRY-2, mcDir);
	mcSync(MC_WAIT, NULL, &ret);

	for(i=j=0; i<ret; i++)
	{
		//printf("%s: create=%08X, modify=%08X\n", mcDir[i].name, ps2timetostamp((PS2TIME*)&mcDir[i]._create), ps2timetostamp((PS2TIME*)&mcDir[i]._modify));
		if(!strcmp(mcDir[i].name, "..")) {
		//	volumeserialnumber = ps2timetostamp((PS2TIME*)&mcDir[i]._create);
		}
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR && (!strcmp(mcDir[i].name, ".") || !strcmp(mcDir[i].name, "..")))
			continue;

		strcpy(info[j].name, mcDir[i].name);
		info[j].attr = mcDir[i].attrFile;
		info[j].fileSizeByte = mcDir[i].fileSizeByte;
		memcpy(&info[j].createtime, &mcDir[i]._create, 8);
		memcpy(&info[j].modifytime, &mcDir[i]._modify, 8);
		info[j].timestamp = ps2timetostamp(&info[j].modifytime);
		info[j].num = j;
		j++;
	}

	return j;
}

//-------------------------------------------------
// CD読み込み
int readCD(const char *path, FILEINFO *info, int max)
{
	//static struct TocEntry TocEntryList[MAX_ENTRY];
	struct TocEntry *TocEntryList;
	char dir[1025];
	int i, j, n;

	TocEntryList = (struct TocEntry*)malloc(sizeof(struct TocEntry)*MAX_ENTRY);
	if (!TocEntryList) return -2;
	//printf("readCD: static memory: %d\n", sizeof(TocEntryList));	// => 294912 bytes
	loadCdModules();

	strcpy(dir, &path[5]);
	CDVD_FlushCache();
	n = CDVD_GetDir(dir, NULL, CDVD_GET_FILES_AND_DIRS, TocEntryList, MAX_ENTRY, dir);

	for(i=j=0; i<n; i++)
	{
		
		if(TocEntryList[i].fileProperties & 0x02 && (!strcmp(TocEntryList[i].filename, ".") || !strcmp(TocEntryList[i].filename, "..")))
			continue;

		if(TocEntryList[i].fileProperties & 0x02){
			info[j].attr = MC_ATTR_SUBDIR;
		}
		else{
			info[j].attr = 0;
			info[j].fileSizeByte = TocEntryList[i].fileSize;
		}
		strcpy(info[j].name, TocEntryList[i].filename);
		memset(&info[j].createtime, 0, sizeof(PS2TIME)); //取得しない
		memset(&info[j].modifytime, 0, sizeof(PS2TIME)); //取得できない
		//info[j].timestamp = ps2timetostamp(&info[j].modifytime);
		//info[j].num = j;
		j++;
	}
	free(TocEntryList);

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
		itoNoVSync();
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
			info[i].attr = MC_ATTR_SUBDIR;
			info[i].fileSizeByte = 0;
			info[i].timestamp = 0;
			info[i].num = i;
			memset(&info[i].createtime, 0, sizeof(PS2TIME));
			memset(&info[i].modifytime, 0, sizeof(PS2TIME));
		}
		return nparties;
	}

	getHddParty(path, NULL, party, dir);
	ret = mountParty(party);
	if(ret<0) return 0;
	dir[3] = ret+'0';

	if((fd=fileXioDopen(dir)) < 0) return 0;

	while(fileXioDread(fd, &dirbuf)){
		itoNoVSync();
		if(dirbuf.stat.mode & FIO_S_IFDIR && (!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;

		if(dirbuf.stat.mode & FIO_S_IFDIR){
			info[i].attr = MC_ATTR_SUBDIR;
		}
		else{
			info[i].attr = 0;
			info[i].fileSizeByte = dirbuf.stat.size;
		}
		strcpy(info[i].name, dirbuf.name);
		memset(&info[i].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[i].modifytime.unknown = dirbuf.stat.mtime[0];
		info[i].modifytime.sec = dirbuf.stat.mtime[1];
		info[i].modifytime.min = dirbuf.stat.mtime[2];
		info[i].modifytime.hour = dirbuf.stat.mtime[3];
		info[i].modifytime.day = dirbuf.stat.mtime[4];
		info[i].modifytime.month = dirbuf.stat.mtime[5];
		info[i].modifytime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
		info[i].timestamp = ps2timetostamp(&info[i].modifytime);
		info[i].num = i;
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

	loadUsbMassModules();
	

	if ((dd = fioDopen(path)) < 0) goto exit;

	while(fioDread(dd, &record) > 0){
		itoNoVSync();
		if((FIO_SO_ISDIR(record.stat.mode)) && (!strcmp(record.name,".") || !strcmp(record.name,"..")))
			continue;

		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = MC_ATTR_SUBDIR;
		}
		else if(FIO_SO_ISREG(record.stat.mode)){
			info[n].attr = 0;
			info[n].fileSizeByte = record.stat.size;
		}
		else
			continue;

		strcpy(info[n].name, record.name);
		strncpy(info[n].name, info[n].name, 32);
		memset(&info[n].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[n].modifytime.unknown = 0;
		info[n].modifytime.sec = record.stat.mtime[1];
		info[n].modifytime.min = record.stat.mtime[2];
		info[n].modifytime.hour = record.stat.mtime[3];
		info[n].modifytime.day = record.stat.mtime[4];
		info[n].modifytime.month = record.stat.mtime[5];
		info[n].modifytime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
		//if (info[n].modifytime.year < 1000) info[n].modifytime.year+=1800;
		info[n].timestamp = ps2timetostamp(&info[n].modifytime);
		info[n].num = n;
		n++;
		if(n==max) break;
	}

exit:
	if(dd >= 0) fioDclose(dd);
	return n;
}

//-------------------------------------------------
// HOST読み込み
int readHOST(const char *path0, FILEINFO *info, int max)
{
	fio_dirent_t record;
	int n=0, dd=-1;
	char path[MAX_PATH];
	
	if (!strcmp(path0, "host:/"))
		sprintf(path, "host:%s", path0+6);
	else
		strcpy(path, path0);
	//strcpy(path, path0);
	if (path[strlen(path)-1]=='/')
		path[strlen(path)-1]=0;
	
	if ((dd = fioDopen(path)) < 0) goto exit;

	while(fioDread(dd, &record) > 0){
		//if (pcmnplay) 
		itoNoVSync();
		if((FIO_SO_ISDIR(record.stat.mode)) && (!strcmp(record.name,".") || !strcmp(record.name,"..")))
			continue;

		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = MC_ATTR_SUBDIR;
		}
		else {
			info[n].attr = 0;
			info[n].fileSizeByte = record.stat.size;
		}
		//else
		//	continue;

		strcpy(info[n].name, record.name);
		strncpy(info[n].name, info[n].name, 32);
		memset(&info[n].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[n].modifytime.unknown = 0;
		info[n].modifytime.sec = record.stat.mtime[1];
		info[n].modifytime.min = record.stat.mtime[2];
		info[n].modifytime.hour = record.stat.mtime[3];
		info[n].modifytime.day = record.stat.mtime[4];
		info[n].modifytime.month = record.stat.mtime[5];
		info[n].modifytime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
		if (info[n].modifytime.year < 1000) info[n].modifytime.year+=1900;
		if (info[n].modifytime.year > 2236) info[n].modifytime.year-=256;
		info[n].timestamp = ps2timetostamp(&info[n].modifytime);
		info[n].num = n;
		n++;
		if(n==max) break;
	}

exit:
	if(dd >= 0) fioDclose(dd);
	return n;
}

//-------------------------------------------------
// その他のデバイスの読み込み1
int readOthers(const char *path0, FILEINFO *info, int max)
{
	fio_dirent_t record;
	int n=0, dd=-1;
	char path[MAX_PATH];
	
//	if (!strcmp(path0, "host:/"))
//		sprintf(path, "host:%s", path0+6);
//	else
//		strcpy(path, path0);
	strcpy(path, path0);
	if (path[strlen(path)-1]=='/')
		path[strlen(path)-1]=0;
	
	if ((dd = fioDopen(path)) < 0) goto exit;

	while(fioDread(dd, &record) > 0){
		itoNoVSync();
		if((FIO_SO_ISDIR(record.stat.mode)) && (!strcmp(record.name,".") || !strcmp(record.name,"..")))
			continue;

		if(FIO_SO_ISDIR(record.stat.mode)){
			info[n].attr = MC_ATTR_SUBDIR;
		}
		else {
			info[n].attr = 0;
			info[n].fileSizeByte = record.stat.size;
		}
		//else
		//	continue;

		strcpy(info[n].name, record.name);
		strncpy(info[n].name, info[n].name, 32);
		memset(&info[n].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[n].modifytime.unknown = 0;
		info[n].modifytime.sec = record.stat.mtime[1];
		info[n].modifytime.min = record.stat.mtime[2];
		info[n].modifytime.hour = record.stat.mtime[3];
		info[n].modifytime.day = record.stat.mtime[4];
		info[n].modifytime.month = record.stat.mtime[5];
		info[n].modifytime.year = record.stat.mtime[6] + record.stat.mtime[7]*256;
		if (info[n].modifytime.year < 1000) info[n].modifytime.year+=1900;
		
		info[n].timestamp = ps2timetostamp(&info[n].modifytime);
		info[n].num = n;
		n++;
		if(n==max) break;
	}

exit:
	if(dd >= 0) fioDclose(dd);
	return n;
}

//-------------------------------------------------
// その他のデバイスの読み込み2
int readOthersX(const char *path, FILEINFO *info, int max)
{
	iox_dirent_t dirbuf;
	char dir[MAX_PATH];
	int i=0, fd;

	strcpy(dir, path);
	if((fd=fileXioDopen(dir)) < 0) return 0;

	while(fileXioDread(fd, &dirbuf)){
		itoNoVSync();
		if(dirbuf.stat.mode & FIO_S_IFDIR && (!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;

		if(dirbuf.stat.mode & FIO_S_IFDIR){
			info[i].attr = MC_ATTR_SUBDIR;
		}
		else{
			info[i].attr = 0;
			info[i].fileSizeByte = dirbuf.stat.size;
		}
		strcpy(info[i].name, dirbuf.name);
		memset(&info[i].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[i].modifytime.unknown = dirbuf.stat.mtime[0];
		info[i].modifytime.sec = dirbuf.stat.mtime[1];
		info[i].modifytime.min = dirbuf.stat.mtime[2];
		info[i].modifytime.hour = dirbuf.stat.mtime[3];
		info[i].modifytime.day = dirbuf.stat.mtime[4];
		info[i].modifytime.month = dirbuf.stat.mtime[5];
		info[i].modifytime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
		info[i].timestamp = ps2timetostamp(&info[i].modifytime);
		info[i].num = i;
		i++;
		if(i==max) break;
	}

	fileXioDclose(fd);

	return i;
}
int readVMC(const char *path, FILEINFO *info, int max)
{
	iox_dirent_t dirbuf;
	char dir[MAX_PATH];
	int i=0, fd;
	
	strcpy(dir, path);
	if((fd=fileXioDopen(dir)) < 0) return 0;
	
	while(fileXioDread(fd, &dirbuf) > 0){
		itoNoVSync();
		if(dirbuf.stat.mode & MC_ATTR_SUBDIR && (!strcmp(dirbuf.name, ".") || !strcmp(dirbuf.name, "..")))
			continue;
		if(dirbuf.stat.mode & MC_ATTR_SUBDIR){
			info[i].attr = dirbuf.stat.mode;
		}
		else{// if(dirbuf.stat.mode & MC_ATTR_FILE){
			info[i].attr = dirbuf.stat.mode;
			info[i].fileSizeByte = dirbuf.stat.size;
		}
		//else continue;
		strcpy(info[i].name, dirbuf.name);
		memset(&info[i].createtime, 0, sizeof(PS2TIME)); //取得しない
		info[i].modifytime.unknown = dirbuf.stat.mtime[0];
		info[i].modifytime.sec = dirbuf.stat.mtime[1];
		info[i].modifytime.min = dirbuf.stat.mtime[2];
		info[i].modifytime.hour = dirbuf.stat.mtime[3];
		info[i].modifytime.day = dirbuf.stat.mtime[4];
		info[i].modifytime.month = dirbuf.stat.mtime[5];
		info[i].modifytime.year = dirbuf.stat.mtime[6]+dirbuf.stat.mtime[7]*256;
		info[i].timestamp = ps2timetostamp(&info[i].modifytime);
		info[i].num = i;
		i++;
		if(i==max) break;
	}

	fileXioDclose(fd);

	return i;
}
//-------------------------------------------------
// ファイルリスト取得
void load_iomanx();
void load_filexio();
int load_irxdriver(char *path, int arg_len, char *args);
int vmcmount=FALSE;
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
	else if(!strncmp(path, "host", 4))
		n=readHOST(path, info, max);
	else if(!strncmp(path, "vmc", 3)) {
		static int loaded=FALSE;
		u8 dir[MAX_PATH];
		if (!loaded) {
			load_iomanx();
			load_filexio();
			n = load_irxdriver(setting->vmc_path, 0, NULL);
			if (n >= 0) {
				loaded=TRUE;
			} 
		}
		if (loaded) {
			if (!vmcmount && nclipFiles) {
				strcpy(dir, clipPath);
				if (dir[strlen(dir)-1] == '/')
					dir[strlen(dir)-1] = 0;
				strcat(dir, clipFiles[0].name);
				printf("clipPath: %s\nclipFile: %s\n", clipPath, clipFiles[0].name);
				if ((n=fileXioMount("vmc:", dir, FIO_MT_RDWR))>=0) {
					vmcmount=TRUE;
				}
			}
			n=readVMC(path, info, max);
		}
		else
			n = 0;
	}
	else if(!strncmp(path, "sd", 2))
		n=readOthersX(path, info, max);
	else 
		n=readOthers(path, info, max);

	return n;
}

