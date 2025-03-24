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
