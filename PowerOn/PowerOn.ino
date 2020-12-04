/*
    此文件需安装Arduino esp8266开发环境支持，环境搭建参见：http://www.bigiot.net/talk/237.html
    本程序可以用来控制四路继电器
    ESP8266烧入此程序直接，使用高低电频控制光耦继电器来控制电灯
    我的继电器默认高电频关闭，所以在初始化时都初始化为高电频，play关闭开启，stop关闭关闭，输入1-4打开或关闭对应的引脚
    代码基于https://github.com/bigiot/bigiotArduino/blob/master/examples/ESP8266/kaiguan/kaiguan.ino
    上的代码进行调整，修复了部分bug，解决了断线重连问题，此代码可以直接烧入到nodemcu模块，分享代码希望对大家有帮助
*/
#include "Auto_PowerOn.h"
#include <ESP8266WiFi.h>
#include <aJSON.h>
//=============  此处必须修该============
String DEVICEID="20123"; // 你的设备编号 ==
String  APIKEY = "5003b1e59"; // 设备密码==
//==================BIGIot===================
unsigned long lastCheckInTime = 0; //记录上次报到时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次

const char* ssid     = "CaoNiMa";//无线名称
const char* password = "jiaolei2b";//无线密码

const char* host = "www.bigiot.net";
const int httpPort = 8181;

//===================Led=================
int LED_Pin = D6;
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_Pin,OUTPUT);
  digitalWrite(LED_Pin,HIGH);
  WiFi.begin(ssid, password);
  Wol_Init(Wol_buf,Pc_mac,Local_port);
}

WiFiClient client;

void loop() {

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Use WiFiClient class to create TCP connections
  if (!client.connected()) {
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      delay(5000);
      return;
    }
  }

  if(millis() - lastCheckInTime > postingInterval || lastCheckInTime==0) {
    checkIn();
  }
  
  // Read all the lines of the reply from server and print them to Serial
  if (client.available()) {
    String inputString = client.readStringUntil('\n');
    inputString.trim();
    Serial.println(inputString);
    int len = inputString.length()+1;
    if(inputString.startsWith("{") && inputString.endsWith("}")){
      char jsonString[len];
      inputString.toCharArray(jsonString,len);
      aJsonObject *msg = aJson.parse(jsonString);
      processMessage(msg);
      aJson.deleteItem(msg);          
    }
  }
}

void processMessage(aJsonObject *msg){
  aJsonObject* method = aJson.getObjectItem(msg, "M");
  aJsonObject* content = aJson.getObjectItem(msg, "C");     
  aJsonObject* client_id = aJson.getObjectItem(msg, "ID");
  if (!method) {
    return;
  }
    String M = method->valuestring;
    if(M == "say"){
      String C = content->valuestring;
      String F_C_ID = client_id->valuestring;
      if(C == "play"){
        Serial.println("********play********");
        digitalWrite(LED_Pin,LOW);
        Wol_Send_packet(Wol_buf,Boardcast_addr,Remote_port);
        sayToClient(F_C_ID,"LED All on!");    
      }else if(C == "stop"){
        Serial.println("********stop********");
        digitalWrite(LED_Pin,HIGH);
        sayToClient(F_C_ID,"LED All off!");    
      }else{
        int pin = C.toInt();
        Serial.println("********show********");             
        sayToClient(F_C_ID,"LED pin:"+pin); 
      }
    }
}

void checkIn() {
    String msg = "{\"M\":\"checkin\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
    client.print(msg);
    lastCheckInTime = millis(); 
}

void sayToClient(String client_id, String content){
  String msg = "{\"M\":\"say\",\"ID\":\"" + client_id + "\",\"C\":\"" + content + "\"}\n";
  client.print(msg);
  lastCheckInTime = millis();
}
