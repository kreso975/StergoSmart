#include "Config.h"
#ifdef MODULE_TICTACTOE

#include "TicTacToe.h"

/*
 *            TicTacToe layout:
 *              0 | 1 | 2
 *              ---+---+---
 *              3 | 4 | 5
 *              ---+---+---
 *              6 | 7 | 8
 */

// test Params
byte HUBproxy = 0;
byte HUBproxy_play = 0;					// 0 = No, 1 = Yes - Some Devices does not have HUBProxy functionality and they might start 1 On 1 game 
IPAddress HUBproxy_ip(192,168,1,34);
int HUBproxy_port = 4210;

// This should be done as sepparate TicTacToe program, not to extend this code
// TICTACTOE_GAMESERVER_CLIENT
//byte TicTacToe_GameServer = 0;
//IPAddress TicTacToe_GameServer_ip(192,168,1,118);
//int TicTacToe_GameServer_port = 4210;

// Config.ino
byte tictac_start, tictac_interval, tictac_webhook, tictac_discord;

// Difficulty defines number of Depth for AI , bigger Depth stronger AI
byte difficulty = 8;
int board[BOARD_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

int receivedMove = -1; // Move received from opponent
int sentMove;			  // Our move sent to opponent

byte gameStarted = 0;
byte player = 0; // Select Who Plays first 1=X, 2=O
byte turn = 0;	  // Value of current Turn in game

// We will use this to auto reset Game because there is no play and it hangs
const long ticTacLastPlayedInterval = 1000 * 10; // 30 sec              // 1000 * 60 * 2 - 120 sec
unsigned long ticTacLastPlayed = ticTacLastPlayedInterval;

// Interval timer for sending Invitations
byte maxRetryInviteEmptyIP = 0;
bool ticCallFirstRun = true;
unsigned long ticTacCallInterval = 1000 * 60 * 1; // 1000 * 60 * 60 - 60min
unsigned long ticCallLMInterval;

char playerName[64];
char opponentName[28] = "";
IPAddress opponentIP;
int opponentUDPport;

// didIaskedToPlay = true or false set when sending invite
bool didIaskedToPlay = false;
//byte nrInvitationTry = 0;

const char serverPrefix[] PROGMEM = "SERVER: TicTac ";

/* ===============================================================================
Function: sendPacket
Purpose : Constructs and sends a formatted UDP packet to the specified IP address and port.
Input   : action - The action to be included in the packet (const char*)
          value  - The value associated with the action (int)
          ip     - The IP address to send the packet to (IPAddress)
          port   - The port to send the packet to (int)
Output  : None
Comments: The function uses snprintf to format the packet with the server prefix, device name, action, and value.
          Possible actions include "WePlay", "Player", and "Move".
TODO    : None */
void sendPacket(const char *action, int value, IPAddress ip, int port)
{
	char replyPacket[100];
	// Determine the message format based on the HUBproxy state
	if ( HUBproxy && HUBproxy_play )
	{
		snprintf(replyPacket, sizeof(replyPacket), "%s;%d;%s%s %s=%d", ip.toString().c_str(), port, serverPrefix, _devicename, action, value);
		#if (DEBUG == 1)
    	writeLogFile(replyPacket, 1, 1);
		#endif
		sendUDP(replyPacket, HUBproxy_ip, HUBproxy_port); // Send to HUB proxy
	}
	else
	{
		snprintf(replyPacket, sizeof(replyPacket), "%s%s %s=%d", serverPrefix, _devicename, action, value);
		sendUDP(replyPacket, ip, port); // Send directly to the target
	}
}

/* ===============================================================================
Function: updateTicTacToe
Purpose : Main TicTacToe Constructor ( listen and initiate game) Part of Main Loop
Input   :
Output  :
Comments:
TODO    : */
void updateTicTacToe()
{
	// We Have config setting for manualy start/stop Tic Tac Toe
	if (tictac_start == 1)
	{
		// this should be removed out from TicTacToe, it belongs to UDP and SSDP
		updateSSDP();

		// Time interval to ask net devices to play tic tac toe
		if ((millis() - ticCallLMInterval > ticTacCallInterval) || ticCallFirstRun)
		{
			ticCallLMInterval = millis();
			ticCallFirstRun = false;

			#if (DEBUG == 1)
			writeLogFile(F("Inside TicTac Invite"), 1, 1);
			#endif

			// Init Game
			inviteDeviceTicTacToe();
		}

		// Time interval to check on inactivity of the game, auto reset
		if ((millis() - ticTacLastPlayed > ticTacLastPlayedInterval) && (gameStarted == 1 && turn > 0 || didIaskedToPlay))
		{
			#if (DEBUG == 1) //---------------------------------------
			if ( gameStarted == 1 && turn > 0 )
				writeLogFile(F("Inside LastPlayedTicTac measure because gameStarted == 1 and turn > 0"), 1, 1);
			else if ( didIaskedToPlay )
				writeLogFile(F("Inside LastPlayedTicTac measure because didIaskedToPlay == true"), 1, 1);
			else
				writeLogFile(F("Inside LastPlayedTicTac measure"), 1, 1);
			#endif //---------------------------------------

			resetTicTacToe();
		}
	}
}

// Setup Start variable values
void startGameValues(const char* playerName)
{
	// Here we will start a game
   // RND am I playing first or second, and then send proper message
   gameStarted = 1;               				// game is in play
	randomSeed(millis());							// Added additional random seed because ESP8266 D1 mini v4 always starts with same random number
   difficulty = random(3, BOARD_SIZE);     	// AI difficulty

   #if (DEBUG == 1)
   char logMessage[50];
   snprintf(logMessage, sizeof(logMessage), PSTR("Difficulty set to: %d"), difficulty);
   writeLogFile(logMessage, 1, 1);
   #endif

   strncpy(opponentName, playerName, 28);  	// Add player name
}

// Before starting new game | After End of game
void resetTicTacToe()
{
	// Reset the board and all relevant variables
	memset(board, 0, sizeof(board)); // Reset the board in one line

	turn = player = 0;
	byte difficulty = 8;
	playerName[0] = '\0';
	memset(opponentName, 0, sizeof(opponentName));
	
	gameStarted = HUBproxy_play = HUBproxy = 0;
	opponentIP = IPAddress(0, 0, 0, 0);
	opponentUDPport = 0;
	didIaskedToPlay = false;
}

GamePhases getGamePhase(const char* input)
{
	// Maybe Shorter
	if (strstr(input, "WePlay") != NULL)
		 return WePlay;
	if (strstr(input, "Player") != NULL)
		 return Player;
	if (strstr(input, "Move") != NULL)
		 return Move;
	return Invalid;
}

/* ======================================================================
Function: win
Purpose : Check if there is a winner
Input   :
Output  : 0 or 1 or -1
Comments:
TODO    : */
int win(const int board[BOARD_SIZE])
{
	if ( turn > 0 )
	{
		// list of possible winning positions
		const byte wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
		byte winPos;
		for ( winPos = 0; winPos < 8; ++winPos )
		{
			if (board[wins[winPos][0]] != 0 && board[wins[winPos][0]] == board[wins[winPos][1]] && board[wins[winPos][0]] == board[wins[winPos][2]])
				return board[wins[winPos][2]];
		}
	}

	return 0;
}

/* ======================================================================
Function: minimax
Purpose : Implements the minimax algorithm to determine the best move for the AI.
Input   : board (int[BOARD_SIZE]) - Current state of the game board.
          player (int) - Current player (1 for AI, -1 for opponent).
          depth (byte) - Current depth of the search tree.
Output  : int - The score of the best move found.
Comments: This function recursively explores possible moves, evaluates their scores, and returns the best score.
TODO    : NOT USING alpha / beta because AI is too good and hard to make him loose - this way if "difficulty" is low there is a chance to loose */
int minimax(int board[BOARD_SIZE], int player, byte depth)
{
	// check if there is a winner
	int winner = win(board);
	if ( winner != 0 )
		return winner * player;

	int move = -1;
	int score = -2;
	byte i;
	for ( i = 0; i < BOARD_SIZE; ++i )
	{
		if ( board[i] == 0 )
		{
			board[i] = player;
			int thisScore = 0;
			if (depth < difficulty)
				thisScore = -minimax(board, player * -1, depth + 1);

			if (thisScore > score)
			{
				score = thisScore;
				move = i;
			}
			// choose the worst move for opponent
			board[i] = 0;
		}
	}
	
	if (move == -1)
		return 0;

	return score;
}

/* ======================================================================
Function: computerMove
Purpose : Determines and makes the best move for the AI on the game board.
Input   : board (int[9]) - Current state of the game board.
Output  : void
Comments: This function uses the minimax algorithm to find the best move and updates the board.
TODO    : N/A */
void computerMove(int board[BOARD_SIZE])
{
	int move = -1;
	if (turn > 0)
	{
		int score = -2;
		byte i;
		for ( i = 0; i < BOARD_SIZE; ++i )
		{
			if ( board[i] == 0 )
			{
				board[i] = 1;
				int tempScore = -minimax(board, -1, 0);

				#if ( DEBUG == 1 )
				char logMessage[50];
				snprintf(logMessage, sizeof(logMessage), PSTR("Temp Score: %d"), tempScore);
				writeLogFile(logMessage, 1, 1);
				#endif

				board[i] = 0;
				if ( tempScore > score )
				{
					score = tempScore;
					move = i;
				}
			}
		}
	}
	else
	{
		// First move is going to be RANDOM otherwhise it's always draw
		// 0 is included, up 9, 9 not included
		move = random(0, BOARD_SIZE);
	}

	// returns a score based on minimax tree at a given node.
	// write move/position of AI move
	board[move] = 1;
	sentMove = move; // Prepare to send My Move to opponent
	ticTacLastPlayed = millis();
	++turn;
}

/* ======================================================================
Function: playerMove
Purpose : Opponent Player || Remote Player
Input   :
Output  : Play move
Comments:
TODO    : */
bool playerMove(int board[BOARD_SIZE], byte moveToPlay)
{
	int move = 0;

	// Serial.println("Input move ([0..8]): ");

	// if number is less than or equal to 8, the move is valid. 'moveToPlay' cannot be negative ( < 0 ) because it is a 'byte' type not 'int'.
	// If the user types '0', keypadConversion[0] = 255 which is also an invalid move (So the error message will appear)
	if ( moveToPlay <= 8 )
		move = moveToPlay;
	else
		move = 255; // Set 'move' to 255 which will be picked up later as an invalid move

	#if (DEBUG == 1)
	char logMessage[50];
	snprintf(logMessage, sizeof(logMessage), PSTR("Opponent played move: %d"), move);
	writeLogFile(logMessage, 1, 1);
	#endif
		
	// Here we check validity of selected Player move
	// This should be modiffied Proper respond when we includ Real player moves not AI
	if ((move > 8 || move < 0) || board[move] != 0)
	{
		#if (DEBUG == 1)
		char logMessage[20];
		snprintf(logMessage, sizeof(logMessage), PSTR("Invalid move")); // Say if the player's move was invalid
		writeLogFile(logMessage, 1, 1);
		#endif
		
		return false;
	}
	else
	{
		// -1 are Other Players moves
		board[move] = -1;

		++turn;
		ticTacLastPlayed = millis();
		return true;
	}
	return false;
}

/* ======================================================================
Function: letsPlay
Purpose : Main logic for playing TicTacToe
Input   : TurnPhase what (INIT_GAME or PLAY_MOVE)
          const char* who (Player name)
Output  : Executes the next step in game play
Comments:
TODO    : Streamlined structure for readability
===================================================================== */
void letsPlay(TurnPhase what, const char *who)
{
	char replyPacket[100];

	switch ( what )
	{
		case INIT_GAME:
			// Initialize game values
			startGameValues(who);

			switch ( static_cast<PlayerRole>(player) )
			{
				case Player1:
						sendPacket("Move", -1, opponentIP, opponentUDPport);
						break;
			
				case Player2:
						computerMove(board);
						#if (DEBUG == 1)
						printLogInPhase("LetsPlay_w1p2");
						draw(board);
						#endif
						sendPacket("Move", sentMove, opponentIP, opponentUDPport);
						break;
			
				default:
						#if (DEBUG == 1)
						writeLogFile(F("Invalid player role"), 1, 1);
						#endif
						break;
			}
			break;

		case PLAY_MOVE:
			#if (DEBUG == 1)
			printLogInPhase("LetsPlay_w2");
			#endif

			// Not All moves are played and still no winner
			if ( turn < BOARD_SIZE && win(board) == 0 )
			{
				if ( (turn + player) % 2 == 0 )
				{
					computerMove(board);
					#if (DEBUG == 1)
					draw(board);
					#endif
					sendPacket("Move", sentMove, opponentIP, opponentUDPport);
				}
				else if ( playerMove(board, receivedMove) )
				{
					#if (DEBUG == 1)
					draw(board);
					#endif
					
					// Not All moves are played and still no winner
					if ( turn < BOARD_SIZE && win(board) == 0 )
					{
						computerMove(board);
						#if (DEBUG == 1)
						draw(board);
						#endif
						sendPacket("Move", sentMove, opponentIP, opponentUDPport);
					}
				}
				else
				{
					sendPacket("Move", -1, opponentIP, opponentUDPport);
				}
			}

			// End of game logic = Check after every move
			checkGameOver();
			break;

		default:
			#if (DEBUG == 1)
			writeLogFile(F("Invalid game phase"), 1, 1);
			#endif
			break;
	}
}

void checkGameOver()
{
	if ( ( turn < BOARD_SIZE && win(board) != 0 ) || turn == BOARD_SIZE )
	{
		 switch ( win(board) )
		 {
			  case DRAW:
					#if (DEBUG == 1)
					writeLogFile(F("It's a draw."), 1, 1);
					#endif
					break;

			  case LOSS:
					#if (DEBUG == 1)
					draw(board);
					writeLogFile(F("You win!"), 1, 1);
					#endif
					break;

			  case WIN:
					#if (DEBUG == 1)
					draw(board); // Optional: display the board for a loss
					writeLogFile(F("You lose. I won!"), 1, 1);
					#endif
					#ifdef MODULE_DISPLAY
					if ( displayON )
					{
						 renderWIN = true;
						 messageWinON = true;
						 messageON = true;
						 server.stop();
						 prevMilMesLife = millis();
					}
					#endif
					sendTicTacWebhook(1); // Webhook only for WIN
					break;
		 }

		 #if (DEBUG == 1)
		 writeLogFile(F("Resetting game; let's start over"), 1, 1);
		 #endif
		 resetTicTacToe();
	}
}

/* ======================================================================
Function: manageTicTacToeGame
Purpose : initial Tic Tac Toe Manager
Input   : String input - UDP ASCII message received
Output  :
TODO    :  */
void manageTicTacToeGame(const char* input)
{
	char replyPacket[100];
	char logMessage[100];
	byte wantToPlay, selectPlayer;

	// this should not be entered each time
	checkIfHUBProxyPlay(input);

	strncpy(playerName, parseAndExtract(input, "", " ", 2), sizeof(playerName) - 1);  	// Parsing Input from UDP, space as delimiter
	playerName[sizeof(playerName) - 1] = '\0'; // Ensure null-termination

	int gamePhase = getGamePhase(input);
	switch ( gamePhase )
	{
		case WePlay:
			wantToPlay = atoi(parseAndExtract(input, "WePlay=", " "));
			if ( !wantToPlay ) // Do not want to WePlay = 0
			{
				// Remote don't want to play
				#if (DEBUG == 1)
				snprintf(logMessage, sizeof(logMessage), PSTR("%s%s do not want to play"), REPLAYER, playerName);
				writeLogFile(logMessage, 1, 1);
				printLogInPhase("WePlay");
				#endif

				resetTicTacToe();
				break;
			}
			else if ( wantToPlay && gameStarted )
			{
				// We cannot play because we are already playing
				// Here we can check if we are playing with the same player already
				// But i think simultanious games with the same Player are not OK
				#if (DEBUG == 1)
				snprintf(logMessage, sizeof(logMessage), PSTR("%s%s ask to play but we are already playing"), REPLAYER, playerName);
				writeLogFile(logMessage, 1, 1);
				printLogInPhase("WePlay");
				#endif

				// This must be fixed because if received over proxy IP is wrong
				sendPacket("WePlay", 0, ntpUDP.remoteIP(), ntpUDP.remotePort());

				#if (DEBUG == 1)
				snprintf(logMessage, sizeof(logMessage), "Reseting game");
				writeLogFile(logMessage, 1, 1);
				#endif

				resetTicTacToe();
				break;
			}
			else if ( wantToPlay && !gameStarted )
			{
				// Init Start Game Values
				startGameValues(playerName);

				// Check if I'm the one asking
				// That Way We know how to respond
				if ( didIaskedToPlay )
				{
					// We will randomly choose To ask Opponent to choose Player 1 || 2 , or we will Choose 1 || 2
					// 0 = Ask, 1 = Play 1st, 2 = Play 2nd
					randNumber = random(0, 3);

					#if (DEBUG == 1)
					writeLogFile("In sending Player decision: " + String(randNumber), 1, 1);
					#endif

					//If randNumber is 1, player will be set to 2.If randNumber is 2, player will be set to 1.
					//If randNumber is any other value (like 0), player retains its current value and is not modified.
					player = (randNumber == 1) ? 2 : (randNumber == 2) ? 1 : player;

					// Here I'm Back with answer and I need to ask oponent is he playin 1st or second
					sendPacket("Player", randNumber, opponentIP, opponentUDPport);
					break;
				}
				else
				{
					// I didnt sent Play Invitation so i just reply Yes I want to play
					sendPacket("WePlay", 1, opponentIP, opponentUDPport);
					break;
				}
			}
			break;

		case Player:
			// First lets check if we can talk
			if ( !gameStarted )
			{
				// I didnt sent Play Invitation so i just reply Yes I want to play
				#if (DEBUG == 1)
				snprintf(logMessage, sizeof(logMessage), PSTR("Somebody Skipped directly to Player"));
				writeLogFile(logMessage, 1, 1);
				#endif

				sendPacket("WePlay", 0, ntpUDP.remoteIP(), ntpUDP.remotePort());
				break;
			}

			selectPlayer = atoi(parseAndExtract(input, "Player=", " "));
			if ( selectPlayer == 0 ) // Not selected
			{
				// We need to make a decision Player 1 || 2
				// We Send What Player are We, and Assign his
				randNumber = random(1, 3);
				player = (randNumber == 1) ? 2 : 1;

				sendPacket("Player", randNumber, opponentIP, opponentUDPport);
			}
			else if ( (selectPlayer == 1 || selectPlayer == 2) && !gameStarted )
			{
				#if (DEBUG == 1)
				writeLogFile(selectPlayer + " Selected to play as Player: " + String(selectPlayer), 1, 1);
				printLogInPhase("Player");
				#endif

				player = selectPlayer;
				letsPlay(INIT_GAME, playerName);
			}
			else if (selectPlayer == 1 && gameStarted && turn == 0)
			{
				// Opponent decided to play As player 1, lets register player as 1 and inform him he plays first, send him to play (-1)
				player = selectPlayer;

				#if (DEBUG == 1)
				printLogInPhase("Player");
				#endif

				sendPacket("Move", -1, opponentIP, opponentUDPport);
			}
			else if (selectPlayer == 2 && gameStarted && turn == 0)
			{
				// Opponent decided to play As player 2, lets register player as 2 and start play
				player = selectPlayer;

				#if (DEBUG == 1)
				printLogInPhase("Player");
				#endif

				letsPlay(PLAY_MOVE, playerName);
				//sendPacket("Move", -1, opponentIP, 4210);
			}
			break;

		case Move:
			// Getting Move response from Remote Player
			receivedMove = atoi(parseAndExtract(input, "Move=", " "));
			#if (DEBUG == 1)
			writeLogFile("Game is in Turn: " + String(turn), 1, 1);
			#endif
			// MyTurn to play but no Turn Played
			if ( receivedMove == -1 && turn == 0 )
				letsPlay(INIT_GAME, playerName);

			if (receivedMove >= 0 && gameStarted )
			{
				// Check If we can play with this player
				// compare if playerName was the one when Game initiated
				if ( strcmp( opponentName, playerName ) == 0 )
				{
					// We have our oponent lets continue
					#if (DEBUG == 1) 								// ----------------------------------------
					snprintf(logMessage, sizeof(logMessage), "%s%s", REPLAYER, playerName);
					writeLogFile(logMessage, 0, 1);
					snprintf(logMessage, sizeof(logMessage), PSTR(" played move: %d"), receivedMove);
					writeLogFile(logMessage, 1, 1);
					#endif 											// ----------------------------------------

					letsPlay(PLAY_MOVE, playerName);
				}
				else
				{
					// Somebody else sent us move
					#if (DEBUG == 1) 								// ----------------------------------------
					snprintf(logMessage, sizeof(logMessage), PSTR("Opponent Name: %s - PlayerName: %s"), opponentName, playerName);
					writeLogFile(logMessage, 1, 1);
					std::string message = std::string("We are not playing With ") + REPLAYER + playerName;
					snprintf(logMessage, sizeof(logMessage), "%s", message.c_str());
					writeLogFile(logMessage, 1, 1);
					#endif 											// ----------------------------------------
				}
			}
			break;
			
		default:
			// Do nothing
			break;
	}
}

/* ========================================================================================
Function: inviteDeviceTicTacToe
Purpose : invite available device to play Tic Tac Toe with you (UDP)
Input   :
Output  :
TODO    : In here We Can have settup for Playing UDP, UDP Hub proxy, Game Server or MQTT */
void inviteDeviceTicTacToe()
{
	// Let's check if we are already playing
	// Not Playing
	if ( !gameStarted )
	{
		char replyPacket[100];
		// We are randomly picking 3 times one of available addresses
		randNumber = random(0, actualSSDPdevices);

		// Marked that we sent invitation | We are locked until we set this to false
		didIaskedToPlay = true;

		// We also do the hack to auto unlock us using Stale game
		ticTacLastPlayed = millis();

		IPAddress rndAddr = foundSSDPdevices[randNumber].ip;
		if ( rndAddr )
		{
			#if (DEBUG == 1)
			writeLogFile("Sending Invitation to IP: " + rndAddr.toString(), 1, 1);
			#endif
			HUBproxy_play = 1;
			sendPacket("WePlay", 1, rndAddr, 4210);
		}
		else
		{
			#if (DEBUG == 1)
			writeLogFile(F("Empty IPlist for Invite"), 1, 1);
			#endif

			// Allow enter again in sending
			if (maxRetryInviteEmptyIP < 50)
				ticCallFirstRun = true;

			maxRetryInviteEmptyIP++;
		}
	}
}

/* ======================================================================
Function: sendTicTacWebhook
Purpose : Send Webhook to Discord or WebServer
Input   : where = 1 / Send to Discord
			 where = 2 / Send to WebServer
Output  :
TODO    :  We should build 2 different calls 1. Discord, 2 Discord Webhook Proxy
			  Without Secure Client it's not possible to send to Discord Directly */
void sendTicTacWebhook(byte where)
{
	const char *localURL = discord_url;
	char data[256]; // Allocate a buffer for the data

	// Check if we can publish to Webhook
	if (where == 1 && (tictac_webhook == 1 || tictac_discord == 1))
	{
		const char *discordUsername = _devicename;
		const char *discordAvatar = discord_avatar;
		char message[100];

		// Create the message
		snprintf(message, sizeof(message), PSTR("I Won! Loser: %s lost."), opponentName);
		//writeLogFile(message, 1, 1);
		// Create the JSON data
		snprintf(data, sizeof(data), PSTR("{\"username\":\"%s\",\"avatar_url\":\"%s\",\"content\":\"%s\"}"), discordUsername, discordAvatar, message);
		
		// Send the webhook
		
		#if ( STERGO_PROGRAM_BOARD > 1 ) // Only boards with more then 1MB
		// Check if url is for Discord or for proxy
		if (strstr(discord_url, "discordapp.com") != NULL) // The substring "discord.com" is present in discord_url
			sendWebhook(localURL, data, true);
	  	else
	  		sendWebhook(localURL, data, false);
		#else
		// Send to Discord Webhook Proxy
		sendWebhook(localURL, data, false);
		#endif
	}
}

// This is used to detect if opponent wants to play not over ProxyHub but directly
void checkIfHUBProxyPlay(const char *message)
{
    // Locate the first semicolon in the message
    const char* firstSemicolon = strchr(message, ';');
    const char* secondSemicolon = firstSemicolon ? strchr(firstSemicolon + 1, ';') : NULL;

    // Check if the message contains at least two semicolons
    // This determines whether it has the ProxyHub prefix
    HUBproxy_play = (firstSemicolon && secondSemicolon) ? 1 : 0;

    if (HUBproxy_play)
    {
        // Extract opponent's IP address from the message
        char opponentIPStr[16] = {0}; // Initialize a temporary array for the IP string
        strncpy(opponentIPStr, message, firstSemicolon - message); // Copy IP substring
        opponentIP.fromString(opponentIPStr); // Convert to IP object

        // Extract opponent's UDP port from the message
        char opponentUDPportStr[6] = {0}; // Initialize a temporary array for the port string
        strncpy(opponentUDPportStr, firstSemicolon + 1, secondSemicolon - firstSemicolon - 1); // Copy port substring
        opponentUDPport = atoi(opponentUDPportStr); // Convert to integer

        return; // Exit the function since ProxyHub data is processed
    }

    // Default behavior: retrieve remote IP and port if no ProxyHub prefix
    opponentIP = ntpUDP.remoteIP();        // Fetch and save remote IP
    opponentUDPport = ntpUDP.remotePort(); // Fetch and save remote port
}

#if (DEBUG == 1)
// Just Draw Grid
void drawNumberedBoard()
{
	writeLogFile("Grid number layout:",1,1);
	writeLogFile(" 0 | 1 | 2 ",1,1);
	writeLogFile("---+---+---",1,1);
	writeLogFile(" 3 | 4 | 5 ",1,1);
	writeLogFile("---+---+---",1,1);
	writeLogFile(" 6 | 7 | 8 ",1,1);
}

// Display in Serial played moves after every move
void draw(int board[BOARD_SIZE])
{
   byte vn; // Variable to iterate through the board
   String bm = F("\n "); // Start building the board output

   for (vn = 0; vn < BOARD_SIZE; ++vn)
   {
      // Append the correct character ('X', 'O', or ' ') to the output
      char cellChar = (board[vn] == -1) ? 'X' : (board[vn] == 0) ? ' ' : (board[vn] == 1) ? 'O' : ' ';
      bm += cellChar;

      // Add separators and row dividers based on position
      if ((vn + 1) % 3 == 0 && vn != BOARD_SIZE - 1) // After every row except the last one
         bm += F("\n---+---+---\n ");
      else if (vn == BOARD_SIZE - 1) // Last cell
         bm += F(" ");
      else // Column separators
         bm += F(" | ");
   }

   // Write the formatted board to the log file
   writeLogFile(bm, 1, 1);
}

// to Optimize code.
void printLogInPhase(String where)
{
	writeLogFile("in " + where + ", gameStarted: " + String(gameStarted), 1, 1);
	writeLogFile("in " + where + ", player: " + String(player), 1, 1);
	writeLogFile("in " + where + ", turn: " + String(turn), 1, 1);
}
#endif

#endif // MODULE_TICTACTOE