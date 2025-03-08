#include <SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL_image.h>
using namespace std;
// Kích thước cửa sổ và các thông số game
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE=40;
const int MAP_WIDTH=SCREEN_WIDTH/TILE_SIZE;
const int MAP_HEIGHT=SCREEN_HEIGHT/TILE_SIZE;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
class Game
{
    public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
}
Game()
{
    running=1;
    if(SDL_Init(SDL_Init_VIDEO)<0)
    {
        cerr<<"SDL could not initialize! SDL_Error: "<<SDL_GetError()<<"\n";
        running=0;
    }
    window=SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(!window)
    {
        cerr<<"SDL could not be created! SDL_Error: "<<SDL_GetError()<<"\n";
        running=0;
    }
    renderer=SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer)
    {
        cerr<<"SDL could not be created! SDL_Error: "<<SDL_GetError()<<"\n";
        running=0;
    }
}
void renderer()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i=1;i<MAP_HEIGHT-1;i++)
    {
        for(int j=1;j<=MAP_WIDTH-1;j++)
        {
            SDL_Rect tile={j*TILE_SIZE, i*TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderFillRect(renderer, &tile);
        }
    }
    SDL_RenderPresent(renderer);
}
void run()
{
    while(running)
    {
        render();
        SDL_Delay(16);
    }
}
Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    sdl_quit();
}
int main(int argc, char* argv[])
{
    Game game;
    if(game.running)
    {
        game.run();
    }
    return 0;
}





















