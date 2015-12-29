Pebble.addEventListener('showConfiguration', function() {
   var url = 'http://paris.memoris.xyz';
   Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
   var configData = JSON.parse(decodeURIComponent(e.response));
   
   if (configData) {
      Pebble.sendAppMessage({
         tickMark: configData.tickMark,
         numberMark: configData.numberMark,
         frameDay: configData.frameDay
      }, function() {
         console.log('Preferences data sent.');
      }, function() {
         console.log('Fail to send preferences data.');
      });
   }
});
