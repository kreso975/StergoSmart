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

# Tic‑Tac‑Toe Communication Protocol

## PHASE 1 — INVITATION NEGOTIATION
This is the “do you want to play?” handshake.

### Messages
- `WePlay=1` → “I want to play”
- `WePlay=0` → “I refuse”

### Device variables involved
- `didIaskedToPlay`
- `gameStarted`
- `opponentIP`
- `opponentUDPport`
- `playerName`
- `opponentName`

### Flow
1. Browser → Device: `WePlay=1`
2. Device checks:
   - If already playing → sends `WePlay=0`
   - If free → accepts and sends `WePlay=1`
3. Device sets:
   - `gameStarted = 1`
   - `didIaskedToPlay = false` (because YOU asked)

### Possible outcomes
- ACCEPTED → go to Phase 2
- REFUSED → stop

---

## PHASE 2 — PLAYER NEGOTIATION (WHO STARTS)
This is the “who is X, who is O” handshake.

### Messages
- `Player=0` → “I don’t choose, you choose”
- `Player=1` → “I am Player 1 (X), I start”
- `Player=2` → “I am Player 2 (O), you start”

### Device variables involved
- `player`
- `turn`
- `randNumber`
- `didIaskedToPlay`
- `gameStarted`

### Flow
After `WePlay=1`:

#### If device asked first (`didIaskedToPlay = true`)
Device randomly chooses:
- `Player=1` (device starts)
- `Player=2` (you start)
- `Player=0` (ask you to choose)

Then sends: `Player=<value>`

#### If you asked first
- Device waits for your `Player=<value>`
- If you send nothing → device sends its own `Player=<value>`

### Possible outcomes
- You start → device sends `Move=-1`
- Device starts → device sends first `Move=<index>`
- You choose manually → browser sends `Player=1` or `Player=2`

---

## PHASE 3 — GAMEPLAY (MOVES)
This is the actual Tic‑Tac‑Toe game.

### Messages
- `Move=-1` → “Your turn, you start”
- `Move=<0..8>` → “I played here”

### Device variables involved
- `board[9]`
- `turn`
- `receivedMove`
- `sentMove`
- `ticTacLastPlayed`
- `difficulty`
- `win(board)`
- `computerMove()`
- `playerMove()`

### Flow
1. Whoever starts sends: `Move=-1` → “your turn”
2. Opponent plays: `Move=<index>`
3. Device:
   - Validates move
   - Updates board
   - Checks win/draw
   - If game continues → sends next move

### Game ends when
- `win(board) != 0`
- OR `turn == 9`

Then device:
- Resets internally (no special message)

---

## COMPLETE OPTION TREE (ALL POSSIBLE PATHS)

### 1. Invitation
- Accept  
- Refuse  
- Already playing → refuse + reset  

### 2. Player selection
- Device chooses  
- Browser chooses  
- Device asks browser to choose  
- Browser asks device to choose  
- Random selection  
- Forced selection (manual override)  

### 3. Gameplay
- Device starts  
- Browser starts  
- Device plays AI move  
- Browser plays human move  
- Invalid move → device sends `Move=-1`  
- Win  
- Loss  
- Draw  
- Auto‑reset after timeout  
