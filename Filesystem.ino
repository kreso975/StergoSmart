String GetMeFSinfo()
{
	String MyOut;

	#if defined(ESP8266)
	struct FSInfo
	{
		size_t totalBytes;	 // total size of useful data on the file system
		size_t usedBytes;		 // number of bytes used by files
		size_t blockSize;		 // filesystem block size
		size_t pageSize;		 // filesystem logical page size
		size_t maxOpenFiles;	 // max number of files which may be open simultaneously
		size_t maxPathLength; // max file name length (including one byte for zero termination)
	};

	LittleFS.info(fs_info);
	int Flashfree = (fs_info.totalBytes - fs_info.usedBytes) / 1000;
	#elif defined(ESP32)
	size_t totalBytes = LittleFS.totalBytes();
	size_t usedBytes = LittleFS.usedBytes();
	int Flashfree = (totalBytes - usedBytes) / 1000;
	#endif

	MyOut = String(Flashfree);
	return MyOut;
}

/* ======================================================================
Function: setupFS
Purpose : Setup LittleFS Filesystem
Input   :
Output  :
TODO    : Implement fail over if LittleFS get corrupted - copy content
			 from web loc or Upload or
			 https://github.com/spacehuhn/esp8266_deauther/blob/master/esp8266_deauther/webfiles.h */
bool setupFS()
{
	if (LittleFS.begin())
		return true;
	else
		return false;
}

// convert the file extension to the MIME type
String getContentType(String filename)
{
	if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".gz"))
		return "application/x-gzip";
	else if (filename.endsWith(".json"))
		return "application/json";
	else if (filename.endsWith(".txt"))
		return "text/html";
	return "text/plain";
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)

	// writeLogFile( "handleFileRead: " + path, 1 );
	if (path.endsWith("/"))
		path += "index.html"; // If a folder is requested, send the index file

	String contentType = getContentType(path); // Get the MIME type
	String pathWithGz = path + ".gz";

	if (LittleFS.exists(pathWithGz) || LittleFS.exists(path))
	{													  // If the file exists, either as a compressed archive, or normal
		if (LittleFS.exists(pathWithGz))		  // If there's a compressed version available
			path += ".gz";							  // Use the compressed verion
		File file = LittleFS.open(path, "r"); // Open the file
		server.streamFile(file, contentType); // Send it to the client
		file.close();								  // Close the file again
		
		return true;
	}

	// If the file doesn't exist, return false
	// writeLogFile( "\tFile Not Found: " + path, 1 );
	return false;
}

void handleFileUpload()
{ // upload a new file to the LittleFS

	HTTPUpload &upload = server.upload();
	String filename = upload.filename;

	if (upload.status == UPLOAD_FILE_START)
	{
		if (!filename.startsWith("/"))
			filename = "/" + filename;

		fsUploadFile = LittleFS.open(filename, "w"); // Open the file for writing in LittleFS (create if it doesn't exist)
		filename = String();
	}
	else if (upload.status == UPLOAD_FILE_WRITE)
	{
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
	}
	else if (upload.status == UPLOAD_FILE_END)
	{
		if (fsUploadFile)
		{								 // If the file was successfully created
			fsUploadFile.close(); // Close the file again

			// writeLogFile( "handleFileUpload Size: " + upload.totalSize, 1 );
			writeLogFile(F("Success file upload: ") + filename, 1);
			server.send(200, "application/json", "{\"success\":\"Success file upload\"}");
		}
		else
		{
			writeLogFile(F("Error file upload: ") + filename, 1);
			server.send(200, "application/json", "{\"Error\":\"Error file upload\"}");
		}
	}
}

/* ======================================================================
Function: saveLogFile
Purpose : Save || Erase Log file
Input   : z = 0 (saveLog); z = 1 (eraseLog)
Output  : true / false
Comments: - */
bool saveLogFile(int z = 0)
{
	String json;

	File file = LittleFS.open(LOG_FILE, "w");
	if (!file)
		return false;

	if (z == 1)
		json = "{\"log\":[]}";

	file.print(json);
	file.close();

	return true;
}

/* ======================================================================
Function: writeLogFile
Purpose : Print to Serial && Write LittleFS json LOG
Input   : message, newLine = for Serial.print (with or without NEW LINE),
			output: default is 2 if not set. ( 1 only serial, 2 only LOG, 3 both )
Output  : true / false
Comments: date:hour, type (warning, info), message
TODO    : use const char* message, FIX time issue ( no timestamp before NTP ), START using TYPE clasification  */
//bool writeLogFile(const char* message, int newLine, int output)
bool writeLogFile(String message, int newLine, int output)
{
	// Write to Serial
	if (output == 1 || output == 3)
	{
		if (logOutput == 0)
		{
			#if (DEBUG == 1)
			if (newLine == 0)
				Serial.print(message);
			else
				Serial.println(message);
			#endif
		}
	}

	// Write to Log only if Filesystem is UP
	if (setupFS())
	{
		// Write to Log
		if (output == 2 || output == 3)
		{
			DynamicJsonDocument jsonBuffer(6000);

			File file = LittleFS.open(LOG_FILE, "r");
			if (!file)
			{
				writeLogFile(fOpen + LOG_FILE, 1);
				if (saveLogFile(1)) // Create proper initial Log File
					writeLogFile(nLOG, 1);
			}
			else
			{
				size_t size = file.size();
				if (size < 10)
				{
					if (saveLogFile(1)) // Create proper initial Log File
						writeLogFile(nLOG, 1);
				}
				else
				{
					std::unique_ptr<char[]> buf(new char[size]);
					file.readBytes(buf.get(), size);
					file.close();

					DeserializationError error = deserializeJson(jsonBuffer, buf.get());

					if (error)
					{
						writeLogFile(faParse + String(LOG_FILE), 1);
						return false;
					}
					else
					{
						JsonArray logData = jsonBuffer["log"];
						JsonObject LogWritedata = logData.createNestedObject();

						long int tps = timeClient.getEpochTime();
						// NEEDS TO BE EXTENDED IN ENTIRE CORE TO PROPERLY SETUP WORNING / INFO MESSAGE TYPE
						int type = 1;

						LogWritedata["id"] = tps;		 // Timestamp
						LogWritedata["id2"] = type;	 // Message Type
						LogWritedata["id3"] = message; // Message

						if (logData.size() > sizeLog)
							logData.remove(0); // - remove first record / oldest

						// Let's now write Fresh log input
						File file = LittleFS.open(LOG_FILE, "w");
						if (!file)
						{
							writeLogFile(fOpen + LOG_FILE, 1);
							if (saveLogFile(1)) // Create proper initial Log File
								writeLogFile(nLOG, 1);
						}
						else
						{
							// if ( rootLog.prettyPrintTo(file) == 0 )
							if (serializeJson(jsonBuffer, file) == 0)
							{
								writeLogFile(fWrite + LOG_FILE, 1);
								//*message = fWrite + LOG_FILE;
								file.close();
								return false;
							}
						}
					}
				}
				file.close();
			}
		}
	}

	return true;
}
