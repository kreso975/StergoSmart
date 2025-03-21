#ifndef MODULE_TIC_TAC_TOE_H
#define MODULE_TIC_TAC_TOE_H

#include <Arduino.h>
// Define the size of the Tic Tac Toe board
#define BOARD_SIZE 9

// test param
extern byte HUBproxy;
extern IPAddress HUBproxy_ip;
extern int HUBproxy_port;

// External functions
extern void updateSSDP();
extern char* parseAndExtract(const char* input, const char* key, const char* delimiter, int part);
extern void sendWebhook(const char* localURL, const char* data);

// Config.ino
extern byte tictac_start, tictac_interval, tictac_webhook, tictac_discord;

// Difficulty defines number of Depth for AI , bigger Depth stronger AI
extern byte difficulty;
extern int board[BOARD_SIZE];

enum GamePhases { WePlay, Player, Move, Invalid };
extern GamePhases gamephase;

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

//extern byte nrInvitationTry; 

extern char playerName[64];
extern byte gameStarted;
extern char opponentName[28];
extern IPAddress opponentIP;
extern int opponentUDPport;

// didIaskedToPlay = true or false set when sending invite
extern bool didIaskedToPlay;

#define REPLAYER "Remote Player"

extern String SERTIC;

extern void updateTicTacToe();
extern void startGameValues( const char* playerName );
extern void resetTicTacToe();
extern GamePhases getGamePhase(const char* input);
extern int win( const int board[BOARD_SIZE] );
extern int minimax( int board[BOARD_SIZE], int player, byte depth );
extern void computerMove( int board[BOARD_SIZE] );
extern bool playerMove(int board[BOARD_SIZE], byte moveToPlay);
extern void letsPlay( byte what, const char* who );
extern void playTicTacToe(const char* input);
extern void inviteDeviceTicTacToe();
extern void sendTicTacWebhook(byte where);
extern void checkIfHUBProxyPlay(const char *message);
#if (DEBUG == 1)
   extern void draw( int board[BOARD_SIZE] );
   extern void printLogInPhase(String where);
#endif

#endif // MODULE_TIC_TAC_TOE_H