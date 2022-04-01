//1.啟動後從nodeMCU的ROM中取出WIFI設定。
//2.若取得合理WIFI密碼，則連線WIFI。若連線失敗自動進入AP模式讓使用者連線重設帳密。
//3.若沒有合理帳密，或是連續按三下flash鍵自動進入AP模式讓使用者連線重設帳密。
//4.進入AP模式後，請連接WIFI名稱為ESPXXXX，之後進入192.168.4.1設定WIFI。
//5.PMS5003T空氣品質感測器接線方式DS18S20 Signal pin on D7


//#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <EasyButton.h>
#include <SoftwareSerial.h>

//設定flash鍵
// Arduino pin where the button is connected to.
#define BUTTON_PIN 0

// Instance of the button.
EasyButton button(BUTTON_PIN);

SoftwareSerial ir_Serial(13,15);

//WIFI相關變數
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
const char* ap_ssid       ;
const char* ap_password   ;
//const char* machine_nam   ;
const char* WIFI_FILE     = "/wifi.ini"; //存在ROM中的 wifi設定檔名稱

//進入AP模式後，nodeMCU啟動的網頁內容
AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "password";
const char* PARAM_INPUT_3 = "machine_nam";
bool        goAPMode      = true;
String mv_Machine_nam;
int resetCount = 1440; //重啟計數器

boolean light = false;

// HTML web page to handle 2 input fields (input1, input2)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>  
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  </head><body>
  <form action="/get">
  <div style="border:10px orange solid;padding:15px;font-size:40px">
    請設定WIFI SSID與密碼：<br>
    <table>
    <tr><td>WIFI SSID    :</td><td> <input type="text" name="ssid" style="font-size:40px;"></td></tr>
    <tr><td>WIFI Password:</td><td> <input type="text" name="password" style="font-size:40px;"></td></tr>
    <tr><td>機器說明:</td><td> <input type="text" name="machine_nam" style="font-size:40px;"></td></tr>
    <tr><td><input type="submit" value="確認" style="font-size:80px;"></td></tr>
    </table>
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  String wifiIni;
 
  String ssid        = ""  ;
  String password    = ""  ;
  String machine_nam = ""  ;
  int timeoutCount   = 30;
  boolean flash = false;
  
  Serial.begin(115200);
  ir_Serial.begin(9600);
  
  //設定flash動作
  // Initialize the button.
  button.begin();
  // Add the callback function to be called when the button is pressed.
  //button.onPressed(onPressed); 
  button.onSequence(3, 1500, sequenceEllapsed);
  if (button.supportsInterrupt())
  {
    button.enableInterrupt(buttonISR);
    Serial.println("Button will be used through interrupts");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  //讀取ROM中的wifi設定
  Serial.println(" ");
  wifiIni  = readConfigFile(WIFI_FILE);
  ssid     = getSsidPwdFromJson(wifiIni,"ssid");
  password = getSsidPwdFromJson(wifiIni,"pwd");
  machine_nam = getSsidPwdFromJson(wifiIni,"machine_nam");
  mv_Machine_nam = machine_nam; 
//  Serial.println("Saved wifi data:" + String(ssid) + String(password) );
  Serial.println("Saved ssid:" + String(ssid));
  Serial.println("Saved pwd:" + String(password) );
  Serial.println("Saved machine_nam:" + String(machine_nam));
  
  //有讀取到合理的wifi設定就開始連接WIFI，若無法成功連線，則自動進入AP模式。
  if (ssid.length() > 0 && password.length() > 0 && ssid != "0" && password != "0"){  
    Serial.println("WIFI MODE ");
    Serial.print(F("Wait for WiFi"));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    while (WiFi.status() != WL_CONNECTED && timeoutCount > 0){
      flash = !flash;
      if (flash ==true){
        digitalWrite(LED_BUILTIN, HIGH);  
      }else{
        digitalWrite(LED_BUILTIN, LOW);  
      }
//      pinMode(LED_BUILTIN, flash);
      Serial.print(".");
      delay(1000);
      timeoutCount--;
    }
    goAPMode = timeoutCount == 0;
   
    if (goAPMode == true) {
      Serial.println("WiFi Failed!");
    }else{
      Serial.println();
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());   
      digitalWrite(LED_BUILTIN, LOW);  
      delay(300); 
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);  
      delay(300); 
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);  
      delay(300); 
      digitalWrite(LED_BUILTIN, HIGH); 
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);  
      delay(300); 
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);  
      delay(300); 
      digitalWrite(LED_BUILTIN, HIGH);
    }    
  }
  //wifi連線失敗，進入AP模式
  if (goAPMode == true){
    auto chipID = ESP.getChipId();
    pinMode(LED_BUILTIN, true);   
    Serial.println("AP MODE ");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(String("ESP-") + String(chipID, HEX)); 
    
    //以下處理網頁動作及取得內容
    // Send web page with input fields to client
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send_P(200, "text/html", index_html); 
    });
   //Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String ssid;
    String pwd;
    String nam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      ssid = request->getParam(PARAM_INPUT_1)->value();
      pwd  = request->getParam(PARAM_INPUT_2)->value();
      nam  = request->getParam(PARAM_INPUT_3)->value();
      writeWifiConfigFile(WIFI_FILE,ssid,pwd,nam);
      ESP.restart();
    }
    //轉換網頁，按下http get後網頁跳轉動作
    request->send(200, "text/html", "<a href=\"/\">Return to Home Page</a>");                                     
  });
  server.onNotFound(notFound);
  server.begin();
  }
}


void onPressed() {
    Serial.println("Button has been pressed!");
}
//利用點三下flash按鈕重設wifi資訊
void sequenceEllapsed()
{
  Serial.println("reset wifi data");
  writeWifiConfigFile(WIFI_FILE,"0","0","0");
  ESP.restart();
}
void buttonISR()
{
  /*
    When button is being used through external interrupts, 
    parameter INTERRUPT must be passed to read() function
   */
  button.read();
}
//用從ROM中讀取檔案
String readConfigFile(String typ){
  String s;
  SPIFFS.begin();
  File data = SPIFFS.open(typ, "r");   
  
  size_t size = data.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // Read and store file contents in buf
  data.readBytes(buf.get(), size-2);
  // Closing file
  data.close();
  //Serial.println(String(typ) + ":" + String(buf.get()));
  s = String(buf.get());
  return s;
}

//將WIFI資訊組成JSON寫入ROM中
void writeWifiConfigFile(String typ,String ssid,String pwd,String nam){
  String str;
  StaticJsonDocument<200> json_doc;
  char json_output[100];
  json_doc["ssid"] = ssid + "#";
  json_doc["pwd"]  = pwd + "#";
  json_doc["machine_nam"]  = nam + "#";
  serializeJson(json_doc, json_output);
 
  SPIFFS.begin();
  File f = SPIFFS.open(typ, "w");  
  if (!f) {
    Serial.println("Failed to open config file for writing");
  }
  f.println(json_output);
  f.close();
}

//從JSON字串中取出WIFI資訊
String getSsidPwdFromJson(String val,String field){
  StaticJsonDocument<200> json_doc;
  DeserializationError json_error;
  //const char* ssid;
  //const char* pwd;
  int i;
  
  json_error = deserializeJson(json_doc, val);
  if (!json_error) {
    String s    = String(json_doc[field]);
    i = s.indexOf('#');
    s = s.substring(0,i);
    //Serial.println(field + ":" + s);
    
    return s;
  }
}

void loop()
{  
  if (WiFi.status() == WL_CONNECTED && goAPMode == false){
    GetCmd(mv_Machine_nam);    
    delay(500);
    resetCount--;
  }
  if (resetCount == 0){
    Serial.println("Reset..");
    ESP.restart();
    }
}
void GetCmd(String dev){
  String req;
  String encode;
  String s;
  String line;
  int    i;
  int    iLnk = 0;
 
  if (client.connect("www.host.com", 8088)) {
    iLnk = 1;
  }
  while(iLnk ==0){
    delay(100);
    Serial.println(F("\nreconnect php srv"));
    if (client.connect("www.host.com", 8088)) {
      iLnk = 1;
    }  
  }
  // This will send the request to the server 
  client.print(String("GET ") +"/homesys/httpcmd/getircmd.php?ir_no=" + dev + 
               " HTTP/1.1\r\n" +
               "Host:www.host.com:8088\r\n"+
               "Connection: close\r\n\r\n");
  // 讀取連線情況
  while (client.connected()) {
    line = client.readStringUntil('\n');
//    Serial.println("*"+line+"*");
    //學習指令
    if (line.indexOf("LeanStandBy") > 0){
      req = "LeanStandBy";
      break;
    }
    if (line.indexOf("LeanAirH") > 0){
      req = "LeanAirH";
      break;
    }
    if (line.indexOf("LeanDryHot") > 0){
      req = "LeanDryHot";
      break;
    }
    if (line.indexOf("LeanDryCold") > 0){
      req = "LeanDryCold";
      break;
    }
    if (line.indexOf("LeanAirCdrPower") > 0){
      req = "LeanAirCdrPower";
      break;
    }
    if (line.indexOf("LeanAirCdrSendCold") > 0){
      req = "LeanAirCdrSendCold";
      break;
    }
    if (line.indexOf("LeanAirCdrSendCold") > 0){
      req = "LeanAirCdrSendCold";
      break;
    }
    if (line.indexOf("LeanAirCdrSendAir") > 0){
      req = "LeanAirCdrSendAir";
      break;
    }
    if (line.indexOf("LeanAirCdrUP") > 0){
      req = "LeanAirCdrUP";
      break;
    }
    if (line.indexOf("LeanAirCdrDOWN") > 0){
      req = "LeanAirCdrDOWN";
      break;
    }
    if (line.indexOf("LeanAirCdrFlow") > 0){
      req = "LeanAirCdrFlow";
      break;
    }
    if (line.indexOf("LeanTvPower") > 0){
      req = "LeanTvPower";
      break;
    }
    if (line.indexOf("LeanTvMute") > 0){
      req = "LeanTvMute";
      break;
    }
    if (line.indexOf("StopLean") > 0){
      req = "StopLean";
      break;
    }
    //exec
    if (line.indexOf("StandBy") > 0){
      req = "StandBy";
      break;
    }
    if (line.indexOf("AirH") > 0){
      req = "AirH";
      break;
    }
    if (line.indexOf("DryHot") > 0){
      req = "DryHot";
      break;
    }
    if (line.indexOf("DryCold") > 0){
      req = "DryCold";
      break;
    }
    if (line.indexOf("AirCdrPower") > 0){
      req = "AirCdrPower";
      break;
    }
    if (line.indexOf("AirCdrSendCold") > 0){
      req = "AirCdrSendCold";
      break;
    }
    if (line.indexOf("AirCdrSendCold") > 0){
      req = "AirCdrSendCold";
      break;
    }
    if (line.indexOf("AirCdrSendAir") > 0){
      req = "AirCdrSendAir";
      break;
    }
    if (line.indexOf("AirCdrUP") > 0){
      req = "AirCdrUP";
      break;
    }
    if (line.indexOf("AirCdrDOWN") > 0){
      req = "AirCdrDOWN";
      break;
    }
    if (line.indexOf("AirCdrFlow") > 0){
      req = "AirCdrFlow";
      break;
    }
    if (line.indexOf("TvPower") > 0){
      req = "TvPower";
      break;
    }
    if (line.indexOf("TvMute") > 0){
      req = "TvMute";
      break;
    }
    
  }           
  if (req.length() > 0){
    //Serial.println(req);
    ExecIrCmd(req);  
  }
  client.stop();
}

void ExecIrCmd(String cmd){
  //紅外線執行
  unsigned char code1[] = {0xe3,0x01};//抽風機待機 StandBy
  unsigned char code2[] = {0xe3,0x02};//抽風機抽風強AirH
  unsigned char code3[] = {0xe3,0x03};//抽風機熱風烘乾DryHot
  unsigned char code4[] = {0xe3,0x04};//抽風機冷風烘乾DryCold
  unsigned char code5[] = {0xe3,0x05};//冷氣電源鍵AirCdrPower
  unsigned char code6[] = {0xe3,0x06};//冷氣-冷氣AirCdrSendCold
  unsigned char code7[] = {0xe3,0x07};//冷氣-送風AirCdrSendAir
  unsigned char code8[] = {0xe3,0x08};//冷氣-升溫AirCdrUP
  unsigned char code9[] = {0xe3,0x09};//冷氣-降溫AirCdrDOWN
  unsigned char code10[] = {0xe3,0x10};//冷氣-風量AirCdrFlow
  unsigned char code11[] = {0xe3,0x11};//電視-電源TvPower
  unsigned char code12[] = {0xe3,0x12};//電視-靜音TvMute
  //紅外線學習
  unsigned char leancode1[] = {0xe0,0x01};//抽風機待機 StandBy
  unsigned char leancode2[] = {0xe0,0x02};//抽風機抽風強AirH
  unsigned char leancode3[] = {0xe0,0x03};//抽風機熱風烘乾DryHot
  unsigned char leancode4[] = {0xe0,0x04};//抽風機冷風烘乾DryCold
  unsigned char leancode5[] = {0xe0,0x05};//冷氣電源鍵AirCdrPower
  unsigned char leancode6[] = {0xe0,0x06};//冷氣-冷氣AirCdrSendCold
  unsigned char leancode7[] = {0xe0,0x07};//冷氣-送風AirCdrSendAir
  unsigned char leancode8[] = {0xe0,0x08};//冷氣-升溫AirCdrUP
  unsigned char leancode9[] = {0xe0,0x09};//冷氣-降溫AirCdrDOWN
  unsigned char leancode10[] = {0xe0,0x10};//冷氣-風量AirCdrFlow
  unsigned char leancode11[] = {0xe0,0x11};//電視-電源TvPower
  unsigned char leancode12[] = {0xe0,0x12};//電視-靜音TvMute
  //退出學習
  unsigned char stopcode[] = {0xe2};
  int ir_rev = 0;
  
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("\nIR Sending... " + cmd);
  //紅外線執行 
  if (cmd == "StandBy"){
    ir_Serial.write(code1,2);
  }
  if (cmd == "AirH"){
    ir_Serial.write(code2,2);
  }
  if (cmd == "DryHot"){
    ir_Serial.write(code3,2);
  }
  if (cmd == "DryCold"){
    ir_Serial.write(code4,2);
  }
  if (cmd == "AirCdrPower"){
    ir_Serial.write(code5,2);
  }
  if (cmd == "AirCdrSendCold"){
    ir_Serial.write(code6,2);
  }
  if (cmd == "AirCdrSendAir"){
    ir_Serial.write(code7,2);
  }
  if (cmd == "AirCdrUP"){
    ir_Serial.write(code8,2);
  }
  if (cmd == "AirCdrDOWN"){
    ir_Serial.write(code9,2);
  }
  if (cmd == "AirCdrFlow"){
    ir_Serial.write(code10,2);
  }
  if (cmd == "TvPower"){
    ir_Serial.write(code11,2);
  }
  if (cmd == "TvMute"){
    ir_Serial.write(code12,2);
  }
  //紅外線學習 
  if (cmd == "LeanStandBy"){
    ir_Serial.write(leancode1,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirH"){
    ir_Serial.write(leancode2,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanDryHot"){
    ir_Serial.write(leancode3,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanDryCold"){
    ir_Serial.write(leancode4,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrPower"){
    ir_Serial.write(leancode5,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrSendCold"){
    ir_Serial.write(leancode6,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrSendAir"){
    ir_Serial.write(leancode7,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrUP"){
    ir_Serial.write(leancode8,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrDOWN"){
    ir_Serial.write(leancode9,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanAirCdrFlow"){
    ir_Serial.write(leancode10,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanTvPower"){
    ir_Serial.write(leancode11,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  if (cmd == "LeanTvMute"){
    ir_Serial.write(leancode12,2);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  //退出學習
  if (cmd == "StopLean"){
    ir_Serial.write(stopcode,1);
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
  ir_rev = ir_Serial.read();
  Serial.println(ir_rev);
  if (ir_rev > 0){
    clean_ir_cmd(mv_Machine_nam,cmd);
  }
 
  digitalWrite(LED_BUILTIN, HIGH);
}

void clean_ir_cmd(String typ,String cmd){
  int iLnk = 0;
  String s;
 
  if (client.connect("www.host.com", 8088)) {
    iLnk = 1;
  }
  while(iLnk ==0){
    delay(100);
    Serial.print(F("\nreconnect php srv"));
    if (client.connect("www.host.com", 8088)) {
      iLnk = 1;
    }   
  }
  
  s = "GET /homesys/httpcmd/clnircmd.php?ir_no=" + typ + "&cmd=" + cmd + 
      " HTTP/1.1\r\n" +
      "Host:www.host.com:8088\r\n"+
      "Connection: close\r\n\r\n";
  Serial.print(s);
  client.println(s);
  String line = client.readStringUntil('\r');                
  client.stop();
}
