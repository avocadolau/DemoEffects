#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <iostream>
#include <vector>
//#include <glad/glad.h>

//#include <imgui.h>
//#include <backends/imgui_impl_opengl3.h>
//#include <backends/imgui_impl_sdl2.h>

//#include <ImGuiBuild.h>
//
//#include <FontIcons/IconsForkAwesome.h>

SDL_Window* Window = nullptr;
//SDL_GLContext GLContext;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//The window we'll be rendering to
SDL_Window* window = NULL;
//The surface contained by the window
SDL_Surface* screenSurface = NULL;

#define FPS 60
int lastTime = 0, currentTime, deltaTime;
float msFrame = 1 / (FPS / 1000.0f);

#define RIPPLE_RADIUS 8
#define RIPPLE_STRENGTH 512

Uint32* pixels = NULL;
Uint32* background = NULL;

int* current = NULL;
int* previous = NULL;


bool initSDL();
void waitTime();

void initEffect();
void update();
void render();

void drop(int mx, int my);
void updateRipples();

void close();

int main(int argc, char* args[])
{
    //Initialize SDL
    if (!initSDL())
    {
        std::cout << "Failed to initialize!\n";
        return 1;
    }
    else
    {
        bool quit = false;
		// Event handler
        SDL_Event e;

        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else 
        {
            initEffect();

            while (!quit)
            {
                //Handle events on queue
                while (SDL_PollEvent(&e) != 0)
                {
                    if (e.type == SDL_KEYDOWN) {

                        // close window
                        if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                            quit = true;

                        // create drop
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            int dx, dy;
                            SDL_GetMouseState(&dx, &dy);
                            drop(dx, dy);
                        }
                        if (e.key.keysym.scancode == SDL_SCANCODE_E)
                        {
                            int dx, dy;
							SDL_GetMouseState(&dx, &dy);
                            drop(dx, dy);
                        }
                    }
                    //User requests quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }
				updateRipples();
				update();
				render();

                //Update the surface
                SDL_UpdateWindowSurface(window);

				waitTime();
            }
        }
    }

    close();

    return 0;
}

bool initSDL() {

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: %s\n" << SDL_GetError();
        return false;
    }
    //Create window
    window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        std::cout << "Window could not be created! SDL_Error: %s\n" << SDL_GetError();
        return false;
    }
    //Get window surface
    screenSurface = SDL_GetWindowSurface(window);
    return true;
}

void waitTime() {
    currentTime = SDL_GetTicks();
    deltaTime = currentTime - lastTime;
    if (deltaTime < (int)msFrame) {
        SDL_Delay((int)msFrame - deltaTime);
    }
    lastTime = currentTime;
}

void initEffect()
{
    // init surface
    screenSurface = SDL_GetWindowSurface(window);
    
    // init buffers
	current = new int[SCREEN_WIDTH * SCREEN_HEIGHT];
	previous = new int[SCREEN_WIDTH * SCREEN_HEIGHT];

    // Create checker
	background = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
    int tileSize = 32;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {

            int checkerX = x / tileSize;
            int checkerY = y / tileSize;

            bool isWhite = (checkerX + checkerY) % 2 == 0;

            Uint8 color = isWhite ? 255 : 0;

            background[y * SCREEN_WIDTH + x] =
                SDL_MapRGB(screenSurface->format, color, color, color);
        }
    }
}

void update()
{
    SDL_LockSurface(screenSurface);
    pixels = (Uint32*)screenSurface->pixels;

    for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 1; x < SCREEN_WIDTH - 1; x++) {

            int i = y * SCREEN_WIDTH + x;

			// calculate slope and divide by 8 to reduce the strength of the effect
            int offsetX = (previous[i - 1] - previous[i + 1]) >> 3;
            int offsetY = (previous[i - SCREEN_WIDTH] - previous[i + SCREEN_WIDTH]) >> 4;

            // diceplacement
            int sx = x + offsetX;
            int sy = y + offsetY;

			// clamp to screen bounds
            if (sx < 0) sx = 0;
            if (sx >= SCREEN_WIDTH) sx = SCREEN_WIDTH - 1;
            if (sy < 0) sy = 0;
            if (sy >= SCREEN_HEIGHT) sy = SCREEN_HEIGHT - 1;

            pixels[i] = background[sy * SCREEN_WIDTH + sx];
        }
    }
    SDL_UnlockSurface(screenSurface);

    //Fill the surface white
    //SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));


}

void render()
{

}

void drop(int mx, int my)
{
    // navigates drop area
    for (int y = -RIPPLE_RADIUS; y < RIPPLE_RADIUS; y++) {
        for (int x = -RIPPLE_RADIUS; x < RIPPLE_RADIUS; x++) {

            int dx = mx + x;
            int dy = my + y;

			// check screen bounds and circle bounds and sett initial strength
            if (dx > 0 && dx < SCREEN_WIDTH && dy > 0 && dy < SCREEN_HEIGHT)
                if(x * x + y * y <= RIPPLE_RADIUS * RIPPLE_RADIUS|| x * x + y * y >= (RIPPLE_RADIUS * RIPPLE_RADIUS)/4)
                    previous[dy * SCREEN_WIDTH + dx] = RIPPLE_STRENGTH;

        }
    }
}

void updateRipples()
{
    float damping = 0.99f;

    for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 1; x < SCREEN_WIDTH - 1; x++) {

            int i = y * SCREEN_WIDTH + x;
            
            // ripple equation u(t + 1) = 2u(t)−u(t−1) + ∇2u(t)
            current[i] = (
                previous[i - 1] +
                previous[i + 1] +
                previous[i - SCREEN_WIDTH] +
                previous[i + SCREEN_WIDTH]
                ) / 2 - current[i];

            current[i] *= damping;
        }
    }

    std::swap(current, previous);
}

void close()
{
	free(current);
	free(previous);
    //Destroy window
    SDL_DestroyWindow(window);

    //Quit SDL subsystems
    SDL_Quit();
}