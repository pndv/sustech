#include <SoftwareSerial.h>
#include "WiFiEsp.h"


#define PORT_TX 10
#define PORT_RX 11
#define PORT_LED 13
#define PORT_SWITCH 2
#define BAUD_RATE 115200
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define HTTPS_PORT 443
#define CLIENT_DELAY_MS 10

void assertEquals(const char* test, int actual, int expected);
void assertEquals(const char* test, char* actual, char* expected);
void fail(const char* test, char* actual, char* expected);
void fail(const char* test, int actual, int expected);
void pass(const char* test);


SoftwareSerial Serial_3(PORT_TX,PORT_RX);

char ssid[] = "tmp_mission";
char pwd[]= "tmp_mission";
char server[] = "https://vpsustech.herokuapp.com";
char host[]="vpsustech.herokuapp.com";

WiFiEspClient client;

float temp;
int ledStatus;

unsigned long update_interval_ms = 20000;
unsigned long check_interval_ms = 2000;
unsigned long current_time = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PORT_LED, OUTPUT);
  pinMode(PORT_SWITCH, INPUT_PULLUP);

  Serial.begin(BAUD_RATE);

  Serial_3.begin(BAUD_RATE);
  Serial_3.println("AT+UART_DEF=9600,8,1,0,0\r\n");
  Serial_3.flush();
  delay(2);
  Serial_3.end();

  Serial_3.begin(SOFTWARE_SERIAL_BAUD_RATE);

  WiFi.init(&Serial_3);

  WiFi.begin(ssid, pwd);
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  assertEquals("Connect to server", client.connectSSL(server,HTTPS_PORT), 1);
  assertEquals("Connected", client.connected(), true);  
}

void updateLight() {
  if (ledStatus == 1) {
    digitalWrite(PORT_LED, HIGH);
  } else {
    digitalWrite(PORT_LED, LOW);
  }
}

void updateStatus() {
  client.println("GET /latest.html HTTP/1.1");
  client.println("Host: vpsustech.herokuapp.com");
  client.println("Connection: keep-alive");
  client.println();
  delay(CLIENT_DELAY_MS);

  bool found=false;
  while(client.available()) {
    if (found == false) {
      client.find((char*) "\r\n\r\n");
      found = true;
    } else {
      // Example info string:
      // {info: "0;255", status=200}
      String info = client.readString();

      int startIndex = info.indexOf(":") + 2;
      int endIndex = info.indexOf(",") - 1;

      info = info.substring(startIndex, endIndex);

      //now, info = 0;255
      int semicolonIndex = info.indexOf(";");
      String stat = info.substring(0, semicolonIndex);
      String temperature = info.substring(semicolonIndex+1);

      ledStatus = stat.toInt();
      temp = temperature.toFloat();
    }
  }
}

void ConnectServer() {
  client.connectSSL(server, HTTPS_PORT);
}

void toggleStatus() {
  client.println("GET /toggle.html HTTP/1.1");
  client.println("Host: vpsustech.herokuapp.com");
  client.println("Connection: keep-alive");
  client.println();
  delay(CLIENT_DELAY_MS);

  while(client.available()) {
    Serial.println(client.readString());
  }
}

void postTemp() {
  String togo = String(analogRead(0)); // replace with actual temp
  client.println("POST /temp.html HTTP/1.1");
  client.println("Host: vpsustech.herokuapp.com");
  client.println("Accept: */*");
  client.println("Content-Lenght: ");
  client.println(togo.length());
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println();
  client.println(togo);
  delay(CLIENT_DELAY_MS);

  while(client.available()) {
    Serial.println(client.readString());
  }
}

void disconnectServer() {
  client.stop();
  Serial.println("Reconnecting...");
  ConnectServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(PORT_SWITCH) == HIGH) {
    while(digitalRead(PORT_SWITCH) == HIGH) { // Wait for button to be unpressed 
      }

    toggleStatus();
    updateStatus();
    updateLight();  
    
  }

  if ((millis() - current_time) > update_interval_ms) {
    postTemp();
    current_time = millis();
  }
  
  if ((millis() - current_time) > check_interval_ms) {
    updateStatus();
    updateLight();
  }

  if(client.connected() == false){
    disconnectServer();
  }
  
}


////////////////////////////////////////////////////////////////////////////////////
void assertEquals(const char* test, int actual, int expected) {
  if (actual == expected)
    pass(test);
  else
    fail(test, actual, expected);
}

void assertEquals(const char* test, char* actual, char* expected)
{
  if (strcmp(actual, expected) == 0)
    pass(test);
  else
    fail(test, actual, expected);
}

void pass(const char* test) {
  Serial.print(F("********************************************** "));
  Serial.print(test);
  Serial.println(" > PASSED");
  Serial.println();
}

void fail(const char* test, char* actual, char* expected) {
  Serial.print(F("********************************************** "));
  Serial.print(test);
  Serial.print(" > FAILED");
  Serial.print(" (actual=\"");
  Serial.print(actual);
  Serial.print("\", expected=\"");
  Serial.print(expected);
  Serial.println("\")");
  Serial.println();
  delay(10000);
}

void fail(const char* test, int actual, int expected) {
  Serial.print(F("********************************************** "));
  Serial.print(test);
  Serial.print(" > FAILED");
  Serial.print(" (actual=");
  Serial.print(actual);
  Serial.print(", expected=");
  Serial.print(expected);
  Serial.println(")");
  Serial.println();
  delay(10000);
}
