#ifndef SERIALTOWIFI
    #define SERIALTOWIFI
    
    #include <Arduino.h>
    #include <WiFi.h>
    #include <WiFiUdp.h>
    #include <esp_wifi.h>
    #include <WiFiClient.h>

    // OPTIONS
    #define NO_OPTION       0
    #define TIMESTAMP       0
    #define RECONNECT       2

    #define SERIAL_TCP_PORT 8881   // Wifi Port
    #define SERIAL_UDP_PORT 14551  // UDP Port 
  
    class WiFiUDPSerial : public Stream
    {
        private:
            int m_server_port;
            IPAddress m_IP;
            IPAddress m_broadcastIP;
            bool  m_connected;
            bool m_show_timestamp;
            bool m_reconnect;

            int m_hours, m_minutes, m_secs;
            int isConnected();

        public:    
            // WiFiClient client;  
            WiFiServer server;  
            WiFiUDP udp;  

            WiFiUDPSerial(); //const char *server, int server_port, int options);
            virtual ~WiFiUDPSerial();

            virtual int available();
            virtual int read();
            virtual int peek();
            virtual void flush();
            void stop();

            int begin(unsigned long baud);
        
            //print implementation
            virtual size_t write(uint8_t val);
            virtual size_t write(const uint8_t *buffer, size_t size);
            using Print::write; 
            

};
#endif

extern WiFiUDPSerial serialToWifi;
//#define Serial serialToWifi    
