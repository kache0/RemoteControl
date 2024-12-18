#include <Arduino.h>
#include <Wifi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi SSID 和 密碼
const char* ssid = "35";
const char* password = "93090909";
//用戶 ID
const int PASSWORD = 27835;
// Power control pin
const int power = 4;
const int restartPower = 2;
WebServer server(80);  

unsigned long previousMillis = 0;  // 上次檢測Wi-Fi的時間
const long interval = 30000;  // 30秒時間檢測一次

void reconnectWiFi() { //預防WiFi斷線 並重新連線
  WiFi.begin(ssid,password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(2000);  
    WiFi.begin(ssid,password);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi reconnected successfully");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi reconnect failed.");
  }
} 

void loop_WiFi_detect() { //計算時間差 避免loop多任務阻塞
  unsigned long currentMillis = millis();  // 得到當前時間
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Trying to reconnect...");
      reconnectWiFi();
    } else {
      Serial.println("WiFi is connecting...");
    }
  }
}


void handleRoot () { //返回字符串內容
  String HTML = "<DOCTYPE html>\
  <html>\
  <head><meta chardset = 'urf-8'></head>\
  <body>\
  </body>\
  </html>";
  server.send(200, "text/html", HTML); 
}

void powerSW(String state) { //控制電源開機、關機、強制關機、重啟
  if (state == "PowerOn/Off") { 
      digitalWrite(power, LOW);
      delay(1500);
      digitalWrite(power, HIGH);
    Serial.println("PowerOn/Off");
  } else if (state == "PowerForceOff") {
      digitalWrite(power, LOW);
      delay(6000);
      digitalWrite(power, HIGH);
      Serial.println("PowerForceOff");
  } else if (state == "PowerReboot") {
      digitalWrite(restartPower, LOW);
      delay(1500);
      digitalWrite(restartPower, HIGH);
      Serial.println("PowerReboot");
  } else if(state == "test") {
      Serial.println("Test");
  } else {
      Serial.println("Invalid state");
      server.send(400, "application/json", "{\"error\":\"Invalid state\"}");
      return;
  }
  server.send(200, "text/html", "POWER IS <b>" + state + "</b>");
}

void handlePostRequest() {
  if (server.method() == HTTP_POST) {
    
    String body = server.arg("plain");  // 取得 POST 數據
    Serial.println("Received POST Data:");
    Serial.println(body);

    // 解析 JSON 数据
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    const int password = doc["password"];
    const char* state = doc["state"];
    if (error) { // 錯誤 JSON 格式
      Serial.println("Failed to parse JSON!");
      server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
      return;
    } else if(password != 27835) { // 錯誤用戶 ID
      Serial.println("Invalid user ID!");
      server.send(401, "application/json", "{\"error\":\"Invalid password\"}");
      return;
    } else if(password == 27835) { // 正確用戶 ID
      server.send(200, "application/json", "{\"message\":\"Data received!\"}");
      powerSW(state);
    }
  } else { // 錯誤請求方式
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

void setup() { //初始化
  Serial.begin(115200);
  digitalWrite(power, HIGH);
  pinMode(power, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //連接WiFi
  while (WiFi.status() != WL_CONNECTED) { //確保WiFi第一次一定連得上
    Serial.print(".");
    WiFi.begin(ssid, password); //連接WiFi
    delay(2000);  // 等待 Wi-Fi 連接
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/powerSW", HTTP_POST, handlePostRequest); //創建POST請求伺服器
  server.on("/", handleRoot); //創建根目錄請求伺服器
  server.begin();
}
                                            
void loop() { 
  loop_WiFi_detect(); //檢測WiFi連接
  server.handleClient();
}