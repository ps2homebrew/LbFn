/* from: svn.ps2dev.org/ps2/trunk/ps2sdk/ee/libgs/src/libgs.c (rev.1589, 2009/05/26 06:08:44)
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include "libgs.h"

/*static*/
static short gs_dma_send(unsigned int *addr, unsigned int qwords);
static short gs_dma_wait(void);
static void  gs_flush_cache(int mode);

QWORD		prim_work[64] __attribute__((aligned(16))); /*aligne to 128bits*/
static char temp[16] __attribute__((aligned(16)));

/*-------------------------------------------
-											-
- LOW LEVEL FUNTIONS						-           
-											-
-------------------------------------------*/


short GsTextureFlush(void)
{

	gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,0,1,0x0e);
	gs_setR_TEXFLUSH(((GS_R_TEXFLUSH *)&prim_work[1]));

	gs_flush_cache(0);
	gs_dma_send((unsigned int *)&prim_work[0],1+1);
	gs_dma_wait();
	

	return 0;
}



/*-------------------------------------------
-											-
- 											-
-											-
-------------------------------------------*/

short GsLoadImage(void *source_addr, GS_IMAGE *dest)
{
	int i;
//	static int	current,max,remainder,img_qwc;
	int	current,max,remainder,img_qwc,xx;
	

	switch(dest->pix_mode)
	{
	case 0:		//32 bit image
		img_qwc = (dest->width * dest->height)*4;
		remainder = 8;
	break;
	case 1:		//24 bit image
		img_qwc = (dest->width * dest->height)*3;
		remainder = 6;
	break;
	case 2:		//16 bit image
		img_qwc = (dest->width * dest->height)*2;
		remainder = 4;
	break;
	case 19:	//8 bit image
		img_qwc = (dest->width * dest->height)*1;
		remainder = 2;
	break;
	case 20:	//4 bit image
		img_qwc = (dest->width * dest->height)/2;
		remainder = 1;
	break;
	default:
		//printf("unable to load unsupported image(%02x)",dest->pix_mode);
	return 0;
	}

	//flush buffer to be safe
	GsTextureFlush();



	xx = 0;
	if (((int)source_addr & 15) && (dest->height == 1)) {
		current = 16 - ((int)source_addr & 15);
		if (current > img_qwc) current = img_qwc;
		for(i=0;i<current;i++) temp[i]=((char*)source_addr)[i];
		img_qwc -= current;

		gs_setGIF_TAG(((GS_GIF_TAG				*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
		gs_setR_BITBLTBUF(((GS_R_BITBLTBUF		*)&prim_work[1]),0,0,0,dest->vram_addr,dest->vram_width,dest->pix_mode);
		gs_setR_TRXPOS(((GS_R_TRXPOS			*)&prim_work[2]), 0,0,dest->x,dest->y,0);
		gs_setR_TRXREG(((GS_R_TRXREG			*)&prim_work[3]), current*2/remainder ,dest->height);
		gs_setR_TRXDIR(((GS_R_TRXDIR			*)&prim_work[4]), 0);
		
		gs_flush_cache(0);
		gs_dma_send((unsigned int *)&prim_work[0],4+1);
		gs_dma_wait();



		// we signal are about to send whats left
		gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), 1,1,0,0,0,2,0,0x00);

		gs_flush_cache(0);
		gs_dma_send((unsigned int *)&prim_work[0],0+1);
		gs_dma_wait();
	
		//send data leftover
		gs_dma_send((unsigned int *)&temp[0],1);

		//dont wait
		gs_dma_wait();

		(unsigned char *)source_addr += current;
		xx = current*2/remainder;
	}



	gs_setGIF_TAG(((GS_GIF_TAG				*)&prim_work[0]), 4,1,0,0,0,0,1,0x0e);
	gs_setR_BITBLTBUF(((GS_R_BITBLTBUF		*)&prim_work[1]),0,0,0,dest->vram_addr,dest->vram_width,dest->pix_mode);
	gs_setR_TRXPOS(((GS_R_TRXPOS			*)&prim_work[2]), 0,0,dest->x+xx,dest->y,0);
	gs_setR_TRXREG(((GS_R_TRXREG			*)&prim_work[3]), dest->width-xx,dest->height);
	gs_setR_TRXDIR(((GS_R_TRXDIR			*)&prim_work[4]), 0);
	
	gs_flush_cache(0);
	gs_dma_send((unsigned int *)&prim_work[0],4+1);
	gs_dma_wait();



	// Ok , We Send Image Now
	img_qwc = (img_qwc +15) /16;
	max			= img_qwc / 16384;
	remainder	= img_qwc % 16384;
	current		= 16384;
	for(i=0;i<max;i++)
	{

		//1st we signal gs we are about to send
		//16384 qwords
	
		gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), current,1,0,0,0,2,0,0x00);

		gs_flush_cache(0);
		gs_dma_send((unsigned int *)&prim_work[0],0+1);
		gs_dma_wait();


		//we now send 16384 more qwords
		gs_dma_send((unsigned int *)source_addr,current);
		gs_dma_wait();


		(unsigned char *)source_addr += current*16;
	}



	//transfer the rest if we have left overs
	current = remainder;
	//or exit if none is left
	if(current)
	{

		// we signal are about to send whats left
		gs_setGIF_TAG(((GS_GIF_TAG *)&prim_work[0]), current,1,0,0,0,2,0,0x00);

		gs_flush_cache(0);
		gs_dma_send((unsigned int *)&prim_work[0],0+1);
		gs_dma_wait();
	
		//send data leftover
		gs_dma_send((unsigned int *)source_addr,current);

		//dont wait
		gs_dma_wait();
	}


	//do a final flush
	GsTextureFlush();

	return 1;
}


/******************************************************************
* STATIC(PRIVATE) MISC
*
*
******************************************************************/
#define gif_chcr	0x1000a000	// GIF Channel Control Register
#define gif_madr	0x1000a010	// Transfer Address Register
#define gif_qwc		0x1000a020	// Transfer Size Register (in qwords)
#define gif_tadr	0x1000a030	// ...


 #define DMA_TAG_REFE	0x00
 #define DMA_TAG_CNT	0x01
 #define DMA_TAG_NEXT	0x02
 #define DMA_TAG_REF	0x03
 #define DMA_TAG_REFS	0x04
 #define DMA_TAG_CALL	0x05
 #define DMA_TAG_RET	0x06
 #define DMA_TAG_END	0x07




typedef struct {
	unsigned direction	:1;	// Direction
	unsigned pad1		:1; // Pad with zeros
	unsigned mode		:2;	// Mode
	unsigned asp		:2;	// Address stack pointer
	unsigned tte		:1;	// Tag trasfer enable
	unsigned tie		:1;	// Tag interrupt enable
	unsigned start_flag	:1;	// start
	unsigned pad2		:7; // Pad with more zeros
	unsigned tag		:16;// DMAtag
}DMA_CHCR;



static short gs_dma_send(unsigned int *addr, unsigned int qwords)
{
	DMA_CHCR		chcr;
	static char		spr;

	if(addr >= (unsigned int *)0x70000000 && addr <= (unsigned int *)0x70003fff)
	{
		spr = 1;
	}
	else
	{
		spr = 0;
	}


	*((volatile unsigned int *)(gif_madr)) = ( unsigned int )((( unsigned int )addr) & 0x7FFFFFFF) << 0 | (unsigned int)((spr) & 0x00000001) << 31;;

	*((volatile unsigned int *)(gif_qwc)) = qwords;






	chcr.direction	=1;
	chcr.mode		=0;
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;
	*((volatile unsigned int *)(gif_chcr)) = *(unsigned int *)&chcr;


	return 0;
}


static short gs_dma_wait(void)
{
	while(*((volatile unsigned int *)(0x1000a000)) & ((unsigned int)1<<8));

	return 0;
}

static void gs_flush_cache(int mode)
{
	__asm__(
		
			"li	$3,100	\n"
			"syscall	\n"
			"jr	$31		\n"
			"nop		\n"
			);

}


/*EOF*/
