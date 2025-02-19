<span align="center">

❌⭕❌  
⭕❌⭕  
❌⭕❌  
  
 # Tic Tac Toe 


</span>
  
  
Autoplay over UDP so that my smart devices can do something while doing nothing :)
  
> [!NOTE]  
> On boot, M-SEARCH over UDP looks for my devices. They are added to a list, and every N minutes,  
> a device will try to initiate a new game with a randomly selected device from the gathered list of available devices.  
>   
> If there is a winner, that device will publish to Discord that it won.  
>  
> The Tic Tac Toe module includes several features:  
> - Initiates game invitations to available devices every N minutes.  
> - Resets the game board automatically after a period of inactivity.  
> - Uses AI with configurable difficulty levels to play against.  
> - Publishes the game outcome to Discord.  
  
> [!TIP]  
> StergoSmart can be compiled to feature only Tic Tac Toe.  
> In Config.h at the top, set `#define STERGO_PROGRAM 2`.  
  
  