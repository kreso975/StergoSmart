/*
 * CODE NOT USED ANY MORE - PRESERVED AS INTERESTING SOLUTIONS
 * 
* Is this an IP? 
* 
* 
bool isIp( String str )
{
  for ( unsigned int i = 0; i < str.length(); i++ )
  {
    int c = str.charAt(i);
    if ( c != '.' && (c < '0' || c > '9') )
      return false;
  }
  return true;
}
*/

/** IP to String? 
  Not Used any more. Most common uses are
  WiFi.localIP().toString()
  WiFi.localIP().toString().c_str()
  WiFi.softAPIP().toString()
String toStringIp2( IPAddress ip )
{
  String res = "";
  for ( int i = 0; i < 3; i++ )
    res += String( ( ip >> (8 * i) ) & 0xFF ) + ".";
  
  res += String( ( ( ip >> 8 * 3 ) ) & 0xFF );
  return res;
}
*/

// Convert Float to double
// Using it in ArduinoJson Array for t,h,P0 add in History.json
double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}

/* ============================================================================
Function: showDuration
Purpose : converts upTime / Time from device started till now (time_t upTime;)
          to readable days hours minutes seconds
Input   : time_t upTime;
Output  : String DD:HH:MM:SS
Comments: 
TODO    :
=============================================================================== */
String showDuration()
{
  time_t currentTime = now();
  time_t elapsedTime = currentTime - startTime;
  
  int days = elapsedTime / 86400;
  elapsedTime %= 86400;
  int hours = elapsedTime / 3600;
  elapsedTime %= 3600;
  int minutes = elapsedTime / 60;
  int seconds = elapsedTime % 60;
  
  String uptime = "";
  uptime += String(days) + "d ";
  uptime += String(hours) + "h ";
  uptime += String(minutes) + "m ";
  uptime += String(seconds) + "s";
  
  return uptime;
}

/* ======================================================================
Function: parseString
Purpose : Process a given string based on a substring and an integer parameter
Input   : str   : The original string to be processed
          found : The substring to search for within str
          what  : Operation type:
                    1 - Delete Before Delimiter: Returns the part of str after the first occurrence of found
                    2 - Delete Before Delimiter To: Returns the part of str starting from the first occurrence of found
                    3 - Select To Marker Last: Returns the part of str after the last occurrence of found
                    4 - Select To Marker: Returns the part of str before the first occurrence of found
Output  : String - Processed substring based on the value of what
Comments: Depending on the value of what, the function performs different types of substring extraction. 
          If what doesn't match any specified case, it returns an empty string.
          
Example Usage:
  String str = "Hello, this is a test string.";
  String found = "test";

  // Case 1: Delete Before Delimiter
  String result1 = parseString(str, found, 1);  // " string."

  // Case 2: Delete Before Delimiter To
  String result2 = parseString(str, found, 2);  // "test string."

  // Case 3: Select To Marker Last
  String result3 = parseString(str, found, 3);  // " string."

  // Case 4: Select To Marker
  String result4 = parseString(str, found, 4);  // "Hello, this is a " */
String parseString( String str, String found, int what )
{
  if ( what == 2 || what == 4 )
  {
    int p = str.indexOf(found);
    if ( what == 2 )
      return str.substring(p);
    else
      return str.substring(0, p);
  }
  else if ( what == 1 )
  {
    int p = str.indexOf(found) + found.length();
    return str.substring(p);
  }
  else if ( what == 3 )
  {
    int p = str.lastIndexOf(found);
    return str.substring(p + found.length());
  }

  return "";
}