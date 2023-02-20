#include <stdlib.h>     // srand() / rand()
//#include <string.h>     // memset(), include if the prototype below won't work
extern void* memset (void*, int, size_t);
#include <time.h>       // time() and clock_gettime()
#include <ncurses.h>

#define BORDER_CHAR         '#'
#define PLACED_TILE_CHAR    'x'
#define ACTIVE_TILE_CHAR    'o'

#define Y_DIM 30
#define X_DIM 20

#define GAME_SPEED_INTERVAL 0.18   // as double in sec


typedef struct{
    int y;
    int x;
} coord_t;

typedef unsigned int mask_t;

typedef struct{
    mask_t mask;
    coord_t pos;
} tile_t;

typedef int game_field_t[Y_DIM][X_DIM];

mask_t rotation_bits[] = { 
    0x1000,
    0x0100,
    0x0010,
    0x0001,
    
    0x2000,
    0x0200,
    0x0020,
    0x0002,
    
    0x4000,
    0x0400,
    0x0040,
    0x0004,

    0x8000,
    0x0800,
    0x0080,
    0x0008
};


#define N_TILE_TYPES 7

mask_t tile_types[] = {
    0x0660,     // block
    0x6220,     // L
    0x6440,     // L mirrored
    0x2222,     // |
    0x2640,     // N
    0x4620,     // N mirrored
    0x0720      // arrow keys
};



double clock_gettime_dbl(void);
void new_tile(tile_t* active_tile);
int move_tile(int dir, tile_t* active_tile, game_field_t area);
void place_tile(tile_t* active_tile, game_field_t area);
int rotate_tile(tile_t* active_tile, game_field_t area);
void check_lines_complete(game_field_t area);
void paint_border(WINDOW* win);
void paint_area( WINDOW* win, game_field_t area );
void paint_atile( WINDOW* win, tile_t* active_tile );
void repaint_all( WINDOW* win, game_field_t area, tile_t* active_tile );


int main(void){
    int retry, running;
    int c;
    double t0, t1;
    game_field_t area;
    tile_t active_tile;
    
    memset(area, 0, X_DIM*Y_DIM*sizeof(area[0][0]));
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    srand(time(NULL));
    
    retry = 1;
    
    while(retry){
        new_tile(&active_tile);
        repaint_all(stdscr, area, &active_tile);
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
                case KEY_RIGHT:
                    move_tile(c, &active_tile, area);
                    break;
                case KEY_UP:
                    rotate_tile(&active_tile, area);
                    break;
                case 'q':
                    running = 0;
                    retry   = 0;
                    break;
            }
            
            t0 = clock_gettime_dbl();
            if( (t1-t0) <= 0 ){
                if(!move_tile(KEY_DOWN, &active_tile, area)){
                    place_tile(&active_tile, area);
                    check_lines_complete(area);
                    new_tile(&active_tile);
                }
                repaint_all(stdscr, area, &active_tile);
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


void new_tile(tile_t* active_tile){
    int k = rand() % N_TILE_TYPES;
    
    active_tile->mask = tile_types[k];
    active_tile->pos.y = 0;
    active_tile->pos.x = X_DIM/2;
}

int move_tile(int dir, tile_t* active_tile, game_field_t area){
    int i, j;
    coord_t tpos = active_tile->pos;
    mask_t mask = active_tile->mask;
    mask_t testbit = 1;
    
    switch(dir){
        case KEY_LEFT:
            tpos.x--;
            
            if(     ( (tpos.x   < 0) && ((0x1111 & mask) > 0) )
                ||  ( (tpos.x+1 < 0) && ((0x2222 & mask) > 0) )
                ||  ( (tpos.x+2 < 0) && ((0x4444 & mask) > 0) )
                ||  ( (tpos.x+3 < 0) )
              ){
                return 0;
            }
            
            break;
        case KEY_RIGHT:
            tpos.x++;
            
            if(     ( (tpos.x+3 >= X_DIM) && ((0x8888 & mask) > 0) )
                ||  ( (tpos.x+2 >= X_DIM) && ((0x4444 & mask) > 0) )
                ||  ( (tpos.x+1 >= X_DIM) && ((0x2222 & mask) > 0) )
                ||  (  tpos.x   >= X_DIM  )
              ){
                return 0;
            }
            
            break;
        case KEY_DOWN:
            tpos.y++;
            
            if(     ( (tpos.y+3 >= Y_DIM) && ((0xF000 & mask) > 0) )
                ||  ( (tpos.y+2 >= Y_DIM) && ((0x0F00 & mask) > 0) )
                ||  ( (tpos.y+1 >= Y_DIM) && ((0x00F0 & mask) > 0) )
                ||  (  tpos.y   >= Y_DIM  )
              ){
                return 0;
            }
            
            break;
    }
    
    for(i=0; i<4; ++i){
        for(j=0; j<4; ++j){
            if( ((testbit & active_tile->mask) > 0) && (area[tpos.y+i][tpos.x+j] > 0) )
                return 0;
            
            testbit <<= 1;
        }
    }
    
    active_tile->pos = tpos;
    
    return 1;
}

void place_tile(tile_t* active_tile, game_field_t area){
    int i, j;
    coord_t tpos = active_tile->pos;
    mask_t testbit = 1;
    
    for(i=0; i<4; ++i){
        for(j=0; j<4; ++j){
            if((testbit & active_tile->mask) > 0)
                area[tpos.y+i][tpos.x+j] = 1;
            
            testbit <<= 1;
        }
    }
    
}


int rotate_tile(tile_t* active_tile, game_field_t area){
    int i, j;
    mask_t new_mask = 0;
    coord_t tpos = active_tile->pos;
    mask_t testbit = 1;
    
    for(i=0; i<16; ++i){
        if( ((1<<i) & active_tile->mask) > 0 )
            new_mask |= rotation_bits[i];
    }
    
    for(i=0; i<4; ++i){
        for(j=0; j<4; ++j){
            if( ((testbit & new_mask) > 0) && (area[tpos.y+i][tpos.x+j] > 0) )
                return 0;
            
            testbit <<= 1;
        }
    }
    
    active_tile->mask = new_mask;
    
    return 1;
}

void check_lines_complete(game_field_t area){
    int i, j, k;
    int complete;
   
    i = Y_DIM-1;
    while(i >= 0){
        complete = 1;
        for(j = 0; j < X_DIM; ++j){
            if(area[i][j] == 0){
                complete = 0;
                break;
            }
        }
        if(complete){
            for(k = i-1; k >= 0; --k){
                for(j = 0; j < X_DIM; ++j){
                    area[k+1][j] = area[k][j];
                }
            }
            for(j = 0; j < X_DIM; ++j)
                area[0][j] = 0;
        } else {
            --i;
        }
    }
}

void paint_border(WINDOW* win){
    int i;

    for(i = 0; i < Y_DIM; ++i){
        mvwaddch(win, i, 0, BORDER_CHAR);
        mvwaddch(win, i, X_DIM+1, BORDER_CHAR);
    }
}

void paint_area( WINDOW* win, game_field_t area ){
    int i, j;
    
    for(i = 0; i < Y_DIM; ++i){
        for(j = 0; j < X_DIM; ++j){
            if(area[i][j] > 0)
                mvwaddch(win, i, j+1, PLACED_TILE_CHAR);
        }
    }
}

void paint_atile( WINDOW* win, tile_t* active_tile ){
    coord_t tpos = active_tile->pos;
    int i, j;
    mask_t testbit = 1;
    
    for(i=0; i<4; ++i){
        for(j=0; j<4; ++j){
            if((testbit & active_tile->mask) > 0)
                mvwaddch(win, tpos.y+i, tpos.x+j+1, ACTIVE_TILE_CHAR);
            testbit <<= 1;
        }
    }
    
}

void repaint_all( WINDOW* win, game_field_t area, tile_t* active_tile ){
    wclear(win);
    paint_border(win);
    paint_area(win, area);
    paint_atile(win, active_tile);
}


double clock_gettime_dbl(){
    struct timespec ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1.0e9;
}
