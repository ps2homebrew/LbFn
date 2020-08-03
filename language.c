#include "launchelf.h"

LANGUAGE *lang=NULL;

//-------------------------------------------------
void InitLanguage(void)
{
	if (lang != NULL) free(lang);
	lang = (LANGUAGE*)malloc(sizeof(LANGUAGE));
	SetLanguage(LANG_ENGLISH);
}

//-------------------------------------------------
void FreeLanguage(void)
{
	if (lang != NULL) free(lang);
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
		strcpy(lang->filer_not_jpg, "This file isn't image.");
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
		strcpy(lang->filer_jpgfile_hint,  "○:OK ×:Cancel △:Up □:%s->%s");
		strcpy(lang->filer_irxfile_hint1, "○:OK ×:Cancel △:Up □:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "○:OK ×:Cancel △:Up □:IRX->*");
		strcpy(lang->filer_dir_hint, "○:OK ×:Cancel △:Up START:Choose");
		strcpy(lang->filer_l2popup_detail, "Detail Mode");
		strcpy(lang->filer_l2popup_dirsize, "Get DirSize");
		strcpy(lang->filer_l2popup_icon, "Icon");
		strcpy(lang->filer_l2popup_flicker, "Flicker Control");
		strcpy(lang->filer_l2popup_sort, "Sort Mode");
		/*
		strcpy(lang->kbd_page[ 0], "ASCII");
		strcpy(lang->kbd_page[ 1], "for IP");
		strcpy(lang->kbd_page[ 2], "HalfMarks");
		strcpy(lang->kbd_page[ 3], "FullMarks");
		strcpy(lang->kbd_page[ 4], "Hiragana");
		strcpy(lang->kbd_page[ 5], "Katakana");
		strcpy(lang->kbd_page[ 6], "Kanji-on");
		strcpy(lang->kbd_page[ 7], "Kanji-kun");
		strcpy(lang->kbd_page[ 8], "History");
		strcpy(lang->kbd_page[ 9], "Raw");
		strcpy(lang->kbd_page[10], "Custom");
		strcpy(lang->kbd_page[11], "Extend");
		strcpy(lang->kbd_page[12], "Direct");
		*/
		strcpy(lang->kbd_page[0], "ASCII");
		strcpy(lang->kbd_page[1], "Hiragana");
		strcpy(lang->kbd_page[2], "Katakana");
		strcpy(lang->kbd_page[3], "Num,Marks");
		strcpy(lang->kbd_page[4], "Kanji-on");
		strcpy(lang->kbd_page[5], "Kanji-kun");
		strcpy(lang->kbd_page[6], "History");
		strcpy(lang->kbd_page[7], "Raw");
		strcpy(lang->kbd_page[8], "Custom");
		strcpy(lang->kbd_page[9], "Extended");

		strcpy(lang->kbd_enter,    "Enter");
		strcpy(lang->kbd_abort,    "Abort");
		strcpy(lang->kbd_helpl, "○:Change ×:BS L1:Left R1:Right R2:Chars");
		strcpy(lang->kbd_helpr, "○:Insert ×:BS △:Return L1:Left R1:Right L2:Type R2:Regist");
		strcpy(lang->kbd_helpc, "○:Insert ×:BS △:Return L1:Left R1:Right L2:Type R2:Delete");
		
		strcpy(lang->kbd_registok, "Registed");
		strcpy(lang->kbd_registfail, "Failed");
		strcpy(lang->kbd_deleteok, "Deleted");
		strcpy(lang->kbd_saving, "Saving...");
		strcpy(lang->kbd_loaded, "Loaded");
		strcpy(lang->kbd_update, "VIRTUAL KEYBOARD DATA WRITE");
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
		strcpy(lang->editor_l2popup_wordwrap, "Auto New Line Mode");
		strcpy(lang->editor_image_help, "○:Frame □:Zoom △/×:Exit  SIZE:%d×%d (%dx%d)");
		strcpy(lang->editor_image_help2, "○:Frame □:Zoom L1:Prev R1:Next △/×:Exit SIZE:%d×%d (%dx%d)");
		strcpy(lang->editor_image_help3, "○:Frame □:Zoom ←:Back →:Next △/×:Exit  SIZE:%d×%d (%dx%d)");
		strcpy(lang->editor_image_help4, "○:Frame □:Zoom ←:Back →:Next L1:Prev R1:Next △/×:Exit SIZE:%d×%d (%dx%d)");
		//								  1234567890123456789012345678901234567890123456789012345678901234
		//									       1         2         3         4         5         6    
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
		strcpy(lang->conf_button_copied, "copied");
		strcpy(lang->conf_button_deleted, "deleted");
		strcpy(lang->conf_button_pasted, "pasted");
		strcpy(lang->conf_launch_btnnum, "BUTTON SETTING OF #%s");
		strcpy(lang->conf_launch_name, "DISPLAY NAME");
		strcpy(lang->conf_launch_padmsk, "PAD");
		strcpy(lang->conf_launch_path, "PATH%d");
		strcpy(lang->conf_launch_list, "CHECK ALL PAD SETTING");
		strcpy(lang->conf_buttonsettinginit, "LAUNCHER SETTING INIT");
		strcpy(lang->conf_launch_pad0, "Push the button(s) please(Arrow key is cancel).");
		strcpy(lang->conf_launch_pad2, "DEFAULT is can't change the pad.");
		strcpy(lang->conf_insert, "Insert");
		strcpy(lang->conf_delete, "Delete");
		//color
		strcpy(lang->conf_background, "BACK GROUND   ");
		strcpy(lang->conf_frame, "FRAME         ");
		strcpy(lang->conf_normaltext, "NORMAL TEXT   ");
		strcpy(lang->conf_highlighttext, "HIGHLIGHT TEXT");
		strcpy(lang->conf_disabletext, "GRAY TEXT     ");
		strcpy(lang->conf_shadowtext, "SHADOW TEXT   ");
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
		strcpy(lang->conf_AsciiFont, "SBCS FONT");
		strcpy(lang->conf_KanjiFont, "DBCS FONT");
		strcpy(lang->conf_DisableCtrl, "DISABLE CONTROL CHARS");	// disable the part of control characters
		strcpy(lang->conf_UseFontCache, "ENABLE FONT CACHE");
		strcpy(lang->conf_CharMargin, "CHAR MARGIN");
		strcpy(lang->conf_LineMargin, "LINE MARGIN");
		strcpy(lang->conf_FontBold, "FONT BOLD");
		strcpy(lang->conf_FontHalf, "FIX FONT WIDTH ");
		strcpy(lang->conf_FontVHalf, "FIX FONT HEIGHT");
		strcpy(lang->conf_FontScaler, "FIX FONT MODE");
		strcpy(lang->conf_FontScaler_A, "Faster (nearest/composite)");
		strcpy(lang->conf_FontScaler_B, "Normal (bilinear)");
		strcpy(lang->conf_FontScaler_C, "Reserved");
		strcpy(lang->conf_AsciiMarginTop,  "SBCS MARGIN TOP ");
		strcpy(lang->conf_AsciiMarginLeft, "SBCS MARGIN LEFT");
		strcpy(lang->conf_KanjiMarginTop,  "DBCS MARGIN TOP ");
		strcpy(lang->conf_KanjiMarginLeft, "DBCS MARGIN LEFT");
		strcpy(lang->conf_fontsettinginit, "FONT SETTING INIT");
		//viewer
		strcpy(lang->conf_linenumber, "DISPLAY LINE NUMBER IN TEXT");
		strcpy(lang->conf_tabspaces,  "WIDTH OF TAB IN TEXT");
		strcpy(lang->conf_chardisp,   "DISPLAY RETURN AND TAB CODE IN TEXT");
		strcpy(lang->conf_wordwrap,   "AUTO NEW LINE IN TEXT");
		strcpy(lang->conf_fullscreen, "FULLSCREEN MODE IN IMAGE");
		strcpy(lang->conf_imageresize,"DO NOT RESIZE IN IMAGE");
		strcpy(lang->conf_autodecode, "AUTO DECODE OF TEK COMPRESSION");
		strcpy(lang->conf_sdtv_aspect,"SCREEN ASPECT RATIO OF SDTV IN IMAGE");
		strcpy(lang->conf_pixelaspect,"APPLY THE PIXEL ASPECT RATIO IN IMAGE");
		strcpy(lang->conf_aniauto,    "AUTOMATIC ANIMATION OF IMAGE");
		strcpy(lang->conf_position,   "DEFAULT DISPLAY AREA OF IMAGE");
		strcpy(lang->conf_bgplay,     "ENABLE BACKGROUND PLAYING OF AUDIO");
		strcpy(lang->conf_volume,     "DEFAULT VOLUME IN AUDIO");
		strcpy(lang->conf_repeat,     "DEFAULT REPEAT MODE IN AUDIO");
		strcpy(lang->conf_viewerinit, "VIEWER SETTING INIT");
		strcpy(lang->conf_sdtv_square," 4:3");
		strcpy(lang->conf_sdtv_wide,  "16:9");
		strcpy(lang->conf_sound[0],   "OFF");
		strcpy(lang->conf_sound[1],   "REPEAT");
		strcpy(lang->conf_sound[2],   "REPEAT ALL");
		strcpy(lang->conf_sound[3],   "RANDOM");
		strcpy(lang->conf_sound[4],   "SHUFFLE");
		strcpy(lang->conf_imgpos[0],  "LEFT-TOP");
		strcpy(lang->conf_imgpos[1],  "TOP");
		strcpy(lang->conf_imgpos[2],  "RIGHT-TOP");
		strcpy(lang->conf_imgpos[3],  "LEFT");
		strcpy(lang->conf_imgpos[4],  "CENTER");
		strcpy(lang->conf_imgpos[5],  "RIGHT");
		strcpy(lang->conf_imgpos[6],  "LEFT-BOTTOM");
		strcpy(lang->conf_imgpos[7],  "BOTTOM");
		strcpy(lang->conf_imgpos[8],  "RIGHT-BOTTOM");
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
		strcpy(lang->conf_getsizecrc32, "DISPLAY CRC32 WITH GET SIZE");
		strcpy(lang->conf_exportname, "ADD EXPORT FILENAME");
		strcpy(lang->conf_exportnames[0], "OFF");
		strcpy(lang->conf_exportnames[1], "TIMESTAMP");
		strcpy(lang->conf_exportnames[2], "CRC32");
		strcpy(lang->conf_exportnames[3], "TIME AND CRC32");
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
		strcpy(lang->conf_downloadpath, "DEFAULT DOWNLOAD DIR");
		strcpy(lang->conf_screenshotflag, "USE SCREENSHOT");
		strcpy(lang->conf_screenshotpad,  "SCREENSHOT TRIGGER");
		strcpy(lang->conf_screenshotpath, "SCREENSHOT DIR");
		strcpy(lang->conf_wallpaperuse, "USE WALLPAPER");
		strcpy(lang->conf_wallpaperpath, "BACKGROUND IMAGE FILE");
		strcpy(lang->conf_wallpapermode, "BACKGROUND MODE");
		strcpy(lang->conf_wallpapercontrast, "CONTRAST");
		strcpy(lang->conf_wallpaperbrightness, "BRIGHTNESS OF SCREEN");
		strcpy(lang->conf_wallpaperwindow, "BRIGHTNESS OF WINDOW");
		strcpy(lang->conf_preview, "PREVIEW");
		strcpy(lang->conf_miscsettinginit, "MISC SETTING INIT");
		strcpy(lang->conf_filersettinginit, "FILER SETTING INIT");
		strcpy(lang->conf_devicesettinginit, "DEVICE SETTING INIT");
		strcpy(lang->conf_wp_mode[0], "CENTERING");
		strcpy(lang->conf_wp_mode[1], "TILING");
		strcpy(lang->conf_wp_mode[2], "TOUCH SCREEN FROM INSIDE");
		strcpy(lang->conf_wp_mode[3], "TOUCH SCREEN FROM OUTSIDE");
		strcpy(lang->conf_wp_mode[4], "STRETCH TO SCREEN");
		strcpy(lang->conf_wp_on[0], "ON (ONLY SCREEN)");
		strcpy(lang->conf_wp_on[1], "ON (SCREEN AND WINDOW)");
		strcpy(lang->conf_wallpaperreload, "Reloading wallpaper...");
		strcpy(lang->conf_wallpaperload,   "Loading wallpaper...");
		strcpy(lang->conf_wallpaperdecode, "Processing wallpaper...");
		strcpy(lang->conf_wallpaperresize, "Drawing wallpaper...");
		strcpy(lang->conf_wallpapererror,  "Wallpaper was failed");
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
		strcpy(lang->gs_widthf,			"FULL SCREEN WIDTH ");
		strcpy(lang->gs_heightf,		"FULL SCREEN HEIGHT");
		strcpy(lang->gs_left,			"HORIZONTAL CENTER OFFSET");
		strcpy(lang->gs_top,			"VERTICAL CENTER OFFSET  ");
		strcpy(lang->gs_mag_x,			"X RESIZE (mag_x) ");
		strcpy(lang->gs_mag_y,			"Y RESIZE (mag_y) ");
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
		strcpy(lang->gs_preset,			"LOAD VMODE DEFAULT");
		strcpy(lang->gs_init,			"LOAD DEFAULT SETTING");
		//strcpy(lang->gs_apply,			"APPLY THE SETTING");
		strcpy(lang->gs_target,			"TARGET: ");
		strcpy(lang->gs_read,			"LOAD SETTING FROM TARGET");
		strcpy(lang->gs_write,			"SAVE SETTING TO TARGET");
		strcpy(lang->gs_vramsize,		"VRAM SIZE");

		strcpy(lang->gse_editsize,		"EDIT TARGET");
		strcpy(lang->gse_magnify,		"MAGNIFICATION TARGET");
		strcpy(lang->gse_convert,		"OVERRIDE WRITE TO %s");
		strcpy(lang->gs_msg_0,			"In case of setting to system crash. continue the setting?");
		
		strcpy(lang->gs_prev,			"Prev");
		strcpy(lang->gs_next,			"Next");
		strcpy(lang->gs_copy,			"Copy");
		strcpy(lang->gs_paste,			"Paste");
		strcpy(lang->gs_apply,			"Apply");
		strcpy(lang->gs_default,		"Default");
		}
		//	FMCB_CONFIG
		{
		strcpy(lang->fmcb[ 0], "NULL");
		strcpy(lang->fmcb[ 1], "Load CNF from: %s");
		strcpy(lang->fmcb[ 2], "Save CNF to: %s");
		strcpy(lang->fmcb[ 3], "Configutre launcher buttons...");
		strcpy(lang->fmcb[ 4], "Configutre OSDSYS options...");
		strcpy(lang->fmcb[ 5], "Configutre ESR path...");
		strcpy(lang->fmcb[ 6], "FastBoot: %s");
		strcpy(lang->fmcb[ 7], "Debug Screen: %s");
		strcpy(lang->fmcb[ 8], "Pad Delay: %d.%d");
		strcpy(lang->fmcb[ 9], "Return");
		strcpy(lang->fmcb[10], "Pad: %s");
		strcpy(lang->fmcb[11], "Check the all launcher buttons");
		strcpy(lang->fmcb[12], "Hacked OSDSYS: %s");
		strcpy(lang->fmcb[13], "Configutre Items...");
		strcpy(lang->fmcb[14], "Configutre Scrolling Options...");
		strcpy(lang->fmcb[15], "Video Mode: %s");
		strcpy(lang->fmcb[16], "Skip MC update check: %s");
		strcpy(lang->fmcb[17], "Skip HDD update check: %s");
		strcpy(lang->fmcb[18], "Skip Disc Boot: %s");
		strcpy(lang->fmcb[19], "Skip Sony Logo: %s");
		strcpy(lang->fmcb[20], "Go to Browser: %s");
		strcpy(lang->fmcb[21], "Selected Color:   ");
		strcpy(lang->fmcb[22], "Unselected Color: ");
		strcpy(lang->fmcb[23], "Menu X: %3d");
		strcpy(lang->fmcb[24], "Menu y: %3d");
		strcpy(lang->fmcb[25], "Enter:            ");
		strcpy(lang->fmcb[26], "Version:          ");
		strcpy(lang->fmcb[40], "Number of Item: %3d");
		strcpy(lang->fmcb[29], "Name: %s");
		strcpy(lang->fmcb[30], "Path%d: %s");
		strcpy(lang->fmcb[31], "Scroll Menu: %s");
		strcpy(lang->fmcb[32], "Displayed Items: %3d");
		strcpy(lang->fmcb[33], "Cursor Max Velocity:%6d");
		strcpy(lang->fmcb[34], "Cursor Acceleration:%6d");
		strcpy(lang->fmcb[35], "Left Cursor: %s");
		strcpy(lang->fmcb[36], "Right Cursor: %s");
		strcpy(lang->fmcb[37], "Top Deimiter: %s");
		strcpy(lang->fmcb[38], "Bottom Delimiter: %s");
		strcpy(lang->fmcb[39], "..");
		strcpy(lang->fmcb[41], "Loaded %s");
		strcpy(lang->fmcb[42], "Failed to load %s");
		strcpy(lang->fmcb[43], "Saved %s");
		strcpy(lang->fmcb[44], "Failed to open %s");
		strcpy(lang->fmcb[51], "Free McBoot Settings");
		strcpy(lang->fmcb[52], "Button Settings");
		strcpy(lang->fmcb[53], "OSD Settings");
		strcpy(lang->fmcb[54], "Item Settings");
		strcpy(lang->fmcb[55], "Scroll Settings");
		strcpy(lang->fmcb[56], "ESR Path");
		strcpy(lang->fmcb[27], "○:OK L1/R1:Change L2:Init R2:Default");// CNF
		strcpy(lang->fmcb[28], "○:OK L1/R1:Change");
		strcpy(lang->fmcb[59], "○:Add ×:Sub +□:Fast");//public
		strcpy(lang->fmcb[60], "○:OK");
		strcpy(lang->fmcb[61], "○:Edit ×:Clear");
		strcpy(lang->fmcb[62], "○:Change");
		strcpy(lang->fmcb[63], "○:Edit");
		strcpy(lang->fmcb[57], "○:Edit ×:Clear L1:Any R1:Special L2:Copy R2:Paste"); //dir
		strcpy(lang->fmcb[58], "○:Edit ×:Clear L2:Copy R2:Paste");	//title
		strcpy(lang->fmcb[45], "○:Add ×:Sub +□:Fast L2:Copy R2:Paste");
		strcpy(lang->fmcb[46], "○:Next ×:Prev L2:Copy R2:Paste");
		strcpy(lang->fmcb[47], "○:Add ×:Sub");
		strcpy(lang->fmcb[48], "○:Next ×:Prev");
		strcpy(lang->fmcb[49], "Setting was initialized");
		strcpy(lang->fmcb[50], "All set to default");
		}
		// ネットワークダウンロード
		{
		strcpy(lang->nupd[ 0], "NetworkDownload");
		strcpy(lang->nupd[ 1], "Download file: %s");
		strcpy(lang->nupd[ 2], "Save folder of downloaded file: %s");
		strcpy(lang->nupd[ 3], "Save with change the filename: %s");
		strcpy(lang->nupd[ 4], "Make the backup: %s");
		strcpy(lang->nupd[ 5], "Execute");
		strcpy(lang->nupd[ 6], "Return");
		strcpy(lang->nupd[ 7], "Begin the download.\nAre you ready?");
		strcpy(lang->nupd[ 8], "Now downloading...");
		strcpy(lang->nupd[ 9], "Now saving...");
		strcpy(lang->nupd[10], "Download was failed.");
		strcpy(lang->nupd[11], "Loading DNS driver");
		strcpy(lang->nupd[12], "Loading HTTP driver");
		strcpy(lang->nupd[13], "Now downloading the list.");
		strcpy(lang->nupd[14], "Failed download the list.");
		strcpy(lang->nupd[15], "Verify was failed.");
		strcpy(lang->nupd[16], "Now making the backup");
		strcpy(lang->nupd[17], "Backup was failed.\nAre you continue?");
		strcpy(lang->nupd[18], "Out of free space.");
		strcpy(lang->nupd[19], "○:Look L1:Prev R1:Next R2:Info △:Up");
		strcpy(lang->nupd[20], "○:Change ×/□:Default △:Up");
		strcpy(lang->nupd[21], "○:OK △:Up");
		strcpy(lang->nupd[22], "Loading TCP/IP (PS2IP) driver");
		strcpy(lang->nupd[23], "Loading LAN (PS2SMAP) device driver");
		strcpy(lang->nupd[24], "Download was completed.");
		}
		// サウンドプレイヤー
		{
		strcpy(lang->sound[ 0], "path");
		strcpy(lang->sound[ 1], "type");
		strcpy(lang->sound[ 2], "title");
		strcpy(lang->sound[ 3], "artist");
		strcpy(lang->sound[ 4], "comment");
		strcpy(lang->sound[ 5], "date");
		strcpy(lang->sound[ 6], "copyright");
		strcpy(lang->sound[ 7], "bitrate");
		strcpy(lang->sound[ 8], "sampling rate");
		strcpy(lang->sound[ 9], "bits");
		strcpy(lang->sound[10], "channels");
		strcpy(lang->sound[11], "time");
		strcpy(lang->sound[12], "volume");
		strcpy(lang->sound[13], "speed");
		strcpy(lang->sound[14], "position");
		strcpy(lang->sound[15], "back");
		strcpy(lang->sound[16], "skip");
		strcpy(lang->sound[17], "slow");
		strcpy(lang->sound[18], "fast");
		strcpy(lang->sound[19], "top");
		strcpy(lang->sound[20], "normal");
		strcpy(lang->sound[21], "play");
		strcpy(lang->sound[22], "stop");
		strcpy(lang->sound[23], "quit");
		strcpy(lang->sound[24], "speed");
		strcpy(lang->sound[25], "seek");
		strcpy(lang->sound[26], "info");
		strcpy(lang->sound[27], "pause / resume");
		strcpy(lang->sound[28], "○:skip ×:back △:quit L1:top L2/R2:seek");
		strcpy(lang->sound[29], "○:enter △:quit L2/R2:seek");
		strcpy(lang->sound[30], "○:add ×:sub +□:fast △:quit L1:default L2/R2:seek");
		strcpy(lang->sound[31], "△:quit L2/R2:seek");
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
		strcpy(lang->filer_not_jpg, "イメージファイルではありません");
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
		strcpy(lang->filer_jpgfile_hint,  "○:決定 ×:キャンセル △:上へ □:%s->%s");
		strcpy(lang->filer_irxfile_hint1, "○:決定 ×:キャンセル △:上へ □:*->IRX");
		strcpy(lang->filer_irxfile_hint2, "○:決定 ×:キャンセル △:上へ □:IRX->*");
		strcpy(lang->filer_dir_hint, "○:決定 ×:キャンセル △:上へ Start:フォルダ選択");
		strcpy(lang->filer_l2popup_detail, "詳細表\示 切り替え");
		strcpy(lang->filer_l2popup_dirsize, "フォルダサイズ表\示");
		strcpy(lang->filer_l2popup_icon, "アイコン表\示");
		strcpy(lang->filer_l2popup_flicker, "フリッカーコントロール");
		strcpy(lang->filer_l2popup_sort, "リスト並び");
		/*
		strcpy(lang->kbd_page[ 0], "ASCII");
		strcpy(lang->kbd_page[ 1], "IP入力用");
		strcpy(lang->kbd_page[ 2], "半角記号");
		strcpy(lang->kbd_page[ 3], "全角記号");
		strcpy(lang->kbd_page[ 4], "ひらがな");
		strcpy(lang->kbd_page[ 5], "カタカナ");
		strcpy(lang->kbd_page[ 6], "漢字(音)");
		strcpy(lang->kbd_page[ 7], "漢字(訓)");
		strcpy(lang->kbd_page[ 8], "漢字履歴");
		strcpy(lang->kbd_page[ 9], "文字一覧");
		strcpy(lang->kbd_page[10], "カスタム");
		strcpy(lang->kbd_page[11], "外部拡張");
		strcpy(lang->kbd_page[12], "直接入力");
		*/
		strcpy(lang->kbd_page[0], "ASCII");
		strcpy(lang->kbd_page[1], "ひらがな");
		strcpy(lang->kbd_page[2], "カタカナ");
		strcpy(lang->kbd_page[3], "英数記号");
		strcpy(lang->kbd_page[4], "漢字(音)");
		strcpy(lang->kbd_page[5], "漢字(訓)");
		strcpy(lang->kbd_page[6], "漢字履歴");
		strcpy(lang->kbd_page[7], "文字一覧");
		strcpy(lang->kbd_page[8], "カスタム");
		strcpy(lang->kbd_page[9], "外部拡張");
		
		strcpy(lang->kbd_enter,    "決定");
		strcpy(lang->kbd_abort,    "中止");
		strcpy(lang->kbd_helpl, "○:決定 ×:削除 L1:左へ R1:右へ R2:入力へ");
		strcpy(lang->kbd_helpr, "○:入力 ×:削除 △:戻る L1:左へ R1:右へ L2:種類へ R2:登録");
		strcpy(lang->kbd_helpc, "○:入力 ×:削除 △:戻る L1:左へ R1:右へ L2:種類へ R2:抹消");
		
		strcpy(lang->kbd_registok, "登録完了");
		strcpy(lang->kbd_registfail, "登録失敗");
		strcpy(lang->kbd_deleteok, "抹消完了");
		strcpy(lang->kbd_saving, "書き込み中..");
		strcpy(lang->kbd_loaded, "読み込み完了");
		strcpy(lang->kbd_update, "ソ\フトキーボードのデータ書き戻し");
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
		strcpy(lang->editor_l2popup_wordwrap, "右端で折り返し変更");
		strcpy(lang->editor_image_help, "○:フル □:ズーム △/×:戻る 画像:%d×%d 表\示:%d×%d");
		strcpy(lang->editor_image_help2, "○:フル □:ズーム L1/R1:前/次 △/×:戻る サイズ:%d×%d (%d×%d)");
		strcpy(lang->editor_image_help3, "○:フル □:ズーム ←/→:戻/進 △/×:戻る 画像:%d×%d 表\示:%d×%d");
		strcpy(lang->editor_image_help4, "○:フル □:ズーム ←/→:戻/進 L1/R1:前/次 △/×:戻る サイズ:%d×%d (%d×%d)");
		//								  1234567890123456789012345678901234567890123456789012345678901234
		//									       1         2         3         4         5         6    
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
		strcpy(lang->conf_button_copied, "コピーしました");
		strcpy(lang->conf_button_deleted, "クリアしました");
		strcpy(lang->conf_button_pasted, "ペーストしました");
		strcpy(lang->conf_launch_btnnum, "[登録番号:%s] のボタン設定");
		strcpy(lang->conf_launch_name, "設定名");
		strcpy(lang->conf_launch_padmsk, "ボタン");
		strcpy(lang->conf_launch_path, "登録%d");
		strcpy(lang->conf_launch_list, "全てのボタン設定を確認する");
		strcpy(lang->conf_buttonsettinginit, "「ランチャー設定」を初期化する");
		strcpy(lang->conf_launch_pad0, "対応させるボタンを押して下さい(方向キーでキャンセル)");
		strcpy(lang->conf_launch_pad2, "DEFAULT は変更できません");
		strcpy(lang->conf_insert, "挿入");
		strcpy(lang->conf_delete, "消去");
		//color
		strcpy(lang->conf_background,    "背景の色      ");
		strcpy(lang->conf_frame,         "フレームの色  ");
		strcpy(lang->conf_normaltext,    "テキスト      ");
		strcpy(lang->conf_highlighttext, "テキスト強調  ");
		strcpy(lang->conf_disabletext,   "テキスト無効  ");
		strcpy(lang->conf_shadowtext,    "テキスト影    ");
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
		strcpy(lang->conf_AsciiFont, "半角フォント");
		strcpy(lang->conf_KanjiFont, "全角フォント");
		strcpy(lang->conf_DisableCtrl, "特殊制御文字を無効にする");
		strcpy(lang->conf_UseFontCache, "フォントキャッシュを有効にする");
		strcpy(lang->conf_CharMargin, "文字の間隔");
		strcpy(lang->conf_LineMargin, "行の間隔  ");
		strcpy(lang->conf_FontBold, "太字にする");
		strcpy(lang->conf_FontHalf, "フォントの幅を補正する　");
		strcpy(lang->conf_FontVHalf, "フォントの高さを補正する");
		strcpy(lang->conf_FontScaler, "フォント補正モード");
		strcpy(lang->conf_FontScaler_A, "高速");
		strcpy(lang->conf_FontScaler_B, "標準");
		strcpy(lang->conf_FontScaler_C, "未定");
		strcpy(lang->conf_AsciiMarginTop, "半角フォントの上の間隔");
		strcpy(lang->conf_AsciiMarginLeft, "半角フォントの左の間隔");
		strcpy(lang->conf_KanjiMarginTop, "全角フォントの上の間隔");
		strcpy(lang->conf_KanjiMarginLeft, "全角フォントの左の間隔");
		strcpy(lang->conf_fontsettinginit, "「フォント設定」を初期化する");
		//viewer
		strcpy(lang->conf_linenumber, "テキストの行番号表\示");
		strcpy(lang->conf_tabspaces,  "テキストのTABの文字数(半角換算)");
		strcpy(lang->conf_chardisp,   "テキストの改行やTABのマーク表\示");
		strcpy(lang->conf_wordwrap,   "テキストの行末の折り返し処理");
		strcpy(lang->conf_fullscreen, "イメージのフルスクリーンモード");
		strcpy(lang->conf_imageresize,"イメージをリサイズしないで表\示");
		strcpy(lang->conf_autodecode, "tek圧縮の自動伸張");
		strcpy(lang->conf_sdtv_aspect,"標準画質出力のアスペクト比");
		strcpy(lang->conf_pixelaspect,"ピクセルアスペクト比の適用");
		strcpy(lang->conf_aniauto,    "イメージの自動アニメーション");
		strcpy(lang->conf_position,   "イメージの初期描画範囲");
		strcpy(lang->conf_bgplay,     "サウンドのBG再生");
		strcpy(lang->conf_volume,     "サウンドの標準音量");
		strcpy(lang->conf_repeat,     "サウンドのループ再生");
		strcpy(lang->conf_viewerinit, "「ビューア設定」を初期化する");
		strcpy(lang->conf_sdtv_square," 4:3");
		strcpy(lang->conf_sdtv_wide,  "16:9");
		strcpy(lang->conf_sound[0],   "off");
		strcpy(lang->conf_sound[1],   "AUTO REPEAT");	// Repeat
		strcpy(lang->conf_sound[2],   "All repeat");
		strcpy(lang->conf_sound[3],   "Random");
		strcpy(lang->conf_sound[4],   "Shuffle");
		strcpy(lang->conf_imgpos[0],  "左上");
		strcpy(lang->conf_imgpos[1],  "上");
		strcpy(lang->conf_imgpos[2],  "右上");
		strcpy(lang->conf_imgpos[3],  "左");
		strcpy(lang->conf_imgpos[4],  "中央");
		strcpy(lang->conf_imgpos[5],  "右");
		strcpy(lang->conf_imgpos[6],  "左下");
		strcpy(lang->conf_imgpos[7],  "下");
		strcpy(lang->conf_imgpos[8],  "右下");
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
		strcpy(lang->conf_getsizecrc32, "サイズ取得でCRC32も表\示する");
		strcpy(lang->conf_exportname, "エクスポートファイル名追加");
		strcpy(lang->conf_exportnames[0], "OFF");
		strcpy(lang->conf_exportnames[1], "更新日時");
		strcpy(lang->conf_exportnames[2], "CRC32");
		strcpy(lang->conf_exportnames[3], "更新日時とCRC32");
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
		strcpy(lang->conf_downloadpath, "標準ダウンロードフォルダ");
		strcpy(lang->conf_screenshotflag, "スクリーンショット機能\を利用する");
		strcpy(lang->conf_screenshotpad,  "スクリーンショットボタン");
		strcpy(lang->conf_screenshotpath, "スクリーンショットフォルダ");
		strcpy(lang->conf_wallpaperuse, "背景に壁紙を利用する");
		strcpy(lang->conf_wallpaperpath, "背景イメージ");
		strcpy(lang->conf_wallpapermode, "背景モード");
		strcpy(lang->conf_wallpapercontrast, "コントラスト");
		strcpy(lang->conf_wallpaperbrightness, "全体の明るさ");
		strcpy(lang->conf_wallpaperwindow, "ウィンドウの明るさ");
		strcpy(lang->conf_preview, "プレビュー");
		strcpy(lang->conf_wp_mode[0], "中央に表\示");
		strcpy(lang->conf_wp_mode[1], "並べて表\示");
		strcpy(lang->conf_wp_mode[2], "画面全体に表\示(内側)");
		strcpy(lang->conf_wp_mode[3], "画面全体に表\示(外側)");
		strcpy(lang->conf_wp_mode[4], "画面全体に表\示(全体)");
		strcpy(lang->conf_wp_on[0], "ON (背景のみ)");
		strcpy(lang->conf_wp_on[1], "ON (背景とウィンドウ)");
		strcpy(lang->conf_miscsettinginit, "「その他設定」を初期化する");
		strcpy(lang->conf_filersettinginit, "「ファイラー設定」を初期化する");
		strcpy(lang->conf_devicesettinginit, "「デバイス設定」を初期化する");
		strcpy(lang->conf_wallpaperreload, "壁紙を再設定しています...");
		strcpy(lang->conf_wallpaperload,   "壁紙を読み込み中です...");
		strcpy(lang->conf_wallpaperdecode, "壁紙を準備しています...");
		strcpy(lang->conf_wallpaperresize, "壁紙を描画しています...");
		strcpy(lang->conf_wallpapererror,  "壁紙の設定に失敗しました");
		
		

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
		strcpy(lang->gs_widthf,			"水平解像度(フル)");
		strcpy(lang->gs_heightf,		"垂直解像度(フル)");
		strcpy(lang->gs_left,			"水平中央位置");
		strcpy(lang->gs_top,			"垂直中央位置");
		strcpy(lang->gs_mag_x,			"水平拡大値(mag_x)");
		strcpy(lang->gs_mag_y,			"垂直拡大値(mag_y)");
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
		strcpy(lang->gs_preset,			"プリセット値の読み込み");
		strcpy(lang->gs_init,			"標準値の読み込み");
		//strcpy(lang->gs_apply,			"現在の設定を反映");
		strcpy(lang->gs_target,			"アクセス対象: ");
		strcpy(lang->gs_read,			"設定を読み込む");
		strcpy(lang->gs_write,			"設定を保存する");
		strcpy(lang->gs_vramsize,		"必要なVRAMのサイズ");

		strcpy(lang->gse_editsize,		"編集サイズ");
		strcpy(lang->gse_magnify,		"拡大基準値");
		strcpy(lang->gse_convert,		"%s へ上書き保存する");
		strcpy(lang->gs_msg_0,			"設定によってはシステムが停止する可能\性があります。\n続行してもよろしいですか?");

		strcpy(lang->gs_prev,			"前へ");
		strcpy(lang->gs_next,			"次へ");
		strcpy(lang->gs_copy,			"コピー");
		strcpy(lang->gs_paste,			"ペースト");
		strcpy(lang->gs_apply,			"適用");
		}
		strcpy(lang->gs_default,		"標準");
		//	FMCB_CONFIG
		{
		strcpy(lang->fmcb[ 0], "NULL");
		strcpy(lang->fmcb[ 1], "ロード: %s");
		strcpy(lang->fmcb[ 2], "セーブ: %s");
		strcpy(lang->fmcb[ 3], "ランチャー設定...");
		strcpy(lang->fmcb[ 4], "OSDSYSメニューの設定...");
		strcpy(lang->fmcb[ 5], "ESR設定...");
		strcpy(lang->fmcb[ 6], "高速起動(FASTBOOT): %s");
		strcpy(lang->fmcb[ 7], "デバッグスクリーン: %s");
		strcpy(lang->fmcb[ 8], "ボタン入力待ち時間: %d.%d");
		strcpy(lang->fmcb[ 9], "戻る");
		strcpy(lang->fmcb[10], "ボタン: %s");
		strcpy(lang->fmcb[11], "すべてのランチャー設定を確認する");
		strcpy(lang->fmcb[12], "OSDSYSをハックする: %s");
		strcpy(lang->fmcb[13], "メニューアイテムの設定...");
		strcpy(lang->fmcb[14], "スクロール設定...");
		strcpy(lang->fmcb[15], "映像出力: %s");
		strcpy(lang->fmcb[16], "メモリーカードを検索しない: %s");
		strcpy(lang->fmcb[17], "ハードディスクを検索しない: %s");
		strcpy(lang->fmcb[18], "CD/DVDの起動をスキップ: %s");
		strcpy(lang->fmcb[19], "起動時のSONYロゴ非表\示: %s");
		strcpy(lang->fmcb[20], "起動したらブラウザ画面: %s");
		strcpy(lang->fmcb[21], "カーソ\ルの色:   ");
		strcpy(lang->fmcb[22], "アイテムの色:   ");
		strcpy(lang->fmcb[23], "メニューの水平位置: %3d");
		strcpy(lang->fmcb[24], "メニューの垂直位置: %3d");
		strcpy(lang->fmcb[25], "決定:           ");
		strcpy(lang->fmcb[26], "本体設定:       ");
		strcpy(lang->fmcb[40], "メニューアイテム番号: %3d");
		strcpy(lang->fmcb[29], "アイテム名: %s");
		strcpy(lang->fmcb[30], "登録%d: %s");
		strcpy(lang->fmcb[31], "メニューをスクロール式にする: %s");
		strcpy(lang->fmcb[32], "表\示するメニューアイテム数: %3d");
		strcpy(lang->fmcb[33], "カーソ\ル最大移動速度:%7d");
		strcpy(lang->fmcb[34], "カーソ\ルの高速化まで:%7d");
		strcpy(lang->fmcb[35], "左側のカーソ\ル: %s");
		strcpy(lang->fmcb[36], "右側のカーソ\ル: %s");
		strcpy(lang->fmcb[37], "上部の装飾表\示: %s");
		strcpy(lang->fmcb[38], "下部の装飾表\示: %s");
		strcpy(lang->fmcb[39], "..");
		strcpy(lang->fmcb[41], "%s を読み込みました");
		strcpy(lang->fmcb[42], "%s の読み込みに失敗しました");
		strcpy(lang->fmcb[43], "%s へ保存しました");
		strcpy(lang->fmcb[44], "%s への保存に失敗しました");
		strcpy(lang->fmcb[51], "FMCBCONFIG");
		strcpy(lang->fmcb[52], "ランチャー設定");
		strcpy(lang->fmcb[53], "メニュー設定");
		strcpy(lang->fmcb[54], "アイテム設定");
		strcpy(lang->fmcb[55], "スクロール設定");
		strcpy(lang->fmcb[56], "ESR設定");
		strcpy(lang->fmcb[27], "○:OK L1/R1:変更 L2:初期化 R2:標準化");// CNF
		strcpy(lang->fmcb[28], "○:OK L1/R1:変更");
		strcpy(lang->fmcb[59], "○:増やす ×:減らす +□:高速");//public
		strcpy(lang->fmcb[60], "○:OK");
		strcpy(lang->fmcb[61], "○:編集 ×:削除");
		strcpy(lang->fmcb[62], "○:変更");
		strcpy(lang->fmcb[63], "○:編集");
		strcpy(lang->fmcb[57], "○:編集 ×:削除 L1:MC変更 R1:機能\登録 L2:コピー R2:ペースト"); //dir
		strcpy(lang->fmcb[58], "○:編集 ×:削除 L2:コピー R2:ペースト");	//title
		strcpy(lang->fmcb[45], "○:増やす ×:減らす +□:高速 L2:コピー R2:ペースト");
		strcpy(lang->fmcb[46], "○:次へ ×:前へ L2:コピー R2:ペースト");
		strcpy(lang->fmcb[47], "○:増やす ×:減らす");
		strcpy(lang->fmcb[48], "○:次へ ×:前へ");
		strcpy(lang->fmcb[49], "設定を初期化しました");
		strcpy(lang->fmcb[50], "標準値をロードしました");
		}
		// ネットワークダウンロード
		{
		strcpy(lang->nupd[ 0], "ネットワークダウンロード");
		strcpy(lang->nupd[ 1], "ファイル: %s");
		strcpy(lang->nupd[ 2], "ダウンロード先: %s");
		strcpy(lang->nupd[ 3], "ファイル名の変更: %s");
		strcpy(lang->nupd[ 4], "バックアップ作成: %s");
		strcpy(lang->nupd[ 5], "実行する");
		strcpy(lang->nupd[ 6], "戻る");
		strcpy(lang->nupd[ 7], "ダウンロードを開始します。\nよろしいですか?");
		strcpy(lang->nupd[ 8], "ダウンロードしています。");
		strcpy(lang->nupd[ 9], "ファイルを書き込み中です。");
		strcpy(lang->nupd[10], "ファイルのダウンロードが失敗しました。");
		strcpy(lang->nupd[11], "DNSドライバを読み込み中です");
		strcpy(lang->nupd[12], "HTTPドライバを読み込み中です");
		strcpy(lang->nupd[13], "ダウンロードリストを取得しています");
		strcpy(lang->nupd[14], "ダウンロードリストの取得に失敗しました");
		strcpy(lang->nupd[15], "ファイルの検査で失敗しました");
		strcpy(lang->nupd[16], "バックアップを作成しています");
		strcpy(lang->nupd[17], "バックアップの作成に失敗しました\n続行してもよろしいですか?");
		strcpy(lang->nupd[18], "空き容量が不足しています");
		strcpy(lang->nupd[19], "○:見る L1:前へ R1:次へ R2:情報 △:戻る");
		strcpy(lang->nupd[20], "○:変更 ×/□:標準 △:戻る");
		strcpy(lang->nupd[21], "○:決定 △:戻る");
		strcpy(lang->nupd[22], "TCP/IPドライバを読み込み中です");
		strcpy(lang->nupd[23], "LANデバイスドライバを読み込み中です");
		strcpy(lang->nupd[24], "ダウンロードが完了しました");
		}
		// サウンドプレイヤー
		{
		strcpy(lang->sound[ 0], "ファイル名");
		strcpy(lang->sound[ 1], "ファイル形式");
		strcpy(lang->sound[ 2], "タイトル");
		strcpy(lang->sound[ 3], "アーティスト");
		strcpy(lang->sound[ 4], "コメント");
		strcpy(lang->sound[ 5], "日付");
		strcpy(lang->sound[ 6], "著作権");
		strcpy(lang->sound[ 7], "ビットレート");
		strcpy(lang->sound[ 8], "再生周波数");
		strcpy(lang->sound[ 9], "ビット数");
		strcpy(lang->sound[10], "チャンネル数");
		strcpy(lang->sound[11], "再生時間");
		strcpy(lang->sound[12], "再生音量");
		strcpy(lang->sound[13], "再生速度");
		strcpy(lang->sound[14], "再生位置");
		strcpy(lang->sound[15], "巻き戻し");
		strcpy(lang->sound[16], "早送り");
		strcpy(lang->sound[17], "低速化");
		strcpy(lang->sound[18], "高速化");
		strcpy(lang->sound[19], "頭出し");
		strcpy(lang->sound[20], "通常速度");
		strcpy(lang->sound[21], "再生");
		strcpy(lang->sound[22], "停止");
		strcpy(lang->sound[23], "終了");
		strcpy(lang->sound[24], "速度");
		strcpy(lang->sound[25], "位置");
		strcpy(lang->sound[26], "音声情報");
		strcpy(lang->sound[27], "一時停止 / 再開");
		strcpy(lang->sound[28], "○:早送り ×:巻き戻し △:終了 L1:頭出し L2/R2:位置");
		strcpy(lang->sound[29], "○:変更 △:終了 L2/R2:位置");
		strcpy(lang->sound[30], "○:増やす ×:減らす +□:高速 △:終了 L1:標準 L2/R2:位置");
		strcpy(lang->sound[31], "△:終了 L2/R2:位置");
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
