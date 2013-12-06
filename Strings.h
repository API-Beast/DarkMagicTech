// Sorry! Very old and ugly code!

#ifndef STRINGCONVERSION_H
#define STRINGCONVERSION_H

#include <string>
#include <vector>
#include <sstream>

std::string strip(const std::string& toStrip, char charToStrip);
std::string strip(const std::string& toStrip, std::string charsToStrip);
std::string lstrip(const std::string& toStrip, char charToStrip);
std::string lstrip(const std::string& toStrip, std::string charsToStrip);
std::string rstrip(const std::string& toStrip, char charToStrip);
std::string rstrip(const std::string& toStrip, std::string charsToStrip);

template<class container = std::vector<std::string>>
inline container seperateString(const std::string& toConvert, char seperator = ',')
{
	std::vector<std::string> vect;
	std::string::size_type curPos = 0;
	std::string::size_type lastPos = 0;
	while(true)
	{
		curPos = toConvert.find_first_of(seperator, curPos + 1);
		if(lastPos == toConvert.npos)
			break;
		vect.push_back(strip(toConvert.substr(lastPos, curPos - lastPos), ", \n"));
		lastPos = curPos;
	}
	container retVal(vect.begin(), vect.end());
	return retVal;
};

template<class T>
inline T FromString(const std::string& value, T defaultValue)
{
  T retVal = defaultValue;
  try
  {
    std::istringstream(value) >> retVal;
  }
  catch(...)
  {
    return defaultValue;
  }
  return retVal;
}

#endif // STRINGCONVERSION_H
// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on;  replace-tabs on;
