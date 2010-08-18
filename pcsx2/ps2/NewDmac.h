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

#pragma once

#include "Common.h"
#include "Hardware.h"

#undef dmacRegs
#undef intcRegs

enum DMA_StallMode
{
	// No stalling logic is performed (STADR is not read or written)
	DmaStall_None,
	
	// STADR is written with the MADR after data is transfered.
	DmaStall_Source,

	// STADR is read and MADR is not allowed to advance beyond that point.
	DmaStall_Drain,
};

enum DMA_DirectionMode
{
	// Indicates a DMA that transfers from peripheral to memory
	DmaDir_Source,
	
	// Indicates a DMA that transfers from peripheral to memory
	DmaDir_Drain,
	
	// Indicates a DAM that bases its transfer direction on the DIR bit of its CHCR register.
	DmaDir_Both,
};

enum DMA_ChannelId
{
	DmaId_VIF0 = 0,
	DmaId_VIF1,
	DmaId_GIF,
	DmaId_fromIPU,
	DmaId_toIPU,
	DmaId_SIF0,
	DmaId_SIF1,
	DmaId_SIF2,
	DmaId_fromSPR,
	DmaId_toSPR,

	DmaId_None
};

#define __dmacall

// Returns the number of QWC actually transferred.  Return value can be 0, in cases where the
// peripheral has no room to receive data (SIF FIFO is full, or occupied by another channel,
// for example).
typedef uint __dmacall FnType_ToPeripheral(const u128* srcBase, uint srcSize, uint srcStartQwc, uint lenQwc);

// Returns the number of QWC actually transferred.  Return value can be 0, in cases where the
// peripheral has no data to provide (SIF FIFO is empty, or occupied by another channel,
// for example).
typedef uint __dmacall FnType_FromPeripheral(u128* dest, uint destSize, uint destStartQwc, uint lenQwc);

typedef FnType_ToPeripheral*	Fnptr_ToPeripheral;
typedef FnType_FromPeripheral*	Fnptr_FromPeripheral;


// --------------------------------------------------------------------------------------
//  DMA_ChannelRegisters
// --------------------------------------------------------------------------------------
struct DMA_ChannelRegisters
{
	tDMA_CHCR	chcr;
	u32	_null0[3];

	tDMAC_ADDR	madr;
	u32 _null1[3];

	tDMA_QWC	qwc;
	u32 _null2[3];


	tDMAC_ADDR	tadr;
	u32 _null3[3];

	tDMAC_ADDR	asr0;
	u32 _null4[3];

	tDMAC_ADDR	asr1;
	u32 _null5[11];

	tDMA_SADR	sadr;
	u32 _null6[3];
};

// --------------------------------------------------------------------------------------
//  tDMA_TAG64
// --------------------------------------------------------------------------------------
union tDMA_TAG64
{
	struct
	{
		u16 QWC;
		
		struct
		{
			u16 _reserved2	: 10;
			u16 PCE			: 2;
			u16 ID			: 3;
			u16 IRQ			: 1;
		};

		// Upper 32 bits
		tDMAC_ADDR	addr;
	};
	u64 _u64;
	u32 _u32[2];
	u16 _u16[4];

	tDMA_TAG64(u64 val) { _u64 = val; }

	// Returns the upper 16 bits of the tag, which is typically stored to the channel's
	// CHCR register during chain mode processing.
	u16 Bits16to31() const { return _u16[1]; }
	
	wxString ToString(DMA_DirectionMode dir) const
	{
		const char* label;
		switch(ID)
		{
			case TAG_REFE:
				label = (dir == DmaDir_Source) ? "CNTS" : "REFE";
			break;

			case TAG_CNT:	label = "CNT";		break;
			case TAG_NEXT:	label = "NEXT";		break;
			case TAG_REF:	label = "REF";		break;
			case TAG_REFS:	label = "REFS";		break;
			case TAG_CALL:	label = "CALL";		break;
			case TAG_RET:	label = "RET";		break;
			case TAG_END:	label = "END";		break;

			default:		label = "????";		break;
		}

		return pxsFmt("%s ADDR=0x%08X%s, QWC=0x%04X, IRQ=%s, PCE=%s",
			addr.ADDR, addr.SPR ? "(SPR)" : "", QWC,
			IRQ ? "on" : "off",
			PCE ? "on" : "off"
		);
	}

	void Clear() { _u64 = 0; }

};

// --------------------------------------------------------------------------------------
//  Exception::DmaRaiseIRQ
// --------------------------------------------------------------------------------------
// This is a local exception for doing error/IRQ-related flow control within the context
// of the DMAC.  The exception is not (and should never be) leaked to any external contexts.
namespace Exception
{
	class DmaRaiseIRQ
	{
	public:
		bool			m_BusError;
		bool			m_MFIFOstall;
		bool			m_Verbose;
		const wxChar*	m_Cause;

		DmaRaiseIRQ( const wxChar* _cause=NULL )
		{
			m_BusError		= false;
			m_MFIFOstall	= false;
			m_Verbose		= false;
			m_Cause			= _cause;
		}

		DmaRaiseIRQ& BusError()		{ m_BusError	= true; return *this; }
		DmaRaiseIRQ& MFIFOstall()	{ m_MFIFOstall	= true; return *this; }
		DmaRaiseIRQ& Verbose()		{ m_Verbose		= true; return *this; }
		
	};
}

// --------------------------------------------------------------------------------------
//  DMA_ChannelInformation
// --------------------------------------------------------------------------------------
struct DMA_ChannelInformation
{
	const char*		NameA;
	const wxChar*	NameW;

	uint			regbaseaddr;
	
	DMA_StallMode	DmaStall;
	bool			hasSourceChain;
	bool			hasDestChain;
	bool			hasAddressStack;
	bool			isSprChannel;

	// (Drain) Non-Null for channels that can xfer from main memory to peripheral.
	Fnptr_ToPeripheral		toFunc;

	// (Source) Non-Null for channels that can xfer from peripheral to main memory.
	Fnptr_FromPeripheral	fromFunc;

	DMA_DirectionMode GetRawDirection() const
	{
		if (toFunc && fromFunc) return DmaDir_Both;
		return toFunc ? DmaDir_Drain : DmaDir_Source;
	}

	DMA_ChannelRegisters& GetRegs() const
	{
		return (DMA_ChannelRegisters&)eeMem->HW[regbaseaddr];
	}

	tDMA_CHCR& CHCR() const
	{
		return GetRegs().chcr;
	}

	tDMAC_ADDR& MADR() const
	{
		return GetRegs().madr;
	}

	tDMA_QWC& QWC() const
	{
		return GetRegs().qwc;
	}
	
	tDMAC_ADDR& TADR() const
	{
		pxAssert(hasSourceChain || hasDestChain);
		return GetRegs().tadr;
	}

	tDMAC_ADDR& ASR0() const
	{
		pxAssert(hasAddressStack);
		return GetRegs().asr0;
	}

	tDMAC_ADDR& ASR1() const
	{
		pxAssert(hasAddressStack);
		return GetRegs().asr1;
	}
	
	tDMA_SADR& SADR() const
	{
		pxAssert(isSprChannel);
		return GetRegs().sadr;
		
	}
	
	wxCharBuffer ToUTF8() const;
};

// --------------------------------------------------------------------------------------
//  DMA_ChannelState
// --------------------------------------------------------------------------------------
class DMA_ChannelState
{
protected:
	const DMA_ChannelId				Id;
	DMA_ControllerRegisters&		dmacReg;
	const DMA_ChannelInformation&	chan;
	DMA_ChannelRegisters&			creg;
	tDMA_CHCR&						chcr;
	tDMAC_ADDR&						madr;

public:
	DMA_ChannelState( DMA_ChannelId chanId );

	DMA_DirectionMode GetDir() const;
	bool DrainStallActive() const;
	bool SourceStallActive() const;
	bool MFIFOActive() const;
	bool TestArbitration();
	void TransferData();

	bool IsSliced()
	{
		return (Id < DmaId_fromSPR);
	}

	bool IsBurst()
	{
		return (Id >= DmaId_fromSPR);
	}

protected:
	void TransferInterleaveData();
	void TransferNormalAndChainData();

	void MFIFO_SrcChainUpdateTADR();
	void MFIFO_SrcChainUpdateMADR( const tDMA_TAG64& tag );

	void SrcChainUpdateTADR();
	void SrcChainUpdateMADR( const tDMA_TAG64& tag );

	void DstChainUpdateTADR();
	void DstChainUpdateMADR();
};