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
#include "TinyATCommandParser.h"

TinyATCommandParser ATParse;

unsigned int TinyATCommandParser::getResponseValue(char* response, char* atcommand, int pos, char** returnvalue, bool firstMatch, int optFilterListPos, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH], byte optFilterListLen, int optFilterPos, char* optFilter, bool optFilterCaseSensitive) {
	return parse(response, atcommand, pos, returnvalue, firstMatch, NULL		, optFilterListPos, optFilterList, optFilterListLen			, optFilterPos, optFilter, optFilterCaseSensitive);
}

bool TinyATCommandParser::hasResponseValue(char* response, char* atcommand, int pos, char* value					, int optFilterListPos, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH], byte optFilterListLen, int optFilterPos, char* optFilter, bool optFilterCaseSensitive) {
	return parse(response, atcommand, pos, NULL , NULL, value					, optFilterListPos, optFilterList, optFilterListLen			, optFilterPos, optFilter, optFilterCaseSensitive);
}


unsigned int TinyATCommandParser::getResponseValueNoListFilter(char* response, char* atcommand, int pos, char** returnvalue, bool firstMatch		, int optFilterPos, char* optFilter, bool optFilterCaseSensitive) {
	return parse(response, atcommand, pos, returnvalue, firstMatch, NULL		, INT_MIN, NULL, INT_MIN									, optFilterPos, optFilter, optFilterCaseSensitive);
}

bool TinyATCommandParser::hasResponseValueNoListFilter(char* response, char* atcommand, int pos, char* value								, int optFilterPos, char* optFilter, bool optFilterCaseSensitive) {
	return parse(response, atcommand, pos, NULL , NULL, value					, INT_MIN, NULL, INT_MIN									, optFilterPos, optFilter, optFilterCaseSensitive);
}


//Case sensitive, Opt filter is optionally case insensitive
//if position does not exist: false, if command doesnt exist false, etc.
//pos=-1: get body line 0, -2 is get body line 1 etc.
//return 0 if not found, otherwise the next position in data with which this function can be called again (data+return = new pointer) without reparsing what was already parst (can use first=true, and get all values for example)
//TODO: possible optimization: give split a list of pos and filters and let it match them all (filterlist would be hard then)
// char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH]
unsigned int TinyATCommandParser::parse(char* response, char* atcommand, int pos, char** retvalue, bool firstMatch, char* filter, int optFilterListPos, char (*optFilterList)[AT_RESPONSE_MAX_FILTERLIST_STRINGLENGTH], byte optFilterListLen, int optFilterPos, char* optFilter, bool optFilterCaseSensitive)
//replytocommand     //param1 (smscommand param, or just value(signal quality)    //callerid or mac list (response is uniq)  //smscommand (list is response)
{
	//so i dont have to comment it out in the library
#ifdef TINYATCOMMANDPARSE_DEBUG_ASSERTIONS
	//in order not to use memory, the same position cannot be parsed twice as its altered (\0 is inserted at end)
	//in combination with reverse parsing values no copies of strings have to be used
	//none of the 3 positions can be equal when not INT_MIN
	assertTrue((((pos != optFilterPos && pos != optFilterListPos) || pos==INT_MIN ) && ((optFilterPos != optFilterListPos) || optFilterPos ==INT_MIN)),0);

	if (filter==NULL) {
		assertTrue(retvalue != NULL /*looking for value, when no filter*/,1);
	} else {
		assertTrue(retvalue == NULL /*not looking for value, no return value when filter or pos==INT_MIN*/,2);
		assertTrue(firstMatch == NULL ,3);
	}

	if (optFilterListPos > INT_MIN) {
		assertTrue(optFilterList != NULL /*when optFilterListPos>INT_MIN*/,4);
	}

	if (optFilterPos > INT_MIN) {
		assertTrue(optFilter != NULL /*when optFilterPos>INT_MIN*/,5);
	}
#endif

	if (filter==NULL) {
		*retvalue = NULL;
	}

	unsigned int nextdatapos = 0;
	char *saveptr = NULL;
	bool funcres = false;
	//this stores result returned by strtok, so its different then results returned by split
	char* responseline = NULL;
	int i = 0;

	//go through all responslines, break if empty or not started with an atcommand
	//response line looks something like this:
	//\r\n+CMGL: 1,\"REC UNREAD\",\"+85291234


	//copy needs to be here or pointer (or stack version) is deleted when exiting the next scope {}
	//char* copy;
#ifdef AT_RESPONSE_COPY
#ifndef AT_RESPONSE_ALLOWCOPY
	//implement some min heap declaration size for this when using runtime to reduce fragmentation
#error FOR TESTING PURPOSES ONLY/HIGH MEM USAGE Serial.println(F("ATTENTION: copying response data, this is only meant for testing same response multiple times in a row, not for runtime usage / use return value to continue parsing the same string instead of restarting, no re-parsing supported (UNTESTED)"));
#endif
	int len = strlen(response);
	char copy[len+1];
	//partly copied from split, as returning all matches from split in a 2D array uses to much memory, and we only need 1 once its found

	//copy = new char[len+1](); //on heap, not for runtime usage (need defragmentation functionality) otherwise cant dynamically create it
	//make a copy, strtok mangles it
	strcpy(copy, response);
	responseline = strtok_r_strdelim(copy, AT_RESPONSE_DELIM, &saveptr);
#else
	responseline = strtok_r_strdelim(response, AT_RESPONSE_DELIM, &saveptr);
#endif

	while ( responseline != NULL )
	{
		nextdatapos = nextdatapos + strlen(responseline) + LEN2D(AT_RESPONSE_DELIM)-1;

		if (i > 0)
			responseline = strtok_r_strdelim(NULL, AT_RESPONSE_DELIM, &saveptr);

		i++; //one-up response line nr

		//always will be a CRLF if there is any command result, so this is k
		if (!strlen(responseline)) //nothing seperated by CRLF meands no command result, also no CRU result etc.
		{
			//delete[] responseline;
			continue; //TODO: set match and return value
		}

		//first line can contain \r\n as the response is split on \r\n\r\n
		if (i == 1)
			responseline = ltrim(responseline);

		//response line can be anything still, get the part after the +
		char* responselinenoatprefix = NULL;


		//split gives: 1: always empty, 0 .. the command or whatever
		if (responseline[0] ==  AT_RESPONSE_COMMANDSTART_DELIM) {
			//found a command, get pointer to past the + symbol=where command starts
			responselinenoatprefix = responseline + 1;
			int noprefixlen = strlen(responselinenoatprefix);

			char* reponseatcommand = NULL;
			reponseatcommand = split(responselinenoatprefix, AT_RESPONSE_COMMANDEND_DELIM1, 0);

			char* delim = NULL;
			bool commandfound = false;

			//no space can be behind the at command before the delim!
			if (strlen(reponseatcommand) != noprefixlen && (strcmp(reponseatcommand, atcommand) == 0 || (strcmp(AT_RESPONSE_ANYCOMMAND, atcommand) == 0) )) {
				delim = AT_RESPONSE_COMMANDEND_DELIM1;
				commandfound = true;
			} else {
				//try the other seperator
				//TODO: can use same char array?
				reponseatcommand = split(responselinenoatprefix, AT_RESPONSE_COMMANDEND_DELIM2, 0);
				//no space can be behind the at command before the delim!
				if (strlen(reponseatcommand) != noprefixlen && (strcmp(reponseatcommand, atcommand) == 0 || (strcmp(AT_RESPONSE_ANYCOMMAND, atcommand) == 0) )) {
					delim = AT_RESPONSE_COMMANDEND_DELIM2;
					commandfound = true;
				}
			}

			//not allowed to delete stuff from stack delete[] reponseatcommand; //not needed anymore
			if (!commandfound)
				continue;

			//move pointer to the right
			char* atresult = responselinenoatprefix + strlen(reponseatcommand) + strlen(delim);

			//Serial.print(atresult);
			//RETURN VALUE                      : bool f(char* response, char* atcommand, int pos, char** retvalue, bool firstMatch) {
			//RETURN IFMATCHFILTER (FIRST/LAST) : bool f(char* response, char* atcommand, int pos,                 bool firstMatch, char* filter) {
			//OPTIONAL, IF MATCHES ANY IN LIST AT POS                                                                                             int optFilterListPos, char* optFilterList,
			//OPTIONAL, IF MATCHES THIS STRING (FIRST OR LAST MATCH)                                                                              int optFilterPos, char* optFilter)

			//first do the optional filters
			bool optFilterListRes = false;
			bool optFilterRes = false;

			int orderedposar[AT_RESPONSE_NROF_POSARGS] = {optFilterPos,optFilterListPos,pos};
			byte ordertooriginal[AT_RESPONSE_NROF_POSARGS] = {0,1,2}; //TODO: if more then 3 pos args (AT_RESPONSE_NROF_POSARGS), this array needs to be created dynamically

			char* values[AT_RESPONSE_NROF_POSARGS];
			char* atbody = NULL;
			char* atparam = NULL;

			//TODO: if more then 3 pos args (AT_RESPONSE_NROF_POSARGS), this function needs to be changed to support any nr of elements
			sortar3(orderedposar,ordertooriginal);

			int zeroix = -1;
			int ix;
			for (byte l=0;l<AT_RESPONSE_NROF_POSARGS;l++) {
				if (orderedposar[l] >=0) {
					if (zeroix==-1)
						zeroix=l;

					//as soon as the current position parsed is positive or 0, start counting backwards to parse backwards to avoid \0 to end the string being parsed
					//this is not necessary for negative pos as -1*negative = positive counting backwards.
					ix = (AT_RESPONSE_NROF_POSARGS-1) - l + zeroix;
				} else ix = l;
				int curpos = orderedposar[ix];

				//order: body, param and within negative to positive pos. body is negative so that is implied in the sort order!
				if (curpos!=INT_MIN) {
					//parse body just once, as it removes the body from atresult (using \0)
					if (curpos < 0) {
						if (atbody == NULL) {
							atbody = split(atresult, AT_RESPONSE_BODYPARAM_DELIM, 1); //body (after CRLF)

							//if on body, quit, wont result in true
							if (!strlen(atbody))
								continue;
						}

						//because they are ordered negative ascending, when multiplied by -1 they are ordered descending
						//because this index does start at 1, 1 has to be deducted
						values[ordertooriginal[ix]] = split(atbody, AT_RESPONSE_BODY_DELIM, (curpos * -1) - 1); //make -1 -> 1 .. and -1 .. to 0
					}

					if (curpos >=0) {
						if (atparam == NULL) {
							atparam = split(atresult, AT_RESPONSE_BODYPARAM_DELIM, 0); //parameter (before CRLF)

							if (!strlen(atparam))
								continue;
						}
						values[ordertooriginal[ix]] = (char*)split(atparam, AT_RESPONSE_PARAM_DELIM, curpos, AT_RESPONSE_PARAM_QUOTE);
					}
				}
			}

			//OPTIONAL 1: optFilterPos, if avail, break if no match
			if (optFilterPos > INT_MIN) {
				optFilterRes = false;

				//if pos found
				if (strlen(values[0])) {
					//TODO: also trims, make trim a function parameter?
					char* compvalue = cleanValue(values[0], AT_RESPONSE_TRIM_OPTFILTER);

					if (
							(optFilterCaseSensitive && !strcmp(optFilter, compvalue))
							||
							(!optFilterCaseSensitive && !strcicmp(optFilter, compvalue))
					)
					{
						optFilterRes = true;
					} else {
						continue;//pos not found
					}
				} else {
					//optimization: break as all filter have to match for line to qualify, go right to the next line
					continue;//pos not found
				}
			} else optFilterRes = true;

			//OPTIONAL 2: optFilterListPos, if avail, break if no match
			if (optFilterListPos > INT_MIN) {
				optFilterListRes = false;

				//if pos found
				if (strlen(values[1])) {
					char* compvalue = cleanValue(values[1], AT_RESPONSE_TRIM_OPTFILTERLIST);

					for (byte j = 0; j < optFilterListLen; j++) {
						char * optFilterListValue = optFilterList[j];

						if (
#ifdef AT_RESPONSE_CASESENSITIVE_OPTFILTERLIST
								!strcmp(optFilterListValue, compvalue)
#else
								!strcicmp(optFilterListValue, compvalue)
#endif
								/*(optFilterListCaseSensitive &&*//* !strcmp(optFilterListValue, compvalue))*/
								/*||
                (!optFilterListCaseSensitive && !strcicmp(optFilter,compvalue))*/
						)
						{
							optFilterListRes = true;
							//one is enough, break out of for loop
							break;
						}
					}
					if (!optFilterListRes) {
						continue; //optimization: qgo to the next iteration of while loop right away, no match
					}
				} else {
					//optimization: break as all filter have to match for line to qualify, go right to the next line
					continue;//pos not found
				}
			} else optFilterListRes = true;

			if (optFilterRes && optFilterListRes) { //if both true, value memory is still declared/not deleted
				//MANDATORY get pos, all previous filters matched (otherwise: pos not found or value not matching) or were not supplied (pos'ses would be -1)
				//if pos found
				if (strlen(values[2])) {
					bool isMatch = false;
					//if this and previous filter matching or not present (will be true then)

					char* compvalue = cleanValue(values[2], AT_RESPONSE_TRIM_VALUE);

					if (!filter) {
						//this value is matching all filters, store it, and if no other opt filter match, this will contain the last opt filter match value  / or just the last value if no opt filters
						//value will dissapear after popped from stack, so need to copy the value

						*retvalue = compvalue;

						isMatch = true;
						funcres = true;//set this to true (no filter match, but 2x opt filter match)
					} else {
						if (
#ifdef AT_RESPONSE_CASESENSITIVE_VALUE
								!strcmp(filter, compvalue)
#else
								!strcicmp(filter, compvalue)
#endif
						) {
							isMatch = true;

							funcres = true; //all three filters match, dont have to return value
						} else continue; //optimization
					}
					//break immediately if all matching and first match (otherwise it will continue trough all responses
					//firstMach/lastmatch matters only if no filter=return a value / if there is a filter its any match (otherwise better supply Nth match, but cannot now this in advance before parsing)
					if (isMatch && (firstMatch || filter)) {
						break;
					}

					//else the result (funcres/resvalue) wil be overwritten if there is another match of all filters
				}
			} //end parsing main filter after both opt filters are true or both INT_MIN (unsupplied)
		} //end parsing of + command
	} //no at response, the string is parsed so that also the body (message) of the AT command is in a single to-be-parsed line

	//when arrived here, we have either
	//firstmatch (breaks right away at first complete matching response line
	//or lastmatch (keeps looping, storing funcres= true at first match/does not matter, and overwriting resvalue at each next match if filter not set
	if (funcres)
		return nextdatapos;
	else
		return 0;
}

char* TinyATCommandParser::split(char* copy, char* delim, int index, int quote) {
	char *token = NULL;
	char *saveptr = NULL;

	int i = 0;

	//https://linux.die.net/man/3/strtok_r
	token = strtok_r_strdelim(copy, delim, &saveptr, quote, (i==index));
	while ( token != NULL )
	{
		//Serial.println(token);
		if (i == index) break;
		i++; //one-up token nr
		token = strtok_r_strdelim(NULL, delim, &saveptr, quote,(i==index));
	}

	//TODO: TEST
	//added or i != index, so the complete string is not returned when index is not 0
	if (token == NULL || i != index) token = "";
	//token is just a pointer in copy, copy will be freed, so token will point to nothing, so copy result to/for out

	return token;
}


//made thread safe like strtok_r
//https://stackoverflow.com/questions/29788983/split-char-string-with-multi-character-delimiter-in-c
char* TinyATCommandParser::strtok_r_strdelim(char *input, char *delimiter, char **save_ptr, int quote, bool endStr) {
	//*save_ptrstatic char *string;

	if (input != NULL)
		*save_ptr = input; //save input to str

	if (*save_ptr == NULL) //
		return *save_ptr;

	char *end = strstr_quoted(*save_ptr, delimiter, quote); //returns pointer to first occurence of delimiter in str

	if (end == NULL) { //if no end found
		char *temp = *save_ptr; //then complete string is the result
		*save_ptr = NULL; //free memory??
		return temp;
	}

	char *temp = *save_ptr;

	if (endStr)
		*end = '\0';

	*save_ptr = end + strlen(delimiter); //move pointer up, for the next parse (might not happen)
	return temp;
}

char* TinyATCommandParser::strstr_quoted (const char *s1, const char *s2, int quote)
{
	const char *p = s1;
	const size_t len = strlen (s2);
	for (; (p = strchr_quoted (p, *s2, quote)) != 0; p++)
	{
		if (strncmp (p, s2, len) == 0)
			return (char *)p;
	}
	return (0);
}

char* TinyATCommandParser::strchr_quoted (register const char *s, int c, int quote)
{
	bool evenquotes = true;
	do {
		if ((*s == c) && evenquotes)
		{
			return (char*)s;
		}
		else if (*s == quote)
		{
			evenquotes = !evenquotes;
		}
	} while (*s++);
	return (0);
}


//trim, unescape VT to ',', remove quotes
char* TinyATCommandParser::cleanValue(char* value, bool trimvalue) {
	char* newvalue = value;
	if (trimvalue)
		newvalue = trim(value);

	if (newvalue[0] == '"') {
		newvalue = newvalue + 1; //move pointer one to right
		newvalue[strlen(newvalue) - 1] = '\0';
	}

	return newvalue;
}


//https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
int TinyATCommandParser::strcicmp(char const *a, char const *b)
{
	for (;; a++, b++) {
		int d = tolower((unsigned char) * a) - tolower((unsigned char) * b);
		if (d != 0 || !*a)
			return d;
	}
}

//https://stackoverflow.com/questions/656542/trim-a-string-in-c
char* TinyATCommandParser::ltrim(char *s)
{
	while (isspace(*s)) s++;
	return s;
}

char* TinyATCommandParser::rtrim(char *s)
{
	char* back = s + strlen(s);
	while (isspace(*--back));
	*(back + 1) = '\0';
	return s;
}

char* TinyATCommandParser::trim(char *s)
{
	return rtrim(ltrim(s));
}

void TinyATCommandParser::sortar3(int a[3], byte tracka[3]) {
	if (a[0] > a[1]) {
		swapar(a,tracka,0,1);
	}
	if (a[0] > a[2]) {
		swapar(a,tracka,0,2);
	}
	if (a[1] > a[2]) {
		swapar(a,tracka,1,2);
	}
}

void TinyATCommandParser::swapar(int a[], byte tracka[], byte pos1, byte pos2) {
	int temp = a[pos1];
	a[pos1] = a[pos2];
	a[pos2] = temp;

	int temp2 = tracka[pos1];
	tracka[pos1]=tracka[pos2];
	tracka[pos2]=temp2;
}

#ifdef TINYATCOMMANDPARSE_DEBUG_ASSERTIONS
void TinyATCommandParser::assertTrue(bool assertion, short code) {
	if (!assertion) {
#ifdef HardwareSerial_h
		Serial.print("Assertion failed, assertion nr (check source): ");
		Serial.println(code);
#endif
		// abort program execution.
		abort();
	}
}
#endif
