module.exports = [
  {
    "type": "heading",
    "defaultValue": "Fit Face"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "toggle",
        "messageKey": "BT_VIBE",
        "label": "Vibrate on bluetooth disconnect",
        "defaultValue": false
      }      
    ]
  },
  {
    "type": "section",
    "capabilities": ["HEALTH"],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Health Settings"
      },
      {
        "type": "slider",
        "messageKey": "STEPGOAL",
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
        "messageKey": "CIRCLE_ROUNDED",
        "label": "Rounded ends on step circle",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "STEP_AVG",
        "label": "Show line indicating average steps",
        "defaultValue": false
      },
      {
        "type": "select",
        "messageKey": "STEP_AVG_SCOPE",
        "label": "Step average scope",
        "defaultValue": "daily",
        "options": [
          {
            "label": "Daily",
            "value": "daily"
          },
          {
            "label": "Weekend/Weekday",
            "value": "weekend"
          },
          {
            "label": "Day of the Week",
            "value": "weekly"
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
        "defaultValue": "Weather Settings"
      },        
      {
        "type": "input",
        "messageKey": "LOCATION",
        "label": "Location",
        "description": "Leave blank to use your phone's GPS services.",
        "defaultValue": ""
      },
      {
        "type": "select",
        "messageKey": "TEMP_UNITS",
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
          "messageKey": "COLOR_BG",
          "defaultValue": "0000FF",
          "label": "Background Color",
          "sunlight": true,
          "allowGray": true
        },
        {
          "type": "color",
          "messageKey": "COLOR_CIRCLE_PRIMARY",
          "defaultValue": "FFAA00",
          "label": "Primary Circle Color",
          "sunlight": true,
          "allowGray": true
        },
        {
          "type": "color",
          "messageKey": "COLOR_CIRCLE_SECONDARY",
          "defaultValue": "AAAAAA",
          "label": "Secondary Circle Color",
          "sunlight": true,
          "capabilities": ["HEALTH"],
          "allowGray": true
        },
        {
          "type": "color",
          "messageKey": "COLOR_TEXT_PRIMARY",
          "defaultValue": "FFFFFF",
          "label": "Primary Text Color",
          "sunlight": true
        },
        {
          "type": "color",
          "messageKey": "COLOR_TEXT_SECONDARY",
          "defaultValue": "FFAAAA",
          "label": "Secondary Text Color",
          "sunlight": true
        },
        {
          "type": "color",
          "messageKey": "COLOR_AVG_LINE",
          "defaultValue": "FFFFAA",
          "label": "Step Average Line Color",
          "sunlight": true,
          "capabilities": ["HEALTH"],
          "allowGray": true
        }
      ]
    },      
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
];