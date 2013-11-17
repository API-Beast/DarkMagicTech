#include <iostream>
#include "TagRuleSet.h"
#include "ConfigFile.h"

#include <algorithm>

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
		possibleRules.sortByPriority();
		//possibleRules.sortByScore(tagsToScoreBy);
		possibleRules[0]->apply(tags);
		possibleRules[0]->Priority--;
		return possibleRules[0];
	}
	return nullptr;
}

int main(int argc, char **argv)
{
	ConfigFile file("./Data/DarkMagicStart.ini");
	
	TagRuleSet rules;
	TagRuleSet actions;
	TagRuleSet triggers;
	TagMap tags;
	TagMap oldTags;
	tags["Situation"].Mutable.insert("Start");
	
	   rules.loadFromConfig(file, "Rule"   );
	 actions.loadFromConfig(file, "Action" );
	triggers.loadFromConfig(file, "Trigger");
	
	for(int i=0; i<10; i++)
	{
		/*cout << "Explicit: ";
		printContainer(tags["Situation"].Mutable);
		cout << " Implicit: ";
		printContainer(tags["Situation"].Implicit);
		cout << " Implicit-Removal: ";
		printContainer(tags["Situation"].ImplicitRemoved);
		cout << endl;*/
		

		oldTags = tags;
		TagMap deltaTags = tags.difference(oldTags);
		if(TagRule* appliedAction = applyBestRule(actions, tags, deltaTags))
		{
			if(!(*appliedAction)["Text"].empty())
				cout << (*appliedAction)["Text"] << endl;
			TagSet trigs = tags.CurrentTriggers;
			for(const std::string& trigger : trigs)
			{
				tags.CurrentTriggers.clear();
				tags.CurrentTriggers.insert(trigger);
				applyBestRule(triggers, tags, deltaTags);
			}
			tags.CurrentTriggers.clear();
			TagRule* appliedResult = applyBestRule(appliedAction->SubRules, tags, deltaTags);
			if(appliedResult)
				cout << (*appliedResult)["Text"] << endl;
		}
		else
			cout << "No possible action." << endl;
		tags.resetImplications();
		rules.findAndApplyRecursive(tags);
	}
	return 0;
}
