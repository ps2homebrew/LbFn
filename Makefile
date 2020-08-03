#------------------------------------
LIBITO=$(PS2DEV)\libito
PS2ETH=$(PS2DEV)\ps2eth
PS2_IP=192.168.0.10

#------------------------------------
#psbファイル実行機能の有無
#PSB = yes

#アイコンの有無
#ICON = yes


#------------------------------------
EE_BIN = LbFn.ELF

EE_OBJS = main.o pad.o config.o elf.o draw.o loader.o  filer.o cd.o language.o\
	cnf.o tek.o viewer.o shiftjis.o bmp.o jpeg.o gif.o\
	poweroff.o iomanx.o filexio.o ps2atad.o ps2dev9.o ps2hdd.o ps2fs.o\
	usbd.o usbhdfsd.o cdvd.o ps2ip.o ps2smap.o ps2ftpd.o fakehost.o

EE_INCS := -I$(LIBITO)/include -I$(PS2DEV)/libcdvd/ee

EE_LDFLAGS := -L$(LIBITO)/lib -L$(PS2DEV)/libcdvd/lib -s

EE_LIBS = -lpad -lito -lmc -lhdd -lcdvd -lcdvdfs -lfileXio -lpatches -lpoweroff -lkbd -lmouse -ldebug\


ifeq ($(PSB), yes)
EE_CFLAGS += -DENABLE_PSB
EE_CXXFLAGS += -DENABLE_PSB
endif

ifeq ($(ICON), yes)
EE_OBJS += icon.o
EE_CFLAGS += -DENABLE_ICON
EE_CXXFLAGS += -DENABLE_ICON
endif


#------------------------------------
all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) *.o *.s

test: all
	ps2client -h $(PS2_IP) -t 1 execee host:$(EE_BIN)

reset: clean
	ps2client -h $(PS2_IP) reset


#------------------------------------
usbd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/usb/usbd/bin/usbd.irx out:usbd.ir5
	bin2s usbd.ir5 usbd.s usbd_irx

usbhdfsd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2DEV)/usbhdfsd/bin/usbhdfsd.irx out:usbhdfsd.ir5
	bin2s usbhdfsd.ir5 usbhdfsd.s usbhdfsd_irx

#cdvd.irx ulaunchELF 4.01
cdvd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:modules/cdvd.irx out:cdvd.ir5
	bin2s cdvd.ir5 cdvd.s cdvd_irx

poweroff.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/poweroff.irx out:poweroff.ir5
	bin2s poweroff.ir5 poweroff.s poweroff_irx

iomanx.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/iomanx.irx out:iomanx.ir5
	bin2s iomanX.ir5 iomanx.s iomanx_irx

filexio.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/filexio.irx out:filexio.ir5
	bin2s fileXio.ir5 filexio.s filexio_irx

ps2dev9.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/ps2dev9.irx out:ps2dev9.ir5
	bin2s ps2dev9.ir5 ps2dev9.s ps2dev9_irx

ps2atad.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/ps2atad.irx out:ps2atad.ir5
	bin2s ps2atad.ir5 ps2atad.s ps2atad_irx

ps2hdd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/ps2hdd.irx out:ps2hdd.ir5
	bin2s ps2hdd.ir5 ps2hdd.s ps2hdd_irx

ps2fs.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/ps2fs.irx out:ps2fs.ir5
	bin2s ps2fs.ir5 ps2fs.s ps2fs_irx

fakehost.s:
	bin2s $(PS2SDK)/iop/irx/fakehost.irx fakehost.s fakehost_irx

ps2smap.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2ETH)/smap/ps2smap.irx out:ps2smap.ir5
	bin2s ps2smap.ir5 ps2smap.s ps2smap_irx

ps2ip.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:$(PS2SDK)/iop/irx/ps2ip.irx out:ps2ip.ir5
	bin2s ps2ip.ir5 ps2ip.s ps2ip_irx

#ps2ftpd.ir5 uLaunchELF 4.01
ps2ftpd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:modules/ps2ftpd.irx out:ps2ftpd.ir5
	bin2s ps2ftpd.ir5 ps2ftpd.s ps2ftpd_irx

#loader.elf uLaunchELF 4.12
loader.s:
	bin2s loader/loader.elf loader.s loader_elf

ps2kbd.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:irx/ps2kbd.irx out:irx/ps2kbd.ir5
	bin2s irx/ps2kbd.ir5 ps2kbd.s ps2kbd_irx

ps2mouse.s:
	bim2bin -osacmp -tek5 clv:5 eopt:@ eprm:@ in:irx/ps2mouse.irx out:irx/ps2mouse.ir5
	bin2s irx/ps2mouse.ir5 ps2mouse.s ps2mouse_irx

icon.s:image/icon.iif
	bin2s image/icon.iif icon.s icon_iif

cd.o config.o draw.o elf.o filer.o main.o pad.o viewer.o language.o cnf.o:launchelf.h language.h


#------------------------------------
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
