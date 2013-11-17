// Sorry, very old and ugly code!

#include "Strings.h"
#include <stdexcept>

std::string strip(const std::string& toStrip, std::string charsToStrip)
{
	std::string result;
	for(char x : toStrip)
		if(charsToStrip.find(x) == charsToStrip.npos)
			result+=x;
		return result;
}

std::string lstrip(const std::string& toStrip, char charToStrip)
{
	try
	{
		return toStrip.substr(toStrip.find_first_not_of(charToStrip));
	}
	catch(std::out_of_range)
	{
		return "";
	}
}

std::string lstrip(const std::string& toStrip, std::string charsToStrip)
{
	try
	{
		return toStrip.substr(toStrip.find_first_not_of(charsToStrip));
	}
	catch(std::out_of_range)
	{
		return "";
	}
}

std::string rstrip(const std::string& toStrip, char charToStrip)
{
	try
	{
		return toStrip.substr(0, toStrip.find_last_not_of(charToStrip)+1);
	}
	catch(std::out_of_range)
	{
		return "";
	}
}

std::string rstrip(const std::string& toStrip, std::string charsToStrip)
{
	try
	{
		return toStrip.substr(0, toStrip.find_last_not_of(charsToStrip)+1);
	}
	catch(std::out_of_range)
	{
		return "";
	}
}

std::string strip(const std::string& toStrip, char charToStrip)
{
	std::string result;
	for(char x : toStrip)
		if(x != charToStrip)
			result+=x;
		return result;
}