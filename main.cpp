#include <SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL_image.h>
#include <fstream>
#include<cmath>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
using namespace std;
// Kích thước cửa sổ và các thông số game
const int SCREEN_WIDTH = 840;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE=20;
const int MAP_WIDTH=SCREEN_WIDTH/TILE_SIZE;
const int MAP_HEIGHT=SCREEN_HEIGHT/TILE_SIZE;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
const Uint8* keystates = SDL_GetKeyboardState(NULL);
int mapp[50][50];  // Khởi tạo map
class wall
{
public:
    int x,y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;
    wall(int startx, int starty, SDL_Renderer* renderer, const string& imagePath)
    {
        x = startx;
        y = starty;
        //active = 1;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        texture = loadTexture(imagePath, renderer);  // Tải texture từ ảnh
    }
    // Hàm tải ảnh thành texture
    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "Không thể tải ảnh: " << IMG_GetError() << endl;
            return nullptr;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture)
        {
            cerr << "Không thể tạo texture: " << SDL_GetError() << endl;
        }
        return texture;
    }

    // Hàm vẽ tường
    void render(SDL_Renderer* renderer)
    {
        if (active && texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);  // Vẽ ảnh lên màn hình
        }
    }
};
class DestructibleWall : public wall
{
public:
    DestructibleWall(int startx, int starty, SDL_Renderer* renderer, const string& imagePath) : wall(startx,starty,renderer,imagePath){}
};
class Buff
{
public:
    enum BuffType { AMMO, LIFE };
    int x, y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;
    BuffType type;


    Buff(int startx, int starty, SDL_Renderer* renderer, const string& imagePath, BuffType buffType) :
    x(startx), y(starty), active(true), type(buffType)
    {
        rect = { x, y, TILE_SIZE, TILE_SIZE };
        texture = loadTexture(imagePath, renderer);
    }
    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "Không thể tải ảnh: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cerr << "Không thể tạo texture: " << SDL_GetError() << endl;
        }
        return texture;
    }

    void render(SDL_Renderer* renderer)
    {
        if (active && texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
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
    SDL_Texture* texture;  // Thêm thuộc tính texture để lưu hình ảnh đạn

    // Khởi tạo đạn với hình ảnh
    bullet(int startx, int starty, int dirx, int diry, SDL_Renderer* renderer, const string& texturePath)
    {
        x = startx;
        y = starty;
        dx = dirx;
        dy = diry;
        active = 1;
        rect = {x, y, 10, 10};  // Kích thước mặc định của đạn
        texture = loadTexture(renderer, texturePath);  // Tải hình ảnh đạn
    }

    // Hàm tải texture
    SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& path)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "khong the tai anh: " << IMG_GetError();
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cerr << "khong the tao texture: " << SDL_GetError();
        }
        return texture;
    }
    void move()
    {
        x=x+dx/4;
        y=y+dy/4;
        rect.x=x;
        rect.y=y;
        if(x<=10 || x>=SCREEN_WIDTH || y<=10 || y>=SCREEN_HEIGHT)
        {
            active=0;
        }
    }
    // Render đạn
    void render(SDL_Renderer* renderer)
    {
        if (active)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);  // Vẽ hình ảnh đạn
        }
    }
};
class playertank
{
public:
    //sound
    Mix_Chunk *shootSound = nullptr;
    Mix_Chunk *hurtSound = nullptr;
    // Animation
    SDL_Texture* spriteSheet = nullptr;
    int currentFrame = 0;
    Uint32 lastFrameTime = 0;
    int frameDelay = 100;
    int currentDirection = 1; // 0: trái, 1: phải, 2: dưới, 3: trên
    int frameWidth;
    int frameHeight;
    SDL_Renderer* pRenderer=nullptr; // Lưu trữ renderer
    int lives = 3;
    int ammo = 40;
    int currentAmmo = ammo;
    bool isShooting = false;
    int x,y;
    int dirx,diry;
    SDL_Rect rect;
    SDL_Texture* texture;  // Thêm texture cho xe tăng
    vector<bullet> bullets;
    void shoot(SDL_Renderer* renderer, const string& texturePath)
    {
        if (currentAmmo > 0 && isShooting==true)
        {
            Mix_PlayChannel(-1, shootSound, 0);
            bullets.push_back(bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, this->dirx, this->diry, renderer, texturePath));
            currentAmmo--;
            isShooting = false; // Đặt lại cờ isShooting sau khi bắn

        }
    }
    void updatebullets()
    {
        for(auto &bullet:bullets)
        {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](bullet &b){return !b.active;}),bullets.end());
    }
    playertank(int startx, int starty, SDL_Renderer* renderer)
    {
        x = startx;
        y = starty;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirx = 0;
        diry = -1;
        if (pRenderer)
        {
            //spriteSheet = loadTexture("player_move.png", pRenderer);
            if (spriteSheet)
            {
                SDL_QueryTexture(spriteSheet, NULL, NULL, &frameWidth, &frameHeight);
                frameWidth /= 6;
                frameHeight /= 4;
            }
        }
        //texture = loadTexture(imagePath, renderer);  // Tải texture cho xe tăng
        shootSound = Mix_LoadWAV("playershoot.wav");
        hurtSound = Mix_LoadWAV("hurt.wav");
        lastFrameTime = SDL_GetTicks();
    }
    // Hàm tải ảnh thành texture
    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "Không thể tải ảnh: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }
    void move(int dx,int dy,const vector<wall>& walls,const vector<DestructibleWall>& destructibleWalls)
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
        for(int i=0;i<destructibleWalls.size();i++)
        {
            if(destructibleWalls[i].active and SDL_HasIntersection(&newrect, &destructibleWalls[i].rect))
            {
                return;
            }
        }
        if(newx>=0 and newx<=SCREEN_WIDTH and newy>=0 and newy<=SCREEN_HEIGHT)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
        if (dx < 0)
            currentDirection = 0; // Trái
        else if (dx > 0)
            currentDirection = 1; // Phải
        else if (dy > 0)
            currentDirection = 2; // Dưới
        else if (dy < 0)
            currentDirection = 3; // Lên
    }
    void update()
    {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime >= frameDelay)
        {
            currentFrame = (currentFrame + 1) % 2; // 6 frames mỗi hướng
            lastFrameTime = currentTime;
        }
    }
    void render(SDL_Renderer* renderer)
    {
       if (spriteSheet)
        {
            SDL_Rect srcRect = {currentFrame * frameWidth, currentDirection * frameHeight, frameWidth, frameHeight};
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &rect);
        } else { // Vẽ hình chữ nhật dự phòng nếu không có sprite sheet
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
        for(auto& bullet:bullets)
        {
            bullet.render(renderer);
        }
    }
    void handleInput(SDL_Event &e) {
       // Xử lý sự kiện cho player (di chuyển, bắn, ...)

        if (e.type == SDL_KEYDOWN) {

             if (e.key.keysym.sym==SDLK_SPACE)
            {
                isShooting = true;
            }



        } else if (e.type == SDL_KEYUP) {
              if (e.key.keysym.sym==SDLK_SPACE)
            {
               isShooting = false;

            }

        }
    }
    void playhurtSound()
    {
        Mix_PlayChannel(-1, hurtSound, 0);
    }
};
class Boss
{
private:
    // Animation
    int currentFrame = 0;
    Uint32 lastFrameTime = 0;
    int frameDelay = 300;

public:
    SDL_Rect healthBarRect;            // Rect cho thanh máu
    SDL_Color healthColor;
    int currentDirection = 0;
    int frameWidth;
    int frameHeight;
    SDL_Renderer *pRenderer=nullptr;
    SDL_Texture *spriteSheet = nullptr; //Thêm cho boss animation.
    int x, y;
    int dirx,diry;
    int health = 10;
    int movedelay;
    SDL_Rect rect;
    SDL_Texture* texture;
    vector<bullet> boss_bullets;
    int shootDelay = 187; // 3 giây (3s * 60fps = 180 frames)
    Boss(int startx, int starty, SDL_Renderer* renderer, const string& imagePath) : x(startx), y(starty)
    {
        pRenderer = renderer;
        if (pRenderer)
        {
            spriteSheet = loadTexture("boss.png", pRenderer);
            if (!spriteSheet)
            {
               cerr<<"Không thể tải spriteSheet boss: "<< IMG_GetError() << endl;
            }
            else
            {
                SDL_QueryTexture(spriteSheet, NULL, NULL, &frameWidth, &frameHeight);
                frameWidth /= 24;
                frameHeight /= 1; // Chỉ có 1 hàng, 24 frame
            }
        }
        rect = { x, y, 30, 30};
        healthBarRect.x = x;
        healthBarRect.y = y - 10;//vị trí healthbar so với boss
        healthBarRect.w = rect.w;
        healthBarRect.h = 5;
        texture = loadTexture(imagePath, renderer);
        lastFrameTime = SDL_GetTicks();
    }
    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "Không thể tải ảnh: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }
    void update(SDL_Renderer *renderer)
    {
        Uint32 currentTime = SDL_GetTicks();
        if(currentTime - lastFrameTime >=frameDelay)
        {
            currentFrame= (currentFrame+1)%10; //Cho boss animation
            lastFrameTime = currentTime;
        }
        shootDelay--;
        if (shootDelay <= 0)
        {
            shootDelay = 150; // Reset shootDelay
            // Bắn đạn tản ra xung quanh
            boss_bullets.push_back(bullet(x + rect.w / 2 - 5, y + rect.h / 2 - 5, 0, -5, renderer, "boss_bullet.png")); // Lên
            boss_bullets.push_back(bullet(x + rect.w / 2 - 5, y + rect.h / 2 - 5, 0, 5, renderer, "boss_bullet.png")); // Xuống
            boss_bullets.push_back(bullet(x + rect.w / 2 - 5, y + rect.h / 2 - 5, -5, 0, renderer, "boss_bullet.png")); // Trái
            boss_bullets.push_back(bullet(x + rect.w / 2 - 5, y + rect.h / 2 - 5, 5, 0, renderer, "boss_bullet.png")); // Phải
        }
        //update bullets
        for (auto &bullet : boss_bullets)
        {
            bullet.move();
        }
        boss_bullets.erase(std::remove_if(boss_bullets.begin(),boss_bullets.end(),[](bullet &b){return !b.active;}),boss_bullets.end());
    }
    void move(const vector<wall>& walls, const playertank& player, SDL_Renderer* renderer)
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
        if(newx>=0 and newx<=SCREEN_WIDTH and newy>=0 and newy<=SCREEN_HEIGHT)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
    }
    void render(SDL_Renderer *renderer)
    {
        if (spriteSheet)
        {
            SDL_Rect srcRect = { currentFrame * frameWidth, currentDirection * frameHeight, frameWidth, frameHeight };
            SDL_RenderCopy(renderer, spriteSheet, &srcRect, &rect);
        }
        else if(texture)
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect); //Vẽ texture thường nếu ko có spriteSheet
        }
        SDL_SetRenderDrawColor(renderer,0,0,0,0);
        //Vẽ thanh máu
        if (health > 6)
        {
            healthColor = {0,255,0};
        }
        else if (health >3)
        {
                healthColor = {255,255,0};
        }
        else healthColor= {255,0,0};
        SDL_SetRenderDrawColor(renderer, healthColor.r, healthColor.g, healthColor.b, 255);
        healthBarRect.w = rect.w * (static_cast<double>(health) / 10);
        healthBarRect.x = rect.x;
        healthBarRect.y=rect.y-10;
        SDL_RenderFillRect(renderer, &healthBarRect);
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderDrawRect(renderer,&healthBarRect);
        for (auto& bullet:boss_bullets)
        {
            bullet.render(renderer);
        }

    }
};
class enemytank
{
public:
    //sound
    Mix_Chunk* hit = nullptr;
    // Animation
    SDL_Texture* spriteSheet = nullptr;
    int currentFrame = 0;
    Uint32 lastFrameTime = 0;
    int frameDelay = 100; // Điều chỉnh giá trị này nếu cần
    int currentDirection = 0; // 0: trái, 1: phải, 2: dưới, 3: trên
    int frameWidth;
    int frameHeight;
    SDL_Renderer *pRenderer;
    int x,y;
    int dirx,diry;
    int movedelay,shootdelay;
    SDL_Rect rect;
    bool active;
    vector<bullet> bullets;
    SDL_Texture* texture;  // Thêm texture cho xe tăng kẻ thù

    enemytank(int startx, int starty, SDL_Renderer* renderer)
    {
        movedelay=15;
        shootdelay=5;
        x = startx;
        y = starty;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirx = 0;
        diry = -1;
        active = true;
        pRenderer=renderer;
        if (pRenderer)
        {
            spriteSheet = loadTexture("enemy_move.png", pRenderer);
            if (!spriteSheet)
            {
                cerr << "Không thể tải sprite sheet enemy: " << IMG_GetError() << endl;
            }
            else
            {
                SDL_QueryTexture(spriteSheet, NULL, NULL, &frameWidth, &frameHeight);
                frameWidth /= 6; // 6 frame mỗi hàng
                frameHeight /= 4; // 4 hàng
            }
            //texture = loadTexture(imagePath, renderer);
        }
        hit = Mix_LoadWAV("hit.wav");
        lastFrameTime = SDL_GetTicks();
    }
    void update()
    {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime >= frameDelay)
        {
            currentFrame = (currentFrame + 1) % 6;
            lastFrameTime = currentTime;
        }
    }
    // Hàm tải ảnh thành texture
    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "Không thể tải ảnh: " << IMG_GetError() << endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cerr << "Không thể tạo texture: " << SDL_GetError() << endl;
        }

        return texture;
    }
    void move(const vector<wall>& walls, const playertank& player, SDL_Renderer* renderer)
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
        if(newx>=0 and newx<=SCREEN_WIDTH and newy>=0 and newy<=SCREEN_HEIGHT)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
         // Cập nhật hướng di chuyển cho animation
        if (dirx < 0)
            currentDirection = 0; // Trái
        else if (dirx > 0)
            currentDirection = 1; // Phải
        else if (diry > 0)
            currentDirection = 2; // Dưới
        else if (diry < 0)
            currentDirection = 3; // Lên
    }
    void shoot(SDL_Renderer* renderer, const string& bulletTexturePath)
    {
        //cout<<shootdelay<<"\n";
        if (--shootdelay > 0) return;
        shootdelay = 5;
        // Tạo đạn cho kẻ thù, sử dụng hàm tải texture cho đạn của kẻ thù
        bullets.push_back(bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, this->dirx, this->diry, renderer, bulletTexturePath));
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
         if (spriteSheet)
         {
            SDL_Rect srcRect = {currentFrame * frameWidth, currentDirection * frameHeight, frameWidth, frameHeight};
             SDL_RenderCopy(renderer, spriteSheet, &srcRect, &rect);

         }
         else if (texture) { // Vẽ texture dự phòng nếu không có sprite sheet
             SDL_RenderCopy(renderer, texture, NULL, &rect);
         }
        for(auto& bullet:bullets)
        {
            bullet.render(renderer);
        }
    }
    void playerhit()
    {
        Mix_PlayChannel(-1, hit, 0);
    }
};
class Game
{
    private:
    TTF_Font* gFont;
    SDL_Rect playRect, exitRect, introRect; // Rect cho các chữ trong menu
    SDL_Texture *playTexture, *exitTexture, *introTexture;
    SDL_Texture* backButtonTexture = nullptr; // Texture của nút Back
    SDL_Rect backButtonRect;          // Rect của nút Back
    SDL_Texture* introImage = nullptr; // Texture hình ảnh hướng dẫn
    SDL_Rect introImageRect;   // Rect của hình ảnh
    TTF_Font* livesnanmoFont;
    SDL_Texture* ammoTexture = nullptr;
    SDL_Texture* livesTexture = nullptr;
    SDL_Rect ammoRect;
    SDL_Rect livesRect;
    vector<Buff> buffs;
    vector<DestructibleWall> destructibleWalls;
    SDL_Texture* pauseBackground = nullptr;
    SDL_Rect pauseBackgroundrect;
    SDL_Texture* pauseButtonTexture = nullptr;
    SDL_Rect pauseButtonRect;
    SDL_Texture* resumeButtonTexture = nullptr;
    SDL_Rect resumeButtonRect;
    SDL_Texture* menuButtonTexture = nullptr;    // Texture cho nút Menu
    SDL_Rect menuButtonRect;        // Rect cho nút Menu
    bool gamePaused = 0;
    public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    bool inmenu;
    vector<wall> walls;
    playertank player;
    Boss* boss=nullptr;
    int enemynumber=5;
    vector<enemytank> enemies;
    void generatewalls()
    {
        for(int i=0;i<28;i++)
        {
            for(int j=0;j<42;j++)
            {
                if(mapp[i][j]==0)
                {
                    wall w = wall(j * TILE_SIZE, 40+i * TILE_SIZE, renderer, "wall.png");
                    walls.push_back(w);
                }
                if(mapp[i][j]==4)
                {
                     destructibleWalls.push_back(DestructibleWall(j * TILE_SIZE, 40+i * TILE_SIZE, renderer, "breakable_wall.png"));
                }
            }
        }
    }
    //hàm để upload ảnh
    SDL_Texture* loadTexture(const string& path)
    {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface)
        {
            cerr << "khong the tai anh: " << IMG_GetError();
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            cerr << "khong the tao texture: " << SDL_GetError();
        }
        return texture;
    }

    Game():player(0, 80, nullptr),boss(nullptr)
    {
        running=1;
        gamePaused=0;
        inmenu=1;
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

        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        {
            cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() <<"\n";
            running = 0;
        }
        //player.texture = player.loadTexture("player_tank.png", renderer); // Tải texture dự phòng
        player.spriteSheet = player.loadTexture("player_move.png", renderer); // Tải sprite sheet
        if (player.spriteSheet)
        {
            SDL_QueryTexture(player.spriteSheet, NULL, NULL, &player.frameWidth, &player.frameHeight);
            player.frameWidth /= 6;
            player.frameHeight /= 4;
        }
        else
        {
             cerr << "Không thể tải sprite sheet: " << IMG_GetError() << endl;

        }
        // Khởi tạo SDL_ttf
        if (TTF_Init() == -1)
        {
            cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << "\n";
            running = false;
            return; // Hoặc xử lý lỗi theo cách khác
        }
        gFont = TTF_OpenFont("menufont.ttf", 50);  // Tải font
        if (gFont == NULL) {
            cerr << "Failed to load lazy font! SDL_ttf Error: " << TTF_GetError() << "\n";
            running = 0;
            return;
        }
        livesnanmoFont = TTF_OpenFont("normal.ttf", 10);  // Tải font
        if (livesnanmoFont == NULL) {
            cerr << "Failed to load lazy font! SDL_ttf Error: " << TTF_GetError() << "\n";
            running = 0;
            return;
        }
        // Tải texture cho chữ PLAY
        SDL_Color textColor = { 255, 255, 255 }; // Màu trắng
        SDL_Surface* playSurface = TTF_RenderText_Solid(gFont, "PLAY", textColor);
        SDL_Surface* exitSurface = TTF_RenderText_Solid(gFont, "EXIT", textColor);
        SDL_Surface* introSurface = TTF_RenderText_Solid(gFont, "INTRODUCTION", textColor);
        if (playSurface&&exitSurface&&introSurface)
        {
            playTexture=SDL_CreateTextureFromSurface(renderer,playSurface);
            exitTexture=SDL_CreateTextureFromSurface(renderer,exitSurface);
            introTexture=SDL_CreateTextureFromSurface(renderer,introSurface);
            playRect = { (SCREEN_WIDTH - playSurface->w) / 2, 200, playSurface->w, playSurface->h };
            exitRect = { (SCREEN_WIDTH - exitSurface->w) / 2, 300, exitSurface->w, exitSurface->h };
            introRect = { (SCREEN_WIDTH - introSurface->w) / 2, 400, introSurface->w, introSurface->h };
            SDL_FreeSurface(playSurface);
            SDL_FreeSurface(exitSurface);
            SDL_FreeSurface(introSurface);
        }
        /*introImage= loadTexture("instruction.png");
        if (!introImage)
        {
            cerr << "Không tải được ảnh hướng dẫn: " << IMG_GetError() << endl;
        }*/
        //pause load
        pauseButtonTexture = loadTexture("pause_button.png");
        SDL_QueryTexture(pauseButtonTexture, NULL, NULL, &pauseButtonRect.w, &pauseButtonRect.h);
        pauseButtonRect.w =40;  // Điều chỉnh kích thước nút Pause
        pauseButtonRect.h =40;
        pauseButtonRect.x = SCREEN_WIDTH - pauseButtonRect.w; // Góc phải
        pauseButtonRect.y = 0;
        pauseBackground = loadTexture("pause_background.png");
        SDL_QueryTexture(pauseBackground, NULL, NULL, &pauseBackgroundrect.w, &pauseBackgroundrect.h);
        pauseBackgroundrect.w =300;  // Điều chỉnh kích thước nút Pause
        pauseBackgroundrect.h =150;
        pauseBackgroundrect.x = 300; // Góc phải
        pauseBackgroundrect.y = 200;
        resumeButtonTexture = loadTexture("resume_button.png");
        SDL_QueryTexture(resumeButtonTexture, NULL, NULL, &resumeButtonRect.w, &resumeButtonRect.h);
        resumeButtonRect.w=60; resumeButtonRect.h=60;
        resumeButtonRect.x= 500;
        resumeButtonRect.y = 250;
        menuButtonTexture = loadTexture("menu_button.png");
        SDL_QueryTexture(menuButtonTexture, NULL, NULL, &menuButtonRect.w, &menuButtonRect.h);
        menuButtonRect.w=60;
        menuButtonRect.h=60;
        menuButtonRect.x = 400;
        menuButtonRect.y= 250;
        introImageRect= {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        generatewalls();
        spawnenemies();
        generateBuffs();
        spawnBoss();
    }
    //menu trước khi vào game
    void renderMenu()
    {
        // Tải ảnh nền menu
        SDL_Texture* backgroundTexture = loadTexture("menu_background.png");
        if (!backgroundTexture)
        {
            cerr << "Không tìm thấy ảnh nền menu: " << IMG_GetError() << "\n";
            return;
        }
        // Vẽ ảnh nền menu
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        if (playTexture && exitTexture && introTexture)
        {
            SDL_RenderCopy(renderer, playTexture, NULL, &playRect);
            SDL_RenderCopy(renderer, exitTexture, NULL, &exitRect);
            SDL_RenderCopy(renderer, introTexture, NULL, &introRect);
        }
        SDL_RenderPresent(renderer);
        // Hiển thị lên màn hình
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(backgroundTexture);
    }
    void handleMenu()
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = e.button.x;
                int y = e.button.y;
                if (x >= playRect.x && x < playRect.x + playRect.w && y >= playRect.y && y < playRect.y + playRect.h)
                {
                    inmenu = false; // Bắt đầu game
                }
                else if (x >= exitRect.x && x < exitRect.x + exitRect.w && y >= exitRect.y && y < exitRect.y + exitRect.h) {
                    running = false; // Thoát game
                }
                else if (x>=introRect.x && x <introRect.x + introRect.w && y>=introRect.y && y < introRect.y + introRect.h)
                {
                    introduction();
                }
            }
         }
    }
    //intro
    void introduction()
    {
        SDL_Event e;
        bool quit = false;
        SDL_Texture* introImage= loadTexture("instruction.png");//Load lại ảnh trong intro để tránh lỗi
        if (!introImage)
        {
            cerr << "Không tải được ảnh hướng dẫn: " << IMG_GetError() <<endl;
            return;
        }
        SDL_Texture *backButtonTexture = loadTexture("back.png");  // Load texture cho back button trong introduction
        SDL_Rect backButtonRect;
        if (!backButtonTexture)
        {
           cerr << "Không thể tạo texture cho back button \n";
        }
        else
        {
            SDL_QueryTexture(backButtonTexture, NULL, NULL, &backButtonRect.w, &backButtonRect.h);
            backButtonRect.w /= 1.5; // Giảm kích thước nút Back nếu cần
            backButtonRect.h /= 1.5;
            backButtonRect.x = 10;
            backButtonRect.y = 520;
        }
        SDL_RenderClear(renderer);
        SDL_Texture* backgroundTexture = loadTexture("menu_background.png");
        //SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        SDL_RenderCopy(renderer, introImage, NULL, &introImageRect);
        if(backButtonTexture!= nullptr)
        {
            SDL_RenderCopy(renderer,backButtonTexture,NULL,&backButtonRect);

        }
        SDL_RenderPresent(renderer);
        while (!quit)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    running=false;
                    quit= true;
                }
                else if(e.type == SDL_MOUSEBUTTONDOWN)
                {
                    //Lấy tọa độ chuột
                    int x=e.button.x;
                    int y = e.button.y;
                    if (x >= backButtonRect.x && x <= backButtonRect.x + backButtonRect.w && y >= backButtonRect.y && y <= backButtonRect.y + backButtonRect.h)
                    {
                        quit= true;
                    }
               }
            }
        }
        inmenu = true;
        SDL_DestroyTexture(introImage);
        //SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(backButtonTexture);

    }
    //buffs
    void generateBuffs()
    {
        for (int i = 0; i < 30; ++i)
        {
            for (int j = 0; j < 42; ++j)
            {
                if (mapp[i][j] == 3)
                {
                    buffs.push_back(Buff(j * TILE_SIZE, 40 + i * TILE_SIZE, renderer, "buff_ammo.png", Buff::AMMO));
                }
                else if (mapp[i][j] == 5)
                {
                    buffs.push_back(Buff(j * TILE_SIZE, 40 + i * TILE_SIZE, renderer, "buff_life.png", Buff::LIFE));
                }
            }
        }
    }
    void handlebuff()
    {
        for (auto &buff:buffs)
        {
            if(SDL_HasIntersection(&player.rect,&buff.rect) && buff.active==true)
            {
                if(buff.type == Buff::AMMO)
                {
                    player.currentAmmo=player.currentAmmo+10;
                    buff.active=false;

                }
                if(buff.type== Buff::LIFE)
                {

                    player.lives=std::min(player.lives+1,3);
                    buff.active=false;
                }
             }
         }
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
                ey=(rand()%(MAP_HEIGHT-4)+3)*TILE_SIZE;
                validposition=1;
                for(const auto& wall:walls)
                {
                    if(wall.active && wall.x==ex && wall.y==ey)
                    {
                        validposition=0;
                        break;
                    }
                }
                for(const auto& buff:buffs)
                {
                    if(buff.active && buff.x==ex && buff.y==ey)
                    {
                        validposition=0;
                        break;
                    }
                }
            }
            enemies.push_back(enemytank(ex, ey, renderer));  // Khởi tạo kẻ thù với ảnh
        }
    }
    //boss
    void spawnBoss()
    {
        int bx=730;//Thay giá trị tọa độ để hiển thị boss lên màn hình.
        int by = 200;
        boss = new Boss(bx, by, renderer, "boss.png");
        if(boss->spriteSheet)
        {
            SDL_QueryTexture(boss->spriteSheet,NULL, NULL, &boss->frameWidth, &boss->frameHeight);
            boss->frameWidth/=24;//Số frame theo chiều ngang
            boss->frameHeight/=1;
        }
    }
    //pausegame
    void pausegame()
    {
        // Tải ảnh pause
        SDL_RenderCopy(renderer,pauseBackground,NULL,&pauseBackgroundrect);//vẽ hình nền pause riêng
        SDL_RenderCopy(renderer, resumeButtonTexture, NULL, &resumeButtonRect);
        SDL_RenderCopy(renderer, menuButtonTexture, NULL, &menuButtonRect);
        SDL_RenderPresent(renderer);
    }
    //draw
    void render()
    {
        //window
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);
        if(gamePaused==0)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            for(int i=0;i<30;i++)
            {
                for(int j=0;j<42;j++)
                {
                    SDL_Rect tile={j*TILE_SIZE, i*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_RenderFillRect(renderer, &tile);
                }
            }
            if (!livesnanmoFont) return; // Bỏ qua nếu font chưa được tải
            SDL_Color textColor = {255, 255, 255};
            // Hiển thị đạn
            SDL_Surface* ammoSurface = TTF_RenderText_Solid(livesnanmoFont, ("Ammo: " + to_string(player.currentAmmo)).c_str(), textColor);
            if (!ammoSurface)
            {
                cerr << "Lỗi render text: " << TTF_GetError() << endl;
                return;
            }
            ammoTexture = SDL_CreateTextureFromSurface(renderer, ammoSurface);
            ammoRect = {10, 10, ammoSurface->w, ammoSurface->h};
            SDL_RenderCopy(renderer, ammoTexture, NULL, &ammoRect);
            SDL_FreeSurface(ammoSurface);
            // Hiển thị mạng
            SDL_Surface* livesSurface = TTF_RenderText_Solid(livesnanmoFont, ("Lives: " + to_string(player.lives)).c_str(), textColor);
            if (!livesSurface)
            {
                cerr << "Lỗi render text: " << TTF_GetError() << endl;
                return;
            }
            livesTexture = SDL_CreateTextureFromSurface(renderer, livesSurface);
            livesRect = {10, 30, livesSurface->w, livesSurface->h}; // Vị trí bên dưới đạn
            SDL_RenderCopy(renderer, livesTexture, NULL, &livesRect);
            SDL_FreeSurface(livesSurface);
            //wall
            for(int i=0;i<walls.size();i++)
            {
                walls[i].render(renderer);
            }
            for (auto& destructibleWall: destructibleWalls)
            {
                destructibleWall.render(renderer);

            }
            //player
            player.render(renderer);
            //boss
            if(boss)
            {
                boss->render(renderer);
            }
            //enemy
            for(auto& enemy:enemies)
            {
                enemy.render(renderer);
            }
            //buffs
            for (auto& buff : buffs)
            {
                buff.render(renderer);
            }
            //pause
            SDL_RenderCopy(renderer, pauseButtonTexture, NULL, &pauseButtonRect);
        }
        else {pausegame();}
        SDL_RenderPresent(renderer);

    }
    //palyer move
    void handle()
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            player.handleInput(event);
            if(gamePaused==0)
            {
                if(event.type==SDL_QUIT)
                {
                    running=0;
                }
                if (keystates[SDL_SCANCODE_UP])
                {
                    player.move(0, -5, walls, destructibleWalls);
                }
                if (keystates[SDL_SCANCODE_DOWN])
                {
                    player.move(0, 5, walls, destructibleWalls);
                }
                if (keystates[SDL_SCANCODE_LEFT])
                {
                    player.move(-5, 0, walls, destructibleWalls);
                }
                if (keystates[SDL_SCANCODE_RIGHT])
                {
                    player.move(5, 0, walls, destructibleWalls);
                }
                if (keystates[SDL_SCANCODE_SPACE])
                {
                     player.shoot(renderer, "playerbullet.jpg");
                }
            }
            if (event.type==SDL_MOUSEBUTTONDOWN)
            {
                if (!gamePaused)
                {
                    int x = event.button.x; int y = event.button.y;
                    if (x >= pauseButtonRect.x && x < pauseButtonRect.x + pauseButtonRect.w && y >= pauseButtonRect.y && y < pauseButtonRect.y + pauseButtonRect.h)
                    {
                        gamePaused = !gamePaused;
                    }
                }
                if (gamePaused)
                {
                    int x = event.button.x; int y = event.button.y;
                    if (x >= resumeButtonRect.x && x < resumeButtonRect.x + resumeButtonRect.w && y >= resumeButtonRect.y && y < resumeButtonRect.y + resumeButtonRect.h)
                    {
                        gamePaused = !gamePaused;//Quay trở lại game
                    }
                    else if (x >= menuButtonRect.x && x < menuButtonRect.x + menuButtonRect.w && y >= menuButtonRect.y && y < menuButtonRect.y + menuButtonRect.h)
                    {
                        inmenu = 1;//Về lại màn hình menu
                        gamePaused=!gamePaused;
                        resetGame();
                        break;
                    }
                }
            }
        }
    }
    void update()
    {
        if(gamePaused==0)
        {
            player.update(); // Cập nhật animation của player
            handlebuff();
            player.updatebullets();
            if(boss)
            {
                boss->move(walls, player, renderer);
                boss->update(renderer);
                for (auto &bullet : boss->boss_bullets)
                {
                    if (SDL_HasIntersection(&bullet.rect, &player.rect))
                    {
                        player.lives--;
                        player.playhurtSound();
                        bullet.active = false;
                        if(player.lives<=0)
                        {
                            SDL_Texture* loseTexture = loadTexture("lose.png");
                            if (!loseTexture)
                            {
                                cerr << "Không tìm thấy ảnh lose: " << IMG_GetError() << endl;
                                return;
                            }
                            // Vẽ ảnh thua lên màn hình
                            SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                            SDL_RenderCopy(renderer, loseTexture, NULL, &destRect);
                            SDL_RenderPresent(renderer);
                            // Giữ màn hình thua trong 3 giây
                            SDL_Delay(3000);
                            SDL_DestroyTexture(loseTexture);
                            running=0;
                            return ;
                        }
                        break;
                    }
                    for(auto& wall:walls)
                    {
                        if(SDL_HasIntersection(&wall.rect,&bullet.rect))
                        {
                            bullet.active=0;
                        }
                    }
                }
            }
            for (auto& bullet : player.bullets)
            {
                if (boss && SDL_HasIntersection(&bullet.rect, &boss->rect))
                {
                    boss->health--;
                    bullet.active = 0;//Hủy đạn
                    if (boss->health <= 0)
                    {
                        boss = nullptr; // reset lại biến boss về nullptr
                    }
                }
            }
            for(auto &bullet:player.bullets)
            {
                for(auto& wall:walls)
                {
                    if(wall.active && SDL_HasIntersection(&bullet.rect,&wall.rect))
                    {
                        //wall.active=0;
                        bullet.active=0;
                        break;
                    }
                }
                for (auto& wall : destructibleWalls)
                {
                    if(wall.active && SDL_HasIntersection(&bullet.rect,&wall.rect))
                    {
                        wall.active=0;
                        bullet.active=0;
                        break;
                    }
                }
                for(auto& enemy:enemies)
                {
                    if(enemy.active && SDL_HasIntersection(&bullet.rect,&enemy.rect))
                    {
                        enemy.playerhit();
                        enemy.active=0;
                        bullet.active=0;
                        break;
                    }
                }
            }
            for(auto& enemy:enemies)
            {
                enemy.update(); // Cập nhật animation của enemy
                enemy.move(walls, player, renderer);
                enemy.updatebullets();
                if(rand()%100<2)
                {
                    enemy.shoot(renderer,"enemybullet.png");
                }
                for(auto& bullet:enemy.bullets)
                {
                    for(auto& wall:walls)
                    {
                        if(wall.active && SDL_HasIntersection(&bullet.rect,&wall.rect))
                        {
                            //wall.active=0;
                            bullet.active=0;
                            break;
                        }
                    }
                    for(auto& bulletp:player.bullets)
                    {
                        if(SDL_HasIntersection(&bullet.rect,&bulletp.rect))
                        {
                            bullet.active=0;
                            bulletp.active=0;
                        }
                    }
                }
            }
            enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](enemytank &e){return !e.active;}),enemies.end());
            // Kiểm tra nếu thắng (không còn kẻ thù)
            if (enemies.empty() && !boss)
            {
                // Tải ảnh chiến thắng
                SDL_Texture* winTexture = loadTexture("win.png");
                if (!winTexture)
                {
                    cerr << "Không tìm thấy ảnh win: " << IMG_GetError() << endl;
                    return;
                }

                // Vẽ ảnh chiến thắng lên màn hình
                SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderCopy(renderer, winTexture, NULL, &destRect);
                SDL_RenderPresent(renderer);

                // Giữ màn hình chiến thắng trong 3 giây
                SDL_Delay(3000);
                SDL_DestroyTexture(winTexture);

                // Kết thúc game
                running = 0;
                return;
            }
            for(auto& enemy:enemies)
            {
                for(auto& bullet:enemy.bullets)
                {
                    if(SDL_HasIntersection(&bullet.rect,&player.rect))
                    {
                        player.lives--;//trừ mạng
                        player.playhurtSound();
                        bullet.active = 0; // Xóa đạn
                        if(player.lives<=0)
                        {
                            SDL_Texture* loseTexture = loadTexture("lose.png");
                            if (!loseTexture)
                            {
                                cerr << "Không tìm thấy ảnh lose: " << IMG_GetError() << endl;
                                return;
                            }
                            // Vẽ ảnh thua lên màn hình
                            SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                            SDL_RenderCopy(renderer, loseTexture, NULL, &destRect);
                            SDL_RenderPresent(renderer);
                            // Giữ màn hình thua trong 3 giây
                            SDL_Delay(3000);
                            SDL_DestroyTexture(loseTexture);
                            running=0;
                            return ;
                        }

                    }
                }
            }
        }
    }
    void resetGame()
    { // Hàm reset game
        player.lives = 3;
        player.currentAmmo = player.ammo;
        player.x=0; player.y=80; player.rect= {player.x,player.y,TILE_SIZE,TILE_SIZE};
        player.dirx=0; player.diry=-1; //reset vị trí player
        enemynumber=5; //Khởi tạo lại enemynumber.
        spawnenemies();//reset enemy.
        player.bullets.clear();
        //Reset lại buff
        buffs.clear();
        generateBuffs();
        //Xóa boss cũ, nếu có
        if (boss!=nullptr)
        {
            delete boss;
            boss = nullptr;
        }
        spawnBoss();
        if(boss!=nullptr) //reset lại boss
        {
            boss->health = 10;
            boss->shootDelay =180;
            boss->boss_bullets.clear();
        }
        SDL_Delay(50);
    }
    void run()
    {
        while(running)
        {
            if(inmenu)
            {
                handleMenu();
                renderMenu();
            }
            else
            {
                handle();
                if(gamePaused==0)
                {
                    render();
                    update();
                }
                else {pausegame();}
            }
            SDL_Delay(16);
        }
    }
    ~Game()
    {
        TTF_CloseFont(gFont);
        SDL_DestroyTexture(ammoTexture);
        SDL_DestroyTexture(livesTexture);
        SDL_DestroyTexture(playTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyTexture(backButtonTexture);
        SDL_DestroyTexture(introImage);
        SDL_DestroyWindow(window);
        SDL_Quit();
        IMG_Quit();
    }

};
int main(int argc, char* argv[])
{
    ifstream file("map.txt");  // Mở file để đọc
    if (!file) {
        cout << "Không thể mở file map!" << endl;
        return 1;
    }
    // Đọc các phần tử vào mảng
    for (int i = 0; i < 28; ++i) {
        for (int j = 0; j < 42; ++j) {
            file >> mapp[i][j];
        }
    }
    file.close();  // Đóng file
    if (Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC | MIX_INIT_MOD) == 0)
    {
        cerr <<"Mix khoi tao loi" << Mix_GetError();
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() <<"\n";
    }
    Game game;
    if(game.running)
    {
        game.run();
    }
    return 0;
}
