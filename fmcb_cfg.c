// ◯ 
#include "launchelf.h"

//デフォルトの設定の値
enum
{
	MAX_OSDSYS_ITEMS = 100,
	MAX_BOOT_BUTTONS = 17,
	
};


typedef struct {
} FMCB_SETTING;
FMCB_SETTING *fmcb = NULL;

typedef struct {
	char name[128];
	int mask;
	void *callproc;
} MENUITEM_DATA;

//-------------------------------------------------
//FreeMcBOOT Configture
void fmcb_cfg(char *mainMsg)
{
	cnf_init();
	cnf_mode(1);
	cnf_load("mc0:/SYS-CONF/FREEMCB.CNF");
	/*	「Free McBoot Configurator 1.3 beta 6」 based
				Load CNF from: 
				Configutre launcher buttons...
					..
					BUTTON: 
					PATH1:
					PATH2:
					PATH3:
					CHECK ALL LAUNCHER SETTINGS
				Configutre OSDSYS options...
					..
					Hacked OSDSYS:
					Configutre Item xx:
						..
						Name:
						Path1:
						Path2:
						Path3:
					Configutre Scrolling Options...
						..
						Scroll Menu:
						Displayed Items:
						Menu y:
						Cursor Max Velocity:
						Cursor Acceleration:
						Left Cursor:
						Right Cursor:
						Top Delimiter:
						Bottom Delimiter:
					Video Mode:
					Skip MC update check:
					Skip HDD update check:
					Skip Disc Boot:
					Skip Sony Logo:
					Go to Browser:
					Selected Color:		R:00  G:00  B:00  A:00  ■
					Unselected Color:	R:00  G:00  B:00  A:00  ■
					Menu X:
					Enter:				X:        Y:
					Version:			X:        Y:
				Configutre ESR path...
					..
					Path1:
					Path2:
					Path3:
				FastBoot:
				Debug Screen:
				Pad Delay:
				Save CNF to:
				Return

CNF_version
Debug_Screen
FastBoot
ESR_Path_E1
ESR_Path_E2
ESR_Path_E3
pad_delay
LK_Auto_E1
LK_Circle_E1
LK_Cross_E1
LK_Square_E1
LK_Triangle_E1
LK_L1_E1
LK_R1_E1
LK_L2_E1
LK_R2_E1
LK_L3_E1
LK_R3_E1
LK_Up_E1
LK_Down_E1
LK_Left_E1
LK_Right_E1
LK_Start_E1
LK_Select_E1
hacked_OSDSYS
OSDSYS_video_mode
OSDSYS_Skip_Disc
OSDSYS_Skip_Logo
OSDSYS_Inner_Browser
OSDSYS_selected_color
OSDSYS_unselected_color
OSDSYS_scroll_menu
OSDSYS_menu_x
OSDSYS_menu_y
OSDSYS_enter_x
OSDSYS_enter_y
OSDSYS_version_x
OSDSYS_version_y
OSDSYS_cursor_max_velocity
OSDSYS_cursor_acceleration
OSDSYS_left_cursor
OSDSYS_right_cursor
OSDSYS_menu_top_delimiter
OSDSYS_menu_bottom_delimiter
OSDSYS_num_displayed_items
OSDSYS_Skip_MC
OSDSYS_Skip_HDD
name_OSDSYS_ITEM_?
path?_OSDSYS_ITEM_?

	*/
	cnf_free();
	return;
}
