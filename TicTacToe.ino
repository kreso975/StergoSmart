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

// Before starting new game
void resetTicTacToe()
{
  for( int i = 0; i < 9;  ++i )
    board[i] = 0;

  turn = 0;
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

void drawNumberedBoard()
{
  Serial.println("Grid number layout:");
  Serial.println(" 0 | 1 | 2 ");
  Serial.println("---+---+---");
  Serial.println(" 3 | 4 | 5 ");
  Serial.println("---+---+---");
  Serial.println(" 6 | 7 | 8 ");
}

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
  //list of possible winning positions
  unsigned wins[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
  int winPos;
  for ( winPos = 0; winPos < 8; ++winPos )
  {
    if ( board[wins[winPos][0]] != 0 && board[wins[winPos][0]] == board[wins[winPos][1]] && board[wins[winPos][0]] == board[wins[winPos][2]] )
      return board[wins[winPos][2]];
  }
  return 0;
}

int minimax( int board[9], int player, int depth )
{
  //check the positions for players
  int winner = win(board);
  if ( winner != 0 ) return winner * player;

  int move = -1;
  int score = -2;
  int i;
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
  //unsigned long computerMoveStartTime = millis();
  //Serial.println("Thinking..."); //Say when the computer is thinking (without it the program looks like it has crashed)
  
  int move = -1;
  int score = -2;
  int i;
  for ( i = 0; i < 9; ++i )
  {
    if ( board[i] == 0 )
    {
      board[i] = 1;
      int tempScore = -minimax(board, -1, 0);
      board[i] = 0;
      if ( tempScore > score )
      {
        score = tempScore;
        move = i;
      }
    }
  }
  //returns a score based on minimax tree at a given node.
  //write move/position of AI move
  board[move] = 1; 
  sentMove = move;  // Prepare to send My Move to opponent
  ++turn;

  //Serial.print("Arduino was thinking for "); Serial.print(millis() - computerMoveStartTime); Serial.println(" ms");
}

void playerMove( int board[9], byte moveKeypadNum )
{
  int move = 0;
  
  Serial.println("Input move ([1..9]): ");

  //if keypad number is less than or equal to 9, the move is valid. 'moveKeypadNum' cannot be negative ( < 0 ) because it is a 'byte' type not 'int'.
  //If the user types '0', keypadConversion[0] = 255 which is also an invalid move (So the error message will appear)
  if ( moveKeypadNum <= 9 )
    move = moveKeypadNum; 
  else
    move = 255; //Set 'move' to 255 which will be picked up later as an invalid move
  
  if ( (move > 8 || move < 0) || board[move] != 0 )
      Serial.println("Invalid move"); //Say if the player's move was invalid

  board[move] = -1;

  ++turn;
}

void letsPlay()
{
  Serial.println(" 2. Player is: " + String(player) );

  if ( turn < 9 && win(board) == 0 )
  {
    // Even || Odd player
    if ( (turn + player) % 2 == 0 )
    {
      computerMove( board );
      draw(board);
    }
    else
    {
      playerMove( board, receivedMove );
      draw(board);
      if ( turn < 9 && win(board) == 0 )
        computerMove(board);
        draw(board);
    } 
  }

  if ( ( turn < 9 && win(board) != 0 ) ||  turn == 9 )
  {
    switch ( win(board) )
    {
      case 0:
        Serial.println("It's a draw.");
        break;
      case 1:
        draw(board);
        Serial.println("You lose. Arduino wins!");
        break;
      case -1:
        Serial.println("You win!");
        break;
    }
    // Let's prepare for new game
    Serial.println("Reseting game lets start over");
    resetTicTacToe();
  }
  //Serial.println(" 2. Turn is: " + String(turn) );
  //for (int gr = 0; gr < 9; ++ gr) Serial.println( String(gr) + " - " + String(board[gr]) );
}

#endif
