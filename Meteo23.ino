/*********************************************************************
Программа для проверки OLED дисплея 128x32 с интерфейсом I2C
Вывод 4 используется для сброса дисплея
Измерение температуры производится с помощью датчика LM75A

*********************************************************************/
//--------------------------Подключаемые файлы-----------------------
#include "cMeteo.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_BMP280.h"  // Добавляем драйвер для BMP280
#include <iarduino_DHT.h>     // подключаем библиотеку для работы с датчиком DHT
//------------------------------Константы----------------------------
#define OLED_RESET 4
#define DHT_PIN 2                 // Вывод, к которому подключают датчик DHT

//#define DHT_DWELL 2000            //Период опроса датчика
#define DHT_DWELL 500             //Период опроса датчика

//-------------------------Глобальные переменные---------------------

iarduino_DHT sensor(DHT_PIN);     
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BMP280 bmp;  

String sStatus;

long MillisCurrent;               // реализация задержки
long MillisPrevious;              //

float fPressurePa;
float fAltitude;
float fPressureMm;
float fPT1000Temperature;

int DisplayPhase = 0;

//String sBuffer = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
String sBuffer = "H=90.0% P=760 MM T=20.0*C";

//-------------------------Глобальные константы----------------------

String sVersion = "Meteo\nv2.3.0\n";

//---------------------Программа начальной установки-----------------
void setup()
{                
  if (!bmp.begin()) 
    sStatus = sVersion + "error";
  else
    sStatus = sVersion + "Ok";

  //Инициализация MCP342
  Wire.begin();
  Wire.beginTransmission(0x68);//i2c адрес MCP3421= B1101000
  Wire.write(B11100); // настройка АЦП: постоянное преобразование, 18бит, усиление=1.
  Wire.endTransmission();
  
  //Инициализация дисплея, начальная заставка
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.display();
  display.clearDisplay();

  scrollScreen();
  
  MillisCurrent = millis();       // настройка задержки
  MillisPrevious = MillisCurrent;
  
}//End of void setup()

//------------------------Главный цикл программы---------------------
void loop() 
{
  MillisCurrent = millis();
  if(MillisCurrent - MillisPrevious > DHT_DWELL)
  {
    MillisPrevious = MillisCurrent;
    
    display.stopscroll();

    //---
    sStatus = " ";//
    sStatus += sBuffer.substring(DisplayPhase, DisplayPhase + 6);
    
    // Вывод данных на дисплей части большой строки
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,10);
    display.println(sStatus);
    display.display();
    //---
    

    display.startscrollleft(0x00, 0x07);
    
    display.clearDisplay();

    DisplayPhase++;

    // Завершение индикации, новые измерения
    if(DisplayPhase == sBuffer.length())
    {
      DisplayPhase = 0;
      
      //Измерение давления с помощью датчика BMP280T
      fPressurePa = bmp.readPressure();
      fAltitude = bmp.readAltitude(ALTITUDE_LEVEL);
      fPressureMm = fPressurePa / 133.3;

      //Измерение температуры с помощью датчика LM75A
      Wire.requestFrom(0x4f,2); //запросить 1 байта данных
      char Most = Wire.read();
      char Least = Wire.read();

      //Most = -5;
      //Least = 0;
      
      int high = ((int)Most) << 8;
      int low = (int)Least;
 
      int value = high + low;
      
      fPT1000Temperature = (float)(value / 256);  
 
      // Измерение температуры и влажности с помощью датчика DHT-11
      int DHTStatus = DHT_OK;
      DHTStatus = sensor.read();
    
      // Подготовка данных к выводу на дисплей
      sBuffer = "";

      sBuffer += "T=";
      sBuffer += (String)fPT1000Temperature;

      sBuffer += " *C ";
      
      sBuffer += "P=";
      sBuffer += (String)fPressureMm;
      sBuffer += " mm ";

      switch(DHTStatus)
      {    
        // читаем показания датчика
        case DHT_OK:               sBuffer +=("H=" + (String)sensor.hum + "% ");  break;
        //case DHT_OK:               sBuffer +=("H=" + (String)sensor.hum + "% T_DHT11=" + (String)sensor.tem + "*C ");  break;
        case DHT_ERROR_CHECKSUM:   sBuffer +=(         "Error: Checksum\n");                               break;
        case DHT_ERROR_DATA:       sBuffer +=(         "Error: Protocol\n");                               break;
        case DHT_ERROR_NO_REPLY:   sBuffer +=(         "Error: Timeout\n");                                break;
        default:                   sBuffer +=(         "Error: Sensor\n");                                 break;
      }

      
    }//End of if(DisplayPhase == sBuffer.length())
    
  }//End of if(MillisCurrent - MillisPrevious > DHT_DWELL)

}//End of void loop()

//-----------------------------Подпрограммы--------------------------

void scrollScreen()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println(sStatus);
  display.display();
  delay(1);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  display.clearDisplay();
}

  /*
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);    
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(2000);
  */
