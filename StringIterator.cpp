/*
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include "StringIterator.h"

ConstUtf8StringIterator::ConstUtf8StringIterator(const std::string& parent, size_t pos) : mParent(parent), StrIndex(pos)
{
	// Skip incomplete characters
	while(mParent[StrIndex] >= 0x80 && mParent[StrIndex] < 0xC0) StrIndex++;
}

namespace
{
	// We want: 00000000 00000001 01101000 01000100
	// First we need to ignore the bits used to declare the multibyte
	// We have: 11110000 10010110 10100001 10000100
	// Ignore:  ^^^^^    ^^       ^^       ^^
	// We can do that by 0'ing the first bits (Note: 0xff >> x = first x bits = 0, the rest = 1)
	// Octet 0: 11110000 & (0xff >> 5) = xxxx x000 
	// Octet 1: 10010110 & (0xff >> 2) = xx01 0110
	// Octet 2: 10100001 & (0xff >> 2) = xx10 0001
	// Octet 3: 10000100 & (0xff >> 2) = xx00 0100
	// Now we need to compose the 4 byte integer out of these, so we shift them into the right position and "or" them.
	// Octet 0: 11110000 << 6*3 = 000__ ________ ________
	// Octet 1: 10010110 << 6*2 = ___01 0110____ ________
	// Octet 2: 10100001 << 6*1 = _____ ____1000 01______
	// Octet 3: 10000100 <<   0 = _____ ________ __000100
	// Or them all:
	// We get:  00000000 00000001 01101000 01000100
	// We want: 00000000 00000001 01101000 01000100
	// Yay.
	Codepoint ComposeFromMultibyte(const char* c, int bytes)
	{
		Codepoint result = 0;
		result = ((*c++) & (0xff >> 5)) << (bytes-1)*6;
		int i = 0;
		while(i++ < bytes)
			result |= (*c++) & (0xff >> 2) << (bytes-1-i)*6;
		return result;
	};
	
	Codepoint GetUtf8Codepoint(const char& c, int& bytes)
	{
		bytes = 1;
		// Look at the first byte to find length
		if(c < 0x80) // ASCII / Length 1 Octet
			return c;
		else if(c < 0xC0) // Continuation byte? Something is wrong here, skip it, output "bad" character
			return 0xFFFD; // Unicode Replacement Character - Replace the "bad" character to be better able to debug all this
		else
		{ 
			if(c < 0x800)              bytes = 2;
			else if(c-0xd800u < 0x800) bytes = 3;
			else if(c < 0x10000)       bytes = 4;
			else if(c < 0x110000)      bytes = 5;
			return ComposeFromMultibyte(&c, bytes);
		}
	}
}

ConstUtf8StringIterator& ConstUtf8StringIterator::operator++()
{
	int bytes;
	mCurrentCodepoint = GetUtf8Codepoint(mParent[StrIndex], bytes);
	StrIndex += bytes;
	return *this;
}

ConstUtf8StringIterator& ConstUtf8StringIterator::operator--()
{
	StrIndex--;
	// Advance to last actual start byte
	// FIXME If a string has loose continuation bytes operator-- doesn't reverse operator++
	while(mParent[StrIndex] >= 0x80 && mParent[StrIndex] < 0xC0) StrIndex--;
	int bytes;
	mCurrentCodepoint = GetUtf8Codepoint(mParent[StrIndex], bytes);
}