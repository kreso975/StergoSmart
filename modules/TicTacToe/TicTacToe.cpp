#ifdef MODULE_TICTACTOE

#include "TicTacToe.h"


/*
 *  IDEAS:  - 1st request includes: DO YOU WANT TO PLAY + CHOOSE PLAY 1st or 2nd
 *          
 *          - add flag : game in play | already playing ( this is till more simultanious games can be played )
 *          - starting new game Reset board.
 *          
 *          - far future: add GUI for playing against devices
 *          - GUI must be placed into WebServer so that codeFootprint on devices will not grow for such non important features
 *          - In Order to monitor games in Play, Webserver should now (be notified) that game is startig at: device - then need to think if to send to server all moves
 *            or in future all games must be played via/over WebServer (code related for devices - uneccecery growth )
 * 
 * 
 *            TicTacToe layout:
 *              0 | 1 | 2  
 *              ---+---+---
 *              3 | 4 | 5 
 *              ---+---+--- 
 *              6 | 7 | 8 
*/

// Config.ino
byte tictac_start, tictac_interval, tictac_webhook, tictac_discord;

// Difficulty defines number of Depth for AI , bigger Depth stronger AI
byte difficulty = 8;
int board[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

//int selectPlayer;     // Select Who Plays first 1=X, 2=O
int receivedMove = -1;  // Move received from opponent
int sentMove;           // Our move sent to opponent
  
byte player = 0;  // Select Who Plays first 1=X, 2=O
byte turn = 0;    // Value of current Turn in game

//We will use this to auto reset Game because there is no play and it hangs
const long ticTacLastPlayedInterval = 1000 * 10;     //30 sec              // 1000 * 60 * 2 - 120 sec
unsigned long ticTacLastPlayed = ticTacLastPlayedInterval;

// Interval timer for sending Invitations
byte maxRetryInviteEmptyIP = 0;
bool ticCallFirstRun = true;
unsigned long ticTacCallInterval = 1000 * 60 * 60;                   // 1000 * 60 * 60 - 60min
unsigned long ticCallLMInterval;

byte nrInvitationTry = 0; 

String playerName = "";
byte gameStarted = 0;
char opponentName[28] = "";
IPAddress opponentIP;
int opponentUDPport;

byte wantToPlay, selectPlayer;

// didIaskedToPlay = true or false set when sending invite
bool didIaskedToPlay = false;

String SERTIC = "SERVER: TicTac ";

/* ======================================================================
Function: updateTicTacToe
Purpose : Main TicTacToe Constructor ( listen and initiate game)
Input   : 
Output  : 
Comments: 
TODO    : */
void updateTicTacToe()
{
	// We Have config setting for manualy start/stop Tic Tac Toe
	if ( tictac_start == 1 )
	{
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
		if ((millis() - ticTacLastPlayed > ticTacLastPlayedInterval) && gameStarted == 1 && turn > 0)
		{
			#if (DEBUG == 1)
			writeLogFile(F("Inside LastPlayedTicTac measure and gameStarted == 1 and turn > 0"), 1, 1);
			#endif
			resetTicTacToe();
		}
		if ((millis() - ticTacLastPlayed > ticTacLastPlayedInterval) && didIaskedToPlay)
		{
			#if (DEBUG == 1)
			writeLogFile(F("Inside LastPlayedTicTac measure and didIaskedToPlay == true"), 1, 1);
			#endif
			resetTicTacToe();
		}
	}
}
// Setup Start variable values
void startGameValues( String playerName )
{
  // Here we will start a game 
  // RND am i playing first or second. and then send Proper message
  gameStarted = 1; // game Is In Play
  difficulty = random(3, 9); // AI difficulty
  
  #if ( DEBUG == 1 )
  writeLogFile("Difficulty set to: " + String(difficulty),1,1);
  #endif

  playerName.toCharArray(opponentName, 28); // Add Player Name
  opponentIP = ntpUDP.remoteIP(); // Let's fetch his IP and save it to later use
  opponentUDPport = ntpUDP.remotePort(); // Let's fetch his Port and save it to later use
}

// Before starting new game | After End of game
void resetTicTacToe()
{
  // Reset board to all 0
  for( int i = 0; i < 9;  ++i )
    board[i] = 0;

  turn = 0;
  player = 0;
  byte difficulty = 8;
  playerName = "";
  memset(opponentName, 0, sizeof(opponentName));
  gameStarted = 0;
  opponentIP = (0,0,0,0);
  opponentUDPport = 0;
  didIaskedToPlay = false;
}

GamePhases getGamePhase(String input)
{
  // Maybe Shorter
  int index = input.indexOf("WePlay");
  int index2 = input.indexOf("Player");
  int index3 = input.indexOf("Move");

  if (index > 0)  return WePlay;
  if (index2 > 0) return Player;
  if (index3 > 0) return Move;
  return Invalid;
}

char displayChar( int c )
{
  switch (c)
  {
    case -1:
      return 'X';
    case 0:
      return ' ';
    case 1:
      return 'O';
  }
  return 0;
}

// Just Draw Grid
void drawNumberedBoard()
{
  #if ( DEBUG == 1 )
  //writeLogFile("Grid number layout:",1,1);
  //writeLogFile(" 0 | 1 | 2 ",1,1);
  //writeLogFile("---+---+---",1,1);
  //writeLogFile(" 3 | 4 | 5 ",1,1);
  //writeLogFile("---+---+---",1,1);
  //writeLogFile(" 6 | 7 | 8 ",1,1);
  #endif
}

#if ( DEBUG == 1 )
// Display in Serial played moves after every move
void draw( int board[9] )
{
  byte vn;
  String bm = "\n ";
  for ( vn = 0; vn < 9; ++vn )
  {
    bm += String(displayChar(board[vn]));
    if ( vn == 2 || vn == 5 )
      bm += "\n---+---+---\n ";
    else if ( vn == 8 )
      bm += " ";
    else
      bm += " | ";
  }
  writeLogFile( bm, 1, 1);
}
#endif

/* ======================================================================
Function: win
Purpose : Check if there is a winner
Input   : 
Output  : 0 or 1 or -1
Comments: 
TODO    : */
int win( const int board[9] )
{
  if ( turn > 0 )
  {
    //list of possible winning positions
    byte wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
    byte winPos;
    for ( winPos = 0; winPos < 8; ++winPos )
    {
      if ( board[wins[winPos][0]] != 0 && board[wins[winPos][0]] == board[wins[winPos][1]] && board[wins[winPos][0]] == board[wins[winPos][2]] )
        return board[wins[winPos][2]];
    }
  }
  
  return 0;
}

/* ======================================================================
Function: minimax
Purpose : Calculate the best move for AI
Input   : 
Output  : Return best move - dependant on depth (IQ)
Comments: 
TODO    : */
int minimax( int board[9], int player, byte depth )
{
  //check if there is a winner
  int winner = win(board);
  if ( winner != 0 )
    return winner * player;
  
  int move = -1;
  int score = -2;
  byte i;
  for ( i = 0; i < 9; ++i )
  {
    if ( board[i] == 0 )
    {
      board[i] = player;
      int thisScore = 0;
      if ( depth < difficulty )
        thisScore = -minimax( board, player * -1, depth + 1 );

      if ( thisScore > score )
      {
        score = thisScore;
        move = i;
      }
      //choose the worst move for opponent
      board[i] = 0;
    }
  }
  if ( move == -1 ) return 0;
  return score;
}

/* ======================================================================
Function: computerMove
Purpose : AI Player
Input   : 
Output  : Play move
Comments: 
TODO    : */
void computerMove( int board[9] )
{
  int move = -1;
  if ( turn > 0 )
  {
    int score = -2;
    byte i;
    for ( i = 0; i < 9; ++i )
    {
      if ( board[i] == 0 )
      {
        board[i] = 1;
        int tempScore = -minimax(board, -1, 0);

        #if ( DEBUG == 1 )
        Serial.println("Temp Score: " + String(tempScore));
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
    move = random(0, 9);
  }
  
  //returns a score based on minimax tree at a given node.
  //write move/position of AI move
  board[move] = 1; 
  sentMove = move;  // Prepare to send My Move to opponent
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
bool playerMove( int board[9], byte moveKeypadNum )
{
  int move = 0;
  
  //Serial.println("Input move ([0..8]): ");

  //if keypad number is less than or equal to 8, the move is valid. 'moveKeypadNum' cannot be negative ( < 0 ) because it is a 'byte' type not 'int'.
  //If the user types '0', keypadConversion[0] = 255 which is also an invalid move (So the error message will appear)
  if ( moveKeypadNum <= 8 )
    move = moveKeypadNum; 
  else
    move = 255; //Set 'move' to 255 which will be picked up later as an invalid move
  
  #if ( DEBUG == 1 )
  Serial.println("Opponent played move: " + move);
  #endif

  // Here we check validity of selected Player move
  // This should be modiffied Proper respond when we includ Real player moves not AI
  if ( (move > 8 || move < 0) || board[move] != 0 )
  {
    #if ( DEBUG == 1 )
    Serial.println("Invalid move"); //Say if the player's move was invalid
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
Input   : what = 1 / Init Game // who
          what = 2 / Play move
Output  : What in next step in Game Play
Comments: 
TODO    : */
void letsPlay( byte what, String who )
{
  
  // Initialize game
  if ( what == 1 )
  {
    // Init StartGame values
    startGameValues( who );

    if ( player == 1 )
    {
      // This is fast response , need to brainstorm before 
      
      // Send message back to play his first move
      String replyPacket = SERTIC + String(_devicename) + " Move=-1";
      sendUDP ( replyPacket, opponentIP, 4210 );
    }

    if ( player == 2 )
    {
      #if ( DEBUG == 1 )
      printLogInPhase( "LetsPlay_w1p2" );
      #endif
      
      delay(1000);
      computerMove( board );
      delay(1000);
      #if ( DEBUG == 1)
      draw(board);
      #endif

      // This is fast response , need to brainstorm before 
      String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
      sendUDP ( replyPacket, opponentIP, 4210 );
    }
  }

  if ( what == 2 )
  {
    #if ( DEBUG == 1 )
    printLogInPhase( "LetsPlay_w2" );
    #endif

    String _tmp;
    _tmp = String(opponentName);
    // We are still Playing
    if ( turn < 9 && win(board) == 0 )
    {
      // Even || Odd player
      if ( (turn + player) % 2 == 0 )
      {
        computerMove( board );
        #if ( DEBUG == 1)
        draw(board);
        #endif

        String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
        sendUDP ( replyPacket, opponentIP, 4210 );
      }
      else
      {
        if ( playerMove( board, receivedMove ) )
        {
          #if ( DEBUG == 1)
          draw(board);
          #endif
          if ( turn < 9 && win(board) == 0 )
          {
            computerMove(board);
            #if ( DEBUG == 1)
            draw(board);
            #endif

            String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
            sendUDP ( replyPacket, opponentIP, 4210 );
          }
        }
        else
        {
          // -1 Is == Not Registered previous move || not allowed move
          String replyPacket = SERTIC + String(_devicename) + " Move=-1";
          sendUDP ( replyPacket, opponentIP, 4210 );
        }
      } 
    }

    // Checking agains board to decide Game Outcome
    if ( ( turn < 9 && win(board) != 0 ) ||  turn == 9 )
    {
      switch ( win(board) )
      {
        case 0:
          #if ( DEBUG == 1 )
          Serial.println("It's a draw.");
          #endif
          //sendTicTacWebhook(2);
          break;
        case 1:
          #if ( DEBUG == 1 )
          draw(board);
          Serial.println("You " + _tmp + " lose. Arduino wins!");
          #endif
          #ifdef MODULE_DISPLAY
          renderWIN = true;
				  messageWinON = true;
          messageON = true;
				  server.stop(); // Stopping webServer because it scrambles scroll buffer if accessed during scroll
          prevMilMesLife = millis();
          #endif
          sendTicTacWebhook(1);
          sendTicTacWebhook(2);
          break;
        case -1:
          #if ( DEBUG == 1 )
          Serial.println("You win!");
          #endif
          break;
      }
      // Let's prepare for new game
      #if ( DEBUG == 1 )
      Serial.println("Reseting game lets start over");
      #endif
      resetTicTacToe();
    }
  }
}

/* ======================================================================
Function: playTicTacToe
Purpose : initial Tic Tac Toe Manager
Input   : String input - UDP ASCII message received
Output  : 
TODO    :  */
void playTicTacToe( String input )
{
  playerName = parseUDP(input , 2);     // Parsing Input from UDP

  switch (getGamePhase(input))
  {
    case WePlay:
      wantToPlay = parseString(input, "WePlay=", 1).toInt();
      if ( wantToPlay == 0 )
      {
        // Remote don't want to play
        #if ( DEBUG == 1 )
        Serial.print(F(REPLAYER));
        Serial.print(playerName);
        Serial.println(F(" do not want to play"));
        printLogInPhase( "WePlay" );
        #endif
        
        break;
      } 
      else if ( wantToPlay == 1 && gameStarted == 1 )
      {
        // We cannot play because we are already playing
        // Here we can check if we are playing with the same player already
        // But i think simultanious games with the same Player are not OK
        #if ( DEBUG == 1 )
        Serial.print(REPLAYER + playerName);
        Serial.println(F(" ask to play but we are already playing"));
        printLogInPhase( "WePlay" );
        #endif
        
        String replyPacket = SERTIC + String(_devicename) + " WePlay=0";
        sendUDP ( replyPacket, ntpUDP.remoteIP(), ntpUDP.remotePort() );
       
        #if ( DEBUG == 1 )
        Serial.println(F("Reseting game"));
        #endif
        resetTicTacToe();
       
        break;
      }
      else if ( wantToPlay == 1 && gameStarted == 0 )
      {
        // Init Start Game Values
        startGameValues( playerName );

        // Check if I'm the one asking
        // That Way We know how to respond
        if ( didIaskedToPlay )
        {
          // We will randomly choose To ask Opponent to choose Player 1 || 2 , or we will Choose 1 || 2
          // 0 = Ask, 1 = Play 1st, 2 = Play 2nd
          randNumber = random(0, 3);
          #if ( DEBUG == 1 )
          writeLogFile( "In sending Player decision: " + String(randNumber), 1, 1 );
          #endif
          if ( randNumber == 2 )
            player = 1;
          if ( randNumber == 1 )
            player = 2;

          // Here I'm Back with answer and I need to ask oponent is he playin 1st or second
          String replyPacket = SERTIC + String(_devicename) + " Player=" + randNumber;
          sendUDP ( replyPacket, opponentIP, 4210 );
          break;
        }
        else
        {
          // I didnt sent Play Invitation so i just reply Yes I want to play
          String replyPacket = SERTIC + String(_devicename) + " WePlay=1";
          sendUDP ( replyPacket, opponentIP, 4210 );
          break;
        }
        
      }
      break;

    case Player:
      // First lets check if we can talk 
      if ( gameStarted == 0 )
      {
        // I didnt sent Play Invitation so i just reply Yes I want to play
        #if ( DEBUG == 1 )
        Serial.println(F("Somebody Skipped directly to Player"));
        #endif
        String replyPacket = SERTIC + String(_devicename) + " WePlay=0";
        sendUDP ( replyPacket, ntpUDP.remoteIP(), ntpUDP.remotePort() );
        break;
      }

      selectPlayer = parseString( input, "Player=", 1).toInt();
      if ( selectPlayer == 0 )
      {
        // We need to make a decision Player 1 || 2
        // We Send What Player are We, and Assign his
        randNumber = random(1, 3);  
        if ( randNumber == 1 )
          player = 2;
        else
          player = 1;

        String replyPacket = SERTIC + String(_devicename) + " Player=" + randNumber;
        sendUDP ( replyPacket, opponentIP, 4210 );
      }
      else if ( ( selectPlayer == 1 || selectPlayer == 2) && gameStarted == 0 )
      {
        #if ( DEBUG == 1 )
        writeLogFile( selectPlayer + " Selected to play as Player: " + String(selectPlayer), 1, 1 );
        #endif
        player = selectPlayer;

        #if ( DEBUG == 1 )
        printLogInPhase( "Player" );
        #endif

        letsPlay( 1, playerName );
      }
      else if ( selectPlayer == 1 && gameStarted == 1 && turn == 0 )
      {
        // Opponent decided to play As player 1, lets register player as 1 and send him to play
        player = selectPlayer;

        #if ( DEBUG == 1 )
        printLogInPhase( "Player" );
        #endif

        String replyPacket = SERTIC + String(_devicename) + " Move=-1";
        sendUDP  (replyPacket, opponentIP, 4210 );
      }
      else if ( selectPlayer == 2 && gameStarted == 1 && turn == 0 )
      {
        // Opponent decided to play As player 2, lets register player as 2 and start play
        player = selectPlayer;

        #if ( DEBUG == 1 )
        printLogInPhase( "Player" );
        #endif

        letsPlay( 2, playerName );
        // replyPacket = SERTIC + String(_devicename) + " Move=-1";
        //sendUDP(replyPacket, opponentIP,4210);
      }
      break;

    case Move:
      // Getting Move response from Remote Player
      receivedMove = parseString(input, "Move=", 1).toInt();
      #if ( DEBUG == 1 )
      writeLogFile( "Game is in Turn: " + String(turn), 1, 1 );
      #endif
      // MyTurn to play but no Turn Played
      if ( receivedMove == -1 && turn == 0 )
      {
        letsPlay( 1, playerName );
      }

      if ( receivedMove >= 0 && gameStarted == 1 )
      {
        // Check If we can play with this player
        if ( String(opponentName) == playerName )
        {
          // We have our oponent lets continue
          #if ( DEBUG == 1 )
          Serial.print(REPLAYER + playerName);
          Serial.print(F(" played move: "));
          Serial.println( String(receivedMove) );
          #endif
          
          letsPlay( 2, playerName );
        }
        else
        {
          // Somebody else sent us move
          #if ( DEBUG == 1 )
          Serial.println( "Opponent Name: " + String(opponentName) + " - PlayeName: " + playerName );
          Serial.print(F("We are not playing With "));
          Serial.println(REPLAYER + playerName);
          #endif
        }   
      }
      break;
    default:
      // Do nothing
      break;
  }
}

/* ======================================================================
Function: inviteDeviceTicTacToe
Purpose : invite available device to play Tic Tac Toe with you (SSDP)
Input   : 
Output  : 
TODO    : In here We Can have settup for Playing UDP or MQTT */
void inviteDeviceTicTacToe()
{
  // Let's check if we are already playing
  if ( gameStarted == 0 )
  {
    // We are randomly picking 3 times one of available addresses
    randNumber = random(0, actualSSDPdevices);  
    //Serial.println("Random broj: " + String(randNumber));
    
    // Marked that we sent invitation | We are locked until we set this to false
    didIaskedToPlay = true;
    // We also do the hack to auto unlock us using Stale game
    ticTacLastPlayed = millis();

    IPAddress rndAddr = IPAddress(foundSSDPdevices[randNumber]);
    if ( rndAddr )
    {
      #if ( DEBUG == 1 )
      writeLogFile( "Sending Invitation to IP: " + rndAddr.toString(), 1, 1 );
      #endif
      
      String replyPacket = SERTIC + String(_devicename) + " WePlay=1";
      sendUDP ( replyPacket, rndAddr, 4210 );
    }
    else
    {
      #if ( DEBUG == 1 )
       writeLogFile( F("Empty IPlist for Invite"), 1, 1 );
      #endif
      
      //Allow enter again in sending
      if ( maxRetryInviteEmptyIP < 50 )
        ticCallFirstRun = true;

      maxRetryInviteEmptyIP++;
    }
   

    //ntpUDP.beginPacket( IPAddress(foundSSDPdevices[randNumber]), 4210 );
    //ntpUDP.write( replyPacket.c_str() );
    //ntpUDP.endPacket();
  }
}

/* ======================================================================
Function: sendTicTacWebhook
Purpose : Send Webhook to Discord or WebServer
Input   : where = 1 / Send to Discord
          where = 2 / Send to WebServer
Output  : 
TODO    :  We should build 2 different calls 1. Discord, 2 Webhook
           Without Secure Client it's not possible to send to Discord Directly */
void sendTicTacWebhook(byte where)
{
	char *localURL;
	String data;

	localURL = discord_url;
	String discordUsername = _devicename;

	// We also check if we can publish to Webhook
	// For now its just demo
	if (where == 1 && (tictac_webhook == 1 || tictac_discord == 1))
	{
		String discordAvatar = discord_avatar;
		String message = F("I Won! Loser: ") + playerName + F(" lost.");
		data = F("{\"username\":\"") + discordUsername + F("\",\"avatar_url\":\"") + discordAvatar + F("\",\"content\":\"") + message + F("\"}");
		sendWebhook(localURL, data);
	}
}

#if (DEBUG == 1)
// to Optimize code.
void printLogInPhase(String where)
{
	writeLogFile("in " + where + ", gameStarted: " + String(gameStarted), 1, 1);
	writeLogFile("in " + where + ", player: " + String(player), 1, 1);
	writeLogFile("in " + where + ", turn: " + String(turn), 1, 1);
}
#endif

#endif   // MODULE_TICTACTOE