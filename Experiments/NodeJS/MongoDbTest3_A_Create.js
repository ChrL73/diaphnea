var mongoose = require("mongoose");
var aSync = require("async");

var db = mongoose.connect("mongodb://localhost/testDb");

var n1 = 10;

var questionSchema = mongoose.Schema({ index: Number, text: String });
var categorySchema = mongoose.Schema({ index: Number, name: String, questions: [questionSchema] });
var testSchema;
var TestModel;

clearArray();

function clearArray()
{
   console.log("n=" + n1 + " clearArray");
   
   levelSchema = mongoose.Schema({ name: String, categories: [categorySchema] }, { collection: "levels" + n1.toString() });
   LevelModel = mongoose.model("Level" + n1.toString(), levelSchema);
   
   LevelModel.remove({}, fillArray);
}

function fillArray(err)
{
   if (err)
   {
      console.log(err);
   }
   else
   {
      console.log("n=" + n1 + " fillArray");

      var iArray = getIndexArray(3);
      
      aSync.each(iArray, function(i, callback)
      {
         var level = new LevelModel();
         level.name = "Level" + i.toString();

         var j;
         for (j = 0; j < 6; ++j)
         {
            var category = { index: j, name: "Category" + j.toString(), questions: [] };
            
            var k;
            for (k = 0; k < (j + 1) * n1; ++k)
            {
               var question = { index: k, text: "Question" + k.toString() };
               category.questions.push(question);
            }
            
            level.categories.push(category);
         }

         level.save(function() { callback(false); });
      },
      function(err)
      {
         if (err)
         {
            console.log("err = " + err);
         }
         else
         {
            if (n1 < 1500)
            {
               n1 *= 2;
               clearArray();
            }
            else
            {
               console.log("End");
            }
         }
      });
   }
}

function getIndexArray(n)
{
   var i, array = [];
   for (i = 0; i < n; ++i) array.push(i);
   return array;
}