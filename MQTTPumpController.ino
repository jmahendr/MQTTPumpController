#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11
#define DHTPIN 2 

#define FULL_PIN A0
#define THREEFOURTH_PIN A1
#define HALF_PIN A2
#define QUARTER_PIN A3
#define INFLOW_PIN A4
#define SUMPEMPTY_PIN A5
#define MOTOR_PIN 6
#define FAULT_PIN 7

//temp variables
float t, prevT = 0.0;
float h, prevH = 0.0;

//tank level
String levelMessage, prevLevelMessage = "";

// Update these with values suitable for your network.
byte mac[] = { 0xDE, 0xAD, 0x09, 0xEF, 0xCE, 0x0F };
byte server[] = { 192, 168, 0, 90 }; //local Mosquitto broker

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
DHT dht(DHTPIN, DHTTYPE);


void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String payloadStr = String((char*) payload);
    if(payloadStr=="ON")
    {
       Serial.println("Turn ON Pump");
       digitalWrite(MOTOR_PIN, HIGH);
    }
    if(payloadStr=="OFF")
    {
       Serial.println("Turn OFF Pump");
       digitalWrite(MOTOR_PIN, LOW);
     }
}


void setup()
{
  Serial.begin(9600);
  
  pinMode(FULL_PIN,INPUT);
  pinMode(THREEFOURTH_PIN,INPUT);
  pinMode(HALF_PIN,INPUT);
  pinMode(QUARTER_PIN,INPUT);
  pinMode(INFLOW_PIN,INPUT);
  pinMode(SUMPEMPTY_PIN,INPUT);

  pinMode(MOTOR_PIN,OUTPUT);
  pinMode(FAULT_PIN, OUTPUT);
  digitalWrite(FAULT_PIN, LOW);

  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  //pinMode(10, OUTPUT);
  //digitalWrite(10, HIGH);
  
  Serial.println("Initiating Ethernet Connection");
  //Serial.println(Ethernet.begin(mac));
  if(Ethernet.begin(mac))
  {
    Serial.println("Ethernet connection successful");
    Serial.print("IP Address assigned via DHCP Server : ");
    Serial.println(Ethernet.localIP());
  }
  else
  {
    Serial.println("Ethernet connection failed. Going into a endless loop...");
    for(;;){}
  }
 
  if(ethClient.connected())
    Serial.println("Ehternet Client is connected.");
  else
    Serial.println("Ethernet Client is disconnected.");
    
  connectToBroker();
  
  if(ethClient.connected())
    Serial.println("Ehternet Client is connected.");
  else
    Serial.println("Ethernet Client is disconnected.");
  
}
  
 void connectToBroker()
 {
  Serial.println("Initiating connection to Broker.");
  if (client.connect("motorController")) {
    Serial.println("Connected to MQTT Broker");
    client.publish("/com/jmahendr/h1/utility/pump/controller","CONNECTED");
    client.subscribe("/com/jmahendr/h1/utility/pump/P1");
  }
  else
  {
    Serial.println("Could not connect to MBroker");
    digitalWrite(FAULT_PIN, HIGH);
  }
}

void loop()
{
  client.loop();
  checkLevel();
  readTempHumid();
  delay(10000);
}




void checkLevel()
{
  Serial.println("in checklevel");
  String levelMessage = "0";
  if (digitalRead(QUARTER_PIN))
    levelMessage = "25";
  
  if (digitalRead(HALF_PIN))
    levelMessage = "50";
    
  if (digitalRead(THREEFOURTH_PIN))
    levelMessage = "75";
    
  if (digitalRead(FULL_PIN))
    levelMessage = "100";

  publishMessage("/com/jmahendr/h1/utility/OHT", levelMessage);

}


void readTempHumid() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  String temperature = String(t,2);
  publishMessage("/com/jmahendr/h1/T1",temperature);
  
  String humidity = String(h,2);
  publishMessage("/com/jmahendr/h1/H1", humidity);
}






void publishMessage(String topic, String message)
{
  char messageArray[10];
  message.toCharArray(messageArray, sizeof(messageArray));
  
  char topicArray[50];
  topic.toCharArray(topicArray, sizeof(topicArray));
  
  client.publish(topicArray, messageArray);
}
