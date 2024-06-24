#include "Arduino.h"
#include "BoardConfig.h"
#include "WiFiConfig.h"
#include <esp_wifi.h>
#include <WiFi.h>

#define MAX_NMEA_CLIENTS 4
#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
WiFiServer server(SERIAL_TCP_PORT);
WiFiClient TCPClient[MAX_NMEA_CLIENTS];
#endif

uint8_t buf1[bufferSize];
uint16_t i1=0;

uint8_t buf2[bufferSize];
uint16_t i2=0;

IPAddress IP(0, 0, 0, 0);

void startWifi()
{
 
 int wifi_retry = 0;

  if (WLAN_CLIENT == 1) {
    CONSOLE.println("Start WLAN Client"); // WiFi Mode Client

    WiFi.mode(WIFI_STA);

    delay(100);
    WiFi.begin(CL_ssid, CL_password);

    CONSOLE.print("try to Connect to Wireless network: ");

    while (WiFi.status() != WL_CONNECTED  && wifi_retry < 40) {         // Check connection, try 20 seconds
      wifi_retry++;
      delay(500);
      CONSOLE.print(".");
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {   // No client connection start AP
    // Init wifi connection
    CONSOLE.println("KO");
    CONSOLE.println("");
    CONSOLE.println("Start WLAN AP"); // WiFi Mode Client

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_ssid, AP_password);
    delay(100);
    WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet);
    IP = WiFi.softAPIP();
    CONSOLE.print("AP IP address: ");
    CONSOLE.println(IP);
    wifiType = 1;

  } else {  // Wifi Client connection was sucessful

    IP = WiFi.localIP();
    CONSOLE.println("OK");
    CONSOLE.println("");
    CONSOLE.println("WiFi client connected");
    CONSOLE.print("IP client address: ");
    CONSOLE.println(IP);
  }

  // #ifdef PROTOCOL_TCP
  // CONSOLE.print("Starting TCP Server 2 - ");  
  // CONSOLE.print(IP);  
  // CONSOLE.print(":");  
  // CONSOLE.println(SERIAL_TCP_PORT);  
  // server.begin(); // start TCP server 
  // server.setNoDelay(true);
  // #endif

  esp_err_t esp_wifi_set_max_tx_power(34);  //lower WiFi Power

}