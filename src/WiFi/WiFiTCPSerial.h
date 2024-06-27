#ifndef TCPSERVERSERIAL
    #define TCPSERVERSERIAL
    
    #include <Arduino.h>
    #include <WiFi.h>
    #include <esp_wifi.h>
    #include <WiFiClient.h>
    #include <WiFiServer.h>

    // OPTIONS
    #define NO_OPTION       0
    #define TIMESTAMP       0
    #define RECONNECT       2

    #define SERIAL_TCP_PORT 8881   // Wifi Port
    #define SERIAL_UDP_PORT 14551  // UDP Port 
  
    class WiFiTCPSerial : public Stream
    {
        private:
            WiFiClient client;  
            WiFiServer server;  
            int m_server_port;
            IPAddress m_IP;
            bool m_connected;
            bool m_reconnect;
            int connected();

        public:
            WiFiTCPSerial(); //const char *server, int server_port, int options);
            virtual ~WiFiTCPSerial();

            int available();
            int read();
            int peek();
            void flush();
            void end();
            int begin(unsigned long baud);
        
            //print implementation
            virtual size_t write(uint8_t val);
            virtual size_t write(const uint8_t *buffer, size_t size);
            using Print::write; 
            

};
#endif
