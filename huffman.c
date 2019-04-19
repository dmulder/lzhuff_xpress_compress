#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
	uint8_t FreeBits;
	uint16_t NextWord;
	uint8_t *OutputPosition1;
	uint8_t *OutputPosition2;
	uint8_t *OutputPosition;
	uint8_t *BufferEnd;
} Output;

typedef struct
{
	uint16_t SymbolCode[512];
	uint8_t SymbolLength[512];
} HuffmanCodes;

static void Output_init(Output *o, uint8_t *OutputBufferPointer, uint8_t *OutputBufferEnd)
{
	o->FreeBits = 16;
	o->NextWord = 0;
	o->OutputPosition1 = OutputBufferPointer + 256;
	o->OutputPosition2 = OutputBufferPointer + 258;
	o->OutputPosition = OutputBufferPointer + 260;
	o->BufferEnd = OutputBufferEnd;
}

#define SET_UINT16(x,val) (x[0]=(uint8_t)(val), x[1]=(uint8_t)((val)>>8))
static void WriteBits(Output *o, uint8_t NumberOfBitsToWrite, uint32_t BitsToWrite)
{
	if (o->OutputPosition + 1 > o->BufferEnd) {
		errno = EOVERFLOW;
		return;
	}
	if (o->FreeBits >= NumberOfBitsToWrite) {
		o->FreeBits = o->FreeBits – NumberOfBitsToWrite;
		o->NextWord = (o->NextWord << NumberOfBitsToWrite) + BitsToWrite;
	} else {
		o->NextWord = (o->NextWord << o->FreeBits);
		o->NextWord = o->NextWord + (BitsToWrite >> (NumberOfBitsToWrite – o->FreeBits));
		o->FreeBits = o->FreeBits – NumberOfBitsToWrite;
		SET_UINT16(o->OutputPosition1, o->NextWord);
		o->OutputPosition1 = o->OutputPosition2;
		o->OutputPosition2 = o->OutputPosition;
		o->OutputPosition += 2;
		o->FreeBits = o->FreeBits + 16;
		o->NextWord = BitsToWrite;
	}
}

static void WriteByte(Output *o, uint8_t ByteToWrite)
{
	if (o->OutputPosition + 1 > o->BufferEnd) {
		errno = EOVERFLOW;
		return;
	}
	*o->OutputPosition++ = ByteToWrite;
}

static void WriteTwoBytes(Output *o, uint16_t BytesToWrite)
{
	if (o->OutputPosition + 1 > o->BufferEnd) {
		errno = EOVERFLOW;
		return;
	}
	SET_UINT16(o->OutputPosition, BytesToWrite);
	o->OutputPosition += 2;
}



static void FlushBits(Output *o)
{
	o->NextWord <<= o->FreeBits;
	SET_UINT16(o->OutputPosition1, o->NextWord);
	memset(o->OutputPosition2, 0, 2);
}

#define GET_UINT32(x) (x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24))
#define GET_UINT16(x) (x[0]|(x[1]<<8))
static void HuffmanEncode(const uint8_t* in, const uint8_t* in_end, uint8_t* out, uint8_t* out_end)
{
	
	Output o; Output_init(&o, out, out_end);
	/* Write the 256-byte table of symbol bit lengths */
	HuffmanCodes h;
	/* While there are more literals or matches to encode */
	uint32_t mask;
	for (mask = GET_UINT32(in), in += 4; in < in_end; mask >>= 1)
		/* If the next thing is a literal */
		if (!(mask & 1)) {
			uint16_t LiteralValue = *in++;
			WriteBits(&o, h.SymbolLength[LiteralValue], h.SymbolCode[LiteralValue]);
		} else { /* the next thing is a match */
			/* Extract the length and distance of the match */
			uint8_t RawValue = *in++;
			uint16_t Distance = GET_UINT16(in); in += 2;
			uint16_t MatchSymbolValue = 0x100 | RawValue;
			WriteBits(&o, h.SymbolLength[MatchSymbolValue], h.SymbolCode[MatchSymbolValue]);
			if ((MatchSymbolValue & 0xF) == 0xF) {
				uint8_t Length8 = *in++;
				WriteByte(&o, Length8);
				if (Length8 == 0xFF) {
					WriteTwoBytes(&o, GET_UINT16(in));
					in += 2;
				}
			}
			WriteBits(&o, Distance, MatchSymbolValue >> 4);
		}
	}
	WriteBits(&o, h.SymbolLength[256], h.SymbolCode[256]);
	FlushBits(&o);
}

