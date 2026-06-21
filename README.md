# OOP_PROJECT_SNOWBROS
Developed by Muhammad Rayyan Ahmed and Muhammad Anas Zahid. This Snow Bros remake is our OOP Spring 2026 project at FAST-NU Islamabad. Built with C++ and SFML, it implements inheritance and polymorphism to manage game physics, AI, and levels, showcasing clean software design in a classic arcade setting.

Here's teh full overview:


# Snow Bros – OOP Game Project


Snow Bros is a 2D arcade-style game developed as an **Object-Oriented Programming (OOP)** semester project. The game is inspired by the classic Snow Bros arcade game, where the player controls a snowman character, attacks enemies using snowballs, collects power-ups, defeats bosses, and progresses through different story-based levels.

This project was developed using **C++ and SFML**. SFML was used for graphics, window handling, sprites, animations, keyboard input, sound effects, and background music. The project also uses **C++ file handling** to save and manage leaderboard records.

The main purpose of this project is to apply core OOP concepts such as **classes, objects, encapsulation, inheritance, polymorphism, abstraction, composition, file handling, and modular programming** in a complete game-based application.

---

## Project Overview

The Snow Bros OOP project is a complete arcade-style game designed using object-oriented programming. The game includes multiple enemy types, animated characters, sound effects, background music, power-ups, bonus items, boss fights, multiplayer mode, story mode, leaderboard support, high-score saving, improved collision detection, and enhanced game physics.

The project is divided into separate classes and modules so that each part of the game performs a specific responsibility. This makes the code easier to understand, debug, maintain, and extend.

---

## Game Concept

In the game, the player controls a snowman who moves around the screen and attacks enemies using snowballs. The objective is to defeat enemies, collect bonus items, survive enemy attacks, complete story-based stages, defeat bosses, and achieve a high score.

The game becomes more challenging as the player progresses. Different enemy types, boss fights, power-ups, multiplayer gameplay, leaderboard tracking, and story mode make the project more engaging and complete.

---

## Features

### 1. Player Movement
The player can move, jump, and interact with the game environment using keyboard controls.

### 2. Snowball Attack System
The player can shoot snowballs to attack enemies. Snowballs interact with enemies through collision detection and game physics.

### 3. Different Enemy Types
The game includes multiple enemy types with different movement patterns, behaviors, and difficulty levels.

### 4. Sound Effects and Background Music
Sound effects and background music are implemented using SFML audio features to improve the arcade gameplay experience.

### 5. Animations
Animations are implemented for the player, enemies, snowballs, and other gameplay actions to make the game visually appealing.

### 6. High-Score Saving
The game includes high-score saving using C++ file handling, allowing scores to be stored and viewed later.

### 7. Leaderboard Support
A leaderboard system is implemented using file handling in C++. Player scores can be saved, loaded, and displayed to show ranking or performance records.

### 8. Multiplayer Mode
A multiplayer mode is included so that more than one player can participate in the game.

### 9. Power-Ups and Bonus Items
The game includes power-ups and bonus items that help the player during gameplay and increase the score.

### 10. Boss Fights
Boss fights are added to make the game more challenging and exciting.

### 11. Story Mode
Story mode is included to give the game a proper progression system and make the gameplay more meaningful.

### 12. Improved Collision Accuracy
Collision detection has been improved to make player, enemy, snowball, boss, power-up, and platform interactions more accurate.

### 13. Improved Game Physics
The game physics have been improved for smoother movement, jumping, snowball behavior, and object interaction.

### 14. Score System
The player earns points by defeating enemies, collecting bonus items, clearing levels, and progressing through the game.

### 15. Lives and Game Over System
The player has limited lives. When all lives are lost, the game ends and the game over screen is displayed.

### 16. Level System
The game includes levels with different enemies, obstacles, bosses, and increasing difficulty.

### 17. Game States
The project includes different game states such as menu, story mode, playing, pause, win, leaderboard, and game over.

---

## OOP Concepts Used

| OOP Concept | Usage in Project |
|---|---|
| Classes and Objects | Player, enemies, snowballs, bosses, power-ups, levels, leaderboard, and game objects are represented using classes and objects |
| Encapsulation | Data members and functions are grouped inside classes, and important variables are protected from direct access |
| Inheritance | Common properties of game objects are inherited by player, enemy, boss, snowball, and power-up classes |
| Polymorphism | Different objects perform their own versions of common functions such as move, update, draw, attack, and collide |
| Abstraction | Complex game logic is divided into simple functions and classes |
| Composition | The main game class manages objects such as players, enemies, bosses, snowballs, levels, sounds, leaderboard, and score |
| File Handling | Used to save and load high scores and leaderboard records |
| Modular Programming | The project is divided into multiple files and classes for better organization |

---

## Suggested Class Structure

The project may include the following main classes:

| Class Name | Responsibility |
|---|---|
| Game | Controls the main game loop, events, updates, rendering, and game states |
| Player | Handles player movement, lives, actions, shooting, and animations |
| Enemy | Handles enemy behavior, movement, collision, and defeat logic |
| Boss | Handles boss enemy behavior, health, attacks, and boss fight logic |
| Snowball | Represents the projectile fired by the player |
| PowerUp | Handles power-ups and bonus item effects |
| Level | Stores level layout, platforms, enemies, bosses, and difficulty |
| CollisionManager | Checks and handles collisions between game objects |
| ScoreManager | Stores and updates the player's score |
| Leaderboard | Saves, loads, and displays leaderboard records |
| FileManager | Handles file reading and writing for scores or leaderboard data |
| SoundManager | Handles sound effects and background music using SFML |
| AnimationManager | Manages animations for player, enemies, snowballs, and objects |
| StoryManager | Handles story mode progression and story screens |
| Menu | Displays the main menu and handles menu options |
| GameObject | Base class for common game object properties such as position, sprite, movement, and rendering |

---

## Technologies Used

| Technology | Purpose |
|---|---|
| C++ | Main programming language used for implementing the game and OOP concepts |
| SFML | Used for graphics, sprites, animations, window handling, audio, and keyboard input |
| OOP Concepts | Used to structure the game using classes and objects |
| SFML Graphics | Used for rendering the player, enemies, snowballs, levels, menus, and visual elements |
| SFML Audio | Used for sound effects and background music |
| Keyboard Input Handling | Used to control player movement, attacks, menu selection, and multiplayer controls |
| C++ File Handling | Used for saving and loading high scores and leaderboard data |

---

## How to Run the Project

1. Download or clone the project folder.
2. Open the project in your C++ IDE.
3. Make sure SFML is installed and correctly linked with the project.
4. Make sure all assets such as images, fonts, sounds, and music are placed in the correct folders.
5. Compile the source code.
6. Run the executable file.
7. Use the keyboard controls to play the game.

---

## Basic Controls

| Key | Action |
|---|---|
| Left Arrow / A | Move left |
| Right Arrow / D | Move right |
| Up Arrow / W | Jump or move up |
| Spacebar | Shoot snowball |
| Enter | Select menu option |
| Esc | Pause or exit |

For multiplayer mode, the second player may use a separate set of keys depending on the project implementation.

---

## Folder Structure

A possible folder structure for the project is:

```text
SnowBros-OOP/
│
├── src/
│   ├── main.cpp
│   ├── Game.cpp
│   ├── Player.cpp
│   ├── Enemy.cpp
│   ├── Boss.cpp
│   ├── Snowball.cpp
│   ├── PowerUp.cpp
│   ├── Level.cpp
│   ├── CollisionManager.cpp
│   ├── ScoreManager.cpp
│   ├── Leaderboard.cpp
│   ├── StoryManager.cpp
│   ├── SoundManager.cpp
│   └── FileManager.cpp
│
├── include/
│   ├── Game.h
│   ├── Player.h
│   ├── Enemy.h
│   ├── Boss.h
│   ├── Snowball.h
│   ├── PowerUp.h
│   ├── Level.h
│   ├── CollisionManager.h
│   ├── ScoreManager.h
│   ├── Leaderboard.h
│   ├── StoryManager.h
│   ├── SoundManager.h
│   └── FileManager.h
│
├── assets/
│   ├── images/
│   ├── sounds/
│   ├── music/
│   └── fonts/
│
├── data/
│   ├── highscores.txt
│   └── leaderboard.txt
│
└── README.md
```

The actual folder structure may vary depending on the implementation.

---

## Learning Outcomes

Through this project, the following concepts were practiced:

- Applying object-oriented programming in a complete game project
- Creating and managing multiple classes and objects
- Implementing inheritance and polymorphism in game entities
- Managing player, enemy, boss, power-up, and story mode behavior
- Using SFML for graphics, sprites, animations, audio, and input handling
- Handling animations, sounds, and background music
- Implementing collision detection and game physics
- Saving high scores and leaderboard data using C++ file handling
- Building multiplayer game logic
- Organizing code into separate modules
- Debugging object interaction and gameplay logic
- Creating a complete interactive game from scratch

---

## Challenges Faced

Some challenges faced during development included:

- Setting up and linking SFML correctly
- Managing collision detection between multiple moving objects
- Implementing different enemy behaviors
- Handling boss fight mechanics
- Adding multiplayer controls without conflicts
- Managing animations smoothly
- Connecting sound effects and background music with game events
- Saving and loading leaderboard data correctly through file handling
- Improving game physics for smooth movement and snowball interaction
- Organizing the project into proper OOP classes

---

## Future Improvements

Some possible future improvements are:

- Add more levels and maps
- Add more boss characters
- Add more advanced enemy AI
- Add online multiplayer support
- Add difficulty settings
- Add more visual effects and particle effects
- Improve user interface screens
- Add more story chapters
- Add more power-up variations
- Add achievements and unlockable characters

---

## Project Information

**Project Name:** Snow Bros  
**Course:** Object-Oriented Programming  
**Project Type:** 2D Arcade Game  
**Programming Language:** C++  
**Graphics Library:** SFML  
**Leaderboard System:** C++ File Handling  
**Main Concepts:** Classes, Objects, Inheritance, Polymorphism, Encapsulation, Abstraction, File Handling  
**Project Category:** Semester Project / Game Development Project  

---

## Conclusion

Snow Bros is a complete OOP-based arcade game project that demonstrates how object-oriented concepts can be used to build an interactive and feature-rich application. The project includes gameplay mechanics such as enemies, snowball attacks, boss fights, animations, sounds, multiplayer mode, story mode, power-ups, leaderboard support, high-score saving, and improved collision physics.

This project helped in understanding how real software is divided into classes, how objects interact with each other, and how game logic can be managed in a clean, reusable, and organized way.

This project is created for academic and learning purposes.

