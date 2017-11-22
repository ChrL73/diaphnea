﻿using MongoDB.Bson;
using MongoDB.Driver;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace QuestionInstantiation
{
    abstract class Category
    {
        private readonly Int32 _weightIndex;
        private readonly string _questionNameInLog;
        private readonly QuizData _quizData;
        private readonly XmlMapParameters _mapParameters;

        protected Category(int weightIndex, string questionNameInLog, QuizData quizData, XmlMapParameters mapParameters)
        {
            _weightIndex = weightIndex;
            _questionNameInLog = questionNameInLog;
            _quizData = quizData;
            _mapParameters = mapParameters;
        }

        protected Int32 WeightIndex
        {
            get { return _weightIndex; }
        }

        protected string QuestionNameInLog
        {
            get { return _questionNameInLog; }
        }

        protected QuizData QuizData
        {
            get { return _quizData; }
        }

        protected BsonDocument getMapParameterBsonDocument()
        {
            if (_mapParameters == null) throw new NotImplementedException();

            BsonArray categoryArray = new BsonArray();
            foreach (XmlMapCategory mapCategory in _mapParameters.category)
            {
                categoryArray.Add(mapCategory.id);
            }

            BsonDocument mapParametersDocument = new BsonDocument()
            {
                { "framing_level", _mapParameters.framingLevel },
                { "answer_draw_depth", _mapParameters.answerDrawDepth },
                { "wrong_choice_draw_depth", _mapParameters.wrongChoiceDrawDepth },
                { "category_selection_mode", _mapParameters.categorySelectionMode.ToString() },
                { "categories", categoryArray}
            };

            return mapParametersDocument;
        }

        abstract internal BsonDocument getBsonDocument(IMongoDatabase database, string questionnaireId);
    }
}
