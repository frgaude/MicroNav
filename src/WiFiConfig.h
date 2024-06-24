#include <WiFi.h>

#define PROTOCOL_TCP
#define WLAN_CLIENT 1  // Set to 1 to enable client network. 0 to act as AP only

// Wifi cofiguration Client and Access Point
const char *AP_ssid = "Casachoc";  // ESP32 as AP
const char *CL_ssid = "Casachoc";   // ESP32 as client in network

const char *AP_password = "banner28";   // AP password. Must be longer than 7 characters
const char *CL_password = "banner28";  // Client password

// Put IP address details here
IPAddress AP_local_ip(192, 168, 8, 1);  // Static address for AP
IPAddress AP_gateway(192, 168, 8, 1);
IPAddress AP_subnet(255, 255, 255, 0);

IPAddress CL_local_ip(192, 168, 8, 100);  // Static address for Client Network. Please adjust to your AP IP and DHCP range!
IPAddress CL_gateway(192, 168, 8, 1);
IPAddress CL_subnet(255, 255, 255, 0);

int wifiType = 0; // 0= Client 1= AP

#define SERIAL_TCP_PORT 8881       // Wifi Port UART1
#define bufferSize 1024
