#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

LiquidCrystal_I2C lcd(0x27,16,2);
MAX6675 thermo1(7, 6, 5);
float temp1;
String json;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Monitoreo Horno");

}

void loop() {
  temp1 = thermo1.readCelsius();
  lcd.setCursor(0, 1);
  lcd.print("Tem: " + String(temp1) + (char)223);
  json = "{\"temp1\":" + String(temp1) + "}";
  Serial.println(json);
  delay(800);
  lcd.setCursor(0, 1);
  lcd.print("Tem: ");
}
