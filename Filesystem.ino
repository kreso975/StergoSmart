String GetMeFSinfo()
{
  String MyOut;
  struct FSInfo {
    size_t totalBytes;      // total size of useful data on the file system
    size_t usedBytes;       // number of bytes used by files
    size_t blockSize;       // filesystem block size
    size_t pageSize;        // filesystem logical page size
    size_t maxOpenFiles;    // max number of files which may be open simultaneously
    size_t maxPathLength;   // max file name length (including one byte for zero termination)
  };

  SPIFFS.info(fs_info);
  int Flashfree = ( fs_info.totalBytes - fs_info.usedBytes ) / 1000;
  MyOut = String( Flashfree );
  
  return MyOut;
}

/* ======================================================================
Function: setupFS
Purpose : Setup SPIFFS Filesystem
Input   : 
Output  : 
TODO    : Implement fail over if SPIFFS get corrupted - copy content 
          from web loc or Upload or
          https://github.com/spacehuhn/esp8266_deauther/blob/master/esp8266_deauther/webfiles.h
====================================================================== */
bool setupFS()
{
  if ( SPIFFS.begin() )
    return true;
  else
    return false;
}

// convert the file extension to the MIME type
String getContentType(String filename)
{ 
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".txt")) return "text/html";
  return "text/plain";
}

bool handleFileRead( String path )
{ // send the right file to the client (if it exists)
  
  //writeLogFile( "handleFileRead: " + path, 1 );
  if ( path.endsWith("/") )
    path += "index.html";                                 // If a folder is requested, send the index file
  
  String contentType = getContentType(path);              // Get the MIME type
  String pathWithGz = path + ".gz";
  
  if ( SPIFFS.exists( pathWithGz ) || SPIFFS.exists( path ) )
  {                                                        // If the file exists, either as a compressed archive, or normal
    if ( SPIFFS.exists( pathWithGz ) )                     // If there's a compressed version available
      path += ".gz";                                       // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    server.streamFile(file, contentType);                  // Send it to the client
    file.close();                                          // Close the file again
    //writeLogFile( "\tSent file: " + path, 1 );
    return true;
  }
  
  // If the file doesn't exist, return false   
  //writeLogFile( "\tFile Not Found: " + path, 1 );
  return false;
}

void handleFileUpload()
{ // upload a new file to the SPIFFS

  HTTPUpload& upload = server.upload();
  String filename = upload.filename;
  
  if( upload.status == UPLOAD_FILE_START )
  {
    if( !filename.startsWith("/") )
      filename = "/"+filename;
         
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  }
  else if(  upload.status == UPLOAD_FILE_WRITE )
  {
    if( fsUploadFile )
      fsUploadFile.write( upload.buf, upload.currentSize ); // Write the received bytes to the file
  }
  else if( upload.status == UPLOAD_FILE_END )
  {
    if( fsUploadFile )
    {                                                     // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      
      //writeLogFile( "handleFileUpload Size: " + upload.totalSize, 1 );
      writeLogFile( "Success file upload: " + filename, 1 );
      server.send(200, "application/json", "{\"success\":\"Success file upload\"}");
    }
    else
    {
      writeLogFile( "Error file upload: " + filename, 1 );
      server.send(200, "application/json", "{\"Error\":\"Error file upload\"}");
    }
  }
}

/* ======================================================================
Function: saveLogFile
Purpose : Save || Erase Log file
Input   : z = 0 (saveLog); z = 1 (eraseLog) 
Output  : true / false
Comments: -
====================================================================== */
bool saveLogFile( int z = 0 )
{ 
  String json;
  
  File file = SPIFFS.open( LOG_FILE, "w" );
  if ( !file )
    return false;

  if ( z == 1 )
    json = "{\"log\":[]}";

  file.print( json );
  file.close();

  return true;
}

 /* ======================================================================
Function: writeLogFile
Purpose : Print to Serial && Write SPIFF json LOG
Input   : message, newLine = for Serial.print (with or without NEW LINE)
Output  : true / false
Comments: date:hour, type (warning, info), message
TODO    : FIX time issue ( no timestamp before NTP ), START using TYPE clasification 
====================================================================== */
bool writeLogFile( String message, int newLine )
{
	/*
	if ( logOutput == 0 )
	{
		if ( newLine == 0 )
			Serial.print( message );
		else
			Serial.println( message );
	}
	*/
  
	DynamicJsonBuffer jsonBuffer(6000);

	File file = SPIFFS.open( LOG_FILE, "r" );
	if (!file)
	{
		writeLogFile( fOpen + LOG_FILE, 1 );
		if ( saveLogFile( 1 ) ) // Create proper initial Log File
			writeLogFile( nLOG, 1 );
	}
	else
	{
		size_t size = file.size();
		if ( size < 10 )
		{
			if ( saveLogFile( 1 ) ) // Create proper initial Log File
				writeLogFile( nLOG, 1 );
		}
		else
		{
			std::unique_ptr<char[]> buf (new char[size]);
			file.readBytes(buf.get(), size);
			file.close();
        
			JsonObject& rootLog = jsonBuffer.parseObject(buf.get());
       
			if ( !rootLog.success() )
			{
				writeLogFile( faParse + String(LOG_FILE), 1 );
				return false;
			}
			else
			{
				JsonArray& logData = rootLog["log"];
				JsonObject& LogWritedata = logData.createNestedObject();

				long int tps = timeClient.getEpochTime();
				//NEEDS TO BE EXTENDED IN ENTIRE CORE TO PROPERLY SETUP WORNING / INFO MESSAGE TYPE
				int type = 1;

				LogWritedata["id"] = tps;       // Timestamp
				LogWritedata["id2"] = type;     // Message Type
				LogWritedata["id3"] = message;  // Message
  
				if ( logData.size() > sizeLog )
					logData.remove(0); 			// - remove first record / oldest

				//Let's now write Fresh log input
				File file = SPIFFS.open( LOG_FILE, "w" );
				if (!file)
				{
					writeLogFile( fOpen + LOG_FILE, 1 );
					if ( saveLogFile( 1 ) ) // Create proper initial Log File
						writeLogFile( nLOG, 1 );
				}
				else
				{
					if ( rootLog.prettyPrintTo(file) == 0 )
					{
						writeLogFile( fWrite + LOG_FILE, 1 );
						//*message = fWrite + LOG_FILE;
						file.close();
						return false;
					}
				}
			}
		}
   
		file.close();
	}
  
	return true;
}
