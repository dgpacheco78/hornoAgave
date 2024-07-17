/*
  MEDIDOR DE PH CON LCD
  ING. CRISTIAN LOPEZ
  ELECTRONICA NEXTIA FENIX
  PROGRAMA V.1
  DATOS A CONSIDERAR : FORMULA DE TBALA EN EXCEL (CALIBRACION)
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PINOUT:
  A0: P0(MODULO MEDIDOR DE PH )
  A4: SDA PANTALLA LCD I2C
  A5: SCL PANTALLA LCD 12C
  5V
  GND 
  

  con este programa va poder trabajar con el modulo medidor de pH,
  recuerde instalar la librerias  Wire.h y LiquidCrystal_I2C.h
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
float calibration_value = 27.189; // (VALOR DE CALIBRACION OBTENIDO DE GRAFICA FORMULA DE EXCEL)
int phval = 0;
unsigned long int avgval;
int buffer_arr[10], temp;
void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   MEDIDOR PH   ");
  lcd.setCursor(2, 1);
  lcd.print("NEXTIA FENIX");
  delay(5000);
  lcd.clear();
}
void loop() {
  for (int i = 0; i < 10; i++)
  {
    buffer_arr[i] = analogRead(A2);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buffer_arr[i] > buffer_arr[j])
      {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];
  float volt = (float)avgval * 5.0 / 1024 / 6;
  float ph_act = -6.3094 * volt + calibration_value; // (ACOMPLETAR VALORES CON DATOS OBTENIDOS DE FORMULA GRAFICA DE EXCEL)
  Serial.print("volt val:");
  Serial.println(volt);
 
  lcd.setCursor(0, 0);
  lcd.print("PH Val:");
  lcd.setCursor(8, 0);
  lcd.print(ph_act);
  lcd.setCursor(0, 1);
  lcd.print("VOLT Val:");
  lcd.setCursor(9, 1);
  lcd.print(volt);
  Serial.print("PH:");
  Serial.println(ph_act);
  delay(1000);
}
