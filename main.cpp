#include <iostream>
#include "TagRuleSet.h"
#include "Parsing/ConfigFile.h"
#include "PreciseClock.h"

#include <algorithm>

#include <cstdlib>
#include <ctime>

#include <iomanip>

using namespace std;

template<typename T>
void printContainer(const T& cont)
{
	for(auto Z : cont){ cout << '\"' << Z << '\"' << " "; };
};

TagRule* applyBestRule(const TagRuleSet& set, TagMap& tags, const TagMap& tagsToScoreBy)
{
	TagRuleSet possibleRules = set.findMatches(tags);
	if(possibleRules.numRules())
	{
		possibleRules.sortByPriority();
		//possibleRules.sortByScore(tagsToScoreBy);
		std::vector<TagRule*> toExecute = possibleRules[0]->getMatchingSubrules(tags);
		if((*possibleRules[0])["PlayerAction"].empty() == false)
		{
			cout << setw(20) << left << (*possibleRules[0])["PlayerAction"] << " -> ";
		}
		for(auto rule : toExecute)
		{
			rule->apply(tags);
			//if((*rule)["Text"].empty() == false)
			//	cout << (*rule)["Text"] << " ";
		}
		//cout << endl;
		return possibleRules[0];
	}
	return nullptr;
}

std::vector<std::string> getPossibleActions(const TagRuleSet& actions, const TagMap& tags)
{
	std::vector<std::string> possibleActions;
	for(TagRule* rule : actions.findMatches(tags).Rules)
	{
		std::vector<std::string>& temp = rule->ListProperties["PlayerAction"];
		possibleActions.insert(possibleActions.end(), temp.begin(), temp.end());
	}
	return possibleActions;
}

int main(int argc, char **argv)
{
	ConfigFile file;
	file.loadFromFile("./Data/PerformanceTest.xini");
	
	srand(time(NULL));
	
	PreciseClock timer;
	timer.start();
	{
		TagRegistry reg;
		TagRuleSet rules;
		TagRuleSet actions;
		TagRuleSet events;
		TagMap tags;
		TagMap oldTags;
		tags["Global"].Mutable.insert("Start");
		
			rules.loadFromConfig(file, "Rule"  , &reg);
		actions.loadFromConfig(file, "Action", &reg);
			events.loadFromConfig(file, "Event" , &reg);
		
		cout << "Time needed for loading " << timer.elapsed() << "s" << endl;
		timer.start();
		
		int i = 0;
		while(!(tags["Global"].Mutable.count("FinalStop")))
		{
			oldTags = tags;
			TagMap deltaTags = tags.difference(oldTags);
			applyBestRule(events, tags, deltaTags);
			i++;
			
			tags.resetImplications();
			rules.findAndApplyRecursive(tags);
			
			//applyBestRule(actions, tags, deltaTags);
			
			tags.resetImplications();
			rules.findAndApplyRecursive(tags);
		}
		cout << "Executed " << i << " turns" << endl;
		cout << "Time needed for execution " << timer.elapsed() << "s" << endl;
		return 0;
	}
}
