module.exports = function(minified) {
  var clayConfig = this;
  
  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var platform = clayConfig.meta.activeWatchInfo.platform;
    if (platform === 'aplite') {
      clayConfig.getItemByAppKey('STEPGOAL').hide();
    }
  });
};