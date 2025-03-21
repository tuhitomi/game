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
class wall
{
public:
    int x,y;
    SDL_Rect rect;
    bool active;
    wall(int startx,int starty)
    {
        x=startx;
        y=starty;
        active=1;
            rect={x,y,TILE_SIZE, TILE_SIZE};
    }
    void render(SDL_Renderer* renderer)
    {
        if(active)
        {
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};
class bullet
{
public:
    int x,y;
    int dx,dy;
    SDL_Rect rect;
    bool active;
    bullet(int startx,int starty,int dirx,int diry)
    {
        x=startx;
        y=starty;
        dx=dirx;
        dy=diry;
        active=1;
        rect={x,y,10,10};
    }
    void move()
    {
        x=x+dx;
        y=y+dy;
        rect.x=x;
        rect.y=y;
        if(x<TILE_SIZE || x>SCREEN_WIDTH-TILE_SIZE || y<TILE_SIZE || y>SCREEN_HEIGHT-TILE_SIZE)
        {
            active=0;
        }
    }
    void render(SDL_Renderer* renderer)
    {
        if(active)
        {
            SDL_SetRenderDrawColor(renderer,255,255,255,255);
            SDL_RenderFillRect(renderer,&rect);
        }
    }
};
class playertank
{
public:
    int x,y;
    int dirx,diry;
    SDL_Rect rect;
    vector<bullet> bullets;
    void shoot()
    {
        bullets.push_back(bullet(x+TILE_SIZE/2-5,y+TILE_SIZE/2-5,this->dirx,this->diry));
    }
    void updatebullets()
    {
        for(auto &bullet:bullets)
        {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](bullet &b){return !b.active;}),bullets.end());
    }
    playertank(int startx, int starty)
    {
        x=startx;
        y=starty;
        rect={x,y,TILE_SIZE,TILE_SIZE};
        dirx=0;
        diry=-1;
    }
    void move(int dx,int dy,const vector<wall>& walls)
    {
        int newx=x+dx;
        int newy=y+dy;
        this->dirx=dx;
        this->diry=dy;
        SDL_Rect newrect={newx,newy,TILE_SIZE,TILE_SIZE};
        for(int i=0;i<walls.size();i++)
        {
            if(walls[i].active and SDL_HasIntersection(&newrect, &walls[i].rect))
            {
                return;
            }
        }
        if(newx>=TILE_SIZE and newx<=SCREEN_WIDTH-TILE_SIZE*2 and newy>=TILE_SIZE and newy<=SCREEN_HEIGHT-TILE_SIZE*2)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
    }
    void render(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer,255,255,0,255);
        SDL_RenderFillRect(renderer,&rect);
        for(auto& bullet:bullets)
        {
            bullet.render(renderer);
        }

    }
};
class enemytank
{
public:
    int x,y;
    int dirx,diry;
    int movedelay,shootdelay;
    SDL_Rect rect;
    bool active;
    vector<bullet> bullets;
    enemytank(int startx, int starty)
    {
        movedelay=15;
        shootdelay=5;
        x=startx;
        y=starty;
        rect={x,y,TILE_SIZE,TILE_SIZE};
        dirx=0;
        diry=1;
        active=1;
    }
    void move(const vector<wall>& walls)
    {
        if(--movedelay>0) return;
        movedelay=15;
        int r=rand()%4;
        //up
        if(r==0)
        {
            this->dirx=0;
            this->diry=-5;
        }
        //down
        else if(r==1)
        {
            this->dirx=0;
            this->diry=5;
        }
        //left
        else if(r==2)
        {
            this->dirx=-5;
            this->diry=0;
        }
        //right
        else
        {
            this->dirx=5;
            this->diry=0;
        }
        int newx=x+this->dirx;
        int newy=y+this->diry;
        SDL_Rect newrect={newx,newy,TILE_SIZE,TILE_SIZE};
        for(int i=0;i<walls.size();i++)
        {
            if(walls[i].active and SDL_HasIntersection(&newrect, &walls[i].rect))
            {
                return;
            }
        }
        if(newx>=TILE_SIZE and newx<=SCREEN_WIDTH-TILE_SIZE*2 and newy>=TILE_SIZE and newy<=SCREEN_HEIGHT-TILE_SIZE*2)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
    }
    void shoot()
    {
        if(--shootdelay>0) return;
        shootdelay=5;
        bullets.push_back(bullet(x+TILE_SIZE/2-5,y+TILE_SIZE/2-5,this->dirx,this->diry));
    }
    void updatebullets()
    {
        for(auto &bullet:bullets)
        {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](bullet &b){return !b.active;}),bullets.end());
    }
    void render(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer,255,255,0,255);
        SDL_RenderFillRect(renderer,&rect);
        for(auto& bullet:bullets)
        {
            bullet.render(renderer);
        }
    }
};
class Game
{
    public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    bool pause;
    vector<wall> walls;
    playertank player;
    int enemynumber=3;
    vector<enemytank> enemies;
    void generatewalls()
    {
        for(int i=3;i<MAP_HEIGHT-3;i+=2)
        {
            for(int j=3;j<MAP_WIDTH-3;j+=2)
            {
                wall w=wall(j*TILE_SIZE,i*TILE_SIZE);
                walls.push_back(w);
            }
        }
    }
    Game(): player(((MAP_WIDTH-1)/2)*TILE_SIZE, (MAP_HEIGHT-2)*TILE_SIZE)
    {
        running=1;
        if(SDL_Init(SDL_INIT_VIDEO)<0)
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
        generatewalls();
        spawnenemies();
    }
    //enemiestank
    void spawnenemies()
    {
        enemies.clear();
        for(int i=0;i<enemynumber;i++)
        {
            int ex,ey;
            bool validposition=false;
            while(!validposition)
            {
                ex=(rand()%(MAP_WIDTH-2)+1)*TILE_SIZE;
                ey=(rand()%(MAP_HEIGHT-2)+1)*TILE_SIZE;
                validposition=1;
                for(const auto& wall:walls)
                {
                    if(wall.active && wall.x==ex && wall.y==ey)
                    {
                        validposition=0;
                        break;
                    }
                }
            }
            enemies.push_back(enemytank(ex,ey));
        }
    }
    //draw
    void render()
    {
        //window
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for(int i=1;i<MAP_HEIGHT-1;i++)
        {
            for(int j=1;j<MAP_WIDTH-1;j++)
            {
                SDL_Rect tile={j*TILE_SIZE, i*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect(renderer, &tile);
            }
        }
        //wall
        for(int i=0;i<walls.size();i++)
        {
            walls[i].render(renderer);
        }
        //player
        player.render(renderer);
        for(auto& enemy:enemies)
        {
            enemy.render(renderer);
        }
        SDL_RenderPresent(renderer);
    }
    //palyertank move
    void handle()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type==SDL_QUIT)
            {
                running=0;
            }
            else if(event.type==SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP: player.move(0,-5,walls);break;
                    case SDLK_DOWN: player.move(0,5,walls);break;
                    case SDLK_LEFT: player.move(-5,0,walls);break;
                    case SDLK_RIGHT: player.move(5,0,walls);break;
                    case SDLK_SPACE: player.shoot();break;
                }
            }
        }
    }
    void update()
    {
        player.updatebullets();
        for(auto &bullet:player.bullets)
        {
            for(auto& wall:walls)
            {
                if(wall.active && SDL_HasIntersection(&bullet.rect,&wall.rect))
                {
                    wall.active=0;
                    bullet.active=0;
                    break;
                }
            }
        }
        for(auto &bullet:player.bullets)
        {
            for(auto& enemy:enemies)
            {
                if(enemy.active && SDL_HasIntersection(&bullet.rect,&enemy.rect))
                {
                    enemy.active=0;
                    bullet.active=0;
                    break;
                }
            }
        }
        for(auto& enemy:enemies)
        {
            enemy.move(walls);
            enemy.updatebullets();
            if(rand()%100<2)
            {
                enemy.shoot();
            }
        }
        for(auto& enemy:enemies)
        {
            for(auto& bullet:enemy.bullets)
            {
                for(auto& wall:walls)
                {
                    if(wall.active && SDL_HasIntersection(&bullet.rect,&wall.rect))
                    {
                        wall.active=0;
                        bullet.active=0;
                        break;
                    }
                }
            }
        }
        enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](enemytank &e){return !e.active;}),enemies.end());
        if(enemies.empty())
        {
            running=0;
        }
        for(auto& enemy:enemies)
        {
            for(auto& bullet:enemy.bullets)
            {
                if(SDL_HasIntersection(&bullet.rect,&player.rect))
                {
                    running=0;
                    return ;
                }
            }
        }
    }
    void run()
    {
        while(running)
        {
            handle();
            render();
            update();
            SDL_Delay(16 );
        }
    }
    ~Game()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

};
int main(int argc, char* argv[])
{
    Game game;
    if(game.running)
    {
        game.run();
    }
    return 0;
}





















