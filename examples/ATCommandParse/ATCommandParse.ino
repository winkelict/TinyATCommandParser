#include "TinyATCommandParser.h"

//ATTENTION: This example requires at least 1719 bytes of SRAM (For example Atmega328p: Arduino, Arduino Pro Mini etc.)

//contains a combination of actual GSM (SIM800) AT and Bluetooth (FSC-BT836B) AT responses
const char testresponse[] PROGMEM =
"\r\n"
"+CMGL: 1,\"REC READ\",\"+31012345678\",\"\",\"22/01/02,20:14:54+08\"\r\n"
"Martin Luther King Jr.\r\n"
"\r\n"
"+CMGL: 2,\"REC READ\",\"+31032145678\",\"\",\"22/01/02,20:16:11+08\"\r\n"
"Line1\nLine2\n Line3 \n\r\n"
"\r\n"
"+CMGL: 3,\"REC READ\",\"+31032145678\",\"\",\"22/01/02,20:17:57+08\"\r\n"
"I have a dream that one day on the red hills of Georgia,  \nthe sons of former slaves and the sons of former slave owners will be able to sit down together at the table of brotherhood. \r\n"
"\r\n"
"+CMGR: \"REC READ\",\"+31012543678\",\"\",\"22/01/02,20:17:57+10\"\r\n"
"I have a dream that one day even the state of Mississippi,\na state sweltering with the heat of injustice, \n, sweltering with the heat of oppression will be transformed into an oasis of freedom and justice. \r\n"
"\r\n"
"OK\r\n"
"\r\n"
"+CMGR: \"REC READ\",\"+31012345678\",\"\",\"22/01/02,20:21:02+08\"\r\n"
"I have a dream that my four little children will one day live in a nation where they will not be judged by the color of their skin but by the content of their character.\r\n"
"\r\n"
"OK\r\n"
"\r\n"
"+SCAN={\r\n"
"\r\n"
"+SCAN=1,2,C8AXX32XEA12,-64,14,PHONE 1\r\n"
"\r\n"
"+SCAN=2A,2,AXCXX9A0XF54,-58,13,LAPTOP 22A\r\n"
"\r\n"
"+SCAN=3,2,AXCXX9A0XF54,-58,13,LAPTOP 22B\r\n"
"\r\n"
"+SCAN=}\r\n"
"\r\n"
"+NAME=FSC-BT836B\r\n"
"\r\n"
"OK\r\n"
"\r\n"
"+VER=8.0.2,FSC-BT836B\r\n";


char response[sizeof(testresponse)];

void setup() {

  //TODO: pleas eadjust your serial monitor to this baudrate or adjust the baudrate here
  Serial.begin(4800);
  while (!Serial);

  //after each call to ATParse, response will be reset to testresponse as the ATParse function mangles the response text while parsing
  strcpyprog(response, testresponse);

  Serial.println(F("Sample AT response to parse: "));
  Serial.println(F("-----------------------------"));
  Serial.println(response);
  Serial.println(F("-----------------------------"));
  char* value;
  bool bvalue;


  /************************* ONE FILTER OR VALUE ************************/

  //get the date/time (parameter position 4) of the first (true) text message in the text message list (command: CMGL)
  ATParse.getResponseValue(response, "CMGL", 4, &value, true);
  printResultandResetTest(value);

  //same as above, but match the value instead of returning it (first or last value does not matter anymore here)
  bvalue = ATParse.hasResponseValue(response, "CMGL", 4, "22/01/02,20:14:54+08");
  printResultandResetTestB(bvalue);

  //get the date/time (parameter position 4) of the LAST (false) text message in the text message list (command: CMGL)
  //result:22/01/02,20:17:57+08
  ATParse.getResponseValue(response, "CMGL", 4, &value, false);
  printResultandResetTest(value);

  //get the first (-1) line of the text message body of the same text message
  ATParse.getResponseValue(response, "CMGL", -1, &value, false);
  printResultandResetTest(value);

  //get the second (-2) line of the text message body of the same text message
  ATParse.getResponseValue(response, "CMGL", -2, &value, false);
  printResultandResetTest(value);
  //of the 2nd text message (parameter 0=2) (CGML command)

  //same as above, but match the (text body) value instead of returning it (first or last value does not matter anymore here)
  //attention: take note that the value matches although there is an extra space in the response line, the values are matched after being trimmed, see: #define AT_RESPONSE_TRIM_VALUE true
  bvalue = ATParse.hasResponseValue(response, "CMGL", -2, "the sons of former slaves and the sons of former slave owners will be able to sit down together at the table of brotherhood.");
  printResultandResetTestB(bvalue);

  
  /************************* ADDITIONAL FILTER  ************************/

  //get the third (-3) line of the text message body of message nr 2 (message nr = at position 0)
  ATParse.getResponseValueNoListFilter(response, "CMGL", -2, &value, false, 0, "2", true);
  printResultandResetTest(value);  

  //the same but compare it instead of returning it (notice that the comparison is after trimming the reponse value)
  bvalue = ATParse.hasResponseValueNoListFilter(response, "CMGL", -2, "Line2", 0, "2", true);
  printResultandResetTestB(bvalue);


  /************************* ADDITIONAL FILTER (filter based on a list) ************************/

  //please adjust this value in the .h value according to your needs, default is based on maximum international phone nr length
  char filterlist[][AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH] = {
    "+31032145678"
    , "dummy"
    , "+31012543678"
  };

  //return the date (parameter at position 3) of the last (false) command (CMGR) matching any of the phone numbers (parameter position 1)
  //in the list (case senstive, no trimming), see #define AT_RESPONSE_CASESENSITIVE_OPTFILTERLIST true / #define AT_RESPONSE_TRIM_OPTFILTERLIST false 
  //LEN2D = macro specified in the .h file
  ATParse.getResponseValue(response, "CMGR", 3, &value, false, 1, filterlist, LEN2D(filterlist));
  printResultandResetTest(value);

  //the same can be done by comparing it analog to previous examples
  bvalue = ATParse.hasResponseValue(response, "CMGR", 3, "22/01/02,20:17:57+10", 1, filterlist, LEN2D(filterlist));
  printResultandResetTestB(bvalue);  

  //get the last (false) device name (pos5) with a MAC address (pos2) in this list (command SCAN / = instead of : delimiter / tried automatically)
  char filterlist2[][AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH] = {
    "000009A0XF54"
    , "AXCXX9A0XF54"
    , "dummy"
  };

  ATParse.getResponseValue(response, "SCAN", 5, &value, false, 2, filterlist2, LEN2D(filterlist2));
  printResultandResetTest(value);


  /************************* ADDITIONAL FILTER + ADDITIONAL FILTER BASED ON LIST ************************/

  //same as above but get the one with sequence nr 2 (pos 0), matching case sensitive
  ATParse.getResponseValue(response, "SCAN", 5, &value, false, 2, filterlist2, LEN2D(filterlist2), 0, "2A", true);
  printResultandResetTest(value);

  //same but compare
  bvalue = ATParse.hasResponseValue(response, "SCAN", 5, "LAPTOP 22A", 2, filterlist2, LEN2D(filterlist2), 0, "2A", true);
  printResultandResetTestB(bvalue);   

  //same but no match because its a case sensitive match
  bvalue = ATParse.hasResponseValue(response, "SCAN", 5, "LAPTOP 22A", 2, filterlist2, LEN2D(filterlist2), 0, "2a", true);
  printResultandResetTestB(bvalue);                                                                          //^lower case!
}

void loop() {
  // put your main code here, to run repeatedly:
}


/*************** Do not mind these functions, these are just helper functions to enable the tests *************************/

int currenttestnr=1;

//does to things: prints result, make a copy of the test text for the next tet
void printResultandResetTest(char* result) {
  Serial.print("Result nr ");
  Serial.print(currenttestnr);
  Serial.print(" : ");
  Serial.println(result);
  //copy the test response back as it has been 'messed up' by the previous call to ATParse
  strcpyprog(response, testresponse);
  currenttestnr++;
}

void printResultandResetTestB(bool result) {
  if (result)
    printResultandResetTest("TRUE");
  else
    printResultandResetTest("FALSE");
}

//copy from prog mem to ram
void strcpyprog(char* dest, const char* src)
{
  char c;
  int i=0;
  if (!src) {
    return;
    dest[0]=0x00;
  }
  while ((c = pgm_read_byte(src++))) {
    dest[i++] = c;
  }
}
