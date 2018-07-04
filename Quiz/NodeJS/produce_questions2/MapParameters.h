/* --------------------------------------------------------------------
 *
 * Copyright (C) 2018
 *
 * This file is part of the Diaphnea project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------- */

#pragma once

#include "QuizData.h"

namespace produce_questions
{
    class MapSubParameters;

    class MapParameters
    {
    private:
        // Int
        int _framingLevel;

        // MapSubParameters
        int _questionParameters;

        // MapSubParameters
        int _answerParameters;

        // MapSubParameters
        int _wrongChoiceParameters;

        // Bool
        int _allAnswersSelectionMode;

    public:
        int getFramingLevel(void) const { return _framingLevel; }
        const MapSubParameters *getQuestionParameters(void) const { return reinterpret_cast<MapSubParameters *>(mapSubParameterss + _questionParameters); }
        const MapSubParameters *getAnswerParameters(void) const { return reinterpret_cast<MapSubParameters *>(mapSubParameterss + _answerParameters); }
        const MapSubParameters *getWrongChoiceParameters(void) const { return reinterpret_cast<MapSubParameters *>(mapSubParameterss + _wrongChoiceParameters); }
        bool getAllAnswersSelectionMode(void) const { return _allAnswersSelectionMode != 0; }
    };
}
