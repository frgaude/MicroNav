#include "WiFiTCPSerial.h"
#include "BoardConfig.h"
#include "Globals.h"

// TODO: document the class and its methods
// TODO: implement MAX_CLIENTS > 1

// Constructor
WiFiTCPSerial::WiFiTCPSerial() // params: const char *server, int server_port, int options
{
    m_server_port = SERIAL_TCP_PORT;
    m_IP = IPAddress(0,0,0,0);

    m_connected = false;
    m_reconnect = RECONNECT;
}

// private methods
WiFiTCPSerial::~WiFiTCPSerial()
{
    client.stop();
    server.stop();
}

// the baud parameter is specified but ignored for compatibility with the Serial library when used as a 
// drop-in replacement
int WiFiTCPSerial::begin(unsigned long baud)    
{
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
        m_connected = false;
    else
    {
        CONSOLE.print("starting TCP server on port ");
        CONSOLE.println(m_server_port);

    server.begin(m_server_port);
    server.setNoDelay(true);
    m_connected = true;
    }

    return m_connected;
}

// Check if connection is still live and reconnect if option 
// int WiFiTCPSerial::connected()
// {
//     // TCP client mode
//         // m_connected = client.connected();
//         // if (!m_connected && m_reconnect) // attempt reconnection if option is enabled
//         // {
//         //    if (!client.connect(m_server, m_server_port, 250))
//         //     m_connected = false;
//         // }
//     //TCP server mode

//     //UDP
//         m_connected = true;

//    return m_connected;
 
// }

int WiFiTCPSerial::connected()
{
    // Check if a new client is connecting only if we don't have an active connection
    if (!client.connected() && server.hasClient()) {
        // If we have an active client, stop it
        if (client) client.stop();
        // Accept the new client connection
        client = server.available();
        CONSOLE.print("new tcp connection 1");
    }

    // Return the available data count for the current client, which might be 0
    return client.connected();
}

int WiFiTCPSerial::available()
{
    // If the current client is connected and has data available, return the available data count
    if (client.connected() && client.available() > 0) {
        return client.available();
    }

    // Check if a new client is connecting only if we don't have an active connection
    if (!client.connected() && server.hasClient()) {
        // If we have an active client, stop it
        if (client) client.stop();
        // Accept the new client connection
        CONSOLE.print("new tcp connection 2");
        client = server.available();
    }

    // Return the available data count for the current client, which might be 0
    return client.available();
}

int WiFiTCPSerial::read()
{
    // if (available() > 0) {
            return client.read();
        // }
        // return -1; // No data available
}

int WiFiTCPSerial::peek()
{
    // if (!connected())
    //     return 0;

    return client.peek();
}
void WiFiTCPSerial::flush()
{
    // if (connected())
        client.flush();

}

void WiFiTCPSerial::end()
{
        client.stop();
        server.stop();
}

size_t WiFiTCPSerial::write(const uint8_t *buffer, size_t size)
{
    if (!connected()) // don't send data, we're not connected
        return 0;

    // we're good, send data to the server
    size_t written = client.write(buffer, size);
    
    return written; 
}

size_t WiFiTCPSerial::write(uint8_t i)
{
    if (!connected()) // don't send data, we're not connected
        return 0;

    // we're good, send data to the server
    return client.write(i);
}
