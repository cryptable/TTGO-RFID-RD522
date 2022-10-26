/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         33         // Configurable, see typical pin layout above
#define SS_PIN          2          // Configurable, see typical pin layout above

#define SPI_CS          SS_PIN
#define SPI_MISO        4
#define SPI_MOSI        13
#define SPI_SCK         14

// Replace the next variables with your SSID/Password combination
/*
const char* ssid = "<Wifi SSID>";
const char* password = "<Wifi password>";
*/
const char* ssid = "Cryptable";
const char* password = "aquatictomato116";

// Add your MQTT Broker IP address, example:
/*
const char* mqtt_server = "<MQTT broker IP>";
const char* mqtt_username = "<MQTT Username>";
const char* mqtt_password = "<MQTT Password>";
const char* mqtt_student_id = "<Student ID without spaces>";
*/

const char* mqtt_server = "3.72.41.159";
const char* mqtt_username = "IoT";
const char* mqtt_password = "DitIsGoed";
const char* mqtt_student_id = "s123456";

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
void setup_wifi();
void on_message(char* topic, byte* message, unsigned int length);
void reconnect();
char *stringHex(byte *buffer, byte bufferSize);

WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Arduino setup routine, which initializes:
// - Serial port to send information to terminal
// - SPI to communicate with the RFID reader
// - RFID reader
// - the Wifi connection
// - mqtt server callback functions to receive the responses from the server.py application 
void setup() { 
  Serial.begin(115200);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);			// Init SPI bus
	rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(on_message);
}

// Main Arduino loop
// - connect to MQTT server and retrieve its messages if there are messages
// - When a card is present, read the UID
// - When we have a new UID, we will send it to MQTT Server with a message "Hello, world"
// Does this makes sense? Not yet!
void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    char publishTopic[255] = {0};
    sprintf(publishTopic, "AP/rfid/lezer/%s/%s", mqtt_student_id, stringHex(rfid.uid.uidByte, rfid.uid.size));
    mqttClient.publish(publishTopic, "Hello, world");
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

// Code which setup the Wifi connection
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Code when the server.py publishes (send back) its response
void on_message(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if(messageTemp == "0"){
    Serial.println("door closed");
  }
  else {
    Serial.println("door open");
  }
}

// Connect to MQTT Server
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    // Attempt to connect
    if (! mqttClient.connect(mqtt_server, mqtt_username, mqtt_password)) {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    Serial.print(".");
  }
  char subscribeTopic[255] = {0};
  sprintf(subscribeTopic, "AP/rfid/deur/%s/+", mqtt_student_id);
  mqttClient.subscribe(subscribeTopic);

}

// Helper routine to dump a byte array as hex values to Serial. 
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Helper routine to dump a byte array as hex values to Serial. 
char bufferke[256];
char *stringHex(byte *buffer, byte bufferSize) {
  char hexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  memset(bufferke, 0, sizeof(bufferke));
  for (byte i = 0; i < bufferSize; i++) {
    bufferke[i*2+1] = hexChars[buffer[i] & 0x0F];
    bufferke[i*2] = hexChars[buffer[i]>>8 & 0x0F];
  }

  return bufferke;
}

// Helper routine to dump a byte array as dec values to Serial.
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}