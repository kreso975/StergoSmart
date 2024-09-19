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


/* ============================================================================
Function: showDuration
Purpose : converts upTime / Time from device started till now (time_t upTime;)
          to readable days hours minutes seconds
Input   : time_t upTime;
Output  : String DD:HH:MM:SS
Comments: 
TODO    :
=============================================================================== */

String showDuration( uint32_t duration )
{
  String S = "";
  // prints the duration in days, hours, minutes and seconds
  if ( duration >= SECS_PER_DAY )
  {
     S += duration / SECS_PER_DAY;
     S += "d ";
     duration = duration % SECS_PER_DAY;
  }
  
  if ( duration >= SECS_PER_HOUR )
  {
     S += duration / SECS_PER_HOUR;
     S += "h ";
     duration = duration % SECS_PER_HOUR;
  }
  if ( duration >= SECS_PER_MIN )
  {
     S += duration / SECS_PER_MIN;
     S += "m ";
     duration = duration % SECS_PER_MIN;
  }
  
  S += duration;
  S += "s";

  return S;
}