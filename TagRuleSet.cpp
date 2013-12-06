/*
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *   0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include "TagRuleSet.h"
#include "Strings.h"
#include "Parsing/ConfigFile.h"

#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

TagRule& TagRegistry::registerRule(const TagRule& rule)
{
	Rules.push_back(rule);
	return Rules.back();
}

void TagRuleSet::loadFromConfig(ConfigFile& file, const std::string& cat, TagRegistry* reg)
{	
	for(ConfigFile::Object& action : file.Root.Children)
	{
		if(action.TypeHint != cat)
			continue;
		
		TagRule rule;
		rule.loadFromConfigObject(action, reg);
		Rules.push_back(&(reg->registerRule(rule)));
	}
}

void TagRule::loadFromConfigObject(ConfigFile::Object& obj, TagRegistry* reg)
{
	for(ConfigFile::Object& sub : obj.Children)
	{
		if(sub.TypeHint == "Rule")
		{
			TagRule newRule;
			newRule.loadFromConfigObject(sub, reg);
			SubRules.Rules.push_back(&(reg->registerRule(newRule)));
		}
		if(sub.TypeHint == "Result")
		{
			TagRule newRule;
			newRule.loadFromConfigObject(sub, reg);
			Results.Rules.push_back(&(reg->registerRule(newRule)));
		}
		if(sub.TypeHint == "Needs")
		{
			for(auto& target : sub.Values)
			{
				if(target.Value.isList())
					Expressions[target.Key].Require.loadList(target.Value);
				else
					Expressions[target.Key].Require.load(target.Value);
			}
		}
		if(sub.TypeHint == "Changes")
		{
			for(auto& target : sub.Values)
			{
				if(target.Value.isList())
					Expressions[target.Key].Apply.loadList(target.Value);
				else
					Expressions[target.Key].Apply.load(target.Value);
			}
		}
		if(sub.TypeHint == "Implies")
		{
			for(auto& target : sub.Values)
			{
				if(target.Value.isList())
					Expressions[target.Key].Apply.loadList(target.Value);
				else
					Expressions[target.Key].Apply.load(target.Value);
			}
		}
	}
	for(ConfigFile::KeyValue& val : obj.Values)
	{
		Properties[val.Key] = val.Value.asString();
		ListProperties[val.Key] = val.Value.asList();
	}
	
	Priority      = FromString(obj["Priority"], 0);
	ResultOrder   = FromString(obj["ResultOrder"], 0);
	MaxExecutions = FromString(obj["MaxExecutions"], 0);
}

void TagRuleSet::sortByPriority()
{
	std::sort(Rules.begin(), Rules.end(), [](TagRule* rule, TagRule* ruleB){ return rule->Priority > ruleB->Priority;});
}

void TagRuleSet::sortByScore(const TagMap& set)
{
	std::random_shuffle(Rules.begin(), Rules.end());
}

TagRuleSet TagRuleSet::findMatches(const TagMap& set) const
{
	TagRuleSet result;
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

void TagExpression::loadList(const vector< string >& list)
{
	for(const std::string& item : list)
	{
		if(item.front() == '!')
			this->Items.emplace_back(item.substr(1), true);
		else
			this->Items.emplace_back(item);
	}
}

bool TagRule::matches(const TagMap& set)
{
	if(MaxExecutions)
		if(Executions >= MaxExecutions)
			return false;
	
	if(!ScoreNeededForMatch)
	{
		for(auto& reqSets : this->Expressions)
		{
			TagExpression& subSet = reqSets.second.Require;
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

bool TagRule::apply(TagMap& set)
{
	bool changesMade = false;
	Executions++;
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

std::vector< TagRule* > TagRule::getMatchingSubrules(const TagMap& set)
{
	std::vector< TagRule* > temp;
	std::vector< TagRule* > result;
	
	temp.reserve(3);
	result.reserve(6);
	
	auto bestMatchingResults = Results.findMatches(set);
	//bestMatchingResults.sortByScore(set);
	bestMatchingResults.sortByPriority();
	
	auto matchingSubrules = SubRules.findMatches(set);
	
	// Default order
	// First top level rule
	temp.push_back(this);
	
	// Then subrules
	for(auto& rule : matchingSubrules.Rules)
		temp.push_back(rule);
	
	// Then the result
	if(bestMatchingResults.numRules())
		temp.push_back(bestMatchingResults[0]);
	
	// However the order can be changed by the ResultOrder attribute
	auto sortByResultOrder = [](TagRule* ruleA, TagRule* ruleB){ return ruleA->ResultOrder > ruleB->ResultOrder; };
	std::sort_heap(temp.begin(), temp.end(), sortByResultOrder);
	
	// Add sub subrules
	for(auto rule : temp)
	{
		if(rule == this)
		{
			result.push_back(rule);
			continue;
		}
		auto subSubRules = rule->getMatchingSubrules(set);
		result.insert(result.end(), subSubRules.begin(), subSubRules.end());
	}
	
	return result;
}



