#include "launchelf.h"

LANGUAGE *lang;

//-------------------------------------------------
void InitLanguage(void)
{
	lang = (LANGUAGE*)malloc(sizeof(LANGUAGE));
	SetLanguage(LANG_ENGLISH);
}

//-------------------------------------------------
void FreeLanguage(void)
{
	free(lang);
}

//-------------------------------------------------
void SetLanguage(const int langID)
{
	int l;

	l=langID;
	if(l<LANG_ENGLISH) l=LANG_ENGLISH;
	if(l>=NUM_LANG) l=LANG_ENGLISH;
	memset(lang, 0, sizeof(LANGUAGE));

	if(l==LANG_ENGLISH){
		//general
		strcpy(lang->gen_ok, "OK");
		strcpy(lang->gen_cancel, "CANCEL");
		strcpy(lang->gen_yes, "YES");
		strcpy(lang->gen_no, "NO");
		strcpy(lang->gen_loading, "Loading...");
		strcpy(lang->gen_decoding, "Processing...");
		//main
		{
		strcpy(lang->main_launch_hint, "PUSH ANY BUTTON or D-PAD!");
		strcpy(lang->main_loadhddmod, "Loading HDD Modules...");
		strcpy(lang->main_loadftpmod, "Loading FTP Modules...");
		strcpy(lang->main_notfound, " is Not Found.");
		strcpy(lang->main_readsystemcnf, "Reading SYSTEM.CNF...");
		strcpy(lang->main_failed, "Failed");
		strcpy(lang->main_nodisc, "No Disc");
		strcpy(lang->main_detectingdisc, "Detecting Disc");
		strcpy(lang->main_stopdisc, "Stop Disc");
		}
		//filer
		{
		strcpy(lang->filer_menu_copy, "Copy");
		strcpy(lang->filer_menu_cut, "Cut");
		strcpy(lang->filer_menu_paste, "Paste");
		strcpy(lang->filer_menu_delete, "Delete");
		strcpy(lang->filer_menu_rename, "Rename");
		strcpy(lang->filer_menu_newdir, "New Dir");
		strcpy(lang->filer_menu_getsize, "Get Size");
		strcpy(lang->filer_menu_exportpsu, "Export psu");
		strcpy(lang->filer_menu_importpsu, "Import psu");
		strcpy(lang->filer_menu_compress, "Compression");
		strcpy(lang->filer_menu_editor, "Look");
		strcpy(lang->filer_overwrite, "Overwrite?");
		strcpy(lang->filer_not_elf, "This file isn't ELF.");
#ifdef ENABLE_PSB
		strcpy(lang->filer_execute_psb, "Execute?");
#endif
		strcpy(lang->filer_not_fnt, "This file isn't FNT.");
		strcpy(lang->filer_copy_to_clip, "Copied to the Clipboard");
		strcpy(lang->filer_delete, "Delete?");
		strcpy(lang->filer_deletemarkfiles, "Mark Files Delete?");
		strcpy(lang->filer_deleting, "deleting...");
		strcpy(lang->filer_deletefailed, "Delete Failed");
		strcpy(lang->filer_renamefailed, "Rename Failed");
		strcpy(lang->filer_pasting, "Pasting...");
		strcpy(lang->filer_pastefailed, "Paste Failed");
		strcpy(lang->filer_direxists, "directory already exists");
		strcpy(lang->filer_newdirfailed, "NewDir Failed");
		strcpy(lang->filer_checkingsize, "Checking Size...");
		strcpy(lang->filer_getsizefailed, "Get Size Failed");
		strcpy(lang->filer_export, "Export?");
		strcpy(lang->filer_exportmarkfiles, "Mark Files Export?");
		strcpy(lang->filer_exporting, "Exporting...");
		strcpy(lang->filer_exportfailed, "Export psu Failed");
		strcpy(lang->filer_exportto, "Export to");
		strcpy(lang->filer_import, "Import?");
		strcpy(lang->filer_importmarkfiles, "Mark Files Import?");
		strcpy(lang->filer_importing, "Importing...");
		strcpy(lang->filer_importfailed, "Import psu Failed");
		strcpy(lang->filer_importto, "Import to");
		strcpy(lang->filer_keyboard_hint, "○:OK ×:Back L1:Left R1:Right START:Enter");
		strcpy(lang->filer_anyfile_hint1, "○:OK △:Up ×:Mark □:RevMark L1:TitleOFF R1:Menu R2:GetSize");
		strcpy(lang->filer_anyfile_hint2, "○:OK △:Up ×:Mark □:RevMark L1:TitleON  R1:Menu R2:GetSize");
		strcpy(lang->filer_elffile_hint1, "○:OK ×:Cancel △:Up □:*->ELF");
		strcpy(lang->filer_elffile_hint2, "○:OK ×:Cancel △:Up □:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "○:OK ×:Cancel △:Up □:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "○:OK ×:Cancel △:Up □:FNT->*");
		strcpy(lang->filer_irxfile_hint1, "○:OK ×:Cancel △:Up □:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "○:OK ×:Cancel △:Up □:IRX->*");
		strcpy(lang->filer_dir_hint, "○:OK ×:Cancel △:Up START:Choose");
		strcpy(lang->filer_l2popup_detail, "Detail Mode");
		strcpy(lang->filer_l2popup_dirsize, "Get DirSize");
		strcpy(lang->filer_l2popup_icon, "Icon");
		strcpy(lang->filer_l2popup_flicker, "Flicker Control");
		strcpy(lang->filer_l2popup_sort, "Sort Mode");
		}
		//editor
		{
		strcpy(lang->editor_viewer_help, "○:Count □:TAB(%d) △/×:Exit L1:Left R1:Right START:TAB/CR/LF");
		strcpy(lang->editor_viewer_error1, "Can not open the file.");
		strcpy(lang->editor_viewer_error2, "Out of memory.");
		strcpy(lang->editor_l2popup_tabmode, "TAB Space Mode");
		strcpy(lang->editor_l2popup_charset, "Charactor Set");
		strcpy(lang->editor_l2popup_linenum, "Line Number Count");
		strcpy(lang->editor_l2popup_flicker, "Flicker Control");
		strcpy(lang->editor_image_help, "○:FullScreen △/×:Exit  SIZE:%d×%d");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "Save Failed");
		strcpy(lang->conf_saveconfig, "Save Config");
		strcpy(lang->conf_loadconfig, "Load Config");
		strcpy(lang->conf_initializeconfig, "Initialize Config");

		strcpy(lang->conf_setting_button,  "BUTTON SETTING");
		strcpy(lang->conf_setting_filer,   "FILER SETTING");
		strcpy(lang->conf_setting_color,   "COLOR SETTING");
		strcpy(lang->conf_setting_screen,  "SCREEN SETTING");
		strcpy(lang->conf_setting_font,    "FONT SETTING");
		strcpy(lang->conf_setting_device,  "DEVICE SETTING");
		strcpy(lang->conf_setting_view,    "VIEWER SETTING");
		strcpy(lang->conf_setting_misc,    "MISC SETTING");
		strcpy(lang->conf_ok, "OK");
		strcpy(lang->conf_cancel, "CANCEL");

		//button
		strcpy(lang->conf_launch_btnnum, "NUMBER OF SETTING");
		strcpy(lang->conf_launch_name, "DISPLAY NAME");
		strcpy(lang->conf_launch_padmsk, "PAD");
		strcpy(lang->conf_launch_elfnum, "NUMBER OF PATH");
		strcpy(lang->conf_launch_path, "PATH");
		strcpy(lang->conf_launch_list, "CHECK ALL PAD SETTING");
		strcpy(lang->conf_buttonsettinginit, "LAUNCHER SETTING INIT");
		strcpy(lang->conf_launch_pad0, "Push the button please(Arrow key is cancel).");
		strcpy(lang->conf_launch_pad2, "DEFAULT is can't change the pad.");
		//color
		strcpy(lang->conf_background, "BACK GROUND   ");
		strcpy(lang->conf_frame, "FRAME         ");
		strcpy(lang->conf_normaltext, "NORMAL TEXT   ");
		strcpy(lang->conf_highlighttext, "HIGHLIGHT TEXT");
		strcpy(lang->conf_disabletext, "GRAY TEXT     ");
		strcpy(lang->conf_folder, "FOLDER        ");
		strcpy(lang->conf_file, "FILE          ");
		strcpy(lang->conf_ps2save, "PS2 SAVE      ");
		strcpy(lang->conf_ps1save, "PS1 SAVE      ");
		strcpy(lang->conf_elffile, "ELF FILE      ");
		strcpy(lang->conf_psufile, "PSU FILE      ");
		strcpy(lang->conf_outside, "OUTSIDE       ");
		strcpy(lang->conf_flicker_alpha, "FLICKER ALPHA");
		strcpy(lang->conf_presetcolor, "LOAD PRESET COLOR");
		//screen
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_displayname,		"NAME       ");
		strcpy(lang->conf_screen_scan,		"SCREEN SIZE");
		strcpy(lang->conf_screen_scan_crop, "NORMAL");
		strcpy(lang->conf_screen_scan_full, "FULL");
		strcpy(lang->conf_resolution,		"RESOLUTION ");
		strcpy(lang->conf_depth,			"COLOR DEPTH");
		strcpy(lang->conf_dither,			"DITHER     ");
		strcpy(lang->conf_interlace,		"INTERLACE  ");
		strcpy(lang->conf_ffmode,			"FFMODE     ");
		strcpy(lang->conf_gsedit_default,	"LOAD DEFAULT");
		strcpy(lang->conf_screen_x, "SCREEN X");
		strcpy(lang->conf_screen_y, "SCREEN Y");
		strcpy(lang->conf_flickercontrol, "FLICKER CONTROL");
		strcpy(lang->conf_screensettinginit, "SCREEN SETTING INIT");
		strcpy(lang->conf_screenmodemsg1, "You want to change the setting.\nIf screen isn't displayed,\nreturn to old setting\n10 seconds later.");
		strcpy(lang->conf_screenmodemsg2, "Changed the setting.\nDo you apply\nthe this screen setting?\n(return to old setting\nwithout reply 10 seconds later.)");
		//network
		strcpy(lang->conf_ipaddress, "IP ADDRESS");
		strcpy(lang->conf_netmask, "NETMASK   ");
		strcpy(lang->conf_gateway, "GATEWAY   ");
		strcpy(lang->conf_ipsettinginit, "LOAD DEFAULT IPCONFIG");
		strcpy(lang->conf_ipsaved, "Saved");
		strcpy(lang->conf_ipsavefailed, "Save Failed");
		//font
		strcpy(lang->conf_AsciiFont, "ASCII FONT");
		strcpy(lang->conf_KanjiFont, "KANJI FONT");
		strcpy(lang->conf_CharMargin, "CHAR MARGIN");
		strcpy(lang->conf_LineMargin, "LINE MARGIN");
		strcpy(lang->conf_FontBold, "FONT BOLD");
		strcpy(lang->conf_FontHalf, "FIX FONT WIDTH ");
		strcpy(lang->conf_FontVHalf, "FIX FONT HEIGHT");
		strcpy(lang->conf_FontScaler, "FIX FONT MODE");
		strcpy(lang->conf_FontScaler_A, "Faster (nearest/composite)");
		strcpy(lang->conf_FontScaler_B, "Normal (bilinear)");
		strcpy(lang->conf_FontScaler_C, "4x AA (and bilinear)");
		strcpy(lang->conf_AsciiMarginTop,  "ASCII MARGIN TOP ");
		strcpy(lang->conf_AsciiMarginLeft, "ASCII MARGIN LEFT");
		strcpy(lang->conf_KanjiMarginTop,  "KANJI MARGIN TOP ");
		strcpy(lang->conf_KanjiMarginLeft, "KANJI MARGIN LEFT");
		strcpy(lang->conf_fontsettinginit, "FONT SETTING INIT");
		//viewer
		strcpy(lang->conf_linenumber, "DISPLAY LINE NUMBER IN TEXT");
		strcpy(lang->conf_tabspaces,  "WIDTH OF TAB IN TEXT");
		strcpy(lang->conf_chardisp,   "DISPLAY RETURN AND TAB CODE IN TEXT");
		strcpy(lang->conf_fullscreen, "FULLSCREEN MODE IN IMAGE");
		strcpy(lang->conf_viewerinit, "VIEWER SETTING INIT");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "日本語");
		strcpy(lang->conf_timeout, "TIME OUT");
		strcpy(lang->conf_disc_control, "DISC CONTROL");
		strcpy(lang->conf_print_only_filename, "PRINT ONLY FILENAME");
		strcpy(lang->conf_print_all_filename, "DISPLAY ALL FILES");
		strcpy(lang->conf_fileicon, "FILEICON");
		strcpy(lang->conf_disc_ps2save_check, "DISC PS2SAVE CHECK");
		strcpy(lang->conf_disc_elf_check, "DISC ELF CHECK");
		strcpy(lang->conf_file_ps2save_check, "FILE PS2SAVE CHECK");
		strcpy(lang->conf_file_elf_check, "FILE ELF CHECK");
		strcpy(lang->conf_export_dir, "EXPORT DIR");
		strcpy(lang->conf_defaulttitle, "DEFAULT SHOW TITLE");
		strcpy(lang->conf_defaultdetail, "DEFAULT SHOW DETAIL");
		strcpy(lang->conf_defaultdetail_none, "NONE");
		strcpy(lang->conf_defaultdetail_size, "SIZE");
		strcpy(lang->conf_defaultdetail_modifytime, "MODIFYTIME");
		strcpy(lang->conf_defaultdetail_both, "SIZE AND MODIFYTIME");
		strcpy(lang->conf_sort_type, "FILELIST SORT");
		strcpy(lang->conf_sort_types[0], "NONE");
		strcpy(lang->conf_sort_types[1], "FILENAME");
		strcpy(lang->conf_sort_types[2], "EXTENSION");
		strcpy(lang->conf_sort_types[3], "GAMETITLE");
		strcpy(lang->conf_sort_types[4], "SIZE");
		strcpy(lang->conf_sort_types[5], "TIMESTAMP");
		strcpy(lang->conf_sort_dir, "LIST TOP FOLDER");// always display folder in the list top
		strcpy(lang->conf_sort_ext, "LIST TOP ELF"); // always display ELF in the file top
		strcpy(lang->conf_usbmass_use, "USE EXTEND USB_MASS");
		strcpy(lang->conf_usbmass_path, "USB_MASS.IRX");
		strcpy(lang->conf_usbd_use, "USE EXTEND USB_driver");
		strcpy(lang->conf_usbd_path, "USBD.IRX");
		strcpy(lang->conf_usbmass_devs, "ADD USB_MASS DEVICES");
		strcpy(lang->conf_usbkbd_use, "USE USB KEYBOARD");
		strcpy(lang->conf_usbkbd_path, "PS2KBD.IRX");
		strcpy(lang->conf_usbmouse_use, "USE USB MOUSE");
		strcpy(lang->conf_usbmouse_path, "PS2MOUSE.IRX");
		strcpy(lang->conf_miscsettinginit, "MISC SETTING INIT");
		strcpy(lang->conf_filersettinginit, "FILER SETTING INIT");
		strcpy(lang->conf_devicesettinginit, "DEVICE SETTING INIT");
		//gsconfig
		{
		strcpy(lang->gs_easymode,		"EASY MODE");
		strcpy(lang->gs_detailmode,		"DETAIL MODE");
		strcpy(lang->gs_autoapply,		"CHANGE WITH APPLY");
		strcpy(lang->gs_gsinit,			"INIT GSCONFIG");
		strcpy(lang->gs_ok,				"OK");
		strcpy(lang->gs_cancel,			"CANCEL");

		strcpy(lang->gs_number,			"NUMBER OF SETTING");
		strcpy(lang->gs_name,			"DISPLAY NAME");
		strcpy(lang->gs_width,			"SCREEN WIDTH ");
		strcpy(lang->gs_height,			"SCREEN HEIGHT");
		strcpy(lang->gs_left,			"HORIZONTAL OFFSET");
		strcpy(lang->gs_top,			"VERTICAL OFFSET  ");
		strcpy(lang->gs_mag_x,			"X RESIZE");
		strcpy(lang->gs_mag_y,			"Y RESIZE");
		strcpy(lang->gs_depth,			"COLOR DEPTH");
		strcpy(lang->gs_bufferwidth,	"BUFFER WIDTH");
		strcpy(lang->gs_x1,				"X1");
		strcpy(lang->gs_y1,				"Y1");
		strcpy(lang->gs_x2,				"X2");
		strcpy(lang->gs_y2,				"Y2");
		strcpy(lang->gs_zleft,			"Z BUFFER LEFT");
		strcpy(lang->gs_ztop,			"Z BUFFER TOP");
		strcpy(lang->gs_zdepth,			"Z DEPTH");
		strcpy(lang->gs_dither,			"DITHER");
		strcpy(lang->gs_interlace,		"INTERLACE");
		strcpy(lang->gs_ffmode,			"FFMODE");
		strcpy(lang->gs_vmode,			"VMODE");
		strcpy(lang->gs_vesa,			"VESA");
		strcpy(lang->gs_double,			"DOUBLE BUFFER");
		strcpy(lang->gs_f0_left,		"1ST FRAME LEFT");
		strcpy(lang->gs_f0_top,			"1ST FRAME TOP ");
		strcpy(lang->gs_f1_left,		"2ND FRAME LEFT");
		strcpy(lang->gs_f1_top,			"2ND FRAME TOP ");
		strcpy(lang->gs_preset,			"INIT THIS SETTING");
		strcpy(lang->gs_init,			"LOAD DEFAULT SETTING");
		strcpy(lang->gse_convert,		"OVERRIDE WRITE TO %s");
		strcpy(lang->gs_vramsize,		"ABOUT THE VRAM SIZE");

		strcpy(lang->gs_msg_0,			"In case of setting to system crash. continue the setting?");
		
		strcpy(lang->gs_prev,			"Prev");
		strcpy(lang->gs_next,			"Next");
		strcpy(lang->gs_copy,			"Copy");
		strcpy(lang->gs_paste,			"Paste");
		strcpy(lang->gs_apply,			"Apply");
		strcpy(lang->gs_default,		"Default");
		}
		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "Edit");
		strcpy(lang->conf_clear, "Clear");
		strcpy(lang->conf_add, "Add");
		strcpy(lang->conf_away, "Sub");
		strcpy(lang->conf_change, "Change");
		strcpy(lang->conf_up, "Up");
		strcpy(lang->conf_detail, "Detail");
		strcpy(lang->conf_fast, "Fast");
		strcpy(lang->conf_default, "Default");
		}
	}
	if(l==LANG_JAPANESE){
		//general
		strcpy(lang->gen_ok, "決定");
		strcpy(lang->gen_cancel, "キャンセル");
		strcpy(lang->gen_yes, "はい");
		strcpy(lang->gen_no, "いいえ");
		strcpy(lang->gen_loading, "読み込み中です...");
		strcpy(lang->gen_decoding, "準備中です...");
		//main
		{
		strcpy(lang->main_launch_hint, "ボタンで起動 or 十\字キーで選択");
		strcpy(lang->main_loadhddmod, "HDD を起動しています");
		strcpy(lang->main_loadftpmod, "FTP を起動しています");
		strcpy(lang->main_notfound, " が見つかりません");
		strcpy(lang->main_readsystemcnf, "SYSTEM.CNF を読み込み中");
		strcpy(lang->main_failed, "SYSTEM.CNF を読み込み失敗しました");
		strcpy(lang->main_nodisc, "ディスクがありません");
		strcpy(lang->main_detectingdisc, "ディスク検出中");
		strcpy(lang->main_stopdisc, "ディスク停止");
		}
		//filer
		{
		strcpy(lang->filer_menu_copy, "コピー");
		strcpy(lang->filer_menu_cut, "切り取り");
		strcpy(lang->filer_menu_paste, "貼\り付け");
		strcpy(lang->filer_menu_delete, "削除");
		strcpy(lang->filer_menu_rename, "リネーム");
		strcpy(lang->filer_menu_newdir, "フォルダ作成");
		strcpy(lang->filer_menu_getsize, "サイズ取得");
		strcpy(lang->filer_menu_exportpsu, "エクスポート");
		strcpy(lang->filer_menu_importpsu, "インポート");
		strcpy(lang->filer_menu_compress, "圧縮");
		strcpy(lang->filer_menu_editor, "表\示");
		strcpy(lang->filer_overwrite, "上書きしますか?");
		strcpy(lang->filer_not_elf, "ELFファイルではありません");
#ifdef ENABLE_PSB
		strcpy(lang->filer_execute_psb, "実行しますか?");
#endif
		strcpy(lang->filer_not_fnt, "FNTファイルではありません");
		strcpy(lang->filer_copy_to_clip, "クリップボードへコピーしました");
		strcpy(lang->filer_delete, "削除しますか?");
		strcpy(lang->filer_deletemarkfiles, "マークしたファイルを削除しますか?");
		strcpy(lang->filer_deleting, "削除しています");
		strcpy(lang->filer_deletefailed, "削除を失敗しました");
		strcpy(lang->filer_renamefailed, "リネームを失敗しました");
		strcpy(lang->filer_pasting, "貼\り付けしています");
		strcpy(lang->filer_pastefailed, "貼\り付け失敗しました");
		strcpy(lang->filer_direxists, "フォルダは既に存在しています");
		strcpy(lang->filer_newdirfailed, "フォルダ作成を失敗しました");
		strcpy(lang->filer_checkingsize, "サイズを計算しています");
		strcpy(lang->filer_getsizefailed, "サイズ取得失敗しました");
		strcpy(lang->filer_export, "エクスポートしますか?");
		strcpy(lang->filer_exportmarkfiles, "マークしたファイルをエクスポートしますか?");
		strcpy(lang->filer_exporting, "エクスポートしています");
		strcpy(lang->filer_exportfailed, "エクスポート失敗しました");
		strcpy(lang->filer_exportto, "エクスポートした場所");
		strcpy(lang->filer_import, "インポートしますか?");
		strcpy(lang->filer_importmarkfiles, "マークしたファイルをインポートしますか?");
		strcpy(lang->filer_importing, "インポートしています");
		strcpy(lang->filer_importfailed, "インポート失敗しました");
		strcpy(lang->filer_importto, "インポートした場所");
		strcpy(lang->filer_keyboard_hint, "○:決定 ×:削除 L1:左へ R1:右へ");
		strcpy(lang->filer_anyfile_hint1, "○:決定 △:上へ ×:マーク □:マーク反転 L1:タイトル R1:メニュー");
		strcpy(lang->filer_anyfile_hint2, "○:決定 △:上へ ×:マーク □:マーク反転 L1:タイトル R1:メニュー");
		strcpy(lang->filer_elffile_hint1, "○:決定 ×:キャンセル △:上へ □:*->ELF");
		strcpy(lang->filer_elffile_hint2, "○:決定 ×:キャンセル △:上へ □:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "○:決定 ×:キャンセル △:上へ □:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "○:決定 ×:キャンセル △:上へ □:FNT->*");
		strcpy(lang->filer_irxfile_hint1, "○:決定 ×:キャンセル △:上へ □:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "○:決定 ×:キャンセル △:上へ □:IRX->*");
		strcpy(lang->filer_dir_hint, "○:決定 ×:キャンセル △:上へ Start:フォルダ選択");
		strcpy(lang->filer_l2popup_detail, "詳細表\示 切り替え");
		strcpy(lang->filer_l2popup_dirsize, "フォルダサイズ表\示");
		strcpy(lang->filer_l2popup_icon, "アイコン表\示");
		strcpy(lang->filer_l2popup_flicker, "フリッカーコントロール");
		strcpy(lang->filer_l2popup_sort, "リスト並び");
		}
		//editor
		{
		strcpy(lang->editor_viewer_help, "○:行番号 □:TAB(%d) △/×:戻る L1:左へ R1:右へ Start:TAB/改行");
		strcpy(lang->editor_viewer_error1, "ファイルのオープンに失敗しました");
		strcpy(lang->editor_viewer_error2, "メモリが足りません");
		strcpy(lang->editor_l2popup_tabmode, "TABモード変更");
		strcpy(lang->editor_l2popup_charset, "文字コード変更");
		strcpy(lang->editor_l2popup_linenum, "行番号表\示");
		strcpy(lang->editor_l2popup_flicker, "フリッカーコントロール");
		strcpy(lang->editor_image_help, "○:フルスクリーン △/×:戻る サイズ:%d×%d");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "設定の保存に失敗しました");
		strcpy(lang->conf_saveconfig, "設定保存");
		strcpy(lang->conf_loadconfig, "設定読み込み");
		strcpy(lang->conf_initializeconfig, "設定初期化");

		strcpy(lang->conf_setting_button,  "ランチャー");
		strcpy(lang->conf_setting_filer,   "ファイラー");
		strcpy(lang->conf_setting_color,   "配色設定");
		strcpy(lang->conf_setting_screen,  "画面設定");
		strcpy(lang->conf_setting_font,    "フォント設定");
		strcpy(lang->conf_setting_device,  "デバイス設定");
		strcpy(lang->conf_setting_view,    "ビューア設定");
		strcpy(lang->conf_setting_misc,    "その他");
		strcpy(lang->conf_ok, "保存して戻る");
		strcpy(lang->conf_cancel, "キャンセル");

		//button
		strcpy(lang->conf_launch_btnnum, "登録番号");
		strcpy(lang->conf_launch_name, "設定名");
		strcpy(lang->conf_launch_padmsk, "ボタン");
		strcpy(lang->conf_launch_elfnum, "実行番号");
		strcpy(lang->conf_launch_path, "ファイル");
		strcpy(lang->conf_launch_list, "全てのボタン設定を確認する");
		strcpy(lang->conf_buttonsettinginit, "「ランチャー設定」を初期化する");
		strcpy(lang->conf_launch_pad0, "対応させるボタンを押して下さい(方向キーでキャンセル)");
		strcpy(lang->conf_launch_pad2, "DEFAULT は変更できません");
		//color
		strcpy(lang->conf_background,    "背景の色      ");
		strcpy(lang->conf_frame,         "フレームの色  ");
		strcpy(lang->conf_normaltext,    "テキスト      ");
		strcpy(lang->conf_highlighttext, "テキスト強調  ");
		strcpy(lang->conf_disabletext,   "テキスト無効  ");
		strcpy(lang->conf_folder,        "フォルダ      ");
		strcpy(lang->conf_file,          "ファイル      ");
		strcpy(lang->conf_ps2save,       "PS2 セーブ    ");
		strcpy(lang->conf_ps1save,       "PS1 セーブ    ");
		strcpy(lang->conf_elffile,       "ELF ファイル  ");
		strcpy(lang->conf_psufile,       "PSU ファイル  ");
		strcpy(lang->conf_outside,       "画面外の色    ");
		strcpy(lang->conf_flicker_alpha, "フリッカーコントロールの不透明度");
		strcpy(lang->conf_presetcolor,   "配色設定の初期化");
		//screen
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_displayname,		"表\示名        ");
		strcpy(lang->conf_screen_scan,		"画面サイズ    ");
		strcpy(lang->conf_screen_scan_crop, "ノーマル");
		strcpy(lang->conf_screen_scan_full, "フル");
		strcpy(lang->conf_resolution,		"画面解像度    ");
		strcpy(lang->conf_depth,			"色深度        ");
		strcpy(lang->conf_dither,			"ディザリング  ");
		strcpy(lang->conf_interlace,		"インターレース");
		strcpy(lang->conf_ffmode,			"FFMODE        ");
		strcpy(lang->conf_gsedit_default, 	"デフォルトに戻す");
		strcpy(lang->conf_screen_x, "画面位置 X");
		strcpy(lang->conf_screen_y, "画面位置 Y");
		strcpy(lang->conf_flickercontrol, "フリッカーコントロール");
		strcpy(lang->conf_screensettinginit, "「画面設定」を初期化する");
		strcpy(lang->conf_screenmodemsg1, "画面設定を変更します。\n表\示されない場合でも、\n約10秒後に元の画面に戻ります。");
		strcpy(lang->conf_screenmodemsg2, "画面設定を変更しました。\nこの設定を適用してよろしいですか？\n(応答がない場合は約10秒後に\n元の画面に戻ります。)");
		//network
		strcpy(lang->conf_ipaddress, "IPアドレス  ");
		strcpy(lang->conf_netmask, "ネットマスク");
		strcpy(lang->conf_gateway, "ゲートウェイ");
		strcpy(lang->conf_ipsettinginit, "デフォルトに戻す");
		strcpy(lang->conf_ipsaved, "保存完了");
		strcpy(lang->conf_ipsavefailed, "保存失敗");
		//font
		strcpy(lang->conf_AsciiFont, "アスキーフォント");
		strcpy(lang->conf_KanjiFont, "漢字フォント    ");
		strcpy(lang->conf_CharMargin, "文字の間隔");
		strcpy(lang->conf_LineMargin, "行の間隔  ");
		strcpy(lang->conf_FontBold, "太字にする");
		strcpy(lang->conf_FontHalf, "フォントの幅を補正する　");
		strcpy(lang->conf_FontVHalf, "フォントの高さを補正する");
		strcpy(lang->conf_FontScaler, "フォント補正モード");
		strcpy(lang->conf_FontScaler_A, "高速");
		strcpy(lang->conf_FontScaler_B, "標準");
		strcpy(lang->conf_FontScaler_C, "高画質");
		strcpy(lang->conf_AsciiMarginTop, "アスキーフォントの上の間隔");
		strcpy(lang->conf_AsciiMarginLeft, "アスキーフォントの左の間隔");
		strcpy(lang->conf_KanjiMarginTop, "漢字フォントの上の間隔");
		strcpy(lang->conf_KanjiMarginLeft, "漢字フォントの左の間隔");
		strcpy(lang->conf_fontsettinginit, "「フォント設定」を初期化する");
		//viewer
		strcpy(lang->conf_linenumber, "テキストの行番号表\示");
		strcpy(lang->conf_tabspaces,  "テキストのTABの文字数(半角換算)");
		strcpy(lang->conf_chardisp,   "テキストの改行やTABのマーク表\示");
		strcpy(lang->conf_fullscreen, "イメージのフルスクリーンモード");
		strcpy(lang->conf_viewerinit, "「ビューア設定」を初期化する");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "日本語");
		strcpy(lang->conf_timeout, "オートロードまでの秒数");
		strcpy(lang->conf_disc_control, "ディスクを停止する");
		strcpy(lang->conf_print_only_filename, "ファイル名のみ表\示");
		strcpy(lang->conf_print_all_filename, "すべてのファイルを表\示");
		strcpy(lang->conf_fileicon, "FileBrowserのアイコン");
		strcpy(lang->conf_disc_ps2save_check, "CD/DVDのときセーブデータか調べる");
		strcpy(lang->conf_disc_elf_check, "CD/DVDのときELFか調べる");
		strcpy(lang->conf_file_ps2save_check, "CD/DVD/MC以外のときセーブデータか調べる");
		strcpy(lang->conf_file_elf_check, "CD/DVD以外のときELFか調べる");
		strcpy(lang->conf_export_dir, "エクスポートフォルダ");
		strcpy(lang->conf_defaulttitle, "ゲームタイトル表\示をデフォルトにする");
		strcpy(lang->conf_defaultdetail, "詳細表\示のデフォルト");
		strcpy(lang->conf_defaultdetail_none, "なし");
		strcpy(lang->conf_defaultdetail_size, "サイズ");
		strcpy(lang->conf_defaultdetail_modifytime, "更新日時");
		strcpy(lang->conf_defaultdetail_both, "サイズと更新日時");
		strcpy(lang->conf_sort_type, "ファイルリストの並び順");
		strcpy(lang->conf_sort_types[0], "何もしない");
		strcpy(lang->conf_sort_types[1], "ファイル名");
		strcpy(lang->conf_sort_types[2], "拡張子");
		strcpy(lang->conf_sort_types[3], "ゲーム名");
		strcpy(lang->conf_sort_types[4], "サイズ");
		strcpy(lang->conf_sort_types[5], "更新日時");
		strcpy(lang->conf_sort_dir, "常にフォルダを上にする");
		strcpy(lang->conf_sort_ext, "ELFファイルを上にする");
		strcpy(lang->conf_usbmass_use, "外部USB_MASSドライバを使用する");
		strcpy(lang->conf_usbmass_path, "USB_MASS.IRX");
		strcpy(lang->conf_usbd_use, "外部USBドライバを使用する");
		strcpy(lang->conf_usbd_path, "USBD.IRX");
		strcpy(lang->conf_usbmass_devs, "USB_MASSデバイス数拡張");
		strcpy(lang->conf_usbkbd_use, "USBキーボードを使用する");
		strcpy(lang->conf_usbkbd_path, "PS2KBD.IRX");
		strcpy(lang->conf_usbmouse_use, "USBマウスを使用する");
		strcpy(lang->conf_usbmouse_path, "PS2MOUSE.IRX");
		strcpy(lang->conf_miscsettinginit, "「その他設定」を初期化する");
		strcpy(lang->conf_filersettinginit, "「ファイラー設定」を初期化する");
		strcpy(lang->conf_devicesettinginit, "「デバイス設定」を初期化する");
		//gsconfig
		{
		strcpy(lang->gs_easymode,		"簡単設定");
		strcpy(lang->gs_detailmode,		"詳細設定");
		strcpy(lang->gs_autoapply,		"変更と同時に適用");
		strcpy(lang->gs_gsinit,			"GSCONFIGを初期化する");
		strcpy(lang->gs_ok,				"保存して戻る");
		strcpy(lang->gs_cancel,			"キャンセル");
		strcpy(lang->gs_number,			"設定番号");
		strcpy(lang->gs_name,			"設定名");
		strcpy(lang->gs_width,			"水平解像度");
		strcpy(lang->gs_height,			"垂直解像度");
		strcpy(lang->gs_left,			"水平中央位置");
		strcpy(lang->gs_top,			"垂直中央位置");
		strcpy(lang->gs_mag_x,			"水平拡大値");
		strcpy(lang->gs_mag_y,			"垂直拡大値");
		strcpy(lang->gs_depth,			"色彩深度");
		strcpy(lang->gs_bufferwidth,	"1ラインのピクセル数");
		strcpy(lang->gs_x1,				"水平開始位置");
		strcpy(lang->gs_y1,				"垂直開始位置");
		strcpy(lang->gs_x2,				"水平終了位置");
		strcpy(lang->gs_y2,				"垂直終了位置");
		strcpy(lang->gs_zleft,			"水平Z位置");
		strcpy(lang->gs_ztop,			"垂直Z位置");
		strcpy(lang->gs_zdepth,			"Z深度");
		strcpy(lang->gs_dither,			"ディザリング");
		strcpy(lang->gs_interlace,		"インタレース");
		strcpy(lang->gs_ffmode,			"FFMODE");
		strcpy(lang->gs_vmode,			"VMODE");
		strcpy(lang->gs_vesa,			"VESA");
		strcpy(lang->gs_double,			"ダブルバッファ");
		strcpy(lang->gs_f0_left,		"フレーム0の水平位置");
		strcpy(lang->gs_f0_top,			"フレーム0の垂直位置");
		strcpy(lang->gs_f1_left,		"フレーム1の水平位置");
		strcpy(lang->gs_f1_top,			"フレーム1の垂直位置");
		strcpy(lang->gs_preset,			"プリセット値に戻す");
		strcpy(lang->gs_init,			"標準値の読み込み");
		strcpy(lang->gse_convert,		"%s へ上書き保存する");
		strcpy(lang->gs_vramsize,		"必要なVRAMのサイズ");
		strcpy(lang->gs_msg_0,			"設定によってはシステムが停止する可能\性があります。\n続行してもよろしいですか?");

		strcpy(lang->gs_prev,			"前へ");
		strcpy(lang->gs_next,			"次へ");
		strcpy(lang->gs_copy,			"コピー");
		strcpy(lang->gs_paste,			"ペースト");
		strcpy(lang->gs_apply,			"適用");
		strcpy(lang->gs_default,		"標準");
		}
		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "編集");
		strcpy(lang->conf_clear, "削除");
		strcpy(lang->conf_add, "増やす");
		strcpy(lang->conf_away, "減らす");
		strcpy(lang->conf_change, "変更");
		strcpy(lang->conf_up, "上へ");
		strcpy(lang->conf_detail, "詳細");
		strcpy(lang->conf_fast, "高速");
		strcpy(lang->conf_default, "標準");
		}
	}
}
