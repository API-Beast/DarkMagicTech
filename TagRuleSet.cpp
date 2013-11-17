/*
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include "TagRuleSet.h"
#include "Strings.h"
#include "ConfigFile.h"

#include <iostream>
#include <algorithm>

using namespace std;

TagRuleSet::TagRuleSet(TagRuleSet && other)
{
	Rules.swap(other.Rules);
	CoreRules = other.CoreRules;
	OwnsCoreRules = other.OwnsCoreRules;
	other.OwnsCoreRules = false;
}

TagRuleSet::TagRuleSet()
{

}

TagRuleSet::~TagRuleSet()
{
	if(OwnsCoreRules) delete CoreRules;
}

void TagRuleSet::loadFromConfig(const ConfigFile& file, const std::string& cat)
{
	if(OwnsCoreRules)
		delete CoreRules;
	
	CoreRules = new std::vector<TagRule>();
	OwnsCoreRules = true;
	const auto& children = file.getChildGroups(cat);
	for(const std::string& entry : children)
	{
		TagRule rule;
		for(const std::string& entryB : file.getChildGroups(entry))
		{
			std::string tagSet = entryB.substr(entryB.find_last_of('.')+1);
			if(tagSet == "Result") // Reserved, this is not! TODO better solution than hard coding
				continue;
			TagRule::Data& set = rule.Expressions[tagSet];
			set.Apply.load(file.get(entryB+".Apply"));
			set.Imply.load(file.get(entryB+".Imply"));
			set.Require.load(file.get(entryB+".Require"));
		}
		for(const std::string& entryC : file.getChildEntries(entry))
		{
			std::string withoutPrefix = entryC.substr(entry.length()+1);
			rule.Extra[withoutPrefix] = file.get(entryC);
		}
		if(file.getChildGroups(entry+".Result").size())
			rule.SubRules.loadFromConfig(file, entry+".Result");
		CoreRules->push_back(std::move(rule));
	}
	for(TagRule& rule : *CoreRules)
		Rules.push_back(&rule);
}

void TagRuleSet::sortByPriority()
{
	std::sort(Rules.begin(), Rules.end(), [](TagRule* rule, TagRule* ruleB){ return rule->Priority > ruleB->Priority;});
}

void TagRuleSet::sortByScore(const TagMap& set)
{
	std::sort(Rules.begin(), Rules.end(), [&set](TagRule* rule, TagRule* ruleB){ return (rule->calcScore(set) + rule->Priority) < (ruleB->calcScore(set) + ruleB->Priority);});
}

TagRuleSet TagRuleSet::findMatches(const TagMap& set) const
{
	TagRuleSet result;
	result.CoreRules = CoreRules;
	
	for(TagRule* entry : Rules)
	{
		if(entry->matches(set))
			result.Rules.push_back(entry);
	}
	return result;
}

bool TagRuleSet::apply(TagMap& set)
{
	bool changesMade = false;
	for(TagRule* entry : Rules)
		changesMade |= entry->apply(set);
	return changesMade;
}

void TagRuleSet::findAndApplyRecursive(TagMap& set)
{
	bool cont = true;
	while(cont)
	{
		TagRuleSet matches = findMatches(set);
		cont &= matches.apply(set);
	}
}

TagRule*& TagRuleSet::operator[](int index)
{
	return Rules[index];
}

int TagRuleSet::numRules()
{
	return Rules.size();
}

bool TagRule::matches(const TagMap& set)
{
	if(!ScoreNeededForMatch)
	{
		for(auto& reqSets : this->Expressions)
		{
			TagExpression subSet = reqSets.second.Require;
			for(TagExpression::Item& item : subSet.Items)
			{
				ScoreNeededForMatch++;
				if(item.Link == TagExpression::Or)
					ScoreNeededForMatch--;
			}
		}
	}
	return calcScore(set) >= ScoreNeededForMatch;
}

bool TagRule::apply(TagMap& set) const
{
	bool changesMade = false;
	for(auto& i : this->Expressions)
	{
		TagMap::Data& subSet = set[i.first];
		// Foo.Apply = A B C
		for(const TagExpression::Item& item : i.second.Apply.Items)
		{
			if(!item.Negate)
				changesMade |= subSet.Mutable.insert(item.Tag).second;
			else
			{
				auto iter = subSet.Mutable.find(item.Tag);
				if(iter != subSet.Mutable.end())
				{
					changesMade = true;
					subSet.Mutable.erase(iter);
				}
			}
		}
		// Foo.Imply = A B C
		for(const TagExpression::Item& item : i.second.Imply.Items)
		{
			if(!item.Negate)
				changesMade |= subSet.Implicit.insert(item.Tag).second;
			else
			{
				subSet.Implicit.erase(item.Tag);
				changesMade |= subSet.ImplicitRemoved.insert(item.Tag).second;
			}
		}
	}
	set.CurrentTriggers = ApplyTriggers;
	return changesMade;
}

void TagExpression::load(const std::string& list)
{
	std::vector<std::string> strings = seperateString(list, ' ');
	for(std::string curItem : strings)
	{
		if(curItem == "")
			continue;
		if(curItem == "||")
		  Items.back().Link = Or;
		else if(curItem[0] == '!')
			Items.push_back(Item(curItem.substr(1), true, And));
		else
			Items.push_back(Item(curItem, false, And));
	}
}

void TagMap::resetImplications()
{
	for(auto it = mData.begin(); it != mData.end(); it++)
	{
		it->second.Implicit.clear();
		it->second.ImplicitRemoved.clear();
	}
}

TagMap TagMap::difference(const TagMap& other)
{
	TagMap result;
	for(auto it = mData.begin(); it != mData.end(); it++)
	{
		try
		{
			const Data& subSet = it->second;
			const Data& otherSubSet = other.mData.at(it->first);
			Data& differenceSet = result.mData[it->first];
			
			for(const std::string& tag : subSet.Mutable)
				if(otherSubSet.Mutable.count(tag))
					differenceSet.Mutable.insert(tag);
				
			for(const std::string& tag : subSet.Implicit)
				if(otherSubSet.Implicit.count(tag))
					differenceSet.Implicit.insert(tag);
				
			for(const std::string& tag : subSet.ImplicitRemoved)
				if(otherSubSet.ImplicitRemoved.count(tag))
					differenceSet.ImplicitRemoved.insert(tag);
		}
		catch(...)
		{
			continue;
		}
	}
	return result;
}

int TagRule::calcScore(const TagMap& set)
{
	int score = 0;
	for(auto& tmp : this->Expressions)
	{
		tmp.second.Require;
		bool nextIsOr = false;
		bool lastWasTrue = false;
		for(const TagExpression::Item& item : tmp.second.Require.Items)
		{
			bool subResult = false;
			const TagMap::Data& subSet = set[tmp.first];
			if(item.Negate)
			{
				if(subSet.ImplicitRemoved.count(item.Tag))
					subResult = true;
				else if(!subSet.Mutable.count(item.Tag) && !subSet.Implicit.count(item.Tag))
					subResult = true;
			}
			else
			{
				if(subSet.Mutable.count(item.Tag) || subSet.Implicit.count(item.Tag))
					if(!subSet.ImplicitRemoved.count(item.Tag))
						subResult = true;
			}
			if(subResult)
			{
				if(nextIsOr)
				{
					if(!lastWasTrue)
						score += 1;
				}
				else
					score += 1;
			}
			else
			{
				if(nextIsOr)
				{
					if(!lastWasTrue)
						score -= 1;
				}
				else if(item.Link != TagExpression::Or)
					score -= 1;
			}
			
			nextIsOr = (item.Link == TagExpression::Or);
			lastWasTrue = subResult;
		}
	}

	return score;
}



