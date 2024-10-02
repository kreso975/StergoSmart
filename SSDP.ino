/* ======================================================================
Function: setupSSDP
Purpose : Initialize SSDP (Simple Service Discovery Protocol) Service
Input   : 
Output  : 
Comments: -
====================================================================== */
void setupSSDP()
{
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName( String(deviceName) );
  SSDP.setDeviceType("urn:schemas-upnp-org:device:StergoSmart:1");  //In case: put after SSDP.begin
  SSDP.setSerialNumber(SERIAL_NUMBER);                              //This must be adjusted to chipID
  SSDP.setURL("index.html");
  SSDP.setModelName(MODEL_NAME);
  SSDP.setModelNumber(MODEL_NUMBER);                                // This must be SET in main config 
  SSDP.setModelURL( String(COMPANY_URL) + "/" + PRODUCT );          // Product is Model_name + Model_number
  SSDP.setManufacturer( "Stergo" );
  SSDP.setManufacturerURL( COMPANY_URL );
  SSDP.begin();
  writeLogFile( F("SSDP Started"), 1, 3 );
}

/* ======================================================================
Function: parseSSDP
Purpose : 
Input   : str, found, 
        : what :  deleteBeforeDelimiter = 1,
                  deleteBeforeDelimiterTo = 2,
                  selectToMarkerLast = 3,
                  selectToMarker = 4
Output  : String
Comments: - NEED TO DESCRIBE BETTER WHAT IS DOING TO INPUT
====================================================================== */
String parseSSDP( String str, String found, int what )
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

/* ======================================================================
Function: requestSSDP
Purpose : UDP packets send
Input   : what = 1 (M-SEARCH devisec upnp)
          what = 2 (Send string for communication with Stergo device)
Output  : M-SEARCH & 
Comments: -
====================================================================== */
void requestSSDP( int what )
{
  if ( what == 1 )
  {
    char ReplyBuffer[] = "M-SEARCH * HTTP/1.1\r\nHost:239.255.255.250:1900\r\nMan:\"ssdp:discover\"\r\nST:ssdp:all\r\nMX:1\r\n\r\n";
     
    ntpUDP.beginPacket( IPAddress(SSDPADRESS), SSDPPORT );
    ntpUDP.write( ReplyBuffer );
    ntpUDP.endPacket();
  }
}

/* ======================================================================
Function: handleSSDP
Purpose : UDP packets receive
Input   : 
Output  : 
TODO    : Add save to file , check for duplicate entry / JSON format
          fields: Device name(name from settings), modelName, modelNumber, IP
          Load existing list and check if servers responds - something simple
====================================================================== */
void handleSSDP()
{
  String input = "";
  String modelNumber = "";
  String modelName = "";

  char packetBuffer[512];

  int packetSize = ntpUDP.parsePacket();
  if ( packetSize )
  {
    int len = ntpUDP.read(packetBuffer, 512);
    if ( len > 0 )
      packetBuffer[len] = 0;
    
    input += packetBuffer;
    
    int i = input.indexOf("Arduino"); // From M-SEARCH
    int a = input.indexOf("Stergo");  // From D2D (device2device) communication
    int b = input.indexOf("TicTac");  // From D2D (device2device) communication

    if ( i > 0 || a > 0 || b > 0 )
    {
      if ( i > 0 )
      {
        modelNumber = parseSSDP(input, "Arduino", 1); // Should be changed
        modelNumber = parseSSDP(modelNumber, "\n", 4);
        //Serial.println(chipIDremote); // First Row

        modelName = parseSSDP(modelNumber, "Stergo", 2);
        modelName = parseSSDP(modelName, "/", 4);
        //Serial.println(ssdpName);
      
        modelNumber = parseSSDP(modelNumber, "/", 3);
        //Serial.println(chipIDremote);

        //writeLogFile( modelNumber + " - " + ntpUDP.remoteIP().toString(), 1 );
        
        isSSDPfoundBefore( ntpUDP.remoteIP() );
      
        
        for( int iS = 0; iS < NUMBER_OF_FOUND_SSDP; iS++)
        {
          if ( foundSSDPdevices[iS] != IPAddress(0,0,0,0) )
          {
            writeLogFile( F("Found Device: ") + foundSSDPdevices[iS].toString(), 1, 1 );
          }
        }
        
        writeLogFile( F("Actual Devices: ") + String(actualSSDPdevices), 1, 1 );
      }
      else if ( a > 0 )
      {
        modelNumber = parseSSDP(input, "Stergo", 1);
        modelNumber = parseSSDP(modelNumber, "\n", 4);
        //Serial.println(chipIDremote); // First Row

        modelName = parseSSDP(modelNumber, "Stergo", 2);
        modelName = parseSSDP(modelName, "/", 4);
        //Serial.println(ssdpName);
      
        modelNumber = parseSSDP(modelNumber, "/", 3);
        //Serial.println(chipIDremote);
        
        String replyPacket = "Hi there! " + modelNumber + ", I'm " + String(_devicename);
        sendUDP(replyPacket, ntpUDP.remoteIP());

      }
      #if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 2 )
      else if ( b > 0 )  // Play TicTacToe
      {
        playTicTacToe( input );
      }
      #endif
    }
  }
}


void isSSDPfoundBefore( IPAddress ssdpDeviceName )
{ 
  for ( int x = 0; x < NUMBER_OF_FOUND_SSDP; x++ )
  {
    if ( foundSSDPdevices[x] != IPAddress(0,0,0,0) )
    {
      if ( foundSSDPdevices[x] == ssdpDeviceName )
        return;                                     // Device SSDP Already in my list
    }
    else
    {
      actualSSDPdevices = x+1;
      foundSSDPdevices[x] = ssdpDeviceName;         // ADD Device to my list
      
      return;
    }
  }
}


/* ==========================================================================================
Function: sendUDP
Purpose : send UDP
Input   : payloadUDP - Prepared message to be sent
Output  : no output.
Comments: 
TODO    : Diffrently handle the IP and Port
============================================================================================= */
void sendUDP( String payloadUDP, IPAddress ssdpDeviceIP, int udpPort )
{
  #if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 2 )                            //===============================================
  // Instead of remoteIP i will need to specify IP, Port will always be the same
  if ( didIaskedToPlay )
    ntpUDP.beginPacket( ssdpDeviceIP, udpPort );
  else
    ntpUDP.beginPacket( ntpUDP.remoteIP(), ntpUDP.remotePort() );
  #endif                                                                        //===============================================

  #if ( STERGO_PROGRAM == 1 || STERGO_PROGRAM == 3 )                            //===============================================
  ntpUDP.beginPacket( ntpUDP.remoteIP(), ntpUDP.remotePort() );
  #endif                                                                        //===============================================
  
  
  ntpUDP.write( payloadUDP.c_str() );
  ntpUDP.endPacket();
  return;

  // Lets comunicate with one of nodes :)
  // Sey hello to node
  /*
  IPAddress ssdpAdress(192,168,1,131);
  ntpUDP.remoteIP() 
  ntpUDP.remotePort()
  String replyBuffer = "TICTACTOE: " + payloadUDP;
  ntpUDP.beginPacket( ssdpAdress, LOCAL_UDP_PORT );
  ntpUDP.write( replyBuffer.c_str() );
  ntpUDP.endPacket();
  */
}