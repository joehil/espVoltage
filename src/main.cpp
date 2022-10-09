#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "lallinger";
const char* password = "Fritz9.3.1990";
const char* mqtt_server = "192.168.0.211";

const char* clientId = "espVoltage";

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0;  // value read from the pot

const double timeout = 1800e6;

WiFiClient espClient;
PubSubClient client(espClient);
void eventWiFi(WiFiEvent_t event);

long lastMsg = 0;
char timestring[50];
char msg[50];
int status1 = 2;
int status2 = 2;
int oldstatus1 = 0;
int oldstatus2 = 0;
unsigned int cnt = 0;
unsigned int cmdcnt = 0;
unsigned int oldcmdcnt = 0;
unsigned int count = 0;
int ev2cnt = 0;

void setup_wifi() {
  int loopcnt = 0;
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.setAutoReconnect(false);
  WiFi.onEvent(eventWiFi); 
  WiFi.hostname("espVoltage");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    loopcnt++;
    if (loopcnt > 20){
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/********************************************************
/*  Handle WiFi events                                  *
/********************************************************/
void eventWiFi(WiFiEvent_t event) {
     
  switch(event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
      Serial.println("EV1");
    break;
    
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      Serial.println("EV2");
      WiFi.reconnect();
      ev2cnt++;
      if (ev2cnt > 20) 
         ESP.restart();
    break;
    
    case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
      Serial.println("EV3");
    break;
    
    case WIFI_EVENT_STAMODE_GOT_IP:
      Serial.println("EV4");
    break;
    
    case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
      Serial.println("EV5");
      ESP.restart();
    break;
    
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
      Serial.println("EV6");
    break;
    
    case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
      Serial.println("EV7");
      ESP.restart();
    break;
    
    case WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED:
      Serial.println("EV8");
    break;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}

void reconnect() {
  // Loop until we're reconnected
  int loopcnt = 0;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (client.connect(clientId)) {
      loopcnt = 0;
      Serial.println("connected");
      // Once connected, publish an announcement...
      strcat(msg,"/outTopic/IP");
      client.publish(msg, WiFi.localIP().toString().c_str());
      // ... and resubscribe
      strcpy(msg,clientId);
      strcat(msg,"/inTopic");
      client.subscribe(msg);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      loopcnt++;
      if (loopcnt > 20){
        if (WiFi.status() != WL_CONNECTED) {
          ESP.restart();
        }
      }
    }
  }
}

void setup() {
  sensorValue = 0;

  for (int i=0;i<5;i++){
    sensorValue += analogRead(analogInPin);
    delay(2000);
  }

  sensorValue=sensorValue/5;

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
    count=0;
  }
  client.loop();

  if (WiFi.status() != WL_CONNECTED){
    ESP.restart();
  }
 
  char buffer[20];
  char* topic = "/outTopic/voltvalue";
  char* path = (char *) malloc(1 + strlen(clientId) + strlen(topic) + 20);
  strcpy(path, clientId);
  strcat(path, topic);
  sprintf(buffer,"%d",sensorValue);
  client.publish(path, buffer);
 
  delay(2000);
  ESP.deepSleep(timeout);
}
