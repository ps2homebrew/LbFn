# History

## LbF

Initially in 2006 user yi (or yi4xx) created his own fork of LaunchELF v3.41 called **LbF**. It was modified mainly for better Japanese language support. It was developed until the end of 2008. Last version: LbF v0.70.
[Original homepage](https://web.archive.org/web/20181105030806id_/http://sky.geocities.jp/yi4xx/) - backup at webarchive.

## LbFn

In 2008 user [nika](https://twitter.com/nika_towns/with_replies) continued work on **LbF** and changed its name to **LbFn**. The app was actively developed until 2014. Last version: LbFn v0.70.19
[Official homepage](https://wiki.nika-2ch.net/?appli/LbFn).
Source code for latest version (2012-2014) was lost, so last source code published for v0.70.18.

## Original readme (English)

### LbFn v0.70.19 WIP by nika (translate by based on excite translate)

- About LbFn
  - LbFn is an unofficial filer remodeled based on LbF. 
  - LbF is an unofficial filer remodeled based on LaunchELF v3.41 by yi.

- Attached files
  - readme.txt
  - readme_e.txt
  - LbFnc.ELF (compressed on default setting, by ps2-packer.exe)

- Point different from LaunchELFv3.41 and LbFv0.70
  - Can be used by the resolution such as 640x448. (and customized it)
  - Can be screen output by 720p and 1080i, VGA modes.
  - The character can be displayed by using the font of BIOS. 
  - The character can be displayed by using the font file of the FONTX2 type (Can be used tek5 compression). (Max: 64 dot width)
  - The save data can be exported to the memory juggler's psu file (Compatible is not complete). 
  - The import of the save data can be done from the memory juggler's psu file (Compatible is not complete). 
  - The screen position of each resolution can be adjusted. 
  - The smoothing of the font can be done in each resolution. 
  - The screen size can be changed to the full size (720x480 etc.). 
  - Can be used as a FTP server function. 
  - Can be format the PS2 memory card. 
  - text file and image file can be display. 
  - Japanese input is possible by the virtual keyboard. 
  - The setting change like FMCB_CFG.ELF is possible. 
  - Network update supported. 
  - Virtual MemoryCard supported. 
  - Playback NSF supported. 

- About the settings
  - Setting filename is LBFN.INI 
  - There is no compatible with the CNF file to LbF and LbFn v0.70.10. 
  - All the configuration file names became capital letters from LbF v0.43. 
  - LBFN.INI is preserved in the booted folder or SYS-CONF folder. 
  - LBFN.INI is read from booted folder or the SYS-CONF folder. When starting from HDD, only the SYS-CONF folder.
  - IPCONFIG edits mc0:/SYS-CONF/IPCONFIG.DAT. The folder is made when there is no SYS-CONF folder. 
  - If DISC PS2SAVE CHECK was dissabled, that is not checked whether the folder in CD/DVD is PS2 save data. 
  - If DISC ELF CHECK was dissabled, that is skip whether the file in CD/DVD is ELF file. 
  - If FILE PS2SAVE CHECK was dissabled, that is not checked whether the folder in HDD and more is PS2 save data. 
  - If FILE ELF CHECK was dissabled, that is skip whether the file in not CD/DVD is ELF file. 
  - The folder and the file type are fast if it doesn't check it. 
  - The external font file that can be used is FONTX2 type (with tek compression supported). 

- About the export and the import the psu file
  - Compatibility is not complete. 
  - The memory juggler might not be able to treat psu that exports with LbFn. 
  - Psu made by the memory juggler might not be able to treat LbFn. 
  - Import is made from only mc0 or mc1. 
  - Import does not support to the subfolder. 
  - Psu is made for the specified folder when the export folder is specified. 
  - When the export folder is not specified, psu is made for that place. 
  - There is a possibility of garbling when a wrong extend driver is used when exporting to mass.
  - Supported psu export with timestamp and crc. (mc is not support)

- About the compatibility of the psu file
  - The interchangeability of the psu file is do it open or able to edit and it judged it by the corresponding application. 
  In "ps2save-builder" and "PSU File Manager", etc. it is possible to open. 
  - The save data that the game made is never touched, exporting what is high, and compatibility is the highest. 
  yi think software that edit the psu file for who to be able to read. 
  - After moving and deleting the file in the save folder by using FileBrowser,
  the exporting psu file has the possibility that compatibility with memory juggler's psu disappears. 
  - Unofficial LaunchELF makes the save data and when mcPaste is done from mc besides mc, the bin file that records the attribute etc. is made. 
  It becomes not compatible because it is exported that the folder is exported including the bin file. 
  It might be good when exporting after the bin file is deleted. 

- About an external driver
  - When LbFn.ELF is booted from MC or CD or HDD, an arbitrary USB mass storage driver and the USB driver can be handled. 
  - When booting from USB, the configuration file of mc is used as for the external driver's setting. 
  - The kind of USB thumb drive that can be read might increase if an outside USB mass storage driver and the USB driver are handled. 
  - The USB mass storage driver and the USB driver that uses it think that the stability versions such as "USBHDFSD.IRX" and "USBD.IRX" of the FreeMcBOOT attachment are good. 
  - An arbitrary external USB mass storage driver should exist on mc. 
  - Customized version of "USBHDFSD.IRX" (Rev.1534) to be built in LbFn.ELF. 
  - The driver built in is used when failing in reading an arbitrary external driver. 
  - The Japanese file name written with the file name and PC written to use the USB mass storage driver that doesn't correspond to Japanese from PS2 is garbled. 
  - The USB keyboard can be used by preparing the USB keyboard driver on the screen of a soft keyboard. 
  - When the USB keyboard is used, it is necessary to prepare the USB keyboard driver separately (It is not built in LbFn). 
  - The file of the arbitrary external driver can use the tek compressed one (tek5 compression recommendation). 

- Memorycard format
  - When booting while pushing "L1+L2+R1+R2", the memory card of mc0: can be formatted. 
  - It formats after "mc0:/BxDATA-SYSTEM/BOOT.ELF" and "mc0:/BxDATA-SYSTEM/TITLE.DB" are backed up on the memory, and it returns it. 
  - If "MISC/McFormat" is executed, it becomes the same as the time that booted while pushing "L1+L2+R1+R2". 

- About the FMCB Configurator function
  - This is mounted based on the operation of Free McBoot Configurator 1.3 beta 6 (FMCB 1.8 test 22).
  - Operation excluding the Free MC Boot v1.8 system is non-recommendation. I will recommend other versions (before v1.7 and since v1.9 etc.) not to use it. 
  - Because this function is a temporary mounting, the operation of each version of LbFn might be changed. 
  - Even if the setting is changed, it is not automatically preserved. Please preserve it before exit this function when you want to update the setting. 
  - Operation of Free MC Boot v1.9 and after was not checked.  

- About the function of the network update
  - The downloadable file can be preserved by connecting with the update server. 
  - Please never execute it in PlayStation2 not connected with the network. 
  - Please do not occupy the band with PC etc. while executing the network update. 
  - Please do not touch the memory card, the controller, and LAN cable, etc. while executing the network update. 
  - push triangle button to abort the download.

- About the function of the audio player
  - Current version was not supported.

- About the virtual memorycard support
  - This function is maybe incomplete. 
  - Virtual memorycard of made in PSP and PS3 was not supported. 
  - Do not use virtual memorycard of operationed in old LbFn and uLaunchELF. 
  - Can not format and create new virtual memorycard.  
  - Operation in HD Project/OPL and restored PMC are unconfirmed. Self-responsibility, please.

- About the function of NSF/NES Player
  - This function is incomplete. 
  - Rendering engine is modify based InfoNES v0.91 for OSASK and after. 
  - Support audio is pulse/triangle/noise/DPCM/VRC6/VRC7/FDS/MMC5/N163/SUNSOFT5B. 
  - APU and VRC7 was not completed. 
  - No menu. 
  - L1R1 button is change the screen in NSF files. 
  - L1+R1 push to pause in NES files, after CIRCLE/CROSS/START/TRIANGLE button are continue/reset/reboot/quit.

- About the operation of NES Playing
  - circle -> A button
  - cross -> B button
  - square -> ZOOM modes
  - triangle -> frame toggle
  - L1+[circle/cross/square/triangle/R2/START/SELECT] -> audio mute
  - L1+L2 -> N163 noise mode
  - R1+[circle/cross] -> rapid A/B button
  - L2 -> wait toggle
  - R2 -> screen filter toggle
  - L1+R1 -> pause/menu

- Exemption matters
  - Use it by the self-responsibility please. Sales are the prohibitions, distribute it again freely. 
  - The license is the same as LbF and LaunchELF. 
  - Please do not inquire the part concerning remodeling since LbFn of yi. 

- Web sites
  - [Application's page (at nika's wiki)](https://wiki.nika-2ch.net/?appli/LbFn)
  - [Released site](http://www.geocities.jp/nika_towns/)
  - [nika's forum](http://nika-2ch.net/bbs/)
  - http://nika-2ch.net/bbs/
  
When the character that cannot be input with a virtual keyboard is discovered or the trouble of FMCBCONFIG is found, being possible to report to nika's wiki might be good. 

- Thanks (Honorific title omission, In any order)
  - LbF
    - [yi](http://sky.geocities.jp/yi4xx/)
  - LaunchELF  
    - [mirakichi](http://mirakichi.karou.jp/software/)
  - [Unofficial LaunchELF](https://github.com/ps2homebrew/wLaunchELF)
  - [Free McBoot](https://sites.google.com/view/ysai187/home/projects/fmcbfhdb)
  - [Altimit v0.1 (alpha test version)](https://github.com/ps2homebrew/altimit)
  - [ps2menu](https://github.com/ps2homebrew/ps2menu)
  - OSASK (tek compression format)
    - Hidemi Kawai
    - http://osask.jp/
    - http://community.osdev.info/index.php?tek
  - JPEG decoder
    - nikq, ku-min, Hidemi Kawai, I.Tak., Akkie

## History of update

### 2014/12/20 LbFn ver0.70.19 WIP

*Source code was lost, so this WIP is made a regular version.
  - Fixed JPEG decoder. 
  - Added fast font draw for texture cached font.
  - Fixed psu import and psu export. 
  - Supported Virtual Memorycard. 
  - Added display the freespace in mass. 
  - Supported read of zip compression folder. 
  - Supported 2nd port connected controler. 
  - Disabled Audio player. 
  - Added NSF/NES Player. 
  - Supported can be regist of not ELF files to launcher. 
  - Other update was unknown. 

### 2011/10/22 LbFn ver0.70.18
  - Fixed refresh the wallpaper, that in change the screen size and preview, and more... .
  - Fixed exit case of no release the malloced memory buffer in image viewer.

### 2011/10/10 LbFn ver0.70.17
  - Fixed display the wrong letter of bottom message in MessageBox. 
  - Fixed do not display the connecting message in "MISC/SelfUpdate". 
  - Supported PNG images in image viewer. (Max 3M pixels)
  - Fixed can be display GIF images of only "GIF89a" in image viewer. 
  - Fixed memory fragmented of tek compressed font in use. 
  - Added audio player. (choice the "look" from popup menu of FileBrowser)
  - Supported playback of WAV files in audio player. (Max. 24MB, recommend in PCM format of 48kHz/16bit/2ch)
  - Fixed output wronged filename in psu export. 
  - Added smallest file check the psu format in psu import. 
  - Changed and added preset setting in "MISC/GSCONFIG". 
  - Changed can be setting of two resolutions in "MISC/GSCONFIG". 
  - Added doublebuffer mode of like 2x vertical resolution only NTSC and PAL in "MISC/GSCONFIG". 
  - Fixed press any button to exit of "MISC/ShowFont". 
  - Fixed dithering matrix are vertical smoothing at if FFMODE are FRAME and dithering enabled. 
  - Added the texture load operation by libgs.c based process. 
  - Supported new texture load function in use of the imageviewer in case of do not resize. 
  - Supported automatic animation in imageviewer. 
  - Fixed "DO NOT RESIZE IN IMAGE" was counterproductived in "VIEWER SETTING". 
  - Fixed do not display 4096MB or bigger size of "GET SIZE" in FileBrowser. 
  - Supported setting of first displaying image position in image viewer. 
  - Supported wallpaper in use. (if in used, can be "LOOK" the file of filesize limit to smaller in viewer and others) 
  - Fixed can not open the image in case of downgrade the bit per pixel in small of memory remain.
  - Supported dump the screenshot in pad operation. 
  - Fixed again the wronged short filename making case in Japanese supoorted version of USB mass storage driver.  
  - and more small changes...

[Full changelog (translated to English)](HISTORY.md)
