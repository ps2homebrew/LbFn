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
	char filer_exportfailed[MAX_LANGUAGE_STR];
	char filer_exportto[MAX_LANGUAGE_STR];
	char filer_importfailed[MAX_LANGUAGE_STR];
	char filer_importto[MAX_LANGUAGE_STR];
	char filer_keyboard_hint[MAX_LANGUAGE_STR];
	char filer_hint1[MAX_LANGUAGE_STR];
	char filer_hint2[MAX_LANGUAGE_STR];
	char filer_hint3[MAX_LANGUAGE_STR];
	char filer_hint4[MAX_LANGUAGE_STR];
	char filer_hint5[MAX_LANGUAGE_STR];
	char filer_export_files[MAX_LANGUAGE_STR];
	char filer_export_header[MAX_LANGUAGE_STR];
	char filer_import_files[MAX_LANGUAGE_STR];
	char filer_import_header[MAX_LANGUAGE_STR];

	//config
	char conf_savefailed[MAX_LANGUAGE_STR];
	char conf_saveconfig[MAX_LANGUAGE_STR];
	char conf_loadconfig[MAX_LANGUAGE_STR];
	char conf_initializeconfig[MAX_LANGUAGE_STR];

	//config
	char conf_menu[MAX_LANGUAGE_STR];
	//button
	char conf_buttonsettinginit[MAX_LANGUAGE_STR];
	//screen
	char conf_background[MAX_LANGUAGE_STR];
	char conf_frame[MAX_LANGUAGE_STR];
	char conf_highlighttext[MAX_LANGUAGE_STR];
	char conf_normaltext[MAX_LANGUAGE_STR];
	char conf_folder[MAX_LANGUAGE_STR];
	char conf_file[MAX_LANGUAGE_STR];
	char conf_ps2save[MAX_LANGUAGE_STR];
	char conf_elffile[MAX_LANGUAGE_STR];
	char conf_screen_x[MAX_LANGUAGE_STR];
	char conf_screen_y[MAX_LANGUAGE_STR];
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
	char conf_export_dir[MAX_LANGUAGE_STR];
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

} LANGUAGE;

#endif
