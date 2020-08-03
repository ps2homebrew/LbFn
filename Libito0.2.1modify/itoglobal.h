// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ito - free library for PlayStation 2 by Jules - www.mouthshut.net
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// global vars used within ito

// packet
#ifndef _ITOGLOBAL_H
#define _ITOGLOBAL_H

#include <ito.h>

extern uint64 *itoActivePacket align(128);
extern uint64 *itoActivePacketStart align(128);

//-----------------------------------------------------------------
// Framebuffer 1 is visible & active at init
//-----------------------------------------------------------------

extern uint8	itoActiveFrameBuffer;
extern uint8	itoVisibleFrameBuffer;

struct
{
	struct
	{
	uint16	width; // vck width = (width-1)*mag_x 
	uint16	height; // -1, unless its passed as framebuffer width
	uint16	x;
	uint16	y;
	uint16	dwidth;
	uint16	psm;
	uint8	mag_x;
	uint8	mag_y;
	uint16  offset_x;
	uint16  offset_y;
	} screen;

	struct
	{
	uint8	fog;
	uint8	alpha;
	uint8	antialiasing;
	uint8	shade;
	uint8	texture_coords;
	uint16  offset_x;
	uint16  offset_y;
	} prim;

	struct
	{
	uint8	psm;
	uint32	base;
	uint8	test;
	uint8	test_method;
	}zbuffer;

	struct
	{
	uint64 test; 
	} regs;

	struct
	{
	uint32 base;
	} texturebuffer;

	struct
	{
	uint8	tcc;
	uint8	tfx;
	uint32	src;
	uint32	tbw;
	uint8	psm;
	uint16	w;
	uint16	h;
	uint32	cbp;
	uint8	cpsm;
	uint16	csa;
	} texture;

	//-----------------------------------------------------------------
	// stores the visible frame buffer pointer (fbp
	//-----------------------------------------------------------------
	uint32  fbp[2];

} ito;


#endif
