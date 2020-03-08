#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <Wire.h>
#include <DHT.h>
#include "icon.h"

// 按照下列连接引脚
/*  ESP8266     D1      D2      D3      D4
 *  OLED                       SDA     SCL
 *  DHT11      Data
 *  FAN                VCC
 */

// OLED主控型号
//U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D4, /* data=*/ D3, /* reset=*/ U8X8_PIN_NONE);//SH1106
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D4, /* data=*/ D3, /* reset=*/ U8X8_PIN_NONE);//SSD1306

DHT dht(D1, DHT11);
WiFiClient client;
Ticker flipper;
WiFiUDP Udp;

const char* ssid     = "wifi-ssid";          // 需要访问的端口
const char* password = "wifi-password";      // 需要访问的端口
const char* host     = "192.168.2.10";       // 需要访问的IP
const int   port     = 9999;                 // 需要访问的端口
static const char ntpServerName[] = "ntp.aliyun.com"; // NTP域名
const int timeZone   = 8;                    
unsigned int NTPPort = 8888;                 
String nowTime, nowDay;                      
float humi, temp;                            
time_t prevDisplay   = 0;                    
byte i = 1;
boolean isNTPConnected = false, PConline = false;  
int CPU_FREQ, GPU_FREQ, CPU_TEMP, GPU_TEMP, CPU_FAN, GPU_FAN, GPU_USE, CPU_USE, RAM_USE, CASE_FAN;
byte fanSpeed, startTemp = 30, fullTemp = 50;      

time_t getNtpTime();                    
void sendNTPpacket(IPAddress &address); 
void dataProcess(String s);            
void httprequest();            
void ClockDisplay();             
void fanControl();                    
void u8g2Init();                     
void drawTime();                     
void drawGPU();                          
void drawCPU();                          

void setup() {
  u8g2Init();
  dht.begin();
  Serial.begin(115200);
  Serial.println("Start!");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  delay(500);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
  Udp.begin(NTPPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);    
  flipper.attach(5, flip);
  pinMode(D2, OUTPUT);
}

void loop() {
  httprequest();
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  fanControl();
  if (timeStatus() != timeNotSet){
    if (now() != prevDisplay){
      prevDisplay = now();
      ClockDisplay();
    }
  }
  if(i == 1 && PConline){
    drawCPU();
  }
  else if(i == 2 && PConline){
    drawGPU();
  }
  else if(i == 3 || !PConline){
    drawTime();
  }
  delay(500);
}

void flip() {
  i++;
  if(i>=4){
    i=1;
  }
}

void u8g2Init(void) {
  u8g2.begin();
  u8g2.setFontPosTop();   
  u8g2.setFontDirection(0);
  u8g2.drawXBMP(0, 16, 128,32, mianlogo);
  u8g2.sendBuffer();
}

void drawTime(void){
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_crox5h_tn);
    u8g2.setCursor(5,0);
    u8g2.print(nowDay);
    u8g2.drawXBMP(10, 30, 16, 16, timeicon);
    u8g2.setCursor(26,22);
    u8g2.print(nowTime);
    u8g2.setFont(u8g2_font_crox3t_tn);
    u8g2.drawXBMP(0, 48, 16, 16, tempicon);
    u8g2.drawXBMP(56, 48, 8, 16, celicon);
    u8g2.setCursor(18,46);
    u8g2.print(temp);
    u8g2.drawXBMP(64, 48, 16, 16, humiicon);
    u8g2.drawXBMP(120, 48, 8, 16, pericon);
    u8g2.setCursor(82,46);
    u8g2.print(humi);
  } while ( u8g2.nextPage() );
}

void drawGPU(void){
  u8g2.setFont(u8g2_font_crox3h_tf);
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(8, 16, 64, 32, gpulogo);
    // 时钟
    //u8g2.drawXBMP(0, 0, 16, 16, timeicon);
    u8g2.setCursor(10,0);
    u8g2.print(nowTime);
    // 散热架转速
    u8g2.drawXBMP(74, 0, 16, 16, fanicon);
    //u8g2.drawXBMP(120, 0, 8, 16, pericon);
    u8g2.setCursor(90,0);
    u8g2.print(GPU_FAN);
    // RAM使用率
    u8g2.drawXBMP(80, 15, 16, 16, ramicon);
    u8g2.drawXBMP(120, 16, 8, 16, pericon);
    u8g2.setCursor(100,16);
    u8g2.print(RAM_USE);
    // GPU温度
    u8g2.drawXBMP(80, 31, 16, 16, tempicon);
    u8g2.drawXBMP(120, 32, 8, 16, celicon);
    u8g2.setCursor(100, 32);
    u8g2.print(GPU_TEMP); 
    // GPU使用率
    u8g2.drawXBMP(80, 48, 16, 16, useicon);
    u8g2.drawXBMP(120, 48, 8, 16, pericon);
    u8g2.setCursor(100, 48);
    u8g2.print(GPU_USE);
    // GPU频率
    u8g2.drawXBMP(0, 48, 16, 16, freqicon);
    u8g2.drawXBMP(56, 48, 16, 16, mhzicon);
    u8g2.setCursor(18, 48);
    u8g2.print(GPU_FREQ);
  } while ( u8g2.nextPage() );
}

void drawCPU(void){
  u8g2.setFont(u8g2_font_crox3h_tf);
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(8, 16, 64, 32, cpulogo);
    // 时钟
    //u8g2.drawXBMP(0, 0, 16, 16, timeicon);
    u8g2.setCursor(10,0);
    u8g2.print(nowTime);
    // 散热架转速
    u8g2.drawXBMP(74, 0, 16, 16, fanicon);
    //u8g2.drawXBMP(120, 0, 8, 16, pericon);
    u8g2.setCursor(90,0);
    u8g2.print(CPU_FAN);
    // RAM使用率
    u8g2.drawXBMP(80, 15, 16, 16, ramicon);
    u8g2.drawXBMP(120, 16, 8, 16, pericon);
    u8g2.setCursor(100,16);
    u8g2.print(RAM_USE);
    // CPU温度
    u8g2.drawXBMP(80, 31, 16, 16, tempicon);
    u8g2.drawXBMP(120, 32, 8, 16, celicon);
    u8g2.setCursor(100, 32);
    u8g2.print(CPU_TEMP); 
    // CPU使用率
    u8g2.drawXBMP(80, 48, 16, 16, useicon);
    u8g2.drawXBMP(120, 48, 8, 16, pericon);
    u8g2.setCursor(100, 48);
    u8g2.print(CPU_USE);
    // CPU频率
    u8g2.drawXBMP(0, 48, 16, 16, freqicon);
    u8g2.drawXBMP(56, 48, 16, 16, mhzicon);
    u8g2.setCursor(18, 48);
    u8g2.print(CPU_FREQ);
  } while ( u8g2.nextPage() );
}

void fanControl(){
  if(!PConline){
    CASE_FAN = 0;
    fanSpeed = 0;
  }
  else if(CPU_TEMP < startTemp && GPU_TEMP < startTemp){
    CASE_FAN = 0;
    fanSpeed = 0;
  }
  else if((CPU_TEMP > startTemp && CPU_TEMP < fullTemp) || (GPU_TEMP > startTemp && GPU_TEMP < fullTemp)){
    byte temp = CPU_TEMP > GPU_TEMP ? CPU_TEMP:GPU_TEMP ; // 赋值较高的温度
    CASE_FAN = map(temp, startTemp, fullTemp, 0, 100);
    fanSpeed = map(temp, startTemp, fullTemp, 0, 255);
  }
  else if(CPU_TEMP < fullTemp || GPU_TEMP < fullTemp){
    CASE_FAN = 100;
    fanSpeed = 255;
  }
  analogWrite(D2,fanSpeed);
}

void ClockDisplay(){
  nowTime="";
  nowDay="";
  if (isNTPConnected){
    int years, months, days, hours, minutes, seconds, weekdays;
    years = year();
    months = month();
    days = day();
    hours = hour();
    minutes = minute();
    seconds = second();
    weekdays = weekday();
    if (hours < 10)
      nowTime += 0;
    nowTime += hours;
    nowTime += ":";
    if (minutes < 10)
      nowTime += 0;
    nowTime += minutes;
    nowTime += ":";
    if (seconds < 10)
      nowTime += 0;
    nowTime += seconds;
    nowDay += years;
    nowDay += "/";
    if (months < 10)
      nowDay += 0;
    nowDay += months;
    nowDay += "/";
    if (days < 10)
      nowDay += 0;
    nowDay += days;
  }
  else{
    nowTime="NTP Error";
  }
}

/*-------- HTTP代码 ----------*/

void httprequest(){
  if (!client.connect(host, port)) {  // 测试是否正常连接
    Serial.println("connection failed");
    PConline = false;
    return;  
  }  
  PConline = true;
  delay(10);   
  String postRequest =(String)("GET ") + "/sse" + " HTTP/1.1\r\n" +  
    "Content-Type: text/html;charset=utf-8\r\n" +  
    "Host: " + host + "\r\n" + 
    "User-Agent: BuildFailureDetectorESP8266\r\n" +
    "Connection: close\r\n\r\n";   
  client.print(postRequest);  // 发送HTTP请求
  client.print(postRequest);  // 发送HTTP请求
  delay(50);
  char buffer[420];
  client.readBytes(buffer,421);
  String data = String(buffer);
  Serial.println("HTTP Success!");
  dataProcess(data);
}

void dataProcess(String s) {
  int datStart = 0, datEnd = 0;
  String datstr;
 
  char cpu_freq[] = "CPU_FREQ";
  datStart = s.indexOf(cpu_freq) + strlen(cpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FREQ = datstr.toInt();
 
  char gpu_freq[] = "GPU_FREQ";
  datStart = s.indexOf(gpu_freq) + strlen(gpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FREQ = datstr.toInt();

  char cpu_temp[] = "CPU_TEMP";
  datStart = s.indexOf(cpu_temp) + strlen(cpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_TEMP = datstr.toInt();

  char gpu_temp[] = "GPU_TEMP";
  datStart = s.indexOf(gpu_temp) + strlen(gpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_TEMP = datstr.toInt();

  char cpu_fan[] = "CPU_FAN";
  datStart = s.indexOf(cpu_fan) + strlen(cpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FAN = datstr.toInt();

  char gpu_fan[] = "GPU_FAN";
  datStart = s.indexOf(gpu_fan) + strlen(gpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FAN = datstr.toInt();
  
  char cpu_use[] = "CPU_USE";
  datStart = s.indexOf(cpu_use) + strlen(cpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_USE = datstr.toInt();

  char gpu_use[] = "GPU_USE";
  datStart = s.indexOf(gpu_use) + strlen(gpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_USE = datstr.toInt();

  char ram_use[] = "RAM_USE";
  datStart = s.indexOf(ram_use) + strlen(ram_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  RAM_USE = datstr.toInt();
}

/*-------- NTP代码 ----------*/

const int NTP_PACKET_SIZE = 48; // NTP报头大小48字节
byte packetBuffer[NTP_PACKET_SIZE]; // 容纳传入和传出数据包的缓冲区

time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP服务器地址

  while (Udp.parsePacket() > 0) ; // 丢弃任何以前接收的包
  // 从池中随机获取一个服务器
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      isNTPConnected = true;
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // 将数据包读入缓冲区
      unsigned long secsSince1900;
      // 将从位置40开始的四个字节转换为一个长整数
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 如果无法获取时间，则返回0
}

// 发送一个NTP请求到时间服务器的地址
void sendNTPpacket(IPAddress &address){
  // 将缓冲区中的所有字节设置为0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // 初始化NTP请求所需的值
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8字节的零为根延迟和根分散
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // 现在，所有NTP字段都被赋予了值
  // 你可以发送一个请求时间戳的数据包:
  Udp.beginPacket(address, 123); // NTP请求到123端口
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
