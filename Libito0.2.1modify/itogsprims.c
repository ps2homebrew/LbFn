// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// ito - free library for PlayStation 2 by Jules - www.mouthshut.net
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#include <ito.h>
#include <itogs.h>
#include <itoglobal.h>


//-----------------------------------------------------------------
//-----------------------------------------------------------------
// GS Prims
//-----------------------------------------------------------------
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// GifTag pointer used for LineStrip, TriangleTrip and Trianglefan,
// since they size is unknown, the QWC in the GifTag will be added
// in the end and therefor requires a pointer.
//-----------------------------------------------------------------


uint64 * ITO_GIFTAG_PTR;

uint8	itoActiveFrameBuffer;
uint8	itoVisibleFrameBuffer;


//-----------------------------------------------------------------
// Sprite
//-----------------------------------------------------------------

void itoSprite(uint64 color, uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint32 z)
{	
		
	itoGifTag(4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_SPRITE	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------		
	// Sprites do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------

	itoRGBAQ(color);
	//-----------------------------------------------------------------
	// top-left choords
	//-----------------------------------------------------------------

	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z);
	
	//-----------------------------------------------------------------
	// bottom-right choords
	//-----------------------------------------------------------------
	
	itoXYZ2((ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z);
	//-----------------------------------------------------------------
	// Check if the GS packet has reached its maximum size
	//-----------------------------------------------------------------
	
	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Textured Sprite float
// - All special float versions of prims is for STQ texture
// mapping so you can past values like 1.0f etc.
//-----------------------------------------------------------------

void itoTextureSpritef(uint64 color, uint16 x1, uint16 y1, float tx1, float ty1, uint16 x2, uint16 y2, float tx2, float ty2, uint32 z)
{
	/*itoTextureSprite(color, x1, y1, fmti(tx1), fmti(ty1), x2, y2, fmti(tx2), fmti(ty2), z);*/
	
	itoGifTag( 6, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_SPRITE	, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// top-left
	//-----------------------------------------------------------------
	itoRGBAQ( color);
	
	itoST( fmti(tx1) , fmti(ty1));
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z);
	
	//-----------------------------------------------------------------
	// bottom-right
	//-----------------------------------------------------------------

	itoST( fmti(tx2), fmti(ty2));
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z);

	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Textured Sprite
//-----------------------------------------------------------------

void itoTextureSprite(uint64 color, uint16 x1, uint16 y1, uint32 tx1, uint32 ty1, uint16 x2, uint16 y2, uint32 tx2, uint32 ty2, uint32 z)
{	
	itoGifTag( 6, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_SPRITE	, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// top-left
	//-----------------------------------------------------------------
	itoRGBAQ( color);
	
	itoUV( tx1 << 4 , ty1 << 4);
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z);
	
	//-----------------------------------------------------------------
	// bottom-right
	//-----------------------------------------------------------------

	itoUV( tx2 << 4, ty2 << 4);
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z);

	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Triangle
//-----------------------------------------------------------------

void itoTriangle(	uint64 color1, uint16 x1, uint16 y1, uint32	z1,
					uint64 color2, uint16 x2, uint16 y2, uint32 z2,
					uint64 color3, uint16 x3, uint16 y3, uint32 z3)
{	
	//-----------------------------------------------------------------
	// Gouraud
	//-----------------------------------------------------------------
	if(ito.prim.shade == ITO_PRIM_SHADE_GOURAUD)
	{
		itoGifTag( 7, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE, ITO_PRIM_SHADE_GOURAUD, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
		
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
	
	//-----------------------------------------------------------------
	// Flat
	//-----------------------------------------------------------------
	else
	{
		itoGifTag( 5, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
		
		itoRGBAQ( color1 );
		
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
	
	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Textured Triangle float
//-----------------------------------------------------------------

void itoTextureTrianglef(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, float tx1, float ty1,
							uint64 color2, uint16 x2, uint16 y2, uint32 z2, float tx2, float ty2,
							uint64 color3, uint16 x3, uint16 y3, uint32 z3, float tx3, float ty3)
{
	/*
	itoTextureTriangle(	color1, x1, y1, z1, fmti(tx1), fmti(ty1),
						color2, x2, y2, z2, fmti(tx2), fmti(ty2),
						color3, x3, y3, z3, fmti(tx3), fmti(ty3));
	*/
	
	itoGifTag( 10, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_TRIANGLE, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoST( fmti(tx1), fmti(ty1) );
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
	itoST( fmti(tx2), fmti(ty2) );
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	itoST( fmti(tx3), fmti(ty3) );
	itoRGBAQ( color3 );
	itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);

	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Textured Triangle
//-----------------------------------------------------------------

void itoTextureTriangle(uint64 color1, uint16 x1, uint16 y1, uint32 z1, uint32 tx1, uint32 ty1,
						uint64 color2, uint16 x2, uint16 y2, uint32 z2, uint32 tx2, uint32 ty2,
						uint64 color3, uint16 x3, uint16 y3, uint32 z3, uint32 tx3, uint32 ty3)
{	
	//-----------------------------------------------------------------
	// FLAT/UV coords does not require more then 1 RGBAQ value 
	//-----------------------------------------------------------------

	if(ito.prim.shade == ITO_PRIM_SHADE_FLAT)
	{
		itoGifTag( 8, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2 << 4, ty2 << 4 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3 << 4, ty3 << 4);
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
	else
	{	
		itoGifTag( 10, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4);
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2 << 4, ty2 << 4);
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3 << 4, ty3 << 4);
		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
		}
	
	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Line Strip
//-----------------------------------------------------------------

void itoLineStrip(	uint64 color1, uint16 x1, uint16 y1, uint32 z1,
					uint64 color2, uint16 x2, uint16 y2, uint32 z2 )
{
		ITO_GIFTAG_PTR = &itoActivePacket[0];

		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_LINE_STRIP, ito.prim.shade, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		if(ito.prim.shade == ITO_PRIM_SHADE_GOURAUD)
			itoRGBAQ( color2 );
		
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);

}

//-----------------------------------------------------------------
// Triangle Strip
//-----------------------------------------------------------------
void itoTriangleStrip(	uint64 color1, uint16 x1, uint16 y1, uint32 z1,
						uint64 color2, uint16 x2, uint16 y2, uint32 z2,
						uint64 color3, uint16 x3, uint16 y3, uint32 z3)
{
		ITO_GIFTAG_PTR = &itoActivePacket[0];

		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_STRIP, ito.prim.shade, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);

		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);

}

//-----------------------------------------------------------------
// Triangle Fan
//-----------------------------------------------------------------

void itoTriangleFan(	uint64 color1, uint16 x1, uint16 y1, uint32 z1,
						uint64 color2, uint16 x2, uint16 y2, uint32 z2,
						uint64 color3, uint16 x3, uint16 y3, uint32 z3)
{
		ITO_GIFTAG_PTR = &itoActivePacket[0];

		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_FAN, ito.prim.shade, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);

		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);

}
//-----------------------------------------------------------------
// Textured Triangle Fan float
//-----------------------------------------------------------------

void itoTextureTriangleFanf(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, float tx1, float ty1,
								uint64 color2, uint16 x2, uint16 y2, uint32 z2, float tx2, float ty2,
								uint64 color3, uint16 x3, uint16 y3, uint32 z3, float tx3, float ty3)
{
	ITO_GIFTAG_PTR = &itoActivePacket[0];

	itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_TRIANGLE_FAN, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoST( fmti(tx1), fmti(ty1) );
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
	itoST( fmti(tx2), fmti(ty2) );
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	itoST( fmti(tx3), fmti(ty3) );
	itoRGBAQ( color3 );
	itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);


}

//-----------------------------------------------------------------
// Textured Triangle Fan
//-----------------------------------------------------------------
void itoTextureTriangleFan(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, uint32 tx1, uint32 ty1,
							uint64 color2, uint16 x2, uint16 y2, uint32 z2, uint32 tx2, uint32 ty2,
							uint64 color3, uint16 x3, uint16 y3, uint32 z3, uint32 tx3, uint32 ty3)
{	
	ITO_GIFTAG_PTR = &itoActivePacket[0];

	//-----------------------------------------------------------------
	// FLAT/UV coords does not require more then 1 RGBAQ value 
	//-----------------------------------------------------------------
	if( (ito.prim.shade == ITO_PRIM_SHADE_FLAT))
	{
		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_FAN, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2 << 4, ty2 << 4 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3 << 4, ty3 << 4);
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
	else
	{ 
		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_FAN, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2 << 4, ty2 << 4 );
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3 << 4, ty3 << 4 );
		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
}

//-----------------------------------------------------------------
// Textured Triangle Strip Float
//-----------------------------------------------------------------
void itoTextureTriangleStripf(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, float tx1, float ty1,
								uint64 color2, uint16 x2, uint16 y2, uint32 z2, float tx2, float ty2,
								uint64 color3, uint16 x3, uint16 y3, uint32 z3, float tx3, float ty3)
{
	/*
	itoTextureTriangleStrip(	color1, x1, y1, z1, fmti(tx1), fmti(ty1),
								color2, x2, y2, z2, fmti(tx2), fmti(ty2),
								color3, x3, y3, z3, fmti(tx3), fmti(ty3));
	*/
	
	ITO_GIFTAG_PTR = &itoActivePacket[0];

	itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_TRIANGLE_STRIP, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoST( fmti(tx1), fmti(ty1) );
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
	itoST( fmti(tx2), fmti(ty2) );
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	itoST( fmti(tx3), fmti(ty3));
	itoRGBAQ( color3 );
	itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);


}

//-----------------------------------------------------------------
// Textured Triangle Strip
//-----------------------------------------------------------------
void itoTextureTriangleStrip(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, uint32 tx1, uint32 ty1,
								uint64 color2, uint16 x2, uint16 y2, uint32 z2, uint32 tx2, uint32 ty2,
								uint64 color3, uint16 x3, uint16 y3, uint32 z3, uint32 tx3, uint32 ty3)
{	
	ITO_GIFTAG_PTR = &itoActivePacket[0];
	
	//-----------------------------------------------------------------
	// FLAT/UV coords does not require more then 1 RGBAQ value 
	//-----------------------------------------------------------------
	
	if(ito.prim.shade == ITO_PRIM_SHADE_FLAT)
	{
		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_STRIP, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2 << 4, ty2 << 4 );
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3 << 4, ty3 << 4);
		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	}
	else
	{ 
		
		itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_TRIANGLE_STRIP, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1, ty1 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoUV( tx2, ty2 );
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
		itoUV( tx3, ty3 );
		itoRGBAQ( color3 );
		itoXYZ2( (ito.prim.offset_x + x3) << 4, (ito.prim.offset_y + y3) << 4, z3);
	} 
}

//-----------------------------------------------------------------
// Textured Linestrip float
//-----------------------------------------------------------------


void itoTextureLineStripf(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, float tx1, float ty1,
							uint64 color2, uint16 x2, uint16 y2, uint32 z2, float tx2, float ty2 )

{	
	ITO_GIFTAG_PTR = &itoActivePacket[0];
	
	itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_LINE_STRIP, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoST( fmti(tx1), fmti(ty1));
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
	itoST( fmti(tx2), fmti(ty2) );
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);

}


//-----------------------------------------------------------------
// Textured Line
//-----------------------------------------------------------------

void itoTextureLineStrip(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, uint32 tx1, uint32 ty1,
							uint64 color2, uint16 x2, uint16 y2, uint32 z2, uint32 tx2, uint32 ty2 )
{	
	ITO_GIFTAG_PTR = &itoActivePacket[0];

	itoGifTag( 0, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_LINE_STRIP, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
	itoUV( tx1 << 4, ty1 << 4 );
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
	itoUV( tx2 << 4, ty2 << 4 );
		
	//-----------------------------------------------------------------
	// FLAT/UV coords does not require more then 1 RGBAQ value 
	//-----------------------------------------------------------------
	if(ito.prim.shade == ITO_PRIM_SHADE_GOURAUD)
		itoRGBAQ( color2 );
		
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
}


//-----------------------------------------------------------------
// Used for trianglestrip/fan & linestrip for adding more vertexes
//-----------------------------------------------------------------
void itoAddVertex(uint64 color, uint16 x, uint16 y, uint32 z)
{	
	itoRGBAQ( color );
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);		
}

//-----------------------------------------------------------------
// Used for trianglestrip/fan & linestrip for adding more textured 
// vertexes
//-----------------------------------------------------------------
void itoAddTextureVertex(uint64 color, uint16 x, uint16 y, uint32 z, uint32 tx, uint32 ty)
{
	itoUV( tx << 4, ty  << 4);
	itoRGBAQ( color );
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);		
}

//-----------------------------------------------------------------
// Used for trianglestrip/fan & linestrip for adding more textured 
// vertexes using float for STQ texture mapping.
//-----------------------------------------------------------------
void itoAddTextureVertexf(uint64 color, uint16 x, uint16 y, uint32 z, float tx, float ty)
{	
	itoST( fmti(tx), fmti(ty));
	itoRGBAQ( color );
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);		
}


//-----------------------------------------------------------------
// When no more vertexes are to be added to trianglefan/trip or
// linestrip then this must be called so that the vertex
// information actually gets setup correctly.
//-----------------------------------------------------------------

void itoEndVertex()
{
	uint32 size;

	size = ((itoActivePacket - ITO_GIFTAG_PTR)/2)-1;
	*ITO_GIFTAG_PTR |= size; // apply size to giftag
	itoCheckGsPacket();
}

// -------------------------------------------------
// Line
// -------------------------------------------------

void itoLine(	uint64 color1, uint16 x1, uint16 y1, uint32 z1,
				uint64 color2, uint16 x2, uint16 y2, uint32 z2 )
{	
	//-----------------------------------------------------------------
	// Gouraud
	//-----------------------------------------------------------------
	if(ito.prim.shade == ITO_PRIM_SHADE_GOURAUD)
	{
		itoGifTag( 5, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_LINE, ITO_PRIM_SHADE_GOURAUD, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	}

	//-----------------------------------------------------------------
	// Flat
	//-----------------------------------------------------------------
	else
	{
		itoGifTag( 4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_LINE, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	}

	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Textured Line float
//-----------------------------------------------------------------
void itoTextureLinef(	uint64 color1, uint16 x1, uint16 y1, uint32 z1, float tx1, float ty1,
						uint64 color2, uint16 x2, uint16 y2, uint32 z2, float tx2, float ty2 )

{	
	/*
	itoTextureLine(	color1, x1, y1, z1, fmti(tx1), fmti(ty1),
					color2, x2, y2, z2, fmti(tx2), fmti(ty2));
	*/
	itoGifTag( 7, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_LINE, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoST( fmti(tx1), fmti(ty1) );
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	

	itoST( fmti(tx2), fmti(ty2) );
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	itoCheckGsPacket();

}

//-----------------------------------------------------------------
// Textured Line
//-----------------------------------------------------------------
void itoTextureLine(uint64 color1, uint16 x1, uint16 y1, uint32 z1, uint16 tx1, uint16 ty1,
					uint64 color2, uint16 x2, uint16 y2, uint32 z2, uint16 tx2, uint16 ty2 )
{	
	
	//-----------------------------------------------------------------
	// FLAT/UV coords does not require more then 1 RGBAQ value 
	//-----------------------------------------------------------------
	if( ito.prim.shade == ITO_PRIM_SHADE_FLAT )
	{
		itoGifTag( 6, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_LINE, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1 << 4 );
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	

		itoUV( tx2 << 4, ty2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	}
	else
	{	
		//-----------------------------------------------------------------
		// FLAT/STQ, GOURAUD/STQ & GOURAUD/UV all require 3 RGBAQ values.
		//-----------------------------------------------------------------
		
		itoGifTag( 7, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
		itoPRIM( ITO_PRIM_LINE, ito.prim.shade, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
		itoUV( tx1 << 4, ty1  << 4);
		itoRGBAQ( color1 );
		itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, z1);
	

		itoUV( tx2 << 4, ty2  << 4);
		itoRGBAQ( color2 );
		itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, z2);
	
	}


	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// Point
//-----------------------------------------------------------------

void itoPoint(uint64 color, uint16 x, uint16 y, uint32 z)
{	
		
	itoGifTag(3, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// Points do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------
	itoRGBAQ(color);

	//-----------------------------------------------------------------
	// Coord
	//-----------------------------------------------------------------
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);
	
	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// GS actually supports Textured Point. I can't think of a use for 
// it, but here it is anyway.
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Textured Point float
//-----------------------------------------------------------------
void itoTexturePointf(uint64 color, uint16 x, uint16 y, uint32 z, float tx, float ty)
{	
	/*
	itoTexturePoint(color, x, y, z, fmti(tx), fmti(ty));
	*/
	itoGifTag( 4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_STQ, itoActiveFrameBuffer, 0);
	
	itoRGBAQ( color );
	itoST( fmti(tx), fmti(ty) );
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);
	
	itoCheckGsPacket();

}

//-----------------------------------------------------------------
// Textured Point
//-----------------------------------------------------------------
void itoTexturePoint(uint64 color, uint16 x, uint16 y, uint32 z, uint16 tx, uint16 ty)
{	
	itoGifTag( 4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT, ITO_PRIM_SHADE_FLAT, TRUE, ito.prim.fog, ito.prim.alpha, FALSE, ITO_PRIM_TEXCOORDS_UV, itoActiveFrameBuffer, 0);
	
	itoRGBAQ( color );
	itoUV( tx << 4, ty << 4 );
	itoXYZ2( (ito.prim.offset_x + x) << 4, (ito.prim.offset_y + y) << 4, z);
	
	itoCheckGsPacket();
}

//-----------------------------------------------------------------
// added by nika
//-----------------------------------------------------------------

void itoPoint2(uint64 color, uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{	
		
	itoGifTag(4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// Points do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------
	itoRGBAQ(color);

	//-----------------------------------------------------------------
	// Coord
	//-----------------------------------------------------------------
	itoXYZ2( (ito.prim.offset_x + x0) << 4, (ito.prim.offset_y + y0) << 4, 0);
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, 0);
	
	itoCheckGsPacket();
}

void itoPoint2c(uint64 color1, uint16 x0, uint16 y0, uint64 color2, uint16 x1, uint16 y1)
{	
		
	itoGifTag(5, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	itoRGBAQ(color1);
	itoXYZ2( (ito.prim.offset_x + x0) << 4, (ito.prim.offset_y + y0) << 4, 0);
	itoRGBAQ(color2);
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, 0);
	
	itoCheckGsPacket();
}

void itoLineX(uint64 color, uint16 x1, uint16 y1, uint16 x2, uint16 y2)
{	
	//-----------------------------------------------------------------
	// Flat
	//-----------------------------------------------------------------
	itoGifTag( 4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	
	itoPRIM( ITO_PRIM_LINE, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	itoRGBAQ( color );
	itoXYZ2( (ito.prim.offset_x << 4) + x1, (ito.prim.offset_y << 4) + y1, 0);
	itoXYZ2( (ito.prim.offset_x << 4) + x2, (ito.prim.offset_y << 4) + y2, 0);

	itoCheckGsPacket();
}

void itoPointX(uint64 color, uint16 x, uint16 y)
{	
		
	itoGifTag(3, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// Points do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------
	itoRGBAQ(color);

	//-----------------------------------------------------------------
	// Coord
	//-----------------------------------------------------------------
	itoXYZ2( (ito.prim.offset_x << 4) + x, (ito.prim.offset_y << 4) + y, 0);
	
	itoCheckGsPacket();
}

void itoPoint2X(uint64 color, uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{	
		
	itoGifTag(4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_POINT	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------
	// Points do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------
	itoRGBAQ(color);

	//-----------------------------------------------------------------
	// Coord
	//-----------------------------------------------------------------
	itoXYZ2( (ito.prim.offset_x << 4) + x0, (ito.prim.offset_y << 4) + y0, 0);
	itoXYZ2( (ito.prim.offset_x << 4) + x1, (ito.prim.offset_y << 4) + y1, 0);
	
	itoCheckGsPacket();
}

void itoSpriteX(uint64 color, uint16 x1, uint16 y1, uint16 x2, uint16 y2)
{	
		
	itoGifTag(4, TRUE, FALSE, 0, ITO_GIFTAG_TYPE_LIST, 1, ITO_GIFTAG_A_D);
	itoPRIM( ITO_PRIM_SPRITE	, ITO_PRIM_SHADE_FLAT, FALSE, ito.prim.fog, ito.prim.alpha, FALSE, 0, itoActiveFrameBuffer, 0);
	
	//-----------------------------------------------------------------		
	// Sprites do not support gouraud shading, therefor
	// only 1 color is passed to the RGBAQ register.
	//-----------------------------------------------------------------

	itoRGBAQ(color);
	//-----------------------------------------------------------------
	// top-left choords
	//-----------------------------------------------------------------

	itoXYZ2( (ito.prim.offset_x << 4) + x1, (ito.prim.offset_y << 4) + y1, 0);
	
	//-----------------------------------------------------------------
	// bottom-right choords
	//-----------------------------------------------------------------
	
	itoXYZ2( (ito.prim.offset_x << 4) + x2, (ito.prim.offset_y << 4) + y2, 0);
	//-----------------------------------------------------------------
	// Check if the GS packet has reached its maximum size
	//-----------------------------------------------------------------
	
	itoCheckGsPacket();
}

void itoAddVertex2(uint64 color1, uint16 x1, uint16 y1, uint64 color2, uint16 x2, uint16 y2)
{	
	itoRGBAQ( color1 );
	itoXYZ2( (ito.prim.offset_x + x1) << 4, (ito.prim.offset_y + y1) << 4, 0);		
	itoRGBAQ( color2 );
	itoXYZ2( (ito.prim.offset_x + x2) << 4, (ito.prim.offset_y + y2) << 4, 0);		
}

