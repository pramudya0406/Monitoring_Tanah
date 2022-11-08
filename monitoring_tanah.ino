// No Produksi -> "CHM-xxx"
// Declare library disini //
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define TdsSensorPin A0
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,temperature = 25;

// Data wire is plugged into pin D4 on the Arduino 
#define ONE_WIRE_BUS 4 
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

String data_sender;

SoftwareSerial asw(7, 6); // (RX, TX)

String nilai = "";

void setup() {
  Serial.begin(115200);
  sensors.begin();
  asw.begin(115200);
  pinMode(TdsSensorPin,INPUT);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//sensor ds18b20
float suhu;
void sensor_ds18b20(){
  suhu = sensors.getTempCByIndex(0);

  return suhu;
}

//sensor tds
//float nilai_tds;
float tdsValue;
void sensor_tds(){
    static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
     printTimepoint = millis();
     for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
       analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
     averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
     float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
     float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
     tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5;
     //nilai_tds=tdsValue;
     //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      //Serial.print("TDS Value:");
      //Serial.print(tdsValue,0);
      //Serial.println("ppm");
   }
}  
//filter untuk TDS
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inisiasi payload

String message1;
String message2;
String message3;
String message4;
String message5;
String message6;

void dataStructure() {
  message1 = "[{\"value\":";
  message2 = ",\"id_sensor\":";
  message3 = ",\"status\":\"online\"},";
  message4 = "{\"value\":";
  message5 = ",\"id_sensor\":";
  message6 = ",\"status\":\"online\"}]";
}

// Inisiasi id //
#define id_suhuTanah 12
#define id_tds 13

void kirimData() {
  data_sender = "";

  data_sender += message1;
  data_sender += String(suhu);
  data_sender += message2;
  data_sender += id_suhuTanah;
  data_sender += message3;
  data_sender += message4;
  data_sender += String(tdsValue);
  data_sender += message5;
  data_sender += id_tds;
  data_sender += message6;


  nilai += data_sender;
  asw.println(nilai);
  Serial.println(nilai);

  delay(5000);
  nilai = "";
}

void cekdata() {
  delay(5000);

  nilai = "";
  nilai += suhu;
  nilai += ", ";
  nilai += tdsValue;
  nilai += ".. ";
  
  nilai = "";
}


// fungsi untuk millis, waktunya ditentukan dari hasil random //

long interval = 5000;

void loop() {
  sensor_tds();
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  dataStructure();
  if (currentMillis - previousMillis >= interval)
  {
  
    previousMillis = currentMillis;
    sensor_ds18b20();
    kirimData();
    cekdata();
  }

}
