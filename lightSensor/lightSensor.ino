
#include <Wire.h>
#include <Digital_Light_TSL2561.h>
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  TSL2561.init();
}
 
void loop()
{
  
  if (TSL2561.readVisibleLux()>0){
    Serial.print("The Light value is: ");
    Serial.println(TSL2561.readVisibleLux());
  }
  delay(200);
}