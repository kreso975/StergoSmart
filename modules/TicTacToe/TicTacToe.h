#ifndef MODULE_TIC_TAC_TOE_H
#define MODULE_TIC_TAC_TOE_H

#include <Arduino.h>

// External functions
extern void updateSSDP();
extern String parseUDP(String input, int part);
extern String parseString( String str, String found, int what );
extern void sendWebhook(const char* localURL, const char* data);

// Config.ino
extern byte tictac_start, tictac_interval, tictac_webhook, tictac_discord;

// Difficulty defines number of Depth for AI , bigger Depth stronger AI
extern byte difficulty;
extern  int board[9];

enum GamePhases { WePlay, Player, Move, Invalid };
extern GamePhases gamephase;

//int selectPlayer;     // Select Who Plays first 1=X, 2=O 
extern int receivedMove;  // Move received from opponent
extern int sentMove;      // Our move sent to opponent
  
extern byte player;  // Select Who Plays first 1=X, 2=O
extern byte turn;    // Value of current Turn in game

//We will use this to auto reset Game because there is no play and it hangs
extern const long ticTacLastPlayedInterval;     // 1000 * 60 * 2 - 120 sec
extern unsigned long ticTacLastPlayed;

// Interval timer for sending Invitations
extern byte maxRetryInviteEmptyIP;
extern bool ticCallFirstRun;
extern unsigned long ticTacCallInterval;        // 1000 * 60 * 60 - 60min
extern unsigned long ticCallLMInterval;

extern byte nrInvitationTry; 

extern String playerName;
extern byte gameStarted;
extern char opponentName[28];
extern IPAddress opponentIP;
extern int opponentUDPport;

// wantToPlay = Parsed value of WePlay=
// selectPlayer = Parsed value of Player=
extern byte wantToPlay, selectPlayer;

// didIaskedToPlay = true or false set when sending invite
extern bool didIaskedToPlay;

#define REPLAYER "Remote Player"
extern String SERTIC;

extern void updateTicTacToe();
extern void startGameValues( const char* playerName );
extern void resetTicTacToe();
extern GamePhases getGamePhase(String input);
extern int win( const int board[9] );
extern int minimax( int board[9], int player, byte depth );
extern void computerMove( int board[9] );
extern bool playerMove( int board[9], byte moveKeypadNum );
extern void letsPlay( byte what, const char* who );
extern void playTicTacToe( String input );
extern void inviteDeviceTicTacToe();
extern void sendTicTacWebhook(byte where);
#if (DEBUG == 1)
   extern void draw( int board[9] );
   extern void printLogInPhase(String where);
#endif

#endif // MODULE_TIC_TAC_TOE_H