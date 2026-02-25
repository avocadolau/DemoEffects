#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
//#include <glad/glad.h>

//#include <imgui.h>
//#include <backends/imgui_impl_opengl3.h>
//#include <backends/imgui_impl_sdl2.h>

//#include <ImGuiBuild.h>
//
//#include <FontIcons/IconsForkAwesome.h>

//#define VECTORS_MODIFICATION

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

#define BLACK 0xFF000000
#define WHITE 0xFFFFFFFF

Uint32* pixels = NULL;
Uint32* background = NULL;

// strength buffers for ripple
int* current = NULL;
int* previous = NULL;
Uint32* currColors = NULL;
Uint32* prevColors = NULL;


// with vectors-----------------------------------
std::vector<int> currBuffer(SCREEN_WIDTH* SCREEN_HEIGHT);
std::vector<int> prevBuffer(SCREEN_WIDTH* SCREEN_HEIGHT);

std::vector<Uint32> backgroundBuffer;
std::vector<Uint32*> pixelsBuffer(SCREEN_WIDTH* SCREEN_HEIGHT);
//-------------------------------------------------------

Mix_Music* mySong;
#define BPM_MUSIC 128
#define MSEG_BPM (60000 / BPM_MUSIC)
#define FLASH_MAX_TIME 300
int flashtime;
int MusicCurrentTime;
int MusicCurrentTimeBeat;
int MusicCurrentBeat;
int MusicPreviousBeat;

bool initSDL();
void waitTime();

void initEffect();
void distorsion();
void render();

void drop(int mx, int my);
void updateRipples();
Uint32 blendColorsNoBlack(Uint32 c1, Uint32 c2, float a);
Uint32 blendColors(Uint32 c1, Uint32 c2, float a);

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
				if (currentTime % 100 == 0) drop(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
				updateRipples();
				distorsion();
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
    
#ifndef VECTORS_MODIFICATION
    // init buffers
    current = new int[SCREEN_WIDTH * SCREEN_HEIGHT];
    previous = new int[SCREEN_WIDTH * SCREEN_HEIGHT];
    currColors = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
    prevColors = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];

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

            currColors[y * SCREEN_WIDTH + x] = BLACK;
            prevColors[y * SCREEN_WIDTH + x] = BLACK;
        }
    }
#else
	int tileSize = 32;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {

            int checkerX = x / tileSize;
            int checkerY = y / tileSize;

            bool isWhite = (checkerX + checkerY) % 2 == 0;

            Uint8 color = isWhite ? 255 : 0;

			backgroundBuffer.push_back(SDL_MapRGB(screenSurface->format, color, color, color));
        }
    }
    int i = 0;
    Uint32* pixels = (Uint32*)screenSurface->pixels;
    for (auto& p : pixelsBuffer)
    {
        p = &pixels[i];
        i++;
    }
#endif // !VECTORS_MODIFICATION

    

    

    // init audio
    /*Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Init(MIX_INIT_MP3);
    mySong = Mix_LoadMUS("Project/resources/PhilCollins_InTheAirTonight.mp3");
    if (!mySong) {
        std::cout << "Error loading Music: " << Mix_GetError() << std::endl;
        close();
        exit(1);
    }*/
}

void distorsion()
{
    float a=0.5f;

    pixels = (Uint32*)screenSurface->pixels;
    int dCoords;
    Uint32 r = 0;
    Uint32 g = 0;
    Uint32 b = 0;
    // iterate all pixels except borders
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
            dCoords = sy * SCREEN_WIDTH + sx;
            Uint32 dPixel = background[dCoords];
            
            if (abs(offsetX) + abs(offsetY)>1)
            {
                a = 1.0f / (abs(offsetX) + abs(offsetY)/2);
				if (a < 0.3f) a = 0.3f;
            }
            else a = 1.0f;

			//if (a <0.5f) a = 0.5f;
            
            //setting the pixel color from displaced coords from background
            //Uint32 blendedC = blend
            pixels[i] = blendColors(background[dCoords],prevColors[i], a);
            //pixels[i] = prevColors[i];

			//pixels[i] = background[dCoords];
        }
    }

}

void render()
{

}

void drop(int mx, int my)
{
    Uint32 c= SDL_MapRGB(screenSurface->format, rand() % 255, rand() % 255, rand() % 255);

    // navigates drop area
    for (int y = -RIPPLE_RADIUS; y < RIPPLE_RADIUS; y++) {
        for (int x = -RIPPLE_RADIUS; x < RIPPLE_RADIUS; x++) {

            int dx = mx + x;
            int dy = my + y;

            // check screen bounds and circle bounds and sett initial strength
            if (dx > 0 && dx < SCREEN_WIDTH && dy > 0 && dy < SCREEN_HEIGHT)
                if (x * x + y * y <= RIPPLE_RADIUS * RIPPLE_RADIUS || x * x + y * y >= (RIPPLE_RADIUS * RIPPLE_RADIUS) / 4)
                {
                    previous[dy * SCREEN_WIDTH + dx] = RIPPLE_STRENGTH;
                    prevColors[dy * SCREEN_WIDTH + dx] = c;
                }
        }
    }

}

void updateRipples()
{
    float damping = 0.99f;

    for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {

        int* prevRow = &previous[y * SCREEN_WIDTH];
        int* prevRowUp = &previous[(y - 1) * SCREEN_WIDTH];
        int* prevRowDown = &previous[(y + 1) * SCREEN_WIDTH];
        int* currRow = &current[y * SCREEN_WIDTH];

        Uint32* prevRowC = &prevColors[y * SCREEN_WIDTH];
        Uint32* prevRowUpC = &prevColors[(y - 1) * SCREEN_WIDTH];
        Uint32* prevRowDownC = &prevColors[(y + 1) * SCREEN_WIDTH];
        Uint32* currRowC = &currColors[y * SCREEN_WIDTH];

        for (int x = 1; x < SCREEN_WIDTH - 1; x++) {
            currRow[x] = ((
                prevRow[x - 1] + prevRow[x + 1] +
                prevRowUp[x] + prevRowDown[x]
                ) >> 1) - currRow[x];

            currRow[x] = (int)(currRow[x] * damping);

            if (currRow[x] == 0)
                currRowC[x] = BLACK;
            else
                currRowC[x] = blendColorsNoBlack(blendColorsNoBlack(prevRowC[x - 1], prevRowC[x + 1],0.5f), blendColorsNoBlack(prevRowUpC[x], prevRowDownC[x], 0.5f),0.5f);
                
        }
    }

    std::swap(current, previous);
    std::swap(currColors, prevColors);
}

Uint32 blendColorsNoBlack(Uint32 c1, Uint32 c2, float a)
{
    if (c1 == BLACK)
    {
        if (c2 == BLACK)
            return BLACK;
        else
            return c2;
    }
    else if (c2 == BLACK)
    {
        return c1;
    }
    else
    {
        return blendColors(c1, c2, a);
    }



    return BLACK;
}

Uint32 blendColors(Uint32 c1, Uint32 c2, float a)
{
    Uint8 r = (c1 >> 16 & 0xFF) * a + (c2 >> 16 & 0xFF) * (1 - a);
    Uint8 g = (c1 >> 8 & 0xFF) * a + (c2 >> 8 & 0xFF) * (1 - a);
    Uint8 b = (c1 & 0xFF) * a + (c2 & 0xFF) * (1 - a);
    return SDL_MapRGB(screenSurface->format, r, g, b);
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