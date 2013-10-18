#include "WiFi.h"
#include "utility/SimplelinkWifi.h"
#include "Energia.h"

#define DISABLE	(0)
#define ENABLE	(1)
extern volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP,
OkToDoShutDown, ulCC3000DHCP_configured;
extern volatile unsigned char ucStopSmartConfig;

tNetappIpconfigRetArgs WiFiClass::ipConfig = {{0}};
unsigned char WiFiClass::fwVersion[] = {0, 0};

extern uint8_t CSpin;
extern uint8_t ENpin;
extern uint8_t IRQpin;

// Simple Config Prefix
const char aucCC3000_prefix[] = {'T', 'T', 'T'};

//AES key "smartconfigAES16"
const unsigned char smartconfigkey[] = {0x73,0x6d,0x61,0x72,0x74,0x63,0x6f,0x6e,0x66,0x69,0x67,0x41,0x45,0x53,0x31,0x36};

uint8_t calculator_socket_number = 0;
WiFiClass::WiFiClass()
{
	// Driver initialization
	init();
}

void WiFiClass::setCSpin(uint8_t pin){
	CSpin=pin;
}
void WiFiClass::setENpin(uint8_t pin){
	ENpin=pin;
}
void WiFiClass::setIRQpin(uint8_t pin){
	IRQpin=pin;
}


void WiFiClass::init()
{
	//nothing to be done for init
}

// connect to an open AP
int WiFiClass::begin(char* ssid)
{
	uint8_t status=0;
	pio_init();
	init_spi();

	wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
	
	wlan_start(0);
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	wlan_connect(WLAN_SEC_UNSEC, ssid, strlen(ssid), NULL, NULL, 0);

	while (ulCC3000Connected  == 0)
	{
		delay(100);
	}

	return status;
}

// connect to a WPA AP
int WiFiClass::begin(char* ssid, const char* pass)
{
	uint8_t status=0;
	pio_init();
	init_spi();

	wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

	wlan_start(0);
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	wlan_connect(WLAN_SEC_WPA2, ssid, strlen(ssid), NULL, (unsigned char *)pass, strlen((char *)(pass)));

	while (ulCC3000Connected  == 0)
	{
		delay(100);
	}

	return status;
}

// connect to a WEP AP
int WiFiClass::begin(char* ssid, uint8_t key_idx, unsigned char *key)
{
	uint8_t status=0;
	pio_init();
	init_spi();

	wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
	
	wlan_start(0);
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	wlan_connect(WLAN_SEC_WEP, ssid, strlen(ssid), NULL, key, key_idx);
	
	while (ulCC3000Connected  == 0)
	{
		delay(100);
	}

	return status;
}

// Do stuff before smartconfig
int WiFiClass::begin()
{
	pio_init();
	init_spi();

	wlan_init( CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
	
	wlan_start(0);

	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	ucStopSmartConfig   = 0;

	int i=0;
	while (i<5)
	{
		delay(100);
		i++;
	}

	if(ulCC3000Connected==0)
	return WL_DISCONNECTED;
	else return WL_CONNECTED;
}

int WiFiClass::disconnect()
{
//	if(wlan_ioctl_statusget() == WLAN_STATUS_CONNECTED);
//		wlan_disconnect();
	return 1;
}
uint8_t WiFiClass::status()
{
	if(ulCC3000Connected == 0)
		return WL_DISCONNECTED;
	else
		return WL_CONNECTED;
}

unsigned char* WiFiClass::firmwareVersion()
{
	nvmem_read_sp_version(fwVersion);
	return fwVersion;
}

IPAddress WiFiClass::localIP()
{
	tNetappIpconfigRetArgs config;
	netapp_ipconfig(&config);
	IPAddress ret(config.aucIP[3], config.aucIP[2], config.aucIP[1], config.aucIP[0]);
	return ret;
}

IPAddress WiFiClass::subnetMask()
{
	tNetappIpconfigRetArgs config;
	netapp_ipconfig(&config);
	IPAddress ret(config.aucSubnetMask[3], config.aucSubnetMask[2], config.aucSubnetMask[1], config.aucSubnetMask[0]);
	return ret;
}

IPAddress WiFiClass::gatewayIP()
{
	tNetappIpconfigRetArgs config;
	netapp_ipconfig(&config);
	IPAddress ret(config.aucDefaultGateway[3], config.aucDefaultGateway[2], config.aucDefaultGateway[1], config.aucDefaultGateway[0]);
	return ret;
}

char* WiFiClass::SSID()
{
	netapp_ipconfig(&ipConfig);
	return (char *)ipConfig.uaSSID;
}

uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
	tNetappIpconfigRetArgs config;
	netapp_ipconfig(&config);
	memcpy(mac, &config.uaMacAddr, WL_MAC_ADDR_LENGTH);
	return mac;
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult) {
	uint32_t ip;
	int ret = gethostbyname((char *)aHostname, strlen(aHostname), &ip);
	aResult = ntohl(ip);
	return ret;
}
/*
int WiFiClass::startSmartConfig()
{	ulSmartConfigFinished = 0;
	ulCC3000Connected = 0;
	ulCC3000DHCP = 0;
	OkToDoShutDown=0;
	
	// Reset all the previous configuration
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);	
	wlan_ioctl_del_profile(255);
	
	//Wait until CC3000 is disconnected
	while (ulCC3000Connected == 1)
	{
		//__delay_cycles(1000);
	}
	
	// Trigger the Smart Config process

	wlan_smart_config_set_prefix((char*)aucCC3000_prefix);
     
	
	// Start the SmartConfig start process
	wlan_smart_config_start(1);
                                                                           
	
	// Wait for Smartconfig process complete
	while (ulSmartConfigFinished == 0)
	{
		
		//__delay_cycles(6000000);
		
		
	}
	

#ifndef CC3000_UNENCRYPTED_SMART_CONFIG
	// create new entry for AES encryption key
	nvmem_create_entry(NVMEM_AES128_KEY_FILEID,16);

	
	// write AES key to NVMEM
	aes_write_key((unsigned char *)(&smartconfigkey[0]));

	// Decrypt configuration information and add profile
	
	wlan_smart_config_process();

#endif    
	
	// Configure to connect automatically to the AP retrieved in the 
	// Smart config process
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);
	
	// reset the CC3000
	wlan_stop();
	
	//__delay_cycles(6000000);

	wlan_start(0);
	
	// Mask out all non-required events
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	return 0;
}
*/

// if it return 0, it means the socket number is over how many CC3000 can handler
bool WiFiClass::countSocket(bool add_sock)
{
	if(add_sock)
		calculator_socket_number++; 
	else
		calculator_socket_number--;

	if (calculator_socket_number>4) 
		return 0;
	else
		return 1;
}

WiFiClass WiFi;
