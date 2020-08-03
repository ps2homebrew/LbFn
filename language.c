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
		strcpy(lang->filer_exportfailed, "Export psu Failed");
		strcpy(lang->filer_exportto, "Export to");
		strcpy(lang->filer_importfailed, "Import psu Failed");
		strcpy(lang->filer_importto, "Import to");
		strcpy(lang->filer_keyboard_hint, "○:OK ×:Back L1:Left R1:Right START:Enter");
		strcpy(lang->filer_anyfile_hint1, "○:OK △:Up ×:Mark □:RevMark L1:TitleOFF R1:Menu R2:GetSize");
		strcpy(lang->filer_anyfile_hint2, "○:OK △:Up ×:Mark □:RevMark L1:TitleON  R1:Menu R2:GetSize");
		strcpy(lang->filer_elffile_hint1, "○:OK ×:Cancel △:Up □:*->ELF");
		strcpy(lang->filer_elffile_hint2, "○:OK ×:Cancel △:Up □:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "○:OK ×:Cancel △:Up □:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "○:OK ×:Cancel △:Up □:FNT->*");
		strcpy(lang->filer_dir_hint, "○:OK ×:Cancel △:Up Start:Choose");
		strcpy(lang->filer_export_files, "files");
		strcpy(lang->filer_export_header, " num: attr:     size: filename");
		strcpy(lang->filer_import_files, "files");
		strcpy(lang->filer_import_header, " num: attr:     size: filename");
		strcpy(lang->filer_l2popup_detail, "Detail Mode");
		strcpy(lang->filer_l2popup_dirsize, "Get DirSize");
		strcpy(lang->filer_l2popup_icon, "Icon");
		strcpy(lang->filer_l2popup_flicker, "Flicker Control");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "Save Failed");
		strcpy(lang->conf_saveconfig, "Save Config");
		strcpy(lang->conf_loadconfig, "Load Config");
		strcpy(lang->conf_initializeconfig, "Initialize Config");

		strcpy(lang->conf_setting_button,  "BUTTON SETTING");
		strcpy(lang->conf_setting_screen,  "SCREEN SETTING");
		strcpy(lang->conf_setting_network, "NETWORK SETTING");
		strcpy(lang->conf_setting_font,    "FONT SETTING");
		strcpy(lang->conf_setting_misc,    "MISC SETTING");
		strcpy(lang->conf_ok, "OK");
		strcpy(lang->conf_cancel, "CANCEL");

		//button
		strcpy(lang->conf_buttonsettinginit, "BUTTON SETTING INIT");
		//screen
		strcpy(lang->conf_background, "BACK GROUND   ");
		strcpy(lang->conf_frame, "FRAME         ");
		strcpy(lang->conf_highlighttext, "HIGHLIGHT TEXT");
		strcpy(lang->conf_normaltext, "NORMAL TEXT   ");
		strcpy(lang->conf_folder, "FOLDER        ");
		strcpy(lang->conf_file, "FILE          ");
		strcpy(lang->conf_ps2save, "PS2 SAVE      ");
		strcpy(lang->conf_elffile, "ELF FILE      ");
		strcpy(lang->conf_ps1save, "PS1 SAVE      ");
		strcpy(lang->conf_disabletext, "DISABLE TEXT  ");
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_interlace, "INTERLACE");
		strcpy(lang->conf_ffmode, "FFMODE");
		strcpy(lang->conf_ffmode_field, "FIELD");
		strcpy(lang->conf_ffmode_frame, "FRAME");
		strcpy(lang->conf_screen_x, "SCREEN X");
		strcpy(lang->conf_screen_y, "SCREEN Y");
		strcpy(lang->conf_flickercontrol, "FLICKER CONTROL");
		strcpy(lang->conf_screensettinginit, "SCREEN SETTING INIT");
		//network
		strcpy(lang->conf_ipaddress, "IP ADDRESS");
		strcpy(lang->conf_netmask, "NETMASK   ");
		strcpy(lang->conf_gateway, "GATEWAY   ");
		strcpy(lang->conf_ipoverwrite, "SAVE IPCONFIG.DAT");
		strcpy(lang->conf_ipsettinginit, "NETWORK SETTING INIT");
		strcpy(lang->conf_ipsaved, "Saved");
		strcpy(lang->conf_ipsavefailed, "Save Failed ");
		//font
		strcpy(lang->conf_AsciiFont, "ASCII FONT");
		strcpy(lang->conf_KanjiFont, "KANJI FONT");
		strcpy(lang->conf_CharMargin, "CHAR MARGIN");
		strcpy(lang->conf_LineMargin, "LINE MARGIN");
		strcpy(lang->conf_FontBold, "FONT BOLD");
		strcpy(lang->conf_AsciiMarginTop,  "ASCII MARGIN TOP ");
		strcpy(lang->conf_AsciiMarginLeft, "ASCII MARGIN LEFT");
		strcpy(lang->conf_KanjiMarginTop,  "KANJI MARGIN TOP ");
		strcpy(lang->conf_KanjiMarginLeft, "KANJI MARGIN LEFT");
		strcpy(lang->conf_fontsettinginit, "FONT SETTING INIT");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "日本語");
		strcpy(lang->conf_timeout, "TIME OUT");
		strcpy(lang->conf_disc_control, "DISC CONTROL");
		strcpy(lang->conf_print_only_filename, "PRINT ONLY FILENAME");
		strcpy(lang->conf_fileicon, "FILEICON");
		strcpy(lang->conf_disc_ps2save_check, "DISC PS2SAVE CHECK");
		strcpy(lang->conf_disc_elf_check, "DISC ELF CHECK");
		strcpy(lang->conf_export_dir, "EXPORT DIR");
		strcpy(lang->conf_miscsettinginit, "MISC SETTING INIT");

		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "Edit");
		strcpy(lang->conf_clear, "Clear");
		strcpy(lang->conf_add, "Add");
		strcpy(lang->conf_away, "Away");
		strcpy(lang->conf_change, "Change");
		strcpy(lang->conf_up, "Up");
		}
	}
	if(l==LANG_JAPANESE){
		//general
		strcpy(lang->gen_ok, "決定");
		strcpy(lang->gen_cancel, "キャンセル");
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
		strcpy(lang->filer_exportfailed, "エクスポート失敗しました");
		strcpy(lang->filer_exportto, "エクスポートした場所");
		strcpy(lang->filer_importfailed, "インポート失敗しました");
		strcpy(lang->filer_importto, "インポートした場所");
		strcpy(lang->filer_keyboard_hint, "○:決定 ×:削除 L1:左へ R1:右へ");
		strcpy(lang->filer_anyfile_hint1, "○:決定 △:上へ ×:マーク □:マーク反転 L1:タイトル R1:メニュー");
		strcpy(lang->filer_anyfile_hint2, "○:決定 △:上へ ×:マーク □:マーク反転 L1:タイトル R1:メニュー");
		strcpy(lang->filer_elffile_hint1, "○:決定 ×:キャンセル △:上へ □:*->ELF");
		strcpy(lang->filer_elffile_hint2, "○:決定 ×:キャンセル △:上へ □:ELF->*");
		strcpy(lang->filer_fntfile_hint1, "○:決定 ×:キャンセル △:上へ □:*->FNT");
		strcpy(lang->filer_fntfile_hint2, "○:決定 ×:キャンセル △:上へ □:FNT->*");
		strcpy(lang->filer_dir_hint, "○:決定 ×:キャンセル △:上へ Start:フォルダ選択");
		strcpy(lang->filer_export_files, "個のファイル");
		strcpy(lang->filer_export_header, "番号: 属性:   サイズ: ファイルの名前");
		strcpy(lang->filer_import_files, "個のファイル");
		strcpy(lang->filer_import_header, "番号: 属性:   サイズ: ファイルの名前");
		strcpy(lang->filer_l2popup_detail, "詳細表\示 切り替え");
		strcpy(lang->filer_l2popup_dirsize, "フォルダサイズ表\示");
		strcpy(lang->filer_l2popup_icon, "アイコン表\示");
		strcpy(lang->filer_l2popup_flicker, "フリッカーコントロール");
		}
		//config
		{
		strcpy(lang->conf_savefailed, "設定の保存に失敗しました");
		strcpy(lang->conf_saveconfig, "設定保存");
		strcpy(lang->conf_loadconfig, "設定読み込み");
		strcpy(lang->conf_initializeconfig, "設定初期化");

		strcpy(lang->conf_setting_button,  "ランチャー");
		strcpy(lang->conf_setting_screen,  "画面設定");
		strcpy(lang->conf_setting_network, "IP設定");
		strcpy(lang->conf_setting_font,    "フォント設定");
		strcpy(lang->conf_setting_misc,    "その他");
		strcpy(lang->conf_ok, "保存して戻る");
		strcpy(lang->conf_cancel, "キャンセル");

		//button
		strcpy(lang->conf_buttonsettinginit, "「ランチャー設定」を初期化する");
		//screen
		strcpy(lang->conf_background, "背景の色      ");
		strcpy(lang->conf_frame, "枠の色        ");
		strcpy(lang->conf_highlighttext, "強調文字      ");
		strcpy(lang->conf_normaltext, "文字          ");
		strcpy(lang->conf_folder, "フォルダ      ");
		strcpy(lang->conf_file, "ファイル      ");
		strcpy(lang->conf_ps2save, "PS2 セーブ    ");
		strcpy(lang->conf_elffile, "ELF ファイル  ");
		strcpy(lang->conf_ps1save, "PS1 セーブ    ");
		strcpy(lang->conf_disabletext, "無効文字      ");
		strcpy(lang->conf_screen_x, "画面位置 X");
		strcpy(lang->conf_screen_y, "画面位置 Y");
		strcpy(lang->conf_tvmode, "TV MODE");
		strcpy(lang->conf_interlace, "インターレース");
		strcpy(lang->conf_ffmode, "FFMODE");
		strcpy(lang->conf_ffmode_field, "FIELD");
		strcpy(lang->conf_ffmode_frame, "FRAME");
		strcpy(lang->conf_flickercontrol, "フリッカー調整");
		strcpy(lang->conf_screensettinginit, "「画面設定」を初期化する");
		//network
		strcpy(lang->conf_ipaddress, "IPアドレス  ");
		strcpy(lang->conf_netmask, "ネットマスク");
		strcpy(lang->conf_gateway, "ゲートウェイ");
		strcpy(lang->conf_ipoverwrite, "IPCONFIG.DATを上書き保存する");
		strcpy(lang->conf_ipsettinginit, "「IP設定」を初期化する");
		strcpy(lang->conf_ipsaved, "保存しました");
		strcpy(lang->conf_ipsavefailed, "保存失敗しました ");
		//font
		strcpy(lang->conf_AsciiFont, "アスキーフォント");
		strcpy(lang->conf_KanjiFont, "漢字フォント    ");
		strcpy(lang->conf_CharMargin, "文字の間隔");
		strcpy(lang->conf_LineMargin, "行の間隔  ");
		strcpy(lang->conf_FontBold, "太字にする");
		strcpy(lang->conf_AsciiMarginTop, "アスキーフォントの上の間隔");
		strcpy(lang->conf_AsciiMarginLeft, "アスキーフォントの左の間隔");
		strcpy(lang->conf_KanjiMarginTop, "漢字フォントの上の間隔");
		strcpy(lang->conf_KanjiMarginLeft, "漢字フォントの左の間隔");
		strcpy(lang->conf_fontsettinginit, "「フォント設定」を初期化する");
		//misc
		strcpy(lang->conf_language, "LANGUAGE");
		strcpy(lang->conf_language_us, "ENGLISH");
		strcpy(lang->conf_language_jp, "日本語");
		strcpy(lang->conf_timeout, "オートロードまでの秒数");
		strcpy(lang->conf_disc_control, "ディスクを停止する");
		strcpy(lang->conf_print_only_filename, "ランチャーアイテムをファイル名のみにする");
		strcpy(lang->conf_fileicon, "FileBrowserのアイコン");
		strcpy(lang->conf_disc_ps2save_check, "CD/DVDのときセーブデータか調べる");
		strcpy(lang->conf_disc_elf_check, "CD/DVDのときELFか調べる");
		strcpy(lang->conf_export_dir, "エクスポートフォルダ");
		strcpy(lang->conf_miscsettinginit, "「その他設定」を初期化する");

		strcpy(lang->conf_on, "ON");
		strcpy(lang->conf_off, "OFF");
		strcpy(lang->conf_edit, "編集");
		strcpy(lang->conf_clear, "削除");
		strcpy(lang->conf_add, "増やす");
		strcpy(lang->conf_away, "減らす");
		strcpy(lang->conf_change, "変更");
		strcpy(lang->conf_up, "上へ");
		}
	}
}
