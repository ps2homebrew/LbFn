// ◯ 
#include "launchelf.h"

//-------------------------------------------------
// 新スクリーンキーボード
/*
	レイアウト案A
 +----+------------------------+----+
 |←L2| (編集欄)               |R2→|
 +----+------------------------+----+
      | あいうえお  かきくけこ ^L1
      |                        |
      |                        vR1
      +------------------------+
 
	レイアウト案B
 +-----------------------------------+
 | (編集欄)                          |
 +---------~-------------------------+
 +----------+ +----------------------+
 | > 英記号 | | あいうえおかきくけこ | ASCII
 |   平仮名 | | 　　　　　　　　　　 | hira
 |   片仮名 | | 　　　　　　　　　　 | kata
 |   漢字   | | 　　　　　　　　　　 | kanji
 |   漢字歴 | | 　　　　　　　　　　 | history
 |   完了   | | 　　　　　　　　　　 | OK
 +----------+ +----------------------+
 
	レイアウト案C
 +-----------------------+
 | (編集欄)              |
 |-----------------------|
 | 文字種 戻る 進む 終了 | type back frwd OK
 | あいうえお かきくけこ |
 |                       |
 +-----------------------+
 
	レイアウト案D
 +--------------------------------+
 | (編集欄)                       |
 |--------------------------------|
 | あ い う え お  か き く け こ |
 |                                |
 | 決定  中止  文字種  戻す  進む |
 +--------------------------------+
*/
// ○:決定 ×:削除 L1:左へ R1:右へ L2:種類へ R2:文字へ
/*
	ソフトキーボードの各データの扱い

# ソフトキーボード用セッション
[virtual keyboard]
# history:漢字履歴用。最大80文字くらいあれば良いか。
history=11535,11535,11535,11535,11535,11535,11535,11535,11535
# custom:カスタマイズ用。単一のページのみ。最大500文字くらいだろうか
custom=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20
# addpage:ページ拡張用。複数入力不可。
addpage=mc:/SYS-CONF/skbdext.dat

*/
enum{
	SKBD_LENGTH=40,
	SKBD_WIDTH=44,
	SKBD_ITEMS=8,
	SKBD_LEFT=12,
	SKBD_HEIGHT=SKBD_ITEMS+4,
};

enum{
	PG_ALPHABET=0x01,
	PG_HIRAGANA,
	PG_KATAKANA,
	PG_NUMBER,
	PG_CUSTOM,
	PG_INDEXKANJI=0x10,
	PG_KANJI,
	PG_HISTORYKANJI=0x0F,
	PG_INDEXKANJI2=0x40,
	PG_KANJI2,
	PG_RAW=0x70,
	PG_EXTEND=0xD0,
	PG_INDEXDBCS=0xD0,
	PG_DBCS,
	PG_ENTER=0x080000,
	PG_ABORT=0x080001,
	MAX_PAGES=16,
	MAX_CHARS=8000,
	MAX_PAGENUM=0x200,
	CUR_NORMAL=0,
	CUR_ENGLISH,
	CUR_LINE,
};
// ページ数
//	訓読み漢字	44+1	3D	40
//	音読み漢字	44+1	3D	40
//	文字コード	94+1	5F	60
//	平片数記英   5      05	05
//	履歴         1      01	01
//	ユーザー     1      01	01
//	───────────
//  合計       192      E0	E7
//	残り        64      40	39

/* [C:\osask\nask\oscspeed\speed.org] 00003B30-00003BDF */
static unsigned short kanjimap[45] = {
	0x0582, 0x05AB, 0x05E5,	0x0600, 0x0636, 
	0x0659, 0x0744, 0x07FA,	0x0828, 0x0894, 
	0x0940, 0x09B0, 0x0B3D,	0x0B65, 0x0BD8, 
	0x0C3B, 0x0CA8, 0x0CFE,	0x0D1B, 0x0D5D, 
	0x0DD5, 0x0DE8, 0x0DFB,	0x0DFC, 0x0E08, 
	0x0E16, 0x0E8C, 0x0EE3,	0x0F27, 0x0F4A, 
	0x0FAB, 0x0FCC, 0x0FDC,	0x0FE6, 0x0FF7, 
	0x1012, 0x1024, 0x1044,
	0x106C, 0x107F, 0x10C1, 0x10C6, 0x10E7,
	0x1104, 0x1116
};
static unsigned short convmap[50] = {
	0x179,0x17B,0x17D,0x17F,0x181,	0x182,0x184,0x186,0x188,0x18A,
	0x18C,0x18E,0x190,0x192,0x194,	0x196,0x198,0x19B,0x19D,0x19F,
	0x1A1,0x1A2,0x1A3,0x1A4,0x1A5,	0x1A6,0x1A9,0x1AC,0x1AF,0x1B2,
	0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,	0x1BB,0x1BD,0x1BF,0x000,0x000,
	0x1C0,0x1C1,0x1C2,0x1C3,0x1C4,	0x1C6,0x000,0x000,0x000,0x000
};
static unsigned char convpage[50] = {
	0xB1,0xB2,0xB3,0xB4,0xB5, 0xB6,0xB7,0xB8,0xB9,0xBA,
	0xBB,0xBC,0xBD,0xBE,0xBF, 0xC0,0xC1,0xC2,0xC3,0xC4,
	0xC5,0xC6,0xC7,0xC8,0xC9, 0xCA,0xCB,0xCC,0xCD,0xCE,
	0xCF,0xD0,0xD1,0xD2,0xD3, 0xD4,0xD5,0xD6,0x00,0x00,
	0xD7,0xD8,0xD9,0xDA,0xDB, 0xDC,0x00,0x00,0x00,0x00
};
/* 訓読み漢字検索 by TrueMAZE(仮) */
static unsigned short kanjioffset[45] = {
	   0,  65, 107, 130, 138,
	 188, 240, 263, 290, 294,
	 321, 343, 369, 398, 401,
	 411, 453, 462, 495, 500,
	 527, 548, 554, 560, 565,
	 586, 629, 658, 679, 684,
	 696, 733, 754, 765, 770,
	 790, 811, 821,
	 837, 839, 842, 843, 844,
	 845, 857
};
static unsigned short kanjitable[858] = {
	0x0AEA,0x068E,0x0A63,0x0916,0x096C,0x0D8D,0x1034,0x0FE9,0x080A,0x0902,
	0x058E,0x06A6,0x0C09,0x072F,0x0B85,0x0B97,0x08F7,0x0DB1,0x0F5F,0x0AD4,
	0x0AC2,0x0ACD,0x0768,0x0AD2,0x10F2,0x0A34,0x0CE4,0x0BB6,0x0FAE,0x09E5,
	0x0C2A,0x0E7B,0x103E,0x1046,0x0F42,0x0A65,0x0656,0x0DA1,0x0B1A,0x08CD,
	0x0A5F,0x0A8A,0x085D,0x08D5,0x082E,0x09BB,0x0F88,0x0746,0x1026,0x0D4E,
	0x071E,0x0F25,0x10CE,0x100F,0x0959,0x0695,0x0F4F,0x0EA4,0x1079,0x0C07,
	0x088E,0x0A30,0x0586,0x0C93,0x0000,0x0DF3,0x05C7,0x0A77,0x0B79,0x0903,
	0x0750,0x0891,0x104D,0x07A6,0x105E,0x0665,0x0D73,0x0888,0x0B6B,0x0B6D,
	0x0BB0,0x0CAE,0x102D,0x08CA,0x0B91,0x0BB5,0x09D5,0x0E75,0x0D63,0x06C5,
	0x09BF,0x08AF,0x09E7,0x09D0,0x0587,0x0875,0x075E,0x0FE8,0x0930,0x0FCD,
	0x0699,0x0E8D,0x0B0A,0x0959,0x073D,0x0A72,0x0000,0x0C44,0x0B79,0x0DB5,
	0x0B05,0x0D99,0x07B4,0x1032,0x0AEA,0x07B2,0x07A4,0x09CC,0x066B,0x0DD7,
	0x0606,0x0EAF,0x0E22,0x08D9,0x069F,0x0935,0x0624,0x0BAB,0x0A4C,0x0000,
	0x0A6C,0x0DB5,0x0AD5,0x0608,0x0E3F,0x05AF,0x091A,0x0000,0x09B9,0x063A,
	0x0EA9,0x0A51,0x07A6,0x1075,0x1041,0x10FA,0x063E,0x10FD,0x0A96,0x0791,
	0x0BB1,0x08F3,0x0C64,0x07A7,0x0C3D,0x0643,0x08C9,0x0E79,0x078D,0x108B,
	0x064A,0x0644,0x063B,0x0D73,0x0888,0x0903,0x0CF2,0x0A4B,0x09EC,0x104B,
	0x07CA,0x07C7,0x0EE6,0x0652,0x0658,0x1060,0x0D28,0x0CA6,0x0DA6,0x0BFA,
	0x0585,0x0B29,0x0FA9,0x0653,0x0807,0x0A9E,0x07D8,0x0000,0x0C62,0x08C3,
	0x0E91,0x0F3E,0x0A36,0x0910,0x09DF,0x0AA6,0x091D,0x06D8,0x06A8,0x0E70,
	0x0768,0x1C91,0x0605,0x083D,0x0A6C,0x087E,0x0DA1,0x0B57,0x10B1,0x0671,
	0x0F06,0x0F6A,0x0F3F,0x0834,0x0D3F,0x0AE2,0x0D78,0x0BDD,0x06CC,0x1010,
	0x07F7,0x0E92,0x0586,0x105E,0x0EBB,0x0AE7,0x06F0,0x0B23,0x1072,0x080A,
	0x0B2C,0x0CF4,0x0C4B,0x065B,0x084D,0x0BAF,0x06D2,0x066C,0x0700,0x0000,
	0x0B9B,0x1003,0x107F,0x106F,0x085B,0x0648,0x07E0,0x0A42,0x0E8F,0x0738,
	0x0CB6,0x0F97,0x05C2,0x08F2,0x0684,0x0888,0x0B31,0x0822,0x0B77,0x0B7C,
	0x082D,0x079D,0x0000,0x074A,0x0BE7,0x106F,0x0847,0x0C0F,0x0EF4,0x0A38,
	0x094C,0x080F,0x101D,0x0962,0x08CE,0x0920,0x1046,0x05FF,0x0E97,0x05A6,
	0x0C1F,0x05AD,0x07CD,0x0A1F,0x08F7,0x0924,0x088D,0x0D4B,0x0660,0x0000,
	0x0FFC,0x0A6A,0x0882,0x0000,0x09BD,0x0AB3,0x092D,0x09E4,0x0CEF,0x061A,
	0x0FE5,0x10DD,0x0B23,0x066C,0x0D77,0x0EC8,0x0B17,0x09C1,0x0694,0x0891,
	0x07EC,0x0A40,0x1084,0x08D4,0x0966,0x05BE,0x0991,0x05C2,0x07C7,0x0692,
	0x0000,0x0659,0x10C9,0x0976,0x06CB,0x0CBE,0x08DB,0x0608,0x0B7A,0x07B2,
	0x0B60,0x07C0,0x0A3B,0x0D23,0x0B7C,0x08B8,0x0CAD,0x10A0,0x0B22,0x0A2D,
	0x09E3,0x0C71,0x0000,0x0BAB,0x09CB,0x0CA9,0x07C4,0x08DB,0x0635,0x09F8,
	0x09BE,0x0FF9,0x0E7C,0x0B86,0x0659,0x0B29,0x0667,0x0DF9,0x0A10,0x0A69,
	0x0D7E,0x0F98,0x0E45,0x0B0F,0x0764,0x05D6,0x0AF0,0x07F9,0x0000,0x08D4,
	0x0A62,0x0CF7,0x0C02,0x095E,0x0A4A,0x0D94,0x0FAC,0x0B62,0x0FC1,0x075F,
	0x0B5C,0x0AB4,0x102C,0x0AA6,0x0C6B,0x068D,0x0A9C,0x09DC,0x0865,0x07EF,
	0x0B2D,0x0AAE,0x0B60,0x094A,0x0BD3,0x0A78,0x0613,0x0000,0x08E5,0x10D2,
	0x0000,0x0E70,0x0991,0x0627,0x05CA,0x06AB,0x07B6,0x0630,0x0BA8,0x080A,
	0x0000,0x0D5B,0x108E,0x0C2A,0x086D,0x0843,0x0A77,0x0F2C,0x0911,0x0F62,
	0x096F,0x08D9,0x08D8,0x0AEB,0x0865,0x0EFF,0x0913,0x0FFD,0x06D4,0x0CB8,
	0x0A9C,0x0B76,0x0CF7,0x102A,0x0CC4,0x0C7E,0x109A,0x109B,0x0765,0x0C86,
	0x0A38,0x06D5,0x1071,0x07E1,0x0A37,0x079C,0x093F,0x10D3,0x1078,0x0FDA,
	0x0F4A,0x0C89,0x0000,0x0BAA,0x085F,0x0AB3,0x07F6,0x0B29,0x10B4,0x0EF2,
	0x0CBC,0x0000,0x10E5,0x0A51,0x0CB6,0x0E87,0x09B5,0x0861,0x097B,0x0C21,
	0x0BEE,0x0D58,0x0D71,0x0D94,0x0B18,0x08AE,0x0D6F,0x0FDC,0x0F46,0x0A5F,
	0x08FA,0x06CC,0x106A,0x07DE,0x096E,0x0D17,0x10C9,0x0913,0x07C4,0x0BF3,
	0x0FF5,0x0B2C,0x072C,0x0F29,0x0000,0x0A33,0x0ACD,0x0A77,0x0768,0x0000,
	0x089E,0x0EA5,0x0EE8,0x0877,0x0DF7,0x0765,0x0632,0x07B4,0x09E9,0x0A8F,
	0x0A91,0x0E02,0x0D59,0x0CDD,0x102E,0x07BB,0x0E6D,0x0CE4,0x0DD6,0x07B6,
	0x0CAD,0x0F78,0x0A45,0x08A8,0x0CF4,0x0D3C,0x0000,0x0FE7,0x0B6F,0x0FDE,
	0x0CF7,0x09EC,0x0CC1,0x0CC2,0x0609,0x0CF2,0x1093,0x1105,0x0D90,0x065C,
	0x0DD4,0x0FA9,0x10F5,0x10C3,0x07E6,0x1012,0x07DD,0x0000,0x0676,0x0B1A,
	0x0B7F,0x0DD4,0x0D26,0x0000,0x072C,0x0656,0x0A30,0x0EEA,0x0AC7,0x0000,
	0x0658,0x0939,0x0E00,0x0BDF,0x0000,0x0AED,0x1017,0x0623,0x0F89,0x074E,
	0x0BAC,0x0C24,0x0B11,0x0747,0x0D67,0x0ABF,0x09F9,0x0F6C,0x09C0,0x07CA,
	0x0761,0x0E80,0x0763,0x0764,0x0A3F,0x0000,0x0B32,0x05E8,0x0669,0x0606,
	0x0B73,0x105C,0x0608,0x0CAC,0x09E0,0x065A,0x0AF1,0x06A0,0x084A,0x10B1,
	0x0E41,0x090C,0x05CA,0x10DA,0x05FE,0x0E53,0x0A90,0x09BA,0x05CD,0x0CC7,
	0x10E4,0x06AB,0x0B6B,0x0753,0x0F75,0x0673,0x0677,0x0B36,0x0610,0x0F59,
	0x0E1F,0x09E3,0x0EDC,0x0C00,0x10B8,0x0887,0x0B2E,0x09EC,0x0000,0x0DF1,
	0x066D,0x0EC8,0x10C9,0x0E9C,0x1062,0x0A4F,0x08C8,0x0D83,0x078E,0x0B7C,
	0x0945,0x105A,0x0610,0x05CD,0x0B30,0x0DBD,0x0D90,0x07E6,0x0DAC,0x0CD5,
	0x0F2C,0x06A6,0x0BCD,0x08DC,0x1055,0x103C,0x0E41,0x0000,0x0B44,0x0C1C,
	0x0F09,0x10FA,0x0D42,0x0B1F,0x0C5B,0x0F87,0x0F94,0x0BEF,0x0DE8,0x0990,
	0x0DCF,0x0F12,0x0C3D,0x0BC4,0x0F25,0x09B6,0x0D76,0x0896,0x0000,0x0843,
	0x0661,0x08F7,0x0A21,0x0000,0x1047,0x0AE3,0x0C11,0x10F2,0x0B72,0x0966,
	0x092B,0x0629,0x0DAB,0x0FA4,0x0FF0,0x0000,0x0705,0x0B22,0x072F,0x0C1C,
	0x0F00,0x0616,0x099D,0x0BCF,0x0DF7,0x0F9D,0x0B76,0x0AB2,0x102C,0x0AA6,
	0x0B70,0x08C3,0x0A3F,0x0D64,0x0942,0x0CE7,0x06B4,0x0964,0x0BD3,0x0D41,
	0x0587,0x06D3,0x073C,0x0889,0x0A32,0x08BF,0x0611,0x0FEB,0x0736,0x061E,
	0x074E,0x0A47,0x0000,0x099B,0x089B,0x0A63,0x087C,0x0A0B,0x0FAC,0x0D24,
	0x05E5,0x13B5,0x0B48,0x08A1,0x09F4,0x1077,0x0DB0,0x06A2,0x08F0,0x0DE3,
	0x088C,0x0793,0x0D6B,0x0000,0x10FE,0x08CF,0x06B7,0x0CC9,0x085E,0x0FE5,
	0x0A48,0x0C39,0x09D2,0x0A05,0x0000,0x0A9E,0x1005,0x073C,0x0836,0x0000,
	0x09E8,0x0A2C,0x0F59,0x0B7A,0x1058,0x0F89,0x0954,0x073A,0x0659,0x0886,
	0x0FA7,0x0749,0x0F17,0x0A1D,0x0EC3,0x0A32,0x0B1C,0x10A3,0x09C0,0x0000,
	0x1019,0x0665,0x064D,0x0C86,0x0B32,0x0C6D,0x102C,0x0A1B,0x08DE,0x05A3,
	0x0790,0x0D72,0x0A70,0x0E27,0x0E1E,0x099E,0x05A8,0x0C03,0x1105,0x0A67,
	0x0000,0x0903,0x085E,0x05BE,0x1043,0x09D2,0x0BA4,0x0F78,0x0794,0x0FDD,
	0x0000,0x0B65,0x10AE,0x0747,0x1014,0x05AE,0x0640,0x0A22,0x103A,0x077F,
	0x0778,0x0BE8,0x1071,0x06B6,0x0747,0x0FC7,0x0000,0x0D90,0x0000,0x10C6,
	0x10D3,0x0000,0x0000,0x0000,0x0000,0x0683,0x0E6E,0x0A2C,0x0F48,0x07DD,
	0x095F,0x0D66,0x09CF,0x0901,0x0AD5,0x058E,0x0000,0xFFFF,
};
static unsigned short convjmap[50] = {
	0x11B,0x11D,0x11F,0x121,0x123,	0x124,0x126,0x128,0x12A,0x12C,
	0x12E,0x130,0x132,0x134,0x136,	0x138,0x13A,0x13D,0x13F,0x141,
	0x143,0x144,0x145,0x146,0x147,	0x148,0x14B,0x14E,0x151,0x154,
	0x157,0x158,0x159,0x15A,0x15B,	0x15D,0x15F,0x161,0x000,0x000,
	0x162,0x163,0x000,0x000,0x000,	0x168,0x000,0x000,0x000,0x000,
};
static unsigned char convjpage[50] = {
	0xB1,0xB2,0xB3,0xB4,0xB5, 0xB6,0xB7,0xB8,0xB9,0xBA,
	0xBB,0xBC,0xBD,0xBE,0xBF, 0xC0,0xC1,0xC2,0xC3,0xC4,
	0xC5,0xC6,0xC7,0xC8,0xC9, 0xCA,0xCB,0xCC,0xCD,0xCE,
	0xCF,0xD0,0xD1,0xD2,0xD3, 0xD4,0xD5,0xD6,0x00,0x00,
	0xD7,0xD8,0x00,0x00,0x00, 0xDC,0x00,0x00,0x00,0x00
};
static unsigned char alphamap[100] =
 "1234567890"
 "ABCDEFGHIJ"
 "KLMNOPQRST"
 "UVWXYZ   \7"
 "abcdefghij"
 "klmnopqrst"
 "uvwxyz   @"
 "|()[]<>{}`"
 ",.-_/;:!?~"
 "*#+^\\=$%&'";
static unsigned short hiramap[100] = {
	0x11B,0x11D,0x11F,0x121,0x123,	0x124,0x126,0x128,0x12A,0x12C,
	0x12E,0x130,0x132,0x134,0x136,	0x138,0x13A,0x13D,0x13F,0x141,
	0x143,0x144,0x145,0x146,0x147,	0x148,0x14B,0x14E,0x151,0x154,
	0x157,0x158,0x159,0x15A,0x15B,	0x15D,0x15F,0x161,0x000,0x001,
	0x162,0x163,0x164,0x165,0x166,	0x168,0x16B,0x16C,0x01B,0x002,
	0x125,0x127,0x129,0x12B,0x12D,	0x12F,0x131,0x133,0x135,0x137,
	0x139,0x13B,0x13E,0x140,0x142,	0x149,0x14C,0x14F,0x152,0x155,
	0x14A,0x14D,0x150,0x153,0x156,	0x170,0x171,0x172,0x173,0x174,
	0x11A,0x11C,0x11E,0x120,0x122,	0x15C,0x15E,0x160,0x13C,0x167,
	0x169,0x16A,0x16D,0x000,0x000,	0x16E,0x16F,0x000,0x000,0x000
};
static unsigned short katamap[100] = {
	0x179,0x17B,0x17D,0x17F,0x181,	0x182,0x184,0x186,0x188,0x18A,
	0x18C,0x18E,0x190,0x192,0x194,	0x196,0x198,0x19B,0x19D,0x19F,
	0x1A1,0x1A2,0x1A3,0x1A4,0x1A5,	0x1A6,0x1A9,0x1AC,0x1AF,0x1B2,
	0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,	0x1BB,0x1BD,0x1BF,0x000,0x001,
	0x1C0,0x1C1,0x1C2,0x1C3,0x1C4,	0x1C6,0x1C9,0x1CA,0x01B,0x002,
	0x183,0x185,0x187,0x189,0x18B,	0x18D,0x18F,0x191,0x193,0x195,
	0x197,0x199,0x19C,0x19E,0x1A0,	0x1A7,0x1AA,0x1AD,0x1B0,0x1B3,
	0x1A8,0x1AB,0x1AE,0x1B1,0x1B4,	0x1CE,0x1CF,0x1D0,0x1D1,0x1D2,
	0x178,0x17A,0x17C,0x17E,0x180,	0x1BA,0x1BC,0x1BE,0x19A,0x1C5,
	0x1C7,0x1C8,0x1CB,0x000,0x000,	0x1CC,0x1CD,0x1D3,0x1D4,0x1D5
};
static unsigned short nummap[230] = {
	0x0031,0x0032,0x0033,0x0034,0x0035,0x002C,0x002E,0x0025,0x0026,0x007C,
	0x0036,0x0037,0x0038,0x0039,0x0030,0x002B,0x002D,0x002A,0x002F,0x003D,
	0x01CC,0x01CD,0x01CE,0x01CF,0x01D0,0x0103,0x0104,0x0152,0x0154,0x0122,
	0x01D1,0x01D2,0x01D3,0x01D4,0x01CB,0x013B,0x013C,0x0155,0x011E,0x0140,
	0x0105,0x0106,0x0107,0x0108,0x0109,0x010A,0x010B,0x010C,0x010D,0x010E,
	0x010F,0x0110,0x0111,0x0119,0x011A,0x011B,0x011C,0x011D,0x011F,0x0120,
	0x0121,0x0122,0x0123,0x0124,0x0125,0x0126,0x0127,0x0128,0x0129,0x012A,
	0x012B,0x012C,0x012D,0x012E,0x012F,0x0130,0x0131,0x0132,0x0133,0x0134,
	0x0135,0x0136,0x0137,0x0138,0x0139,0x013A,0x013D,0x013E,0x013F,0x0141,
	0x0142,0x0143,0x0144,0x0145,0x0146,0x0147,0x0148,0x0149,0x014A,0x014B,
	0x014C,0x014D,0x014E,0x014F,0x0150,0x0151,0x0152,0x0153,0x0156,0x0157,
	0x0158,0x0159,0x015A,0x015B,0x015C,0x015D,0x015E,0x015F,0x0160,0x0161,
	0x0162,0x0163,0x0164,0x0165,0x0166,0x0167,0x0168,0x0169,0x016A,0x016B,
	0x016C,0x016D,0x013C,0x0120,0x0170,0x0171,0x0172,0x0173,0x0174,0x0175,
	0x0176,0x0177,0x0178,0x0179,0x017A,0x017B,0x017C,0x017D,0x017E,0x017F,
	0x0180,0x0181,0x0182,0x0183,0x0184,0x0185,0x0186,0x0187,0x0188,0x0189,
	0x018A,0x018B,0x018C,0x018D,0x018E,0x018F,0x0190,0x0121,0x0192,0x0193,
	0x0194,0x0195,0x0196,0x0197,0x0198,0x0199,0x019A,0x019B,0x019C,0x019D,
	0x019E,0x019F,0x01A0,0x01A1,0x01A2,0x01A3,0x01A4,0x01A5,0x01A6,0x01A7,
	0x01A8,0x01A9,0x01AA,0x01AB,0x01AC,0x01AD,0x01AE,0x01AF,0x01B0,0x01B1,
	0x01B2,0x01B3,0x01B4,0x01B5,0x01B6,0x01B7,0x01B8,0x01B9,0x01BA,0x01BB,
	0x0100,0x01BC,0x01BD,0x01BE,0x01BF,0x01C0,0x01C1,0x01C2,0x01C3,0x01C4,
	0x01C5,0x01C6,0x01C7,0x01C8,0x01C9,0x01CA,

};
#define	_lang(member)	offsetof(LANGUAGE,member)
static ptrdiff_t plist[MAX_PAGES] = {
	_lang(kbd_page[0]), 
	_lang(kbd_page[1]), 
	_lang(kbd_page[2]), 
	_lang(kbd_page[3]), 
	_lang(kbd_page[4]), 
	_lang(kbd_page[5]),
//	_lang(kbd_page[6]),
	_lang(kbd_page[7]),
//	_lang(kbd_page[8]), 
//	_lang(kbd_page[9]), 
	
	_lang(kbd_enter),
	_lang(kbd_abort),
};
static int pindex[MAX_PAGES] = {
	PG_ALPHABET,
	PG_HIRAGANA,
	PG_KATAKANA,
	PG_NUMBER,
	PG_INDEXKANJI,
	PG_INDEXKANJI2,
//	PG_HISTORYKANJI,
	PG_RAW,
//	PG_CUSTOM,
//	PG_EXTEND,
	
	PG_ENTER,
	PG_ABORT,
};
static int pages=10-3;
static int pageisindex=0;
unsigned char ctrlchar[32] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
//	static unsigned char ctrlchar[32] = "@ABCDEFGH>vKL<NOPQRSTYVWXYZ[\\]^_";
static unsigned short *kbdbuff=NULL;
static int nowpage=0;
static int cursor[MAX_PAGENUM];
static int curm=0;

void printXYw(char *normal, char *control, char *dbcs, int left, int top, uint64 color1, uint64 color2, int cleft, int clen)
{
	char temp[clen+2];

	strncpy(temp, &normal[cleft], clen);
	if (dbcs[cleft] == 2) temp[0] = 32;
	temp[clen] = 0;
	if (dbcs[cleft+clen-1] == 1) temp[clen-1] = 0;
	printXY(temp, left, top, color1, TRUE);
	strncpy(temp, &control[cleft], clen);
	temp[clen] = 0;
	if (dbcs[cleft+clen-1] == 1) temp[clen-1] = 0;
	printXY(temp, left, top, color2, TRUE);
}

int pagemake(short *dist, int limit, int page)
{	// 文字入力欄の作成
	// in:	*dist	書き込み先バッファ
	//		limit	バッファのサイズ
	//		page	作成するページの番号
	int i,k,c,max=0,a,b,s,e;
	memset(dist, 0, limit);
	pageisindex=0;
	nowpage = page;
	curm = CUR_NORMAL;
	if (page == PG_ALPHABET) {
		for (i=0; i<112; i++) {
			dist[i] = alphamap[i];
		}
		max = 100;
		curm = CUR_ENGLISH;
	} else if (page == PG_HIRAGANA) {
		for (i=0; i<100; i++) {
			dist[i] = hiramap[i]+256;
		}
		max = 100; 
	} else if (page == PG_KATAKANA) {
		for (i=0; i<100; i++) {
			dist[i] = katamap[i]+256;
		}
		max = 100;
	} else if (page == PG_NUMBER) {
		for (i=0; i<230; i++) {
			dist[i] = nummap[i];
		}
		max = 230;
	} else if (page == PG_INDEXKANJI) {
		for (i=0; i<50; i++) {
			dist[i] = convmap[i]+256;
		}
		max = 50;
		pageisindex=1;
	} else if ((page >= PG_KANJI) && (page < PG_KANJI+44)) {
		max = kanjimap[page-PG_KANJI+1]-kanjimap[page-PG_KANJI];
		for (i=0; i<max; i++) {
			dist[i] = kanjimap[page-PG_KANJI]+i+256;
		}
	} else if (page == PG_INDEXKANJI2) {
		for (i=0; i<50; i++) {
			dist[i] = convjmap[i]+256;
		}
		max = 50;
		pageisindex=1;
	} else if ((page >= PG_KANJI2) && (page < PG_KANJI2+44)) {
		max = kanjioffset[page-PG_KANJI2+1]-kanjioffset[page-PG_KANJI2]-1;
		for (i=0; i<max; i++) {
			dist[i] = kanjitable[kanjioffset[page-PG_KANJI2]+i]+256;
		}
	} else if (page == PG_CUSTOM) {
		for (i=1; i<127; i++) {
			dist[i] = i;
		}
		max = 126;
		curm = CUR_LINE;
	} else if (page == PG_INDEXDBCS) {
		pageisindex=1;
	} else if (page == PG_HISTORYKANJI) {
	} else if (page == PG_RAW) {
		max = 0;
///*
		
		unsigned short *font_kanji_short;
		font_kanji_short = (unsigned short*)font_kanji;
		k = font_kanji[17];
		//printf("k:0x%08X\n", font_kanji[8]);
		for (i=0; i<k; i++) {
			c=font_kanji_short[9+i*2];
			a=(c >> 8) - 0x81; if (a > 0x1E) a-=0x40;
			b=(c & 255) - 0x40; if (b >=0x3F) b--;
			s=a*188+b+256;
			c=font_kanji_short[10+i*2];
			a=(c >> 8) - 0x81; if (a > 0x1E) a-=0x40;
			b=(c & 255) - 0x40; if (b >=0x3F) b--;
			e=a*188+b+256;
			for(c=s; c<=e; c++) {
				dist[max++] = c;
			}
		}
/*/		extern unsigned char *font_kanji;
		k = font_kanji[17];
		printf("k:0x%08X\n", font_kanji[8]);
		for (i=0; i<k; i++) {
			c=i*2;
			a=font_kanji[19+c] - 0x81; if (a > 0x1E) a-=0x40;
			b=font_kanji[18+c] - 0x40; if (b >=0x3F) b--;
			s=a*188+b+256;
			a=font_kanji[21+c] - 0x81; if (a > 0x1E) a-=0x40;
			b=font_kanji[20+c] - 0x40; if (b >=0x3F) b--;
			e=a*188+b+256;
			for(c=s; c<=e; c++) {
				dist[max++] = c;
			}
		}
*/
		//printf("raw-max: %d\n", max);
	}
	return max;
}
int pagemakefromindex(short *dist, int limit, int ofs)
{
	if (nowpage == PG_INDEXKANJI) {
		if ((ofs < 50) && convpage[ofs])
			return pagemake(dist, limit, PG_KANJI+convpage[ofs]-0xB1);
	} else if (nowpage == PG_INDEXKANJI2) {
		if ((ofs < 50) && convjpage[ofs])
			return pagemake(dist, limit, PG_KANJI2+convpage[ofs]-0xB1);
	} else if (nowpage == PG_INDEXDBCS) {
		
	}
	return 0;
}

int softkbd2(int type, char *c, int max)
{	// 	文字列の入力(SJIS専用)
	//	in:	*out	書き込み用バッファ
	//		max		制限文字数
	//	out:(ret)	=0:正常終了(更新済),=1:キャンセル(未更新)
	
	int i=0,j=0,ret=0,c0,c1,c2;
	int cur,cl=0,redraw=fieldbuffers;
	//int tx[8], ty[8];
	int lx=0,ly=0,lz=0,ld=1,lr=1,cx=0,cy=0,cz=0,cd;
	unsigned char edit[3][max+4],a,b,t=0;
	//unsigned char temp[32];
	uint64 lc,rc;
	//PS2KbdRawKey k;
	char k;
	int left,top,width,height;
	int marw,marh,fonw,fonh,fona;
	marw=GetFontMargin(CHAR_MARGIN);
	marh=GetFontMargin(LINE_MARGIN);
	fonw=GetFontSize(KANJI_FONT_WIDTH);
	fonh=GetFontSize(KANJI_FONT_HEIGHT);
	fona=(fonw - GetFontSize(ASCII_FONT_WIDTH)+1)>>1;
	//if (marw < 0) marw=0;
	//if (marh < 0) marh=0;
	for(i=0;i<MAX_PAGENUM;i++)
		cursor[i] = 0;
	
	// バッファの確保
	kbdbuff = (unsigned short*)malloc(MAX_CHARS*sizeof(unsigned short));
	if (kbdbuff == NULL) return -2;
	
	ld = pages+2;
	// 初期値の解析
	cur = strlen(c);
	memset(edit[1], 0, max);
	for (i=0; i<max; i++) {	// 全角文字判定
		edit[0][i] = a = c[i];
		edit[2][i] = b = 0;
		if (a) edit[2][i] = 32;
		if (i+1<max) b = c[i+1];
		if ((a > 0) && (a < 32)) {
			edit[0][i] = 32;
			edit[2][i] = ctrlchar[a];
		} else if ((((a>=0x81)&&(a<0xA0))||((a>=0xE0)&&(a<=0xFC)))
			&&(b>=0x40)&&(b<=0xFC)&&(b!=0x7F)) {
			edit[1][i++] = 1;
			edit[0][i] = c[i];
			edit[1][i] = 2;
			edit[2][i] = 32;
		} 
		if (a == 0) break;
	}
	// 選択可能な文字の種類のリスト作成
	cd = (pagemake(kbdbuff, MAX_CHARS, PG_ALPHABET)+9)/10;
	
	// 状態の初期化
	if (usbkbd) {
		PS2KbdFlushBuffer();
	//	PS2KbdSetReadmode(PS2KBD_READMODE_RAW);
	}
	width  = FONT_WIDTH  * SKBD_WIDTH;
	height = FONT_HEIGHT * SKBD_HEIGHT;
	left = (SCREEN_WIDTH - width) >> 1;
	top = (SCREEN_HEIGHT - height) >> 1;
	
	// ボタン入力ループ
	while(1){
		waitPadReady(0, 0);
		if (readpad()) {
			if (new_pad) redraw = fieldbuffers;
			if (new_pad & PAD_START) {
				// 終了ボタンへ移動
				lr = 0;
				ly = pages;
			} if (new_pad & PAD_SELECT) {
				// 中止ボタンへ移動
				lr = 0;
				ly = pages+1;
			} if (new_pad & PAD_L1) {
				// 左へ
				if (cur > 0) cur-=(edit[1][cur-1]>0)+1;
			} if (new_pad & PAD_R1) {
				// 右へ
				if ((cur < max) && edit[0][cur]) cur+=(edit[1][cur]>0)+1;
			} if (new_pad & PAD_L2) {
				// 左枠へ
				lr = 0;
			} if (new_pad & PAD_R2) {
				// 右枠へ
				lr = 1;
				ly = lx;
			} if (new_pad & PAD_CROSS) {
					if (cur > 0) cur-=(edit[1][cur-1]>0)+1;
					if (edit[1][cur]>0) {c0=2;} else {c0=1;}
					for(i=cur; i<strlen(edit[0]); i++) {
						edit[0][i] = edit[0][i+c0];
						edit[1][i] = edit[1][i+c0];
						edit[2][i] = edit[2][i+c0];
					}
			}
			if (!lr) {
				if (new_pad & PAD_UP) {
					if (ly > 0) ly--;
				} if (new_pad & PAD_DOWN) {
					if (++ly >= ld) ly = ld-1;
				} if (new_pad & PAD_CIRCLE) {
					//printf("ly: %d, pindex[ly]: %d\n", ly, pindex[ly]);
					if (pindex[ly] == PG_ENTER) {
						ret = 0;
						break;
					} else if (pindex[ly] == PG_ABORT) {
						ret = -1;
						break;
					}
					lx = ly;
					lr = 1;
					cursor[nowpage] = cy;
					cd = (pagemake(kbdbuff, MAX_CHARS, pindex[ly])+9)/10;
					cy = cursor[nowpage];
				} if (new_pad & PAD_TRIANGLE) {
					ly = pages;
				} if (new_pad & PAD_LEFT) {
					ly -= SKBD_ITEMS/2;
				} if (new_pad & PAD_RIGHT) {
					ly += SKBD_ITEMS/2;
				}
			} else {
				if (new_pad & PAD_UP) {
					if (cy > 0) cy--;
				} if (new_pad & PAD_DOWN) {
					if (++cy >= cd) cy = cd-1;
				} if (new_pad & PAD_LEFT) {
					//if (cx > 0) cx--;
					cx = (cx+9) % 10;
				} if (new_pad & PAD_RIGHT) {
					//if (cx < 9) cx++;
					cx = (cx+1) % 10;
				} if (new_pad & PAD_CIRCLE) {
					if (pageisindex) {
						cursor[nowpage] = cy;
						i = (pagemakefromindex(kbdbuff, MAX_CHARS, cy*10+cx)+9)/10;
						cy = cursor[nowpage];
						if (i) cd = i;
					} else if (kbdbuff[cy*10+cx]) {
						if (kbdbuff[cy*10+cx] < 256) {c0=1;} else {c0=2;}
						edit[2][max-1] = edit[0][max-1] = 0;
						if (edit[1][max-1] == 2) {
							edit[2][max-2] = edit[1][max-2] = edit[0][max-2] =0;
						}
						for (i=strlen(edit[0]); i>=cur; i--) {
							edit[0][i+c0] = edit[0][i];
							edit[1][i+c0] = edit[1][i];
							edit[2][i+c0] = edit[2][i];
						}
						edit[2][max-1] = edit[0][max-1] = 0;
						if (edit[1][max-1] == 2) {
							edit[2][max-2] = edit[1][max-2] = edit[0][max-2] =0;
						}
						if (c0 < 2) {
							c1 = kbdbuff[cy*10+cx];
							c2 = 32;
							if (c1 < 32) {
								c2 = ctrlchar[c1];
								c1 = 32;
							}
							edit[0][cur] = c1;
							edit[1][cur] = 0;
							edit[2][cur++] = c2;
						} else {
							c0 = kbdbuff[cy*10+cx]-256;
							c1 = c0 / 188 + 0x81; if (c1 >= 0xA0) c1+=0x40;
							c2 =(c0 % 188)+ 0x40; if (c2 >= 0x7F) c2++;
							edit[0][cur] = c1;
							edit[1][cur] = 1;
							edit[2][cur++] = 32;
							edit[0][cur] = c2;
							edit[1][cur] = 2;
							edit[2][cur++] = 32;
						}
					}
				} if (new_pad & PAD_TRIANGLE) {
					if (pindex[ly] != nowpage) {
						cursor[nowpage] = cy;
						cd = (pagemake(kbdbuff, MAX_CHARS, pindex[ly])+9)/10;
						cy = cursor[nowpage];
					} else {
						lr = 0;
					}
				}
			}
		}
		/*
		if ((usbkbd) && (PS2KbdReadRaw(&k))) {
			redraw = fieldbuffers;
			if (k.state == 0xF1)
			printf("usbkbd: key=%02X(%3d)\n", k.key, k.key);
		}
		*/
		if ((usbkbd) && (PS2KbdRead(&k))) {
			redraw = fieldbuffers;
			if (k==PS2KBD_ESCAPE_KEY) {
				PS2KbdRead(&k);
				if (k == 0x29) {		// →
					//cx = (++cx) % 10;
					if ((cur < max) && edit[0][cur]) cur+=(edit[1][cur]>0)+1;
				} else if (k == 0x2A) {	// ←
					//cx = (cx+9) % 10;
					if (cur > 0) cur-=(edit[1][cur-1]>0)+1;
				} else if (k == 0x2B) {	// ↓
					//cy++;
				} else if (k == 0x2C) {	// ↑
					//cy--;
				} else if (k == 0x24) {	// home
					cur = 0;
				} else if (k == 0x27) {	// end
					cur = strlen(edit[0]);
				} else if (k == 0x25) { // page up
					//if (cur>0) cur--;
				} else if (k == 0x28) {	// page down
					//if (cur<strlen(edit[0])) cur++;
				} else if (k == 0x26) {	// delete
					//if (cur > 0) cur-=(edit[1][cur-1]>0)+1;
					if (cur < strlen(edit[0])) {
						//cur=strlen(edit[0])-(edit[1][strlen(edit[0])-1]>0)-1;
						if (cur < 0) cur = 0;
						if (edit[1][cur]>0) {c0=2;} else {c0=1;}
						for(i=cur; i<strlen(edit[0]); i++) {
							edit[0][i] = edit[0][i+c0];
							edit[1][i] = edit[1][i+c0];
							edit[2][i] = edit[2][i+c0];
						}
					}
				} else if (k == 0x1B) {	// esc
					ret = -1;
					break;
				}
			} else {
				if (k == 8) {			// BS
					if (cur > 0) {
						cur-=(edit[1][cur-1]>0)+1;
						if (edit[1][cur]>0) {c0=2;} else {c0=1;}
						for(i=cur; i<strlen(edit[0]); i++) {
							edit[0][i] = edit[0][i+c0];
							edit[1][i] = edit[1][i+c0];
							edit[2][i] = edit[2][i+c0];
						}
					}
				} else if ((k == 10)||(k == 13)) {	// Enter
					ret = 0;
					break;
				} else if (k == 27) {
					ret = -1;
					break;
				} else {
					i = strlen(edit[0]);
					edit[0][max-2] = edit[2][max-2] = 0;
					if (edit[1][max-2]==2)
						edit[0][max-3] = edit[1][max-3] = edit[2][max-3] = 0;
					for (i=strlen(edit[0]); i>=cur; i--) {
						edit[0][i+1] = edit[0][i];
						edit[1][i+1] = edit[1][i];
						edit[2][i+1] = edit[2][i];
					}
					if (k < 32) {
						edit[0][cur] = 32;
						edit[2][cur] = ctrlchar[(int)k];
					} else {
						edit[0][cur] = k;
						edit[2][cur] = 32;
					}
					edit[1][cur] = 0;
					cur++;
				}
			}
		}
		t++;
		if ((t==SCANRATE/2) || (t == SCANRATE)) redraw = framebuffers;
		if (cur < 0) cur = 0;
		if (cur >= max) {cur = max-1; if (edit[1][cur] == 2) cur--;}
		if (cl > cur) cl = cur;
		if (cl+SKBD_LENGTH < cur) cl = cur-SKBD_LENGTH;
		if (edit[1][cl] == 2) cl++;
		if (cl < 0) cl = 0;
		if (cy < 0) cy = 0;
		if (cy >= cd) cy = cd-1;
		if (cz > cy) cz = cy;
		if (cz+SKBD_ITEMS <= cy) cz = cy - SKBD_ITEMS +1;
		if (ly >= ld) ly = ld-1;
		if (ly < 0) ly = 0;
		if (lz+SKBD_ITEMS <= ly) lz = ly - SKBD_ITEMS +1;
		if (lz > ly) lz = ly;
		if (redraw) {
			//	printf("cur: %d\n", cur);
			drawDialogTmp(left, top, left+width, top+height, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			//入力中の文字列の表示
			printXYw(edit[0], edit[2], edit[1], left+FONT_WIDTH*2, top+FONT_HEIGHT*0.5, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], cl, SKBD_LENGTH);

			//キャレット
			if(t==SCANRATE) t=0;
			if(t<SCANRATE/2){
				printXY("|",
					left+FONT_WIDTH*(cur-cl+1.5),
					top+FONT_HEIGHT*0.5,
					setting->color[COLOR_HIGHLIGHTTEXT], TRUE);
				//	itoLine(setting->color[COLOR_TEXT], i=left+(cur+2)*FONT_WIDTH, j=top+FONT_HEIGHT, 0,
				//			setting->color[COLOR_TEXT], i, j+FONT_HEIGHT, 0);	
			}
			if (lr) {
				lc = setting->color[COLOR_TEXT];
				rc = setting->color[COLOR_HIGHLIGHTTEXT];
			} else {
				lc = setting->color[COLOR_HIGHLIGHTTEXT];
				rc = setting->color[COLOR_TEXT];
			}
			// 左枠
			i=left+FONT_WIDTH/2;
			drawDialogTmp(i, top+FONT_HEIGHT*1.5, i+FONT_WIDTH*SKBD_LEFT, top+height-FONT_HEIGHT*1.5, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			for(i=0;i<SKBD_ITEMS;i++) {
				// 項目表示
				if (plist[i+lz] >= 0)
					printXY(((char*) &lang->gen_ok[0]) +plist[i+lz], left+FONT_WIDTH*2, top+FONT_HEIGHT*(2+i), setting->color[COLOR_TEXT], TRUE);
			}
			if (ld > SKBD_ITEMS) {
				i=left+FONT_WIDTH*(SKBD_LEFT-1);
				drawBar(i, top+FONT_HEIGHT*2, i+FONT_WIDTH-2, top+FONT_HEIGHT*(2+SKBD_ITEMS), setting->color[COLOR_FRAME], lz, SKBD_ITEMS, ld);
			}
			printXY(">", left+FONT_WIDTH, top+FONT_HEIGHT*(2+ly-lz), lc, TRUE);
			printXY(((char*) &lang->gen_ok[0]) +plist[ly], left+FONT_WIDTH*2, top+FONT_HEIGHT*(2+ly-lz), lc, TRUE);
			// 右枠
			drawDialogTmp(left+FONT_WIDTH*(0.5+SKBD_LEFT), top+FONT_HEIGHT*1.5, left+width-FONT_WIDTH/2, top+height-FONT_HEIGHT*1.5, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			if (cd > SKBD_ITEMS)
				drawBar(left+width-FONT_WIDTH*2, top+FONT_HEIGHT*2, left+width-FONT_WIDTH-2, top+FONT_HEIGHT*(2+SKBD_ITEMS), setting->color[COLOR_FRAME], cz, SKBD_ITEMS, cd);
			itoPrimAlphaBlending( TRUE );
			j = top + (cy-cz +2) * FONT_HEIGHT -2;
			if (curm == CUR_NORMAL || curm == CUR_ENGLISH) {
				i = left + (SKBD_LEFT + cx*2.5 +2 +(cx>4)) * FONT_WIDTH -2;
				itoSprite(rc|0x10000000, i, j, i+FONT_WIDTH*2 -marw*2+5, j+FONT_HEIGHT -marh+5, 0);
				drawFrame(i, j, i+FONT_WIDTH*2 -marw*2+5, j+FONT_HEIGHT -marh+5, rc);
			} else if (curm == CUR_LINE) {
				i = left + (SKBD_LEFT + 2) * FONT_WIDTH -2;
				itoSprite(rc|0x10000000, i, j, i+FONT_WIDTH*(SKBD_WIDTH-SKBD_LEFT-4), j+FONT_HEIGHT -marh+4, 0);
				drawFrame(i, j, i+FONT_WIDTH*(SKBD_WIDTH-SKBD_LEFT-4), j+FONT_HEIGHT -marh+4, rc);
			}
			itoPrimAlphaBlending(FALSE);
			for (i=cz;i<cz+SKBD_ITEMS;i++) {
				for (j=0;j<10;j++) {
					if (kbdbuff[i*10+j] < 256) {
						drawChar_JIS(kbdbuff[i*10+j], 
							left + (SKBD_LEFT + j*2.5 +2 +(j>4)) * FONT_WIDTH +fona, 
							top + (i-cz +2) * FONT_HEIGHT, 
							setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], ctrlchar);
					} else {
						drawChar_JIS(kbdbuff[i*10+j], 
							left + (SKBD_LEFT +j*2.5 +2 +(j>4)) * FONT_WIDTH, 
							top + (i-cz +2) * FONT_HEIGHT, 
							setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], ctrlchar);
					}
				}
			}
			//printXY(">", left+FONT_WIDTH*(SKBD_LEFT+1+cx*3), top+FONT_HEIGHT*(2+cy-cz), rc, TRUE);
			// 下枠
			//strcpy(temp, "codepoint:%3d-%2d, SJIS:0x%04X size:%2dx%2d"); 
			//printXY(temp, left+FONT_WIDTH*2, top+FONT_HEIGHT*(0.5+SKBD_ITEMS+2), setting->color[COLOR_TEXT], TRUE);
			{
				char temp[1024];
				// 下部(お知らせなど)の再描画
				i = kbdbuff[cy*10+cx];
				if (i < 256) {
					if (i == 0)
						strcpy(temp,  "[ ]  0x0000, -----, U+0000");
					else if (i < 128) 
						sprintf(temp, "[%c]  0x%04X, -----, U+%04X", i, i, i);
					else
						sprintf(temp, "[%c]  0x%04X, -----, ------", i, i);
					//drawString(info, TXT_ASCII, tl+((defw+3)-strlen(info))*FONT_WIDTH, tt+th - FONT_HEIGHT*5/4, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], ctrlchar);
				} else {
					int k,m,u;
					extern unsigned short sjistable[];
					i = kbdbuff[cy*10+cx] - 256;
					k = (i / 188) + 0x81;
					m = (i % 188) + 0x40;
					u = sjistable[i];
					if (k > 0x9F) k += 0x40;
					if (m >= 0x7F) m++;
					if (u == 0) u = 0x3f;
					sprintf(temp, "[%c%c] 0x%02X%02X, %02d-%02d, U+%04X", k, m, k, m, i / 94 +1, (i % 94) +1, u);
					//drawString(info, TXT_SJIS, tl+((defw+3)-strlen(info))*FONT_WIDTH, tt+th - FONT_HEIGHT*5/4, setting->color[COLOR_TEXT], setting->color[COLOR_HIGHLIGHTTEXT], ctrlchar);
				}
				printXY(temp, left+FONT_WIDTH*16, top+FONT_HEIGHT*(0.5+SKBD_ITEMS+2), setting->color[COLOR_TEXT], TRUE);
			}
			// 操作説明
			i = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[COLOR_BACKGROUND], 0, i, SCREEN_WIDTH, i+FONT_HEIGHT, 0);
			if (lr)
				printXY(lang->kbd_helpr, FONT_WIDTH, i, setting->color[COLOR_TEXT], TRUE);
			else
				printXY(lang->kbd_helpl, FONT_WIDTH, i, setting->color[COLOR_TEXT], TRUE);
			
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	//PS2KbdFlushBuffer();
	//PS2KbdSetReadmode(PS2KBD_READMODE_NORMAL);
	free(kbdbuff);
	kbdbuff = NULL;
	if (ret >= 0) {
		for (i=0; i<=strlen(edit[0]); i++) {
			if ((edit[0][i] == 32) && (edit[2][i] != 32)) {
				for (j=0; j<32; j++) {
					if (edit[2][i] == ctrlchar[j]) {
						c[i] = j;
						break;
					}
				}
			} else {
				c[i] = edit[0][i];
			}
			if (i==max-1) {
				c[i] = 0;
				if (edit[1][i] == 2) c[i-1] = 0;
				break;
			}
		}
	}
	return ret;
};

int keyboard(int mode, char *buff, int limit)
{
	drawDarks(0);
	if (mode > 15) return softkbd2(SKBD_ALL, (char*)mode, (int)buff);
	return softkbd2(mode, buff, limit);
};

typedef struct {
	int flag;
	int	size;
	int chk;
	char url[256];
	char def[32];
	char eng[80];
	char jap[80];
} dldata;

int makecheckword(char *src, int size)
{
	unsigned int *buff=(unsigned int*)src, sz, siz, i, old=0;
	siz = size - (size & 3);
	sz = ((unsigned int)size) >> 2;
	//a = 69069; b = 16807;
	switch(size & 3) {
		case 3:
			old |= (unsigned int) src[siz+2] << 16;
		case 2:
			old |= (unsigned int) src[siz+1] << 8;
		case 1:
			old |= (unsigned int) src[siz];
			break;
		case 0:
			old = 69069;
			break;
	}
	for (i=0; i<sz; i++) {
		old *= 16807;
		old += buff[i]; 
	}
	return (int)old;
}

typedef struct {
	char *buff;
	int size;
} string;

int file_put_contents(char *path, char *buff, int size)
{
	int dl, dt, dw, dh, tl, tt, tw, th, bl, bt, bw, bh, ml, mt, my, r, fd;
	FILE *fp=NULL;
	size_t now,rem,tsize,packetsize=4096;
	char tmp[192];
	uint64 oldframe=0;
	fp = fopen(path, "wb");
	if (fp != NULL) {
		//drawDarks(0);
		//drawMsg(lang->nupd[8]);
		bw = tw = FONT_WIDTH*32;	th = FONT_HEIGHT*4;
		dw = tw + 8 + FONT_WIDTH*2; dh = th + FONT_HEIGHT;
		dl = (SCREEN_WIDTH  - dw) >> 1;
		dt = (SCREEN_HEIGHT - dh) >> 1;
		ml = bl = tl = dl + 4 + FONT_WIDTH; mt = tt = dt + 4 + (FONT_HEIGHT >> 1);
		my = mt + FONT_HEIGHT;
		bt = mt + FONT_HEIGHT*5/2; bh = FONT_HEIGHT;
		fd = 0;
		//ダイアログ
		for (now=0, rem=size; now<size; now+=packetsize, rem-=packetsize) {
			if (size-now < packetsize) tsize = size-now; else tsize = packetsize;
			//sprintf(msg, "%s [%d/%dKB:%3d%%]", lang->nupd[8], now >> 10, (size+1023) >> 10, now * 100 / size);
			//drawMsg(msg);
			////itoGsFinish();
			if ((oldframe != totalcount) || (rem <= packetsize)) {
				for(r=0;r<fieldbuffers;r++){
					if (fd < framebuffers) {
						drawDialogTmp(dl, dt, dl+dw, dt+dh, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
						//メッセージ
						printXY(lang->nupd[9], ml, mt, setting->color[COLOR_TEXT], TRUE);
					}
					sprintf(tmp, "%5d KB / %d KB ( %3d%% )", (now +tsize +512) >> 10, (size +512) >> 10, (now+tsize) * 100 / size);
					// *****/*****KB (***%)
					if (fd >= framebuffers)
						itoSprite(setting->color[COLOR_BACKGROUND], ml, my, ml+FONT_WIDTH*32, my+FONT_HEIGHT+1, 0);
					printXY(tmp, ml, my, setting->color[COLOR_TEXT], TRUE);
					//プログレスバー
					drawBar(bl, bt, bl+bw, bt+bh, setting->color[COLOR_FRAME], 0, now+packetsize, size);
					itoGsFinish();
					if (ffmode)
						itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
					else
						itoSwitchFrameBuffers();
					if (fd < framebuffers) fd++;
				}//*/
				oldframe = totalcount;
			}
			if (fwrite(buff +now, 1, tsize, fp) < tsize) {
				fclose(fp);
				return -2;
			}
		}
		fclose(fp);
		return 0;
	}
	return -1;
}

string file_get_contents(char *url, int dsize)
{
	int dl, dt, dw, dh, tl, tt, tw, th, bl, bt, bw, bh, ml, mt, my, r, fd, cnt, k=IDNO;
	FILE *fp=NULL;
	size_t size=0,now,rem,tsize,packetsize=1024,contentlength,i;
	char *buff=NULL, *rema=NULL, tmp[192];
	string ret;
	uint64 oldframe=0;
	enum{defaultsize=262144,extendsize=131072};
	fp = fopen(url, "rb");
	if (fp != NULL) {
		drawDarks(0);
		//drawMsg(lang->nupd[8]);
		fseek(fp, 0, SEEK_END);
		size = contentlength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (size == 0) size = dsize;
		if (size == 0) size = defaultsize;
		buff = (char*)malloc(size);
		if (buff != NULL) {
			bw = tw = FONT_WIDTH*32;	th = FONT_HEIGHT*4;
			dw = tw + 8 + FONT_WIDTH*2; dh = th + FONT_HEIGHT;
			dl = (SCREEN_WIDTH  - dw) >> 1;
			dt = (SCREEN_HEIGHT - dh) >> 1;
			ml = bl = tl = dl + 4 + FONT_WIDTH; mt = tt = dt + 4 + (FONT_HEIGHT >> 1);
			my = mt + FONT_HEIGHT;
			bt = mt + FONT_HEIGHT*5/2; bh = FONT_HEIGHT;
			fd = 0; cnt = totalcount -32;
			//ダイアログ
			for (now=0, rem=size; now<size; now+=tsize, rem-=tsize) {
				if (size-now < packetsize) tsize = size-now; else tsize = packetsize;
				//sprintf(msg, "%s [%d/%dKB:%3d%%]", lang->nupd[8], now >> 10, (size+1023) >> 10, now * 100 / size);
				//drawMsg(msg);
				////itoGsFinish();
				if ((oldframe != totalcount) || (rem <= packetsize)) {
					if (readpad()) {
						if (new_pad & PAD_TRIANGLE) {
							k = MessageBox("abort the download?", LBFN_VER, MB_YESNO|MB_DEFBUTTON2);
							if (k == IDYES) {
								break;
							}
							fd = 0;
						}
					}
					for(r=0;r<fieldbuffers;r++){
						if (fd < framebuffers) {
							drawDialogTmp(dl, dt, dl+dw, dt+dh, setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
							//メッセージ
							printXY(lang->nupd[8], ml, mt, setting->color[COLOR_TEXT], TRUE);
						}
						if (contentlength)
							sprintf(tmp, "%5d KB / %d KB ( %3d%% )", (now +tsize +512) >> 10, (size +512) >> 10, (now+tsize) * 100 / size);
						else
							sprintf(tmp, "%5d KB", (now +tsize + 512) >> 10);
						// *****/*****KB (***%)
						if (fd >= framebuffers) 
							itoSprite(setting->color[COLOR_BACKGROUND], ml, my, ml+FONT_WIDTH*32, my+FONT_HEIGHT+1, 0);
						printXY(tmp, ml, my, setting->color[COLOR_TEXT], TRUE);
						//プログレスバー
						if (contentlength)
							drawBar(bl, bt, bl+bw, bt+bh, setting->color[COLOR_FRAME], 0, now+packetsize, size);
						else
							drawBar(bl, bt, bl+bw, bt+bh, setting->color[COLOR_FRAME], ((totalcount - cnt) % (96 + 32)) - 32, 32, 96);
						itoGsFinish();
						if (ffmode)
							itoSetActiveFrameBuffer(itoGetActiveFrameBuffer()^1);
						else
							itoSwitchFrameBuffers();
						if (fd < framebuffers) fd++;
					}//*/
					oldframe = totalcount;
				}
				i = fread(buff +now, 1, tsize, fp);
				if (!contentlength) {
					if (i < tsize) {
						size = now+i;
						break;
					} else if (now + tsize >= size) {
						rema = (char*)realloc(buff, size+extendsize);
						//printf("file_get_contents: remalloc(%d): 0x%08X => 0x%08X\n", size+extendsize, (u32)buff, (u32)rema);
						if (rema == NULL) {
							break;
						} else if (rema != buff) {
							buff = rema;
						}
						size+=extendsize;
						rem+=extendsize;
					}
				}
				//printf("file_get_contents: fread=%d\n", i);
			}
		}
		fclose(fp);
		if (k == IDYES) {
			free(buff);
			ret.buff = 0;
			ret.size = -123456;
			return ret;
		}
	} else {
		size = -(int)fp;
	}
	ret.buff = buff;
	ret.size = size;
	return ret;
}

int NetworkDownload(char* msg0)
{	
	int ret=-1,i,k,m,files=0;
	char tmp[2048], key[16], msg1[240];
	char testpath[1024];
	extern char tmps[32][MAX_PATH];
	extern int tmpi[32], explodeconfig(const char *src);
	dldata *list=NULL;
	string data;
	loadHTTPModules();
	
	cnf_init();
	if (cnf_load("http://www.geocities.jp/nika_towns/lbfn_upd.ini") != 0) {
		strcpy(msg0, lang->nupd[14]);
		drawMsg(msg0);
		cnf_free();
		return -1;
	}
	if (cnf_getstr("files", tmp, "")>=0)
		files=atoi(tmp);
	printf("update: %d files\n", files);
	list = (dldata*)malloc(sizeof(dldata)*files);
	if (list == NULL) {
		cnf_free();
		strcpy(msg0, lang->editor_viewer_error2);
		drawMsg(msg0);
		return -2;
	}
	for (i=0; i<files; i++) {
		sprintf(key, "file%d", i);
		if (cnf_getstr(key, tmp, "")<0) break;
		m = explodeconfig((const char*)tmp);
		//printf("update: %s==%s\n", key, tmp);
		//printf("update:	url[%d]=%s\n	default[%d]=%s\n	english[%d]=%s\n	japanese[%d]=%s\n", i, tmps[0], i, tmps[1], i, tmps[2], i, tmps[3]);
		k = 0;
		// ファイルフォーマット
		//   file%d=[flag],[size],"http://(url)","(default filename)","(English document)","(Japanese document)"
		list[i].flag = tmpi[k++];
		list[i].size = tmpi[k++];
		list[i].chk = tmpi[k++];
		strcpy(list[i].url, tmps[k++]);
		strcpy(list[i].def, tmps[k++]);
		strcpy(list[i].eng, tmps[k++]);
		strcpy(list[i].jap, tmps[k++]);
		//printf("upload: chk=0x%08X, url=%s\n", list[i].chk, list[i].url);
	}
	
	enum{
		downloadfile=0,
		downloadpath,
		downloadname,
		execute,
		downloadback,
		menuitems,
		testurl,
		testdl,
		backupcopy,
	};
	uint64 color;
	int nList=0, sel=0, top=0, redraw=framebuffers;
	int pushed=TRUE;
	int x, y, y0, y1;
	int backup=0,filenum=0,displaytype=3;
	char dlpath[MAX_PATH],dlrename[64];
	char config[menuitems][MAX_PATH];
	char *onoff[2] = {lang->conf_off, lang->conf_on};
	//char *lang0[10] = {lang->nupd[1], lang->nupd[2], lang->nupd[3], "testurl: %s", "testdl", lang->nupd[5], lang->nupd[6], lang->nupd[4]};
	//char *lang1[10] = {lang->nupd[19], lang->nupd[20], lang->nupd[20], lang->nupd[20], lang->nupd[21], lang->nupd[21], lang->nupd[21], lang->nupd[20]};
	char *lang0[8] = {lang->nupd[1], lang->nupd[2], lang->nupd[3], lang->nupd[5], lang->nupd[6], lang->nupd[4]};
	char *lang1[8] = {lang->nupd[19], lang->nupd[20], lang->nupd[20], lang->nupd[21], lang->nupd[21], lang->nupd[20]};
	strcpy(dlpath, setting->downloadpath);
	strcpy(dlrename, list[0].def);
	strcpy(testpath, "http://www.nicovideo.jp/ranking/hourly/fav/all");

	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if(new_pad) {pushed=TRUE; redraw = framebuffers;}
			if(new_pad & PAD_UP)
				sel--;
			else if(new_pad & PAD_DOWN)
				sel++;
			else if(new_pad & PAD_LEFT)
				sel-=MAX_ROWS/2;
			else if(new_pad & PAD_RIGHT)
				sel+=MAX_ROWS/2;
			else if(new_pad & PAD_TRIANGLE)
				break;
			else if(new_pad & PAD_CIRCLE){
				// downloadfile,path,name, backupcopy, execute, back
				if(sel==backupcopy)
					backup ^= 1;
				else if(sel==downloadpath) {
				//	strcpy(dlrename, list[filenum].def);
					strcpy(tmp, dlpath);
					getFilePath(tmp, DIR);
					if(strncmp(tmp, "cdfs", 2))
						strcpy(dlpath, tmp);
				}
				else if(sel==downloadname){
					strcpy(tmp, dlrename);
					if(keyboard(SKBD_FILE, tmp, 63)>=0)
						strcpy(dlrename, tmp);
				}
				else if(sel==downloadfile){
					// ファイルを開く
					data = file_get_contents(list[filenum].url, list[filenum].size);
					if (data.buff != NULL) {
						int checkword;
						char chkstr[8];
						strcpy(chkstr, "error");
						checkword = makecheckword(data.buff, data.size);
						if (checkword == list[filenum].chk) strcpy(chkstr, "ok");
						i = CRC32Check(data.buff, data.size);
						printf("update: checkword: %s, list: 0x%08X, dl: 0x%08X, CRC32:%08X\n", chkstr, list[filenum].chk, checkword, i);
						viewer(0, list[filenum].url, data.buff, data.size);
						free(data.buff);
						data.buff = NULL;
					}
				}
				else if(sel==testurl){
					strcpy(tmp, testpath);
					if(keyboard(SKBD_ALL, tmp, 1024)>=0)
						strcpy(testpath, tmp);
				}
				else if(sel==testdl){
					data = file_get_contents(testpath, 0);
					if (data.buff != NULL) {
						viewer(0, testpath, data.buff, data.size);
						free(data.buff);
					}
				}
				else if(sel==downloadback){
					break;
				}
				else if(sel==execute) {
					// ダウンロード実行
					drawMsg(lang->nupd[0]);
					if (MessageBox(lang->nupd[7], lang->nupd[0], MB_OKCANCEL) == IDOK) {
						pushed = FALSE;
						data = file_get_contents(list[filenum].url, list[filenum].size);
						if (data.buff != NULL) {
							// ダウンロード成功
							if ((list[filenum].chk == 0xFFFFFFFF) || 
								(makecheckword(data.buff, data.size) == list[filenum].chk)
							) {	// ダウンロードの検証OK→セーブ
								sprintf(tmps[0], "%s%s", dlpath, dlrename);
								// パス変更
								if (!strncmp(dlpath, "mc:", 3)) {
									int mcport;
									if(boot==MC_BOOT)
										mcport = LaunchElfDir[2]-'0';
									else
										mcport = CheckMC();
									if (mcport<0 || mcport>1) mcport = 0;
									sprintf(tmps[0], "mc%d:%s%s", mcport, &dlpath[3], dlrename);
								} else if (!strncmp(dlpath, "hdd0:", 5)) {
									sprintf(tmps[0], "pfs0:%s%s", &dlpath[5], dlrename);
								}
								// 書き出し
								if (!(i=file_put_contents(tmps[0], data.buff, data.size))) {
									// 完了
									strcpy(msg0, lang->nupd[24]);
								} else if (i == -2) {
									// 失敗
									strcpy(msg0, lang->nupd[18]);
								} else {
									strcpy(msg0, lang->nupd[10]);
								}
							} else {
								// checkwordの相違エラー
								strcpy(msg0, lang->nupd[15]);
							}
							free(data.buff);
							data.buff = NULL;
						} else if (data.size == -123456) {
							// ダウンロード中止
							msg0[0] = 0;
						} else {
							// ダウンロード失敗
							sprintf(msg0, "%s [%d]", lang->nupd[10], -(int)data.size);
						}
						if (msg0[0]) MessageBox(msg0, lang->nupd[0], MB_OK);
					}
				}
			}
			else if(new_pad & PAD_SQUARE) {
				if (sel==downloadpath) {
					strcpy(dlpath, "mc:/BOOT/");
				}
			}
			else if (new_pad & PAD_L1) {
				if (filenum > 0) {
					filenum--;
					strcpy(dlrename, list[filenum].def);
				}
			}
			else if (new_pad & PAD_R1) {
				if (filenum < files-1) {
					filenum++;
					strcpy(dlrename, list[filenum].def);
				}
			}
			else if (new_pad & PAD_START) {
				if (sel == execute) sel = downloadfile; else sel = execute;
			}
			else if (new_pad & PAD_SELECT) {
				sel = downloadback;
			}
		}

		// downloadfile,path,name, backupcopy, execute, back
		for (i=0; i<menuitems; i++) {
			if (i==downloadfile){
				switch(displaytype) {
					case 0:
						sprintf(msg1, "%s", list[filenum].url);
						break;
					case 1:
						sprintf(msg1, "%s", list[filenum].def);
						break;
					case 2:
						if (setting->language == LANG_JAPANESE)
							sprintf(msg1, "%s", list[filenum].jap);
						else
							sprintf(msg1, "%s", list[filenum].eng);
						break;
					case 3:
						if (setting->language == LANG_JAPANESE)
							sprintf(msg1, "%s (%s)", list[filenum].def, list[filenum].jap);
						else
							sprintf(msg1, "%s (%s)", list[filenum].def, list[filenum].eng);
						break;
				}
			}
			else if (i==testurl)
				strcpy(msg1, testpath);
			else if (i==downloadpath)
				strcpy(msg1, dlpath);
			else if (i==downloadname)
				strcpy(msg1, dlrename);
			else if (i==backupcopy) {
				strcpy(msg1, onoff[backup != 0]);
			}
			sprintf(config[i], lang0[i], msg1);
			if (i==sel) strcpy(tmp, msg1);
		}
		
		// リスト表示用変数の正規化
		nList = menuitems;
		if(top > nList-MAX_ROWS)	top=nList-MAX_ROWS;
		if(top < 0)			top=0;
		if(sel >= nList)		sel=nList-1;
		if(sel < 0)			sel=0;
		if(sel >= top+MAX_ROWS)	top=sel-MAX_ROWS+1;
		if(sel < top)			top=sel;

		// 画面描画開始
		if (redraw) {
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
				printXY(config[top+i], x+FONT_WIDTH*2, y, color, TRUE);
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
			if(pushed) sprintf(msg0, "%s", lang->nupd[0]);
			// 操作説明
			// downloadfile,path,name, backupcopy, execute, back
			sprintf(msg1, lang1[sel], tmp);
			setScrTmp(msg0, msg1);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
	}
	
	msg0[0]=0;
	free(list); list=NULL;
	cnf_free();
	return ret;
}
/*{
	FILE *fp=NULL;
	size_t size;
	char *buff=NULL;
	char temp[]="http://www.geocities.jp/nika_towns/lbfn_upd.ini";
	//"http://com-nika.osask.jp/image/sms1024i_japanese3s.jpg";
	//"http://dic.nicovideo.jp/img/logo_nicopedia.gif";
	//"http://dic.nicovideo.jp/mml/3256";
	//"http://smile-cll10.nicovideo.jp/smile?m=9208920.91908";
	//"http://gyazo.com/f277f7d1cdb064808ead4ac767d57904.png";
	//"http://res.nimg.jp/img/base/head/icon/nico/417.gif";
	//	"http://192.168.0.4/tek_comp.c";
	//	http://202.248.110.224/img/base/head/logo/nine.png
	fp = fopen(temp, "rb");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (size == 0) size = 128;
		buff = (char*)malloc(size);
		if (buff != NULL) {
			fread(buff, 1, size, fp);
			viewer(0, temp, buff, size);
			free(buff);
		}
		fclose(fp);
	}
}*/

unsigned int crc32table[256] = {0,0,};
void CRC32Init(void)
{
	if (crc32table[1]) return;
	unsigned int poly=0xEDB88320, u, i, j;

	for (i = 0; i < 256; i++) {
		u = i;
		for (j = 0; j < 8; j++) {
			if (u & 1) {
				u = (u >> 1) ^ poly;
			} else {
				u >>= 1;
			}
		}
		crc32table[i] = u;
	}
	printf("CRC32Inited\n");
}

unsigned int CRC32check(unsigned char *buff, unsigned int size, unsigned int oldcrc)
{
    unsigned int ret=oldcrc, i;
	if (!crc32table[1]) CRC32Init();
    for(i = 0; i < size; i++){
        ret = (ret >> 8) ^ crc32table[buff[i] ^ (ret & 0xFF)];
    }
	//printf("CRC32check: %08X => %08X\n", oldcrc, ~ret);
    return ~ret;
	
}
unsigned int CRC32Check(unsigned char *buff, unsigned int size)
{
	return CRC32check(buff, size, 0xFFFFFFFF);
}
unsigned int CRC32file(char *file)
{
	int fd;
	char *p;
	unsigned char buffer[32768];
	unsigned int size, i, crc, siz;
	char fullpath[MAX_PATH], tmp[MAX_PATH];
	
	crc = ~0xFFFFFFFF;
	if (!strncmp(file, "hdd0", 4)) {
		sprintf(tmp, "hdd0:%s", &file[6]);
		p = strchr(tmp, '/');
		sprintf(fullpath, "pfs0:%s", p);
		*p = 0;
		//printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		printf("viewer: hddpath: %s\n", fullpath);
		//fileXioMount("pfs0:", tmp, FIO_MT_RDONLY);
		if ((fd = fileXioOpen(fullpath, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH)) < 0){
			//fileXioUmount("pfs0:");
			return -1;
		}
		size = fileXioLseek(fd, 0, SEEK_END);
		//printf("viewer: size: %d\n", size);
		fileXioLseek(fd, 0, SEEK_SET);
		for(i=0; i<size; i+=sizeof(buffer)) {
			siz = sizeof(buffer);
			if (i+siz > size) siz = size - i;
			fileXioRead(fd, buffer, siz);
			crc = CRC32check(buffer, siz, ~crc);
		}
		fileXioClose(fd);
		//fileXioUmount("pfs0:");
	} else {
		strcpy(fullpath, file);
		//printf("viewer: mode: %d\nviewer: file: %s\n", mode, file);
		printf("viewer: file: %s\n", file);
		fd = fioOpen(fullpath, O_RDONLY);
		if (fd<0)
			return -1;
		size = fioLseek(fd, 0, SEEK_END);
		//printf("viewer: size: %d\n", size);
		fioLseek(fd, 0, SEEK_SET);
		for(i=0; i<size; i+=sizeof(buffer)) {
			siz = sizeof(buffer);
			if (i+siz > size) siz = size - i;
			fioRead(fd, buffer, siz);
			crc = CRC32check(buffer, siz, ~crc);
		}
		fioClose(fd);
	}
	return crc;
}


//-------------------------------------------------
//メッセージボックス
int MessageBox(const char *Text, const char *Caption, int type)
{
	char MessageText[2048];
	int i,n;
	char *p;
	int tw;
	int ret=0;
	char CaptionText[256];
	char ButtonText[256];
	int DialogType;
	int len;
	int dialog_x;		//ダイアログx位置
	int dialog_y;		//ダイアログy位置
	int dialog_width;	//ダイアログ幅
	int dialog_height;	//ダイアログ高さ
	int sel, redraw=framebuffers;
	int x, y;
	int timeout;
	char tmp[256];		//表示用

	//
	sel=0;
	if(type&MB_DEFBUTTON1) sel=0;
	if(type&MB_DEFBUTTON2) sel=1;
	if(type&MB_DEFBUTTON3) sel=2;
	timeout=-5;
	if(type&MB_USETIMEOUT) timeout=9.5*SCANRATE;
	//メッセージ
	strncpy(MessageText, Text, 2048);
	//\n区切りを\0区切りに変換 n:改行の数
	for(i=0,n=1; MessageText[i]!=0; i++)
		if(MessageText[i]=='\n'){MessageText[i]='\0';n++;}
	//メッセージの一番長い行の文字数を調べる tw:文字数
	p = MessageText;
	tw = 0;
	for(i=0;i<n;i++){
		len = strlen(p);
		if(len>tw) tw=len;
		p += len+1;
	}
	//キャプション
	if(Caption==NULL)
		strcpy(CaptionText, "error");
	else{
		//\nまでをキャプションにする
		strncpy(CaptionText, Caption, 256);
		p = strchr(CaptionText, '\n');
		if(p!=NULL) *p='\0';
	}
	//ダイアログのボタン
	DialogType = type&0xf;
	if(DialogType==MB_OK)
		sprintf(ButtonText, "%s", lang->gen_ok);
	else if(DialogType==MB_OKCANCEL)
		sprintf(ButtonText, " %-10s %-10s", lang->gen_ok, lang->gen_cancel);
	else if(DialogType==MB_YESNOCANCEL)
		sprintf(ButtonText, " %-10s %-10s %-10s", lang->gen_yes, lang->gen_no, lang->gen_cancel);
	else if(DialogType==MB_YESNO)
		sprintf(ButtonText, " %-10s %-10s", lang->gen_yes, lang->gen_no);
	else if(DialogType==MB_MC0MC1CANCEL)
		sprintf(ButtonText, " %-10s %-10s %-10s", "mc0:/", "mc1:/", lang->gen_cancel);
	else
		return 0;
	//ダイアログに表示する最大の文字数
	if(tw<strlen(ButtonText)) tw=strlen(ButtonText);
	if(tw<strlen(CaptionText)) tw=strlen(CaptionText);
	
	dialog_width = FONT_WIDTH*(tw+2);
	dialog_height = FONT_HEIGHT*(n+5);
	dialog_x = (SCREEN_WIDTH-dialog_width)/2;
	dialog_y = (SCREEN_HEIGHT-dialog_height)/2;
	while(1){
		waitPadReady(0, 0);
		if(readpad()){
			if (new_pad) redraw=fieldbuffers;
			if(new_pad & PAD_LEFT){
				sel--;
				if(sel<0) sel=0; 
			}
			else if(new_pad & PAD_RIGHT){
				sel++;
				if(DialogType==MB_OKCANCEL||DialogType==MB_YESNO){
					if(sel>1) sel=1;
				}
				if(DialogType==MB_YESNOCANCEL||DialogType==MB_MC0MC1CANCEL){
					if(sel>2) sel=2;
				}
			}
			else if(new_pad & PAD_CROSS){	//キャンセル
				ret=0;
				break;
			}
			else if(new_pad & PAD_CIRCLE){
				if(DialogType==MB_OK)
					ret=IDOK;
				if(DialogType==MB_OKCANCEL){
					if(sel==0) ret=IDOK;
					if(sel==1) ret=IDCANCEL;
				}
				if(DialogType==MB_YESNOCANCEL){
					if(sel==0) ret=IDYES;
					if(sel==1) ret=IDNO;
					if(sel==2) ret=IDCANCEL;
				}
				if(DialogType==MB_YESNO){
					if(sel==0) ret=IDYES;
					if(sel==1) ret=IDNO;
				}
				if(DialogType==MB_MC0MC1CANCEL){
					if(sel==0) ret=IDMC0;
					if(sel==1) ret=IDMC1;
					if(sel==2) ret=IDCANCEL;
				}
				break;
			}
			else if(new_pad & PAD_SELECT){	//キャンセルにカーソルを移動
				if(DialogType==MB_OKCANCEL||DialogType==MB_YESNO) sel=1;
				if(DialogType==MB_YESNOCANCEL||DialogType==MB_MC0MC1CANCEL) sel=2;
			}
			else if(new_pad & PAD_START){	//OKにカーソルを移動
				sel=0;
			}
			//△ボタンで決定
			if(new_pad & PAD_TRIANGLE){
				if(type&MB_USETRIANGLE){
					if(DialogType==MB_OK)
						ret=IDOK|IDTRIANGLE;
					if(DialogType==MB_OKCANCEL){
						if(sel==0) ret=IDOK|IDTRIANGLE;
						if(sel==1) ret=IDCANCEL|IDTRIANGLE;
					}
					if(DialogType==MB_YESNOCANCEL){
						if(sel==0) ret=IDYES|IDTRIANGLE;
						if(sel==1) ret=IDNO|IDTRIANGLE;
						if(sel==2) ret=IDCANCEL|IDTRIANGLE;
					}
					if(DialogType==MB_YESNO){
						if(sel==0) ret=IDYES|IDTRIANGLE;
						if(sel==1) ret=IDNO|IDTRIANGLE;
					}
					if(DialogType==MB_MC0MC1CANCEL){
						if(sel==0) ret=IDMC0|IDTRIANGLE;
						if(sel==1) ret=IDMC1|IDTRIANGLE;
						if(sel==2) ret=IDCANCEL|IDTRIANGLE;
					}
					break;
				}
			}
			//□ボタンで決定
			if(new_pad & PAD_SQUARE){
				if(type&MB_USESQUARE){
					if(DialogType==MB_OK)
						ret=IDOK|IDSQUARE;
					if(DialogType==MB_OKCANCEL){
						if(sel==0) ret=IDOK|IDSQUARE;
						if(sel==1) ret=IDCANCEL|IDSQUARE;
					}
					if(DialogType==MB_YESNOCANCEL){
						if(sel==0) ret=IDYES|IDSQUARE;
						if(sel==1) ret=IDNO|IDSQUARE;
						if(sel==2) ret=IDCANCEL|IDSQUARE;
					}
					if(DialogType==MB_YESNO){
						if(sel==0) ret=IDYES|IDSQUARE;
						if(sel==1) ret=IDNO|IDSQUARE;
					}
					if(DialogType==MB_MC0MC1CANCEL){
						if(sel==0) ret=IDMC0|IDSQUARE;
						if(sel==1) ret=IDMC1|IDSQUARE;
						if(sel==2) ret=IDCANCEL|IDSQUARE;
					}
					break;
				}
			}
		}
		if (timeout == 0) {
			ret = IDTIMEOUT;
			break;
		}

		if (redraw) {
			// 描画開始
			drawDialogTmp(dialog_x, dialog_y,
				dialog_x+dialog_width, dialog_y+dialog_height,
				setting->color[COLOR_BACKGROUND], setting->color[COLOR_FRAME]);
			itoLine(setting->color[COLOR_FRAME], dialog_x+FONT_WIDTH, dialog_y+FONT_HEIGHT*1.75, 0,
				setting->color[COLOR_FRAME], dialog_x+dialog_width-FONT_WIDTH, dialog_y+FONT_HEIGHT*1.75, 0);
			//キャプション
			x = dialog_x+FONT_WIDTH*1;
			y = dialog_y+FONT_HEIGHT*0.5;
			printXY(CaptionText, x, y, setting->color[COLOR_TEXT], TRUE);
			//メッセージ
			x = dialog_x+FONT_WIDTH*1;
			y = dialog_y+FONT_HEIGHT*2.5;
			p = MessageText;
			for(i=0;i<n;i++){
				printXY(p, x, y, setting->color[COLOR_TEXT], TRUE);
				p += strlen(p)+1;
				y += FONT_HEIGHT;
			}
			y += FONT_HEIGHT;	//空行
			//ボタン
			x = dialog_x+(dialog_width-(strlen(ButtonText)*FONT_WIDTH))/2;
			printXY(ButtonText, x, y, setting->color[COLOR_TEXT], TRUE);
			//カーソル
			if(DialogType!=MB_OK){
				x = dialog_x+(dialog_width-(strlen(ButtonText)*FONT_WIDTH))/2 + (sel*FONT_WIDTH*11);
				printXY(">", x, y, setting->color[COLOR_TEXT], TRUE);
			}
			// 操作説明
			x = FONT_WIDTH*1;
			y = SCREEN_MARGIN+(MAX_ROWS+4)*FONT_HEIGHT;
			itoSprite(setting->color[COLOR_BACKGROUND],
				0, y,
				SCREEN_WIDTH, y+FONT_HEIGHT, 0);
			sprintf(tmp,"○:%s ×:%s", lang->gen_ok, lang->gen_cancel);
			printXY(tmp, x, y, setting->color[COLOR_TEXT], TRUE);
			drawScr();
			redraw--;
		} else {
			itoVSync();
		}
		if (timeout > 0) timeout--;
	}
	return ret;
}

char LBF_VER[64] = LBFN_VER;
