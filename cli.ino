
#include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Ethernet.h>
#include <ACROBOTIC_SSD1306.h>
#include <Arduino_JSON.h>
#include "DHTesp.h"
///
#define RED_LED_GPIO 12
#define GREEN_LED_GPIO 13
#define BLUE_LED_GPIO 15
#define DHT11_GPIO 5
#define BUZZER_GPIO  14

/*
 * http配置
*/
const char* host = "192.168.2.103";
const int Port = 2500;
String url = "/data/api/in";
const char* clientId = "85C05BE9296E49EBAACA1A4BF77AD0E2";
/*
   联网不一定连MQTT服务器
   不联网一定不连MQTT服务器
*/
DHTesp dht;
boolean isConnectToWifi ;
boolean isConnectToMqtt ;
typedef struct SysConfig
{
  char ssid[32];
  char password[32];
  char mqttServer[64] = "broker.mqtt-dashboard.com";
  char mqttClientId[64] = "e48f00dbccec45523bdcfcce8fa5ddcb9557f803";
  char mqttUsername[64] = "e48f00dbccec45523bdcfcce8fa5ddcb9557f803";
  char mqttPassword[64] = "e48f00dbccec45523bdcfcce8fa5ddcb9557f803";
  int mqttPort = 1883;
};

SysConfig sysConfig;
/**
   连接Mqtt用的
*/
WiFiClient espClient;
PubSubClient mqttClient("broker.mqtt-dashboard.com", 1883, espClient);
/**
   OLED 显示屏
*/
void display(int line, int x, boolean isClear, char content[]) {
  if (isClear) {
    oled.clearDisplay();
    oled.setTextXY(0, 0);
    oled.putString(content);
  } else {
    oled.setTextXY(0, 0);
    oled.putString(content);
  }

}
/**
   加载配置
*/

void loadSysConfig() {
  EEPROM.begin(sizeof(SysConfig));
  EEPROM.get(0, sysConfig);
  Serial.println("--------Load wifi SysConfig from EPROM--------");
  Serial.print("|SSID: ");
  Serial.print(sysConfig.ssid);
  Serial.print("\t Password: ");
  Serial.println(sysConfig.password);
  Serial.println("----------------------------------------------");

}
/**
   写入配置到EPROM
*/

void writeSysConfig() {
  EEPROM.begin(sizeof(SysConfig));
  EEPROM.put(0, sysConfig);
  EEPROM.commitReset();
}
/**
   打印banner
*/
void welcome() {
  Serial.println("");

  Serial.println("**********************************************");
  Serial.println("*     Welcome use NodeMcu Cli Mode V0.0.1    *");
  Serial.println("*     Send 'help' for more information       *");
  Serial.println("**********************************************");

}

/**
   Help
*/
void showHelp() {

  Serial.println("");
  Serial.println("**********************************************");
  Serial.println("|SysConfig Wlan: wlanset -s {SSID} -p {Password}|");
  Serial.println("|Print Wlan Info:wlan                        |");
  Serial.println("|Test Buzzer {n} times: beep {n}             |");
  Serial.println("|Print tempature:temp                        |");
  Serial.println("|Print tempature:hum                         |");
  Serial.println("**********************************************");
}
void echoTemp() {
  Serial.println("*------------------------------------------*");
  Serial.print("|             Current tempature is:      ");
  Serial.print(35);  Serial.println("|");
  Serial.println("*------------------------------------------*");
}
void echoHumidity() {
  Serial.println("*------------------------------------------*");
  Serial.print("|     Current humidity:             |");
  Serial.print(35);  Serial.println("|");
  Serial.println("*------------------------------------------*");
}
void echoWlan() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("*------------------------------------------*");
    Serial.print("|state:");
    Serial.print("ON\t");
    Serial.print("     \tIP:");
    Serial.println(WiFi.localIP());
    Serial.println("*------------------------------------------*");
  } else {
    Serial.println("|Network has disconnected.");
  }

}
/**
   配置网络
*/
void setupWifi() {
  Serial.println("-----------------SetUp wifi-------------------");
  Serial.println("|Try  connect to network.");
  Serial.print("|");
  WiFi.begin(sysConfig.ssid, sysConfig.password);
  for (int i = 0 ; i < 10; i++) {
    Serial.print("=>");
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.hostname("ESP8266_NODE_1");
      Serial.println("\n|Success connect to network.");
      isConnectToWifi = true;
      writeSysConfig();
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n|Connect to network failure.Please try again!");
  }

}
/**
   初始化MQTT
*/
void initMqtt() {
  Serial.println("-----------------SetUp mqtt-------------------");
  mqttClient.setCallback([](char* topic, byte * payload, unsigned int length)-> void{
    String msg ;
    for (int i = 0 ; i < length; i++) {
      msg += (char )payload[i];
    }
    JSONVar dataJson = JSON.parse(msg);
    if (JSON.typeof(dataJson) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    } else{
      Serial.println(dataJson);

    }


  });
  for (int i = 0 ; i < 10; i++) {
    //mqttClient.connect("NodeMcu10010");
    mqttClient.connect(sysConfig.mqttClientId, sysConfig.mqttUsername, sysConfig.mqttPassword);
    Serial.print("=>");
    delay(1000);
    if (mqttClient.connected()) {
      //连接成功
      mqttClient.subscribe(String(String("/device/OUT/") + sysConfig.mqttClientId).c_str(), 2);
      isConnectToMqtt = true;
      Serial.println("\n|Connect to mqtt success!");
      break;
    }
  }
  if (!mqttClient.connected() ) {
    isConnectToMqtt = false;
    Serial.println("\n|Connect to mqtt failure.Please try again!");
  }

}
/**
   Mqtt配置:
   准备使用clientId作为Topic
   mqttset|username|password|clientId

*/
void setupMqtt(String cmd) {
  Serial.println("|Start setup mqtt");
}

/**
   配置WIFI
*/
void setWlan( String cmd) {
  // wlanset|206-public|665544332211
  Serial.println("|Start SysConfig wifi");
  String ssid = cmd.substring(cmd.indexOf("|") + 1, cmd.lastIndexOf("|"));
  String password = cmd.substring( cmd.lastIndexOf("|") + 1, cmd.length());
  ssid.trim();
  password.trim();
  if (ssid.length() > 0 && password.length() > 0 ) {
    Serial.print("|Wifi SSID:"); Serial.println(ssid);
    Serial.print("|Password:"); Serial.println(password);
    SysConfig &tempCfg = sysConfig;
    strcpy(tempCfg.ssid, ssid.c_str());
    strcpy(tempCfg.password, password.c_str());
    setupWifi();

  } else {
    Serial.println("SSID and password must not null,command is : wlanset|ssid|password!");
  }



}


/**
   初始化GPIO
*/

void initialGpio() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(14, OUTPUT);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(15, LOW);
  Wire.begin();
  oled.init();
  dht.setup(DHT11_GPIO, DHTesp::DHT11);

}

/**
   指示灯：三色RGB指示灯，可以组合出6种颜色；
   r g b  含义
   0 000  断电
   1 001  联网
   2 010  联网但是MQTT没有连上
   3 011  联网+MQTT连上
   4 100  断网
   5 101  待定
   6 111  待定
*/
void blinkRGB(int pin) {
  digitalWrite(pin, LOW);
  delay(1000);
  digitalWrite(pin, HIGH);
  delay(1000);
}
/**
   蜂鸣器 嘀嘀嘀
   beep|n
*/
void buzzerBeep(int times) {
  Serial.print("|Beep: ");
  Serial.println(times);
  for (int i = 0; i < times; i++) {
    tone(BUZZER_GPIO, 1500); // Send 1MHz sound signal...
    delay(1000);        // ...for 1 sec
    noTone(BUZZER_GPIO);     // Stop sound...
    delay(1000);        // ...for 1sec
  }
}


/**
   初始化
*/
void setup() {
  buzzerBeep(1);
  delay(1000);
  initialGpio();
  display(0, 0, true, "Hello world!NodeMcu!");
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  welcome();
  delay(1000);
  loadSysConfig();
  delay(1000);
  setupWifi();
  delay(1000);
  initMqtt();

}
/*
 *  读取DHT11
*/
JSONVar getDHT11Data() {
  JSONVar data;
  delay(dht.getMinimumSamplingPeriod());
  //湿度
  float h = dht.getHumidity();
  //摄氏度
  float c = dht.getTemperature();
  //华氏度
  float f = dht.toFahrenheit(c);
  //热指数
  float cc = dht.computeHeatIndex(c, h, true);
  float ff = dht.computeHeatIndex(f, h, true);

  data["h"] = h;
  data["c"] = c;
  data["f"] = f;
  data["cc"] = cc;
  data["ff"] = ff;
  return data;
}


JSONVar jsonDataPackage(JSONVar j2strData)
{
  JSONVar jsonData;
  jsonData["clientId"] = clientId;
  jsonData["value"] = JSON.stringify(j2strData);

  return jsonData;
}

String post2server(JSONVar jdata)
{
   String line;
   if(!espClient.connect(host,Port))
   {
    return "connection failed";
   }
   String data=JSON.stringify(jdata);
   int length=data.length();
   String postRequest =(String)("POST ") +url+ "/ HTTP/1.1\r\n" +
          "Content-Type: application/json;charset=utf-8\r\n" +
          "Host: " + host + ":" + Port + "\r\n" +
          "User-Agent: ESP8266\r\n"+          
          "Content-Length: " + length + "\r\n" +
          "Connection: Keep Alive\r\n\r\n" +
          data+"\r\n";
   espClient.print(postRequest);
   delay(100);
    line = espClient.readStringUntil('\n');
    while (espClient.available() > 0) {
      line += espClient.readStringUntil('\n');
    }
   return line;
}





//
int count = 0;
void loop() {
  count += 1;
  if (isConnectToMqtt) {
    mqttClient.loop();
    if (count >= 100000) {
      //mqttClient.publish("/device/IN/NodeMcu10010", JSON.stringify(getDHT11Data()));
      Serial.println(post2server(jsonDataPackage(getDHT11Data())));
      Serial.println(JSON.stringify(getDHT11Data()));
      count = 0;

    }
  }



  String cmd = "";
  if (Serial.available() > 0) {
    cmd = Serial.readStringUntil('\n');
    delay(100);
  }
  if (cmd.length() > 0) {
    if (cmd.equals(String("help"))) {
      showHelp();
    }
    if (cmd.equals(String("wlaninfo"))) {
      echoWlan();
    }
    if (cmd.equals(String("temp"))) {
      echoTemp();
    }
    if (cmd.equals(String("hum"))) {
      echoHumidity();
    }
    /**
       led 闪烁
    */
    if (cmd.equals(String("red"))) {
      blinkRGB(12);
    }
    if (cmd.equals(String("green"))) {
      blinkRGB(13);
    }
    if (cmd.equals(String("blue"))) {
      blinkRGB(15);
    }
    if (cmd.startsWith(String("wlanset|"))) {
      setWlan(cmd);
    }
    if (cmd.startsWith(String("mqttset|"))) {
      setWlan(cmd);
    }
    if (cmd.startsWith(String("beep|"))) {
      int times = cmd.substring( cmd.lastIndexOf("|") + 1, cmd.length()).toInt();
      buzzerBeep(times);
    }

  }

  // wlanset|206-public|665544332211
  if ( isConnectToWifi) {
    // blue
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
    digitalWrite(15, HIGH);
    //if connect to mqtt
    if (isConnectToWifi && isConnectToMqtt) {
      //Green
      digitalWrite(12, LOW);
      digitalWrite(13, HIGH);
      digitalWrite(15, LOW);
    }
  } else {
    //Red
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    digitalWrite(15, LOW);
  }
}
