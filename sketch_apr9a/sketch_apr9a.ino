#include <TinyGPS++.h>
#include <utility>


int GPSBaud = 9600; //Baudrate for GPS on Serial2
int latPositions[] = {0,0,0,0,0,0,0,0,0,0};
int longPositions[] = {0,0,0,0,0,0,0,0,0,0};
int counterLat = 0;
int counterLong = 0;

TinyGPSPlus gps; 

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(GPSBaud); //Starting Serial for GPS-Module on HardwareSerial2
  delay(100);
}

void start_GPS() // Function for starting the GPS-Module and producing the Serial Output of the received GPS Messages
{
  // Displaying information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0) //GPS-Module is connected to HardwareSerial2
    if (gps.encode(Serial2.read()))
      display_GPS_Info();
      

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
 
  if (millis() > 6000 && gps.charsProcessed() < 10) 
  {
    Serial.println("No GPS detected");
    while(true);
  }
}

void display_GPS_Info() //function used in the start_GPS() function that displays GPS Data in the Serial Monitor and loggs the Data in the LoRa payload dataset 
{
  delay(200);
  if (gps.location.isValid()) //decoded GPS Location gets checked for validity by the function isValid() of the TinyGPS++ Library 
  {
    get_GPS_Data();
    stdDev();
    printAvg();
    counterLat, counterLong = 0;
  }
  else
  {
    Serial.println("GPS Location: Not Available");
  }

  if (gps.time.isValid()) //decoded GPS Time gets checked for validity by the function isValid() of the TinyGPS++ Library
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());  
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.println(gps.time.second());
  }
  else
  {
    Serial.println("GPS Time: Not Available");
  }
}

void get_GPS_Data()
{
  if (gps.location.isValid())
  {
    for (int i = 0; i < 10; i++)
    {
      latPositions[i] = gps.location.lat();
      longPositions[i] = gps.location.lng();
      delay(500);
    }
  }
}

//get the standard deviation of the latitude and longitude and eliminate the 6 values that are the farthest from the mean
void stdDev()
{
  int avgLat = getAVGlat();
  int avgLong = getAVGlong();
  int sumLat2 = 0;
  int sumLong2 = 0;
  for (int i = 0; i < 10; i++)
  {
    sumLat2 += (latPositions[i] - avgLat) * (latPositions[i] - avgLat);
    sumLong2 += (longPositions[i] - avgLong) * (longPositions[i] - avgLong);
  }
  int stdDevLat = sqrt(sumLat2 / 10);
  int stdDevLong = sqrt(sumLong2 / 10);
  for (int i = 0; i < 10; i++)
  {
    if (abs(latPositions[i] - avgLat) > 2 * stdDevLat)
    {
      latPositions[i] = 0;
      counterLat++;
    }
    if (abs(longPositions[i] - avgLong) > 2 * stdDevLong)
    {
      longPositions[i] = 0;
      counterLong++;
    }
  }
}

void printAvg()
{
  int sumLat = 0;
  int sumLong = 0;
  for (int i = 0; i < 10; i++)
  {
    sumLat += latPositions[i];
    sumLong += longPositions[i];
  }
  int trueLat = sumLat / 10-counterLat;
  int trueLong = sumLong / 10-counterLong;
  Serial.print("Latitude: ");
  Serial.println(trueLat);
  Serial.print("Longitude: ");
  Serial.println(trueLong);
}

int getAVGlat()
{
  int sumLat = 0;
  for (int i = 0; i < 10; i++)
  {
    sumLat += latPositions[i];
  }
  return sumLat / 10;
}

int getAVGlong()
{
  int sumLong = 0;
  for (int i = 0; i < 10; i++)
  {
    sumLong += longPositions[i];
  }
  return sumLong / 10;
}

void loop()
{
  start_GPS();
  delay(100);
}