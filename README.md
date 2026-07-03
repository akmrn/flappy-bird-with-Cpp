![player_image](https://github.com/[username]/[reponame]/blob/[branch]/image.jpg?raw=true](https://github.com/akmrn/flappy-bird-with-Cpp/blob/main/assets/plane.png)

1- System & Game Loop Architecture:<br />
**SDL3 Setup:** We configured and initialized the **SDL3** library and its image plugin (SDL_image) for window management, 2D graphics, and events.<br />
**Frame Rate Stabilization (VSync):**: We enabled VSync to prevent screen tearing and synchronize frames with the monitor refresh rate.<br />
**Cleanup Lifecycle:** We made the memory management system and hardware resource cleanup (such as textures, renderer, and window) standard when exiting the game to prevent memory leaks.<br />
2- Player and movement physics system:<br />
**Framerate Independent Physics:** We used the time elapsed between frames (delta) to make the player and the pipes move smoothly and evenly across systems with different processing speeds.<br />
**Gravity:** We applied gravity to the player, which gradually increased his vertical speed and pulled him downwards.<br />
**Jump Trigger:** We tracked user inputs via the Spacebar and left mouse clicks in the event loop and, by triggering a flag (jumpRequested), we made the player's vertical speed negative upwards to simulate a jump arc.<br />
**Texture Rendering:** We loaded the player's image (in .png format) into memory and rendered it on the screen as a floating rectangle (SDL_FRect).<br />
3- Obstacles:<br />
**Pipe data structure:** We defined an independent structure called struct Pipe which includes width, horizontal position, height of the top pipe, gap, and speed of movement to the left.<br />
**Spawn and horizontal movement:** The pipes move to the left at a constant speed on the X axis.<br />
**Respawn mechanism:** We implemented logic so that when the pipe completely leaves the left side of the screen, its horizontal position is immediately transferred to the far right side of the screen (win_W) to form an infinite loop of obstacles.<br />
**Geometric drawing of obstacles:** In each frame, we draw the top pipe and the bottom pipe as two solid green rectangles with mathematical calculations.<br />
