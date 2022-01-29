Winkel ICT Tiny AT Command Parser
=================================

0 RAM usage Hayes AT Command parser (GSM/Bluetooth) by using only pointers for parsing char arrays, fast, thoroughly tested.

When having to parse many GSM and/or Bluetooth commands on a low (flash/ram) memory budget this library can help out by keeping the in memory char array's to an absolute minimum.

## Quick start

This library uses the input string for parsing and makes no copies, it uses pointers to char arrays instead of a String object to minimize RAM usage.
The only catch of this approach is that the input string gets **mangled** so after parsing the input char array/string cannot be used anymore.
Generally this is not a problem when parsing AT commands.

