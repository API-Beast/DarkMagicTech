#include "ConfigFile.h"
#include <fstream>
#include "Strings.h"

using namespace std;

ConfigFile::ConfigFile(const string& path)
{
	mData = new ConfigFileData(path);
}

ConfigFile::~ConfigFile()
{
	delete mData;
}

bool ConfigFileData::readConfig(const string& path)
{
	string section;
	string entry;
	string value;
	int pos;

	bool continuation = false;
	string curHierarchy;

	ifstream file;
	file.open(path, ifstream::in);
	if(file.bad())
		return false;

	map<string, int> loadedSections;

	string line;
	while(file.good())
	{
		getline(file, line);
		// Comments ("; This is an comment")
		if(line[0] == ';' || line[0] == '#')
			continue;

		// Sections ("[Game]")
		if(line[0] == '[')
		{
			pos = line.find_first_of(']');
			if(pos == string::npos)
				continue;
			section = line.substr(1, pos - 1);
			// Automatically generate unique names for duplicate section names
			if(section.find('.') == string::npos)
			{
				int& sectionsWithName = loadedSections[section];
				section.append("." + to_string(sectionsWithName));
				sectionsWithName++;
				continue;
			}
		}

		// Values ("Fullscreen=1")
		pos = line.find_first_of('=');
		if(pos != string::npos)
		{
			entry = line.substr(0, pos);
			value = line.substr(pos + 1);

			// Prepend the section name
			if(section.length() != 0)
				entry = section + "." + entry;

			// an backslash shows that the value will be continued in the next line
			if(value[value.length() - 1] == '\\')
			{
				value[value.length() - 1] = '\n';
				continuation = true;
			}
			// Save it.
			addData(entry, value, path);
		}
		else if(!entry.empty() && continuation)
		{
			value = line;
			// an backslash shows that the value will be continued in the next line
			if(value[value.length() - 1] == '\\')
			{
				value[value.length() - 1] = '\n';
				continuation = true;
			}
			else
				continuation = false;
			mFields[entry].value += value;
		}
	}
	return true;
}

ConfigFileData::ConfigFileData(const string& path) : mFields()
{
	readConfig(path);
}

ConfigFileData::ConfigFileData() : mFields() {}

ConfigFileData::ConfigFileData(const ConfigFileData& other) :  mFields(other.mFields)
{
}

ConfigFileData::Entry& ConfigFileData::operator[](const string& key)
{
	return mFields[key];
}

void ConfigFileData::addData(const string& key, const string& content, const string& file, bool changed)
{
	size_t pos;
	string group;
	string parentGroup;

	pos = key.find_last_of('.');
	if(pos != string::npos)
	{
		group = key.substr(0, pos);
	}

	mGroups[group].childEntries.insert(key);

	// Iterate through the parent groups and insert the inhertance
	while((pos = group.find_last_of('.')) != string::npos)
	{
		parentGroup = group.substr(0, pos);
		mGroups[parentGroup].childGroups.insert(group);
		group = parentGroup;
	}

	mFields[key] = {content, file, changed};
}

string ConfigFile::get(string field, const char* defaultValue) const
{
	if(!mCurGroup.empty())
		field = mCurGroup + "." + field;

	if(mData->mFields.find(field) != mData->mFields.end())
		return mData->mFields[field].value;

	return string(defaultValue);
}

const set< string >& ConfigFile::getChildGroups(const string& group) const
{
	return mData->mGroups[group].childGroups;
}

const set< string >& ConfigFile::getChildEntries(const string& group) const
{
	return mData->mGroups[group].childEntries;
}

void ConfigFile::setGroup(const string& group) const
{
	mCurGroup = group;
}