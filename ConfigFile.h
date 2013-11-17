#ifndef GUL_CONFIGFILE_H
#define GUL_CONFIGFILE_H

#include <string>
#include <map>
#include <set>

struct ConfigFileData;

class ConfigFile
{
public:
	ConfigFile() {};
	ConfigFile(const std::string& path);
	~ConfigFile();
	std::string get(std::string field, const char* defaultValue = "") const;
	void setGroup(const std::string& group) const;
	const std::set< std::string >& getChildGroups(const std::string& group) const;
	const std::set< std::string >& getChildEntries(const std::string& group) const;
private:
	ConfigFileData* mData = nullptr;
	mutable std::string mCurGroup;
};


struct ConfigFileData
{
	struct Entry;
	struct Group;

	ConfigFileData();
	ConfigFileData(const std::string& path);
	ConfigFileData(const ConfigFileData& other);

	void addData(const std::string& key, const std::string& content, const std::string& file, bool changed = false);
	bool readConfig(const std::string& path);

	Entry& operator[](const std::string& key);

	/* ---------------- */
	/* Data structures  */
	/* ---------------- */
	struct Entry
	{
		std::string value;
		std::string file;
		bool changed;
	};

	struct Group
	{
		std::set< std::string > childGroups;
		std::set< std::string > childEntries;
	};

	/* ---------------- */
	/* Variables        */
	/* ---------------- */
	std::map<std::string, Entry> mFields;
	std::map<std::string, Group> mGroups;
};

#endif