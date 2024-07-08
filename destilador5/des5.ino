#include <Wire.h>
#include <LiquidCrystal.h>
#include <max6675.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
MAX6675 thermo1(26, 24, 22);
MAX6675 thermo2(32, 30, 28);
MAX6675 thermo3(38, 36, 34);
MAX6675 thermo4(44, 42, 40);
MAX6675 thermo5(50, 48, 46);

float temp1, temp2, temp3, temp4, temp5;
const int analogInPin = A0;
int sensorValue = 0;
String json;

void setup() {
  Serial.begin(9600); 
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SENSORES");
  lcd.setCursor(0, 1);
  lcd.print("DESTILACIÓN");
  delay(2000);
}

void loop() {
  //temp1 = thermo1.readCelsius();
  temp1 = thermo2.readCelsius();
  temp2 = thermo3.readCelsius();
  temp3 = thermo4.readCelsius();
  temp4 = thermo5.readCelsius();
  
  //temp1 = random(10, 80);
  //temp2 = random(10, 80);
  //temp3 = random(10, 80);
  //temp4 = random(10, 80);
  //temp5 = random(10, 80);

  json = "{\"temp1\":" + String(temp1) + ", \"temp2\":" + String(temp2) + ", "
         "\"temp3\":" + String(temp3) + ", \"temp4\":" + String(temp4) + "} ";
         //"\"temp5\":" + String(temp5) +"}";
  Serial.println(json);
  
  lcd.setCursor(0, 0);
  lcd.print("Tem1 = " + String(temp1));
  lcd.setCursor(0, 1);
  lcd.print("Tem2 = " + String(temp2));
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tem3 = " + String(temp3));
  lcd.setCursor(0, 1);
  lcd.print("Tem4 = " + String(temp4));
  delay(2000);
  lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.print("Tem5 = " + String(temp5));
  //delay(2000);
  //lcd.clear();
  
  /*
   sensorValue = analogRead(analogInPin);
  Serial.println(sensorValue); 
  delay(100); 
  if (sensorValue <1000 ||  sensorValue>1100) {
    limpiarLinea();
    lcd.print(opcion(sensorValue));
  }  
  */
}


String opcion(int sensor){
  if(sensor > 400 && sensor<500){
    return("Izquierda");
  }
  if(sensor > 600 && sensor<750){
    return("Seleccion");
  }
  if(sensor > 250 && sensor<350){   //valor modificado al diagrama anterior
    return("Abajo");
  }
  if(sensor > 90 && sensor<150){    //valor modificado al diagrama anterior
    return("Arriba");
  }
  if(sensor < 10){
    return("Derecha");
  }
}


void limpiarLinea(){
  lcd.setCursor(0, 2);
  for (int i = 0; i < 16; i = i + 1) {
    lcd.print(" ");
  }
  lcd.setCursor(0, 2);
}
