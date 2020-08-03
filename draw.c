#include "launchelf.h"

#define LBF_VER "LbF v0.43"

itoGsEnv screen_env;

int initbiosfont=0;
char *biosfont=NULL;

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

//------------------------------------------------------------
int InitBIOSFont(void)
{
	int fd=0;
	size_t size;
	int ret=0;

	//すでにロードしている
	if(initbiosfont) return 0;

	//フォントファイルオープン
	fd = fioOpen("rom0:KROM", O_RDONLY);
	if(fd<0){
		ret=-1;
		goto error;
	}

	//サイズを調べる
	size = fioLseek(fd,0,SEEK_END);
	fioLseek(fd,0,SEEK_SET);	//シークを0に戻す

	//メモリを確保
	biosfont = (char*)malloc(size);
	if(biosfont==NULL){
		ret=-2;
		goto error;
	}
	
	//メモリに読み込む
	fioRead(fd, biosfont, (size_t)size);

	//フォントロード成功
	initbiosfont=1;

error:
	if(fd>0) fioClose(fd);
	return ret;
}

//------------------------------------------------------------
void FreeBIOSFont(void)
{
	free(biosfont);
	initbiosfont=0;
}

//------------------------------------------------------------
// 暗くする(半透明の黒い四角)
void drawDark(void)
{
	//アルファブレンド有効
	itoPrimAlphaBlending( TRUE );
	//
	itoSprite(ITO_RGBA(0,0,0,0x10),
		FONT_WIDTH*1.5, SCREEN_MARGIN+FONT_HEIGHT*2.5,
		FONT_WIDTH*62.5, SCREEN_MARGIN+FONT_HEIGHT*19.5, 0);
	//アルファブレンド無効
	itoPrimAlphaBlending(FALSE);
}

//------------------------------------------------------------
// ダイアログの背景
void drawDialogTmp(int x1, int y1, int x2, int y2, uint64 color1, uint64 color2)
{
	//
	itoSprite(color1, x1, y1, x2, y2, 0);
	drawFrame(x1+2, y1+2, x2-2, y2-2, color2);

}
////////////////////////////////////////////////////////////////////////
// 画面表示のテンプレート
void setScrTmp(const char *msg0, const char *msg1)
{
	// バージョン表記
	printXY(LBF_VER, FONT_WIDTH*53, SCREEN_MARGIN, setting->color[3], TRUE);
	
	// メッセージ
	printXY(msg0, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*1, setting->color[3], TRUE);
	
	// 枠
	drawFrame(FONT_WIDTH*1.5, SCREEN_MARGIN+FONT_HEIGHT*2.5,
		FONT_WIDTH*62.5, SCREEN_MARGIN+FONT_HEIGHT*19.5,
		setting->color[1]);
	
	// 操作説明
	printXY(msg1, FONT_WIDTH*2, SCREEN_MARGIN+FONT_HEIGHT*20, setting->color[3], TRUE);
}

////////////////////////////////////////////////////////////////////////
// メッセージ描画
void drawMsg(const char *msg)
{
	itoSprite(setting->color[0], 0, SCREEN_MARGIN+FONT_HEIGHT,
		SCREEN_WIDTH, SCREEN_MARGIN+FONT_HEIGHT*2, 0);
	printXY(msg, SCREEN_MARGIN, SCREEN_MARGIN+FONT_HEIGHT,
		setting->color[3], TRUE);
	drawScr();
}

////////////////////////////////////////////////////////////////////////
// setup ito
void setupito(void)
{
	itoInit();

	// screen resolution
	screen_env.screen.width		= 640;
	screen_env.screen.height	= 448;
	screen_env.screen.psm		= ITO_RGBA32;

	// These setting work best with my tv, experiment for youself
	screen_env.screen.x			= setting->screen_x; 
	screen_env.screen.y			= setting->screen_y;
	
	screen_env.framebuffer1.x	= 0;
	screen_env.framebuffer1.y	= 0;
	
	screen_env.framebuffer2.x	= 0;
	screen_env.framebuffer2.y	= 448;

	// zbuffer
	screen_env.zbuffer.x		= 0;
	screen_env.zbuffer.y		= 448*2;
	screen_env.zbuffer.psm		= ITO_ZBUF32;
	
	// scissor 
	screen_env.scissor_x1		= 0;
	screen_env.scissor_y1		= 0;
	screen_env.scissor_x2		= 640;
	screen_env.scissor_y2		= 448;
	
	// misc
	screen_env.dither			= TRUE;
	screen_env.interlace		= ITO_INTERLACE;
	screen_env.ffmode			= ITO_FIELD;
	screen_env.vmode			= ITO_VMODE_AUTO;
	
	itoGsEnvSubmit(&screen_env);

	//アルファブレンド
	itoSetAlphaBlending(
		ITO_ALPHA_COLOR_SRC, // A = COLOR SOURCE
		ITO_ALPHA_COLOR_DST, // B = COLOR DEST
		ITO_ALPHA_VALUE_SRC, // C = ALPHA VALUE SOURCE
		ITO_ALPHA_COLOR_DST, // C = COLOR DEST
		0x80);				 // Fixed Value
	//
	itoSetBgColor(setting->color[0]);
}

////////////////////////////////////////////////////////////////////////
// 画面のクリア
void clrScr(uint64 color)
{
	itoSprite(color, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

////////////////////////////////////////////////////////////////////////
// 画面の描画
void drawScr(void)
{
	itoGsFinish();
	itoVSync();
	itoSwitchFrameBuffers();
}

////////////////////////////////////////////////////////////////////////
// 枠の描画
void drawFrame(int x1, int y1, int x2, int y2, uint64 color)
{
	uint64 color2;	//アルファ付き

	color = color&0x00FFFFFF;	//透明度を除外
	color = color|0x80000000;	//不透明
	color2 = color|0x10000000;	//半透明

	//FLICKER CONTROL: ON
	if(setting->flickerControl){
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
	itoLine(color, x1, y1, 0, color, x2, y1, 0);
	//右の縦線
	itoLine(color, x2, y1, 0, color, x2, y2, 0);	
	//下の横線
	itoLine(color, x2, y2, 0, color, x1, y2, 0);
	//左の縦線
	itoLine(color, x1, y2, 0, color, x1, y1, 0);
}

////////////////////////////////////////////////////////////////////////
//半角文字の表示
void drawChar(unsigned char c, int x, int y, uint64 color)
{
	unsigned int i, j;
	unsigned char cc;
	unsigned char *pc;
	uint64 color2;

	//初期化していないか、初期化失敗している
	if(!initbiosfont) return;

	//半角スペースのときは、何もしない
	if(c==' ') return;

	color = color&0x00FFFFFF;	//透明度を除外
	color = color|0x80000000;	//不透明
	color2 = color|0x10000000;	//半透明

	pc = &biosfont[104670+(c-33)*15];
	cc = *pc;
	//
	for(i=0; i<16; i++){
		for(j=0; j<8; j++){
			if(cc & 0x80){
				if(setting->flickerControl){
					//itoPoint(color2, x+j, y+i+1, 0);
					//itoPoint(color2, x+j+1, y+i+1, 0);	//太字にする
					itoLine(color2, x+j, y+i+1, 0, color2, x+j+2, y+i+1, 0);
				}
				//itoPoint(color, x+j, y+i, 0);
				//itoPoint(color, x+j+1, y+i, 0);	//太字にする
				itoLine(color, x+j, y+i, 0, color, x+j+2, y+i, 0);
			}
			cc = cc << 1;
		}
		cc = *pc++;
	}
}

////////////////////////////////////////////////////////////////////////
//全角文字の表示
void drawChar_SJIS(unsigned int c, int x, int y, uint64 color)
{
	int i, j, a;
	int ret, sum;
	unsigned char cc;
	unsigned char *pc;
	uint64 color2;

	//初期化していないか、初期化失敗している
	if(!initbiosfont) return;

	//何番目のブロックにあるか調べる
	ret=-1;
	for(i=0;i<51;i++){
		if((font_sjis_table[i*2] <= c) && (font_sjis_table[i*2+1] >= c)){
			ret=i;
			break;
		}
	}
	//見つからないときは、なにもしない
	if (ret==-1) return;

	color = color&0x00FFFFFF;	//透明度を除外
	color = color|0x80000000;	//不透明
	color2 = color|0x10000000;	//半透明

	//アドレス算出
	sum = 0;
	for(i=0;i<ret;i++){
		sum += font_sjis_table[i*2+1] - font_sjis_table[i*2];
	}

	//
	a = (sum + ret + ( c - font_sjis_table[ret*2] ) ) * 30;
	pc = &biosfont[a];                                

	//
	for(i=0; i<15; i++) {
		//左半分
		cc = *pc++;
		for(j=0; j<8; j++) {
			if(cc & 0x80){
				if(setting->flickerControl){
					//itoPoint(color2, x+j, y+i+1, 0);
					//itoPoint(color2, x+j+1, y+i+1, 0);	//太字にする
					itoLine(color2, x+j, y+i+1, 0, color2, x+j+2, y+i+1, 0);
				}
				//itoPoint(color, x+j, y+i, 0);
				//itoPoint(color, x+j+1, y+i, 0);	//太字にする
				itoLine(color, x+j, y+i, 0, color, x+j+2, y+i, 0);
			}
			cc = cc << 1;
		}
		//右半分
		cc = *pc++;
		for(j=0; j<8; j++) {
			if(cc & 0x80){
				if(setting->flickerControl){
					//itoPoint(color2, x+8+j, y+i+1, 0);
					//itoPoint(color2, x+8+j+1, y+i+1, 0);	//太字にする
					itoLine(color2, x+8+j, y+i+1, 0, color2, x+8+j+2, y+i+1, 0);
				}
				//itoPoint(color, x+8+j, y+i, 0);
				//itoPoint(color, x+8+j+1, y+i, 0);	//太字にする
				itoLine(color, x+8+j, y+i, 0, color, x+8+j+2, y+i, 0);
			}
			cc = cc << 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// draw a string of characters
int printXY(const unsigned char *s, int x, int y, uint64 color, int draw)
{
	uint16 code;
	int i;

	//FLICKER CONTROL: ON
	if(setting->flickerControl)
		itoPrimAlphaBlending( TRUE );	//アルファブレンド有効

	i=0;
	while(s[i]){
		if (( s[i]>=0x81 && s[i]<=0x9f ) || ( s[i]>=0xe0 && s[i]<=0xff )){
			code = s[i++];
			code = (code<<8) + s[i++];
			drawChar_SJIS(code, x, y, color);
			x += FONT_WIDTH*2;
		}           
		else{
			if(draw) drawChar(s[i], x, y, color);
			i++;
			x += FONT_WIDTH;
		}
/*
		if(x > SCREEN_WIDTH-SCREEN_MARGIN-FONT_WIDTH){
			//x=16; y=y+8;
			return x;
		}
*/
	}

	itoPrimAlphaBlending(FALSE);	//アルファブレンド無効

	return x;
}
