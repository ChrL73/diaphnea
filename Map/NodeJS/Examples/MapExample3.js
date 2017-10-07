$(function()
{            
   $('#connectionForm').submit(function(e)
   {
      e.preventDefault();
      var url = 'http://' + $('#serverInput').val() + ':' + $('#portInput').val();
      mapServerInterface.createNewConnection(url, onConnected, onError);
   });

   function onError(error)
   {
      console.error(error);
      $('#errorSpan').text('Error: ' + error.message);
      $('#errorModal').modal('show');
   }

   function onConnected(mapServerConnection)
   {
      $('#mapForm').css('display', 'block');
      $('#connectionForm').css('display', 'none');

      var mapIds = mapServerConnection.getMapIds();
      mapIds.forEach(function(mapId)
      {
         var mapName = mapId;
         while(mapName.charAt(0) === '_') mapName = mapName.substr(1);

         $('#mapSelect').append('<option value="' + mapId + '">' + mapName + '</option>');
      });

      onMapChange();
      $('#mapSelect').change(onMapChange);
      
      function onMapChange()
      {
         loadMap($('#mapSelect').val());
      }
      
      function loadMap(mapId)
      {
         var canvasId = 'canvas';
         mapServerConnection.loadMap(mapId, canvasId, onMapLoaded);

         function onMapLoaded(map)
         {
            var elements;
            
            $('#languageSelect').empty();
            var languages = map.getLanguages();
            languages.forEach(function(language)
            {
               $('#languageSelect').append('<option value="' + language.id + '">' + language.name + '</option>');          
            });
            
            $('#languageSelect').change(function()
            {
               updateCategories(elements);
            });
            
            var elementIds = map.getElementIds();
            map.loadElements(elementIds, function(elementArray)
            {
               elements = elementArray;
               updateCategories(elementArray);
            });
                             
            function updateCategories(elementArray)
            {
               $('#categoryDiv').empty();
               
               var categories = map.getCategories();
               var categoryArray = [];
               categories.forEach(function(category)
               {
                  categoryArray.push([]);
               });
               
               elementArray.forEach(function(element)
               {
                  categoryArray[element.getCategoryIndex()].push(element);
               });
               
               var html = '<div class="panel panel-default" style="margin-bottom:0px;">'
                  + '<div class="panel-heading">'
                  + '<input type="checkbox"><span> All (' + elementArray.length + ')</span>'
                  + '<span class="glyphicon glyphicon-triangle-top pull-right" id="collpaseAllIcon"'
                  + 'data-toggle="collapse" data-target="#collapseAll" aria-controls="collapseAll"></span>'
                  + '</div></div>'
                  + '<div id="collapseAll" class="collpase"></div>';
               $('#categoryDiv').append(html);
               
               $('#collapseAll').on('shown.bs.collapse', function()
               {
                  $('#collpaseAllIcon').removeClass('glyphicon-triangle-bottom').addClass('glyphicon-triangle-top');
               });
               $('#collapseAll').on('hidden.bs.collapse', function()
               {
                  $('#collpaseAllIcon').removeClass('glyphicon-triangle-top').addClass('glyphicon-triangle-bottom');
               });
               
               var languageId = $('#languageSelect').val();
               categories.forEach(function(category, i)
               {
                  if (categoryArray[i].length > 0)
                  {
                     categoryArray[i].sort(function(a, b) { return a.getName(languageId).localeCompare(b.getName(languageId)); });
                     addCategory(category[languageId], i, categoryArray[i], languageId);
                  }
               });
               
               $('#collapseAll').collapse();
            }
         }
      }
   }
   
   function addCategory(categoryName, categoryIndex, elementArray, languageId)
   {
      var html = '<div class="panel panel-default" style="margin-bottom:0px;">'
         + '<div class="panel-heading">'
         + '<input type="checkbox"><span> ' + categoryName + ' (' + elementArray.length + ')</span>'
         + '<span class="glyphicon glyphicon-triangle-bottom pull-right" id="collpaseIcon_' + categoryIndex + '"'
         + 'data-toggle="collapse" data-target="#collapse_' + categoryIndex + '" aria-controls="collapse_' + categoryIndex + '"></span>'
         + '</div>'
         + '<div class="collapse" id="collapse_' + categoryIndex + '">'
         + '<div class="panel-body">';
      
      elementArray.forEach(function(element)
      {
         html += '<p><input type="checkbox" value="' + element.getId() + '" style="margin-left:12px;"><span> ' + element.getName(languageId)+ '</span></p>';      
      });
      
      html += '</div></div></div>'; 
      
      $('#collapseAll').append(html);
      
      $('#collapse_' + categoryIndex).on('shown.bs.collapse', function(e)
      {
         $('#collpaseIcon_' + categoryIndex).removeClass('glyphicon-triangle-bottom').addClass('glyphicon-triangle-top');
         e.stopImmediatePropagation();
      });
      $('#collapse_' + categoryIndex).on('hidden.bs.collapse', function(e)
      {
         $('#collpaseIcon_' + categoryIndex).removeClass('glyphicon-triangle-top').addClass('glyphicon-triangle-bottom');
         e.stopImmediatePropagation();
      });
   }
   
   resizeCanvas();
         
   window.onresize = function()
   {
      resizeCanvas();
      //map.redraw();
   }

   function resizeCanvas()
   {
      var h = (window.innerHeight - 34).toString() + 'px';
      $('#column1').height((window.innerHeight - 34).toString() + 'px');
      $('#column2').height((window.innerHeight - 8).toString() + 'px');
   }
   
});
