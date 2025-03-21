<span align="center">

❌⭕❌  
⭕❌⭕  
❌⭕❌  

# Tic Tac Toe

</span>

Tic Tac Toe that *never stops playing!* With **Autoplay over UDP**, your smart devices find clever ways to stay busy... even when doing absolutely nothing. 😄  

> [!NOTE]    
> On boot, an **M-SEARCH over UDP** scans for your devices in the local network. Detected devices are compiled into a list, and every **N minutes**, a device initiates a new game with a randomly chosen partner from the list of available devices.  
>  
> If a **UDP proxy HUB** is detected in the local network, the game transitions seamlessly to communicate through the HUB. The HUB modifies messages to enable centralized monitoring and live tracking of gameplay across all devices. It also provides the option to insert a human player to compete against a selected device.  
>  
> When a victorious device emerges, it proudly announces its win by publishing to Discord. 🏆✨  

---

### Tic Tac Toe Module Features
- **Autonomous Game Invitations**: Devices automatically send game invitations to others every **N minutes** for endless fun and engagement.  
- **Dynamic Communication Modes**: Automatically switches between direct communication and communication via the UDP proxy HUB based on network detection.  
- **Live Monitoring**: The UDP proxy HUB facilitates centralized gameplay tracking and even allows human players to join the fun.  
- **Smart Board Reset**: The game board resets itself after a set period of inactivity to ensure seamless gameplay.  
- **AI with Configurable Difficulty**: Play against an AI opponent and adjust the difficulty level to suit your challenge preferences.  
- **Discord Integration**: Game outcomes are shared on Discord, allowing devices to boast about their victories to the world.  

> [!TIP]    
> Want to enjoy just Tic Tac Toe? You can compile **StergoSmart** to include only this module.  
> Simply update your `settings.h` file by setting:  
> `#define STERGO_PROGRAM 2`
