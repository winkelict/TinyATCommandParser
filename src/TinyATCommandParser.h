/*

Winkel ICT TinyATCommandParser

Extremely low ram usage Hayes AT Command parser by using only pointers for parsing char arrays.

Copyright (C) 2022 Klaas-Jan Winkel / Winkel ICT

All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */

#ifndef TINYATCOMMANDPARSER_H_
#define TINYATCOMMANDPARSER_H_

#include <Arduino.h>
#include <limits.h>

//this ensures that functins are called with the right parameter combinations, comment out to save flash memory
#define TINYATCOMMANDPARSE_DEBUG_ASSERTIONS

#define STRING(s) #s
//as strings take up a lot of space, using a wildcard can save ram memory
#define AT_RESPONSE_ANYCOMMAND STRING(*)

//For testing purposes only: will copy complete input text! Allows for parsing same text multiple times for testing.
//#define AT_RESPONSE_COPY
//#define AT_RESPONSE_ALLOWCOPY

//please adjust to your needs to minimize memory usage, current value based on max international phone nr and mac address length
#define AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH (16+1)

//initial delimiter to be tried
//example: +CMGL:
#define AT_RESPONSE_COMMANDEND_DELIM1 ": "

//alternate delimiter to be tried
//some devices return responses using = instead of :, for example, feasycom BT836B bluetooth module
//example: +SCAN={\r\n"
#define AT_RESPONSE_COMMANDEND_DELIM2 "="


#define AT_RESPONSE_DELIM "\r\n\r\n"
#define AT_RESPONSE_BODYPARAM_DELIM "\r\n"
#define AT_RESPONSE_COMMANDSTART_DELIM '+'
#define AT_RESPONSE_PARAM_DELIM ","
#define AT_RESPONSE_PARAM_QUOTE '"'
#define AT_RESPONSE_BODY_DELIM "\n"

//WARNING: code change needed before adjusting this! see TODO in .cpp file
#define AT_RESPONSE_NROF_POSARGS 3

#define AT_RESPONSE_TRIM_VALUE true
#define AT_RESPONSE_TRIM_OPTFILTER true
//if true can be used to get all commands (using the return pos, increase char pointer and looping)
#define AT_RESPONSE_TRIM_OPTFILTERLIST false
#define AT_RESPONSE_CASESENSITIVE_VALUE true
#define AT_RESPONSE_CASESENSITIVE_OPTFILTERLIST true

//credits: https://stackoverflow.com/questions/34134074/c-size-of-two-dimensional-array
#define LEN2D(arr) ((byte) (sizeof (arr) / sizeof (arr)[0]))

#define NO_FILTER NULL

class TinyATCommandParser {
public:

	/*
	 * defaults (changable using macro definitions in .h file)
	 *
	 * filter			: trim 		+ case sensitive				(intented usage: generic)
	 * opt filter		: trim 		+ case INsensitive (selectable) (intended usage: human written command's in text message)
	 * opt filter list  : NOtrim 	+ case sensitive  				(intended usage: mac address, phone nr, has to be exact match)
	 *
	 */

	//pos = INT_MIN: get all values, not the [pos]th position in the comma seperated list
	static unsigned int getResponseValue				(char* response, char* atcommand, int pos, char** returnvalue, bool firstMatch		, int optFilterListPos = INT_MIN, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH] = NULL, byte optFilterListLen = INT_MIN, int optFilterPos = INT_MIN, char* optFilter = NULL, bool optFilterCaseSensitive = false);
	static bool hasResponseValue				(char* response, char* atcommand, int pos, char* value							, int optFilterListPos = INT_MIN, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH] = NULL, byte optFilterListLen = INT_MIN, int optFilterPos = INT_MIN, char* optFilter = NULL, bool optFilterCaseSensitive = false);
	static unsigned int getResponseValueNoListFilter	(char* response, char* atcommand, int pos, char** returnvalue, bool firstMatch		, int optFilterPos = INT_MIN, char* optFilter = NULL, bool optFilterCaseSensitive = false);
	static bool hasResponseValueNoListFilter	(char* response, char* atcommand, int pos, char* value							, int optFilterPos = INT_MIN, char* optFilter = NULL, bool optFilterCaseSensitive = false);


	static char* split(char* copy, char* delim, int index, int quote = NULL);

	//made thread safe like strtok_r
	//https://stackoverflow.com/questions/29788983/split-char-string-with-multi-character-delimiter-in-c
	static char* strtok_r_strdelim(char *input, char *delimiter, char **save_ptr, int quote = NULL, bool endStr = true);
	static char* strstr_quoted (const char *s1, const char *s2, int quote);
	static char* strchr_quoted (register const char *s, int c, int quote);

	//trim, , remove quotes
	static char* cleanValue(char* value, bool trimvalue);

	//https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
	static int strcicmp(char const *a, char const *b);

	//https://stackoverflow.com/questions/656542/trim-a-string-in-c
	static char *ltrim(char *s);
	static char *rtrim(char *s);
	static char *trim(char *s);
	static void sortar3(int a[3], byte tracka[3]);
	static void swapar(int a[], byte tracka[], byte pos1, byte pos2);


protected:
	static unsigned int parse(char* response, char* atcommand, int pos, char** retvalue, bool firstMatch, char* filter, int optFilterListPos, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH], byte optFilterListLen, int optFilterPos, char* optFilter, bool optFilterCaseSensitive);

private:
#ifdef TINYATCOMMANDPARSE_DEBUG_ASSERTIONS
	static void assertTrue(bool assertion, short code);
#endif

};

extern TinyATCommandParser ATParse;

#endif /* TINYATCOMMANDPARSER_H_ */
