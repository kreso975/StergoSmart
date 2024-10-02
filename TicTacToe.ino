#if ( STERGO_PROGRAM == 0 || STERGO_PROGRAM == 2 )
/*
 *  IDEAS:  - 1st request includes: DO YOU WANT TO PLAY + CHOOSE PLAY 1st or 2nd
 *          - at the END of the game, Looser sends: Congratulations + Do you want to play + Choose play 1st or 2nd
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
TicTacToe layout:
  0 | 1 | 2  
  ---+---+---
  3 | 4 | 5 
  ---+---+--- 
  6 | 7 | 8 
*/

// Setup Start variable values
void startGameValues( String playerName )
{
  // Here we will start a game 
  // RND am i playing first or second. and then send Proper message
  gameStarted = 1; // game Is In Play
  difficulty = random(3, 9); // AI difficulty
  Serial.println("Difficulty set to: " + String(difficulty));
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
  Serial.println("Grid number layout:");
  Serial.println(" 0 | 1 | 2 ");
  Serial.println("---+---+---");
  Serial.println(" 3 | 4 | 5 ");
  Serial.println("---+---+---");
  Serial.println(" 6 | 7 | 8 ");
}

// Display in Seral played moves after every move
void draw( int board[9] )
{
  Serial.print("\n "); Serial.print(displayChar(board[0])); Serial.print(" | "); Serial.print(displayChar(board[1])); Serial.print(" | "); Serial.print(displayChar(board[2])); Serial.println(" ");
  Serial.println("---+---+---");
  Serial.print(" "); Serial.print(displayChar(board[3])); Serial.print(" | "); Serial.print(displayChar(board[4])); Serial.print(" | "); Serial.print(displayChar(board[5])); Serial.println(" ");
  Serial.println("---+---+---");
  Serial.print(" "); Serial.print(displayChar(board[6])); Serial.print(" | "); Serial.print(displayChar(board[7])); Serial.print(" | "); Serial.print(displayChar(board[8])); Serial.println(" ");
}

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
        Serial.println("Temp Score: " + String(tempScore));
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
    // 0 is included, till 9, 9 not included
    move = random(0, 9);
  }
  
  //returns a score based on minimax tree at a given node.
  //write move/position of AI move
  board[move] = 1; 
  sentMove = move;  // Prepare to send My Move to opponent
  ticTacLastPlayed = millis();
  ++turn;
}

//
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
  
  Serial.println("Opponent played move: " + move);
  
  // Here we check validity of selected Player move
  // This should be modiffied Proper respond when we includ Real player moves not AI
  if ( (move > 8 || move < 0) || board[move] != 0 )
  {
    Serial.println("Invalid move"); //Say if the player's move was invalid
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


// what = 1 / Init Game // who
// what = 2 / Play move
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
      sendUDP(replyPacket, opponentIP);
    }

    if ( player == 2 )
    {
      printLogInPhase( "LetsPlay_w1p2" );

      delay(1000);
      computerMove( board );
      delay(1000);
      draw(board);

      // This is fast response , need to brainstorm before 
      String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
      sendUDP(replyPacket, opponentIP);
    }
  }

  if ( what == 2 )
  {
    printLogInPhase( "LetsPlay_w2" );

    String _tmp;
    _tmp = String(opponentName);
    // We are still Playing
    if ( turn < 9 && win(board) == 0 )
    {
      // Even || Odd player
      if ( (turn + player) % 2 == 0 )
      {
        computerMove( board );
        draw(board);

        String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
        sendUDP(replyPacket, opponentIP);
      }
      else
      {
        if ( playerMove( board, receivedMove ) )
        {
          draw(board);
          if ( turn < 9 && win(board) == 0 )
          {
            computerMove(board);
            draw(board);

            String replyPacket = SERTIC + String(_devicename) + " Played Move=" + String(sentMove);
            sendUDP(replyPacket, opponentIP);
          }
        }
        else
        {
          // -1 Is == Not Registered previous move || not allowed move
          String replyPacket = SERTIC + String(_devicename) + " Move=-1";
          sendUDP(replyPacket, opponentIP);
        }
      } 
    }

    // Checking agains board to decide Game Outcome
    if ( ( turn < 9 && win(board) != 0 ) ||  turn == 9 )
    {
      switch ( win(board) )
      {
        case 0:
          Serial.println("It's a draw.");
          break;
        case 1:
          draw(board);
          Serial.println("You " + _tmp + " lose. Arduino wins!");
          sendWebhook( 2 );
          break;
        case -1:
          Serial.println("You win!");
          break;
      }
      // Let's prepare for new game
      Serial.println("Reseting game lets start over");
      resetTicTacToe();
    }
  }
}


/* ======================================================================
Function: playTicTacToe
Purpose : initial Tic Tac Toe Manager
Input   : String input - UDP ASCII message received
Output  : 
TODO    : 
====================================================================== */
void playTicTacToe( String input )
{
  playerName = parseValues(input , 2);     // Parsing Input from UDP
  // NameSpaces 
  gamephase = getGamePhase(input);
  switch (gamephase)
  {
    case WePlay:
      wantToPlay = parseSSDP(input, "WePlay=", 1).toInt();
      if ( wantToPlay == 0 )
      {
        // Remote don't want to play
        Serial.print(F(REPLAYER));
        Serial.print(playerName);
        Serial.println(F(" do not want to play"));
        printLogInPhase( "WePlay" );
        
        break;
        // Trigger Proces for Looking someone else to play
        // If we are the one Initiating GameStart
       
      } 
      else if ( wantToPlay == 1 && gameStarted == 1 )
      {
        // We cannot play because we are already playing
        // Here we can check if we are playing with the same player already
        // But i think simultanious games with the same Player are not OK
        Serial.print(REPLAYER + playerName);
        Serial.println(F(" ask to play but we are already playing"));
        
        
        printLogInPhase( "WePlay" );
        
        String replyPacket = SERTIC + String(_devicename) + " WePlay=0";
        sendUDP(replyPacket, ntpUDP.remoteIP());
       
        Serial.println(F("Reseting game"));
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
          Serial.println("In sending Player decision: " + String(randNumber));
          if ( randNumber == 2 )
            player = 1;
          if ( randNumber == 1 )
            player = 2;

          // Here I'm Back with answer and I need to ask oponent is he playin 1st or second
          String replyPacket = SERTIC + String(_devicename) + " Player=" + randNumber;
          sendUDP(replyPacket, opponentIP);
          break;
        }
        else
        {
          // I didnt sent Play Invitation so i just reply Yes I want to play
          String replyPacket = SERTIC + String(_devicename) + " WePlay=1";
          sendUDP(replyPacket, opponentIP);
          break;
        }
        
      }
      break;

    case Player:
      // First lets check if we can talk 
      if ( gameStarted == 0 )
      {
        // I didnt sent Play Invitation so i just reply Yes I want to play
        Serial.println(F("Somebody Skipped directly to Player"));
        String replyPacket = SERTIC + String(_devicename) + " WePlay=0";
        sendUDP(replyPacket, ntpUDP.remoteIP());
        break;
      }

      selectPlayer = parseSSDP( input, "Player=", 1).toInt();
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
        sendUDP(replyPacket, opponentIP);
      }
      else if ( ( selectPlayer == 1 || selectPlayer == 2) && gameStarted == 0 )
      {
        Serial.println( selectPlayer + " Selected to play as Player: " + String(selectPlayer) );
        player = selectPlayer;
        printLogInPhase( "Player" );

        letsPlay( 1, playerName );
      }
      else if ( selectPlayer == 1 && gameStarted == 1 && turn == 0 )
      {
        // Opponent decided to play As player 1, lets register player as 1 and send him to play
        player = selectPlayer;
        printLogInPhase( "Player" );

        String replyPacket = SERTIC + String(_devicename) + " Move=-1";
        sendUDP(replyPacket, opponentIP);
      }
      else if ( selectPlayer == 2 && gameStarted == 1 && turn == 0 )
      {
        // Opponent decided to play As player 2, lets register player as 2 and start play
        player = selectPlayer;
        printLogInPhase( "Player" );

        letsPlay( 2, playerName );
        // replyPacket = SERTIC + String(_devicename) + " Move=-1";
        //sendUDP(replyPacket, opponentIP);
      }
      break;

    case Move:
      // Getting Move response from Remote Player
      receivedMove = parseSSDP(input, "Move=", 1).toInt();
      Serial.println("Game is in Turn: " + String(turn));
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
          Serial.print(REPLAYER + playerName);
          Serial.print(F(" played move: "));
          Serial.println( String(receivedMove) );
          
          letsPlay( 2, playerName );
        }
        else
        {
          // Somebody else sent us move
          Serial.println( "Opponent Name: " + String(opponentName) + " - PlayeName: " + playerName );
          Serial.print(F("We are not playing With "));
          Serial.println(REPLAYER + playerName);
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
TODO    : In here We Can have settup for Playing UDP or MQTT
====================================================================== */
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

    Serial.println("Sending Invitation to IP: " + foundSSDPdevices[randNumber].toString());

    String replyPacket = SERTIC + String(_devicename) + " WePlay=1";
    ntpUDP.beginPacket( IPAddress(foundSSDPdevices[randNumber]), 4210 );
    ntpUDP.write( replyPacket.c_str() );
    ntpUDP.endPacket();
  }
}


/* ======================================================================
String  : parseValues()
Purpose : to Optimize code. Easyer to read playTicTacToe
Input   : input - received raw ASCII from UDP | SERVER: TicTac deviceNAME+chipID Move=8
          part  - requested element of array to be returned
Output  : String of input[part]
TODO    : 
====================================================================== */
String parseValues( String input, int part )
{
  char buf[input.length() + 1];
  input.toCharArray(buf, sizeof(buf));
  char* values[4];
  byte i=0;
  char* token = strtok(buf, " "); // Delimiter is " " , we get 5 Strings

  while (token != NULL)
  { //Serial.println(token);
    values[i++] = token;
    token = strtok(NULL, " ");
  }

  return values[part];
}

// to Optimize code.
void printLogInPhase( String where )
{
  /*
  Serial.println("in " + where + ", gameStarted: " + String(gameStarted));
  Serial.println("in " + where + ", player: " + String(player));
  Serial.println("in " + where + ", turn: " + String(turn));
  */
}
#endif
