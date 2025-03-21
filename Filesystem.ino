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
	return LittleFS.begin();
}

/* ============================================================================
Function: getContentType
Purpose : Determines the MIME type of a file based on its extension.
Input   : String filename - The name of the file whose MIME type is to be determined.
Output  : String - The corresponding MIME type as a string.
Comments: - Supports common file extensions like .html, .css, .js, etc.
          - Returns "text/plain" as a default for unsupported file extensions.
          - Special handling for .txt files, which are treated as "text/html". */
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

/* ============================================================================
Function: handleFileRead
Purpose : Sends the requested file to the client if it exists in the filesystem.
Input   : String path - The requested file path.
Output  : bool - Returns true if the file exists and is successfully sent to
                 the client, otherwise returns false.
Comments: - Automatically appends "index.html" to paths ending with "/".
          - Prioritizes sending compressed (.gz) versions of files if available.
          - Streams the file to the client using the MIME type determined by getContentType.
          - Closes the file after streaming to free up resources. */
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

/* ============================================================================
Function: handleFileUpload
Purpose : Handles the upload of a new file to the LittleFS filesystem. Processes
          file creation, writing, and finalization, with logging and JSON replies.
Input   : None (relies on the `server.upload()` object for upload data).
Output  : None
Comments: - Handles three stages of an HTTP file upload:
            1. UPLOAD_FILE_START: Prepares the file for writing.
            2. UPLOAD_FILE_WRITE: Writes chunks of uploaded data to the file.
            3. UPLOAD_FILE_END: Finalizes the upload, closes the file, and logs
               the result.
          - Generates appropriate log messages and JSON responses based on the
            upload status (success or error).
          - Requires LittleFS and HTTP server libraries to be included.
TODO    : Add error handling for edge cases, such as filesystem errors or large files. */
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
			sendJSONheaderReply(1, F("Success file upload"));
		}
		else
		{
			writeLogFile(F("Error file upload: ") + filename, 1);
			sendJSONheaderReply(0, F("Error file upload"));
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
	File file = LittleFS.open(LOG_FILE, "w");
	if (!file) return false;

	if (z == 1) file.print("{\"log\":[]}");
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
	#if (DEBUG == 1)
	if ((output == 1 || output == 3) && logOutput == 0)
		newLine == 0 ? Serial.print(message) : Serial.println(message);
	#endif

	// Write to Log only if Filesystem is UP
	if (!setupFS())
		return false;

	if ( output == 2 || output == 3 )
	{
		DynamicJsonDocument jsonBuffer(6000);
		File file = LittleFS.open(LOG_FILE, "r");

		if (!file || file.size() < 10)
		{
			writeLogFile(fOpen + LOG_FILE, 1);
			if (saveLogFile(1))
				writeLogFile(nLOG, 1);
		}
		else
		{
			std::unique_ptr<char[]> buf(new char[file.size()]);
			file.readBytes(buf.get(), file.size());
			file.close();

			if (deserializeJson(jsonBuffer, buf.get()))
			{
				writeLogFile(faParse + String(LOG_FILE), 1);
				return false;
			}

			JsonArray logData = jsonBuffer["log"];
			JsonObject logEntry = logData.createNestedObject();

			logEntry["id"] = timeClient.getEpochTime(); // Timestamp
			logEntry["id2"] = 1;								  // Message Type
			logEntry["id3"] = message;						  // Message

			if (logData.size() > sizeLog)
				logData.remove(0); // Remove oldest entry

			File writeFile = LittleFS.open(LOG_FILE, "w");
			if (!writeFile || serializeJson(jsonBuffer, writeFile) == 0)
			{
				writeLogFile(fWrite + LOG_FILE, 1);
				writeFile.close();
				return false;
			}
			writeFile.close();
		}
	}
	return true;
}
