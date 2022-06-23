#include <Arduino.h>
#include <ESP8266WiFi.h>  // import WiFi library
#include <PubSubClient.h> // Import mqtt library
#include <SimpleDHT.h>

const char *ssid = "ZTE_2.4G_x3Pjbs";
const char *password = "bayarsek";
const char *mqtt_server = "138.2.59.106"; // this is our broker

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define sensorSoilMoisture A0
#define relayModule D5
SimpleDHT11 dht11(D1);

int value = 0;


int nilaiSensorSoil;


long now = millis();
long lastMeasure = 0;
void setup_wifi(){
  // setting up wifi connection

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);        // select "wifi station" mode (client mode)
  WiFi.begin(ssid, password); // initiate connection

  while (WiFi.status() != WL_CONNECTED)
  { // wait for connection (retry)
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println(""); // when connected
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char *topic, byte *payload, unsigned int length)
{ // callback funtion (trigger)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  { // print received character array in one line
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();
  if (String(topic) == "room/pompa")
  {
  if (messageTemp == "Siram")
    {
      digitalWrite(relayModule, LOW);
      Serial.println("On");
      delay(10000);
      digitalWrite(relayModule, HIGH);
      Serial.println("Off");
      client.publish("pompa/siramOff", "off");
    }
  }  else if (String(topic) == "room/pompaManual")
  {
    if (messageTemp == "on")
    {
      digitalWrite(relayModule, LOW);
      Serial.println("On");
    }
    else
    {
      digitalWrite(relayModule, HIGH);
      Serial.println("Off");
    }
  }
}


void reconnect()
{ // connecting to MQTT broker
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("room/pompa");
      client.subscribe("room/pompaManual");

    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
 Serial.begin(115200);
 setup_wifi();
 pinMode(relayModule, OUTPUT);
 digitalWrite(relayModule, HIGH);
 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);
}

void loop() {
        if (!client.connected())
      {
        reconnect();
      }
      if (!client.loop())
      {
        client.connect("ESP8266Client");
      }
      now = millis();
      if (now - lastMeasure > 5000)
      {
        lastMeasure = now;
         ++value;
        int err = SimpleDHTErrSuccess;
        nilaiSensorSoil = analogRead(sensorSoilMoisture);
        nilaiSensorSoil = map(nilaiSensorSoil, 1023, 165, 0, 100);
        byte temperature = 0;
        byte humidity = 0;
        if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
        {
          Serial.print("Pembacaan DHT11 gagal, err=");
          Serial.println(err);
          delay(1000);
          return;
        }
        static char temperatureTemp[7];
        static char humString[8];
        static char soilMoisture[9];

        dtostrf(humidity, 1, 2, humString);
        dtostrf(temperature, 4, 2, temperatureTemp);
        dtostrf(nilaiSensorSoil, 4, 2, soilMoisture);

        Serial.print("Temperature: ");
        Serial.println(temperatureTemp);

        Serial.print("Humidity: ");
        Serial.println(humString);

        Serial.print("soilMoisture: ");
        Serial.println(soilMoisture);

        client.publish("room/hum", humString);
        client.publish("room/suhu", temperatureTemp);
        client.publish("soil/Kelembaban", soilMoisture);

      }
}