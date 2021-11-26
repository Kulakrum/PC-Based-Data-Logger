/*
   Victor Kulankash
   Complete Project Details email victormoses@students.uonbi.ac.ke
*/

//including libraries
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "Wire.h"
#include <SPI.h> //for the SD card module
#include <SD.h> // for the SD card
#include <DHT.h> // for the DHT sensor
#include <RTClib.h> // for the RTC


//define DHT pin
#define DHTPIN 30 // what pin we're connected to

//define DHT sensor type
#define DHTTYPE DHT11   // DHT 11 


// initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE );

//led pin connections to Arduino
int RED_LED = 52;
int Green_LED = 50;

//Sim Module pin Tx and Rx connections to Arduino
SoftwareSerial gprsSerial(46, 44);

// Arduino Ethernet shield and modules: pin 4
// Data logging SD shields and modules: pin 10 SD shield: pin 8
const int chipSelect = 10;

//led display pin connections to Arduino
int rs = 7;
int en = 9;
int d4 = 10;
int d5 = 11;
int d6 = 12;
int d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Create a file to store the data
File myFile;

// RTC
RTC_DS1307 rtc;


void setup() 
{
  //initializing the DHT sensor
  pinMode(RED_LED, OUTPUT);
  pinMode(Green_LED, OUTPUT);
  dht.begin();


  //initializing Serial monitor
  Serial.begin(9600);
  lcd.begin(16, 4);
  gprsSerial.begin(9600);

  // setup for the RTC
  while (!Serial); // for Leonardo/Micro/Zero
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  // setup for the SD card

  Serial.print("Initializing SD card...");
  if (!SD.begin(22, 26, 28, 24)) {
    Serial.println("Card failed, or not present");
    digitalWrite(RED_LED, HIGH);
    delay(10000);
    digitalWrite(RED_LED, LOW);
    return;
  }
  else {
    Serial.println("SD card initialized.");
    digitalWrite(Green_LED, HIGH);
    delay(1000);
    digitalWrite(Green_LED, LOW);
  }

  if(!SD.begin(chipSelect)) {
     Serial.println("initialization failed!");
     return;
    }
    Serial.println("initialization done.");
  
  //open file
  myFile = SD.open("DATA.txt", FILE_WRITE);

  // if the file opened ok, write to it:
  if (myFile) {
    Serial.println("File opened ok");
    // print the headings for our data
    myFile.println("Date,Time,Temperature ºC, Humidity %,");
  }
  myFile.close();
}


void loop() 
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit
  //float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if  (isnan(t) /*|| isnan(f)*/) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  //debugging purposes
  loggingTime();
  loggingTempHum(t, h);

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  //Serial.print(f);
  //Serial.println(" *F\t");
  LCD(t, h);
  Send_message(t, h);

  delay(5000);
}


//logging current time to SD card

void loggingTime() 
{
  DateTime now = rtc.now();
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second(), DEC);
    myFile.print(",");
  }
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.println(now.day(), DEC);
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  myFile.close();
  delay(1000);
}


//logging temperature and Humidity to SD card

void loggingTempHum(float t, float h) 
{

  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("open with success");
    myFile.print(t);
    myFile.print(",");
    myFile.println(h);
    //myFile.print("");
  }
  myFile.close();
}


//Real-time temperature and Humidity to LED display

void LCD(float t, float h) {
  lcd.setCursor(0, 0);
  lcd.print("Temperature:");
  lcd.setCursor(0, 1);
  lcd.print(t);
  lcd.setCursor(0, 2);
  lcd.print("Humidity:");
  lcd.setCursor(0, 3);
  lcd.print(h);
  
}

//Sending messege to API via Sim module

void Send_message(float t, float h)
{

  gprsSerial.println("AT"); //ping the module to see if its up!
  delay(200);

  gprsSerial.println("AT+CPIN?"); //Check if the SIM card is ready to make calls, messages or to start the data packet transmission.
  delay(200);

  gprsSerial.println("AT+CREG?");
  delay(200);

  gprsSerial.println("AT+CGATT?");//Checking SIM card has internet connection or not
  delay(00);

  gprsSerial.println("AT+CIPSHUT");//Reset IP session
  delay(700);

  gprsSerial.println("AT+CIPSTATUS");//Check if IP stack is initialized
  delay(2000);

  gprsSerial.println("AT+CIPMUX=0");//Setting up single connection mode
  delay(1000);

  ShowSerialData();

  gprsSerial.println("AT+CSTT=\"safaricom\"");//start task and setting the APN,
  delay(1000);

  ShowSerialData();

  gprsSerial.println("AT+CIICR");// set up the wireless GPRS connection wiht the service provider and obtain an ip address
  delay(1000);

  ShowSerialData();

  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(1000);

  ShowSerialData();

  gprsSerial.println("AT+CIPSPRT=0");
  delay(1000);

  ShowSerialData();
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(2000);
  ShowSerialData();

  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(3000);
  ShowSerialData();

  String str = "GET https://api.thingspeak.com/update?api_key=EWQTHLBW7OUGDSVV&field1=" + String(t) + "&field2=" + String(h);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server

  delay(4000);
  ShowSerialData();

  gprsSerial.println((char)26);//sending
  delay(3000);//waitting for reply, important! the time is base on the condition of internet
  gprsSerial.println();

  ShowSerialData();

  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
}

void ShowSerialData()
{
  delay(500);
  while (Serial.available())
  {
    gprsSerial.write(Serial.read());
  }

  while (gprsSerial.available())
  {
    Serial.write(gprsSerial.read());

  }

}
