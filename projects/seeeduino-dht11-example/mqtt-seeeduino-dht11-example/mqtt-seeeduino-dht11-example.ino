#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char *serverIP   = "140.118.25.64";
const int serverPort   = 21883;
const char *clientId   = "Thingname_1618541886";
const char *username   = "0b82fdbed0184f9b87";
const char *password   = "r:2b3ad27913a0e1308f";
const char *tempTopic  = "qiot/things/Team3/Thingname/temp";
const char *humidTopic = "qiot/things/Team3/Thingname/humid";

// Enter a MAC address for your controller below.
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient ethClient;
PubSubClient client(serverIP, serverPort, ethClient);

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println();
  Serial.println("Serial port started!");

  // Initialize DHT11 sensor
  dht.begin();
  Serial.println("DHT initialized!");

  // start the Ethernet connection:
  while (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP, wait 1.5s and try again.");
    delay(1500);
  }
  printIPAddress();
}

void loop()
{
  switch (Ethernet.maintain())
  {
  case 1:
    //renewed fail
    Serial.println("Error: renewed fail");
    break;
  case 2:
    //renewed success
    Serial.println("Renewed success");
    printIPAddress();
    break;
  case 3:
    //rebind fail
    Serial.println("Error: rebind fail");
    break;
  case 4:
    //rebind success
    Serial.println("Rebind success");
    printIPAddress();
    break;
  default:
    //nothing happened
    break;
  }

  // Connect to MQTT Broker
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h))
  {
    Serial.println("Failed to read from DHT");
  }
  else
  {
    String dataStr = "Humidity: " + String(h) + "%, Temperature: " + String(t);
    Serial.println(dataStr);

    if (client.connected())
    {
      String tempStr = "{ \"value\": " + String(t) + "}";
      String humidStr = "{ \"value\": " + String(h) + "}";
      client.publish(tempTopic, tempStr.c_str());
      client.publish(humidTopic, humidStr.c_str());
    }
  }
}

void printIPAddress()
{
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}

void reconnect()
{
  if (client.connected())
    return;
  if (client.connect(clientId, username, password))
  {
    Serial.println("MQTT client connected!");
  }
  else
  {
    Serial.print("MQTT client connection failed, rc=");
    Serial.print(client.state());
    Serial.println();
    delay(1500);
  }
}
