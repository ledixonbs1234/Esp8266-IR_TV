#include <Arduino.h>
#include <IRremoteESP8266.h>
#include "ESP8266WiFi.h"
#include <IRrecv.h>
#include <IRutils.h>
#include "IRsend.h"
#include "PubSubClient.h"
#include "string.h"
#include "WString.h"

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = 14;
const uint16_t Pinsend = D2;

#define ssid "Khoi Dien Tu"
#define password "05050456"
#define mqtt_server "broker.hivemq.com"
const uint16_t mqtt_port = 1883;
bool irFinded = false;

WiFiClient espClient;
PubSubClient client(espClient);

IRrecv irrecv(kRecvPin);
IRsend irSend(Pinsend);

decode_results results;

void reconnect();
void callback(char *topic, byte *payload, unsigned int length);

void setup()
{
  Serial.begin(9600);

  //tao ket noi toi wifi
  Serial.print("Dang ket noi toi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi da ket noi thanh cong");
  Serial.println("IP Adress: ");
  Serial.println(WiFi.localIP());

  //tao connect mqtt
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.subscribe("IR_Read");

  irSend.begin();
  irrecv.enableIRIn(); // Start the receiver

  while (!Serial) // Wait for the serial connection to be establised.
    delay(50);

  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
  Serial.print("Chuan bi in ");
  delay(1000);

  irSend.sendNEC(0x20DFA25DUL);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Co tin nhan moi toi tu topic: ");
  Serial.println(topic);
  int le = length;

  //kiem tra topic dung khong
  if (strcmp(topic, "IR_Read") == 0)
  {

    Serial.print("------------------------------------------");
    if ((char)payload[0] == '0')
    {
      //cho phep ir receive
      irFinded = true;
      //xac nhan da nhan tin hieu tu dien thoai
      client.publish("IR_Send", "OK_Read");
    }
  }

  for (int i = 0; i < le; i++)
  {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

void publishMessage(char *data)
{
  client.publish("IR_Send", data);
}

void reconnect()
{
  while (!client.connected())
  {
    if (client.connect("ESP8266_id1", "ESP_offline", 1, 0, "ESP8266_id1_offline"))
    {
      Serial.println("Da ket noi : ");
      client.subscribe("IR_Read");
    }
    else
    {
      Serial.print("Loi:, rc=");
      Serial.print(client.state());
      Serial.println("thu lai trong 5 giay");
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
    reconnect();
  client.loop();

  if (irrecv.decode(&results) && irFinded)
  {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    String stringtext = uint64ToString(results.value, HEX);
    char buffer[50];
    stringtext.toCharArray(buffer, stringtext.length(), 0);
    publishMessage(buffer);
    Serial.println("");
    irrecv.resume(); // Receive the next value
  }
  delay(100);
}