#include <Arduino.h>
#include <string.h>
#include <EEPROM.h>

/*********** Macros & Varaibles ******************/
#define EEPROM_ADDRESS 0
#define LED_PIN 2
String Master_SIM = "";
bool stringComplete = false;
String msg = "";

/*********** Function Definition ****************/
void EEPROM_init();
void SIM808_init();
void serialEvent();


void setup()
{
  Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  EEPROM_init();//Initializing EEPROM with blocksize 1000
  SIM808_init();//Automatic Baud rate initialization  

  Master_SIM = EEPROM.readString(EEPROM_ADDRESS);//Read master number from EEPROM
}

void loop()
{
  serialEvent();
  /*********To Send AT Commands through Serial monitor**********/
  if (Serial.available())
  {                               // If anything comes in Serial (USB),
    Serial2.write(Serial.read()); // read it and send it out Serial2
  }

  if (stringComplete)
  {
    Serial.println(msg);
    if (msg.indexOf("CMT") > 0) //Check whether new SMS is recieved
    {
      char buf[512];
      msg.toUpperCase();
      msg.toCharArray(buf, msg.length());
      char *token = strtok(buf, "\"");
      token = strtok(NULL, "\"");
      token = strtok(NULL, "\"");
      token = strtok(NULL, "\"");
      token = strtok(NULL, "\"");
      if (msg.indexOf(Master_SIM) > 0)  //Authenticating the sender
      {
        if (msg.indexOf("ON") > 0)    // Check whether the keyword is "ON"
        {
          Serial.println("ON");
          digitalWrite(LED_PIN, HIGH);

          Serial2.print("AT+CMGS=\"");
          Serial2.print(Master_SIM);
          Serial2.print("\"");
          delay(100);
          Serial2.print("\nLED Turned ON at ");
          Serial2.print(token);
          Serial2.println("");
          delay(100);
        }
        else if (msg.indexOf("OFF") > 0)    //Check whether the keyword is "OFF"
        {
          Serial.println("OFF");
          digitalWrite(LED_PIN, LOW);

          Serial2.print("AT+CMGS=\"");
          Serial2.print(Master_SIM);
          Serial2.print("\"");
          delay(100);
          Serial2.print("\nLED Turned OFF at ");
          Serial2.print(token);
          Serial2.println("");
          delay(100);
        }
        else if (msg.indexOf("CHANGE") > 0)    // Check whether the keyword is "CHANGE"
        {
          msg.toCharArray(buf, msg.length());
          char *keyword = strtok(buf, " ");
          keyword = strtok(NULL, " ");
          keyword = strtok(NULL, " ");
          if (strlen(keyword) == 10)    // Check whether a valid mobile number is send
          {            
            EEPROM.writeString(EEPROM_ADDRESS, keyword);
            EEPROM.commit();
            delay(100);
            Serial2.print("AT+CMGS=\""); //Confirmation SMS to old master
            Serial2.print(Master_SIM);
            Serial2.print("\"");
            delay(100);
            Serial2.print("\nMobile number is changed to ");
            Serial2.print(keyword);
            Serial2.println("");
            Serial2.flush();
            Master_SIM = keyword;

            delay(5000);

            Serial2.print("AT+CMGS=\""); // Notifying new master
            Serial2.print(Master_SIM);
            Serial2.print("\"");
            delay(100);
            Serial2.print("\nWelcome Aboard !");
            Serial2.println("\nKEYWORDS:");
            Serial2.println("\nON ,OFF ,CHANGE ");
            Serial2.println("");
            delay(100);
          }
          else    // If user sends an undefined keyword
          {
            Serial.println("Invalid No.");
            Serial2.print("AT+CMGS=\""); //Invalid no. SMS
            Serial2.print(Master_SIM);
            Serial2.print("\"");
            delay(100);
            Serial2.print("\nInvalid Mobile No.");
            Serial2.println("");
            delay(100);
          }
        }
        else
        {
          Serial2.print("AT+CMGS=\"");
          Serial2.print(Master_SIM);
          Serial2.print("\"");
          delay(50);
          Serial2.print("\nInvalid Keyword");
          Serial2.println("");
        }
      }
    }
  }
  msg = "";
  stringComplete = false;
}

/******************** Function Definitions **************************/

void serialEvent()
{
  while (Serial2.available())
  {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    msg += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
    {
      stringComplete = true;
    }
  }
}


void EEPROM_init()
{
  Serial.println("...EEPROM  Init...");
  if (!EEPROM.begin(1000))
  { //Init EEPROM
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
}

void SIM808_init()
{
  Serial.println("...SIM808...");
  delay(500);
  Serial2.println("AT");
  delay(100);
  Serial2.println("AT");
}