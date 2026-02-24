#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <iostream>

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

bool initSDL();

void initEffect();
void update();
void render();

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
                        if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                            quit = true;
                        }
                    }
                    //User requests quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }

				update();
				render();

                //Update the surface
                SDL_UpdateWindowSurface(window);
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

void initEffect()
{

}

void update()
{
    //Fill the surface white
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
}

void render()
{

}

void close()
{
    //Destroy window
    SDL_DestroyWindow(window);

    //Quit SDL subsystems
    SDL_Quit();
}