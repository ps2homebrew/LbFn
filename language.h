#ifndef LANGUAGE_H
#define LANGUAGE_H

#define MAX_LANGUAGE_STR 64

enum
{
	LANG_ENGLISH = 0,
	LANG_JAPANESE,
	NUM_LANG
};

typedef struct
{
	//general
	char gen_ok[MAX_LANGUAGE_STR];
	char gen_cancel[MAX_LANGUAGE_STR];
	char gen_yes[MAX_LANGUAGE_STR];
	char gen_no[MAX_LANGUAGE_STR];
	//main
	char main_launch_hint[MAX_LANGUAGE_STR];
	char main_loadhddmod[MAX_LANGUAGE_STR];
	char main_loadftpmod[MAX_LANGUAGE_STR];
	char main_notfound[MAX_LANGUAGE_STR];
	char main_readsystemcnf[MAX_LANGUAGE_STR];
	char main_failed[MAX_LANGUAGE_STR];
	char main_nodisc[MAX_LANGUAGE_STR];
	char main_detectingdisc[MAX_LANGUAGE_STR];
	char main_stopdisc[MAX_LANGUAGE_STR];
	//filer
	char filer_menu_copy[MAX_LANGUAGE_STR];
	char filer_menu_cut[MAX_LANGUAGE_STR];
	char filer_menu_paste[MAX_LANGUAGE_STR];
	char filer_menu_delete[MAX_LANGUAGE_STR];
	char filer_menu_rename[MAX_LANGUAGE_STR];
	char filer_menu_newdir[MAX_LANGUAGE_STR];
	char filer_menu_getsize[MAX_LANGUAGE_STR];
	char filer_menu_exportpsu[MAX_LANGUAGE_STR];
	char filer_menu_importpsu[MAX_LANGUAGE_STR];
	char filer_overwrite[MAX_LANGUAGE_STR];
	char filer_not_elf[MAX_LANGUAGE_STR];
#ifdef ENABLE_PSB
	char filer_execute_psb[MAX_LANGUAGE_STR];
#endif
	char filer_not_fnt[MAX_LANGUAGE_STR];
	char filer_copy_to_clip[MAX_LANGUAGE_STR];
	char filer_delete[MAX_LANGUAGE_STR];
	char filer_deletemarkfiles[MAX_LANGUAGE_STR];
	char filer_deleting[MAX_LANGUAGE_STR];
	char filer_deletefailed[MAX_LANGUAGE_STR];
	char filer_renamefailed[MAX_LANGUAGE_STR];
	char filer_pasting[MAX_LANGUAGE_STR];
	char filer_pastefailed[MAX_LANGUAGE_STR];
	char filer_direxists[MAX_LANGUAGE_STR];
	char filer_newdirfailed[MAX_LANGUAGE_STR];
	char filer_checkingsize[MAX_LANGUAGE_STR];
	char filer_getsizefailed[MAX_LANGUAGE_STR];
	char filer_export[MAX_LANGUAGE_STR];
	char filer_exportmarkfiles[MAX_LANGUAGE_STR];
	char filer_exporting[MAX_LANGUAGE_STR];
	char filer_exportfailed[MAX_LANGUAGE_STR];
	char filer_exportto[MAX_LANGUAGE_STR];
	char filer_import[MAX_LANGUAGE_STR];
	char filer_importmarkfiles[MAX_LANGUAGE_STR];
	char filer_importing[MAX_LANGUAGE_STR];
	char filer_importfailed[MAX_LANGUAGE_STR];
	char filer_importto[MAX_LANGUAGE_STR];
	char filer_keyboard_hint[MAX_LANGUAGE_STR];
	char filer_anyfile_hint1[MAX_LANGUAGE_STR];
	char filer_anyfile_hint2[MAX_LANGUAGE_STR];
	char filer_elffile_hint1[MAX_LANGUAGE_STR];
	char filer_elffile_hint2[MAX_LANGUAGE_STR];
	char filer_fntfile_hint1[MAX_LANGUAGE_STR];
	char filer_fntfile_hint2[MAX_LANGUAGE_STR];
	char filer_irxfile_hint1[MAX_LANGUAGE_STR];
	char filer_irxfile_hint2[MAX_LANGUAGE_STR];
	char filer_dir_hint[MAX_LANGUAGE_STR];
//	char filer_export_files[MAX_LANGUAGE_STR];
//	char filer_export_header[MAX_LANGUAGE_STR];
//	char filer_import_files[MAX_LANGUAGE_STR];
//	char filer_import_header[MAX_LANGUAGE_STR];
	char filer_l2popup_detail[MAX_LANGUAGE_STR];
	char filer_l2popup_dirsize[MAX_LANGUAGE_STR];
	char filer_l2popup_icon[MAX_LANGUAGE_STR];
	char filer_l2popup_flicker[MAX_LANGUAGE_STR];

	//config
	char conf_savefailed[MAX_LANGUAGE_STR];
	char conf_saveconfig[MAX_LANGUAGE_STR];
	char conf_loadconfig[MAX_LANGUAGE_STR];
	char conf_initializeconfig[MAX_LANGUAGE_STR];

	//config
	char conf_setting_button[MAX_LANGUAGE_STR];
	char conf_setting_color[MAX_LANGUAGE_STR];
	char conf_setting_screen[MAX_LANGUAGE_STR];
	char conf_setting_network[MAX_LANGUAGE_STR];
	char conf_setting_font[MAX_LANGUAGE_STR];
	char conf_setting_misc[MAX_LANGUAGE_STR];
	//button
	char conf_buttonsettinginit[MAX_LANGUAGE_STR];
	//color
	char conf_background[MAX_LANGUAGE_STR];
	char conf_frame[MAX_LANGUAGE_STR];
	char conf_normaltext[MAX_LANGUAGE_STR];
	char conf_highlighttext[MAX_LANGUAGE_STR];
	char conf_disabletext[MAX_LANGUAGE_STR];
	char conf_folder[MAX_LANGUAGE_STR];
	char conf_file[MAX_LANGUAGE_STR];
	char conf_ps2save[MAX_LANGUAGE_STR];
	char conf_ps1save[MAX_LANGUAGE_STR];
	char conf_elffile[MAX_LANGUAGE_STR];
	char conf_psufile[MAX_LANGUAGE_STR];
	char conf_flicker_alpha[MAX_LANGUAGE_STR];
	char conf_presetcolor[MAX_LANGUAGE_STR];
	//screen
	char conf_screen_x[MAX_LANGUAGE_STR];
	char conf_screen_y[MAX_LANGUAGE_STR];
	char conf_tvmode[MAX_LANGUAGE_STR];
	char conf_interlace[MAX_LANGUAGE_STR];
	char conf_ffmode[MAX_LANGUAGE_STR];
	char conf_ffmode_field[MAX_LANGUAGE_STR];
	char conf_ffmode_frame[MAX_LANGUAGE_STR];
	char conf_screen_scan[MAX_LANGUAGE_STR];
	char conf_screen_scan_full[MAX_LANGUAGE_STR];
	char conf_screen_scan_crop[MAX_LANGUAGE_STR];
	char conf_flickercontrol[MAX_LANGUAGE_STR];
	char conf_screensettinginit[MAX_LANGUAGE_STR];
	//network
	char conf_ipaddress[MAX_LANGUAGE_STR];
	char conf_netmask[MAX_LANGUAGE_STR];
	char conf_gateway[MAX_LANGUAGE_STR];
	char conf_ipoverwrite[MAX_LANGUAGE_STR];
	char conf_ipsettinginit[MAX_LANGUAGE_STR];
	char conf_ipsaved[MAX_LANGUAGE_STR];
	char conf_ipsavefailed[MAX_LANGUAGE_STR];
	//font
	char conf_AsciiFont[MAX_LANGUAGE_STR];
	char conf_KanjiFont[MAX_LANGUAGE_STR];
	char conf_CharMargin[MAX_LANGUAGE_STR];
	char conf_LineMargin[MAX_LANGUAGE_STR];
	char conf_FontBold[MAX_LANGUAGE_STR];
	char conf_FontHalf[MAX_LANGUAGE_STR];
	char conf_FontVHalf[MAX_LANGUAGE_STR];
	char conf_AsciiMarginTop[MAX_LANGUAGE_STR];
	char conf_AsciiMarginLeft[MAX_LANGUAGE_STR];
	char conf_KanjiMarginTop[MAX_LANGUAGE_STR];
	char conf_KanjiMarginLeft[MAX_LANGUAGE_STR];
	char conf_FontScaler[MAX_LANGUAGE_STR];
	char conf_FontScaler_A[MAX_LANGUAGE_STR];
	char conf_FontScaler_B[MAX_LANGUAGE_STR];
	char conf_FontScaler_C[MAX_LANGUAGE_STR];
	char conf_fontsettinginit[MAX_LANGUAGE_STR];
	//misc
	char conf_language[MAX_LANGUAGE_STR];
	char conf_language_us[MAX_LANGUAGE_STR];
	char conf_language_jp[MAX_LANGUAGE_STR];
	char conf_timeout[MAX_LANGUAGE_STR];
	char conf_disc_control[MAX_LANGUAGE_STR];
	char conf_print_only_filename[MAX_LANGUAGE_STR];
	char conf_fileicon[MAX_LANGUAGE_STR];
	char conf_disc_ps2save_check[MAX_LANGUAGE_STR];
	char conf_disc_elf_check[MAX_LANGUAGE_STR];
	char conf_file_ps2save_check[MAX_LANGUAGE_STR];
	char conf_file_elf_check[MAX_LANGUAGE_STR];
	char conf_export_dir[MAX_LANGUAGE_STR];
	char conf_defaulttitle[MAX_LANGUAGE_STR];
	char conf_defaultdetail[MAX_LANGUAGE_STR];
	char conf_defaultdetail_none[MAX_LANGUAGE_STR];
	char conf_defaultdetail_size[MAX_LANGUAGE_STR];
	char conf_defaultdetail_modifytime[MAX_LANGUAGE_STR];
	char conf_usbd_use[MAX_LANGUAGE_STR];
	char conf_usbd_path[MAX_LANGUAGE_STR];
	char conf_usbmass_use[MAX_LANGUAGE_STR];
	char conf_usbmass_path[MAX_LANGUAGE_STR];
	char conf_auto_decompression[MAX_LANGUAGE_STR];
	char conf_miscsettinginit[MAX_LANGUAGE_STR];

	char conf_ok[MAX_LANGUAGE_STR];
	char conf_cancel[MAX_LANGUAGE_STR];
	char conf_on[MAX_LANGUAGE_STR];
	char conf_off[MAX_LANGUAGE_STR];
	char conf_edit[MAX_LANGUAGE_STR];
	char conf_clear[MAX_LANGUAGE_STR];
	char conf_add[MAX_LANGUAGE_STR];
	char conf_away[MAX_LANGUAGE_STR];
	char conf_change[MAX_LANGUAGE_STR];
	char conf_up[MAX_LANGUAGE_STR];
	char conf_horizontalresolution[MAX_LANGUAGE_STR];
	char conf_fast[MAX_LANGUAGE_STR];
} LANGUAGE;

#endif
