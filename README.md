# ESP32_AWS
Simple example to connect ESP32 microcontroller to AWS IoT Core. In this example, the ESP32 device receives a message from AWS and operates a Servo Motor.
The connection between ESP32 and AWS is achieved using MQTT protocol.
To develop this example, I followed this pretty cool tutorial,https://how2electronics.com/connecting-esp32-to-amazon-aws-iot-core-using-mqtt/, that shows how to create a "thing" in AWS IoT Core and interact with it. Basically these are the steps:
  - Create an AWS Account
  - Create a "thing" in AWS IoT Core Dashboard
  - Generate Device Certificate
  - Create and Attach Policy
    - If you want to send messages from AWS and receive messages from ESP32, you need to configure the following policy actions:
      - iot:Connect
      - iot:Publish
      - iot:Receive
      - iot:Subscribe
  - Download Certificates and Keys
    - Device Certificate
    - Private Key
    - Root CA Certificates
      - After downloading the files, you have to open it then copy the respective contents and paste in the respective places in the "secrets_format.h" file.
      - Also in the "secrets_format.h" you have to configure your "WiFi SSID","WiFi Password","AWS_IOT_ENDPOINT(given by AWS)" and "THINGNAME(configured by you in AWS platafform)"

Libraries:
  - PubSub: https://github.com/knolleary/pubsubclient
  - Json: https://github.com/bblanchon/ArduinoJson
  - Servo Moto: https://github.com/arduino-libraries/Servo
  
