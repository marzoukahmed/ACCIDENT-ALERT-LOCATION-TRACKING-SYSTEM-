// SIM card PIN (leave empty, if not defined)
const char simPIN[]   = "";

// Your phone number to send SMS: + (plus sign) and country code, for Israel +972, followed by phone number
#define SMS_TARGET  "+21654526013"

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <TinyGsmClient.h>
#define SIM800L_IP5306_VERSION_20200811

#include <Arduino.h>
#include "utilities.h"


// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial1

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
void setupModem()
{
       // Set console baud rate
  SerialMon.begin(115200);
  
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif

    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(100);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF);


}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
void sendATCommand(const char* command)
{
    SerialAT.println(command);
    delay(1000);  // Wait for the response
    while (SerialAT.available()) {
        SerialMon.write(SerialAT.read());
    }
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
void setup()
{
    SerialMon.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SerialMon.println("Initializing modem...");
  modem.restart();
  // use modem.init() if you don't need the complete restart
    setupModem();

    // Send each AT command and display the response
    sendATCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    delay(6000);  
    sendATCommand("AT+SAPBR=3,1,\"APN\",\"CMNET\"");
    delay(6000);  
    sendATCommand("AT+SAPBR=1,1");
     delay(6000);  
    sendATCommand("AT+SAPBR=2,1");
     delay(6000);  
    sendATCommand("AT+CLBS=1,1");
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
void loop()
{String message1 = "Accident Alert!! I'm at this location:";
  String locationResponse;
//  String locationResponse;
//  String locationResponse3;
  SerialMon.println("location...");
SerialAT.println("AT+CLBS=1,1");
delay(6000);  // Wait for the response

while (SerialAT.available()) {
  String line = SerialAT.readStringUntil('\n');
  if (line.startsWith("+CLBS:")) {
    locationResponse = line;
  }
}

SerialMon.println(locationResponse);
   

  // Extract latitude, longitude, and precision
  int comma1 = locationResponse.indexOf(',');
  int comma2 = locationResponse.indexOf(',', comma1 + 1);
  int comma3 = locationResponse.indexOf(',', comma2 + 1);
  String latitude = locationResponse.substring(comma1 + 1, comma2);
  String longitude = locationResponse.substring(comma2 + 1, comma3);
  String precision = locationResponse.substring(comma3 + 1, locationResponse.indexOf('\n'));

  SerialMon.println("Sending location via SMS...");
  String smsMessage = "Longitude: " + longitude + ", Latitude: " + latitude + ", Precision: " + precision;
  SerialMon.println(smsMessage);

  if(modem.sendSMS(SMS_TARGET, smsMessage)){
    SerialMon.println(smsMessage);
    message_with_data = message1 + "Latitude = " + String(latitude, 6) + "Longitude = " + String(longitude, 6) + " Link: https://maps.google.com/maps?&z=15&mrt=yp&t=k&q=" + String(latitude, 6) + "," + String(longitude, 6);
   modem.sendSMS(mobile_number, message_with_data);
   message_with_data = "";
    SerialMon.println("done");
  }
  else{
    SerialMon.println("SMS failed to send");
  }
  delay(7000);
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
