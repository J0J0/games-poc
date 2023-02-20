#include <stdlib.h>     // srand() / rand() / abs()
//#include <string.h>     // memset(), include if the prototype below won't work
extern void* memset (void*, int, size_t);
#include <time.h>       // time() and clock_gettime()
#include <ncurses.h>

#define BORDER_CHAR         '#'
#define WALL_CHAR           '#'
#define OBSTICAL_CHAR       '='
#define PLAYER_CHAR         'x'
#define ENEMY_CHAR          'y'
#define BOMB_CHAR           'o'
#define PLAYER_BOMB_CHAR    'X'
#define I_FIRE_CHAR         'F'
#define I_MORE_BOMBS_CHAR   'B'

#define Y_DIM 11
#define X_DIM 21

#define MAX_ENEMIES         20
#define MAX_BOMBS           20
#define GAME_SPEED_INTERVAL 0.08   // as double in sec
#define BOMB_TIMEOUT        2.00   // as double in sec


typedef struct{
    int y;
    int x;
} coord_t;

typedef struct{
    coord_t pos;
    int max_bombs;
    int blast_radius;
    int is_over_bomb;
} player_t;

typedef struct{
    coord_t pos;
} enemy_t;

typedef struct{
    int count;
    enemy_t list[MAX_ENEMIES];
} enemies_t;

typedef struct{
    coord_t pos;
    double timer;
    int radius;
    int detonating;
} bomb_t;

typedef struct{
    int count;
    bomb_t list[MAX_BOMBS];
} bombs_t;

typedef int game_field_t[Y_DIM][X_DIM];

typedef enum{
    ITEM_FIRE=10, ITEM_MORE_BOMBS
} item_t;
#define NUM_ITEMS 2


double clock_gettime_dbl(void);
void init_area(game_field_t area, int obstical_count);
void init_enemies(enemies_t* e, game_field_t area, int count);
void init_player(player_t* p, enemies_t* e, game_field_t area);
int try_move( int dir, coord_t* pos, game_field_t area, char moved_char, int reset_previous_pos );
int move_enemies( enemies_t* e, game_field_t area );
int try_place_bomb( game_field_t area, player_t* p, bombs_t* b );
int detonate_bombs( double t, bombs_t* b, game_field_t area, player_t* p, enemies_t* e );
int blast_on_coord(int y, int x, game_field_t area, player_t* p, enemies_t* e, bombs_t* b);
int leave_item(void);
void apply_item( item_t item, player_t* p );
void paint_border(WINDOW* win);
void paint_area( WINDOW* win, game_field_t area );
void paint_player( WINDOW* win, player_t* p );
void paint_enemies( WINDOW* win, enemies_t* e );
void paint_bombs( WINDOW* win, bombs_t* b, game_field_t area );
void repaint_all( WINDOW* win, game_field_t area, player_t* p,
                    enemies_t* e, bombs_t* b );

int main(void){
    int retry, running;
    int c;
    int newdir, has_moved, died;
    double t0, t1;
    game_field_t area;
    player_t player;
    enemies_t enemies;
    bombs_t bombs;
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    srand(time(NULL));
    
    retry = 1;
    
    while(retry){
        init_area(area, 20);
        init_enemies(&enemies, area, 5);
        init_player(&player, &enemies, area);
        bombs.count = 0;
        
        repaint_all(stdscr, area, &player, &enemies, &bombs);
        refresh();
        
        timeout(-1);
        c = getch();
        ungetch(c);
        
        t0 = t1 = clock_gettime_dbl();
        running = 1;
        timeout(10);
        
        while(running){
            c = getch();
            switch(c){
                case KEY_LEFT:
                case KEY_UP:
                case KEY_RIGHT:
                case KEY_DOWN:
                    newdir = c;
                    break;
                case ' ':
                    try_place_bomb(area, &player, &bombs);
                    break;
                case 'q':
                    running = 0;
                    retry   = 0;
                    break;
            }
            
            t0 = clock_gettime_dbl();
            if( (t1-t0) <= 0 ){
                died = 0;
                
                if(newdir > 0){
                    has_moved = try_move(newdir, &player.pos, area, PLAYER_CHAR, !player.is_over_bomb);
                    if(has_moved >= 101){
                        died = 1;
                    }
                    if(has_moved < 50 && player.is_over_bomb){
                        player.is_over_bomb = 0;
                    }
                    if(has_moved >= 10 && has_moved < 20){
                        apply_item(has_moved, &player);
                    }
                    
                    newdir = -1;
                }
                if(move_enemies(&enemies, area) == 100)
                    died = 1;
                if(detonate_bombs(t0, &bombs, area, &player, &enemies) == 100)
                    died = 1;
                
                if(died){
                    clear();
                    running = 0;
                    break;
                }
                    
                repaint_all(stdscr, area, &player, &enemies, &bombs);
                refresh();
                t1 = t0 + GAME_SPEED_INTERVAL;
            }
        }
    }
    
    timeout(-1);
    getch();
    endwin();
    
    return 0;
}


void init_area( game_field_t area, int obstical_count ){
    int i, j;
    coord_t randc;
    
    memset(area, 0, X_DIM*Y_DIM*sizeof(area[0][0]));
    
    for(j = 1; j < Y_DIM; j+=2){
        for(i = 1; i < X_DIM; i+=2){
            area[j][i] = WALL_CHAR;
        }
    }
    
    while(obstical_count > 0){
        randc.x = rand() % X_DIM;
        randc.y = rand() % Y_DIM;
        
        if(area[randc.y][randc.x] != 0)
            continue;
        
        area[randc.y][randc.x] = OBSTICAL_CHAR;
        obstical_count--;
    }
}

void init_enemies( enemies_t* e, game_field_t area, int count ){
    coord_t randc;
    
    e->count = 0;
    while(count > 0){
        randc.x = rand() % X_DIM;
        randc.y = rand() % Y_DIM;
        
        if(area[randc.y][randc.x] != 0)
            continue;
        
        area[randc.y][randc.x] = ENEMY_CHAR;
        e->list[e->count].pos = randc;
        e->count++;
        count--;
    }
}

void init_player( player_t* p, enemies_t* e, game_field_t area ){
    int i;
    coord_t randc;
    int dx, dy;
    int redo;
    
    p->max_bombs = 1;
    p->blast_radius = 1;
    p->is_over_bomb = 0;
    
    while(1){
        randc.x = rand() % X_DIM;
        randc.y = rand() % Y_DIM;
        
        if(area[randc.y][randc.x] != 0)
            continue;
        
        redo = 0;
        for(i = 0; i < e->count; ++i){
            dx = abs(e->list[i].pos.x - randc.x);
            dy = abs(e->list[i].pos.y - randc.y);
            
            if(    (dx <= 1 && dy <= 1)
                || (dx == 0 && dy == 2)
                || (dx == 2 && dy == 0) ){
                
                redo = 1;
                break;
            }
        }
        
        if(redo)
            continue;
        
        break;
    }
    
    area[randc.y][randc.x] = PLAYER_CHAR;
    p->pos = randc;
}

int try_move( int dir, coord_t* pos, game_field_t area, char moved_char, int reset_previous_pos ){
    coord_t newpos = *pos;
    int is_outside = 0;
    int ret = 0;
    
    switch(dir){
        case KEY_LEFT:
            newpos.x--;
            if(newpos.x < 0)
                is_outside = 1;
            break;
        case KEY_UP:
            newpos.y--;
            if(newpos.y < 0)
                is_outside = 1;
            break;
        case KEY_RIGHT:
            newpos.x++;
            if(newpos.x >= X_DIM)
                is_outside = 1;
            break;
        case KEY_DOWN:
            newpos.y++;
            if(newpos.y >= Y_DIM)
                is_outside = 1;
            break;
    }
    
    if(is_outside)
        return 1;
    
    switch(area[newpos.y][newpos.x]){
        case WALL_CHAR:
            return 50;
        case OBSTICAL_CHAR:
            return 51;
        case BOMB_CHAR:
            return 52;
        case I_FIRE_CHAR:
            ret = ITEM_FIRE;
            break;
        case I_MORE_BOMBS_CHAR:
            ret = ITEM_MORE_BOMBS;
            break;
        case PLAYER_CHAR:
            return 100;
        case ENEMY_CHAR:
            return 101;
        case 0:
            break;
        default:
            fprintf(stderr, "Shouldn't happen!\n");
    }
    
    if(reset_previous_pos)
        area[pos->y][pos->x] = 0;
    area[newpos.y][newpos.x] = moved_char;
    *pos = newpos;
    
    return ret;
}

int move_enemies( enemies_t* e, game_field_t area ){
    int i;
    int dir;
    
    for(i = 0; i < e->count; ++i){
        switch(rand()%55){
            case 1:
                dir = KEY_LEFT;
                break;
            case 2:
                dir = KEY_UP;
                break;
            case 3:
                dir = KEY_RIGHT;
                break;
            case 4:
                dir = KEY_DOWN;
                break;
            default:
                continue;
        }
        
        if(try_move(dir, &e->list[i].pos, area, ENEMY_CHAR, 1) == 100)
            return 100;
    }
    
    return 0;
}

int try_place_bomb( game_field_t area, player_t* p, bombs_t* b ){
    bomb_t *bomb;
    
    if(b->count >= p->max_bombs)
        return 1;
    if(p->is_over_bomb)
        return 2;
    
    area[p->pos.y][p->pos.x] = BOMB_CHAR;
    bomb = &b->list[b->count];
    bomb->pos = p->pos;
    bomb->timer = clock_gettime_dbl();
    bomb->radius = p->blast_radius;
    bomb->detonating = 0;
    b->count++;
    p->is_over_bomb = 1;
    
    return 0;
}

int detonate_bombs( double t, bombs_t* b, game_field_t area, player_t* p, enemies_t* e ){
    int i = 0, j;
    coord_t pos;
    int rad;
    
    while(i < b->count){
        if(t - b->list[i].timer >= BOMB_TIMEOUT + 1){
            area[b->list[i].pos.y][b->list[i].pos.x] = 0;
            if(i < b->count-1)
                b->list[i] = b->list[b->count-1];
            b->count--;
            continue;
        }else if(b->list[i].detonating || t - b->list[i].timer >= BOMB_TIMEOUT){
            b->list[i].detonating = 1;
            pos = b->list[i].pos;
            rad = b->list[i].radius;
            
            if(blast_on_coord(pos.y, pos.x, area, p, e, b) == 100)
                return 100;
            
            for(j = 1; j <= rad; ++j){
                if( pos.x-j < 0  || area[pos.y][pos.x-j] == WALL_CHAR )
                    break;
                if(blast_on_coord(pos.y, pos.x-j, area, p, e, b) == 100)
                    return 100;
            }
            for(j = 1; j <= rad; ++j){
                if( pos.x+j >= X_DIM  || area[pos.y][pos.x+j] == WALL_CHAR )
                    break;
                if(blast_on_coord(pos.y, pos.x+j, area, p, e, b) == 100)
                    return 100;
            }
            for(j = 1; j <= rad; ++j){
                if( pos.y-j < 0  || area[pos.y-j][pos.x] == WALL_CHAR )
                    break;
                if(blast_on_coord(pos.y-j, pos.x, area, p, e, b) == 100)
                    return 100;
            }
            for(j = 1; j <= rad; ++j){
                if( pos.y+j >= Y_DIM || area[pos.y+j][pos.x] == WALL_CHAR )
                    break;
                if(blast_on_coord(pos.y+j, pos.x, area, p, e, b) == 100)
                    return 100;
            }
        }
        i++;
    }
    
    return 0;
}

int blast_on_coord( int y, int x, game_field_t area, player_t* p, enemies_t* e, bombs_t* b ){
    int i;

    if(x == p->pos.x && y == p->pos.y)
        return 100;
    
    switch(area[y][x]){
        case ENEMY_CHAR:
            area[y][x] = 0;
            for(i = 0; i < e->count-1; ++i){
                if(e->list[i].pos.x == x && e->list[i].pos.y == y){
                    e->list[i] = e->list[e->count-1];
                    break;
                }
            }
            e->count--;
            break;
            
        case BOMB_CHAR:
            for(i = 0; i < b->count; ++i){
                if(b->list[i].pos.x == x && b->list[i].pos.y == y){
                    b->list[i].detonating = 1;
                    break;
                }
            }
            break;
            
        case OBSTICAL_CHAR:
            area[y][x] = leave_item();
            break;
            
        /*
        case I_FIRE_CHAR:
        case I_MORE_BOMBS_CHAR:
            area[y][x] = 0;
            break;
        */
    }
    
    return 0;
}

int leave_item(void){
    if(rand()%3 == 0){
        switch(rand()%NUM_ITEMS){
            case 0:
                return I_FIRE_CHAR;
                break;
                
            case 1:
                return I_MORE_BOMBS_CHAR;
                break;
                
            default:
                fprintf(stderr, "Shouldn't happen!\n");
        }
    }   
    
    return 0;
}

void apply_item( item_t item, player_t* p ){
    switch(item){
        case ITEM_FIRE:
            p->blast_radius++;
            break;
            
        case ITEM_MORE_BOMBS:
            p->max_bombs++;
            break;
            
        default:
            fprintf(stderr, "Shouldn't happen!\n");
    }
}

void paint_border(WINDOW* win){
    int i;

    for(i = 0; i < Y_DIM+2; ++i){
        mvwaddch(win, i, 0, BORDER_CHAR);
        mvwaddch(win, i, X_DIM+1, BORDER_CHAR);
    }
    
    for(i = 0; i < X_DIM+2; ++i){
        mvwaddch(win, 0, i, BORDER_CHAR);
        mvwaddch(win, Y_DIM+1, i, BORDER_CHAR);
    }
}

void paint_area( WINDOW* win, game_field_t area ){
    int i, j;
    
    for(j = 0; j < Y_DIM; ++j){
        for(i = 0; i < X_DIM; ++i){
            if(area[j][i] > 0)
                mvwaddch(win, j+1, i+1, area[j][i]);
        }
    }
}

void paint_player( WINDOW* win, player_t* p ){
    mvwaddch(win, p->pos.y+1, p->pos.x+1, p->is_over_bomb ? PLAYER_BOMB_CHAR : PLAYER_CHAR);
}

void paint_enemies( WINDOW* win, enemies_t* e ){
    int i;
    
    for(i = 0; i < e->count; ++i){
        mvwaddch(win, e->list[i].pos.y+1, e->list[i].pos.x+1, ENEMY_CHAR);
    }
}

void paint_bombs( WINDOW* win, bombs_t* b, game_field_t area ){
    int i, j, rad;
    coord_t pos;
    
    for(i = 0; i < b->count; ++i){
        if(!b->list[i].detonating){
            mvwaddch(win, b->list[i].pos.y+1, b->list[i].pos.x+1, BOMB_CHAR);
        }else{
            pos = b->list[i].pos;
            rad = b->list[i].radius;
            mvwaddch(win, pos.y+1, pos.x+1, '+');
            
            for(j = 1; j <= rad; ++j){
                if( pos.x-j < 0  || area[pos.y][pos.x-j] == WALL_CHAR )
                    break;
                mvwaddch(win, pos.y+1, pos.x-j+1, ACS_HLINE);
            }
            for(j = 1; j <= rad; ++j){
                if( pos.x+j >= X_DIM  || area[pos.y][pos.x+j] == WALL_CHAR )
                    break;
                mvwaddch(win, pos.y+1, pos.x+j+1, ACS_HLINE);
            }
            for(j = 1; j <= rad; ++j){
                if( pos.y-j < 0  || area[pos.y-j][pos.x] == WALL_CHAR )
                    break;
                mvwaddch(win, pos.y-j+1, pos.x+1, ACS_VLINE);
            }
            for(j = 1; j <= rad; ++j){
                if( pos.y+j >= Y_DIM || area[pos.y+j][pos.x] == WALL_CHAR )
                    break;
                mvwaddch(win, pos.y+j+1, pos.x+1, ACS_VLINE);
            }
        }
    }
}

void repaint_all( WINDOW* win, game_field_t area, player_t* p,
                    enemies_t* e, bombs_t* b ){
    wclear(win);
    paint_border(win);
    paint_area(win, area);
    paint_enemies(win, e);
    paint_bombs(win, b, area);
    paint_player(win, p);
}


double clock_gettime_dbl(){
    struct timespec ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1.0e9;
}
