// Declare function because of default param provided
/* ======================================================================
Function: writeLogFile
Purpose : Print to Serial && Write LittleFS json LOG
Input   : message, newLine = for Serial.print (with or without NEW LINE), 
          output: default is 2 if not set. ( 1 only serial, 2 only LOG, 3 both )
Output  : true / false
Comments: date:hour, type (warning, info), message
TODO    : FIX time issue ( no timestamp before NTP ), START using TYPE clasification  */
bool writeLogFile( String message, int newLine, int output = 2 );