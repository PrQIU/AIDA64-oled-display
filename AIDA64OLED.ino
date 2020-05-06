#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "icon.h"

/*            **** 接线方式 ****
 *  ESP8266     D1      D2      D3      D4
 *  OLED                       SDA     SCL
 */
/*              **** 配置 ****             */
const char* ssid     = "SSID";               // WiFi的名字
const char* password = "PASSWORD";           // WiFi密码
const char* host     = "192.168.XXX.XXX";    // 需要访问的IP
const int   port     = 9999;                 // 需要访问的端口

/**********************************************************************************************************/

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D4, /* data=*/ D3, /* reset=*/ U8X8_PIN_NONE);
WiFiClient client;
boolean PCOnline = false;                    // PC连接状态
int CPU_FREQ, GPU_FREQ, CPU_TEMP, GPU_TEMP, GPU_USE, CPU_USE, RAM_USE, CPU_FAN, GPU_FAN; // 参数

void setup() {
  u8g2Init();
  wifiInit();
  Serial.begin(115200);
  Serial.println("Start!");
}

void loop() {
  static byte i = 0;
  if(isDelay(1000)){
    httprequest();
    if((i/5 == 0) || !PCOnline){        // 1-5
      drawCPU();
    }
    else if((i / 5 == 1) && PCOnline){    // 6-10
      drawGPU();
    }
    i++;
    if(i>=10){
      i=0;
    }
  }
}

void u8g2Init(void) {
  u8g2.begin();
  u8g2.setFontPosTop();   // 字体左上角为原点
  u8g2.setFontDirection(0);
  u8g2.drawXBMP(0, 16, 128,32, mianlogo);
  u8g2.sendBuffer();
}

void wifiInit(){
  Serial.printf("Connecting to %s \n", ssid);
  WiFi.begin(ssid, password);
  delay(500);
  while (WiFi.status() != WL_CONNECTED){ // 等待连接WiFi
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());        // 开发板的IP地址
}

void drawGPU(void){
  u8g2.setFont(u8g2_font_crox3h_tf);
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(8, 0, 64, 32, gpulogo);
    // RAM使用率
    u8g2.drawXBMP(80, 15, 16, 16, ramicon);
    u8g2.drawXBMP(120, 16, 8, 16, pericon);
    u8g2.setCursor(100,16);
    u8g2.print(RAM_USE);
    // GPU温度
    u8g2.drawXBMP(80, 48, 16, 16, tempicon);
    u8g2.drawXBMP(120, 48, 8, 16, celicon);
    u8g2.setCursor(100, 48);
    u8g2.print(GPU_TEMP); 
    // GPU使用率
    u8g2.drawXBMP(80, 32, 16, 16, useicon);
    u8g2.drawXBMP(120, 32, 8, 16, pericon);
    if(GPU_USE>=10){
      u8g2.setCursor(100, 32);
      u8g2.print(GPU_USE);
    }
    else if(GPU_USE<10){
    u8g2.setCursor(109, 32);
    u8g2.print(GPU_USE);
    }
    // GPU频率
    u8g2.drawXBMP(0, 32, 16, 16, freqicon);
    if(GPU_FREQ>=1000){
      u8g2.setCursor(18, 32);
      u8g2.print(GPU_FREQ);
    }
    else if(GPU_FREQ<1000){
      u8g2.setCursor(27, 32);
      u8g2.print(GPU_FREQ);
    }
    // GPU转速
    u8g2.drawXBMP(0, 48, 16, 16, fanicon);
    if(GPU_FAN>=1000){
      u8g2.setCursor(18,48);
      u8g2.print(GPU_FAN);
    }
    else if(GPU_FAN<1000 && GPU_FAN>=100){
      u8g2.setCursor(27,48);
      u8g2.print(GPU_FAN);
    }
    else if(GPU_FAN<100 && GPU_FAN>=10){
      u8g2.setCursor(36,48);
      u8g2.print(GPU_FAN);
    }
    else if(GPU_FAN<10){
      u8g2.setCursor(45,48);
      u8g2.print(GPU_FAN);
    }
    u8g2.setFont(u8g2_font_t0_12_mr);
    u8g2.setCursor(56,36);
    u8g2.print("MHz");
    u8g2.setCursor(56,52);
    u8g2.print("RPM");
  } while ( u8g2.nextPage() );
}

void drawCPU(void){
  u8g2.setFont(u8g2_font_crox3h_tf);
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(8, 0, 64, 32, cpulogo);
    // RAM使用率
    u8g2.drawXBMP(80, 15, 16, 16, ramicon);
    u8g2.drawXBMP(120, 16, 8, 16, pericon);
    u8g2.setCursor(100,16);
    u8g2.print(RAM_USE);
    // CPU温度
    u8g2.drawXBMP(80, 48, 16, 16, tempicon);
    u8g2.drawXBMP(120, 48, 8, 16, celicon);
    u8g2.setCursor(100, 48);
    u8g2.print(CPU_TEMP); 
    // CPU使用率
    u8g2.drawXBMP(80, 32, 16, 16, useicon);
    u8g2.drawXBMP(120, 32, 8, 16, pericon);
    if(CPU_USE>=10){
      u8g2.setCursor(100, 32);
      u8g2.print(CPU_USE);
    }
    else if(CPU_USE<10){
    u8g2.setCursor(109, 32);
    u8g2.print(CPU_USE);
    }
    // CPU频率
    u8g2.drawXBMP(0, 32, 16, 16, freqicon);
    if(CPU_FREQ>=1000){
      u8g2.setCursor(18, 32);
      u8g2.print(CPU_FREQ);
    }
    else if(CPU_FREQ<1000){
      u8g2.setCursor(27, 32);
      u8g2.print(CPU_FREQ);
    }
    // CPU转速
    u8g2.drawXBMP(0, 48, 16, 16, fanicon);

    if(CPU_FAN>=1000){
      u8g2.setCursor(18,48);
      u8g2.print(CPU_FAN);
    }
    else if(CPU_FAN<1000){
      u8g2.setCursor(27,48);
      u8g2.print(CPU_FAN);
    }
    u8g2.setFont(u8g2_font_t0_12_mr);
    u8g2.setCursor(56,36);
    u8g2.print("MHz");
    u8g2.setCursor(56,52);
    u8g2.print("RPM");
  } while ( u8g2.nextPage() );
}

/*-------- HTTP代码 ----------*/

void httprequest(){
  if (!client.connect(host, port)) {  // 测试是否正常连接
    Serial.println("connection failed");
    PCOnline = false;
    return;
  }  
  PCOnline = true;
  delay(10);   
  String postRequest =(String)("GET ") + "/sse" + " HTTP/1.1\r\n" +  
    "Content-Type: text/html;charset=utf-8\r\n" +  
    "Host: " + host + "\r\n" + 
    "User-Agent: BuildFailureDetectorESP8266\r\n" +
    "Connection: close\r\n\r\n";   
  client.print(postRequest);  // 发送HTTP请求
  client.print(postRequest);  // 发送HTTP请求
  delay(30);
  char buffer[380];
  client.readBytes(buffer,381);
  String data = String(buffer);
  Serial.println(data);
  Serial.println("HTTP Success!");
  dataProcess(data);
}

boolean isDelay(unsigned long time_ms){
    static unsigned long oldtime=0;
    if(millis() - oldtime > time_ms){
        oldtime=millis();//更新时间点
        return true;
    }
    return false;
}

void dataProcess(String s) {
  int datStart = 0, datEnd = 0;
  String datstr;
 
  char cpu_freq[] = "CFRE";
  datStart = s.indexOf(cpu_freq) + strlen(cpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FREQ = datstr.toInt();
 
  char gpu_freq[] = "GFRE";
  datStart = s.indexOf(gpu_freq) + strlen(gpu_freq);
  datEnd = s.indexOf("MHz", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FREQ = datstr.toInt();

  char cpu_temp[] = "CTEM";
  datStart = s.indexOf(cpu_temp) + strlen(cpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_TEMP = datstr.toInt();

  char gpu_temp[] = "GTEM";
  datStart = s.indexOf(gpu_temp) + strlen(gpu_temp);
  datEnd = s.indexOf("℃", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_TEMP = datstr.toInt();
 
  char cpu_use[] = "CUSE";
  datStart = s.indexOf(cpu_use) + strlen(cpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_USE = datstr.toInt();

  char gpu_use[] = "GUSE";
  datStart = s.indexOf(gpu_use) + strlen(gpu_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_USE = datstr.toInt();

  char ram_use[] = "RUSE";
  datStart = s.indexOf(ram_use) + strlen(ram_use);
  datEnd = s.indexOf("%", datStart);
  datstr = s.substring(datStart, datEnd);
  RAM_USE = datstr.toInt();
  
  char cpu_fan[] = "CFAN";
  datStart = s.indexOf(cpu_fan) + strlen(cpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  CPU_FAN = datstr.toInt();

  char gpu_fan[] = "GFAN";
  datStart = s.indexOf(gpu_fan) + strlen(gpu_fan);
  datEnd = s.indexOf("RPM", datStart);
  datstr = s.substring(datStart, datEnd);
  GPU_FAN = datstr.toInt();
  Serial.println(datEnd);
}
