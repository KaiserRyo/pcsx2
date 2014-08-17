/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __R3000A_H__
#define __R3000A_H__

#include <stdio.h>

enum GPRRegs {
	r0 = 0,
	at = 1,
	v0 = 2,
	v1 = 3,
	a0 = 4,
	a1 = 5,
	a2 = 6,
	a3 = 7,
	t0 = 8,
	t1 = 9,
	t2 = 10,
	t3 = 11,
	t4 = 12,
	t5 = 13,
	t6 = 14,
	t7 = 15,
	s0 = 16,
	s1 = 17,
	s2 = 18,
	s3 = 19,
	s4 = 20,
	s5 = 21,
	s6 = 22,
	s7 = 23,
	t8 = 24,
	t9 = 25,
	k0 = 26,
	k1 = 27,
	gp = 28,
	sp = 29,
	s8 = 30,
	ra = 31,
	hi = 32,  // hi needs to be at index 32! don't change
	lo = 33,
};

enum CP0Regs {
	Index = 0,
	Random = 1,
	EntryLo0 = 2,
	EntryLo1 = 3,
	Context = 4,
	PageMask = 5,
	Wired = 6,
	Reserved0 = 7,
	BadVAddr = 8,
	Count = 9,
	EntryHi = 10,
	Compare = 11,
	Status = 12,
	Cause = 13,
	EPC = 14,
	PRid = 15,
	Config = 16,
	LLAddr = 17,
	WatchLO = 18,
	WatchHI = 19,
	XContext = 20,
	Reserved1 = 21,
	Reserved2 = 22,
	Reserved3 = 23,
	Reserved4 = 24,
	Reserved5 = 25,
	ECC = 26,
	CacheErr = 27,
	TagLo = 28,
	TagHi = 29,
	ErrorEPC = 31,
	Reserved6 = 32,
};

struct SVector2D {
	short x, y;
};

struct SVector2Dz {
	short z, pad;
};

struct SVector3D {
	short x, y, z, pad;
};

struct LVector3D {
	short x, y, z, pad;
};

struct CBGR {
	unsigned char r, g, b, c;
};

struct SMatrix3D {
	short m11, m12, m13, m21, m22, m23, m31, m32, m33, pad;
};

// TODO: Unused structs.
/*
union CP2Data {
	struct {
		SVector3D     v0, v1, v2;
		CBGR          rgb;
		s32          otz;
		s32          ir0, ir1, ir2, ir3;
		SVector2D     sxy0, sxy1, sxy2, sxyp;
		SVector2Dz    sz0, sz1, sz2, sz3;
		CBGR          rgb0, rgb1, rgb2;
		s32          reserved;
		s32          mac0, mac1, mac2, mac3;
		u32 irgb, orgb;
		s32          lzcs, lzcr;
	} n;
	u32 r[32];
};

union CP2Ctrl {
	struct {
		SMatrix3D rMatrix;
		s32      trX, trY, trZ;
		SMatrix3D lMatrix;
		s32      rbk, gbk, bbk;
		SMatrix3D cMatrix;
		s32      rfc, gfc, bfc;
		s32      ofx, ofy;
		s32      h;
		s32      dqa, dqb;
		s32      zsf3, zsf4;
		s32      flag;
	} n;
	u32 r[32];
};*/

struct psxRegisters {
	u32 GPR[34];          // General Purpose Registers: Lo, Hi in r[33] and r[32]
	u32 CP0[32];           // Coprocessor0 Registers
	u32 CP2D[32];       // Cop2 data registers
	u32 CP2C[32];        // Cop2 control registers
	u32 pc;                       // Program counter
	u32 code;                 // The instruction
	u32 cycle;
	u32 interrupt;
	u32 sCycle[32];     // start cycle for signaled ints
	s32 eCycle[32];     // cycle delta for signaled ints (sCycle + eCycle == branch cycle)
	//u32 _msflag[32];
	//u32 _smflag[32];
};

extern __aligned16 psxRegisters psxRegs;

extern u32 g_iopNextEventCycle;
extern s32 iopBreak;		// used when the IOP execution is broken and control returned to the EE
extern s32 iopCycleEE;		// tracks IOP's current sych status with the EE

#ifndef _PC_

#define _i32(x) (s32)x //R3000A
#define _u32(x) (u32)x //R3000A

#define _i16(x) (s16)x // Not used
#define _u16(x) (u16)x // Not used

#define _i8(x) (s8)x // Not used
#define _u8(x) (u8)x //R3000A - once

/**** R3000A Instruction Macros ****/
#define _PC_       psxRegs.pc       // The next PC to be executed

#define _Funct_  ((psxRegs.code      ) & 0x3F)  // The funct part of the instruction register
#define _Rd_     ((psxRegs.code >> 11) & 0x1F)  // The rd part of the instruction register
#define _Rt_     ((psxRegs.code >> 16) & 0x1F)  // The rt part of the instruction register
#define _Rs_     ((psxRegs.code >> 21) & 0x1F)  // The rs part of the instruction register
#define _Sa_     ((psxRegs.code >>  6) & 0x1F)  // The sa part of the instruction register
#define _Im_     ((u16)psxRegs.code) // The immediate part of the instruction register
#define _Target_ (psxRegs.code & 0x03ffffff)    // The target part of the instruction register

#define _Imm_	((short)psxRegs.code) // sign-extended immediate
#define _ImmU_	(psxRegs.code&0xffff) // zero-extended immediate

#define _rRs_   psxRegs.GPR.r[_Rs_]   // Rs register
#define _rRt_   psxRegs.GPR.r[_Rt_]   // Rt register
#define _rRd_   psxRegs.GPR.r[_Rd_]   // Rd register
#define _rSa_   psxRegs.GPR.r[_Sa_]   // Sa register
#define _rFs_   psxRegs.CP0.r[_Rd_]   // Fs register

#define _c2dRs_ psxRegs.CP2D.r[_Rs_]  // Rs cop2 data register
#define _c2dRt_ psxRegs.CP2D.r[_Rt_]  // Rt cop2 data register
#define _c2dRd_ psxRegs.CP2D.r[_Rd_]  // Rd cop2 data register
#define _c2dSa_ psxRegs.CP2D.r[_Sa_]  // Sa cop2 data register

#define _rHi_   psxRegs.GPR.n.hi   // The HI register
#define _rLo_   psxRegs.GPR.n.lo   // The LO register

#define _JumpTarget_    ((_Target_ << 2) + (_PC_ & 0xf0000000))   // Calculates the target during a jump instruction
#define _BranchTarget_  (((s32)(s16)_Imm_ * 4) + _PC_)                 // Calculates the target during a branch instruction

#define _SetLink(x)     psxRegs.GPR.r[x] = _PC_ + 4;       // Sets the return address in the link register

extern s32 EEsCycle;
extern u32 EEoCycle;

#endif

extern s32 psxNextCounter;
extern u32 psxNextsCounter;
extern bool iopEventAction;
extern bool iopEventTestIsActive;

// Branching status used when throwing exceptions.
extern bool iopIsDelaySlot;

// --------------------------------------------------------------------------------------
//  R3000Acpu
// --------------------------------------------------------------------------------------

struct R3000Acpu {
	void (*Reserve)();
	void (*Reset)();
	void (*Execute)();
	s32 (*ExecuteBlock)( s32 eeCycles );		// executes the given number of EE cycles.
	void (*Clear)(u32 Addr, u32 Size);
	void (*Shutdown)();
	
	uint (*GetCacheReserve)();
	void (*SetCacheReserve)( uint reserveInMegs );
};

extern R3000Acpu *psxCpu;
extern R3000Acpu psxInt;
extern R3000Acpu psxRec;

extern void psxReset();
extern void __fastcall psxException(u32 code, u32 step);
extern void iopEventTest();
extern void psxMemReset();

// Subsets
extern void (*psxBSC[64])();
extern void (*psxSPC[64])();
extern void (*psxREG[32])();
extern void (*psxCP0[32])();
extern void (*psxCP2[64])();
extern void (*psxCP2BSC[32])();

#endif /* __R3000A_H__ */
