module.exports =[
  // Create a Section to put everything in
  {
    "type": "section",
    "items": [
      
      // Call the section "Configuration"
      {
        "type": "heading",
        "defaultValue": "Configuration"
      },
      
      // Add a Color Picker
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0x000000",
        "label": "Background Color",
        "sunlight": true,
        "allowGray": true
      },
      
      // Add a Radio Group to select the temperature unit
      {
      "type": "radiogroup",
        "messageKey": "TempUnit",
        "label": "Temperature Unit",
        "defaultValue": "K",
        "options": [
          {
            "label": "Kelvin",
            "value": "K"
          },
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
      
      // Add a Submit button to save the custom configuration
      {
        "type": "submit",
        "defaultValue": "Save"
      }
    ]
  }
];