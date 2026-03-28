#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFiS3.h>

#include "arduino_secrets.h" 
// WiFi Configuration
#define WIFI_TIMEOUT 30000  // 30 seconds

// HTTP Server Configuration
#define HTTP_PORT 80
#define MAX_CLIENTS 5

class WiFiController {
public:
  WiFiController();
  ~WiFiController();
  
  void begin();
  void update();
  
  // Connection status
  bool isConnected() const;
  String getLocalIP() const;
  String getStatus() const;
  
  // WiFi configuration
  void setSSID(const char* ssid);
  void setPassword(const char* password);
  
  // Debug
  void printNetworkInfo() const;
  
private:
  WiFiServer server;
  char ssid[32];
  char password[32];
  bool connected;
  unsigned long lastConnectionAttempt;
  
  void connect();
  void handleClient();
  void serveWebInterface(WiFiClient& client);
};

#endif
