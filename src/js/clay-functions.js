module.exports = function(minified) {
  var clayConfig = this;
  
  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var platform = clayConfig.meta.activeWatchInfo.platform;
    if (platform === 'aplite') {
      clayConfig.getItemByAppKey('health-header').hide();
      clayConfig.getItemByAppKey('STEPGOAL').hide();
      clayConfig.getItemByAppKey('CIRCLE_ROUNDED').hide();
      clayConfig.getItemByAppKey('STEP_AVG').hide();
      clayConfig.getItemByAppKey('STEP_AVG_SCOPE').hide();
    }
  });
};