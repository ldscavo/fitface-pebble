module.exports = [
  {
    "type": "heading",
    "defaultValue": "Fit Face"
  },
  {
    "type": "section",
    items: [      
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
        "type": "toggle",
        "appKey": "BT_VIBE",
        "label": "Vibrate on bluetooth disconnect",
        "defaultValue": false
      }    
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather Settings"
      },        
      {
        "type": "input",
        "appKey": "LOCATION",
        "label": "Location",
        "description": "Leave blank to use your phone's GPS services.",
        "defaultValue": ""
      },
      {
        "type": "select",
        "appKey": "TEMP_UNITS",
        "label": "Temperture Units",
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
      }
    ]
  },
    {
      "type": "section",
      "items": [
        { 
          "type": "heading", 
          "defaultValue": "Color Settings" 
        }, 
        {
          "type": "color",
          "appKey": "COLOR_BG",
          "defaultValue": "0000FF",
          "label": "Background Color",
          "sunlight": false
        },
        {
          "type": "color",
          "appKey": "COLOR_CIRCLE_PRIMARY",
          "defaultValue": "FFAA00",
          "label": "Primary Circle Color",
          "sunlight": false
        },
        {
          "type": "color",
          "appKey": "COLOR_CIRCLE_SECONDARY",
          "defaultValue": "AAAAAA",
          "label": "Secondary Circle Color",
          "sunlight": false
        },
        {
          "type": "color",
          "appKey": "COLOR_TEXT_PRIMARY",
          "defaultValue": "FFFFFF",
          "label": "Primary Text Color",
          "sunlight": false
        },
        {
          "type": "color",
          "appKey": "COLOR_TEXT_SECONDARY",
          "defaultValue": "FFAAAA",
          "label": "Secondary Text Color",
          "sunlight": false
        }
      ]
    },      
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
];