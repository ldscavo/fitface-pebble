module.exports = [
  {
    "type": "section",
    items: [
      { 
        "type": "heading", 
        "defaultValue": "Fit Face" 
      }, 
      {
        "type": "slider",
        "appKey": "STEPGOAL",
        "label": "Daily Step Goal",
        "defaultValue": 5000,
        "min": 2000,
        "max": 10000,
        "step": 1000,
        "attributes": {
          "required": "required",
          "type": "number"
        }        
      },
      {
        "type": "select",
        "appKey": "TEMP_UNITS",
        "label": "Tempurture Units",
        "defaultValue": "F",
        "options": [
          {
            "label": "Fahrenheit",
            "value": "F"
          },
          {
            "label": "Celsius",
            "value": "C"
          }
        ]
      },
      {
        "type": "submit",
        "defaultValue": "Save Settings"
      }
    ]
  }
];