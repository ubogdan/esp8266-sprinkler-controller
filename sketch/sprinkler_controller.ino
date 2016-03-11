
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <pgmspace.h>

const char* ssid = "SSID;
const char* password = "PASSWORD";
#define VERSION "1.1a"
int shiftDataPin = 12; // SER Pin 14 on 595 // Blue Wire
int shiftClockPin= 13; // SRCLK Pin 11 on 595 // Green Wire
int shiftLatchPin = 2; // RCLK Pin 12 on 595 // White Wire // connected together with pullup resistor
int shiftEnablePin = 16; // OE Pin 13 on 595 // Black Wire
int oneWireBusPin = 14; // Data pin for DS18B20 temperature sensor

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

byte output_status = B00000000;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(oneWireBusPin);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

void shiftWrite(byte value) {
    digitalWrite(shiftLatchPin, LOW);
    shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, value);
    digitalWrite(shiftLatchPin, HIGH);
}

void handleTest() {
  server.send(200, "text/html", F(""));
}

void handleAbout() {
  String message = "";
  message += "Firmware Version: " VERSION "\n";
  message += "Compile Date: " __DATE__ " " __TIME__ "\n";
  message += "Source: " __FILE__ "\n";
  message += "Flash Chip ID: ";
  message += ESP.getFlashChipId();
  message += "\nFlash Chip Real Size: ";
  message += ESP.getFlashChipRealSize();
  message += "\nFlash Chip Size: ";
  message += ESP.getFlashChipSize();
  message += "\nFlash Chip Speed: ";
  message += ESP.getFlashChipSpeed();
  message += "\nFlash Chip Mode: ";
  message += ESP.getFlashChipMode();
  message += "\nIP Address: ";
  message += WiFi.localIP();
  message += "\n";
  
  server.send(200, "text/plain", message);
}

void handleRoot() {
  String message = ""
"<!DOCTYPE html>"
"<html lang=\"en\">"
"  <head>"
"    <meta charset=\"utf-8\">"
"    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"    <!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->"
"    <title>Andre Sprinkler Controller</title>"
""
"    <!-- Bootstrap -->"
"    <link href=\"https://maxcdn.bootstrapcdn.com/bootswatch/3.3.6/darkly/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-kVo/Eh0sv7ZdiwZK32nRsp1FrDT3sLRLx3zVpSSTI9UdO5H02LJNLBg5F1gwvKg0\" crossorigin=\"anonymous\">"
""
"    <!-- Custom styles for this template -->"
"    <link href=\"bootstrap.css\" rel=\"stylesheet\">"
""
"    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->"
"    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->"
"    <!--[if lt IE 9]>"
"      <script src=\"https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js\"></script>"
"      <script src=\"https://oss.maxcdn.com/respond/1.4.2/respond.min.js\"></script>"
"    <![endif]-->"
"  </head>"
"  <body>"
"    "
"    <!-- Fixed navbar -->"
"    <nav class=\"navbar navbar-default navbar-fixed-top\">"
"      <div class=\"container\">"
"        <div class=\"navbar-header\">"
"          <button type=\"button\" class=\"navbar-toggle collapsed\" data-toggle=\"collapse\" data-target=\"#navbar\" aria-expanded=\"false\" aria-controls=\"navbar\">"
"            <span class=\"sr-only\">Toggle navigation</span>"
"            <span class=\"icon-bar\"></span>"
"            <span class=\"icon-bar\"></span>"
"            <span class=\"icon-bar\"></span>"
"          </button>"
"          <a class=\"navbar-brand\" href=\"#\">Sprinkler Controller</a>"
"        </div>"
"        <div id=\"navbar\" class=\"collapse navbar-collapse\">"
"          <ul class=\"nav navbar-nav\">"
"            <li class=\"active\"><a href=\"#\">Home</a></li>"
"            <li><a href=\"/about\">About</a></li>"
"            <li><a href=\"/update\">Update Firmware</a></li>"
"          </ul>"
"          <ul class=\"nav navbar-nav navbar-right\">"
"            <li class=\"disabled\"><a href=\"#\">Temperature: ";

  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempC(insideThermometer);
  message += tempC;

message += "C</a></li>"
"          </ul>"
"        </div><!--/.nav-collapse -->"
"      </div>"
"    </nav>"
""
"    <!-- Begin page content -->"
"    <div class=\"container\">"
"      <div class=\"page-header\">"
"        "
"      </div>"
"    </div>"
""
"    <div class=\"container\">"
"";

  for (int c=0; c<8; c++) {
    message += "      <p>";

    message += "        Station ";
    message += c+1;
    message += " &nbsp;\n";

    // Print First Button (The OFF Button)
    if (output_status & (B00000001 << c)) {
      // Currently On
      message += "            <a href=\"switch?pin=";
      message += c;
      message += "&state=0\" class=\"btn btn-default\">Off</a>";
    } else {
      // Currently Off
      message += "            <a href=\"switch?pin=";
      message += c;
      message += "&state=0\" class=\"btn btn-success\">Off</a>";
    }

    // Print Second Button (The ON Button)
    if (output_status & (B00000001 << c)) {
      // Currently On
      message += "            <a href=\"switch?pin=";
      message += c;
      message += "&state=1\" class=\"btn btn-warning\">On</a>";
    } else {
      // Currently Off
      message += "            <a href=\"switch?pin=";
      message += c;
      message += "&state=1\" class=\"btn btn-default\">On</a>";
    }
    
    message += "</tr>\n";
  }
  


  message += ""
  "       "
"    </div>"
""
""
"    <!-- Bootstrap core JavaScript"
"    ================================================== -->"
"    <!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->"
"    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>"
"    <!-- Include all compiled plugins (below), or include individual files as needed -->"
"    <script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\" integrity=\"sha384-0mSbJDEHialfmuBBQP6A4Qrprq5OVfW37PRR3j5ELqxss1yVqOtnepnHVP9aJ7xS\" crossorigin=\"anonymous\"></script>"
""
"  </body>"
"</html>"
  "";
  
  
  server.send(200, "text/html", message);
}

void handleSwitch() {
  int pin = server.arg(0).toInt();
  int state = server.arg(1).toInt();
  bitWrite(output_status, pin, state);
  shiftWrite(output_status);
  handleRoot();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void check_wifi(void) {
  // Wait for connection

  // If no connection, set all outputs to 0
  if (WiFi.status() != WL_CONNECTED) {
    shiftWrite(B00000000); // Set all outputs to 0
  } else {
    return;
  }

  Serial.print("CONNECTING TO WIFI:");
  // Wait until we are reconnected
  while (WiFi.status() != WL_CONNECTED) {
    // Flash the LED that's connected to the enable pin while waiting for Wifi
    digitalWrite(shiftEnablePin, HIGH);
    delay(100);
    digitalWrite(shiftEnablePin, LOW);
    delay(100);
    Serial.print(".");
  }
  Serial.println(":CONNECTED");
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

void setup(void) {
    pinMode(shiftDataPin, OUTPUT);
    pinMode(shiftClockPin, OUTPUT);
    pinMode(shiftLatchPin, OUTPUT);
    shiftWrite(B00000000); // Set all outputs to 0
    pinMode(shiftEnablePin, OUTPUT);
    digitalWrite(shiftEnablePin, LOW); // Enable all outputs
    Serial.begin(115200);
    Serial.println("Device Power Up");
    
    WiFi.begin(ssid, password);
  Serial.println("");

  check_wifi();
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("sprinkler")) {
    Serial.println("MDNS responder started");
  }

  httpUpdater.setup(&server);

  server.on("/", handleRoot);

  server.on("/switch", handleSwitch);
   server.on("/about", handleAbout);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Setup temperature sensor
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 

  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data
  
}

void loop() {
  check_wifi();
  server.handleClient();
}




