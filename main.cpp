#include <SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL_image.h>
#include <fstream>
#include<cmath>
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
/*void menu()
{
    SDL_Texture* menuTexture = loadTexture("menu_image.png");  // Tải ảnh menu
    if (!menuTexture)
    {
        cerr << "Không thể tải ảnh menu.\n";
        return;
    }

    // Vẽ menu
    SDL_RenderCopy(renderer, menuTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(menuTexture);
}*/
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
        active = 1;
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
        rect = {x, y, 10, 10};  // Kích thước mặc định của đạn (có thể thay đổi nếu cần)
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
        x=x+dx;
        y=y+dy;
        rect.x=x;
        rect.y=y;
        if(x<0 || x>SCREEN_WIDTH || y<0 || y>SCREEN_HEIGHT)
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
    int x,y;
    int dirx,diry;
    SDL_Rect rect;
    SDL_Texture* texture;  // Thêm texture cho xe tăng
    vector<bullet> bullets;
    void shoot(SDL_Renderer* renderer, const string& texturePath)
    {
        bullets.push_back(bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, this->dirx, this->diry, renderer, texturePath));
    }
    void updatebullets()
    {
        for(auto &bullet:bullets)
        {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](bullet &b){return !b.active;}),bullets.end());
    }
    playertank(int startx, int starty, SDL_Renderer* renderer, const string& imagePath)
    {
        x = startx;
        y = starty;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirx = 0;
        diry = 1;
        texture = loadTexture(imagePath, renderer);  // Tải texture cho xe tăng
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
        if(newx>=0 and newx<=SCREEN_WIDTH and newy>=0 and newy<=SCREEN_HEIGHT)
        {
            x=newx;
            y=newy;
            rect.x=x;
            rect.y=y;
        }
    }
    void render(SDL_Renderer* renderer)
    {
        if (texture)  // Kiểm tra xem texture có hợp lệ không
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);  // Vẽ xe tăng lên màn hình
        }
        else
        {
            // Nếu texture không hợp lệ, có thể vẽ hình chữ nhật dự phòng
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
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
    int movedelay=0,shootdelay;
    SDL_Rect rect;
    bool active;
    vector<bullet> bullets;
    SDL_Texture* texture;  // Thêm texture cho xe tăng kẻ thù

    enemytank(int startx, int starty, SDL_Renderer* renderer, const string& imagePath)
    {
        x = startx;
        y = starty;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirx = 0;
        diry = -1;
        active = true;
        texture = loadTexture(imagePath, renderer);  // Tải texture cho xe tăng kẻ thù
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
        movedelay=30;
        // Tính toán khoảng cách đến người chơi
        float dx = player.x - x;
        float dy = player.y - y;
        float distance = sqrt(dx * dx + dy * dy);

        // Nếu đủ gần, di chuyển theo hướng người chơi
        if (distance < 200)
        {   // Điều chỉnh giá trị này cho phù hợp
            double angle = atan2(dy, dx);
            dirx = 5 * cos(angle);
            diry = 5 * sin(angle);
        }
        else
        {
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
    void shoot(SDL_Renderer* renderer, const string& bulletTexturePath)
    {
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
        if (texture)  // Kiểm tra xem texture có hợp lệ không
        {
            SDL_RenderCopy(renderer, texture, NULL, &rect);  // Vẽ xe tăng kẻ thù lên màn hình
        }
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
    bool inmenu;
    vector<wall> walls;
    playertank player;
    int enemynumber=3;
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

    Game():player(0, 80, nullptr, "player_tank.png")
    {
        running=1;
        pause=0;
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
        player.texture = player.loadTexture("player_tank.png", renderer);
        generatewalls();
        spawnenemies();
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
        // Tạo một rectangle để chứa các nút
        SDL_Rect startButton = { (SCREEN_WIDTH - 200) / 2, SCREEN_HEIGHT / 2 - 50, 200, 100 };
        SDL_Rect quitButton = { (SCREEN_WIDTH - 200) / 2, SCREEN_HEIGHT / 2 + 50, 200, 100 };

        // Vẽ các nút bấm
        SDL_RenderFillRect(renderer, &startButton);
        SDL_RenderFillRect(renderer, &quitButton);

        // Vẽ chữ cho các nút (Có thể dùng SDL_ttf để vẽ chữ thay vì dùng hình ảnh)
        SDL_Texture* startTexture = loadTexture("start_button.png");
        SDL_Texture* quitTexture = loadTexture("quit_button.png");

        // Vẽ texture
        SDL_RenderCopy(renderer, startTexture, NULL, &startButton);
        SDL_RenderCopy(renderer, quitTexture, NULL, &quitButton);

        // Hiển thị lên màn hình
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(startTexture);
        SDL_DestroyTexture(quitTexture);
    }
    void handleMenu()
    {
        SDL_Rect startButton = { (SCREEN_WIDTH - 200) / 2, SCREEN_HEIGHT / 2 - 50, 200, 100 };
        SDL_Rect quitButton = { (SCREEN_WIDTH - 200) / 2, SCREEN_HEIGHT / 2 + 50, 200, 100 };
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                // Kiểm tra xem người dùng có nhấp vào nút Start không
                if (mouseX >= startButton.x && mouseX <= startButton.x + startButton.w && mouseY >= startButton.y && mouseY <= startButton.y + startButton.h)
                {
                    inmenu = false; // Bắt đầu game
                }
                // Kiểm tra xem người dùng có nhấp vào nút Quit không
                else if (mouseX >= quitButton.x && mouseX <= quitButton.x + quitButton.w && mouseY >= quitButton.y && mouseY <= quitButton.y + quitButton.h)
                {
                    running = false; // Thoát game
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
            }
            enemies.push_back(enemytank(ex, ey, renderer, "enemy_tank.png"));  // Khởi tạo kẻ thù với ảnh
        }
    }
    //pausegame
    void pausegame()
    {
        // Tải ảnh pause
        SDL_Texture* pauseTexture = loadTexture("pause.jpeg");
        if (!pauseTexture)
        {
            cerr << "khong tim thay anh pause pause.jpeg\n";
            return;
        }
        else //if (pauseTexture)
        {
            SDL_Rect destRect;
            destRect.x = (SCREEN_WIDTH - 300) / 2;
            destRect.y = (SCREEN_HEIGHT - 100) / 2;
            destRect.w = 300;
            destRect.h = 100;
            SDL_RenderCopy(renderer, pauseTexture, NULL, &destRect);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(pauseTexture);
        }
    }
    //draw
    void render()
    {
        //window
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderClear(renderer);
        if(pause==0)
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
            //wall
            for(int i=0;i<walls.size();i++)
            {
                walls[i].render(renderer);
            }
            //player
            player.render(renderer);
            //enemy
            for(auto& enemy:enemies)
            {
                enemy.render(renderer);
            }
        }
        else {pausegame();}
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
            if (keystates[SDL_SCANCODE_UP])
            {
                player.move(0, -5, walls);
            }
            if (keystates[SDL_SCANCODE_DOWN])
            {
                player.move(0, 5, walls);
            }
            if (keystates[SDL_SCANCODE_LEFT])
            {
                player.move(-5, 0, walls);
            }
            if (keystates[SDL_SCANCODE_RIGHT])
            {
                player.move(5, 0, walls);
            }
            if (keystates[SDL_SCANCODE_SPACE])
            {
                 player.shoot(renderer, "playerbullet.jpg");
            }
            if (keystates[SDL_SCANCODE_ESCAPE])
            {
                   pause = !pause;
            }
        }
    }
    // Hiển thị màn hình thắng
    void renderWin()
    {
        SDL_Texture* winTexture = loadTexture("win.png");
        if (!winTexture)
        {
            cerr << "Không tìm thấy ảnh win: " << IMG_GetError() << "\n";
            return;
        }

        SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, winTexture, NULL, &destRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(winTexture);
    }

    // Hiển thị màn hình thua
    void renderLose()
    {
        SDL_Texture* loseTexture = loadTexture("lose.png");
        if (!loseTexture)
        {
            cerr << "Không tìm thấy ảnh lose: " << IMG_GetError() << "\n";
            return;
        }

        SDL_Rect destRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, loseTexture, NULL, &destRect);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(loseTexture);
    }
    void update()
    {
        if(pause==0)
        {
            player.updatebullets();
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
                enemy.move(walls, player, renderer);
                enemy.updatebullets();
                if(rand()%100<2)
                {
                    enemy.shoot(renderer,"enemybullet.png");
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
                            //wall.active=0;
                            bullet.active=0;
                            break;
                        }
                    }
                }
            }
            enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](enemytank &e){return !e.active;}),enemies.end());
            // Kiểm tra nếu thắng (không còn kẻ thù)
            if (enemies.empty())
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
                if(pause==0)
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
        SDL_DestroyRenderer(renderer);
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
    Game game;
    if(game.running)
    {
        game.run();
    }
    return 0;
}





















