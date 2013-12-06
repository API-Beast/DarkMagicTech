#include <iostream>
#include "TagRuleSet.h"
#include "Parsing/ConfigFile.h"

#include <algorithm>

#include <cstdlib>
#include <ctime>

using namespace std;

template<typename T>
void printContainer(const T& cont)
{
	for(auto Z : cont){ cout << Z << " "; };
};

TagRule* applyBestRule(const TagRuleSet& set, TagMap& tags, const TagMap& tagsToScoreBy)
{
	TagRuleSet possibleRules = set.findMatches(tags);
	if(possibleRules.numRules())
	{
		//possibleRules.sortByPriority();
		possibleRules.sortByScore(tagsToScoreBy);
		std::vector<TagRule*> toExecute = possibleRules[0]->getMatchingSubrules(tags);
		for(auto rule : toExecute)
		{
			rule->apply(tags);
			if((*rule)["Text"].empty() == false)
				cout << (*rule)["Text"] << " ";
		}
		cout << endl;
		return possibleRules[0];
	}
	return nullptr;
}

int main(int argc, char **argv)
{
	ConfigFile file;
	file.loadFromFile("./Data/DarkMagicStart.xini");
	
	srand(time(NULL));
	
	TagRegistry reg;
	TagRuleSet rules;
	TagRuleSet actions;
	TagRuleSet events;
	TagMap tags;
	TagMap oldTags;
	tags["Situation"].Mutable.insert("Start");
	
	   rules.loadFromConfig(file, "Rule"  , &reg);
	 actions.loadFromConfig(file, "Action", &reg);
	  events.loadFromConfig(file, "Event" , &reg);
	
	for(int i=0; i<10; i++)
	{
		tags.resetImplications();
		rules.findAndApplyRecursive(tags);
		oldTags = tags;
		TagMap deltaTags = tags.difference(oldTags);
		applyBestRule(events, tags, deltaTags);
		applyBestRule(actions, tags, deltaTags);
	}
	return 0;
}
