#include "launchelf.h"
#define	FULLHD_WIDTH	setting->fullhd_width
#define tmpbuffersize	256*256
//----------------------------------------------------------
typedef struct {
	char Identifier[6];	// "FONTX2"
	char FontName[8];		// Font名
	unsigned char XSize;
	unsigned char YSize;
	unsigned char CodeType;
	unsigned char Tnum;	// テーブルのエントリ数
	struct {
		unsigned short Start;	// 領域の始まりの文字コード
		unsigned short End;	// 領域の終わりの文字コード
	} Block[];
} FONTX_HEADER;

typedef struct {
	char font_name[9];
	int width;
	int height;
	int size;	//1文字分のサイズ
	int Tnum;
	int offset;
} FONTX_DATA;

#ifdef ENABLE_ICON
extern uint8 *icon_iif[];
extern int size_icon_iif;
#endif

//----------------------------------------------------------
itoGsEnv screen_env;
uint16 buffer_width;
uint16 buffer_height;

int initbiosfont=0;
char *biosfont=NULL;
int SCREEN_LEFT;
int SCREEN_TOP;
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 448;
int SCREEN_MARGIN;
int FONT_WIDTH;
int FONT_HEIGHT;
int MAX_ROWS;
int MAX_ROWS_X;

int char_Margin;	//文字の間隔
int line_Margin;	//行の間隔
int font_bold;
int font_half, font_vhalf;
int fonthalfmode=0;

int interlace;
int ffmode;
int screenscan;
//ascii
int init_ascii=0;	//初期化したかしていないかのフラグ
int framebuffers=2;
char *font_ascii=NULL;	//フォントのバッファ
FONTX_DATA ascii_data;	//フォントの情報
int ascii_MarginTop;	//上のマージン
int ascii_MarginLeft;	//左のマージン
//kanji
int init_kanji=0;
char *font_kanji=NULL;
FONTX_DATA kanji_data;
int kanji_MarginTop;
int kanji_MarginLeft;

unsigned short font_sjis_table[] = {
0x8140,0x817e,
0x8180,0x81ac,
0x81b8,0x81bf,
0x81c8,0x81ce,
0x81da,0x81e8,
0x81f0,0x81f7,
0x81fc,0x81fc,
0x824f,0x8258,
0x8260,0x8279,
0x8281,0x829a,
0x829f,0x82f1,
0x8340,0x837e,
0x8380,0x8396,
0x839f,0x83b6,
0x83bf,0x83d6,
0x8440,0x8460,
0x8470,0x847e,
0x8480,0x8491,
0x849f,0x84be,
0x889f,0x88fc,
0x8940,0x897e,
0x8980,0x89fc,
0x8a40,0x8a7e,
0x8a80,0x8afc,
0x8b40,0x8b7e,
0x8b80,0x8bfc,
0x8c40,0x8c7e,
0x8c80,0x8cfc,
0x8d40,0x8d7e,
0x8d80,0x8dfc,
0x8e40,0x8e7e,
0x8e80,0x8efc,
0x8f40,0x8f7e,
0x8f80,0x8ffc,
0x9040,0x907e,
0x9080,0x90fc,
0x9140,0x917e,
0x9180,0x91fc,
0x9240,0x927e,
0x9280,0x92fc,
0x9340,0x937e,
0x9380,0x93fc,
0x9440,0x947e,
0x9480,0x94fc,
0x9540,0x957e,
0x9580,0x95fc,
0x9640,0x967e,
0x9680,0x96fc,
0x9740,0x977e,
0x9780,0x97fc,
0x9840,0x9872
};

void drawChar_1bpp(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_bilinear(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_filter(void *src, int x, int y, uint64 color, int w, int h);
void drawChar_8bpp(void *src, int x, int y, uint64 color, uint64 back, int w, int h);
void drawChar_resize(void *dist, void *src, int dw, int dh, int sw, int sh);
void drawChar_unpack(void *dist, void *src, int w, int h);
void drawChar_resize2(void *dist, void *src, int w, int h, int x, int y);
static unsigned char bmpsrc[tmpbuffersize], bmpdst[tmpbuffersize];

//-------------------------------------------------
// setup ito
void setupito(int tvmode)
{
	uint8 vmode;
	uint8 psm=0;
	int buffer_height_t, buffersize;

	vmode = ITO_VMODE_AUTO;
	switch(tvmode)
	{
		case 0:	{vmode = ITO_VMODE_AUTO; break;}
		case 1:	{vmode = ITO_VMODE_NTSC; break;}
		case 2:	{vmode = ITO_VMODE_PAL; break;}
		case 3:	{vmode = 0x50; break;}
		case 4:	{vmode = 0x52; break;}
		case 5:	{vmode = 0x51; break;}
		case 6:	{vmode = 0x53; break;}
	}

	if(screenscan){
		switch(vmode)
		{
			case ITO_VMODE_NTSC:
			{
				buffer_width = 704;
				buffer_height= 480;
				psm = ITO_RGBA32;
				break;
			}
			case ITO_VMODE_PAL:
			{
				buffer_width = 704;
				buffer_height= 576;
				psm = ITO_RGBA32;
				break;
			}
			case 0x50://480p
			{
				buffer_width = 768;
				buffer_height= 480;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x51://1080i
			{
				buffer_width = FULLHD_WIDTH;//-48;
				buffer_height= 1080;//-56;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x52://720p:
			{
				buffer_width = 1280;//-64;
				buffer_height= 720;//-36;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x53://1080p
			{
				buffer_width = FULLHD_WIDTH;//-48;
				buffer_height= 1080;//-56;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			default:
			{
				break;
				//NTSC
				buffer_width = 704;
				buffer_height= 480;
				psm = ITO_RGBA32;
				vmode = ITO_VMODE_NTSC;
				break;
			}
		}
	}
	else{
		switch(vmode)
		{
			case ITO_VMODE_NTSC:
			{
				buffer_width = 640;
				buffer_height= 448;
				psm = ITO_RGBA32;
				break;
			}
			case ITO_VMODE_PAL:
			{
				buffer_width = 640;
				buffer_height= 512;
				psm = ITO_RGBA32;
				break;
			}
			case 0x50://480p
			{
				buffer_width = 640;
				buffer_height= 448;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x51://1080i
			{
				buffer_width = FULLHD_WIDTH-((int)(FULLHD_WIDTH / 20));//960-64;//-48;
				buffer_height= 1080-56;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x52://720p:
			{
				buffer_width = 1280-64;
				buffer_height= 720-36;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			case 0x53:
			{
				buffer_width = FULLHD_WIDTH-((int)(FULLHD_WIDTH / 20));//960-64;//-48;
				buffer_height= 1080-56;
				psm = ITO_RGBA32;
				//setting->interlace = ITO_NON_INTERLACE;
				//setting->ffmode = ITO_FIELD;
				break;
			}
			default:
			{
				break;
				//NTSC
				buffer_width = 640;
				buffer_height= 448;
				psm = ITO_RGBA32;
				vmode = ITO_VMODE_NTSC;
				break;
			}
		}
	}
	
	if (ffmode == ITO_FIELD)
		buffer_height_t = buffer_height;
	else
		buffer_height_t = buffer_height >> 1;
	
	buffersize = buffer_height_t*buffer_width*itoGetPixelSize(psm);
	if (buffersize >= 4*1024*1024) {
		psm = ITO_RGBA16;
		buffersize = buffer_height_t*buffer_width*itoGetPixelSize(psm);
	}

	// screen resolution
	screen_env.screen.width		= buffer_width;
	screen_env.screen.height	= buffer_height;
	screen_env.screen.psm		= psm;

	// These setting work best with my tv, experiment for youself
	screen_env.screen.x			= SCREEN_LEFT; 
	screen_env.screen.y			= SCREEN_TOP;
	
	screen_env.framebuffer1.x	= 0;
	screen_env.framebuffer1.y	= 0;
	
	//if ((buffer_width >= 1536) && (buffer_height_t >= 1024)) {
	if (buffersize >= 2*1024*1024) {
		// single frame buffer
		framebuffers = 1;
		screen_env.framebuffer2.x   = 0;
		screen_env.framebuffer2.y	= 0;
		// zbuffer
		screen_env.zbuffer.x		= 0;
		screen_env.zbuffer.y		= (buffer_height_t+15)&-16;
		screen_env.zbuffer.psm		= ITO_ZBUF16;
	} else {
		// double frame buffer
		framebuffers = 2;
		screen_env.framebuffer2.x	= 0;
		screen_env.framebuffer2.y	= (buffer_height_t+15)&-16;
		// zbuffer
		screen_env.zbuffer.x		= 0;
		screen_env.zbuffer.y		= ((buffer_height_t+15)&-16)*2;
		screen_env.zbuffer.psm		= ITO_ZBUF16;
	}
	// scissor 
	screen_env.scissor_x1		= 0;
	screen_env.scissor_y1		= 0;
	screen_env.scissor_x2		= buffer_width;
	screen_env.scissor_y2		= buffer_height_t;
	
	// misc
	screen_env.dither			= FALSE;//TRUE;
	screen_env.interlace		= interlace;
	screen_env.ffmode			= ffmode;
	screen_env.vmode			= vmode;
#if 0
	printf("draw: screen setup:\n");
	printf("\tvmode: %02X\n", screen_env.vmode);
	printf("\tbuffer size: %dx%d\n", screen_env.screen.width, screen_env.screen.height);
	printf("\tvram size: %dx%d\n", buffer_width, buffer_height_t);
	//printf("\tscreen offset: (%d,%d)\n", SCREEN_LEFT, SCREEN_TOP);
	printf("\tframe buffer 1 position: (%d,%d)-(%d,%d)\n", screen_env.framebuffer1.x,screen_env.framebuffer1.y);
	printf("\tframe buffer 2 position: (%d,%d)-(%d,%d)\n", screen_env.framebuffer2.x,screen_env.framebuffer2.y);
	printf("\tZ buffer offset: (%d,%d)\n", screen_env.zbuffer.x,screen_env.zbuffer.y);
	printf("\tscissor: (%d,%d)-(%d,%d)\n", screen_env.scissor_x1, screen_env.scissor_y1, screen_env.scissor_x2, screen_env.scissor_y2);
	printf("\tdither,interlace,ffmode: %d,%d,%d\n",  screen_env.dither, screen_env.interlace, screen_env.ffmode);
#endif
	itoGsEnvSubmit(&screen_env);

	//アルファブレンド
	itoSetAlphaBlending(
		ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
		ITO_ALPHA_COLOR_DST, // B = COLOR DEST
		ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
		ITO_ALPHA_COLOR_DST, // C = COLOR DEST
		0x80);				 // Fixed Value

	itoZBufferUpdate(FALSE);
	itoZBufferTest(FALSE, 0);
	itoSetTextureBufferBase( itoGetZBufferBase() );
	printf("\tTexture buffer base: %08X\n", itoGetTextureBufferBase());
}

//-------------------------------------------------
// 半透過カラー取得用 
uint64 half(uint64 color1, uint64 color2, int blend) {
	unsigned int l3,l2,l1,la;
	unsigned int r3,r2,r1,ra;
	unsigned int t3,t2,t1,ta;
	uint64 lz,rz,tz;
	unsigned int lb,rb;
	if (blend <= 0) return color1;
	if (blend > 0xff) return color2;
	l3 = (color1 >> 16) & 0xff;
	l2 = (color1 >>  8) & 0xff;
	l1 =  color1        & 0xff;
	la = (color1 >> 24) & 0xff;
	lz =  color1 >> 32;
	r3 = (color2 >> 16) & 0xff;
	r2 = (color2 >>  8) & 0xff;
	r1 =  color2        & 0xff;
	ra = (color2 >> 24) & 0xff;
	rz =  color2 >> 32;
	rb = blend & 0xff;
	lb = 0x100 - rb;
	t3 = (l3 * lb + r3 * rb) >> 8;
	t2 = (l2 * lb + r2 * rb) >> 8;
	t1 = (l1 * lb + r1 * rb) >> 8;
	ta = (la * lb + ra * rb) >> 8;
	tz = (lz * lb + rz * rb) >> 8;
	return (tz<<32)|(ta<<24)|(t3<<16)|(t2<<8)|t1;
}

//-------------------------------------------------
// 暗くする(半透明の黒い四角)
void drawDark(void)
{
	//アルファブレンド有効
	itoPrimAlphaBlending( TRUE );
	//
	itoSprite(ITO_RGBA(0,0,0,0x10),
		0, SCREEN_MARGIN+FONT_HEIGHT*2.5,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0);
	//アルファブレンド無効
	itoPrimAlphaBlending(FALSE);
}

//-------------------------------------------------
// ダイアログの背景
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2)
{
	//
	itoSprite(color1, x1, y1, x2, y2, 0);
	drawFrame(x1+2, y1+2, x2-2, y2-2, color2);
}

//-------------------------------------------------
// 画面表示のテンプレート
void setScrTmp(const char *msg0, const char *msg1)
{
	uint64 color1,color;
	uint64 color2;	//アルファ付き

	color = setting->color[COLOR_FRAME]&0x00FFFFFF;	//透明度を除外
	color1 = color|0x80000000;	//不透明
	color2 = color|(setting->flicker_alpha << 24);	//半透明
	//color2 = half(color, setting->color[COLOR_BACKGROUND], 0x80);

	// バージョン表記
	printXY(LBF_VER, FONT_WIDTH*2, SCREEN_MARGIN, setting->color[COLOR_TEXT], TRUE);

	// メッセージ
	printXY(msg0, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT, setting->color[COLOR_TEXT], TRUE);

	//上の横線
	itoLine(color1, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0,
		color1, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5, 0);	
	//下の横線
	itoLine(color1, 0, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0,
		color1, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5), 0);	

	//FLICKER CONTROL: ON
	if(setting->flickerControl==TRUE && setting->interlace==TRUE){
		//アルファブレンド有効
		itoPrimAlphaBlending( TRUE );
		//上の横線
		itoLine(color2, 0, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2.5+1, 0);	
		//下の横線
		itoLine(color2, 0, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5)+1, 0,
			color2, SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+3.5)+1, 0);	
		//アルファブレンド無効
		itoPrimAlphaBlending(FALSE);
	}

	// 操作説明
	printXY(msg1, FONT_WIDTH*1, SCREEN_MARGIN+FONT_HEIGHT*(MAX_ROWS+4), setting->color[COLOR_TEXT], TRUE);
}

//-------------------------------------------------
// メッセージ描画
void drawMsg(const char *msg)
{
	itoSprite(setting->color[COLOR_BACKGROUND], 0, SCREEN_MARGIN+FONT_HEIGHT,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
	//メッセージ
	printXY(msg, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT,
		setting->color[COLOR_TEXT], TRUE);
	drawScr();
}

//-------------------------------------------------
// 画面のクリア
void clrScr(uint64 color)
{
	itoSprite(color, 0, 0, buffer_width, buffer_height, 0);
}

//-------------------------------------------------
// 画面の描画
void drawScr(void)
{
	itoGsFinish();
	itoVSync();
	itoSwitchFrameBuffers();
}

//-------------------------------------------------
// 枠の描画
void drawFrame(int x1, int y1, int x2, int y2, uint64 color)
{
	uint64 color0,color1,color2;	//アルファ付き

	color0 = color&0x00FFFFFF;	//透明度を除外
	color1 = color0|0x80000000;	//不透明
	color2 = color0|(setting->flicker_alpha << 24);	//半透明
	//color2 = half(color, setting->color[COLOR_BACKGROUND], 0x80);

	//FLICKER CONTROL: ON
	if(setting->flickerControl==TRUE && setting->interlace==TRUE){
		//アルファブレンド有効
		itoPrimAlphaBlending( TRUE );
		//上の横線
		itoLine(color2, x1, y1+1, 0, color2, x2, y1+1, 0);
		//下の横線
		itoLine(color2, x2, y2+1, 0, color2, x1, y2+1, 0);
		//アルファブレンド無効
		itoPrimAlphaBlending(FALSE);
	}

	//上の横線
	itoLine(color1, x1, y1, 0, color, x2, y1, 0);
	//右の縦線
	itoLine(color1, x2, y1, 0, color, x2, y2, 0);	
	//下の横線
	itoLine(color1, x2, y2, 0, color, x1, y2, 0);
	//左の縦線
	itoLine(color1, x1, y2, 0, color, x1, y1, 0);
}

//-------------------------------------------------
//MAX_ROWSなどの設定
void SetHeight(void)
{
	//SCREEN_WIDTHとSCREEN_HEIGHT
	if(screenscan){
		switch(setting->tvmode)
		{
			case 0:	//AUTO
			{
				SCREEN_WIDTH = 704;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(ITO_VMODE_AUTO==ITO_VMODE_NTSC){
					//NTSC
					if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
						SCREEN_HEIGHT = 480;
					else
						SCREEN_HEIGHT = 240;
				}
				else{
					//PAL
					if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
						SCREEN_HEIGHT = 576;
					else
						SCREEN_HEIGHT = 288;
				}
				break;
			}
			case 1:	//NTSC
			{
				SCREEN_WIDTH = 704;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
					SCREEN_HEIGHT = 480;
				else
					SCREEN_HEIGHT = 240;
				break;
			}
			case 2:	//PAL
			{
				SCREEN_WIDTH = 704;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
					SCREEN_HEIGHT = 576;
				else
					SCREEN_HEIGHT = 288;
				break;
			}
			case 3:	//480p
			{
				SCREEN_LEFT = setting->screen_x_480p;
				SCREEN_TOP = setting->screen_y_480p;
				SCREEN_WIDTH = 720;
				SCREEN_HEIGHT = 480;
				break;
			}
			case 4:	//720p
			{
				SCREEN_LEFT = setting->screen_x_720p;
				SCREEN_TOP = setting->screen_y_720p;
				SCREEN_WIDTH = 1280;
				SCREEN_HEIGHT = 720;
				break;
			}
			case 5:	//1080i
			case 6:
			{
				SCREEN_WIDTH = FULLHD_WIDTH;//960;//1280;1440-72;//1920-96;
				SCREEN_LEFT = setting->screen_x_1080i;
				SCREEN_TOP = setting->screen_y_1080i;
				if(setting->ffmode_1080i==FALSE)
					SCREEN_HEIGHT = 1080;
				else
					SCREEN_HEIGHT = 540;
				//SCREEN_HEIGHT = 1024;
				break;
			}
		}
	}
	else{
		switch(setting->tvmode)
		{
			case 0:	//AUTO
			{
				SCREEN_WIDTH = 640;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(ITO_VMODE_AUTO==ITO_VMODE_NTSC){
					//NTSC
					if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
						SCREEN_HEIGHT = 448;
					else
						SCREEN_HEIGHT = 224;
				}
				else{
					//PAL
					if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
						SCREEN_HEIGHT = 512;
					else
						SCREEN_HEIGHT = 256;
				}
				break;
			}
			case 1:	//NTSC
			{
				SCREEN_WIDTH = 640;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
					SCREEN_HEIGHT = 448;
				else
					SCREEN_HEIGHT = 224;
				break;
			}
			case 2:	//PAL
			{
				SCREEN_WIDTH = 640;
				SCREEN_LEFT = setting->screen_x_480i;
				SCREEN_TOP = setting->screen_y_480i;
				if(setting->ffmode_480i==FALSE && setting->interlace==TRUE)
					SCREEN_HEIGHT = 512;
				else
					SCREEN_HEIGHT = 256;
				break;
			}
			case 3:	//480p
			{
				SCREEN_LEFT = setting->screen_x_480p;
				SCREEN_TOP = setting->screen_y_480p;
				SCREEN_WIDTH = 640;
				SCREEN_HEIGHT = 480-32;
				break;
			}
			case 4:	//720p
			{
				SCREEN_LEFT = setting->screen_x_720p;
				SCREEN_TOP = setting->screen_y_720p;
				SCREEN_WIDTH = 1280-64;
				SCREEN_HEIGHT = 720-32;
				break;
			}
			case 5:	//1080i
			case 6:
			{
				SCREEN_WIDTH = (FULLHD_WIDTH-((int)(FULLHD_WIDTH / 20))+63) & 0x7fc0;//-48;//1280;1440-72;//1920-96;
				SCREEN_LEFT = setting->screen_x_1080i;
				SCREEN_TOP = setting->screen_y_1080i;
				if(setting->ffmode_1080i==FALSE)
					SCREEN_HEIGHT = 1024;
				else
					SCREEN_HEIGHT = 512;
				//SCREEN_HEIGHT = 1024;
				break;
			}
		}
	}
	//FONT_WIDTH
	if (font_half > 0) 
		FONT_WIDTH = (ascii_data.width+font_half) / (font_half+1) + char_Margin;
	else if (font_half == 0)
		FONT_WIDTH = ascii_data.width + char_Margin;
	else
		FONT_WIDTH = ascii_data.width * (-font_half+1) + char_Margin;

	//FONT_HEIGHT
	if(ascii_data.height>=kanji_data.height)
		FONT_HEIGHT = ascii_data.height;// + line_Margin;
	else
		FONT_HEIGHT = kanji_data.height;// + line_Margin;
	if (font_vhalf > 0)
		FONT_HEIGHT = (FONT_HEIGHT+font_vhalf) / (font_vhalf+1);
	else if (font_vhalf < 0)
		FONT_HEIGHT*= -font_vhalf+1;
	FONT_HEIGHT+= line_Margin;
	
	if (FONT_HEIGHT < 1) FONT_HEIGHT = 1;
	//if (FONT_WIDTH < 1) FONT_WIDTH = 1;
	
	//MAX_ROWS
	MAX_ROWS = SCREEN_HEIGHT/FONT_HEIGHT-6;
	MAX_ROWS_X = SCREEN_WIDTH/FONT_WIDTH-11;

	//SCREEN_MARGIN
	SCREEN_MARGIN = (SCREEN_HEIGHT - ((MAX_ROWS+5) * FONT_HEIGHT))/2;
}

//------------------------------------------------------------
//アスキーフォントをロード
int InitFontAscii(const char *path)
{
	int fd=0, dsize;
	size_t size;
	FONTX_HEADER *fontx_header_ascii;
	char fullpath[MAX_PATH];
	char *tekbuff=NULL;

	if(init_ascii==1) FreeFontAscii();

	if(strcmp(path, "rom0:KROM")==0 || strcmp(path, "systemfont")==0){
		//BIOSFont
		//フォントファイルオープン
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;
	
		//メモリを確保 仮想FONTX2 KROM
		size = 17 + 15*256;	//ヘッダサイズ + 1文字のサイズ*256文字
		font_ascii = (char*)malloc(size);
		memset(font_ascii, 0, size);
		if(font_ascii==NULL){
			fioClose(fd);
			return -1;
		}

		//メモリに読み込む
		fioLseek(fd, 0x198DE, SEEK_SET);
		fioRead(fd, font_ascii + 17 + 15*33, 15*95);//ヘッダサイズ + 1文字のサイズ*33文字, 1文字のサイズ*95文字

		//クローズ
		fioClose(fd);

		//ヘッダのポインタ
		fontx_header_ascii = (FONTX_HEADER*)font_ascii;

		//ヘッダ作成
		strncpy(fontx_header_ascii->Identifier, "FONTX2", 6);
		strncpy(fontx_header_ascii->FontName, "KROM", 8);
		fontx_header_ascii->XSize = 8;
		fontx_header_ascii->YSize =15;
		fontx_header_ascii->CodeType = 0;
/*
		KROMのアスキーフォントをFONTX2でダンプ
		{
			fd=fioOpen("host:KROM.fnt",O_WRONLY | O_TRUNC | O_CREAT);
			fioWrite(fd, font_ascii, size);
			fioClose(fd);
		}
*/
	}
	else{
		//FONTX2
		if(!strncmp(path, "mc:", 3)){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkFONTX2header(fullpath)<0){
				fullpath[2]='1';
				if(checkFONTX2header(fullpath)<0)
					fullpath[0]=0;
			}
		}
		else
			strcpy(fullpath, path);

		//フォントファイルオープン
		fd = fioOpen(fullpath, O_RDONLY);
		if(fd<0) return -1;
	
		//圧縮判定用
		fioRead(fd, fullpath, 32);

		//サイズを調べる
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//シークを0に戻す

		if ((dsize = tek_getsize(fullpath))>=0) {
			//tek展開
			//メモリを確保
			tekbuff = (char*)malloc(size);
			if(tekbuff==NULL){
				fioClose(fd);
				return -1;
			}
			//元データを読み込む
			fioRead(fd, tekbuff, (size_t)size);
			
			//クローズ
			fioClose(fd);
			
			//展開先バッファを確保
			font_ascii = (char*)malloc(dsize);
			if(font_ascii==NULL){
				free(tekbuff);
				return -1;
			}
			//tek展開
			if(tek_decomp(tekbuff, font_ascii, size)<0){
				free(tekbuff);
				free(font_ascii);
				return -1;
			}
		} else {
			//メモリを確保
			font_ascii = (char*)malloc(size);
			if(font_ascii==NULL){
				fioClose(fd);
				return -1;
			}

			//メモリに読み込む
			fioRead(fd, font_ascii, (size_t)size);

			//クローズ
			fioClose(fd);
		}
	}

	//ヘッダのポインタ
	fontx_header_ascii = (FONTX_HEADER*)font_ascii;

	//ヘッダチェック
	if(strncmp(fontx_header_ascii->Identifier, "FONTX2", 6)!=0)
		return -1;
	if(fontx_header_ascii->CodeType!=0)
		return -1;

	//フォントの情報
	strncpy(ascii_data.font_name, fontx_header_ascii->FontName, 8);
	ascii_data.font_name[8] = '\0';
	//1文字のサイズ
	ascii_data.width = fontx_header_ascii->XSize;
	ascii_data.height = fontx_header_ascii->YSize;
	//1文字のサイズ算出
	ascii_data.size = ((ascii_data.width-1)/8+1) * ascii_data.height;
	//
	ascii_data.Tnum = 0;
	//
	ascii_data.offset = 17;

	SetHeight();

	//フォントロード成功
	init_ascii=1;
	return 0;
}

//------------------------------------------------------------
//漢字フォントをロード
int InitFontKnaji(const char *path)
{
	int fd=0, dsize;
	size_t size;
	FONTX_HEADER *fontx_header_kanji;
	char fullpath[MAX_PATH];
	char *tekbuff=NULL;

	if(init_kanji==1) FreeFontKanji();

	if(strcmp(path,"rom0:KROM")==0 || strcmp(path,"systemfont")==0){
		//BIOSFont
		//フォントファイルオープン
		fd = fioOpen("rom0:KROM", O_RDONLY);
		if(fd<0) return -1;

		//メモリを確保 仮想FONTX2 KROM
		size = 18 + 51*4 + 30*3489;	//ヘッダサイズ + テーブルの数*4 + 1文字のサイズ*3489文字
		font_kanji = (char*)malloc(size);
		memset(font_kanji, 0, size);
		if(font_kanji==NULL){
			fioClose(fd);
			return -1;
		}

		//メモリに読み込む
		fioRead(fd, font_kanji + 18 + 51*4, 30*3489);//ヘッダサイズ + テーブルの数*4 ,1文字のサイズ*3489文字

		//クローズ
		fioClose(fd);

		//ヘッダのポインタ
		fontx_header_kanji = (FONTX_HEADER*)font_kanji;

		//ヘッダ作成
		strncpy(fontx_header_kanji->Identifier, "FONTX2", 6);
		strncpy(fontx_header_kanji->FontName, "KROM_k", 8);
		fontx_header_kanji->XSize = 16;
		fontx_header_kanji->YSize = 15;
		fontx_header_kanji->CodeType = 1;
		fontx_header_kanji->Tnum = 51;
		//テーブル
		memcpy(font_kanji+18,font_sjis_table,51*4);

/*
		KROMの漢字フォントをFONTX2でダンプ
		{
			fd=fioOpen("host:KROM_k.fnt",O_WRONLY | O_TRUNC | O_CREAT);
			fioWrite(fd, font_kanji, size);
			fioClose(fd);
		}
*/
	}
	else{
		//FONTX2
		if(!strncmp(path, "mc:", 3)){
			strcpy(fullpath, "mc0:");
			strcat(fullpath, path+3);
			if(checkFONTX2header(fullpath)<0){
				fullpath[2]='1';
				if(checkFONTX2header(fullpath)<0)
					fullpath[0]=0;
			}
		}
		else
			strcpy(fullpath, path);

		//フォントファイルオープン
		fd = fioOpen(fullpath, O_RDONLY);
		if(fd<0) return -1;

		//圧縮判定用
		fioRead(fd, fullpath, 32);

		//サイズを調べる
		size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);	//シークを0に戻す

		if ((dsize = tek_getsize(fullpath))>=0) {
			//tek展開
			//メモリを確保
			tekbuff = (char*)malloc(size);
			if(tekbuff==NULL){
				fioClose(fd);
				return -1;
			}
			//元データを読み込む
			fioRead(fd, tekbuff, (size_t)size);
			
			//クローズ
			fioClose(fd);
			
			//展開先バッファを確保
			font_kanji = (char*)malloc(dsize);
			if(font_kanji==NULL){
				free(tekbuff);
				return -1;
			}
			//tek展開
			if(tek_decomp(tekbuff, font_kanji, size)<0){
				free(tekbuff);
				free(font_kanji);
				return -1;
			}
		} else {
			//メモリを確保
			font_kanji = (char*)malloc(size);
			if(font_kanji==NULL){
				fioClose(fd);
				return -1;
			}

			//メモリに読み込む
			fioRead(fd, font_kanji, (size_t)size);

			//クローズ
			fioClose(fd);
		}
	}
	
	//ヘッダのポインタ
	fontx_header_kanji = (FONTX_HEADER*)font_kanji;

	//ヘッダチェック
	if(strncmp(fontx_header_kanji->Identifier, "FONTX2", 6)!=0)
		return -1;
	if(fontx_header_kanji->CodeType!=1)
		return -1;

	//フォントの情報
	strncpy(kanji_data.font_name, fontx_header_kanji->FontName, 8);
	kanji_data.font_name[8] = '\0';
	//1文字のサイズ
	kanji_data.width = fontx_header_kanji->XSize;
	kanji_data.height = fontx_header_kanji->YSize;
	//1文字のサイズ算出
	kanji_data.size = ((kanji_data.width+7)/8) * kanji_data.height;
	//
	kanji_data.Tnum = fontx_header_kanji->Tnum;
	//
	kanji_data.offset = 18 + kanji_data.Tnum*4;

	SetHeight();

	//フォントロード成功
	init_kanji=1;
	return 0;
}

//------------------------------------------------------------
//半角フォントを開放
void FreeFontAscii(void)
{
	free(font_ascii);
	memset(&ascii_data, 0, sizeof(FONTX_DATA));
	init_ascii=0;
	return;
}

//------------------------------------------------------------
//漢字フォントを開放
void FreeFontKanji(void)
{
	free(font_kanji);
	memset(&kanji_data, 0, sizeof(FONTX_DATA));
	init_kanji=0;
	return;
}

//------------------------------------------------------------
//フォントのマージンを設定
int SetFontMargin(int type, int Margin)
{
	if(type<CHAR_MARGIN || type>KANJI_FONT_MARGIN_LEFT) return -1;

	switch(type)
	{
		case CHAR_MARGIN:
			char_Margin = Margin;
			SetHeight();
			break;
		case LINE_MARGIN:
			line_Margin=Margin;
			SetHeight();
			break;
		case ASCII_FONT_MARGIN_TOP:
			ascii_MarginTop=Margin;
			break;
		case ASCII_FONT_MARGIN_LEFT:
			ascii_MarginLeft=Margin;
			break;
		case KANJI_FONT_MARGIN_TOP:
			kanji_MarginTop=Margin;
			break;
		case KANJI_FONT_MARGIN_LEFT:
			kanji_MarginLeft=Margin;
			break;
	}

	return 0;
}

//------------------------------------------------------------
//フォントのマージンを取得
int GetFontMargin(int type)
{
	if(type<CHAR_MARGIN || type>KANJI_FONT_MARGIN_LEFT) return -1;

	switch(type)
	{
		case CHAR_MARGIN:
			return char_Margin;
		case LINE_MARGIN:
			return line_Margin;
		case ASCII_FONT_MARGIN_TOP:
			return ascii_MarginTop;
		case ASCII_FONT_MARGIN_LEFT:
			return ascii_MarginLeft;
		case KANJI_FONT_MARGIN_TOP:
			return kanji_MarginTop;
		case KANJI_FONT_MARGIN_LEFT:
			return kanji_MarginLeft;
	}

	return 0;
}

//------------------------------------------------------------
//フォントサイズの取得
int GetFontSize(int type)
{
	switch(type)
	{
		case ASCII_FONT_WIDTH:
			if (font_half > 0) 
				return (ascii_data.width+font_half) / (font_half+1);
			else if (font_half == 0)
				return ascii_data.width;
			else
				return ascii_data.width * (-font_half+1);
		case ASCII_FONT_HEIGHT:
			if (font_vhalf > 0) 
				return (ascii_data.height+font_vhalf) / (font_vhalf+1);
			else if (font_vhalf == 0)
				return ascii_data.height;
			else
				return ascii_data.height * (-font_vhalf+1);
		case KANJI_FONT_WIDTH:
			if (font_half > 0) 
				return (kanji_data.width+font_half) / (font_half+1);
			else if (font_half == 0)
				return kanji_data.width;
			else
				return kanji_data.width * (-font_half+1);
		case KANJI_FONT_HEIGHT:
			if (font_vhalf > 0) 
				return (kanji_data.height+font_vhalf) / (font_vhalf+1);
			else if (font_vhalf == 0)
				return kanji_data.height;
			else
				return kanji_data.height * (-font_vhalf+1);
	}
	return 0;
}

//------------------------------------------------------------
//フォントボールドを設定
void SetFontBold(int flag)
{
	font_bold = flag;
	return;
}

//------------------------------------------------------------
//フォントボールドの取得
int GetFontBold(void)
{
	return font_bold;
}

//------------------------------------------------------------
//水平フォントサイズ補正を設定
void SetFontHalf(int flag)
{
	font_half = flag;
	if (flag > 0)
		FONT_WIDTH = (ascii_data.width+flag) / (flag+1) + char_Margin;
	else if (flag == 0)
		FONT_WIDTH = ascii_data.width + char_Margin;
	else
		FONT_WIDTH = ascii_data.width * (-flag+1) + char_Margin;
	return;
}

//------------------------------------------------------------
//水平フォントサイズ補正値の取得
int GetFontHalf(void)
{
	return font_half;
}

//------------------------------------------------------------
//垂直フォントサイズ補正を設定
void SetFontVHalf(int flag)
{
	font_vhalf = flag;
	if (flag > 0)
		FONT_HEIGHT = (ascii_data.height+flag) / (flag+1) + line_Margin;
	else if (flag == 0)
		FONT_HEIGHT = ascii_data.height + line_Margin;
	else
		FONT_HEIGHT = ascii_data.height * (-flag+1) + line_Margin;
	return;
}

//------------------------------------------------------------
//垂直フォントサイズ補正値の取得
int GetFontVHalf(void)
{
	return font_vhalf;
}

//------------------------------------------------------------
//FONTX2ファイルのヘッダチェック
int checkFONTX2header(const char *path)
{
	//char *buf;
	int fd, size=0;
	char fullpath[MAX_PATH], tmp[MAX_PATH], *p;
	char buf[32];
	FONTX_HEADER *fontx_header;

	strcpy(fullpath,path);

	if(!strncmp(fullpath, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &path[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			fileXioUmount("pfs0:");
			goto error;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		if (!size){
			fileXioClose(fd);
			fileXioUmount("pfs0:");
			goto error;
		}
		fileXioLseek(fd, 0, SEEK_SET);
		//buf = (char*)malloc(32);
		fileXioRead(fd, buf, 32);
		fileXioClose(fd);
		fileXioUmount("pfs0:");
	}
	else if(!strncmp(fullpath, "mc", 2) || !strncmp(fullpath, "mass", 4) || !strncmp(fullpath, "cdfs", 4)) {
		if ((fd = fioOpen(fullpath, O_RDONLY)) < 0) 
			goto error;
		size = fioLseek(fd, 0, SEEK_END);
		if (!size){
			fioClose(fd);
			goto error;
		}
		fioLseek(fd, 0, SEEK_SET);
		//buf = (char*)malloc(32);
		fioRead(fd, buf, 32);
		fioClose(fd);
	}
	else {
		return 0;
	}

	//ヘッダのポインタ
	fontx_header = (FONTX_HEADER*)buf;

	//ヘッダチェック
	if(strncmp(fontx_header->Identifier, "FONTX2", 6)!=0){
		if(tek_getsize(buf)<0){
			//free(buf);
			goto error;
		}
	}

	//free(buf);
	return 1;
error:
	return -1;
}

//-------------------------------------------------
//半角文字の表示
void drawChar(unsigned char c, int x, int y, uint64 color)
{
	unsigned char *pc;

	//初期化していないか、初期化失敗している
	if(!init_ascii) return;

	//半角スペースのときは、何もしない
	if(c==' ') return;

	pc = &font_ascii[ascii_data.offset + c * ascii_data.size];

	if (fonthalfmode == 0) 
		drawChar_1bpp(pc, x, y, color, ascii_data.width, ascii_data.height);
	else if (fonthalfmode == 1)
		drawChar_bilinear(pc, x, y, color, ascii_data.width, ascii_data.height);
	else if (fonthalfmode == 2)
		drawChar_filter(pc, x, y, color, ascii_data.width, ascii_data.height);
}

//-------------------------------------------------
//全角文字の表示
void drawChar_SJIS(unsigned int c, int x, int y, uint64 color)
{
	FONTX_HEADER *fontx_header_kanji;
	int ret, sum, a, i;
	void *pc;
	
	//初期化していないか、初期化失敗している
	if(!init_kanji) return;

	//ヘッダのポインタ
	fontx_header_kanji = (FONTX_HEADER*)font_kanji;

	//何番目のテーブルにあるか調べる
	ret=-1;
	for(i=0;i<kanji_data.Tnum;i++){
		if((fontx_header_kanji->Block[i].Start <= c) && (fontx_header_kanji->Block[i].End >= c)){
			ret=i;
			break;
		}
	}
	//見つからないときは、なにもしない
	if (ret==-1) return;

	//アドレス算出
	sum = 0;
	for(i=0;i<ret;i++){
		sum += fontx_header_kanji->Block[i].End - fontx_header_kanji->Block[i].Start;
	}

	//
	a = sum + ret + ( c - fontx_header_kanji->Block[ret].Start );
	pc = &font_kanji[kanji_data.offset + a * kanji_data.size];
	//cc = *pc;

	if (fonthalfmode == 0) 
		drawChar_1bpp(pc, x, y, color, kanji_data.width, kanji_data.height);
	else if (fonthalfmode == 1)
		drawChar_bilinear(pc, x, y, color, kanji_data.width, kanji_data.height);
	else if (fonthalfmode == 2)
		drawChar_filter(pc, x, y, color, kanji_data.width, kanji_data.height);
}

//-------------------------------------------------
// 1bppビットマップフォントの描画(高速モード用)(幅64ドットまで)
void drawChar_1bpp(void *src, int x, int y, uint64 color, int w, int h)
{
	int	i, j;
	//unsigned char cc;
	unsigned char *pc, *cp;
	int bts, xl, xw, bty, xp, yp, n;
	uint64 msks, msk, cc, color2;
	unsigned int rpx, rpy;
	int xxl, xxr, yy;
	int wb=(w+7)>>3;	// 1バイトアライン
	pc = (unsigned char*) src;
	
	color2 = half(setting->color[COLOR_BACKGROUND], color, setting->flicker_alpha<<1);
	//	msk:	マスクビット
	//	bts:	シフトビット数
	//	rpx:	水平倍率
	// GetFontHalf() < 0: 拡大 (-n 倍)
	//				 = 0: 標準 (等倍)
	//				 > 0: 縮小 (1/(n+1)倍)
	msk = 0x8000000000000000;
	if (GetFontHalf() == 0) {
		// 標準
		bts = 1; rpx = 1;
	} else if (GetFontHalf() > 0) {
		// 縮小
		bts = GetFontHalf() + 1; rpx = 1;
		for (i = 1; i < bts; i++) msk |= 0x8000000000000000 >> i;
	} else {
		bts = 1; rpx = -GetFontHalf() + 1;
	}
	msks = msk;
	if (GetFontVHalf() == 0) {
		// 標準
		rpy = 1;	// rpy: リピートライン数(1:標準)
		bty = 1;	// bty: スキップライン数(1:標準)
	} else if (GetFontVHalf() > 0) {
		// 縮小
		rpy = 1; bty = GetFontVHalf()+1;
	} else {
		rpy = -GetFontVHalf()+1; bty = 1;
	}
	for(i=0,yp=0; i<h; i+=bty,yp++) {
		// i: 標準時, yp: 拡大時の垂直位置
		// 水平64ドット以内なら1回で読み込みを行う
		if (bty>1) {
			cc = 0;
			if (w > 32) {
				for(j = 0; j < bty; j++){
					if (i+j >= h) break;
					cp = pc+j*wb;
					cc |= ((uint64) cp[0] << 56)|((uint64) cp[1] << 48)|((uint64) cp[2] << 40)|((uint64) cp[3] << 32)|
							((uint64) cp[4] << 24)|((uint64) cp[5] << 16)|((uint64) cp[6] << 8)|((uint64) cp[7]);
				}
			} else {
				for(j = 0; j < bty; j++){
					if (i+j >= h) break;
					cp = pc+j*wb;
					cc |= ((uint64) cp[0] << 56)|((uint64) cp[1] << 48)|((uint64) cp[2] << 40)|((uint64) cp[3] << 32);
				}
			}
		} else if (w > 32) {
			cc = ((uint64) pc[0] << 56)|((uint64) pc[1] << 48)|((uint64) pc[2] << 40)|((uint64) pc[3] << 32)|
					((uint64) pc[4] << 24)|((uint64) pc[5] << 16)|((uint64) pc[6] << 8)|((uint64) pc[7]);
		} else {
			cc = ((uint64) pc[0] << 56)|((uint64) pc[1] << 48)|((uint64) pc[2] << 40)|((uint64) pc[3] << 32);
		}
		// 描画開始
		xl = -1; xw = 0;
		if (setting->flickerControl) {
			// フリッカーコントロールが有効の場合
			for(j=0,xp=0; j<w; j+=bts,xp++) {
				if (cc & msk) {
					if (xl < 0) xl = xp;
					xw++;
				} else if (xl >= 0) {
					if ((xw > 1) || setting->FontBold || (rpx > 1)) {
						xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+setting->FontBold;
						if (rpy > 1) {
							yy = y+i*rpy;
							for (n = 0; n < rpy; n++) {
								itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
								itoLine(color2, xxl, yy+n+1, 0, color2, xxr, yy+n+1, 0);
							}
						} else {
							itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
							itoLine(color2, xxl, y+yp+1, 0, color2, xxr, y+yp+1, 0);
						}
					} else {
						if (rpy > 1) {
							xxl = x+xl; yy = y+i*rpy;
							itoLine(color, xxl, yy, 0, color, xxl, yy+rpy, 0);
							itoPoint(color2, xxl, yy+rpy, 0);
						} else {
							itoPoint(color, x+xl, y+yp, 0);
							itoPoint(color2, x+xl, y+yp+1, 0);
						}
					}
					xl = -1; xw = 0;
				}
				msk = msk >> bts;
			}
			if (xw > 0) {
				if ((xw > 1) || setting->FontBold || (rpx > 1)) {
					xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+setting->FontBold;
					if (rpy > 1) {
						yy = y+i*rpy;
						for (n = 0; n < rpy; n++) {
							itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
							itoLine(color2, xxl, yy+n+1, 0, color2, xxr, yy+n+1, 0);
						}
					} else {
						itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
						itoLine(color2, xxl, y+yp+1, 0, color2, xxr, y+yp+1, 0);
					}
				} else {
					if (rpy > 1) {
						xxl = x+xl; yy = y+i*rpy;
						itoLine(color, xxl, yy, 0, color, xxl, yy+rpy, 0);
						itoPoint(color2, xxl, yy+rpy, 0);
					} else {
						itoPoint(color, x+xl, y+yp, 0);
						itoPoint(color2, x+xl, y+yp+1, 0);
					}
				}
			}
		} else {
			// 通常時
			for(j=0,xp=0; j<w; j+=bts,xp++) {
				if (cc & msk) {
					if (xl < 0) xl = xp;
					xw++;
				} else if (xl >= 0) {
					if ((xw > 1) || setting->FontBold || (rpx > 1)) {
						xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+setting->FontBold;
						if (rpy > 1) {
							yy = y+i*rpy;
							for (n = 0; n < rpy; n++)
								itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
						} else
							itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
					} else {
						if (rpy > 1)
							itoLine(color, x+xl, y+i*rpy, 0, color, x+xl, y+i*rpy+rpy, 0);
						else
							itoPoint(color, x+xl, y+yp, 0);
					}
					xl = -1; xw = 0;
				}
				msk = msk >> bts;
			}
			if (xw > 0) {
				if ((xw > 1) || setting->FontBold || (rpx > 1)) {
					xxl = x+xl*rpx; xxr = x+(xl+xw)*rpx+setting->FontBold;
					if (rpy > 1) {
						yy = y+i*rpy;
						for (n = 0; n < rpy; n++)
							itoLine(color, xxl, yy+n, 0, color, xxr, yy+n, 0);
					} else
						itoLine(color, xxl, y+yp, 0, color, xxr, y+yp, 0);
				} else {
					if (rpy > 1)
						itoLine(color, x+xl, y+i*rpy, 0, color, x+xl, y+i*rpy+rpy, 0);
					else
						itoPoint(color, x+xl, y+yp, 0);
				}
			}
		}
		pc+= wb * bty;
		msk = msks;
	}
	return;
}

//-------------------------------------------------
void drawChar_filter(void *src, int x, int y, uint64 color, int w, int h)
{
	int dw, dh;
	int i,j;
	
	i = -GetFontHalf();
	j = -GetFontVHalf();
	if ((i == 0) && (j == 0) && (w <= 64)) {
		drawChar_1bpp(src, x, y, color, w, h);
		return;
	}
	if (i > 0)	dw = (i+1)*w;
	else if (i < 0)	dw = w/(-i+1);
	else		dw = w;
	if (j > 0)	dh = (j+1)*h;
	else if (j < 0)	dh = h/(-j+1);
	else		dh = h;
	
	if (dw*dh <= tmpbuffersize) {
	} else
		drawChar_1bpp(src, x, y, color, w, h);
}

void drawChar_bilinear(void *src, int x, int y, uint64 color, int w, int h)
{
	int dw, dh;
	int i,j;
	
	i = -GetFontHalf();
	j = -GetFontVHalf();
	if ((i == 0) && (j == 0) && (w <= 64)) {
		drawChar_1bpp(src, x, y, color, w, h);
		return;
	}
	if (i > 0)	dw = (i+1)*w;
	else if (i < 0)	dw = w/(-i+1);
	else		dw = w;
	if (j > 0)	dh = (j+1)*h;
	else if (j < 0)	dh = h/(-j+1);
	else		dh = h;
	
	if (dw*dh <= tmpbuffersize) {
		drawChar_unpack(bmpsrc, src, w, h);
		if ((i != 0) || (j != 0)) {
			drawChar_resize(bmpdst, bmpsrc, dw, dh, w, h);
			drawChar_8bpp(bmpdst, x, y, color, setting->color[COLOR_BACKGROUND], dw, dh);
		} else 
			drawChar_8bpp(bmpsrc, x, y, color, setting->color[COLOR_BACKGROUND], w, h);
	} else
		drawChar_1bpp(src, x, y, color, w, h);
}

//-------------------------------------------------
// 8bppビットマップフォントの描画(高画質モード用)
void drawChar_8bpp(void *src, int x, int y, uint64 color, uint64 back, int w, int h)
{
	int i,j;
	unsigned char *pc;
	pc = src;
	
	// 高速化は後回し
	for(i=0;i<h;i++)
		for(j=0;j<w;j++)
			if (*pc > 0)
				itoPoint(half(back, color, *pc++*2), x+j, y+i, 0);
			else
				pc++;
}

unsigned char pget8bpp(unsigned char *src, int x, int y, int w, int h)
{
	if ((x >= 0) && (y >= 0) && (x < w) && (y < h))
		return src[y*w+x];
	return 0;
}
void pset8bpp(unsigned char *dst, int x, int y, unsigned char c, int w, int h)
{
	if ((x >= 0) && (y >= 0) && (x < w) && (y < h))
		dst[y*w+x] = c;
}
//-------------------------------------------------
// バイリニアリサイズ
void drawChar_resize(void *dist, void *src, int dw, int dh, int sw, int sh)
{
	int xy, x, y, z, b, l, r;
	unsigned char *s, *d;
	unsigned char yx[8];
	s = src; d = dist;
	// 1st pass: 水平方向のリサイズ(整数倍拡大/縮小限定)
	if (sw > dw) {
		// 縮小
		xy = sw / dw;
		for (y=0; y<sh; y++)
			for (x=0; x<dw; x++) {
				b = 0;
				for (z=0; z<xy; z++)
					b += pget8bpp(s, x*xy+z, y, sw, sh);
				pset8bpp(d, x, y, b/xy, dw, sh);
			}
	} else if (sw < dw) {
		// 拡大
		xy = dw / sw;
		for (z=0; z<xy; z++)
			yx[z] = (z<<8)/xy;
		for (y=0; y<sh; y++)
			for (x=0; x<sw; x++) {
				l = pget8bpp(s, x, y, sw, sh);
				r = pget8bpp(s, x+1, y, sw, sh);
				for (z=0; z<xy; z++) {
					b = (l*(256-yx[z]) + r*yx[z])>>8;
					pset8bpp(d, x*xy+z, y, b, dw, sh);
				}
			}
	} else {
		// そのまま
		for (y=0; y<sh; y++)
			for (x=0; x<sw; x++)
				pset8bpp(d, x, y, pget8bpp(s, x, y, sw, sh), dw, sh);
	}
	// 2nd pass: 垂直方向のリサイズ
	if (sh > dh) {
		// 縮小
		xy = sh / dh;
		for (y=0; y<dh; y++)
			for (x=0; x<dw; x++) {
				b = 0;
				for (z=0; z<xy; z++)
					b += pget8bpp(d, x, y*xy+z, dw, sh);
				pset8bpp(d, x, y, b/xy, dw, dh);
			}
	} else if (sh < dh) {
		// 拡大
		xy = dh / sh;
		for (z=0; z<xy; z++)
			yx[z] = (z<<8)/xy;
		for (y=sh-1; y>=0; y--)
			for (x=0; x<dw; x++) {
				l = pget8bpp(d, x, y, dw, sh);
				r = pget8bpp(d, x, y+1, dw, sh);
				for (z=0; z<xy; z++) {
					b = (l*(256-yx[z]) + r*yx[z])>>8;
					pset8bpp(d, x, y*xy+z, b, dw, dh);
				}
			}
	}
}

//-------------------------------------------------
// 1bppビットマップフォントのデコード(フォントの高画質描画用)
void drawChar_unpack(void *dist, void *src, int w, int h)
{
	int i,j;
	unsigned char msk, msks, *s, *d;
	msks = 0x80;
	for (j=0;j<h;j++){
		s = src + ((w+7)>>3)*j;
		d = dist + w*j;
		msk = msks;
		for (i=0;i<w;i++){
			if (!msk) {
				msk = msks;
				s++;
			}
			if (*s & msk)
				d[i] = 0x80;
			else
				d[i] = 0x00;
			msk >>= 1;
		}
	}
		
	return;
}

//-------------------------------------------------
// 座標を指定して文字列を表示
int printXY(const unsigned char *s, int x, int y, uint64 color, int draw)
{
	uint64 color0,color1,color2;	//アルファ付き
	uint16 code;
	int i;

	color0 = color&0x00FFFFFF;	//透明度を除外
	color1 = color0|0x80000000;	//不透明
	color2 = color0|(setting->flicker_alpha << 24);	//半透明
	//color2 = half(color0, setting->color[COLOR_BACKGROUND], 0x80);

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xfc )){	//SJIS
			code = s[i++];
			code = (code<<8) + s[i++];
			if(draw){
				//if ((x+(FONT_WIDTH-char_Margin)*2+kanji_MarginLeft >= 0) && (x < SCREEN_WIDTH)) {
				if ((x >= 0) && (x < SCREEN_WIDTH))
					drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, color1);
			}
			if (font_half > 0)
				x += (kanji_data.width+font_half) / (font_half+1) + char_Margin * 2;
			else if (font_half == 0)
				x += kanji_data.width + char_Margin * 2;
			else
				x += kanji_data.width * (-font_half+1) + char_Margin * 2;
		}
		else{
			if(draw && (x >= 0) && (x < SCREEN_WIDTH))
				drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, color1);
			i++;
			if (font_half > 0)
				x += (ascii_data.width+font_half) / (font_half+1) + char_Margin;
			else if (font_half == 0)
				x += ascii_data.width + char_Margin;
			else
				x += ascii_data.width * (-font_half+1) + char_Margin;
		}
	}

	return x;
}

#ifdef ENABLE_ICON
//-------------------------------------------------
void loadIcon(void)
{
	itoLoadIIF(icon_iif, 0, 256, 0, 0);

	return;
}

//-------------------------------------------------
int drawIcon(int x, int y, int w, int h, int id)
{
	//アルファブレンド有効
	itoPrimAlphaBlending( TRUE );

	itoSetTexture(0, 256, ITO_RGBA32, ITO_TEXTURE_256, ITO_TEXTURE_32);
	itoTextureSprite(
		ITO_RGBA(0x80,0x80,0x80,0xFF),
		x, y,
		id*16, 0, 
		x+16*w/16, y+16*h/16,
		id*16+16, 16,
		0);

	//アルファブレンド無効
	itoPrimAlphaBlending( FALSE );

	return 0;
}
#endif

/*
//int CurrentPos_x;	//カレントポジションx
//int CurrentPos_y;	//カレントポジションy

//------------------------------------------------------------
//カレントポジションを設定
int SetCurrentPos(int x, int y)
{
	CurrentPos_x = x;
	CurrentPos_y = y;
	return 0;
}

//------------------------------------------------------------
//カレントポジションを取得
int GetCurrentPos(int type)
{
	if(type==CURRENTPOS_X)
		return CurrentPos_x;
	if(type==CURRENTPOS_Y)
		return CurrentPos_y;
	return 0;
}

//------------------------------------------------------------
// カレントポジションの座標に文字列を表示
int printXY2(const unsigned char *s, uint64 color, int draw)
{
	uint64 color0,color1,color2;	//アルファ付き
	uint16 code;
	int i;
	int x,y;

	x=CurrentPos_x;
	y=CurrentPos_y;

	color0 = color&0x00FFFFFF;	//透明度を除外
	color1 = color0|0x80000000;	//不透明
	color2 = color0|(setting->flicker_alpha << 24);	//半透明
	//color2 = half(color0, setting->color[COLOR_BACKGROUND], 0x80);

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xff )){	//SJIS
			code = s[i++];
			code = (code<<8) + s[i++];
			if(draw){
				if(setting->flickerControl){
					//アルファブレンド有効
					itoPrimAlphaBlending( TRUE );
					drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop+1, color2);
					//アルファブレンド無効
					itoPrimAlphaBlending( FALSE );
				}
				drawChar_SJIS(code, x+kanji_MarginLeft, y+kanji_MarginTop, color1);
			}
			x += kanji_data.width + char_Margin * 2;
		}
		else{
			if(draw){
				if(setting->flickerControl){
					//アルファブレンド有効
					itoPrimAlphaBlending( TRUE );
					drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop+1, color2);
					//アルファブレンド無効
					itoPrimAlphaBlending( FALSE );
				}
				drawChar(s[i], x+ascii_MarginLeft, y+ascii_MarginTop, color1);
			}
			i++;
			x += ascii_data.width + char_Margin;
		}
	}

	CurrentPos_y += FONT_HEIGHT;

	return x;
}
*/
