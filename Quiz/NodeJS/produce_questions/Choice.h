#pragma once

#include "ProximityCriterionTypeEnum.h"

#include <string>
#include <vector>

namespace produce_questions
{
    class Choice
    {
    private:
        const std::string _choiceText;
        const std::string _comment;
        const double _doubleCriterionValue;
        const std::string _stringCriterionValue;

        static std::string _emptyString;

    public:
        Choice(const std::string& choiceText, const std::string& comment, const std::vector<double>& doubleCriterionVector, const std::vector<std::string>& stringCriterionVector);

        const std::string& getChoiceText(void) const { return _choiceText; }
        const std::string& getComment(void) const { return _comment; }
        double getDoubleCriterionValue(void) const { return _doubleCriterionValue; }
        const std::string& getStringCriterionValue(void) const { return _stringCriterionValue; }
    };
}
