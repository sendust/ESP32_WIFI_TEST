/*
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
https://cplusplus.com/reference/ctime/tm/
https://cplusplus.com/reference/ctime/strftime/

ESP32 time and wifi practice


*/

#include <WiFi.h>
#include "time.h"

int printwifistatus();
void initWiFi();
int buffer_push(int);
void buffer_reset();
void do_protocol(int);
void printLocalTime();

const char* ssid = "ASUS-SENDUST";
const char* password = "--------";

unsigned long tm_now = millis();
unsigned long tm_last_print = 0;
unsigned long tm_last_wifiretry = 0;
bool led_status = false;
const char* wifistatus[] = {"idle", "no ssid avail", "scan complete", "connected", "fail connect", "connect lost", "disconnected"};
char strftime_buf[64];

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;


uint8_t sbuffer[20];
uint8_t sbuffer_idx = 0;
bool print_serial = false;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, led_status);
  unsigned long tm_start = millis();
  initWiFi();
  Serial.print("WIFI connecting... ");
  Serial.print(millis() - tm_start);
  Serial.print("  mili seconds");
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

}

void loop() {
  int sbyte;
  while(Serial.available())
  {
    sbyte = buffer_push(Serial.read());
    print_serial = true;
  }
  if (print_serial)
  {
    for (int i = 0; i < 20; i++)
    {
      Serial.print(" ");
      Serial.print(sbuffer[i]);
    }
    Serial.println("");
    print_serial = false;
  }

  if (sbyte = 10)
  {
    do_protocol(sbuffer[0]);
    buffer_reset();
  }
  
  tm_now = millis();
  if ((tm_now - tm_last_print) > 1000)
  {
    led_status = !led_status;
    digitalWrite(2, led_status);
    time_t now;
    struct tm * timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", timeinfo);
    Serial.println("strftime_buf variable is ");
    Serial.println(strftime_buf);
    
    if ((printwifistatus() != WL_CONNECTED) && ((tm_now - tm_last_wifiretry) > 5000))
      {
        WiFi.begin(ssid, password);
        Serial.println("Retry Wifi connection...");
        tm_last_wifiretry = tm_now;
      }
    tm_last_print = tm_now;
  }
}



void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

int printwifistatus()
{
  int wstatus = WiFi.status();
  Serial.println(wifistatus[wstatus]);
  if (wstatus == WL_CONNECTED)
  {
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.RSSI());
  }
  return wstatus;
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 1");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
}


int buffer_push(int single_byte)
{
  sbuffer[sbuffer_idx] = single_byte;
  if (sbuffer_idx < 19)
  {
    sbuffer_idx++;
  }
  else
  {
    sbuffer_idx = 0; 
  }
  return single_byte;
}

void buffer_reset()
{
  for (int i = 0; i < 20; i++)
    sbuffer[i] = 0;
  sbuffer_idx = 0;
}


void do_protocol(int cmd)
{
  if (cmd == 49)      // select number 1
  {
    Serial.println("Number 1 pressed\r\nconfig Time");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  }

  if (cmd == 50)      // select number 2
  {
    Serial.println("Number 2 pressed");
    printLocalTime();
  }
}
