# ESP8266_Simple_cli
本项目是一个简单的通过串口配置NodeMcu的固件代码，使用前请先看代码注释。
QQ群：475512169

ESP8266附加开发板管理网址：http://arduino.esp8266.com/stable/package_esp8266com_index.json
ESP32附加开发板管理网址：https://dl.espressif.com/dl/package_esp32_dev_index.json
![Image text](https://github.com/wwhai/ESP8266_Simple_cli/blob/master/img/%E5%9B%BE%E7%89%871.png)

搜索ESP会出现对应的ESP8266、ESP32的开发板库文件，点击进行安装
![Image text](https://github.com/wwhai/ESP8266_Simple_cli/blob/master/img/%E5%9B%BE%E7%89%872.png)
![Image text](https://github.com/wwhai/ESP8266_Simple_cli/blob/master/img/%E5%9B%BE%E7%89%873.png)
搜索cli中使用到的库，点击进行安装
![Image text](https://github.com/wwhai/ESP8266_Simple_cli/blob/master/img/%E5%9B%BE%E7%89%874.png)
![Image text](https://github.com/wwhai/ESP8266_Simple_cli/blob/master/img/%E5%9B%BE%E7%89%875.png)

## 串口提示配置wifi信息
烧录入SDK后根据串口提示进行wifi信息的配置
## HTTP配置
```
/*
 * http配置
*/
const char* host = "192.168.2.103";
const int Port = 2500;
String url = "/data/api/in";
const char* clientId = "85C05BE9296E49EBAACA1A4BF77AD0E2";
```

```
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
```
数据以json形式打包传输

## mqtt
```
typedef struct SysConfig
{
  char ssid[32];
  char password[32];
  char mqttServer[64] = "192.168.1.100";
  char mqttClientId[64] = "testEsp";
  char mqttUsername[64] = "system";
  char mqttPassword[64] = "password";
  int mqttPort = 1883;
};
```
在此配置mqtt相关参数，数据传输同上




