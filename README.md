# Connect 4 – Arduino NeoPixel 10x10

A colorful **Connect 4** game for Arduino, played on a 10x10 NeoPixel LED matrix with physical buttons, timer, demo mode, two 7-segment displays, and day/night brightness control.

---

## ✨ Features

- 10x10 NeoPixel LED matrix board (zigzag wiring)
- Two players, each with 3 buttons (Left, Down, Right)
- **Debounced** button handling for reliable input
- **Demo mode:** Watch the game play automatically with random moves and colors
- **Timer mode:** Make your move before time runs out, or lose your turn!
- **Random player colors** at each game start
- **Random chip start position** for dynamic rounds
- **Animated "melt" effect** after every game
- **Two TM1637 7-segment displays** for scores, timer, and animations
- **Day/Night mode:** Toggle brightness for displays and LEDs
- **Win celebration animation** on the displays
- Well-commented and customizable Arduino code

---

## 🛠️ Hardware Required

- 1 × Arduino UNO (or compatible)
- 1 × 10x10 NeoPixel (WS2812) matrix (100 LEDs, zigzag pattern)
- 2 × TM1637 4-digit 7-segment display modules
- 6 × push buttons (3 per player: Left, Down, Right)
- 1 × Demo mode button
- 1 × Timer mode button
- 1 × Brightness toggle button
- Jumper wires, resistors, breadboard, etc.

---

## 📌 Pin Mapping

| Function           | Arduino Pin   |
|--------------------|--------------|
| NeoPixel Data      | D6           |
| Player 1 Left      | A2           |
| Player 1 Down      | A1           |
| Player 1 Right     | A0           |
| Player 2 Left      | D4           |
| Player 2 Down      | D7           |
| Player 2 Right     | D5           |
| Demo Mode Button   | D8           |
| Timer Mode Button  | D10          |
| Brightness Button  | D9           |
| TM1637 CLK 1       | D2           |
| TM1637 DIO 1       | D3           |
| TM1637 CLK 2       | D11          |
| TM1637 DIO 2       | D12          |

---

## 📦 Libraries Used

- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- [TM1637Display](https://github.com/avishorp/TM1637)
- [TM1637TinyDisplay](https://github.com/jasonacox/TM1637TinyDisplay)

You can install these via Arduino Library Manager or download them from GitHub.

---

## 🎮 How to Play

- **Player 1** uses: `A2` (Left), `A1` (Down), `A0` (Right)
- **Player 2** uses: `D4` (Left), `D7` (Down), `D5` (Right)
- Press "Down" to drop your chip in the selected column.
- First to connect 4 in a row (horizontally, vertically, or diagonally) wins!
- **Timer mode:** If you don't play in time, your turn is lost.
- **Demo mode:** Activate for an automated match with random moves and colors.
- **Brightness button:** Switch between day and night modes.

---

## ⚡ Customization

- Board size: Change `MATRIX_WIDTH` and `MATRIX_HEIGHT` in the code
- Adjust timers, colors, or brightness as you wish
- Edit or add new animations in `animations.h`

---

## 🙏 Credits & Acknowledgements

- Based on the original by [botdemy (Instructables)](https://www.instructables.com/Connect-4-Game-Using-Arduino-and-Neopixel/)
- 7-segment TM1637 library by [Jason A. Cox](https://github.com/jasonacox/TM1637TinyDisplay)

---

## 📸 Screenshots

*(Add gameplay photos or wiring diagrams here if you want)*

---

> **Enjoy playing! Fork,**
