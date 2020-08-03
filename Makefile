#------------------------------------
LIBITO=$(PS2DEV)\libito
PS2ETH=$(PS2DEV)\ps2eth


#------------------------------------
#psbファイル実行機能の有無
#PSB = yes
#アイコンの有無
#ICON = yes


#------------------------------------
EE_BIN = LbF.ELF

EE_OBJS = main.o pad.o config.o elf.o draw.o loader.o  filer.o cd.o language.o cnf.o\
	poweroff.o iomanx.o filexio.o ps2atad.o ps2dev9.o ps2hdd.o ps2fs.o\
	usbd.o usbhdfsd.o cdvd.o ps2ip.o ps2smap.o ps2ftpd.o fakehost.o

EE_INCS := -I$(LIBITO)/include -I$(PS2DEV)/libcdvd/ee

EE_LDFLAGS := -L$(LIBITO)/lib -L$(PS2DEV)/libcdvd/lib -s

EE_LIBS = -lpad -lito -lmc -lhdd -lcdvd -lcdvdfs -lfileXio -lpatches -lpoweroff -ldebug\


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


#------------------------------------
usbd.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx usbd.s usbd_irx

usbhdfsd.s:
	bin2s $(PS2DEV)/usbhdfsd/bin/usbhdfsd.irx usbhdfsd.s usbhdfsd_irx

#cdvd.irx ulaunchELF 4.01
cdvd.s:
	bin2s modules/cdvd.irx cdvd.s cdvd_irx

poweroff.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx poweroff.s poweroff_irx

iomanx.s:
	bin2s $(PS2SDK)/iop/irx/iomanX.irx iomanx.s iomanx_irx

filexio.s:
	bin2s $(PS2SDK)/iop/irx/fileXio.irx filexio.s filexio_irx

ps2dev9.s:
	bin2s $(PS2SDK)/iop/irx/ps2dev9.irx ps2dev9.s ps2dev9_irx

ps2atad.s:
	bin2s $(PS2SDK)/iop/irx/ps2atad.irx ps2atad.s ps2atad_irx

ps2hdd.s:
	bin2s $(PS2SDK)/iop/irx/ps2hdd.irx ps2hdd.s ps2hdd_irx

ps2fs.s:
	bin2s $(PS2SDK)/iop/irx/ps2fs.irx ps2fs.s ps2fs_irx

fakehost.s:
	bin2s $(PS2SDK)/iop/irx/fakehost.irx fakehost.s fakehost_irx

ps2smap.s:
	bin2s $(PS2ETH)/smap/ps2smap.irx ps2smap.s ps2smap_irx

ps2ip.s:
	bin2s $(PS2SDK)/iop/irx/ps2ip.irx ps2ip.s ps2ip_irx

#ps2ftpd.irx uLaunchELF 4.01
ps2ftpd.s:
	bin2s modules/ps2ftpd.irx ps2ftpd.s ps2ftpd_irx

#loader.elf uLaunchELF 4.12
loader.s:
	bin2s loader/loader.elf loader.s loader_elf

icon.s:image/icon.iif
	bin2s image/icon.iif icon.s icon_iif

cd.o config.o draw.o elf.o filer.o main.o pad.o language.o cnf.o:launchelf.h language.h cnf.h


#------------------------------------
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
