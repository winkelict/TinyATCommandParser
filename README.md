Winkel ICT Tiny AT Command Parser
=================================

0 RAM usage Hayes AT Command parser (GSM/Bluetooth) by using only pointers for parsing char arrays, fast, thoroughly tested.

When having to parse many GSM and/or Bluetooth commands on a low (flash/ram) memory budget this library can help out by keeping the in memory char array's to an absolute minimum.

## Quick start

This library uses the input string for parsing and makes no copies, it uses pointers to char arrays instead of a String object to minimize RAM usage.
The only catch of this approach is that the input string gets **mangled** so after parsing the input char array/string cannot be used anymore.
Generally this is not a problem when parsing AT commands. 

Note: it will only mangle the input char array up to, and including, the string found or (assume) completely when nothing is found.

You can find the exact same code of the following chapters in the example INO file: ATCommandParse

### Single parameter or text-body line

1. Get a parameter of a command (getResponseValue)
2. Check if the command has a certain parameter value (hasResponseValue), and can always be used instead of the get[] version.

![simple](https://user-images.githubusercontent.com/98483683/151663443-f6113f71-6c17-44b6-b0ee-9647e77bbf96.png)

*Each function call matches a color, found values and positions searched on are underlined with this color*

### Two parameters
1. Get a parameter of a command and use an extra search criteria on another parameter or text body line
2. Same but use a list of values as the extra search criteria.
3. if needed use the has[] version to check for existence of 2 values and return a boolean.

![addiotionallist](https://user-images.githubusercontent.com/98483683/151663446-c3969754-5871-4fe8-a9ce-3dafa34165df.png)

*Each function call matches 2 colors, found values and positions searched on are underlined with this color, the second color matches the second filter criteria applied and its found value*

### Three parameters
1. Use both a single search criteria and a list as search criteria.

![2listscrop](https://user-images.githubusercontent.com/98483683/151663448-9539860c-a2a7-4f3a-8762-80f4ede948e5.png)

*Each function call matches 3 colors = 3 filter criteria*

### Looping through multiple parameters
As the getResponseValue does only mangle response string (only when needed) upto the found string, the rest of the response string can still be parsed.
This way multiple of the same commands can be parsed in a row, for example all messages returned by the CMGL List SMS messages command:


	char* responsepos = response;
	int pos = 0;
	while (pos <= sizeof(response)) {
	  int pos = ATParse.getResponseValue(responsepos, "CMGL", 4, &value, true);
	  if (pos > 0) {
	    printResultandResetTest(value);
	    responsepos = responsepos + pos;
	  }
	}

### Customizing behavior of 1st, 2nd and 3rd parameter search.

 defaults (changeable using macro definitions in .h file)
 
* 1st parameter/filter: 	filter			 : trim 		+ case sensitive				(intended usage: generic)
* 2nd parameter/filter:  	opt filter		 : trim 		+ case INsensitive (selectable) (intended usage: human written command's in text message)
* 3rd parameter/filter: 	opt filter list  : NOtrim 	+ case sensitive  				(intended usage: mac address, phone nr, has to be exact match)

Only for the 2nd parameter case sensitivity/insensitivity can be selected at runtime, for the others default have to be set in the .h file:

* 1st parameter:


	#define AT_RESPONSE_TRIM_VALUE true
	#define AT_RESPONSE_CASESENSITIVE_VALUE true

* 2nd parameter (case sensitive runtime selectable as function argument):
	
	
	#define AT_RESPONSE_TRIM_OPTFILTER true

* 3d parameter:


	#define AT_RESPONSE_TRIM_OPTFILTERLIST false
	#define AT_RESPONSE_CASESENSITIVE_OPTFILTERLIST true 
	
### Debugging
By default function arguments are checked and any problems are printed if HardwareSerial is available.
This can be turned off by commenting out this macro definition:


	TINYATCOMMANDPARSE_DEBUG_ASSERTIONS
	
### Misc
This library can be used for more exotic purposes or non AT command parsing by adjusting the delimiters.
NOTE: chars ('') have to stay chars.

	#define AT_RESPONSE_COMMANDEND_DELIM1 ": "
	//alternate delimiter to be tried
	#define AT_RESPONSE_COMMANDEND_DELIM2 "="
	#define AT_RESPONSE_DELIM "\r\n\r\n"
	#define AT_RESPONSE_BODYPARAM_DELIM "\r\n"
	#define AT_RESPONSE_COMMANDSTART_DELIM '+'
	#define AT_RESPONSE_PARAM_DELIM ","
	#define AT_RESPONSE_PARAM_QUOTE '"'
	#define AT_RESPONSE_BODY_DELIM "\n"