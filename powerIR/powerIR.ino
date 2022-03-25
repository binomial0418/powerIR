#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>

// wifi
const char* ssid     = "INSKY_HOME";
const char* password = "0988085240";
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
//SoftwareSerial ir_Serial(2,3);
SoftwareSerial ir_Serial(13,15);

void setup() {
  Serial.begin(9600);
  ir_Serial.begin(115200);
  //連線wifi
  Serial.println(F("Wait for WiFi... "));
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.print("WiFi connected  IPaddress:");
  Serial.print(WiFi.localIP());
}

void GetCmd(String dev){
  String req;
  String encode;
  String s;
  int    iLnk = 0;
 
  if (client.connect("inskychen.com", 80)) {
    iLnk = 1;
  }
  while(iLnk ==0){
    delay(500);
    Serial.println(F("\nreconnect php srv"));
    if (client.connect("inskychen.com", 80)) {
      iLnk = 1;
    }  
  }
  // This will send the request to the server 
  client.print(String("GET ") +"/getircmd.php?ir_no=" + dev + 
               " HTTP/1.1\r\n" +
               "Host:inskychen.com \r\n"+
               "Connection: close\r\n\r\n");
  // 讀取連線情況
  while (client.connected()) {
    String line = client.readStringUntil('\n');
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
  }           
  if (req.length() > 0){
    ExecIrCmd(req);  
  }
  client.stop();
}

void ExecIrCmd(String cmd){
  delay(1000);
  unsigned char code1[] = {0xe3,0x01};
  unsigned char code2[] = {0xe3,0x02};
  unsigned char code3[] = {0xe3,0x03};

  if (cmd == "StandBy"){
    Serial.println("\nIR Sending... StandBy");
    ir_Serial.write(code1,2);
    Serial.println(ir_Serial.read());
  }
  if (cmd == "AirH"){
    Serial.println("\nIR Sending... AirH");
    ir_Serial.write(code2,2);
    Serial.println(ir_Serial.read());
  }
  if (cmd == "DryHot"){
    Serial.println("\nIR Sending... DryHot");
    ir_Serial.write(code3,2);
    Serial.println(ir_Serial.read());
  }
}
void loop() {
  GetCmd("bath1");
  delay(2000);  
}
