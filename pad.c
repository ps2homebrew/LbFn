#include "launchelf.h"

struct padButtonStatus buttons;
u32 paddata;
u32 old_pad = 0;
u32 new_pad;

//////////////////////////////////////////////////////////////////////
// read PAD
int readpad(void)
{
	static int n=0, nn=0;
	int ret;
	
	ret = padRead(0, 0, &buttons);
	if (ret != 0){
		paddata = 0xffff ^ buttons.btns;
		new_pad = paddata & ~old_pad;
		if(!new_pad && (old_pad==paddata)){
			int p = SCANRATE * 42 / 100;
			int q = (SCANRATE * 8 +50) / 100;
			n++;		/*
			if(ITO_VMODE_AUTO==ITO_VMODE_NTSC){
				if(n>=25){	// 25/60: 0.41667
					new_pad=paddata;
					if(nn++ < 20)	n=20;	// -5:0.08333
					else			n=23;	// -2
				}
			}
			else{
				if(n>=21){	// 21/50: 0.42
					new_pad=paddata;
					if(nn++ < 20)	n=17;	// -4:0.08
					else			n=19;	// -2
				}
			}			*/
			if (n >= p) {
				new_pad = paddata;
				if (nn++ < 20)	n = p -q;
				else			n = p -2;
			}
		}
		else{
			n=0;
			nn=0;
			old_pad = paddata;
		}
		return 1;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Wait PAD
void waitPadReady(int port, int slot)
{
	int state;//, lastState;
//	char stateString[16];

	state = padGetState(port, slot);
//	lastState = -1;
	while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)){
//		if (state != lastState)
//			padStateInt2String(state, stateString);
//		lastState = state;
		state = padGetState(port, slot);
	}
}

//////////////////////////////////////////////////////////////////////
// setup PAD
int setupPad(void)
{
	static char padBuf[256] __attribute__((aligned(64)));
	int ret, i, modes;
	
	padInit(0);
	if((ret = padPortOpen(0, 0, padBuf)) == 0)
		return 0;
	waitPadReady(0, 0);
	modes = padInfoMode(0, 0, PAD_MODETABLE, -1);
	if (modes != 0){
		i = 0;
		do{
			if (padInfoMode(0, 0, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK){
				padSetMainMode(0, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
				break;
			}
			i++;
		} while (i < modes);
	}
	return 1;
}
