#ifndef wifi_h
#define wifi_h
#include "utility/SimplelinkWifi.h"
#include "IPAddress.h"
#include "TCPclient.h"
#include "TCPserver.h"

#define WL_FW_VER_LENGTH 2

class WiFiClass
{
	private:
	static void init();	
	static tNetappIpconfigRetArgs ipConfig;
	static unsigned char fwVersion[WL_FW_VER_LENGTH];
	static char _networkSsid[WL_NETWORKS_LIST_MAXNUM][WL_SSID_MAX_LENGTH];
	static int32_t _networkRssi[WL_NETWORKS_LIST_MAXNUM];
	static uint8_t _networkEncr[WL_NETWORKS_LIST_MAXNUM];

	public:
	
	WiFiClass();
	void setCSpin(uint8_t pin);
	void setENpin(uint8_t pin);
	void setIRQpin(uint8_t pin);
	int begin();
	int begin(int patchesAvailableAtHost);
	int begin(char* ssid);
	int begin(char* ssid, const char* pass);
	int begin(char* ssid, uint8_t key_idx, unsigned char* key );
	int disconnect(void);
	uint8_t status();
	static unsigned char* firmwareVersion();
	int updateFirmware();
	void config();
	void config(IPAddress local_ip);
	void config(IPAddress local_ip, IPAddress dns_server);
	void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
	void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
	IPAddress localIP();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	int hostByName(const char* aHostname, IPAddress& aResult);
	char* SSID();
	//uint8_t* BSSID(uint8_t* bssid);
	//int32_t RSSI();
	//uint8_t encryptionType();
	char* SSID(uint8_t networkItem);
	int32_t RSSI(uint8_t networkItem);
	uint8_t* macAddress(uint8_t* mac);
	uint8_t encryptionType(uint8_t networkItem);
	int8_t scanNetworks();
	int startSmartConfig();
	bool countSocket(bool add_sock);

	friend class TCPclient;
	friend class TCPserver;
};

extern WiFiClass WiFi;


// client

// server


#define UDP_TX_PACKET_MAX_SIZE 24

class WiFiUDP {
private:
  unsigned short int port;
  long udpSocket;
  sockaddr udpSocketAddr;
  volatile unsigned int bytesRecvd;
  int packetSize;
  unsigned char packetBuffer[25]; //25 is from basic wifi application  pucCC3000_Rx_Buffer[CC3000_APP_BUFFER_SIZE + CC3000_RX_BUFFER_OVERHEAD_SIZE];
  unsigned char rx_buf[25];
  long udpSocket2;
  sockaddr udpSocketAddr2;  

public:
  WiFiUDP();  // Constructor
  uint8_t begin(uint16_t);	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use

  int beginPacket(IPAddress ip, uint16_t _port);
  

  int endPacket();

  size_t write(const uint8_t *buffer, size_t size);
  

  int parsePacket();
  int available();


  int read(char* buffer, size_t len);
  void stop();
  
  friend class WiFiClass;

  

};

#endif
