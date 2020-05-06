# AIDA64-oled-display
用于从AIDA64中获取CPU与GPU的频率、温度等参数

需要材料

  1、一个esp8266开发板

  2、一个12864oled显示屏

  3、杜邦线或者自己焊接

  4、AIDA64软件

  5、Arduino IDE

步骤

  在AIDA64设置中开启RemoteSensor支持

  将配置文件导入AIDA64

  找到自己PC的IP地址

  在.ino文件中配置好WiFi的SSID、密码及PC的IP地址

  写入esp8266
