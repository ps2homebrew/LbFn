#------------------------------------
LIBITO=$(PS2DEV)\libito
PS2ETH=$(PS2DEV)\ps2eth

#EE_BIN = LaunchELF.ELF
EE_BIN = LbF.ELF

EE_OBJS = main.o pad.o config.o elf.o draw.o loader.o  filer.o mass_rpc.o cd.o language.o cnf.o\
	poweroff.o iomanx.o filexio.o ps2atad.o ps2dev9.o ps2hdd.o ps2fs.o ps2netfs.o\
	usbd.o usb_mass.o cdvd.o ps2ip.o ps2smap.o ps2ftpd.o

EE_INCS := -I$(LIBITO)/include -I$(PS2DK)/sbv/include\
	-I$(PS2DEV)/libcdvd/ee

EE_LDFLAGS := -L$(LIBITO)/lib -L$(PS2SDK)/sbv/lib\
	-L$(PS2DEV)/libcdvd/lib -s

EE_LIBS = -lpad -lito -lmc -lhdd -lcdvdfs -lfileXio -lpatches -lpoweroff

#------------------------------------
all: $(EE_BIN)

usbd.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx usbd.s usbd_irx

#usb_mass.irx ulaunchELF 4.01
usb_mass.s:
	bin2s modules/usb_mass.irx usb_mass.s usb_mass_irx

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

ps2netfs.s:
	bin2s $(PS2SDK)/iop/irx/ps2netfs.irx ps2netfs.s ps2netfs_irx

loader.s:
	bin2s loader/loader.elf loader.s loader_elf

ps2smap.s:
	bin2s $(PS2ETH)/smap/ps2smap.irx ps2smap.s ps2smap_irx

ps2ip.s:
	bin2s $(PS2SDK)/iop/irx/ps2ip.irx ps2ip.s ps2ip_irx
#	bin2s modules/ps2ip.irx ps2ip.s ps2ip_irx

#ps2ftpd.irx uLaunchELF 4.01
ps2ftpd.s:
	bin2s modules/ps2ftpd.irx ps2ftpd.s ps2ftpd_irx

#------------------------------------
clean:
	rm -f $(EE_BIN) *.o *.s

#------------------------------------
cd.o config.o draw.o elf.o filer.o main.o pad.o language.o cnf.o:launchelf.h language.h cnf.h

#------------------------------------
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
