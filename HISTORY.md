# Detailed changelog

## 2014/12/20 LbFn ver0.70.19 WIP
*Source code was lost, so this WIP is made a regular version.* 
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

## 2011/10/22 LbFn ver0.70.18
- Fixed refresh the wallpaper, that in change the screen size and preview, and more... .
- Fixed exit case of no release the malloced memory buffer in image viewer.

## 2011/10/10 LbFn ver0.70.17
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

## 2010/09/04 LbFn ver0.70.16
- Fixed very slower decode case of GIF image.
- Supported can be display many images of a file of GIF image and PS1 icon in imageviewer.
- Faster the font drawing in that "FIX FONT MODE" are "fast".
- Fixed drawing speed of no resize time in imageviewer.
- Supported CRC processing abort in FileBrowser of "Get Size".
- Supported can be ignored the control characters of SBCS fontset.
- Added protect of access error in highest main memory address.
- Added "History" and "Custom" input character type in virtual keyboard.
- Fixed bug of no change the target session at made that new session in setting file control module (cnf.c).

## 2010/07/03 LbFn ver0.70.15
- Supported display the savedata icon and texture in image viewer. 
- Fixed "LIST TOP FOLDER" bug in mc.
- Added VGA and 576p TV MODEs to preset setting of "MISC/GSCONFIG".
- Fixed incorrect request header of download in network update. 
- Supported abort the download of network update. 
- Fixed about the DBCS font bug. 
- Fixed ".." display bug at file look after marked many files.
- Aborted compression of inside modules. 
- Fixed incorrect displaying timing of bottom message in "MISC/FMCBCONFIG". 
- Supported psu export with modified timestamp and CRC. (to mc is can not support)
- Added can be display the CRC in "Get Size" of FileBrowser menu.

## 2010/01/12 LbFn ver0.70.14
- It was made to overwrite without confirming it even if the save data existed when the psu import was done. 
- It is corrected that a part of item was not a Japanese display by the setting of the menu of "MISC/FMCBCONFIG". 
- The dithering was corrected in the screen setting of "MISC/CONFIG" and the ineffectual one was corrected about turning on. 
- made charactor list of raw from the DBCS font for which of a virtual keyboard was used. 
- The case where it booted specifying ELF with the PS2 emulator was recognized. 
- The setting was able to be preserved even if there was no SYS-CONF in the memory card when booting from the place that was not able to be written. 
- The flicker control and the processing of the setting of the font bold are supported when the font fix mode is "Normal". 
- The alpha blend was used by drawing when the font correction mode was "Normal". 
- Drawing was made to prepare it when the DBCS font was initialized. 
- "ASCII (SBCS)" and "JIS (ISO-2022-JP)" are added to the character-code of the text viewer. 
  Note: "ASCII (SBCS)" is off the subject of the character-code judgment. The operation that switches the character-code is necessary if necessary. 
- Drawing around the screen of the psu import and the psu export has been improved. 
- The font file for uLaunchELF was made available. (only SBCS fontset)
- Corrected the scrollbar occasionally flew out with a virtual keyboard. 
- Character of 0x01-0x1F was displayed excluding a virtual keyboard and the text viewer. 
- When the font was prepared in Character of 0x01-0x1F, it was given priority and displayed. 
- It is corrected that a short file name was not correctly generated with the USB mass storage driver for Japanese. 
- Change the unicode conversion table from part of "sjis-0213-200-std.txt" to "CP932.TXT". 
- Faster image rendering in image viewer. 
- Can be dissable of resize image in image viewer. 
- Added display using SBCS fontset in "MISC/ShowFont". 
- The height of the font set of the standard was treated as 16 dots. 
- Change the display message in part of config menu etc.
- Supported transparencies in image viewer and GIF decoder. 
- The code of the character input with the virtual keyboard was displayed. 
- Added network update in "MISC/SelfUpdate". 

## 2009/08/19 LbFn ver0.70.13
- It is corrected partially of "MISC/CONFIG" that the passing display was wrong by the menu. 
- Japanese etc. were able to be input with the virtual keyboard (To the Chinese character of the level 1). 
- The setting of FreeMCBoot was able to be edited in "MISC/FMCBCONFIG". 
  Note: It mounted based on the operation of "Free McBoot Configurator 1.3 beta 6 (FMCB 1.8 test 22)"
- The processing code of the menu was newly made (used only "MISC/FMCBCONFIG", now).

## 2009/06/02 LbFn ver0.70.12
- The image viewer supported the GIF image. (Supported GIF87a, max 20M pixels)
- That when the JPEG image was opened, the buffer was broken. This bug was fixed. 
- It was corrected to have returned the time out of launcher setting to 10 seconds. 
- FFMODE was selectable to FRAME in double buffer of easy mode in "MISC/GSCONFIG". 
- The set number that can be used by "MISC/GSCONFIG" has been increased from 10 to 25 or less. 
- Two or more picture files that the image viewer had marked were supported. 
- The display when a set name is a unsetting by "MISC/GSCONFIG" is changed. 
- The text was able to be turned by the text viewer on a right edge of the screen. 
- The tek compressed file was able to be opened in text viewer. 
- The operation of a set menu of the button of launcher setting was changed. 
- Corrected not to be update to the screen after FFMODE is changed to FRAME. 
- Corrected to seem not to be reflected when the setting of the button of the launcher setting is changed and it returns. 
- A part of operation when selected by D-PAD is done with launcher returns is changed. 

## 2009/04/20 LbFn ver0.70.11

- "MISC / GSCONFIG" was to be able to further customize the resolution and the number of colors in the (up to 10 to)
  - Only now easily set.
  - larger resolution setting than the screen to be output can not be basically.
    It can be changed in a range of screen sizes, such as NTSC or 720p.
- The TV MODE was to be selected from the list
Launcher one in the pad of a button to a file or function register was to be able to register up to five
Of the launcher item was to be able to change the display name
Corresponding to press simultaneous plurality of buttons in the launcher
Change, the configuration file name from the "LBF.CNF" to "LBFN.INI"
- an extension of the format of the configuration file
- add an item of "color outside the screen" in the color scheme set
- Each color of the increase or decrease of the color scheme set was also to be able to speed up
Screen Fixed keyboard caret is no longer flashing
Change-Screen position adjustment setting to a value relative to the standard position
- The range determination of when reading from a configuration file simplified
Font correction mode to some extent Fixed becomes thinner and reduction correction in the standard
The value of each item of font set was to be able to return to the standard value
- In the "MISC / CONFIG" it was to be able to save the settings of the text viewer and image viewer
FFMODE - has implemented the process in the case of FRAME
When - 960x1080i such as pixels are not square, Fixed aspect ratio in the image viewer is funny
Flicker control and bold set was also to be saved for each resolution
- Add a preset color scheme settings (high contrast and osask.jp style), you change the initial color scheme
- relaxed and configuration files control routine of restriction
- The "IP Settings" has been moved to the "MISC / IPCONFIG"
- The access of the "IPCONFIG.DAT" mc0: Fixed only did not look
- Screen settings in the same way as the "AUTO" was to select the default language
- And to be able to access multiple mass storage enabled devices in the configuration
File was to be able to change the order of the list
- And the JPEG decompression routine to be decoded even 16bpp (viewable size in the case of 16bpp up to about 12 megapixels)
- Fixed color decoded image there is likely to be wrong in JPEG decompression routine
It was not allowed to exceed the range of the number of viewable lines in the text viewer
- The soft keyboard has to be able to input from the USB keyboard (However, alphanumeric characters and half-width symbols only)
It was to display the processing content when you select the "Display" in the menu of the filer
It was to be able to display the image of the Windows Bitmap format image viewer

## 2009/01/24 LbFn ver0.70.10

A user can change the character code in the text viewer
- it was to be able to change the flicker control in text viewer
- was to allow the horizontal scrolling in a text viewer
- other than when it is needed other than the text viewer was not to the re-drawing
And horizontal resolution is to allow the FFMODE to FIELD even 1080i of 1920 (although it may flicker is generated at the time of re-drawing of the screen)
Font flicker control in the drawing routine is to carry out the processing at the time of ON (when the correction is "high speed" only)
Font to separate the routine to draw was to use the same drawing routines in both the half-width, full-width
External font files in the width of the characters was to be able to use up to 64 dots (in the case of "high speed")
The font of the drawing method "standard" add (bilinear resizing) (?)
Font of was in the drawing method can be selected (Please select the "fast" if you want to use a conventional drawing method)
- Abolition of less than horizontal resolution 480 of 1080i
Soft keyboard Fixed did not correspond to the font correction of
- Host fixes a bug that may not be able to access the configuration file when you start from
Change - 720p output of the 32bpp
- If also the output of 1080i 32bpp is able to change so that the 32bpp
- Disable dithering of GS at the time of the 16bpp
- Add an image viewer (choose the "View" from the menu on the filer)
- OSASK were transplanted JPEG decompression routine from the DLL "PICTURE0.BIN" that are used to deploy the images (up to a viewable size of about 6 megapixels)
- Fixed correction value of the font height "Initialization of the setting of the screen" is not initialized
And change some of the default value
- Since the font of the "standard" of the drawing method is drawing speed is quite slow in spite of the non-corresponding to flicker and bold, usually, please use the "fast"
  By the way, width - height (if you are expanding correction is the case) can be an external font file is the use of up to 65536 dots

## 2009/01/11 LbFn ver0.70.9

- Add a text viewer (choose the "View" from the menu on the filer)

## 2009/01/07 LbFn ver0.70.8

- When you start from a USB mass storage enabled devices, Fixed configuration file that exists can not be read in the same location

## 2009/01/04 LbFn ver0.70.7

- Fixed not to be able to start from a USB mass storage enabled devices in the update of v0.70.5
- "MISC / PS2Disc" Modify the dozens of times execution (failure) Then bug that can no longer be anything
- any of you to be able to specify an external USB driver (USBD.IRX)
- maybe fix a bug that height and the number obtained by adding the distance between the line of the font hangs If it is less than 1
Change, "MISC / INFO" the driver reads the status of the USB relationship always to be displayed
- it was to display the kind of "MISC / DiscStop" even disk
- The external driver mc: when to have put, mc0: and mc1: mc from the slot fixed: (mc0: > mc1: the search) was to be able to
- Color scheme adjust the alpha value of the flicker control settings

## 2008/12/29 LbFn ver0.70.6

- Tek corresponding to the compressed external IRX driver
Changed or built-in to "USBHDFSD.IRX" to those of Japanese support
- abolish the selection of character code when calling the USB Mass Storage driver
- was to be able to operations other than psu also a USB mass storage folder, including Japanese of device
- Modify because ish operation of the flicker control was incorrect
- color scheme set was to be able to adjust the alpha value of the flicker control in
Message partially modified

## 2008/12/21 LbFn ver0.70.5

- it was to be able to change the horizontal resolution of 1080i (except 1920x1080 is FFMODE will FRAME)
And change the color of the display of values ??in two hexadecimal digits
- Screen of the items in the set was separated into an independent set items that are related to the color scheme
FFMODE - has reduced the amount of VRAM in the case of FRAME
- for each resolution it was to be able to correct the font size
Message box in the line Fixed there was a case that overlaps in character
- Change the font of drawing routine, reduce the number of primitives to be used
- was an external USB mass storage driver to be specified
Change or are built-in to "USBHDFSD.IRX" a thing of the UTF-8 support
- The character code of the folder file name when calling the USB mass storage driver has to be able to select
- However, such as a folder, including the Japanese will not be performed is psu import, export, and title operations other than the display
- Tek corresponding to the compressed to have an external font file
And other various minor modifications, such as changing the standard settings

## 2008/11/26 LbFn ver0.70.4

- 480i fixes the setting of horizontal position to always two save those for fine adjustment and what the LbFn of LbF compatible screen
- for from LbFn v0.69.1 v0.70.3 is to record the settings for fine adjustment to those of LbF compatible, make the correction only when the setting value for the fine adjustment of the LbFn does not exist
Therefore, if is extremely changed the horizontal position of the screen of 480i in LbF and LbFn v0.70.4 earlier, may cause a malfunction
- postscript: the horizontal position of the recorded 480i in LbFn but will be reflected even LbF, the vertical position of not reflected (LbF in LbFn also by changing the horizontal position in LbF is reflected in the vertical position of the 480i of LbFn masu)

## 2008/11/26 LbFn ver0.70.3

- Fixed no longer be able to launch the ELF on the HDD from the launcher
A - MISC / McFormat and MISC / DiscStop was to be able to launcher registration
- In the initialization of the screen set to the value of AUTO / NTSC / PAL (480i) other than the screen size of Fixed not initialized

## 2008/11/22 LbFn ver0.70.2

- When was the ELF on the HDD is activated from the launcher, Fixed hung with black screen when the file can not be found
- a routine that is executed when you start holding down the "L1 + L2 + R1 + R2" was in that can be called from the MISC / McFormat
- A disk rotation stop was to be performed by the MISC / DiscStop (where non-supported type of disc in the display)

## 2008/11/03 LbFn ver0.70.1

- The screen size was to be able to full size
- CD / DVD and other than the MC was to be able to not check the save data

## 2008/11/03 LbF ver0.70

- Psu in the file name of the exported file in the windows Fixed some may contain characters that are not allowed in the file name

## 2008/11/01 LbFn ver0.69.1

- 1080i also has to be able to display (where horizontal resolution is extended character is next to half)
Other than - CD / DVD drive was to be able to avoid the ELF also check
- was to be determined by extension, even when not in the ELF check
- full-width fonts up to width of 32 dots to be able to use
- for each output resolution was to hold the position setting of the screen (except AUTO / NTSC / PAL will equate)
- the horizontal position of the screen in the same manner as uLaunchELF was to allow fine adjustment
- you change some of the default value

## 2008/05/14 LbF ver0.69

- you change the display of Messagebox
- the marked file was to be able to import / export
- you change the display when the import / exporting

## 2008/03/27 LbF ver0.68

Fixed psu export and import psu of the screen is funny in FileBrowser ·

## 2008/03/21 LbF ver0.67

- Even if you switch the display by pressing the L1 in FileBrowser, the cursor so as not to move
- When the display of the game title in FileBrowser, was to be displayed to get the save data name of the psu file
- Screen add the color of the psu file's icon to set
Of - FileBrowser file name display of a default were to be able to game title
- a detailed display default of FileBrowser was to be able to select

## 2007/10/28 LbF ver0.66

- From marked with FileBrowser, Fixed there remains a mark also switch the display by pressing the L1 button
- Fixed can not boot from the CD
- Export Fixed there be output fails to mass in psu
Modify the bad character of sjis in - Export psu
And adjustment to shorten the display of file names in FileBrowser
Scroll bar adjusts the display of
And adjustment details Show FileBrowser
Of the software keyboard was to display the frame at the cursor

## 2007/10/20 LbF ver0.65

Modify the sort order is funny in FileBrowser ·
- When you are not insert the mc, Fixed there is that free space is displayed
- Export when you cancel in the psu, was not to display the error message
- Import When you cancel in the psu, was not to display the error message
- Display switching of when you press the L1 button at FileBrowser has to be faster
It was to display the menu when you press the L2 button on the FileBrowser ·
- When you press the L3 button in FileBrowser, abolished the flicker control can ON / OFF
- When you press the R3 button at FileBrowser, abolished can turn ON / OFF the icon display

## 2007/09/25 LbF ver0.64

- Export in psu, attribute, update date and time, was to save the creation date and time
- Import in psu, attribute, update date and time, was to restore the creation date and time
Copy from mc to mc, when you move, attribute, update date and time, creation date and time it was set to be the same
- When you Get Size in mc, and to display the attribute
- Screen was to add the color of invalid characters in the set
Presets you change the color scheme of

## 2007/09/18 LbF ver0.63

- free space on the memory card of ps1 was set to be the correct value
- When you start holding down SELECT button, and so as to initialize and start SCREEN SETTING
- When you cancel the CONFIG, fails the font loading and modify the display of collapse
- Em "over" is a half-width "-" Fixed has become
In - ps1 memory card modifications from being able to output in the Export psu
- Ps1 to format the memory card and Fixed freeze
In - ps1 memory card of Fixed she can save the IPCONFIG.DAT
In - ps1 memory card of Fixed she can save the LBF.CNF
- Itogs.c adjust the

## 2007/09/09 LbF ver0.62

Change the itogs.c of - Libito 0.2.1 it was to be able to output at 480p and 720p
- Screen to TV MODE add
- it was to be able to display the horizontal size of the ASCII font up to 16 dots
- the horizontal size of the Chinese character fonts to be able to display up to 24 dots

## 2007/07/10 LbF ver0.61

- Fixed you can not copy a file from the HDD to other HDD
- When you start from the USB of Memokabuto mock and SMBOOT in SWAPMAGIC, Fixed can not be set file read in the MASS
- it was to display the type when the disc has stopped
- PS1 was to display the save data name
- Screen add the color of the icon of PS1 save data to set
- was put L1 of the description of the FileBrowser (when the Japanese)
- full-width alphanumeric characters and double-byte symbol, adjust the place to be converted to single-byte

## 2007/05/06 LbF ver0.60

- MISC / PS2Browser modification

## 2007/04/19 LbF ver0.59

- PowerOff fix that is to not be able to

## 2007/04/19 LbF ver0.58

- PowerOff Fixed can not
- Modify the is the that the configuration file can not be read

## 2007/04/18 LbF ver0.57

- was needed was not so as not to incorporate a "netfs.irx"
- Psb was file execution function abolition
- R1 copy and paste menu, abolish can run with the button
- Mc format adjustment
Change or the loader.elf to those of uLaunchELF4.12
Change or the name of the "PS2Net" to "PS2Ftpd"
- Add the PowerOff to MISC

## 2007/03/08 LbF ver0.56

- The launcher Fixed unable to register the functions of the MISC

## 2007/03/03 LbF ver0.55

- R1 copy and paste menu, it was to be able to run with the button
- When you start holding down the "L1 + L2 + R1 + R2", was to be able to format the memory card
- Psb was to be able to file execution
- External IRX was returned file read to allow the (USB_MASS.IRX only)

## 2007/01/17 LbF ver0.54

Incorporating usbhdfsd.irx instead of - usb_mass.irx
  (Mass file size acquiring and now can be updated date and time acquisition)
- External IRX and file read abolition
- in the CONFIG of MISC abolished the "MASS FILESIZE CHECK"
When a font file is in the mass or cdfs, Fixed it can not be loaded at startup

## 2006/12/22 LbF ver0.53

And change the display position of the version, mc, the display position of the free space on the hdd
When you start from - cd, Fixed can not read the cd a configuration file
- When you start from the cd, the configuration file, was not to save to mc
- In CONFIG, even when you select the file (folder), was to be able to display the file size and update date and time
- When the language is ENGLISH, of FileBrowser, change the message of R2
File when you get size, if there is no marked files, and do not want to see the message
- Add a "MASS FILESIZE CHECK" in the CONFIG of MISC
  (You can choose whether to display the file size of the mass in the FileBrowser)
And adjusting the operation of the CONFIG of the screen settings
- Fixed had the wrong configuration name Configuration file
    - (mis) aanji_margin_top (positive) kanji_margin_top
    - (mis) aanji_margin_left (positive) kanji_margin_left

## 2006/10/31 LbF ver0.52

- In FileBrowser, when you press R2 button, change to the size acquisition
- In FileBrowser, when you press L2 button, was to be able to display the file size and update date and time, but there are restrictions
    - Hdd0: / update date and time of, it is shifted 4 hours
    - Cdfs: / update date and time of, can not be obtained
    - Mass: / file size of, can not be acquired
    - Mass: / update date and time of, can not be acquired seconds

## 2006/09/22 LbF ver0.51

- The ps2sdk was returned to the revision 1344
- In the Launcher screen character was made to scroll when not be displayed
Press-screen setting at L3, and to select a color scheme presets
Message was partially changed
- The CONFIG was to be able to change other than the SELECT button
Except - mc when you psu export, was not to reduce the file name
Soft keyboard Fixed did not have the effect of flicker control to
- Hdd when you start from, Fixed no longer able to save the settings
File name was not allowed to be copied the file when too long
  (Mc is mass until 128 characters Up to 32 characters hdd until 256 characters)

## 2006/09/11 LbF ver0.50

- Fixed can not be straight boot

## 2006/09/10 LbF ver0.49

- Add the INFO to MISC out when you press the left and right keys in the launcher screen
- Ftp Fixed can not connect
A-built-in ps2ip.irx, it was returned to those of LbFv0.46
- built-in cdvd.irx, ps2ftpd.irx, the usb_mass.irx, was a thing of the uLaunchELFv4.01

## 2006/09/04 LbF ver0.48

- set when you boot from a mass file reading and writing failure correct the lies of that
- interval of the line is too small and Fixed freeze
And change some of the Japanese message

## 2006/09/03 LbF ver0.47

- Fixed could not be set to read when you start from the host
- CNF have changed the format of the file
- it was to be able to use an external font file
- External USB_MASS.IRX was to be able to use the
Also from - mc of SYS-CONF folder, was to be able to read the external IRX file

## 2006/07/29 LbF ver0.46

- Fixed can not start from non-host
- In the CONFIG of MISC SETTING INIT, Fixed language does not return to the ENGLSH
- a string of selected in the CONFIG was to be displayed in emphasis of color
When you press the left and right keys in the launcher screen, it was to be able to perform the functions of the MISC
Main screen change the display of the frame of the

## 2006/07/27 LbF ver0.45

- Paste, when such as Delete, was to re-calculation of free space
And change some of the Japanese message
- Import When you select a destination in the psu, the cursor to the CANCEL in the SELECT was to move
Soft keyboard adjustment
When you start, from the host, it was not allowed to reset the IOP
- Host when you start from, was not allowed to read or write the configuration file located in the host

## 2006/07/16 LbF ver0.44

- The language was to be able to select
- Game title double-byte alphanumeric characters and double-byte symbol, was to be displayed in single-byte
- IPCONFIG.DAT time of storage, mc0: / when there is no SYS-CONF folder was to be created
A - Import psu, was to be able to be from outside the mc
- Import in psu, it was to select a destination from mc0 or mc1
A - Export psu, and so can not only from the mc
- Export in psu, was to be able to specify the export folder

## 2006/06/15 LbF ver0.43

Change or the name of the configuration file to LBF.CNF
- When you mark a file in FileBrowser, was returned to display *
- When there is a BUTTON SETTING of the DEFAULT setting, was to stop the countdown and press any key

## 2006/06/06 LbF ver0.42

- The USBD.IRX in the same location as the executable file was set to use with priority (MC during start-up only)

## 2006/05/24 LbF ver0.41

File size Fixed can not copy a zero-byte file
- BUTTON SETTING of Fixed not to save the settings of the DEFAULT
- FLICKER CONTROL adjustment of

## 2006/05/23 LbF ver0.40

- CNF has changed the format of the file, it does not have a CNF file compatible with up to v0.30
File type was to be able to display an icon
- IPCONFIG.DAT was to allow the editing of
- In FileBrowser, when you see CD / DVD drive, and can be selected so that it does not examine the files of type
- In FileBrowser, when you select a file of the CD / DVD drive, was to check whether the ELF file
And operation of when you enable FLICKER CONTROL has become a little faster

## 2006/04/28 LbF ver0.30

From - uLaunchELF v3.60, it was transplanted the functions of the ftp server
- Use the BIOS of the font was to display the character
When - OK and Motomeraru the input of CANCEL, OK with START, the cursor to the CANCEL in the SELECT was to move
- Re-adjustment of the whole of the display position was

## 2006/04/12 LbF ver0.20

File name in the BOOT.ELF and straight boot, Fixed can not read and write files to the USB storage
- The launcher function revived
- In FileBrowser, it was to display the cursor
- In FileBrowser, reduced the number of files displayed on one screen
And change the default color scheme
And change the settings screen
Options additions to flicker (operation becomes heavier)

## 2006/02/06 LbF ver0.10

- Initial release