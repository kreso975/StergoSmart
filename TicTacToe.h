
#if ( STERGO_PROGRAM == 2 )
#define MODEL_NAME    "TT001"
#define MODEL_NUMBER  "v01"
#endif

// Config.ino
byte tictac_start, tictac_interval, tictac_webhook, tictac_discord;

// Difficulty defines number of Depth for AI , bigger Depth stronger AI
byte difficulty = 8;
int board[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

enum GamePhases { WePlay, Player, Move, Invalid };
GamePhases gamephase;

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

// wantToPlay = Parsed value of WePlay=
// selectPlayer = Parsed value of Player=
byte wantToPlay, selectPlayer;

// didIaskedToPlay = true or false set when sending invite
bool didIaskedToPlay = false;

#define REPLAYER "Remote Player"
String SERTIC = "SERVER: TicTac ";