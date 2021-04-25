#include <ESP8266WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
#include <ArduinoJson.h>
#include <SoftwareSerial.h>



/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "666"
#define WIFI_PASSWD       "12345678"


/* 设备的三元组信息*/
#define PRODUCT_KEY       "" 
#define DEVICE_NAME       ""
#define DEVICE_SECRET     ""
#define REGION_ID         "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY

#define CLIENT_ID         "20210413|securemode=3,timestamp=1234567890,signmethod=hmacsha1|"
// 算法工具: http://iot-face.oss-cn-shanghai.aliyuncs.com/tools.htm 进行加密生成password
// password教程 https://www.yuque.com/cloud-dev/iot-tech/mebm5g
#define MQTT_PASSWD       ""

#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_SUB    "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"
#define ALINK_TOPIC_PROP_POST    "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

int Button = 13;//按键
char trans;//软串口传输的消息
bool state;//继电器状态
unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient  client(espClient);
SoftwareSerial mySerial(4, 5); // RX, TX 软串口引脚
void setup()
{
    pinMode(Button, INPUT_PULLUP);// 按键引脚设置上拉输入
    Serial.begin(115200);//串口波特率设置
    Serial.println("Demo Start");
    mySerial.begin(9600);//软串口波特率设置
    wifiInit();
}


// the loop function runs over and over again forever
void loop()
{
    if (mySerial.available())//软串口通讯
    {
        trans = char(mySerial.read());//作为字符串接收字符，强制类型转换
        if (trans == '1') state = 1;
        if (trans == '0') state = 0;
        Serial.println(trans);
    }
    if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect();
    }
    client.loop();
    mqttIntervalPost();//mqtt发布消息
    key();//按键程序
}

void callback(char* topic, byte* payload, unsigned int length)//MQTT消息订阅
{
   
    StaticJsonBuffer<1024>JsonBuffer1;
    JsonObject& root1 = JsonBuffer1.parseObject(payload);
    String JDQ = root1["JDQ"];//json数据解析，更改为自己在阿里云设置的标识符
    // Serial.println(JDQswitch);
    if (JDQ == "0") {
        mySerial.println("0");
    }
    else if (JDQ == "1") {
        mySerial.println("1");
    }
}
void mqttIntervalPost()
{
    char param[32];
    char jsonBuf[128];
    sprintf(param, "{\"JDQstate\":%d}",state);//向标识符JDQstate发布当前的开关状态
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
}
void wifiInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }

    Serial.println("Connected to AP");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


    Serial.print("espClient [");


    client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接WiFi之后，连接MQTT服务器 */
    client.setCallback(callback);

}
void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))

        {
            Serial.println("MQTT Connected!");
            client.subscribe(ALINK_TOPIC_PROP_SUB);

        }
        else
        {
            Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);
        }
    }
}
void key()
{
   
    if (digitalRead(Button) == LOW)
    {
        delay(20);
        if (digitalRead(Button) == LOW)
        {
            mySerial.println("2");//按键按下后，软串口发送数据2
            
            while (digitalRead(Button) == LOW);
        }
    }
}
