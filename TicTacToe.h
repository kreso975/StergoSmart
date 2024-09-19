
#if ( STERGO_PROGRAM == 2 )
  #define MODEL_NAME    "TT001"
  #define MODEL_NUMBER  "v01"
#endif

#define difficulty 8
int board[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  
int selectPlayer;  // Select Who Plays first 1=X, 2=O
int receivedMove;  // Move received from opponent
int sentMove;      // Our move sent to opponent
  
byte player;
unsigned turn = 0;
