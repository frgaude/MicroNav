#ifndef UDPSERIAL
    #define UDPSERIAL
    
    #include <Arduino.h>
    #include <WiFi.h>
    #include <WiFiUdp.h>
    #include <esp_wifi.h>

    // OPTIONS
    #define NO_OPTION       0
    #define TIMESTAMP       0
    #define RECONNECT       2

    #define SERIAL_TCP_PORT 8881   // Wifi Port
    #define SERIAL_UDP_PORT 14551  // UDP Port 
  
    class WiFiUDPSerial : public Stream
    {
        private:
            WiFiUDP udp;  
            int m_server_port;
            IPAddress m_localIP;
            IPAddress m_broadcastIP;
            bool m_connected;
            bool m_reconnect;
            bool connected();

        public:    
            WiFiUDPSerial(); //const char *server, int server_port, int options);
            virtual ~WiFiUDPSerial();

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
