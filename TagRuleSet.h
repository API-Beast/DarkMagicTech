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

class ConfigFile;

typedef std::set<std::string> TagSet;

class TagMap;
class TagExpression;
class TagRule;
class TagRuleSet
{
public:
	TagRuleSet(TagRuleSet&& other);
	TagRuleSet();
	~TagRuleSet();
	TagRuleSet findMatches(const TagMap& set) const;
	bool apply(TagMap& set);
	void findAndApplyRecursive(TagMap& set);
	void loadFromConfig(const ConfigFile& file, const std::string& cat);
	void sortByPriority();
	void sortByScore(const TagMap& set);
	TagRule*& operator[](int index);
	int numRules();
	std::vector<TagRule*> Rules; 
	std::vector<TagRule>* CoreRules = nullptr;
	bool OwnsCoreRules = false;
};

class TagExpression
{
public:
	void load(const std::string& list);
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
	std::map<std::string, std::string> Extra;
	TagRuleSet SubRules;
	TagSet ApplyTriggers;
	TagSet OnTriggers;
	int Priority = 0;
	int ScoreNeededForMatch = 0;
	
	std::string& operator[](const std::string& i){ return Extra[i]; };
	bool matches(const TagMap& set);
	int calcScore(const TagMap& set);
	bool apply(TagMap& set) const;
};

#endif // TAGRULESET_H
