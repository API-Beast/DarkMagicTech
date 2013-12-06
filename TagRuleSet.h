/*
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#ifndef TAGRULESET_H
#define TAGRULESET_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include "Parsing/ConfigFile.h"

typedef std::set<std::string> TagSet;

class TagMap;
class TagExpression;
class TagRule;

class TagRegistry
{
public:
	TagRule& registerRule(const TagRule& rule);
	std::list<TagRule> Rules;
};

class TagRuleSet
{
public:
	TagRuleSet findMatches(const TagMap& set) const;
	bool apply(TagMap& set);
	void findAndApplyRecursive(TagMap& set);
	void loadFromConfig(ConfigFile& file, const std::string& cat, TagRegistry* reg);
	void sortByPriority();
	void sortByScore(const TagMap& set);
	TagRule*& operator[](int index);
	int numRules();
	std::vector<TagRule*> Rules; 
};

class TagExpression
{
public:
	void load(const std::string& list);
	void loadList(const std::vector<std::string>& list);
public:
	enum Linkage
	{
		And,
		Or
	};
	struct Item
	{
		std::string Tag = "<Error>";
		bool Negate = false;
		Linkage Link = And;
		Item(const std::string& tag, bool negate=false, Linkage link=And) : Tag(tag), Negate(negate), Link(link)
		{
		};
	};
	std::vector<Item> Items;
	bool apply(TagSet& tags);
	bool matches(const TagSet& tags);
	int calcScore(const TagSet& tags);
};

class TagMap
{
public:
	struct Data
	{
		TagSet Mutable;
		TagSet Implicit;
		TagSet ImplicitRemoved;
	};
public:
	Data& operator[](const std::string& which){ return mData[which];  };
	Data operator[](const std::string& which) const{ try{ return mData.at(which);} catch(...){ return Data(); }; };
	void resetImplications();
	TagMap difference(const TagMap& other);
	
	TagSet CurrentTriggers;
private:
	std::map<std::string, TagMap::Data> mData;
};

struct TagRule
{
public:
	struct Data
	{
		TagExpression Require;
		TagExpression Apply;
		TagExpression Imply;
	};
public:
	std::map<std::string, TagRule::Data> Expressions;
	
	std::map<std::string, std::string> Properties;
	std::map<std::string, std::vector<std::string>> ListProperties;
	
	TagRuleSet SubRules;
	TagRuleSet Results;
	TagSet ApplyTriggers;
	TagSet OnTriggers;
	
	int ResultOrder = 0;
	int Priority = 0;
	
	int MaxExecutions = 0;
	int Executions = 0;
	
	int ScoreNeededForMatch = 0;
	
	std::string& operator[](const std::string& i){ return Properties[i]; };
	std::vector<TagRule*> getMatchingSubrules(const TagMap& set);
	void loadFromConfigObject(ConfigFile::Object& obj, TagRegistry* reg);
	bool matches(const TagMap& set);
	int calcScore(const TagMap& set);
	bool apply(TagMap& set);
};

#endif // TAGRULESET_H
