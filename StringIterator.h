/*
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#ifndef STRINGITERATOR_H
#define STRINGITERATOR_H

#include <string>
#include <cinttypes>

class ConstStringIterator
{
public:
	ConstStringIterator(const std::string& parent, size_t pos=0) : mParent(parent), StrIndex(pos) {};
	ConstStringIterator& operator++(){ StrIndex++; return *this; };
	ConstStringIterator& operator--(){ StrIndex--; return *this; };
	bool operator==(const ConstStringIterator& other) const{ return other.StrIndex == StrIndex; };
	operator char() const { return mParent[StrIndex]; };
	bool inRange() const { return (StrIndex < mParent.size()) && (StrIndex > 0); };
	size_t StrIndex;
private:
	const std::string& mParent;
};

typedef char32_t Codepoint;
class ConstUtf8StringIterator
{
public:
	ConstUtf8StringIterator(const std::string& parent, size_t pos=0);
	ConstUtf8StringIterator& operator++();
	ConstUtf8StringIterator& operator--();
	bool operator==(const ConstStringIterator& other) const{ return other.StrIndex == StrIndex; };
	operator Codepoint() const{ return mCurrentCodepoint; };
	bool inRange() const { return (StrIndex < mParent.size()) && (StrIndex > 0); };
	size_t StrIndex;
private:
	const std::string& mParent;
	Codepoint mCurrentCodepoint = 0;
};

#endif // STRINGITERATOR_H
