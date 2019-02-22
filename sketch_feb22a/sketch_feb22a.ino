//for Chinshien only
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#define TEMP_HUMI
#define RX 3
#define TX 4
String AP = "jonathan";       // CHANGE ME
String PASS = "1234567890"; // CHANGE ME
String API = "16W3G2FSQ3GMQIS0";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
int countTrueCommand;
int countTimeCommand; 
int interval=0;
boolean found = false; 
int valSensor = 1;
int tempPin = 0;
SoftwareSerial esp8266(RX,TX); 
LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display 
enum format
{hex,
 dec,
 oct,
 bin
};
void lcd_show_c1(long int showvalue, int col,enum format data_format) {
  lcd.setCursor(0,col);
  if (data_format==hex) {lcd.clear(); lcd.print(showvalue, 16); }
  if (data_format==dec) { lcd.print( float( (showvalue/1024.0)*5000/10) );  }
}


 
void setup() {
  Serial.begin(9600);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  esp8266.begin(9600);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}
void loop() {
  enum format lm75_format;
  lm75_format=dec;
  long int lm75_value;
  lm75_value=float( (analogRead(tempPin)/1024.0)*5000/10 );
  if ( analogRead(tempPin) ) {lcd_show_c1( analogRead(tempPin), 1, lm75_format);}
  while(interval>100) { 
   SentOnCloud( String(lm75_value), String(lm75_value) );
  interval=0;
  }
  Serial.print("interval value=");
  Serial.println(interval);
  interval++;
  delay(100);
}

int getSensorData(){
  return random(1000); // Replace with 
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }

void SentOnCloud( String humi, String temp) {
 String getData = "GET /update?api_key="+ API +"&"+ field1 +"="+humi +"&"+ field2 +"="+temp;
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);delay(1500);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}

#ifdef TEMP_HUMI

int temp;//温度
int humi;//湿度
int tol;//校对码
int j;
unsigned int loopCnt;
int chr[40] = {0};//创建数字数组，用来存放40个bit
unsigned long time;
#define pin 2
void temp_humi() {
  bgn:
  delay(2000);
//设置2号接口模式为：输出
//输出低电平20ms（>18ms）
//输出高电平40μs
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);
  delay(20);
  digitalWrite(pin,HIGH);
  delayMicroseconds(40);
  digitalWrite(pin,LOW);
//设置2号接口模式：输入
  pinMode(pin,INPUT);
  //高电平响应信号 
  loopCnt=10000;
  while(digitalRead(pin) != HIGH)
  {
    if(loopCnt-- == 0)
    {
//如果长时间不返回高电平，输出个提示，重头开始。
      Serial.println("HIGH");
      goto bgn;
    }
  }
  //低电平响应信号
  loopCnt=30000;
  while(digitalRead(pin) != LOW)
  {
    if(loopCnt-- == 0)
    {
//如果长时间不返回低电平，输出个提示，重头开始。
      Serial.println("LOW");
      goto bgn;
    }
  }
//开始读取bit1-40的数值  
    for(int i=0;i<40;i++)
  {
    while(digitalRead(pin) == LOW)
    {}
//当出现高电平时，记下时间“time”
    time = micros();
    while(digitalRead(pin) == HIGH)
    {}
//当出现低电平，记下时间，再减去刚才储存的time
//得出的值若大于50μs，则为‘1’，否则为‘0’
//并储存到数组里去
    if (micros() - time >50)
    {
      chr[i]=1;
    }else{
      chr[i]=0;
    }
  }
   
//湿度，8位的bit，转换为数值
humi=chr[0]*128+chr[1]*64+chr[2]*32+chr[3]*16+chr[4]*8+chr[5]*4+chr[6]*2+chr[7];
   
//温度，8位的bit，转换为数值
temp=chr[16]*128+chr[17]*64+chr[18]*32+chr[19]*16+chr[20]*8+chr[21]*4+chr[22]*2+chr[23];
  //校对码，8位的bit，转换为数值
tol=chr[32]*128+chr[33]*64+chr[34]*32+chr[35]*16+chr[36]*8+chr[37]*4+chr[38]*2+chr[39];
//输出：温度、湿度、校对码
   Serial.print("temp:");
   Serial.println(temp);
   Serial.print("humi:");
   Serial.println(humi);

  delay(500);
  #ifdef DIST_DET
    double dist=dist_detect();
  #else
    double dist=0;
  #endif
  SentOnCloud( String(humi), String(temp));
}
#endif
