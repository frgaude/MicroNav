#include "WiFiUDPSerial.h"
#include "BoardConfig.h"
#include "Globals.h"

// Server IP Address of server (I.E 10.0.0.100), port is the listening port on the server that will handle serial
// inputs and outputs. Options is false for non or TIMESTAMP for a line by line timestamp mimicking the Arduino Serial Monitor
WiFiUDPSerial::WiFiUDPSerial() // params: const char *server, int server_port, int options
{
    m_server_port = SERIAL_UDP_PORT;
    m_broadcastIP = IPAddress(192,168,8,255); //should be dynamic
    m_IP = IPAddress(0,0,0,0);

    m_connected = false;
    m_reconnect = RECONNECT;
}

// private methods
WiFiUDPSerial::~WiFiUDPSerial()
{
    udp.stop();
}

// the baud parameter is specified but ignored for compatibility with the Serial library when used as a 
// drop-in replacement
int WiFiUDPSerial::begin(unsigned long baud)    
{
    m_connected = true;
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
        m_connected = false;
    else
    {
    // TCP client mode
        // if (!client.connect(m_server, m_server_port, 1000))
        //     m_connected = false;

    //TCP server mode
        // server.begin(SERIAL_TCP_PORT);
        // server.setNoDelay(true);
        // m_connected = true;
    //UDP - not useful if bradcasting only?
        CONSOLE.print("starting UDP listening on port ");
        CONSOLE.println(m_server_port);
    m_connected = udp.begin(m_server_port);

    }

    return m_connected;
}

// Check if connection is still live and reconnect if option 
bool WiFiUDPSerial::connected()
{
    // TCP client mode
        // m_connected = client.connected();
        // if (!m_connected && m_reconnect) // attempt reconnection if option is enabled
        // {
        //    if (!client.connect(m_server, m_server_port, 250))
        //     m_connected = false;
        // }
    //TCP server mode

    //UDP
        m_connected = true;

   return m_connected;
 
}

int WiFiUDPSerial::available()
{
    // if (!connected())
        // return 0;
//    return udp.parsePacket();
    return udp.available();
}

int WiFiUDPSerial::read()
{
    // if (available() > 0) {
        return udp.read();
    // }
    // return -1; // no data available
}

int WiFiUDPSerial::peek()
{
    // if (!connected())
        // return 0;

    return udp.peek();
}
void WiFiUDPSerial::flush()
{
    // if (connected())
        udp.flush();

}

void WiFiUDPSerial::end()
{
    udp.stop();
    m_connected = false;
}

size_t WiFiUDPSerial::write(const uint8_t *buffer, size_t size)
{
    if (!m_connected) // don't send data, we're not conected
       return 0;

    // we're good, send data to the server
    udp.beginPacket(m_broadcastIP,m_server_port);
    size_t written = udp.write(buffer, size);
    udp.endPacket();
    CONSOLE.print("UDP:");
    CONSOLE.write(buffer, size);

    return written;       
}

size_t WiFiUDPSerial::write(uint8_t i)
{
    if (!m_connected) // don't send data, we're not conected
       return 0;

    // we're good, send data to the server
    udp.beginPacket(m_broadcastIP,m_server_port);
    size_t written = udp.write(&i, 1);
    udp.endPacket();

    CONSOLE.print("u");
    CONSOLE.write(&i, 1);

    return written;       
}
