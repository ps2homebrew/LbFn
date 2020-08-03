#------------------------------------
LIBITO=$(PS2DEV)\libito
PS2ETH=$(PS2DEV)\ps2eth
PS2_IP=192.168.0.10

#------------------------------------
#psbファイル実行機能の有無
#PSB = yes

#アイコンの有無
#ICON = yes

#TEK = yes
#------------------------------------
EE_BIN = LbFn.ELF

EE_OBJS = main.o pad.o config.o elf.o draw.o loader.o  filer.o cd.o language.o\
	cnf.o tek.o viewer.o shiftjis.o bmp.o jpeg.o gif.o ps2ico.o fmcb_cfg.o misc.o\
	poweroff.o iomanX.o fileXio.o ps2atad.o ps2dev9.o ps2hdd.o ps2fs.o\
	usbd.o usbhdfsd.o cdvd.o ps2ip.o ps2smap.o ps2ftpd.o dns.o ps2http.o fakehost.o
#	vmc_fs.o

EE_INCS := -I$(LIBITO)/include -I$(PS2DEV)/libcdvd/ee

EE_LDFLAGS := -L$(LIBITO)/lib -L$(PS2DEV)/libcdvd/lib -s

EE_LIBS = -lpad -lito -lmc -lhdd -lcdvd -lcdvdfs -lfileXio -lpatches -lpoweroff -lkbd -lmouse -ldebug\

# EE_CFLAGS := -O3

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
	$(MAKE) -C loader clean
	rm -f $(EE_BIN) *.o *.s

test: all
	ps2client -h $(PS2_IP) -t 1 execee host:$(EE_BIN)

reset: clean
	ps2client -h $(PS2_IP) reset


#------------------------------------
ifeq ($(TEK), yes)

#cdvd.irx ulaunchELF 4.01
#ps2ftpd.irx uLaunchELF 4.01
#loader.elf uLaunchELF 4.12

else

usbd.s:
	bin2s $(PS2SDK)/iop/usb/usbd/bin/usbd.irx usbd.s usbd_irx

ps2smap.s:
	bin2s $(PS2ETH)/smap/ps2smap.irx ps2smap.s ps2smap_irx

loader/loader.elf: loader/loader.c
	$(MAKE) -C loader

loader.s: loader/loader.elf
	bin2s loader/loader.elf loader.s loader_elf

%.s : modules/%.irx
	bin2s modules/$*.irx $*.s $*_irx

%.s : $(PS2SDK)/iop/irx/%.irx
	bin2s $(PS2SDK)/iop/irx/$*.irx $*.s $*_irx

endif

icon.s:image/icon.iif
	bin2s image/icon.iif icon.s icon_iif

cd.o config.o draw.o elf.o filer.o main.o pad.o viewer.o language.o cnf.o misc.o fmcb_cfg.o:launchelf.h language.h

#------------------------------------
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
