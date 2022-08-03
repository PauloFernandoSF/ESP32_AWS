/*
 Name:		    ESP32_AWS.ino
 Created:	    02/08/2022
 Author:	    Paulo Fernando <pfsf07@gmail.com>
 Description: MCU(Esp32 in this case) that connects with AWS(Amazon's home assistant), using "Servo" library. This example turnon/turnoff a Servo Motor when a message is received from AWS using MQTT protocol.
 Connections:
	VCC(ESP32)    			                -> Powered by USB 
	VCC(Servo Motor)                    -> 5V external source
  Output pin to control Servo Motor   -> GPIO 04
  Source GND + USB GND connected 	 
*/
//File in which all certificates and WiFi configurations are stored
#include "secrets.h"
#include <WiFiClientSecure.h>
//MQTT library, to handle MQTT connections
#include <PubSubClient.h>
//Json library to handle messages fo MQTT protocol
#include <ArduinoJson.h>
#include "WiFi.h"
//Servo library to control MG996R Servo Motor
#include <Servo.h>
 //Topic name for ESP32 publish messages
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
//Topic name for ESP32  receive AWS messages
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
//Handle for Servo Motor control task
TaskHandle_t handle_task_servo_control;
//Servo Motor pin
static const int servoPin = 4;
//Flag to enable Servo Motor Operation
bool servo = false;
//Initialize WiFi Secure connection
WiFiClientSecure wifi_connection = WiFiClientSecure();
//Connection to start PubSub library  
PubSubClient     client(wifi_connection);
//Servo object
Servo            myservo;
//Function to connect to AWS 
void connectAWS(){
  //Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
  //Wait for connection
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  //Configure WiFiClientSecure to use the AWS IoT device credentials
  wifi_connection.setCACert(AWS_CERT_CA);
  wifi_connection.setCertificate(AWS_CERT_CRT);
  wifi_connection.setPrivateKey(AWS_CERT_PRIVATE);
  //Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  //Function do handle MQTT messages
  client.setCallback(messageHandler);
  Serial.println("Connecting to AWS IOT");
  //THINGNAME is the name declared in "secrets.h" file 
  while(!client.connect(THINGNAME)){
    Serial.print(".");
    delay(10);
  }
  //Verify connection
  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }
  //Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}
//Function to publish messages to AWS IoT device
void publishMessage(){
  //Json formated message
  //Buffer to store message tha will be published
  StaticJsonDocument<100> doc;
  doc["Servo Motor"] = "Operated";
  char jsonBuffer[100];
  serializeJson(doc, jsonBuffer);
  Serial.println(jsonBuffer); 
  Serial.println(AWS_IOT_PUBLISH_TOPIC); 
  //Publish
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
//Function to handle incoming messages
void messageHandler(char* topic, byte* payload, unsigned int length){
  Serial.print("incoming: ");
  Serial.println(topic);
  //Object to store playload
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
  //If "1" is received, move the Servo Motor
  if(message[0] == '1' && !servo){
    servo = true;
    //Message to indicate that the Servo Motor was operated
    publishMessage();
  }
}
//Function to create Servo Motor task
void task_start_servo_control(){
  xTaskCreatePinnedToCore(
      task_servo_control,         
      "task_servo_control",       
      1024,                     
      NULL,                      
      0,                         
      &handle_task_servo_control, 
      1);                        
}
//Servo Motor task
void task_servo_control(void *pvParameters){
    Serial.println("Iniciando Task");
    while(1){
      if(servo){
        move_servo();
        servo = false;
      }
      vTaskDelay(10);
    }
}
//Function do control Servo Motor
void move_servo(){
  Serial.println("Operando Servo");
  //Move Servo in 10 degrees steps - from 0 to 180
  for(int pos = 90; pos <= 180; pos += 5){ 
    myservo.write(pos);
    //100ms to reach the position
    vTaskDelay(100);                       
  }
  vTaskDelay(100);
  //Move Servo in 10 degrees steps - from 180 to 0
  for(int pos = 90; pos >= 0; pos -= 5){ 
    myservo.write(pos);              
    //100ms to reach the position
    vTaskDelay(100);
  }
  //Turn Servo off
  myservo.write(90);
}

void setup(){
  Serial.begin(115200);
  myservo.attach(servoPin);
  //Initialize Servo Control task
  task_start_servo_control();
  //Connect with AWS IoT Core
  connectAWS();
}
 
void loop(){
  //Keep MQTT connection
  client.loop();
  delay(100);
}