(function() {
   var elem = $('#tickMarkId'),
   enumMark = $('#tmId'),
   setActif = function (){
      if (elem[0].checked){
         enumMark.css('display', 'block');
      }else{
         enumMark.css('display', 'none');
      }
   };
   elem.on('click', function(e){
      setActif();
   });
   loadOptions();
   submitHandler();  
   setActif();
})();

function submitHandler() {
   var $submitButton = $('#submitButton');
   $submitButton.on('click', function() {
      var return_to = getQueryParam('return_to', 'pebblejs://close#');
      document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
   });
}

function loadOptions() {
   var $tickMarkId = $('#tickMarkId');
   var $numberMark = $('#numberMark');
   var $dayFrameId = $('#dayFrameId');
   
   if (localStorage.tickMark) {
      $tickMarkId[0].checked = localStorage.tickMark === 'true';
   }
   if (localStorage.numberMark) {
      $numberMark[0].value = localStorage.numberMark;
   }
   if (localStorage.frameDay) {
      $dayFrameId[0].checked = localStorage.frameDay === 'true';
   }
   
}

function getAndStoreConfigData() {
   var $tickMarkId = $('#tickMarkId');
   var $numberMark = $('#numberMark');
   var $dayFrameId = $('#dayFrameId');
   
   var options = {
      tickMark: $tickMarkId[0].checked,
      numberMark: parseInt($numberMark[0].value),
      frameDay: $dayFrameId[0].checked
   };
   
   localStorage.tickMark = options.tickMark;
   localStorage.numberMark = options.numberMark;
   localStorage.frameDay = options.frameDay;
   
   return options;
}

function getQueryParam(variable, defaultValue) {
   var query = location.search.substring(1);
   var vars = query.split('&');
   for (var i = 0; i < vars.length; i++) {
      var pair = vars[i].split('=');
      if (pair[0] === variable) {
         return decodeURIComponent(pair[1]);
      }
   }
   return defaultValue || false;
}
