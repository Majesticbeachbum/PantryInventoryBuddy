#include "ThingSpeak.h"
#include <SPI.h>
#include <WiFi101.h>
char ssid[] = ""; //Network SSID - REMOVED FOR PRIVACY
char pass[] = ""; // Network password - REMOVED FOR PRIVACY
int status = WL_IDLE_STATUS; // WiFi radio's status
unsigned long myChannelNumber = 2766451; // Current ThingSpeak Channel ID
const char * myWriteAPIKey = "4H9G1TPESY6H5NZP"; // Current ThingSpeak API key for writing data to channel
WiFiClient client;

int fsrPin1 = A1; // First sensor is connected to analog port A1
int fsrPin2 = A2; // Second sensor is connected to analog port A2
int fsrPin3 = A3; // Third sensor is connected to analog port A3
int fsrPin4 = A4; // Fourth sensor is connected to analog port A4
int numReadings = 10; // Number of readings for averaging

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect
    }

    Serial.println("Attempting to connect to WPA network...");
    status = WiFi.begin(ssid, pass);
    if (status != WL_CONNECTED) {
        Serial.println("Couldn't get a WiFi connection");
        while (true);
    } else {
        Serial.print("Connected to network: ");
        Serial.println(ssid);
    }
    // ThingSpeak setup
    ThingSpeak.begin(client);
    Serial.println("Waiting for first update...");
    delay(1000);
}
 
float gramConversion(int reading){
  // Currently, we can map the raw analog range of 0 - 1023 to the known force range of 0 - 49 Newtons
  float force = (float(reading) / 1023.0) * 49.0;
  // The calibrated force value is in Newtons, multiply by 101.97 to convert it to grams
  float weight = (force * 101.97) / 1.60;
  return weight;
}

int averageReading(int sensorPin) {
  long sum = 0; // Use long to prevent overflow when summing readings
  for (int i = 0; i < numReadings; i++) {
    sum += analogRead(sensorPin); // Add each reading to the sum
    delay(10); // Short delay between readings for stability
  }
  return sum / numReadings; // Return the average
}

void loop(void) {
  // Average readings for each sensor
  int avgReading1 = averageReading(fsrPin1);
  int avgReading2 = averageReading(fsrPin2);
  int avgReading3 = averageReading(fsrPin3);
  int avgReading4 = averageReading(fsrPin4);

  // Convert averaged readings to weight
  float Weight1 = gramConversion(avgReading1);
  float Weight2 = gramConversion(avgReading2);
  float Weight3 = gramConversion(avgReading3);
  float Weight4 = gramConversion(avgReading4);

  static int count = 1; // Count value for printing to the serial monitor and looping

  // Send readings to ThingSpeak
  ThingSpeak.setField(1, Weight1);
  ThingSpeak.setField(2, Weight2);
  ThingSpeak.setField(3, Weight3);
  ThingSpeak.setField(4, Weight4);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  // Print the count and each of the sensor values
  Serial.print("Reading Number: ");
  Serial.println(count);
  Serial.print("Weight at bucket 1 = ");
  Serial.print(Weight1);
  Serial.println(" grams ");
  Serial.print("Weight at bucket 2 = ");
  Serial.print(Weight2);
  Serial.println(" grams ");
  Serial.print("Weight at bucket 3 = ");
  Serial.print(Weight3);
  Serial.println(" grams ");
  Serial.print("Weight at bucket 4 = ");
  Serial.print(Weight4);
  Serial.println(" grams ");

  count++; // Increment count for looping condition
  delay(20000); // 20000 millisecond (20 second) delay in between each printed sensor reading
}
