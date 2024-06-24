#include "WiFiUDPSerial.h"
#include "BoardConfig.h"
#include "Globals.h"

// Server IP Address of server (I.E 10.0.0.100), port is the listening port on the server that will handle serial
// inputs and outputs. Options is false for non or TIMESTAMP for a line by line timestamp mimicking the Arduino Serial Monitor
WiFiUDPSerial::WiFiUDPSerial() // params: const char *server, int server_port, int options
{
    m_server_port = SERIAL_UDP_PORT;
    m_broadcastIP = IPAddress(255,255,255,255);
    m_IP = IPAddress(0,0,0,0);

    m_show_timestamp = true;
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
    m_connected = udp.begin(m_server_port);

    }

    return m_connected;
}

// Check if connection is still live and reconnect if option 
int WiFiUDPSerial::isConnected()
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
    if (!isConnected())
        return 0;

    return udp.available();
}
int WiFiUDPSerial::read()
{
    if (!isConnected())
        return 0;
    return udp.read();
}
int WiFiUDPSerial::peek()
{
    if (!isConnected())
        return 0;

    return udp.peek();
}
void WiFiUDPSerial::flush()
{
    if (isConnected())
        udp.flush();

}

void WiFiUDPSerial::stop()
{
    udp.stop();
}

size_t WiFiUDPSerial::write(const uint8_t *buffer, size_t size)
{
    if (m_show_timestamp) // if this is a new line, check if the connection is alive (we don't want to check multiple times per lines)
    {
        m_connected = isConnected();
    }

    if ((size==2) && (*buffer==13) && (*(buffer+1)==10)) // if this is a new line, raise the flag so a timestamp will be printed on the next call
       m_show_timestamp = true;
    else
        m_show_timestamp = false;
    
    if (!m_connected) // don't send data, we're not conected
       return 0;

    // we're good, send data to the server
    int ret = udp.beginPacket(m_broadcastIP,m_server_port);
    udp.write(buffer, size);
    udp.endPacket();
    udp.flush();

    return ret != 0;       
}

size_t WiFiUDPSerial::write(uint8_t i)
{
    int ret = udp.beginPacket(m_broadcastIP,m_server_port);
    udp.write(&i, 1);
    udp.endPacket();
    udp.flush();

    return ret != 0;       
}
