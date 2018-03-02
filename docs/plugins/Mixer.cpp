/* Copyright (C) 2002 Pete Warden

Effect plugin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the Lesser GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// dec 25, 2006 ck: MMX version was trashing two bytes beyond output buffer,
// due to movq when movd was intended; also non-MMX version always output
// black, due to shift right 16 when 8 was intended

#define PETE_USE_MMX


#include "Mixer.h"
#include "PeteHelpers.h"


#ifdef PETE_USE_MMX
//#include "mmintrin.h"	// ck: I don't have these
#endif // PETE_USE_MMX

// ck: my cheesy substitutes for mmintrin.h
#define __m64 __int64
#define _mm_set_pi16(a, b, c, d) (__int64(a) << 48) + (__int64(b) << 32) + (__int64(c) << 16) + (__int64(d) << 0)
#define _m_empty __asm emms;

static SPete_Parameter g_Parameters[]={
	{
		"Mix Value",
		128.0f,
		0.0f,
		255.0f,
		PETE_PARAM_INT
	},
};
static int g_nParametersCount=sizeof(g_Parameters)/sizeof(g_Parameters[0]);

void Pete_Mixer_GetParameters(SPete_Parameter** poutpParameters,int* poutnParametersCount) {
	*poutpParameters=&g_Parameters[0];
	*poutnParametersCount=g_nParametersCount;
}

int Pete_Mixer_Init(SPete_Mixer_Data* pInstanceData,int nWidth, int nHeight) {

	Pete_Mixer_DeInit(pInstanceData);

	const int nNumPixels=nWidth*nHeight;
	const int nNumBytes=(nNumPixels*sizeof(U32));
	
	pInstanceData->nWidth=nWidth;
	pInstanceData->nHeight=nHeight;

	return 1;

}

void Pete_Mixer_DeInit(SPete_Mixer_Data* pInstanceData) {
	// do nothing
}

void Pete_Mixer_Render(SPete_Mixer_Data* pInstanceData,SPete_Mixer_Settings* pSettings,U32** ppSources,U32* pOutput) {

	U32* pSourceA=ppSources[0];
	U32* pSourceB=ppSources[1];

	const int nWidth=pInstanceData->nWidth;
	const int nHeight=pInstanceData->nHeight;

	const int nNumPixels=nWidth*nHeight;
	const int nNumBytes=(nNumPixels*sizeof(U32));
	
	const int nMixValue=static_cast<int>(pSettings->m_MixValue);

	const int nOneMinusLerp=(nMixValue<<0);
	const int nLerpValue=(255-nMixValue)<<0;

	U32* pCurrentSourceA=pSourceA;
	U32* pCurrentSourceB=pSourceB;
	U32* pCurrentOutput=pOutput;
	const U32* pOutputEnd=(pOutput+nNumPixels);

#ifdef PETE_USE_MMX

	const __m64 OneMinusLerpReg=
		_mm_set_pi16(
		nOneMinusLerp,
		nOneMinusLerp,
		nOneMinusLerp,
		nOneMinusLerp);

	const __m64 LerpValueReg=
		_mm_set_pi16(
		nLerpValue,
		nLerpValue,
		nLerpValue,
		nLerpValue);

	__asm {

		pxor		mm0,mm0
		movq		mm1,OneMinusLerpReg
		movq		mm2,LerpValueReg

	}

#endif // PETE_USE_MMX

	while (pCurrentOutput!=pOutputEnd) {

#ifndef PETE_USE_MMX
		
		const U32 SourceAColour=*pCurrentSourceA;

		const int nSourceARed=(SourceAColour>>SHIFT_RED)&0xff;
		const int nSourceAGreen=(SourceAColour>>SHIFT_GREEN)&0xff;
		const int nSourceABlue=(SourceAColour>>SHIFT_BLUE)&0xff;

		const U32 SourceBColour=*pCurrentSourceB;

		const int nSourceBRed=(SourceBColour>>SHIFT_RED)&0xff;
		const int nSourceBGreen=(SourceBColour>>SHIFT_GREEN)&0xff;
		const int nSourceBBlue=(SourceBColour>>SHIFT_BLUE)&0xff;

		int nOutputRed=
			(nSourceARed*nLerpValue)+
			(nSourceBRed*nOneMinusLerp);
//		nOutputRed>>=16;	// ck: shifting twice as much as needed
		nOutputRed>>=8;

		int nOutputGreen=
			(nSourceAGreen*nLerpValue)+
			(nSourceBGreen*nOneMinusLerp);
//		nOutputGreen>>=16;	// ck: shifting twice as much as needed
		nOutputGreen>>=8;

		int nOutputBlue=
			(nSourceABlue*nLerpValue)+
			(nSourceBBlue*nOneMinusLerp);
//		nOutputBlue>>=16;	// ck: shifting twice as much as needed
		nOutputBlue>>=8;

		const U32 OutputColour=
			(nOutputRed<<SHIFT_RED)|
			(nOutputGreen<<SHIFT_GREEN)|
			(nOutputBlue<<SHIFT_BLUE);

		*pCurrentOutput=OutputColour;

#else // PETE_USE_MMX

		__asm {

			mov			esi,pCurrentSourceA;
//			movq		mm7,[esi]	// ck: only need 32 bits
			movd		mm7,[esi]	// ck: 32-bit move
			punpcklbw	mm7,mm0		// ck: mm7 = 0, MSW SrcA, 0, LSW SrcA

			mov			esi,pCurrentSourceB;
//			movq		mm6,[esi]	// ck: only need 32 bits
			movd		mm6,[esi]	// ck: 32-bit move
			punpcklbw	mm6,mm0		// ck: mm6 = 0, MSW SrcB, 0, LSW SrcB

			pmullw		mm7,mm1
			pmullw		mm6,mm2

			paddw		mm7,mm6

			psrlw		mm7,8

			packuswb	mm7,mm0

			mov			esi,pCurrentOutput
//			movq		[esi],mm7	// ck: TRASHES 2 bytes beyond output buffer
			movd		[esi],mm7	// ck: 32-bit move

		}


#endif // PETE_USE_MMX

		pCurrentOutput+=1;
		pCurrentSourceA+=1;
		pCurrentSourceB+=1;

	}

	_m_empty();

}
