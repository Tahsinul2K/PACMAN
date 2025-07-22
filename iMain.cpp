#include "iGraphics.h"
#include "iSound.h"
#define N 19 // rows
#define M 23 // columns
#define A 36 // length of each square
#define MAX_NAMES 7
#define SPRITE_SIZE 32
#define LEFT_BUFFER 10
#define RIGHT_BUFFER 300
#define TOP_BUFFER 10
#define BOTTOM_BUFFER 10

#define MAX_HEIGHT (TOP_BUFFER + (N * A) + BOTTOM_BUFFER)
#define MAX_WIDTH (LEFT_BUFFER + (M * A) + RIGHT_BUFFER)
#define MARGIN (A - SPRITE_SIZE)

#define GHOST_SCORE_TIMER 3000

#define MAX_LIVES 3
#define MAX_LEVEL 2

#define DX 300
#define DY 80
#define START_BUTTONX 50
#define START_BUTTONY 450
#define HIGHSCORES_BUTTONX 80
#define HIGHSCORES_BUTTONY 350
#define SETTINGS_BUTTONX 50
#define SETTINGS_BUTTONY 250
#define HELP_BUTTONX 80
#define HELP_BUTTONY 150
#define EXIT_BUTTONX 50
#define EXIT_BUTTONY 50

#define DIM 40 // amount to dim color by

int pac_spawnX, pac_spawnY;
int ghost_spawnX;
int ghost_spawnY;
int win_music_played = 0;
int activate_freeze = 0;
//---------------------------------------------------------------------------------------------------
// TO DO:
//  Sound and music
//  Various menus
//
//
//
//
//-----------------------------------------------------------------------------------------------------------

// the first line of level%d.txt will contain 4 pairs of int, the spaawnx and spawny of pacman and the 4 ghosts
// 0 -> void
// 1 -> wall
// 2 -> pellet
// 3 -> power up
// 4 -> ghost doorway
// 5 -> pacman spawnpoint
// 6 -> ghost spawnpoint
// 7 -> freeze powerups
// 8 -> speedboost
// 9 -> Health
int moving_flag;
int levels[MAX_LEVEL][N][M];
int level = 0;
char username[50] = "";
int username_idx = 0;
int score = -1;
int remaining_pellets = 0; // including power pellets
int tick = 0;
int total_tick = 0;

int dim_start = 0; // amount to dim color by for different buttons
int dim_highscore = 0;
int dim_settings = 0;
int dim_help = 0;
int dim_exit = 0;

enum GameState
{
    SPLASH,
    MENU,
    USERNAME,
    HIGHSCORE,
    SETTINGS,
    HELP,
    GAME,
    DYING,
    PAUSE,
    WIN,
    LOSE,
    EXIT
};

enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

enum GhostName
{
    BLINKY,
    PINKY,
    INKY,
    CLYDE
};

typedef struct
{
    int x = pac_spawnX, y = pac_spawnY, v = 4;
    Direction dir = RIGHT;
    Direction dir_buffer;
    int lives = MAX_LIVES;
    int score = 0;
    int powered_up = 0;
    int power_up_duration = 0;
    int max_duration = 100;
    int death_ticks = 4 * 11;
    int freeze = 0;
    int freeze_duration = 0;
    int activate_speedboost = 0;
    int speed_duration = 0;
} Pacman;

typedef struct
{
    GhostName name = BLINKY;
    int x, y, v = 3;
    Direction dir = RIGHT;
    Direction dir_buffer;
    int value = 400;
    int is_scared = 0;
    int respawn_ticks = 30;

    int score_timer;

    int dead = 0;
    int deathX, deathY;
    int time_of_death;

    int can_shoot_laser=0;
    int laser_duratiton=20;
    int laser_start_time=0;
    int is_shooting_laser=0;

} Ghost;

typedef struct
{
    char name[50];
    int value;
} Score;

GameState game_state;
Pacman pacman;
Ghost ghost[4];
Score highscores[1000];

Image menubg, gameover, highscorebg, splash, win, help_screen;

// arrays for the various pacman and ghost sprites
Image pacman_up[2], pacman_down[2], pacman_left[2], pacman_right[2], pacman_death[12];
Sprite pacman_sprite;

Image ghost_up[4][2], ghost_down[4][2], ghost_left[4][2], ghost_right[4][2], scared[2], flicker[4], freeze[1];
Sprite ghost_sprite[4];
int can_move(int level[N][M], Direction dir);
// Image help_screen;

int comp(const void *a, const void *b)
{
    return ((Score *)b)->value - ((Score *)a)->value;
}

// loads all the sprites and stuff. Called once at the very start
void load_recources()
{
    iLoadImage(&help_screen,"assets/images/help_screen.png");
    iLoadImage(&splash, "assets/images/splash.jpg");
    iResizeImage(&splash, MAX_WIDTH - 2, MAX_HEIGHT - 2);
    iLoadImage(&menubg, "assets/images/bg1.jpg");
    iResizeImage(&menubg, MAX_WIDTH - 2, MAX_HEIGHT - 2);
    iLoadImage(&gameover, "assets/images/gameover.jpg");
    iResizeImage(&gameover, (MAX_WIDTH - 2) / 2, (MAX_HEIGHT - 2) / 2);
    iLoadImage(&highscorebg, "assets/images/highscorebg.jpeg");
    iResizeImage(&highscorebg, MAX_WIDTH - 2, MAX_HEIGHT - 2);
    iLoadImage(&win, "assets/images/WIN.jpg");
    iResizeImage(&win, MAX_WIDTH - 2, MAX_HEIGHT - 2);

    iInitSprite(&pacman_sprite, -1);
    iLoadFramesFromFolder(pacman_up, "assets/images/sprites/pacman/up");
    iLoadFramesFromFolder(pacman_down, "assets/images/sprites/pacman/down");
    iLoadFramesFromFolder(pacman_left, "assets/images/sprites/pacman/left");
    iLoadFramesFromFolder(pacman_right, "assets/images/sprites/pacman/right");
    iLoadFramesFromFolder(pacman_death, "assets/images/sprites/pacman/death");
    for (int i = 0; i < 4; i++)
    {
        iInitSprite(&ghost_sprite[i], -1);
        char address_up[100], address_down[100], address_left[100], address_right[100];
        sprintf(address_up, "assets/images/sprites/ghost%d/up", i);
        sprintf(address_down, "assets/images/sprites/ghost%d/down", i);
        sprintf(address_left, "assets/images/sprites/ghost%d/left", i);
        sprintf(address_right, "assets/images/sprites/ghost%d/right", i);
        iLoadFramesFromFolder(ghost_up[i], address_up);
        iLoadFramesFromFolder(ghost_down[i], address_down);
        iLoadFramesFromFolder(ghost_left[i], address_left);
        iLoadFramesFromFolder(ghost_right[i], address_right);
    }
    iLoadFramesFromFolder(scared, "assets/images/sprites/scared");
    iLoadFramesFromFolder(flicker, "assets/images/sprites/flicker");
}
// resets postions of pacman and ghosts at start of level and start of game
void reset_positions()
{
    iChangeSpriteFrames(&pacman_sprite, pacman_right, 2);
    pacman.x = pac_spawnX;
    pacman.y = pac_spawnY;
    for (int i = 0; i < 4; i++)
    {
        ghost[i].x = ghost_spawnX;
        ghost[i].y = ghost_spawnY;
    }
}

// load level from file into 2D array and also set spawn points
void load_levels()
{
    for (int l = 1; l <= MAX_LEVEL; l++)
    {
        char file_name[100];
        sprintf(file_name, "level%d.txt", l);
        FILE *f = fopen(file_name, "r");
        if (f == NULL)
        {
            printf("Unable to load %s...\n", file_name);
        }
        else
        {

            for (int i = 0; i < N; i++)
            {
                for (int j = 0; j < M; j++)
                {
                    if (fscanf(f, "%d", &levels[l - 1][i][j]) != 1)
                        printf("Unable to read level correctly...\n");
                    if (levels[l - 1][i][j] == 5)
                    {
                        pac_spawnX = LEFT_BUFFER + j * A + 1;
                        pac_spawnY = BOTTOM_BUFFER + (N - 1 - i) * A + 1;
                    }
                    else if (levels[l - 1][i][j] == 6)
                    {
                        ghost_spawnX = LEFT_BUFFER + j * A;
                        ghost_spawnY = BOTTOM_BUFFER + (N - 1 - i) * A;
                    }
                }
            }
            fclose(f);
        }
    }
}

int num; // total number of scores in highscore.txt
// loads scores from highscores.txt into a highscore array
void load_scores()
{
    FILE *f = fopen("highscores.txt", "a+");
    if (f == NULL)
    {
        printf("error loading highscores file...");
    }
    else
    {
        fscanf(f, "%d", &num);
        for (int i = 0; i < num; i++)
        {
            fscanf(f, "%s %d", highscores[i].name, &highscores[i].value);
        }
        fclose(f);
    }
    printf("Loaded highscores: ");
    for (int i = 0; i < num; i++)
    {
        printf("%s: %d, ", highscores[i].name, highscores[i].value);
    }
    printf("\n");
}
int game_music_played = 0, menu_music_played = 0, moving_music_played = 0, move_sound;
void play_music()
{
    if (game_state == WIN && win_music_played == 0)
    {
        iStopAllSounds();
        iPlaySound("assets/sounds/WIN.wav", false, 50);
        win_music_played = 1;
        menu_music_played = 0;
    }
    if (game_state == GAME && game_music_played == 0)
    {
        iStopAllSounds();
        // iPlaySound("assets/sounds/GAME.wav",true,50);
        game_music_played = 1;
        menu_music_played = 0;
        iPlaySound("assets/sounds/GAME.wav", true, 40);
    }
    if (game_state == MENU && menu_music_played == 0)
    {
        iStopAllSounds();
        game_music_played = 0;
        menu_music_played = 1;
        win_music_played = 0;
        iPlaySound("assets/sounds/MENU.wav", true, 50);
    }

    /* moving_flag = can_move(levels[level], pacman.dir);
     if(moving_flag && moving_music_played == 0)
     {
         move_sound = iPlaySound("assets/sounds/MOVE.wav",true,30);
         moving_music_played = 1;
     }
     else
     {
         iStopSound(move_sound);
     }*/
}
// saves highscore array into a highscores.txt file
void save_scores(int num)
{
    FILE *f = fopen("highscores.txt", "w");
    if (f == NULL)
    {
        printf("error trying to save highscores file...");
    }
    else
    {
        fprintf(f, "%d\n", num);
        for (int i = 0; i < num; i++)
        {
            fprintf(f, "%s %d\n", highscores[i].name, highscores[i].value);
        }
        fclose(f);
    }
}

// adds score into highscore array that has already been loaded and sorts it
int add_score(Score *score, int num)
{
    int same_found = 0;
    for (int i = 0; i < num; i++)
    {
        if (strcmp(highscores[i].name, score->name) == 0)
        {
            highscores[i].value = highscores[i].value > score->value ? highscores[i].value : score->value;
            same_found = 1;
        }
    }
    if (!same_found)
    {
        strcpy(highscores[num].name, score->name);
        highscores[num].value = score->value;
        printf("Score added! %d\n", score->value);
        qsort(highscores, num + 1, sizeof(highscores[0]), comp);
    }
    else
    {
        printf("Score added! %d\n", score->value);
        qsort(highscores, num, sizeof(highscores[0]), comp);
    }
    printf(" Current highscore list: ");
    for (int i = 0; i < num + !same_found; i++)
    {
        printf("%d ", highscores[i].value);
    }
    printf("\n");
    return !same_found;
}

int pellet_radius = A / 10;
int pellet_value = 10;
void draw_maze(int level[N][M])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            switch (level[i][j])
            {
            case 0:
                // void
                break;
            case 1: // wall
                iSetColor(0, 143, 150);
                iFilledRectangle(LEFT_BUFFER + j * A, BOTTOM_BUFFER + (N - 1 - i) * A, A, A);
                break;
            case 2: // pellet
                iSetColor(200, 200, 200);
                iFilledCircle(LEFT_BUFFER + j * A + A / 2, BOTTOM_BUFFER + (N - 1 - i) * A + A / 2, pellet_radius);
                break;
            case 3: // power pellet
                if (tick < 10)
                    iSetColor(0, 173, 181);
                else
                    iSetColor(0, 220, 240);
                iFilledCircle(LEFT_BUFFER + j * A + A / 2, BOTTOM_BUFFER + (N - 1 - i) * A + A / 2, pellet_radius * 2);
                break;
            case 4:
                iSetTransparentColor(0, 143, 150, .4);
                iFilledRectangle(LEFT_BUFFER + j * A, BOTTOM_BUFFER + (N - 1 - i) * A, A, A);
                break;
            case 7:
                iSetTransparentColor(3, 255, 247, .4);
                iFilledCircle(LEFT_BUFFER + j * A + A / 2, BOTTOM_BUFFER + (N - 1 - i) * A + A / 2, pellet_radius * 2);
                break;
            case 8:
                if (tick < 10)
                    iSetColor(255, 0, 0);
                else
                    iSetColor(209, 0, 0);

                iFilledCircle(LEFT_BUFFER + j * A + A / 2, BOTTOM_BUFFER + (N - 1 - i) * A + A / 2, pellet_radius * 2);
                break;
            default:
                break;
            }
        }
    }
}

// WILL BE REMOVED LATER. Currently converts a small number of pellets into power pellets
void generate_power_ups(int level[N][M])
{
    srand(time(0));
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            if (level[i][j] == 2)
            {
                int p = rand() % 100;
                if (p < 3)
                    level[i][j] = 3;
            }
        }
    }
}

// checks if pacman will collide with a wall
int can_move(int level[N][M], Direction dir)
{
    int nextX = pacman.x, nextY = pacman.y;
    switch (dir)
    {
    case LEFT:
        nextX -= pacman.v;
        break;
    case RIGHT:
        nextX += pacman.v;
        break;
    case UP:
        nextY += pacman.v;
        break;
    case DOWN:
        nextY -= pacman.v;
        break;
    }
    if (nextX < LEFT_BUFFER || nextY < BOTTOM_BUFFER)
        return 0;
    int blrow = N - 1 - (nextY - BOTTOM_BUFFER) / A;
    int trrow = N - 1 - (nextY + SPRITE_SIZE - BOTTOM_BUFFER) / A;
    int brrow = N - 1 - (nextY - BOTTOM_BUFFER) / A;
    int tlrow = N - 1 - (nextY + SPRITE_SIZE - BOTTOM_BUFFER) / A;

    int blcol = (nextX - LEFT_BUFFER) / A;
    int trcol = (nextX + SPRITE_SIZE - LEFT_BUFFER) / A;
    int brcol = (nextX + SPRITE_SIZE - LEFT_BUFFER) / A;
    int tlcol = (nextX - LEFT_BUFFER) / A;
    if (level[blrow][blcol] == 1 || level[trrow][trcol] == 1 || level[tlrow][tlcol] == 1 || level[brrow][brcol] == 1 || level[blrow][blcol] == 4 || level[trrow][trcol] == 4 || level[tlrow][tlcol] == 4 || level[brrow][brcol] == 4)
        return 0;
    return 1;
}

// function to make ghost collision detection more consistent by snaping the ghost to the center of the tile
void snap_to_center(Ghost *g)
{
    int col = (g->x - LEFT_BUFFER + SPRITE_SIZE / 2) / A;
    int row = (g->y - BOTTOM_BUFFER + SPRITE_SIZE / 2) / A;

    g->x = LEFT_BUFFER + col * A + (A - SPRITE_SIZE) / 2;
    g->y = BOTTOM_BUFFER + row * A + (A - SPRITE_SIZE) / 2;
}

// helper function for ghost pathfinding
int at_tile_center(int x, int y)
{
    return ((x - LEFT_BUFFER) % A < MARGIN) && ((y - BOTTOM_BUFFER) % A < MARGIN);
}

// helper function for ghost pathfinding
Direction opposite_direction(Direction d)
{
    switch (d)
    {
    case UP:
        return DOWN;
    case DOWN:
        return UP;
    case LEFT:
        return RIGHT;
    case RIGHT:
        return LEFT;
    }
}

// helper function for ghost pathfinding
int ghost_can_move(int level[N][M], Ghost *g, Direction dir)
{
    if(g->is_shooting_laser) return 0;

    int nextX = g->x, nextY = g->y;
    switch (dir)
    {
    case LEFT:
        nextX -= g->v;
        break;
    case RIGHT:
        nextX += g->v;
        break;
    case UP:
        nextY += g->v;
        break;
    case DOWN:
        nextY -= g->v;
        break;
    }
    if (nextX < LEFT_BUFFER || nextY < BOTTOM_BUFFER)
        return 0;
    int blrow = N - 1 - (nextY - BOTTOM_BUFFER) / A;
    int trrow = N - 1 - (nextY + SPRITE_SIZE - BOTTOM_BUFFER) / A;
    int brrow = N - 1 - (nextY - BOTTOM_BUFFER) / A;
    int tlrow = N - 1 - (nextY + SPRITE_SIZE - BOTTOM_BUFFER) / A;

    int blcol = (nextX - LEFT_BUFFER) / A;
    int trcol = (nextX + SPRITE_SIZE - LEFT_BUFFER) / A;
    int brcol = (nextX + SPRITE_SIZE - LEFT_BUFFER) / A;
    int tlcol = (nextX - LEFT_BUFFER) / A;
    if (level[blrow][blcol] == 1 || level[trrow][trcol] == 1 || level[tlrow][tlcol] == 1 || level[brrow][brcol] == 1)
        return 0;
    return 1;
}

// helper function for ghost pathfinding
Direction get_random_direction(Ghost *g)
{
    int cnt = 0;
    Direction options[4];
    for (int d = 0; d < 4; d++)
    {
        if (ghost_can_move(levels[level], g, (Direction)d))
        {
            if ((Direction)d != opposite_direction(g->dir))
            {
                options[cnt] = (Direction)d;
                cnt++;
            }
        }
    }

    if (cnt == 0)
        return opposite_direction(g->dir);
    return options[rand() % cnt];
}
void start_level(int l)
{
    remaining_pellets = 0;
    level = l;
    load_levels();
    game_state = GAME;
    pacman.powered_up = 0;
    reset_positions();

    if(l==0){
        ghost[0].can_shoot_laser=1;
    }else if(l==1){
        ghost[0].can_shoot_laser=1;
        ghost[1].can_shoot_laser=1;
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            if (levels[level][i][j] == 2  || levels[level][i][j] == 3 || levels[level][i][j] == 7 || levels[level][i][j] == 8)
                remaining_pellets=15;
    printf("Remaining pellets: %d\n", remaining_pellets);

}

// repeatedly called inside update_pacman() to check if he is on a pellet
void get_pellet(int level_[N][M])
{
    int centeredX = pacman.x + SPRITE_SIZE / 2;
    int centeredY = pacman.y + SPRITE_SIZE / 2;
    int row = N - 1 - (centeredY - BOTTOM_BUFFER) / A;
    int col = (centeredX - LEFT_BUFFER) / A;
    if (level_[row][col] == 2)
    {
        pacman.score += pellet_value;
        remaining_pellets--;
        level_[row][col] = 0;
        if (remaining_pellets <= 0 && level == MAX_LEVEL)
        {
            game_state = WIN;
        }
        else if (remaining_pellets <= 0)
        {
            start_level(level + 1);
        }
    }
}

// repeatedly called inside update_pacman() to check if he is on a a power pellet
void get_power_up(int level_[N][M])
{
    int centeredX = pacman.x + SPRITE_SIZE / 2;
    int centeredY = pacman.y + SPRITE_SIZE / 2;
    int row = N - 1 - (centeredY - BOTTOM_BUFFER) / A;
    int col = (centeredX - LEFT_BUFFER) / A;
    if (level_[row][col] == 3)
    {
        pacman.powered_up = 1;
        pacman.power_up_duration = pacman.max_duration;
        for (int i = 0; i < 4; i++)
        {

            ghost[i].is_scared = 1;
            ghost[i].dir = opposite_direction(ghost[i].dir);
            iChangeSpriteFrames(&ghost_sprite[i], scared, 2);
        }
        level_[row][col] = 0;
        remaining_pellets--;
        if (remaining_pellets <= 0 && level == MAX_LEVEL)
        {
            game_state = WIN;
        }
        else if (remaining_pellets <= 0)
        {
            start_level(level + 1);
        }
    }
}
void get_power_up_freeze(int level_[N][M])
{
    int centeredX = pacman.x + SPRITE_SIZE / 2;
    int centeredY = pacman.y + SPRITE_SIZE / 2;
    int row = N - 1 - (centeredY - BOTTOM_BUFFER) / A;
    int col = (centeredX - LEFT_BUFFER) / A;
    if (level_[row][col] == 7)
    {
        pacman.freeze = 1;
        pacman.freeze_duration = 100;
        pacman.score+=180;
        activate_freeze = 1;
        /* for (int i = 0; i < 4; i++)
        {
            ghost[i].v = 0;
        } */
        level_[row][col] = 0;
        remaining_pellets--;
        if (remaining_pellets <= 0 && level == MAX_LEVEL)
        {
            game_state = WIN;
        }
        else if (remaining_pellets <= 0)
        {
            start_level(level + 1);
        }
    }
}
void get_speedboost(int level_[N][M])
{
    int centeredX = pacman.x + SPRITE_SIZE / 2;
    int centeredY = pacman.y + SPRITE_SIZE / 2;
    int row = N - 1 - (centeredY - BOTTOM_BUFFER) / A;
    int col = (centeredX - LEFT_BUFFER) / A;
    if (level_[row][col] == 8)
    {
        pacman.activate_speedboost = 1;
        pacman.score += 75;
        pacman.speed_duration = 75;
        remaining_pellets--;
        level_[row][col] = 0;
        if (remaining_pellets <= 0 && level == MAX_LEVEL)
        {
            game_state = WIN;
        }
        else if (remaining_pellets <= 0)
        {
            start_level(level + 1);
        }
    }
}

void kill_pacman(){
    pacman.lives--;
    game_state = DYING;
    pacman.death_ticks = 4 * 11;
    iChangeSpriteFrames(&pacman_sprite, pacman_death, 12);
}

int in_range(Pacman *p, Ghost *g, int r){
    int dx=abs(p->x - g->x);
    int dy=abs(p->y - g->y);

    if(dx*dx + dy*dy <= r*r){
        printf("in range\n");
        return 1;
    }
    return 0;
}

void draw_laser(Pacman *p, Ghost *g){
    if(g->is_shooting_laser && tick<g->laser_start_time+g->laser_duratiton){
        iSetColor(250,250,250);
        iLine(p->x + A/2,p->y + A/2,g->x + A/2,g->y + A/2);
    }
}

// draw score obtained from killing ghost; the score will slowly move up
void ghost_score_animation()
{
    for (int i = 0; i < 4; i++)
    {
        if (ghost[i].dead && ghost[i].score_timer)
        {
            iSetColor(200, 200, 200);
            char msg[50];
            sprintf(msg, "%d", ghost[i].value);
            int offset = (total_tick - ghost[i].time_of_death) / 2;
            iText(ghost[i].deathX, ghost[i].deathY + offset, msg);
            ghost[i].score_timer--;
        }
    }
}
// returns a string of ... that varies with time
char *dots()
{
    if (tick < 5)
        return "";
    else if (tick < 10)
        return ".";
    else if (tick < 15)
        return "..";
    return "...";
}

void draw_highscore()
{
    iSetColor(0, 173, 181);
    iTextAdvanced(100, MAX_HEIGHT - 150, "HIGH SCORES", .5, 2.0, GLUT_STROKE_ROMAN);
    int p = 150;
    iSetColor(250, 250, 250);
    // draws leaderboard on to the screen in an alternating fashion
    for (int i = 1; i <= num && i <= MAX_NAMES; i++)
    {
        char msg[100];
        if (i % 2)
            sprintf(msg, "%d. %-25s %6d", i, highscores[i - 1].name, highscores[i - 1].value);
        else
            sprintf(msg, "     %d. %-20s %6d", i, highscores[i - 1].name, highscores[i - 1].value);
        iText(p, MAX_HEIGHT - 200 - 50 * i, msg, GLUT_BITMAP_9_BY_15);
    }
}

void iDraw()
{
    // place your drawing codes here
    char score_text[100];
    char lives_text[100];
    char remaining_pellets_text[100];
    char level_text[100];
    char msg[100], text_field[100];
    int dx, dy, p; // p means padding
    iClear();
    switch (game_state)
    {
    case SPLASH:
        iSetColor(250, 250, 250);
        iFilledRectangle(0, 0, MAX_WIDTH, MAX_HEIGHT);
        iShowLoadedImage(0, 0, &splash);
        sprintf(msg, "LEFT CLICK ANYWHERE TO CONTINUE%s", dots());
        iSetColor(0, 0, 0);
        iText(50, 50, msg, GLUT_BITMAP_8_BY_13);
        break;
    case MENU:
        dx = DX, dy = DY;
        p = .2 * dy;
        iShowLoadedImage(0, 0, &menubg);
        iSetColor(0, 173 - dim_start, 181 - dim_start);
        // iTextAdvanced(50,MAX_HEIGHT-150,"PACMAN",1.0,2.0,GLUT_STROKE_ROMAN);
        iShowImage(MAX_WIDTH - 650, MAX_HEIGHT - 250, "assets/images/pacman_logo.bmp");
        iFilledRectangle(START_BUTTONX, START_BUTTONY, dx, dy);
        iSetColor(250, 250, 250);
        iText(START_BUTTONX + p, START_BUTTONY + p, "PLAY", GLUT_BITMAP_9_BY_15);

        iSetColor(0, 173 - dim_highscore, 181 - dim_highscore);
        iFilledRectangle(HIGHSCORES_BUTTONX, HIGHSCORES_BUTTONY, dx, dy);
        iSetColor(250, 250, 250);
        iText(HIGHSCORES_BUTTONX + p, HIGHSCORES_BUTTONY + p, "HIGH SCORES", GLUT_BITMAP_9_BY_15);

        iSetColor(0, 173 - dim_settings, 181 - dim_settings);
        iFilledRectangle(SETTINGS_BUTTONX, SETTINGS_BUTTONY, dx, dy);
        iSetColor(250, 250, 250);
        iText(SETTINGS_BUTTONX + p, SETTINGS_BUTTONY + p, "SETTINGS", GLUT_BITMAP_9_BY_15);

        iSetColor(0, 173 - dim_help, 181 - dim_help);
        iFilledRectangle(HELP_BUTTONX, HELP_BUTTONY, dx, dy);
        iSetColor(250, 250, 250);
        iText(HELP_BUTTONX + p, HELP_BUTTONY + p, "HELP", GLUT_BITMAP_9_BY_15);

        iSetColor(0, 173 - dim_exit, 181 - dim_exit);
        iFilledRectangle(EXIT_BUTTONX, EXIT_BUTTONY, dx, dy);
        iSetColor(250, 250, 250);
        iText(EXIT_BUTTONX + p, EXIT_BUTTONY + p, "EXIT", GLUT_BITMAP_9_BY_15);

        break;
    case USERNAME:
        dx = 200, dy = 25;
        iSetColor(240, 240, 250);
        sprintf(msg, "Enter name%s", dots());
        iText(MAX_WIDTH / 2 - dx / 2, MAX_HEIGHT / 2 + dy / 2 + 5, msg, GLUT_BITMAP_9_BY_15);
        iFilledRectangle(MAX_WIDTH / 2 - dx / 2, MAX_HEIGHT / 2 - dy / 2, dx, dy);
        iSetColor(0, 0, 0);
        if (tick < 10)
            sprintf(text_field, "%s|", username);
        else
            sprintf(text_field, "%s", username);
        iText(MAX_WIDTH / 2 - dx / 2 + 2, MAX_HEIGHT / 2 - dy / 2 + 5, text_field, GLUT_BITMAP_HELVETICA_18);
        break;
    case HIGHSCORE:
        iShowLoadedImage(0, 0, &highscorebg);
        draw_highscore();

        sprintf(msg, "LEFT CLICK ANYWHERE TO RETURN TO MAIN MENU%s", dots());
        iSetColor(250, 250, 250);
        iText(50, 50, msg, GLUT_BITMAP_8_BY_13);
        break;
    case SETTINGS:
        sprintf(msg, "LEFT CLICK ANYWHERE TO RETURN TO MAIN MENU%s", dots());
        iSetColor(250, 250, 250);
        iText(50, 50, msg, GLUT_BITMAP_8_BY_13);
        break;
    case HELP:
        iShowImage(0, 0, "assets/images/help_screen.bmp");

        sprintf(msg, "LEFT CLICK ANYWHERE TO RETURN TO MAIN MENU%s", dots());
        iSetColor(250, 250, 250);
        iText(50, 50, msg, GLUT_BITMAP_8_BY_13);
        break;
    case PAUSE:
        draw_maze(levels[level]);
        sprintf(score_text, "SCORE: %6d", pacman.score);
        sprintf(lives_text, "LIVES: %d", pacman.lives);
        iSetColor(0, 173, 181);
        iText(LEFT_BUFFER + M * A + 30, MAX_HEIGHT - 30, score_text, GLUT_BITMAP_HELVETICA_18);
        iText(LEFT_BUFFER + M * A + 30, MAX_HEIGHT - 100, lives_text, GLUT_BITMAP_HELVETICA_18);

        for (int i = 0; i < 4; i++)
        {
            iShowSprite(&ghost_sprite[i]);
        }
        iShowSprite(&pacman_sprite);
        iSetTransparentColor(0, 0, 0, .7);
        iFilledRectangle(0, 0, MAX_WIDTH, MAX_HEIGHT);
        iShowImage(250, MAX_HEIGHT / 2, "assets/images/pause.png");
        iSetTransparentColor(5, 17, 242, 0.5);
        iFilledRectangle(MAX_WIDTH / 2 - 200, MAX_HEIGHT / 2 - 100, 300, 100);
        iSetColor(255, 255, 255);
        iText(MAX_WIDTH / 2 - 100, MAX_HEIGHT / 2 - 60, "SETTINGS", GLUT_BITMAP_HELVETICA_18);
        break;
    case DYING:
    case GAME:
        draw_maze(levels[level]);

        sprintf(score_text, "SCORE: %6d", pacman.score);
        sprintf(lives_text, "LIVES: %d", pacman.lives);
        sprintf(remaining_pellets_text, "Remaining Pellets: %3d", remaining_pellets);
        sprintf(level_text, "Level %d", level + 1);
        iSetColor(0, 173, 181);
        iText(LEFT_BUFFER + M * A + 30, MAX_HEIGHT - 50, score_text, GLUT_BITMAP_HELVETICA_18);
        iText(LEFT_BUFFER + M * A + 30, MAX_HEIGHT - 100, lives_text, GLUT_BITMAP_HELVETICA_18);
        iText(LEFT_BUFFER + M * A + 30, 100, remaining_pellets_text, GLUT_BITMAP_HELVETICA_18);
        iText(LEFT_BUFFER + M * A + 30, 50, level_text, GLUT_BITMAP_HELVETICA_18);

        // draw pacman and ghosts
        for (int i = 0; i < 4; i++)
        {
            iShowSprite(&ghost_sprite[i]);
            draw_laser(&pacman,&ghost[i]);
        }
        iShowSprite(&pacman_sprite);
        ghost_score_animation();
        break;
    case LOSE:
        iSetColor(250, 250, 250);
        iShowLoadedImage(MAX_WIDTH / 4, MAX_HEIGHT / 2 - 20, &gameover);
        sprintf(msg, "LEFT CLICK ANYWHERE TO RETURN TO MAIN MENU%s", dots());
        iText(50, 50, msg, GLUT_BITMAP_8_BY_13);
        break;
    case WIN:
        iShowLoadedImage(0, 0, &win);
    default:
        break;
    }
}

/*
function iMouseWheel() is called when the user scrolls the mouse wheel.
dir = 1 for up, -1 for down.
*/
void iMouseWheel(int dir, int mx, int my)
{
    // place your code here
}

/*
function iSpecialKeyboard() is called whenver user hits special keys likefunction
keys, home, end, pg up, pg down, arraows etc. you have to use
appropriate constants to detect them. A list is:
GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11,
GLUT_KEY_F12, GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN,
GLUT_KEY_PAGE_UP, GLUT_KEY_PAGE_DOWN, GLUT_KEY_HOME, GLUT_KEY_END,
GLUT_KEY_INSERT */
void iSpecialKeyboard(unsigned char key)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        pacman.dir_buffer = RIGHT;
        break;
    case GLUT_KEY_LEFT:
        pacman.dir_buffer = LEFT;
        break;
    case GLUT_KEY_UP:
        pacman.dir_buffer = UP;
        break;
    case GLUT_KEY_DOWN:
        pacman.dir_buffer = DOWN;
        break;
    case GLUT_KEY_F5:
        if (game_state != GAME)
            game_state = MENU;
    default:
        break;
    }
}

// repeatedly called inside update_game() on a timer. Controls logic and animation
void update_pacman()
{
    if (game_state != GAME)
        return;
    if (can_move(levels[level], pacman.dir_buffer))
    {
        pacman.dir = pacman.dir_buffer;
    }
    if (can_move(levels[level], pacman.dir))
    {
        switch (pacman.dir)
        {
        case LEFT:
            pacman.x -= pacman.v;
            iChangeSpriteFrames(&pacman_sprite, pacman_left, 2);
            break;
        case RIGHT:
            pacman.x += pacman.v;
            iChangeSpriteFrames(&pacman_sprite, pacman_right, 2);
            break;
        case UP:
            pacman.y += pacman.v;
            iChangeSpriteFrames(&pacman_sprite, pacman_up, 2);
            break;
        case DOWN:
            pacman.y -= pacman.v;
            iChangeSpriteFrames(&pacman_sprite, pacman_down, 2);
            break;
        }
    }

    pacman_sprite.x = pacman.x;
    pacman_sprite.y = pacman.y;
    if (pacman.powered_up)
    {
        pacman.power_up_duration--;
    }
    if (pacman.power_up_duration == 0)
    {
        pacman.powered_up = 0;
    }
    if (activate_freeze)
    {
        pacman.freeze_duration--;
        for (int i = 0; i < 4; i++)
        {
            ghost[i].v = 0;
        }
    }
    if (pacman.freeze_duration == 0 && activate_freeze)
    {
        for (int i = 0; i < 4; i++)
        {
            ghost[i].v = 3;
        }
        activate_freeze = 0;
    }
    if(pacman.activate_speedboost)
    {
        pacman.speed_duration--;
        pacman.v = 7;
    }
    if(pacman.speed_duration ==0 && pacman.activate_speedboost)
    {
        pacman.v = 4;
        pacman.activate_speedboost = 0;
    }
    get_pellet(levels[level]);
    get_power_up(levels[level]);
    get_power_up_freeze(levels[level]);
    get_speedboost(levels[level]);
    if (tick % 4 == 0)
        iAnimateSprite(&pacman_sprite);
}

// repeated called inside update_game() on a timer. Controls logic and animation
void update_ghost(Ghost *g, int pacX, int paxY, Direction pac_dir)
{
    if (game_state != GAME)
        return;
    if (g->dead)
    {
        g->x = 2 * MAX_WIDTH;
        g->y = 2 * MAX_HEIGHT;
        g->respawn_ticks--;
        if (g->respawn_ticks == 0)
        {
            g->dead = 0;
            g->x = ghost_spawnX;
            g->y = ghost_spawnY;
        }
    }
    if (at_tile_center(g->x, g->y))
    {
        snap_to_center(g);
        g->dir = get_random_direction(g);
        if (!g->is_scared)
        {
            switch (g->dir)
            {
            case RIGHT:
                iChangeSpriteFrames(&ghost_sprite[g->name], ghost_right[g->name], 2);
                break;
            case LEFT:
                iChangeSpriteFrames(&ghost_sprite[g->name], ghost_left[g->name], 2);
                break;
            case UP:
                iChangeSpriteFrames(&ghost_sprite[g->name], ghost_up[g->name], 2);
                break;
            case DOWN:
                iChangeSpriteFrames(&ghost_sprite[g->name], ghost_down[g->name], 2);
                break;
            }
        }
    }
    if (pacman.power_up_duration == pacman.max_duration / 2)
    {
        iChangeSpriteFrames(&ghost_sprite[g->name], flicker, 4);
    }
    else if (pacman.power_up_duration == 0)
    {
        g->is_scared = 0;
    }
    ghost_sprite[g->name].x = ghost[g->name].x;
    ghost_sprite[g->name].y = ghost[g->name].y;
    if (tick % 4 == 0)
        iAnimateSprite(&ghost_sprite[g->name]);

    // trigger ghost shooting laser
    if(g->can_shoot_laser && in_range(&pacman,g,100) && game_state!=DYING && g->is_scared==0){
        g->is_shooting_laser = 1;
        g->laser_start_time=tick;
        kill_pacman();
    }else{
        g->is_shooting_laser=0;
    }

    if (ghost_can_move(levels[level], g, g->dir))
    {
        switch (g->dir)
        {
        case LEFT:
            g->x -= g->v;
            break;
        case RIGHT:
            g->x += g->v;
            break;
        case UP:
            g->y += g->v;
            break;
        case DOWN:
            g->y -= g->v;
            break;
        }
    }
}

// checks for collision between pacman and ghosts. Called in update_game()
void handle_collision()
{
    for (int i = 0; i < 4; i++)
    {
        int dx = abs(pacman.x - ghost[i].x);
        int dy = abs(pacman.y - ghost[i].y);

        if (dx > SPRITE_SIZE || dy > SPRITE_SIZE)
            continue;

        if (pacman.powered_up)
        {
            pacman.score += ghost[i].value;
            ghost[i].dead = 1;
            ghost[i].is_scared = 0;
            ghost[i].respawn_ticks = 40;
            ghost[i].deathX = ghost[i].x;
            ghost[i].deathY = ghost[i].y;
            ghost[i].score_timer = GHOST_SCORE_TIMER;
            ghost[i].time_of_death = total_tick;
        }
        else
        {
            kill_pacman();
        }
    }
}

// the main function managing everything
void update_game()
{
    switch (game_state)
    {
    case MENU:

        break;
    case HIGHSCORE:

        break;
    case SETTINGS:

        break;
    case EXIT:
        exit(0);
        break;
    case GAME:
        update_pacman();
        for (int i = 0; i < 4; i++)
        {
            update_ghost(&ghost[i], pacman.x, pacman.y, pacman.dir);
        }
        handle_collision();
        break;
    case DYING:
        if (pacman.death_ticks == 0)
        {
            if (pacman.lives > 0)
            {
                game_state = GAME;
                reset_positions();
                iChangeSpriteFrames(&pacman_sprite, pacman_left, 2);
            }
            else
            {
                game_state = LOSE;
                Score s;
                strcpy(s.name, username);
                s.value = pacman.score;
                int new_num = num + add_score(&s, num);
                save_scores(new_num);
            }
        }
        else if (pacman.death_ticks % 4 == 0)
        {
            iAnimateSprite(&pacman_sprite);
        }
        pacman.death_ticks--;
        break;
    case LOSE:
        break;
    default:
        break;
    }

    tick = (tick + 1) % 20;
    total_tick++;
}

// initializes stuff at the start of a new game
void start_game()
{
    load_scores();
    load_levels();
    start_level(0);

    pacman.lives = MAX_LIVES;
    pacman.score = 0;
}

void iKeyboard(unsigned char key)
{
    if (game_state == USERNAME)
    {
        if (('0' <= key && key <= '9' || 'A' <= key && key <= 'Z' || 'a' <= key && key <= 'z') && username_idx < 49)
        {
            username[username_idx++] = key;
            username[username_idx] = '\0';
        }
        else if (key == '\b')
        {
            username_idx = username_idx - 1 > 0 ? username_idx - 1 : 0;
            username[username_idx] = '\0';
        }
        else if ((key == '\n' || key == '\r') && username[0] != '\0')
        {
            printf("%s", username);
            start_game();
        }
    }
    switch (key)
    {
    case 'p':
        if (game_state == GAME)
        {
            game_state = PAUSE;
        }
        else if (game_state == PAUSE)
        {
            game_state = GAME;
        }
        break;
    case 'b':
        if (game_state != MENU)
        {
            game_state = MENU;
        }
        break;
    default:
        break;
    }
}

/*
function iMouseMove() is called when the user moves the mouse.
(mx, my) is the position where the mouse pointer is.
*/

// hover animation
void iMouseMove(int mx, int my)
{
    switch (game_state)
    {
    case MENU:
        if (START_BUTTONX <= mx && mx <= START_BUTTONX + DX && START_BUTTONY <= my && my <= START_BUTTONY + DY)
        {
            dim_start = DIM;
        }
        else if (HIGHSCORES_BUTTONX <= mx && mx <= HIGHSCORES_BUTTONX + DX && HIGHSCORES_BUTTONY <= my && my <= HIGHSCORES_BUTTONY + DY)
        {
            dim_highscore = DIM;
        }
        else if (SETTINGS_BUTTONX <= mx && mx <= SETTINGS_BUTTONX + DX && SETTINGS_BUTTONY <= my && my <= SETTINGS_BUTTONY + DY)
        {
            dim_settings = DIM;
        }
        else if (HELP_BUTTONX <= mx && mx <= HELP_BUTTONX + DX && HELP_BUTTONY <= my && my <= HELP_BUTTONY + DY)
        {
            dim_help = DIM;
        }
        else if (EXIT_BUTTONX <= mx && mx <= EXIT_BUTTONX + DX && EXIT_BUTTONY <= my && my <= EXIT_BUTTONY + DY)
        {
            dim_exit = DIM;
        }
        else
        {
            dim_start = dim_highscore = dim_settings = dim_help = dim_exit = 0;
        }
        break;
    }
}

/*
function iMouseDrag() is called when the user presses and drags the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouseDrag(int mx, int my)
{
    // place your codes here
}

/*
function iMouse() is called when the user presses/releases the mouse.
(mx, my) is the position where the mouse pointer is.
*/
void iMouse(int button, int state, int mx, int my)
{
    switch (game_state)
    {
    case MENU:
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && START_BUTTONX <= mx && mx <= START_BUTTONX + DX && START_BUTTONY <= my && my <= START_BUTTONY + DY) // start button
        {
            if (username[0] != '\0')
            {
                start_game();
                iStopAllSounds();
            }
            else
            {
                game_state = USERNAME;
            }
        }
        else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && HIGHSCORES_BUTTONX <= mx && mx <= HIGHSCORES_BUTTONX + DX && HIGHSCORES_BUTTONY <= my && my <= HIGHSCORES_BUTTONY + DY)
        {
            game_state = HIGHSCORE;
            load_scores();
        }
        else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && SETTINGS_BUTTONX <= mx && mx <= SETTINGS_BUTTONX + DX && SETTINGS_BUTTONY <= my && my <= SETTINGS_BUTTONY + DY)
        {
            game_state = SETTINGS;
        }
        else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && HELP_BUTTONX <= mx && mx <= HELP_BUTTONX + DX && HELP_BUTTONY <= my && my <= HELP_BUTTONY + DY)
        {
            game_state = HELP;
        }
        else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && EXIT_BUTTONX <= mx && mx <= EXIT_BUTTONX + DX && EXIT_BUTTONY <= my && my <= EXIT_BUTTONY + DY)
        {
            game_state = EXIT;
        }
        break;

    // intentionally no break; to be able to return to the main menu
    case SPLASH:
    case WIN:
    case HIGHSCORE:
    case SETTINGS:
    case HELP:
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            game_state = MENU;
        }
        break;
    case PAUSE:
        if (mx > MAX_WIDTH / 2 - 200 && mx < MAX_WIDTH / 2 + 100 && my > MAX_HEIGHT / 2 - 100 && my < MAX_HEIGHT / 2 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
        {
            game_state = SETTINGS;
        } // MAX_WIDTH/2 - 200 ,MAX_HEIGHT/2 - 100 , 300, 100
        break;
    case LOSE:
        break;
    default:
        break;
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // place your codes here
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    // place your own initialization codes here.
    game_state = SPLASH;
    for (int i = 0; i < MAX_NAMES; i++)
        highscores[i].name[0] = '\0';
    load_scores();
    load_levels();

    for (int i = 0; i < 4; i++)
    {
        ghost[i].name = (GhostName)i;
    }
    load_recources();
    int t = iSetTimer(50, update_game);
    int w = iSetTimer(50, play_music);
    iInitializeSound();
    iInitialize(MAX_WIDTH, MAX_HEIGHT, "Pacman");
    return 0;
}