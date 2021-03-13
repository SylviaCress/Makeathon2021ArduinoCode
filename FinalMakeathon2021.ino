#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include "DHT.h"
#define DHTPIN 8     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

//RFID
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 12 // Configurable, see typical pin layout above
#define SS_PIN 53 // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance


LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7)
DHT dht(DHTPIN, DHTTYPE);

//temp sens
float initTemp;
int fsrAnalogPin = A0;
int fsrReading;

//speed test
const int GREEN_LED = 31;
const int RED_LED = 30;
const int L_SWITCH = 35;
const int R_SWITCH = 34;
const int NUM_TRIALS = 10;
volatile int led = GREEN_LED;
volatile int swtch = L_SWITCH;
volatile int nswtch = R_SWITCH;
volatile long time_cnt = 0;
volatile long total_reaction_time = 0;
volatile float avg_reaction_time = 0;

//breathalizer
float sensorValue; //variable to store sensor value
int breathPin = A1;

void setup() {

  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  delay(4); // Optional delay. Some board do need more time after init to be ready, see Readme

  //speedTest
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(L_SWITCH, INPUT);
  pinMode(R_SWITCH, INPUT);

  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  dht.begin();
  Serial.begin(9600);
  lcd.print("Welcome to Vital");
  lcd.setCursor(0, 1);
  lcd.print("Scan your RFID");

}

void loop()
{

if ( ! mfrc522.PICC_IsNewCardPresent()) {

return;

}
  if ( ! mfrc522.PICC_ReadCardSerial()) {

    return;

  }


  
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  // Ultimately would want this connected to a database with records of users, their medical history, and thier RFID codes

  content.toUpperCase();

  if (content.substring(1) == "6A 4E B1 19" ||content.substring(1) == "04 53 07 43 35 4B 80" ) //change here the UID of the card/cards that you want to give access

  {
    lcd.clear();
    delay(1000);
    String namePerson;
    if(content.substring(1) == "6A 4E B1 19")
    {
      namePerson = "Collin";
    }
    if(content.substring(1) == "04 53 07 43 35 4B 80")
    {
     namePerson = "Clark";
    }
    lcd.print("Hello ");
    lcd.print(namePerson);
    
    delay(5000);
    lcd.clear();

    delay(2000);

    
    initTemp = dht.readTemperature(true);
   
    lcd.print("Temp Check");
    lcd.setCursor(0,1);
    lcd.print("Use Sensor");
    delay(100);
    while (dht.readTemperature(true) < (initTemp + .5))
    {
      delay(1000);
    }
    lcd.setCursor(0, 0);
    lcd.print("                               ");
    delay(1000);
    lcd.clear();
    delay(1000);
    lcd.print("Keep Holding");
    float initTime = millis();
    while (millis() < initTime + 8000) //keeps going for 10 seconds
    {

    }
  
    float finalTemp = dht.readTemperature(true);
    finalTemp = finalTemp+20; //send final temp

     if (isnan(finalTemp)) {
     finalTemp = 98;
     }
     
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your Temp: ");
    lcd.setCursor(0, 1);
    lcd.print(finalTemp);
    delay(5000);

    int finalWeight = takeWeight(); //send finalWeight
    
    int breathResult = breathTest(); //send breathResult

    
    lcd.clear();
    delay(2000);
    lcd.print("Speed test");
    delay(2000);
    
    float avgSpeed = doReactionTest(); //send avgSpeed
    lcd.clear();
    delay(2000);
    lcd.print("Your speed:");
    lcd.setCursor(0,1);
    lcd.print(avgSpeed);
    lcd.print("ms");
    delay(2000);
    
    String finalString = "name=" + namePerson + "&temperature=" + String(finalTemp) + "&gas=" + String(breathResult) +"&acc=" +String(avgSpeed) +"&force=" + String(finalWeight);
    Serial.print(finalString);
    endOfTest();



    
  }

  // Ultimately would want this connected to a database with records of users, their medical history, and thier RFID codes

  else {

    // You sign up for an assigned doctor, but you can also get professional diagnosis on the fly and be told if you should schedule an appointment with your doctor
    lcd.clear();
    lcd.print("device is not recongized.");

  }

   lcd.clear();
   delay(1000);
   
  lcd.print("Welcome to Vital");
  lcd.setCursor(0, 1);
  lcd.print("scan your RFID");
}

int takeWeight()
{
  int maxWeight = 0;
  lcd.clear();
  delay(1000);
  lcd.print("Stand on scale");
  int weight = analogRead(fsrAnalogPin);


  while (weight <= .5)
  {
    weight = analogRead(fsrAnalogPin);
  
    delay(1000);
  }

  float initTime = millis();
  lcd.clear();
  lcd.print("Stay on scale");
  while (millis() < initTime + 7000) //waits 7 seconds
  {
    weight = analogRead(fsrAnalogPin);
    if (weight > maxWeight)
    {
      maxWeight = weight;
    }
    delay(100);
  }

  int personWeight = maxWeight*10;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your Weight: ");
  lcd.setCursor(0, 1);
  lcd.print(personWeight);
  delay(5000);
  return personWeight;
}

void blinkBoth(){
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(500);
}

float doReactionTest(){
blinkBoth();
blinkBoth();
blinkBoth();
   total_reaction_time = 0;
   for(int i = 0; i < NUM_TRIALS; i++){
   digitalWrite(GREEN_LED, LOW);
   digitalWrite(RED_LED, LOW);
   delay(random(500,2000));
    time_cnt = 0;
   if(random(0, 10) < 5) {
     led = GREEN_LED;
      swtch = L_SWITCH;
      nswtch = R_SWITCH;
   } else {
     led = RED_LED;
     swtch = R_SWITCH;
     nswtch = L_SWITCH;
   }
   digitalWrite(led, HIGH);
   // While the correct switch is not pressed or the wrong switch is pressed
   // essentially we have to wait for only correct switch to be pressed
   while ((digitalRead(swtch) == HIGH) || (digitalRead(nswtch) == LOW)) {
       delay(1);
       time_cnt = time_cnt + 1;
   }
   // wait for switch to be released
   while (digitalRead(swtch) == LOW) {}
    total_reaction_time = total_reaction_time + time_cnt;

  } 
  blinkBoth();
 blinkBoth();
blinkBoth();
  avg_reaction_time = total_reaction_time / float(NUM_TRIALS);
  return avg_reaction_time;
}

int breathTest()
{
  lcd.clear();
  lcd.print("Breath test");
  delay(3000);
  lcd.setCursor(0,1);
  lcd.print("BLOW");
  int maxVal=0;
  for(int j=0; j<500; j++)
  {
    int val = analogRead(breathPin);
    if(val>maxVal)
    {
      maxVal = val;
    }
    delay(10);
  }

lcd.clear();
delay(200);

  if(maxVal > 300)
  {
    lcd.print("Alc detected");
  }
  else
  {
    lcd.print("No Alc detected");
  }

  delay(2000); // wait 2s for next reading
  return maxVal;
}

void endOfTest()
{
  lcd.clear();
  lcd.print("Checkup Done");
  lcd.setCursor(0, 1);
  lcd.print("Thank You");
  delay(5000);
}
