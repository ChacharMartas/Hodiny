/*
nejaky program zatim to dela celkem prd


*/

// include the library code:
#include <LiquidCrystal.h>
#include <Wire.h>
#include <stdlib.h>
#include <OneWire.h>
#include <EEPROM.h>

#define RTC0 80  //adresa hodin realneho casu 0x1F volna ram
#define EPROM 81   //adresa pameti eeprom

//blok dat pro hodiny
byte sekundy = 0;
byte o_sekundy = 0;//detekce zmeny sekundy
byte minuty = 0;
byte o_minuty = 0;  //detekce zmeny minuty
byte hodiny = 0;
byte dny = 0;
byte den = 0;//den v tydnu
byte mesic = 0;
int rok = 0;
int sto = 0;//stoleti
byte yeda = 0;
byte wemo = 0;
char znak = 0;
byte i = 0;
byte priznak = 0;
byte cislo = 0;
byte jas = 255;
byte smer = 1; //0 - nahoru, 1-dolu
long last_millis;

//ukladani dat
unsigned int  adresa = 0;
byte  u_adr = 0;
byte  l_adr = 0;
unsigned int  i_adr = 0;

//blok dat teplomer
byte LSB0 = 0;
byte MSB0 = 0;
byte LSB1 = 0;
byte MSB1 = 0;
int tepl = 0;
float  teplota0 = 0;
float  teplota1 = 0;
char  t_tm0[20];
char  t_tm1[20];

//bliknuti ledkou
byte  blik = LOW ;


//inicializace nastaveni cidel
OneWire  tm0(A2);  // cidlo jedna na 10
OneWire  tm1(A3);  // cidlo dve na 9

// inicializace nastaveni displeje definice pinu
//LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(A0, A1, 5, 4, 3, 2);

void setup() {
  pinMode(13, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT);
  analogWrite(6, jas);
  Serial.begin(115200);//nastaveni seriove linky na 9600
  uvodni_zprava();
  lcd.begin(20, 4);
  Wire.begin();//povoleni I2C rozhrani
  
  lcd.setCursor(0, 0);
  lcd.print("Cas:");
  lcd.setCursor(0, 1);
  lcd.print("Datum:");
  lcd.setCursor(0,2);
  lcd.print("Vnitrni:");
  lcd.setCursor(0,3);
  lcd.print("Vnejsi:");
  lcd.setCursor(18,2);
  lcd.write(0xDF);
  lcd.print("C");
  lcd.setCursor(18,3);
  lcd.write(0xDF);
  lcd.print("C");
  
  Wire.beginTransmission(RTC0);
  Wire.write(0x1F);
  Wire.endTransmission();
  Wire.requestFrom(RTC0, 2);
  l_adr = Wire.read();
  u_adr = Wire.read();
  adresa = (((unsigned int)u_adr) << 8) + (unsigned int)l_adr;

  Serial.println(adresa/8, DEC);  
    
  //poctaceni nastaveni mereni teploty
  tm0.reset();//reset dratu
  tm0.skip();//preskoceni adresovani na dratu kazde cidlo zvlast
  tm0.write(0x44,1);//spusteni mereni
  tm1.reset();//reset dratu
  tm1.skip();//preskoceni adresovani na dratu kazde cidlo zvlast
  tm1.write(0x44,1);//spusteni mereni
  delay(1000);//cekani na dokonceni prevodu, dale to bude delat RTC
}

void loop() {
  if(millis() > last_millis){
    last_millis = millis() + 8;
    if(!digitalRead(7)){
      if(jas != 255){
        jas++;
      }
    }
    else{
      if(jas > 5){
        jas--;
      }
    }
  }
  analogWrite(6, jas);
  lcd.setCursor(4, 0);
  Wire.beginTransmission(RTC0);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(RTC0, 1);
  sekundy = Wire.read(); //test na zmenu sekund ridi cteni teploty
  if(sekundy != o_sekundy)  {
    o_sekundy = sekundy;
    if (blik == LOW){
      blik = HIGH;}
    else{
      blik = LOW;}
    digitalWrite(13, blik);
    Wire.requestFrom(RTC0, 4);
    minuty = Wire.read(); 
    hodiny = Wire.read(); 
    yeda = Wire.read(); 
    wemo = Wire.read(); 
    if(hodiny < 10)  {
      lcd.print("0");
    }
    lcd.print(hodiny,HEX);
    lcd.print(":");
    if(minuty < 10)  {
      lcd.print("0");
    }
    lcd.print(minuty,HEX);
    lcd.print(":");
    if(sekundy < 10)  {
      lcd.print("0");
    }
    lcd.print(sekundy,HEX);
    den = wemo >> 5;
    switch (den){
      case 0:
        lcd.print(" Pondeli");
        break;
      case 1:
        lcd.print(" Utery  ");
        break;
      case 2:
        lcd.print(" Streda ");
        break;
      case 3:
        lcd.print(" Ctvrtek");
        break;
      case 4:
        lcd.print(" Patek  ");
        break;
      case 5:
        lcd.print(" Sobota ");
        break;
      case 6:
        lcd.print(" Nedele ");
        break;
    }   
    dny = yeda & B00111111;
    lcd.setCursor(6, 1);
    lcd.print(dny,HEX);
    mesic = wemo & B00011111;
    lcd.print(".");
    lcd.print(mesic,HEX);
    lcd.print(".");
    rok = (int)((yeda & B11000000) >> 6) + 2012;
    lcd.print(rok,DEC);
    lcd.print("  ");//mazani konce radku
    
    //cteni teploty
    tm0.reset();//reset dratu
    tm0.skip();    
    tm0.write(0xBE);
    LSB0 = tm0.read();
    MSB0 = tm0.read();
    tm1.reset();//reset dratu
    tm1.skip();    
    tm1.write(0xBE);
    LSB1 = tm1.read();
    MSB1 = tm1.read();
    tm0.reset();//reset dratu
    tm0.skip();//preskoceni adresovani na dratu kazde cidlo zvlast
    tm0.write(0x44,1);//spusteni mereni
    tm1.reset();//reset dratu
    tm1.skip();//preskoceni adresovani na dratu kazde cidlo zvlast
    tm1.write(0x44,1);//spusteni mereni
    tepl = (((int)MSB0) << 8) + ((int)LSB0);
    teplota0 = ((float)tepl)/16;
    dtostrf(teplota0,5,1,t_tm0);
    lcd.setCursor(13,2);
    lcd.print(t_tm0);
    tepl = (((int)MSB1) << 8) + ((int)LSB1);
    teplota1 = ((float)tepl)/16;
    dtostrf(teplota1,5,1,t_tm1);
    lcd.setCursor(13,3);
    lcd.print(t_tm1);
    //konec cteni teploty
    if(o_minuty != minuty){
      o_minuty = minuty;
      zapis_EEPROM(EPROM, adresa, yeda);
      adresa++;
      zapis_EEPROM(EPROM, adresa, wemo);
      adresa++;
      zapis_EEPROM(EPROM, adresa, hodiny);
      adresa++;
      zapis_EEPROM(EPROM, adresa, minuty);
      adresa++;
      zapis_EEPROM(EPROM, adresa, MSB0);
      adresa++;
      zapis_EEPROM(EPROM, adresa, LSB0);
      adresa++;
      zapis_EEPROM(EPROM, adresa, MSB1);
      adresa++;
      zapis_EEPROM(EPROM, adresa, LSB1);
      adresa++;
      l_adr = (byte)adresa;
      u_adr = (byte)(adresa >> 8);
      Wire.beginTransmission(RTC0);
      Wire.write(0x1F);
      Wire.write(l_adr);
      Wire.write(u_adr);
      Wire.endTransmission();
      }
    
  }
     
  if(Serial.available())  {
    znak = Serial.read();
    if(znak > 64 && znak < 91){
      switch(znak){
        case 'C':
          Serial.println("Zadej cas HHMMSS");
          priznak = 1;
          i = 0;
          break;
        case 'D':
          Serial.println("Zadej datum WDDMMY");
          Serial.println("W Po = 0...Ne = 6");
          Serial.println("0 prestupny 1,2,3 normalni od prestupneho");
          priznak = 2;
          i = 0;
          break;
        case 'T':
          Serial.print("Vnitrni teplota je:");
          Serial.print(t_tm0);
          Serial.write(176);
          Serial.println("C");
          Serial.print("Vnejsi teplota je:");
          Serial.print(t_tm1);
          Serial.write(176);
          Serial.println("C");
          priznak = 0;
          i = 0;
          break;
        case 'R':
          Wire.beginTransmission(RTC0);
          Wire.write(0x1F);
          Wire.write(0x00);
          Wire.write(0x00);
          Wire.endTransmission();
          l_adr = 0;
          u_adr = 0;
          adresa = 0;
          priznak = 0;
          i = 0;
          break;
        case 'V':
          Serial.println(" ");
          Serial.println("Vypis dat");
          Serial.print("Datum a cas\tVnitrni\tVnejsi teplota v [");
          Serial.write(176);
          Serial.println("C]");
          Serial.println(adresa, DEC);
          Serial.println(" ");
          Wire.beginTransmission(EPROM);
          Wire.write(0x00);
          Wire.write(0x00);
          Wire.endTransmission();
          for(i_adr = 0; i_adr < adresa/8; i_adr++){
            Wire.requestFrom(EPROM, 8);
            yeda = Wire.read();
            wemo = Wire.read();
            hodiny = Wire.read();
            minuty = Wire.read();
            MSB0 = Wire.read();
            LSB0 = Wire.read();
            MSB1 = Wire.read();
            LSB1 = Wire.read();
            dny = yeda & B00111111;
            Serial.print(dny,HEX);
            mesic = wemo & B00011111;
            Serial.print(".");
            Serial.print(mesic,HEX);
            Serial.print(".");
            rok = (int)((yeda & B11000000) >> 6) + 2008;
            Serial.print(rok,DEC);
            Serial.print(" ");
            if(hodiny < 10)  {
              Serial.print("0");
            }
            Serial.print(hodiny,HEX);
            Serial.print(":");
            if(minuty < 10)  {
              Serial.print("0");
            }
            Serial.print(minuty,HEX);
            Serial.print("\t");
            tepl = (((int)MSB0) << 8) + ((int)LSB0);
            teplota0 = ((float)tepl)/16;
            dtostrf(teplota0,5,1,t_tm0);
            t_tm0[3] = 44;
            Serial.print(t_tm0);
            Serial.print("\t");
            tepl = (((int)MSB1) << 8) + ((int)LSB1);
            teplota1 = ((float)tepl)/16;
            dtostrf(teplota1,5,1,t_tm1);
            t_tm1[3] = 44;
            Serial.println(t_tm1);
          }
          break;
        default:
          uvodni_zprava();
          priznak = 0;
          i = 0;
          break;
      }
    }
     else{
       if(znak > 47 && znak < 58)  {
         switch (priznak){
           case 1:
             switch(i)  {
               case 0:
                 Serial.write(znak);
                 cislo = ((hodiny & B00001111) | ((znak - 48) << 4));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x04);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 1:
                 Serial.write(znak);
                 cislo = ((hodiny & B11110000) | (znak - 48));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x04);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 2:
                 Serial.write(znak);
                 cislo = ((minuty & B00001111) | ((znak - 48) << 4));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x03);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 3:
                 Serial.write(znak);
                 cislo = ((minuty & B11110000) | (znak - 48));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x03);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 4:
                 Serial.write(znak);
                 cislo = ((sekundy & B00001111) | ((znak - 48) << 4));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x02);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 5:
                 Serial.write(znak);
                 cislo = ((sekundy & B11110000) | (znak - 48));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x02);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
             }
             break;  
           case 2:
             switch(i)  {
               case 0:
                 Serial.write(znak);
                 cislo = ((wemo & B00011111) | ((znak - 48) << 5));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x06);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 1:
                 Serial.write(znak);
                 cislo = ((yeda & B11001111) | (((znak - 48) << 4) & B00110000));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x05);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 2:
                 Serial.write(znak);
                 cislo = ((yeda & B11110000) | (znak - 48));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x05);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
                case 3:
                 Serial.write(znak);
                 cislo = ((wemo & B11101111) | (((znak - 48) << 4) & B00010000));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x06);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
                case 4:
                 Serial.write(znak);
                 cislo = ((wemo & B11110000) | (znak - 48));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x06);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
               case 5:
                 Serial.write(znak);
                 cislo = ((yeda & B00111111) | (((znak - 48) << 6) & B11000000));             
                 Wire.beginTransmission(RTC0);
                 Wire.write(0x05);
                 Wire.write(cislo);
                 Wire.endTransmission();
                 i++;
                 break;
             }
           break;
         }
       }
     }
  }
}

void zapis_EEPROM(byte tw_adr,unsigned int adr_m,byte data){
  byte ladr = (byte)adresa;
  byte uadr = (byte)(adresa >> 8);
  Wire.beginTransmission(tw_adr);
  Wire.write(uadr);
  Wire.write(ladr);
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}

void uvodni_zprava(void){
  Serial.println("Zadej C pro nastaveni casu");
  Serial.println("Zadej D pro nastaveni datumu");
  Serial.println("Zadej T pro cteni teploty");
  Serial.println("Zadej R pro nulovani adresy");
  Serial.println("Zadej V pro vypis dat");
}


/*

char tmp0 = 0;
byte tmp1 = 0;
float  teplota = 0;
char  test[16];
#define TMP0 B1001000

  Wire.beginTransmission(TMP0);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP0, 2);    // request 6 bytes from slave device #2
  tmp0 = Wire.read(); // receive a byte as character
  tmp1 = Wire.read(); // receive a byte as character
  teplota = (float)tmp0;
  teplota = teplota + ((float)(tmp1>>4))/16;
  dtostrf(teplota,4,1,test);
  lcd.setCursor(8,3);
  lcd.print(test);
  lcd.write(0xDF);
  lcd.print("C");
*/

/*
Code:

long n;
byte buf[4];
 




Long -> byte[4]:
Code:

buf[0] = (byte) n;
buf[1] = (byte) n >> 8;
buf[2] = (byte) n >> 16;
buf[3] = (byte) n >> 24;
 




byte[4] -> int
Code:

long value = (unsigned long)(buf[4] << 24) | (buf[3] << 16) | (buf[2] << 8) | buf[1];
 



*/
