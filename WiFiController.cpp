#include "WiFiController.h"
#include "LEDController.h"
#include "StateManager.h"
#define DEBUG

extern LEDController ledController;
extern StateManager stateManager;

WiFiController::WiFiController()
  : server(HTTP_PORT),
    connected(false),
    lastConnectionAttempt(0) {
  strncpy(ssid, WIFI_SSID, sizeof(ssid) - 1);
  strncpy(password, WIFI_PASSWORD, sizeof(password) - 1);
}

WiFiController::~WiFiController() {
}

void WiFiController::begin() {
  Serial.println("[WiFi] Starting WiFi configuration...");
  #ifdef DEBUG
    Serial.println("DEBUG mode enabled");
    Serial.println();
  #endif
  
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    server.begin();
    
#ifdef DEBUG
    Serial.println();
    Serial.println("[WiFi] Connected!");
    printNetworkInfo();
#endif
  } else {
    connected = false;
    Serial.println();
    Serial.println("[WiFi] Failed to connect. Running in offline mode.");
  }
}

void WiFiController::update() {
  if (!connected) {
    // Attempt reconnection every 30 seconds
    if (millis() - lastConnectionAttempt > 30000) {
      lastConnectionAttempt = millis();
      connect();
    }
    return;
  }
  
  // Check connection status
  if (WiFi.status() != WL_CONNECTED) {
    connected = false;
    Serial.println("[WiFi] Connection lost!");
    return;
  }
  
  handleClient();
}

bool WiFiController::isConnected() const {
  return connected;
}

String WiFiController::getLocalIP() const {
  if (connected) {
    return WiFi.localIP().toString();
  }
  return "Not connected";
}

String WiFiController::getStatus() const {
  if (connected) {
    return "Connected";
  }
  return "Disconnected";
}

void WiFiController::setSSID(const char* newSSID) {
  strncpy(ssid, newSSID, sizeof(ssid) - 1);
  ssid[sizeof(ssid) - 1] = '\0';
}

void WiFiController::setPassword(const char* newPassword) {
  strncpy(password, newPassword, sizeof(password) - 1);
  password[sizeof(password) - 1] = '\0';
}

void WiFiController::connect() {
  Serial.println("[WiFi] Attempting to reconnect...");
  WiFi.begin(ssid, password);
  lastConnectionAttempt = millis();
}

void WiFiController::handleClient() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  String request = "";
  String method = "";
  String path = "";
  
  // Parse HTTP request
  while (client.connected() && client.available()) {
    char c = client.read();
    request += c;
    
    // Parse first line for method and path
    if (request.indexOf("\n") > 0 && method == "") {
      int spaceIndex = request.indexOf(" ");
      method = request.substring(0, spaceIndex);
      int secondSpace = request.indexOf(" ", spaceIndex + 1);
      path = request.substring(spaceIndex + 1, secondSpace);
    }
  }
  
#ifdef DEBUG
  Serial.print("[HTTP] ");
  Serial.print(method);
  Serial.print(" ");
  Serial.println(path);
#endif
  
  // Route requests
  if (path == "/" && method == "GET") {
    serveWebInterface(client);
    
  } else if (path.startsWith("/api/")) {
    String response = "";
    
    if (path == "/api/status") {
      response = "{\"power\":";
      response += ledController.isOn() ? "true" : "false";
      response += ",\"brightness\":";
      response += ledController.getBrightness();
      response += ",\"speed\":";
      response += ledController.getAnimationSpeed();
      response += "}";
      
    } else if (path == "/api/power/toggle") {
      if (ledController.isOn()) {
        stateManager.saveLampState(ledController);
        ledController.turnOff();
      } else {
        ledController.turnOn();
        if (stateManager.hasSavedState()) {
          stateManager.restoreLampState(ledController);
        }
      }
      response = "{\"status\":\"ok\"}";
      
    } else if (path == "/api/color/set") {
      int bodyStart = request.indexOf("\r\n\r\n") + 4;
      String body = request.substring(bodyStart);
      
      int rIndex = body.indexOf("\"r\":");
      int gIndex = body.indexOf("\"g\":");
      int bIndex = body.indexOf("\"b\":");
      
      if (rIndex != -1 && gIndex != -1 && bIndex != -1) {
        uint8_t r = body.substring(rIndex + 4, body.indexOf(",", rIndex)).toInt();
        uint8_t g = body.substring(gIndex + 4, body.indexOf(",", gIndex)).toInt();
        uint8_t b = body.substring(bIndex + 4, body.indexOf("}", bIndex)).toInt();
        
        // Removed manual R/G swap - LEDController now expects standard RGB
        ledController.stopAnimation();
        ledController.setSolidColor(r, g, b);
        response = "{\"status\":\"ok\"}";
      }
      
    } else if (path == "/api/brightness/set") {
      int bodyStart = request.indexOf("\r\n\r\n") + 4;
      String body = request.substring(bodyStart);
      
      int brightnessIndex = body.indexOf("\"brightness\":");
      if (brightnessIndex != -1) {
        uint8_t brightness = body.substring(brightnessIndex + 13, body.indexOf("}", brightnessIndex)).toInt();
        if (brightness < 5) brightness = 5; // Prevent total blackout/loss of color data
        ledController.setBrightness(brightness);
        response = "{\"status\":\"ok\"}";
      }
      
    } else if (path == "/api/speed/set") {
      int bodyStart = request.indexOf("\r\n\r\n") + 4;
      String body = request.substring(bodyStart);
      
      int speedIndex = body.indexOf("\"speed\":");
      if (speedIndex != -1) {
        uint16_t speed = body.substring(speedIndex + 8, body.indexOf("}", speedIndex)).toInt();
        // Speed is interval in ms, 10ms (fast) to 1000ms (slow)
        if (speed < 10) speed = 10;
        if (speed > 1000) speed = 1000;
        ledController.setAnimation(ledController.getCurrentAnimation(), speed);
        response = "{\"status\":\"ok\"}";
      }
      
    } else if (path == "/api/animation/set") {
      int bodyStart = request.indexOf("\r\n\r\n") + 4;
      String body = request.substring(bodyStart);
      
      int typeIndex = body.indexOf("\"type\":");
      if (typeIndex != -1) {
        uint8_t type = body.substring(typeIndex + 7, body.indexOf("}", typeIndex)).toInt();
        ledController.setAnimation((AnimationType)type, 50);
        ledController.startAnimation();
        response = "{\"status\":\"ok\"}";
      }
      
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println("{\"error\":\"Not Found\"}");
      delay(1);
      client.stop();
      return;
    }
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(response);
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Not Found");
  }
  
  delay(1);
  client.stop();
}

void WiFiController::serveWebInterface(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  client.println("<!DOCTYPE html><html><head>");
  client.println("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<title>RGB Floor Lamp</title>");
  client.println("<style>");
  client.println("*{margin:0;padding:0;box-sizing:border-box}");
  client.println("body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:linear-gradient(135deg,#1a2a6c,#b21f1f,#fdbb2d);");
  client.println("min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}");
  client.println(".container{background:rgba(255,255,255,0.95);backdrop-filter:blur(10px);border-radius:24px;box-shadow:0 20px 60px rgba(0,0,0,0.4);padding:30px;max-width:400px;width:100%}");
  client.println("h1{text-align:center;color:#333;margin-bottom:25px;font-weight:800;letter-spacing:-1px}");
  client.println(".section{margin-bottom:25px}.section-title{font-size:11px;font-weight:700;color:#888;text-transform:uppercase;margin-bottom:12px;letter-spacing:1px}");
  client.println(".button-group{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-bottom:15px}");
  client.println("button{padding:10px;border:none;border-radius:12px;font-size:13px;font-weight:700;cursor:pointer;transition:all 0.2s;box-shadow:0 2px 4px rgba(0,0,0,0.1)}");
  client.println("button:active{transform:scale(0.95)}");
  client.println(".btn-power{grid-column:1/-1;padding:16px;background:#333;color:white;font-size:16px;margin-bottom:10px}");
  client.println(".btn-color{aspect-ratio:1;border-radius:50%}");
  client.println(".btn-anim{color:white;grid-column:span 2;background:#6366f1}");
  client.println(".wheel-container{display:flex;flex-direction:column;align-items:center;gap:15px}");
  client.println("input[type=\"color\"]{width:100px;height:100px;border:none;border-radius:50%;cursor:pointer;background:none;padding:0}");
  client.println("input[type=\"color\"]::-webkit-color-swatch-wrapper{padding:0}");
  client.println("input[type=\"color\"]::-webkit-color-swatch{border:none;border-radius:50%;box-shadow:0 0 15px rgba(0,0,0,0.2)}");
  client.println("input[type=\"range\"]{width:100%;height:6px;border-radius:3px;background:#eee;outline:none;-webkit-appearance:none}");
  client.println("input[type=\"range\"]::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;background:#333;border-radius:50%;cursor:pointer}");
  client.println(".status-pill{display:inline-block;padding:4px 12px;border-radius:20px;font-size:11px;font-weight:800;background:#eee;color:#666}");
  client.println("</style></head><body>");
  
  client.println("<div class=\"container\">");
  client.println("<h1>LUMINA</h1>");
  client.println("<button class=\"btn-power\" onclick=\"togglePower()\">POWER</button>");
  
  client.println("<div class=\"section\"><div class=\"section-title\">Color Wheel</div>");
  client.println("<div class=\"wheel-container\">");
  client.println("<input type=\"color\" id=\"colorPicker\" value=\"#ffffff\" onchange=\"updateFromPicker(this.value)\">");
  client.println("</div></div>");
  
  client.println("<div class=\"section\"><div class=\"section-title\">Presets</div><div class=\"button-group\">");
  client.println("<button class=\"btn-color\" style=\"background:#ff4b2b\" onclick=\"setColor(255,0,0)\"></button>");
  client.println("<button class=\"btn-color\" style=\"background:#2ecc71\" onclick=\"setColor(0,255,0)\"></button>");
  client.println("<button class=\"btn-color\" style=\"background:#00d2ff\" onclick=\"setColor(0,0,255)\"></button>");
  client.println("<button class=\"btn-color\" style=\"background:#fff\" onclick=\"setColor(255,255,255)\"></button>");
  client.println("</div></div>");
  
  client.println("<div class=\"section\"><div class=\"section-title\">Brightness</div>");
  client.println("<input type=\"range\" id=\"brightness\" min=\"5\" max=\"255\" value=\"200\" onchange=\"setBrightness(this.value)\">");
  client.println("</div>");
  
  client.println("<div class=\"section\"><div class=\"section-title\">Animation Speed</div>");
  client.println("<input type=\"range\" id=\"speed\" min=\"10\" max=\"500\" value=\"50\" dir=\"rtl\" onchange=\"setSpeed(this.value)\">");
  client.println("<div style=\"font-size:10px;color:#aaa;text-align:center;margin-top:5px\">FAST &larr; &rarr; SLOW</div>");
  client.println("</div>");
  
  client.println("<div class=\"section\"><div class=\"section-title\">Moods</div><div class=\"button-group\">");
  client.println("<button class=\"btn-anim\" onclick=\"setAnimation(1)\">Rainbow</button>");
  client.println("<button class=\"btn-anim\" onclick=\"setAnimation(2)\">Pulse</button>");
  client.println("<button class=\"btn-anim\" onclick=\"setAnimation(3)\">Chase</button>");
  client.println("<button class=\"btn-anim\" onclick=\"setAnimation(5)\">Fade</button>");
  client.println("<button class=\"btn-anim\" style=\"background:#ff8c00\" onclick=\"setAnimation(6)\">Sunrise</button>");
  client.println("<button class=\"btn-anim\" style=\"background:#9d50bb\" onclick=\"setAnimation(7)\">Sunset</button>");
  client.println("<button class=\"btn-anim\" style=\"background:#00d2ff\" onclick=\"setAnimation(8)\">Waves</button>");
  client.println("<button class=\"btn-anim\" style=\"background:#f00000\" onclick=\"setAnimation(9)\">Flame</button>");
  client.println("</div></div>");
  
  client.println("<div style=\"text-align:center\"><span class=\"status-pill\" id=\"lampStatus\">OFFLINE</span></div>");
  client.println("</div>");
  
  client.println("<script>");
  client.println("function apiCall(p,b){fetch(p,b?{method:'POST',body:JSON.stringify(b)}:{}).then(r=>r.json()).catch(e=>console.error(e))}");
  client.println("function togglePower(){apiCall('/api/power/toggle')}");
  client.println("function setColor(r,g,b){apiCall('/api/color/set',{r:r,g:g,b:b})}");
  client.println("function updateFromPicker(hex){const r=parseInt(hex.slice(1,3),16),g=parseInt(hex.slice(3,5),16),b=parseInt(hex.slice(5,7),16);setColor(r,g,b)}");
  client.println("function setBrightness(v){apiCall('/api/brightness/set',{brightness:parseInt(v)})}");
  client.println("function setSpeed(v){apiCall('/api/speed/set',{speed:parseInt(v)})}");
  client.println("function setAnimation(t){apiCall('/api/animation/set',{type:t})}");
  client.println("function updateStatus(){fetch('/api/status').then(r=>r.json()).then(d=>{document.getElementById('lampStatus').innerText=d.power?'ONLINE':'SLEEP';document.getElementById('lampStatus').style.background=d.power?'#2ecc71':'#eee';document.getElementById('lampStatus').style.color=d.power?'white':'#666';document.getElementById('brightness').value=d.brightness;document.getElementById('speed').value=d.speed}).catch(e=>{document.getElementById('lampStatus').innerText='OFFLINE'})}");
  client.println("setInterval(updateStatus,2000);updateStatus();");
  client.println("</script></body></html>");
}

void WiFiController::printNetworkInfo() const {
  Serial.println("\n=== WiFi Info ===");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Open http://" + WiFi.localIP().toString() + " in your browser");
  Serial.println("=================\n");
}
