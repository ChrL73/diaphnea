$(function()
{
   var socket = io.connect();
   
   if (unknwon) $('#cnxMessage1').show();
   if (error) $('#cnxMessage2').show();
   
   $('#siteLanguageSelect').change(function()
   {
      var languageId = $(this).val();
      if (!userLogged) document.cookie = 'siteLanguageId=' + languageId + getCookieExpires(180);
      $('#siteLanguageForm').trigger('submit');
   });
   
   $('#questionnaireSelect').change(emitLevelChoice);
   $('#questionnaireLanguageSelect').change(emitLevelChoice);
   $('#levelSelect').change(emitLevelChoice);
   
   function emitLevelChoice()
   {
      var questionnaireId = $('#questionnaireSelect').val();
      var questionnaireLanguageId = $('#questionnaireLanguageSelect').val();
      var levelId = $('#levelSelect').val();
      
      if (!userLogged)
      {
         var expires = getCookieExpires(180);
         document.cookie = 'questionnaireId=' + questionnaireId + expires;
         document.cookie = 'questionnaireLanguageId=' + questionnaireLanguageId + expires;
         document.cookie = 'levelId=' + levelId + expires;
      }
      
      socket.emit('levelChoice', { questionnaireId: questionnaireId, questionnaireLanguageId: questionnaireLanguageId, levelId: levelId }); 
   }
   
   socket.on('updateSelects', function(data)
   {
      $('#questionnaireSelect').find('option').remove(); 
      data.questionnaireList.forEach(function(questionnaire)
      {
         $('#questionnaireSelect').append('<option value="' + questionnaire.id + '">' + questionnaire.name + '</option>');
      });
      $('#questionnaireSelect').val(data.questionnaireId);
      
      if (data.showQuestionnaireLanguageSelect)
      {
         $('.questionnaireLanguageSelection').css('display', 'inline');
      }
      else
      {
         $('.questionnaireLanguageSelection').css('display', 'none');
      }
      
      $('#questionnaireLanguageSelect').find('option').remove();
      data.questionnaireLanguageList.forEach(function(language)
      {
         $('#questionnaireLanguageSelect').append('<option value="' + language.id + '">' + language.name + '</option>');
      });
      $('#questionnaireLanguageSelect').val(data.questionnaireLanguageId);
      
      $('#levelSelect').find('option').remove();
      data.levelList.forEach(function(level)
      {
         $('#levelSelect').append('<option value="' + level.id + '">' + level.name + '</option>');
      });
      $('#levelSelect').val(data.levelId);
   });
});
