#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <HX711.h>


//pin definition
#define HX711WATER_CLK 14
#define HX711WATER_DAT 12
#define HX711FOOD_CLK 4
#define HX711FOOD_DAT 13
#define VMA406_SIGNAL 0
#define AIN1 2
#define AIN2 15
#define PWMA 5


/*
 * EEPROM Location Definitions:
 * MEM[0-15] = FOOD AND WATER USER PROGRAMMED TIMES
 * MEM[20] = TIME OFFSET IS NEGATIVE OR POSITIVE
 * MEM[21] = VALUE FOR TIME OFFSET
 * MEM[25] = USED TO DETERMINE IF THE MCU HAS CONNECTED WITH THE APP YET AND SET A CURRENT OFFSET FOR TIME
 * MEM[50] = USER PROGRAMMED AMOUNT OF FOOD IN THE BOWL
 * MEM[55] = USER PROGRAMMED AMOUNT OF WATER IN THE BOWL
 */


//function definition
void initializeConnect();
void sendNTPpacket(IPAddress& address);
void NTPprinttime();
void wait2Connect();
String prepareHtmlPage();
void handleBody();
int functionSelector(const char* command);
void timeFeedDrink();
void fillFood();
void fillWater();
void setParameters(const char* command);
void convToTimeArr();
int conCat(int a, int b);
void setSystemTime(const char* command);
void setWeights(const char* command);
void convToWeightArr();
void getWeights();



//global variables used throughout program
String message = "";        //used to store the incoming data from the app
const char* command = "";   //used to store the command from the incoming data in message
int currentTime;            //the current unix time
int timeCheck[8];           //used to determine when the animal will get fed(in elements 0-3) and when it will get water(in elements 4-7), values stored in EEPROM in mem[0-15] so times are not lost on reboot
int timeOffset = 0;         //used to determine the offset from unix time and OS time of the device connecting, value stored in EEPROM mem[20]
float setFood = 0;            //weights set for food by user
float setWater = 0;           //weights set for water by user
float maxFood = .11;        //max weight for food in bowl //.29 is the full bowl weight but due to clogging, it is not used 
float maxWater = .4;       //max weight for water in bowl
float currFood = 0;         //current weight of food
float currWater = 0;        //current weight of water
float foodOffset = -.81;    //food weight offset to zero
float waterOffset = .48;     //water weight offset to zero
float foodCali = 103250.5;  //food calibration value
float waterCali = 128650.5; //water calibration value

//HX711 definitions
HX711 scaleFood; 
HX711 scaleWater;


//Local WiFi ssid and password
const char* ssid = "";
IPAddress temp;


// local port to listen for UDP packets
unsigned int localPort = 2390;
WiFiUDP udp; //define the udp variable for the WiFiUDP class


//NTP information to get real time updates
IPAddress timeServer(129, 6, 15, 28); // time.google.com NTP server
IPAddress timeServerIP; // time.google.com NTP server address
const char* ntpServerName = "time.google.com";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets


//initialize variable to make esp8266 a server
ESP8266WebServer server(80);


//initialize wifi manager in order to find WiFi ssid
WiFiManager wifiManager;



/*-------------------------------Initialization of Items---------------------*/
void setup(void)
{
  int j = 0;

  //define relay pin to be pulled low
  pinMode(VMA406_SIGNAL, OUTPUT);
  digitalWrite(VMA406_SIGNAL, LOW);

  //begin serial monitor and EEPROM
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(2000);


  //used to test timeOffset, comment out unless testing:
  //EEPROM.write(25, 255);

  
  //read EEPROM for variables, MEM[25] is used to determine if time has been initialized
  //MEM[20] determines if offset is negative or positive
  //MEM[21] determines the offset amount
  timeOffset = conCat(EEPROM.read(21), 0);
  if (EEPROM.read(20) == 1)
  {
    timeOffset = -1 * timeOffset;
  }
  if (EEPROM.read(25) != 0)
  {
    timeOffset = 0;
  }


  //reset settings for testing
  //wifiManager.resetSettings();


  //determine if the wifi is autoconnected, if not jump to initialize connect function
  //timeout after 15 tries to connect and jump to initialize connect
  if (WiFi.getAutoConnect())
  {
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED)
    {
      wait2Connect();
      j = j + 1;
      if (j > 15)
      {
        initializeConnect();
      }
    }
    j = 0;
  }
  else
  {
    initializeConnect();
  }
  

  // print out the local IP address the controller is connected to
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());


  //start up udp stream
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


  // Get the IP address of the NTP server
  while (!WiFi.hostByName(ntpServerName, timeServerIP))
  {
    Serial.println("DNS lookup failed");
    Serial.flush();
  }
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);


  //begin the server handle the body of any incoming messages
  server.on("/body", handleBody);
  server.begin();


  //calls convToTimeArr in order to pull EEPROM and place it in timeCheck array to be used in loop
  convToTimeArr();

  //calls convToWeightArr in order to assign the EEPROM and place it into setFood and setWater values
  convToWeightArr();
  

  //set up loadcells, call getWeights in order to display current weight on screen
  scaleFood.begin(HX711FOOD_DAT, HX711FOOD_CLK);
  scaleWater.begin(HX711WATER_DAT, HX711WATER_CLK);
  scaleFood.set_scale(foodCali);
  scaleWater.set_scale(waterCali);
  getWeights();


  //prints the current time offset from the controller to user's timezone
  Serial.println("-----Current Time Offset-----");
  Serial.println(timeOffset);

  //ensure that the motor is going forward in the right direction and declare the pins of the ROB-14450 motor driver module
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 0);
  
}



/*-------------------------------Loop---------------------------------------*/
void loop()
{
  command = NULL;

  //check if WiFi is connected and handles any possible incoming data from the client
  if (WiFi.isConnected())
  {
    server.handleClient();

    if (command)
    {
      functionSelector(command);
    }

    // send an NTP packet to a time server
    sendNTPpacket(timeServerIP);
    // wait to see if a reply is available
    delay(2500);

    //parse incoming udp reply
    int cb = udp.parsePacket();
    if (!cb)
    {
      Serial.println("no packet yet");
    }
    else
    {
      NTPprinttime();
      timeFeedDrink();
      getWeights();
    }
    // wait a second before asking for the time again
    delay(1000);
  }
  else
  {
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED)
    {
      wait2Connect();
    }
  }
}



/*------------------------------------Functions-----------------------------------*/
void initializeConnect()
{
  int times = 0;

  //set the mode that the esp8266 will run in and then connect to the desired network
  WiFi.mode(WIFI_AP_STA);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect())
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  Serial.printf("Connecting to %s\n", ssid);
  
  //set autoConnect if it is already not configured
  if (WiFi.getAutoConnect() == false)
  {
    WiFi.setAutoConnect(true);
  }

  //ensure that WiFi is connected before proceeding
  while (WiFi.status() != WL_CONNECTED)
  {
    wait2Connect();
  }

  //disable DHCP for the device and configure a staticIP by obtaining the gateway and subnet
  //so the device may be accessed by the application
  IPAddress gateway(WiFi.gatewayIP());
  IPAddress subnet(WiFi.subnetMask());
  Serial.println();
  Serial.println("Gateway IP: ");
  Serial.print(gateway);
  Serial.println("\nSubnet IP: ");
  Serial.println(subnet);
  
  //configures static IP address by using changing the last portion of the LAN IP to 255
  temp = gateway;
  temp[3] = 255;
  IPAddress staticIP(temp);
  Serial.println("IP Static: ");
  Serial.println(staticIP);
  Serial.println();
  
  WiFi.config(staticIP, gateway, subnet);
  
  ESP.reset();
}


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  ssid = (myWiFiManager->getConfigPortalSSID()).c_str();
}


//function used to send NTP packet to the time.google server
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  // Initialize values needed to form NTP reques
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision

  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


//function to print out and store the appropriate time
void NTPprinttime()
{
  //declare variables
  int hour;
  int minute;

  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE);

  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  // now convert NTP time into everyday time:
  Serial.print("Unix time = ");

  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = secsSince1900 - seventyYears;

  //assign the hour and minute to declared integers
  hour = (epoch  % 86400L) / 3600;
  minute = (epoch  % 3600) / 60;

  //current time is now the concatenated values of hour and minute, assigned by called conCat function
  currentTime = conCat(hour, minute) - timeOffset;

  if(currentTime > 2400)
  {
    currentTime = currentTime - 2400;
  }


  //print the current time to the monitor
  Serial.print("Current Time: ");
  Serial.println(currentTime);
}


//waiting function for connection timeouts
void wait2Connect()
{
  delay(500);
  Serial.print(".");
}


//Handler for the body path, will send Body received if there is an argument sent to the server
void handleBody()
{

  if (server.hasArg("plain") == false) { //Check if body received

    server.send(200, "text/plain", "Body not received");
    return;

  }

  //prints out the body received from the client app
  message = "Body received:\n";
  message += server.arg("plain");
  message += "\n";

  //sets command to the received body
  command = server.arg("plain").c_str();

  //if the current command contains get time(used to get the current weights at this time) send back the current weights
  if(strstr(command, "get_Time"))
  {
    server.send(200, "text/plain", String(currFood) + "," + String(currWater));
    return;
  }

  //sends a reply to the app to acknowledge response
  server.send(200, "text/plain", message);
  Serial.println(message);
}


//selects the appropriate command from the received body by the server, determines if the command has keywords within them
int functionSelector(const char* command)
{
  if (strstr(command, "FillFood") != NULL)
  {
    fillFood();
    return 0;
  }
  if (strstr(command, "FillWater") != NULL)
  {
    fillWater();
    return 0;
  }
  if (strstr(command, "Parameters") != NULL)
  {
    setParameters(command);
    return 0;
  }
  if (strstr(command, "Current Time") != NULL)
  {
    setSystemTime(command);
    return 0;
  }
  if (strstr(command, "Quantities") != NULL)
  {
    setWeights(command);
    return 0;
  }
  else
  {
    return 0;
  }
}


//determines if the current time is equal an assigned time
void timeFeedDrink()
{
  for (int i = 0; i < 8; i++)
  {
    //check if the current time is equal to an assigned feeding or watering time
    if (timeCheck[i] == currentTime || timeCheck[i] == currentTime - 1)
    {
      if (i > 3)
      {
        fillWater();
      }
      else
      {
        fillFood();
      }
    }
  }
}


//fills the food bowl for the animal at this certain time
void fillFood()
{
  delay(1000);
  scaleFood.power_up();
  while(currFood < (maxFood*(setFood/100)))
  {  
    analogWrite(PWMA, 1023);
    currFood = scaleFood.get_units() + foodOffset; 
    Serial.println(currFood);
  }
  analogWrite(PWMA, 0);
  scaleFood.power_down();
}


//fills the water bowl for the animal at this certain time
void fillWater()
{
  Serial.println("Fill Das Water!");
  delay(1000);
  scaleWater.power_up();
  while(currWater < (maxWater*(setWater/100)))
  {  
    digitalWrite(VMA406_SIGNAL, HIGH);
    currWater = scaleWater.get_units() + waterOffset; 
    Serial.println(currWater);
  }
  digitalWrite(VMA406_SIGNAL, LOW);
  scaleWater.power_down();
}



//sets the parameters of when the animal will be fed or watered
void setParameters(const char* command)
{
  //define variables
  char splitParam[100] = "";
  char *token;
  char *newParams[8];
  int convertParams[8];
  int firstInt;
  int secondInt;
  int k = 0;

  //copy command to the splitParam character array ensure that "Parameters: [" and "]" is left out
  for (int i = 14; i < strlen(command) - 2; i++)
  {
    splitParam[i - 14] = command[i];
  }
  Serial.println(splitParam);

  //splits the string using strtok into substrings
  token = strtok(splitParam, ", ");
  newParams[k] = token;

  while (token)
  {
    k++;
    token = strtok(NULL, ", ");
    newParams[k] = token;
  }

  //converts from char* to int for each element in the array then stores the values in EEPROM
  //since EEPROM only holds values from 0-255, split up the first and second part of the times then calls convToTimeArr()
  //HOLDS EEPROM IN LOCATIONS MEM[0-15]
  k = 0;
  for (int i = 0; i < 8; i++)
  {
    convertParams[i] = atoi(newParams[i]);
    firstInt = convertParams[i] / 100;
    secondInt = convertParams[i] % 100;
    EEPROM.write(i + k, firstInt);
    k++;
    EEPROM.write(i + k, secondInt);
    delay(500);
  }

  EEPROM.commit();
  
  //calls convert time to array, to convert the times into an array
  convToTimeArr();
}



void setWeights(const char* command)
{
  //define variables
  char splitParam[100] = "";
  char *token;
  char *newParams[2];
  int convertParams[2];
  int k = 0;

  //copy command to the splitParam character array ensure that "Parameters: [" and "]" is left out
  for (int i = 15; i < strlen(command) - 2; i++)
  {
    splitParam[i - 15] = command[i];
  }

  //splits the string using strtok into substrings
  token = strtok(splitParam, ", ");
  newParams[k] = token;

  while (token)
  {
    k++;
    token = strtok(NULL, ", ");
    newParams[k] = token;
  }

  //Converts the new parameters to integers then written to MEM at locations [50](for food) and [55](for water) then calls convToWeightArr to set global variables
  convertParams[0] = atoi(newParams[0]) * 10;
  convertParams[1] = atoi(newParams[1]) * 10;

  EEPROM.write(50, convertParams[0]);
  EEPROM.write(55, convertParams[1]);
  EEPROM.commit();

  convToWeightArr();
  
}



//converts the EEPROM into an array usable by the loop
void convToTimeArr()
{
  int k = 0;

  Serial.println("--------Current Configured Times--------");
  
  for (int i = 0; i < 8; i++)
  {
    //timecheck is going to be the first and next value of the associated EEPROM, timecheck is a global variable, calls conCat function to concatenate these values
    timeCheck[i] = conCat(EEPROM.read(i + k), EEPROM.read(i + k + 1));
    k++;

    //print out the current times for feeding and drinking
    Serial.print("Time ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(timeCheck[i]);
  }
}



void convToWeightArr()
{
  setFood = EEPROM.read(50);
  setWater = EEPROM.read(55);
  
  Serial.println("-------Current Configured Weights-------");
  Serial.print("Feed Food Weight: ");
  Serial.println(setFood);
  Serial.print("Drink Water Level: ");
  Serial.println(setWater);  
}



//converts two integers to strings and then concatenates them then combines them back into a singular integer
int conCat(int a, int b)
{
  // Convert both the integers to string
  String s1 = String(a);
  String s2 = String(b);

  //if s2 is only a single digit, add a zero to make it two digits
  if (s2.length() < 2)
  {
    s2 = "0" + s2;
  }

  // Concatenate both strings
  String s = s1 + s2;

  // Convert the concatenated string to integer
  int c = s.toInt();

  return c;
}



/*------------------------------------Set Current Time Functions-----------------------------------*/
//parse the packet and seperate the data then subtract the unix time from current time
void setSystemTime(const char* command)
{
  //variable initialization
  char splitParam[4] = "";
  int OSTime = 0;
  int flipInt;
  int oldTimeOffset = timeOffset;
  
  //copy command to the splitParam character array ensure that "Current Time(Hours): " [" and "]" is left out
  //then convert to an integer
  for (int i = 21; i < strlen(command); i++)
  {
    splitParam[i - 23] = command[i];
  }
  OSTime = atoi(splitParam);
  //determine the time offset and save to EEPROM, splits the timeoffset into two memory locations to determine if it is negative or positive.
  //HOLDS EEPROM IN LOCATION MEM[20]
  //IF EEPROM MEM[25] IS INITIALIZED SKIP WRITING TO REGISTER 25
  timeOffset = currentTime - OSTime;

  //if timeOffset is less than 100, return and do not write to memory
  if(abs(timeOffset) < 50)
  {
    timeOffset = oldTimeOffset;
    return;
  }
  
  flipInt = timeOffset / 100;
  
  if(flipInt < 0)
  {
    EEPROM.write(20, 1);
    flipInt = flipInt * -1;
  }
  else
  {
    EEPROM.write(20, 0);
  }
  
  EEPROM.write(21, flipInt);
  
 
  if(EEPROM.read(25) != 0)
  {
    EEPROM.write(25, 0);
  }

  timeOffset = conCat(flipInt,0);

  EEPROM.commit();
  
}



//used to get the current weights of the food bowl, not used for fast feedback such as in filling up the bowls
void getWeights()
{
  scaleFood.power_up();
  scaleWater.power_up();

  //get the average of 3 units for the food offset
  currFood = scaleFood.get_units(3) + foodOffset; 
  currWater = scaleWater.get_units(3) + waterOffset; 

  Serial.print("Current Food Weight: ");
  Serial.println(currFood, 3);
  Serial.print("Current Water Weight: ");
  Serial.println(currWater, 3);
  
  scaleFood.power_down();
  scaleWater.power_down();
}
