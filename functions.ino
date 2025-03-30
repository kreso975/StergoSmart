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
double round2(double value) { return (int)(value * 100 + 0.5) / 100.0; }

/* ============================================================================
Function: showDuration
Purpose : Converts elapsed time (uptime) from when the device started to a 
          human-readable format showing days, hours, minutes, and seconds.
Input   : None (uses global variables time_t currentTime and startTime).
Output  : const char* - Formatted uptime as a C-style string (e.g., "1d 2h 3m 4s").
Comments: Uses a static buffer for efficient memory handling. Ensure the returned 
          pointer is not used concurrently from multiple threads.
TODO    : None */
const char* showDuration()
{
  static char uptime[64]; // Static buffer to hold the uptime string
  memset(uptime, 0, sizeof(uptime)); // Clear the buffer

  time_t currentTime = now();
  time_t elapsedTime = currentTime - startTime;

  int days = elapsedTime / 86400;
  elapsedTime %= 86400;
  int hours = elapsedTime / 3600;
  elapsedTime %= 3600;
  int minutes = elapsedTime / 60;
  int seconds = elapsedTime % 60;

  // Format the uptime into the static buffer
  snprintf(uptime, sizeof(uptime), "%dd %dh %dm %ds", days, hours, minutes, seconds);

  return uptime;
}

/* ============================================================================
Function: isDST
Purpose : Determines whether the current time falls within Daylight Saving Time (DST) 
          based on predefined rules for the European region (e.g., DST starts on the 
          last Sunday of March and ends on the last Sunday of October at 02:00).
Input   : None (uses `now()` function for the current time and adjusts it with the 
          global timeZoneOffset).
Output  : bool - Returns true if the current time is in DST, false otherwise.
Comments: The function calculates DST boundaries by checking the current month, day, 
          and hour and applying rules specific to the European DST schedule. Ensure 
          that the `now()` function is properly configured to provide the current time. 
          Adapt these rules if applying to regions with different DST schedules.
TODO    : None */
bool isDST()
{
  int months = month(now() + timeZoneOffset);
  int days = day(now() + timeZoneOffset);
  int hours = hour(now() + timeZoneOffset);

  // Example: DST starts on last Sunday of March and ends on last Sunday of October
  if ((months > 3 && months < 10) || (months == 3 && (days - weekday() + 7) > 24 && hours >= 2) || (months == 10 && (days - weekday() + 7) <= 24 && hours < 2))
    return true;

  return false;
}