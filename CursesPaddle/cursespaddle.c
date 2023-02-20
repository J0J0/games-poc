#include <stdlib.h>     // srand() / rand() / abs()
//#include <string.h>     // memset(), include if the prototype below won't work
extern void* memset (void*, int, size_t);
#include <time.h>       // time() and clock_gettime()
#include <ncurses.h>

#define PADDLE_CHAR         ACS_VLINE // defined by ncurses
#define BALL_CHAR           'O'

#define P0_KEY_UP       'q'
#define P0_KEY_DOWN     'a'
#define P1_KEY_UP       KEY_UP      // defined by ncurses
#define P1_KEY_DOWN     KEY_DOWN    // defined by ncurses


#define GAME_SPEED_INTERVAL 0.05   // as double in sec


typedef struct{
    int y;
    int x;
} coord_t;

typedef struct{
    coord_t pos;
    int width;
} paddle_t;

typedef struct{
    int id;
    int key;
    int moving;
    paddle_t paddle;
    int wins;
} player_t;

#define INIT_PLAYER(p,i,dims) ( p.id = i,               \
                                p.wins = 0,             \
                                RESET_PLAYER(p,dims)    )

#define RESET_PLAYER(p,dims) ( p.key = -1,                                 \
                               p.moving = 0,                               \
                               p.paddle.width = dims.y/10,                 \
                               p.paddle.pos.y = dims.y/2-p.paddle.width/2  )
        

#define P_KEY_TO_INT(i,key) ( i == 0                                        \
                                ? (key == P0_KEY_UP                         \
                                    ? -1 : (key == P0_KEY_DOWN ? 1 : 0))    \
                                : (key == P1_KEY_UP                         \
                                    ? -1 : (key == P1_KEY_DOWN ? 1 : 0)) )

typedef player_t players_t[2];

typedef struct{
    coord_t pos;
    coord_t dir;
} ball_t;


double clock_gettime_dbl(void);
void check_keys(players_t p);
void move_paddles(players_t p, coord_t dims);
int move_ball(ball_t* b, players_t p, coord_t dims);
void paint_paddles( WINDOW* win, players_t p );
void paint_ball( WINDOW* win, ball_t* b );
void paint_border( WINDOW* win, coord_t dims );
void paint_score( WINDOW* win, players_t p, coord_t dims );
void repaint_all( WINDOW* win, players_t p, ball_t* b, coord_t dims );

int main(void){
    int retry, running;
    int c;
    double t0, t1;
    coord_t dims;
    players_t p;
    ball_t b;
    int status;
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    getmaxyx(stdscr, dims.y, dims.x);
    dims.y -= 4;
    srand(time(NULL));
    
    INIT_PLAYER(p[0], 0, dims);
    INIT_PLAYER(p[1], 1, dims);

    retry = 1;
    
    while(retry){
        RESET_PLAYER(p[0], dims);
        RESET_PLAYER(p[1], dims);
        p[0].paddle.pos.x = 0;
        p[1].paddle.pos.x = dims.x-1;
        
        b.pos.y = dims.y/2;
        b.pos.x = dims.x/2;
        b.dir.y = rand()%2 ? -1 : 1;
        b.dir.x = rand()%2 ? -1 : 1;
        
        repaint_all(stdscr, p, &b, dims);
        refresh();
        
        timeout(-1);
        c = getch();
        ungetch(c);
        
        t0 = t1 = clock_gettime_dbl();
        running = 1;
        timeout(0);
        
        while(running){
            c = getch();
            switch(c){
                case P0_KEY_UP:
                case P0_KEY_DOWN:
                    p[0].key = c;
                    break;
                case P1_KEY_UP:
                case P1_KEY_DOWN:
                    p[1].key = c;
                    break;
                case KEY_BACKSPACE:
                    running = 0;
                    retry   = 0;
                    break;
            }
            
            t0 = clock_gettime_dbl();
            if( (t1-t0) <= 0 ){
                check_keys(p);
                move_paddles(p, dims);
                status = move_ball(&b, p, dims);
                if(status != 0){
                    p[ status == -1 ].wins++;
                    running = 0;
                    break;
                }
                
                repaint_all(stdscr, p, &b, dims);
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

void check_keys(players_t p){
    int i;
    int newdir;
    
    for(i = 0; i <= 1; ++i){
        newdir = P_KEY_TO_INT(i, p[i].key);
        if(newdir){
            if(p[i].moving == 0)
                p[i].moving = newdir;
            else if(p[i].moving != newdir)
                p[i].moving = 0;
        }
        p[i].key = ERR;
    }

}
void move_paddles(players_t p, coord_t dims){
    int i, dir;
    
    for(i = 0; i <= 1; ++i){
        dir = p[i].moving;
        if(dir != 0){
            if(    p[i].paddle.pos.y + dir >= 0 
                && p[i].paddle.pos.y + dir + p[i].paddle.width <= dims.y ){
            
                p[i].paddle.pos.y += dir;
            }else{
                p[i].moving = 0;
            }
        }
    }
}

int move_ball(ball_t* b, players_t p, coord_t dims){
    coord_t npos = b->pos;
    coord_t ndir = b->dir;
    
    npos.x += b->dir.x;
    npos.y += b->dir.y;
    
    if(npos.x < 0) 
        return -1;
    else if(npos.x > dims.x)
        return  1;
    else if(    npos.x == p[0].paddle.pos.x
              || npos.x == p[1].paddle.pos.x ){
        
        if(    npos.y >= p[0].paddle.pos.y 
            && npos.y < p[0].paddle.pos.y + p[0].paddle.width ){
            
            npos.x += 2;
            ndir.x = -ndir.x;
        }else if(   npos.y >= p[1].paddle.pos.y 
                 && npos.y < p[1].paddle.pos.y + p[1].paddle.width ){
            
            npos.x -= 2;
            ndir.x = -ndir.x;
        }
    }
    
    if(npos.y < 0){
        npos.y = -npos.y;
        ndir.y = -ndir.y;
    }
    else if(npos.y > dims.y){
        npos.y = 2*dims.y-npos.y;
        ndir.y = -ndir.y;
    }
    
    b->pos = npos;
    b->dir = ndir;
    
    return 0;
}


void paint_paddles( WINDOW* win, players_t p ){
    int i, j;
    coord_t pos;
    
    for(i = 0; i <= 1; ++i){
        pos = p[i].paddle.pos;
        for(j = 0; j < p[i].paddle.width; ++j){
            mvwaddch(win, pos.y+j, pos.x, PADDLE_CHAR);
        }
    }
}

void paint_ball( WINDOW* win, ball_t* b ){
    mvwaddch(win, b->pos.y, b->pos.x, BALL_CHAR);
}

void paint_border( WINDOW* win, coord_t dims ){
    int i;
    int maxy = getmaxy(win);
    
    if(dims.y >= maxy)
        return;
    
    for(i = 0; i < dims.x; ++i){
        mvwaddch(win, dims.y, i, '#');
    }
}

void paint_score( WINDOW* win, players_t p, coord_t dims ){
    int y = getmaxy(win)-1;
    
    if(dims.y >= y)
        return;
    
    if(dims.y <= y-1)
        y--;
    
    mvwaddch(win, y, dims.x/2, '-');
    mvwprintw(win, y, dims.x/4, "%03d", p[0].wins);
    mvwprintw(win, y, (dims.x*3+3)/4, "%03d", p[1].wins);
}

void repaint_all( WINDOW* win, players_t p, ball_t* b, coord_t dims ){
    wclear(win);
    paint_paddles(win, p);
    paint_ball(win, b);
    paint_border(win, dims);
    paint_score(win, p, dims);
}


double clock_gettime_dbl(){
    struct timespec ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1.0e9;
}
