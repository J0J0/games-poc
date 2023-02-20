#include <stdlib.h>
#include <time.h>
#include <curses.h>

#define BORDER_CHAR '#'
#define HEAD_CHAR   'O'
#define BODY_CHAR   'o'
#define FOOD_CHAR   '+'

#define SNAKE_MAX_LEN       200
#define MAX_FOOD            30
#define GAME_SPEED_INTERVAL 0.18   // as double in sec


typedef struct{
    int y;
    int x;
} coord_t;

typedef struct{
    int len;
    int head_pos;
    coord_t body[SNAKE_MAX_LEN];
} snake_t;

typedef struct{
    int count;
    coord_t list[MAX_FOOD];
} food_t;


double clock_gettime_dbl(void);
void init_snake(snake_t* snake, int len, coord_t* start, coord_t* end);
void init_food(food_t* food, int init_count, 
                snake_t* snake, coord_t* start, coord_t* end);
void add_food(food_t* food, snake_t* snake, coord_t* start, coord_t* end);
void remove_food(food_t* food, int key);
void move_snake(snake_t* snake, int direction, coord_t* start, coord_t* end);
int check_snake(snake_t* snake, food_t* food, coord_t* start, coord_t* end);
void dir_change(int newdir, int* olddir);
void paint_border(WINDOW* win, coord_t* start, coord_t* end);
void paint_food(WINDOW* win, food_t* food);
void paint_snake(WINDOW* win, snake_t* snake, coord_t* start, coord_t* end);
void repaint_all( WINDOW* win, snake_t* snake, food_t* food, 
                    coord_t* start, coord_t* end);


int main(void){
    int retry, running;
    coord_t start, end;
    snake_t snake;
    food_t  food;
    int c;
    int curdir, newdir;
    double t0, t1;
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    start.x = 1;
    start.y = 1;
    end.x   = 80;
    end.y   = 30;
    
    srand(time(NULL));
    
    retry = 1;
    
    while(retry){
        init_snake(&snake, 3, &start, &end);
        init_food(&food, 5, &snake, &start, &end);
        
        paint_border(stdscr, &start, &end);
        paint_snake(stdscr, &snake, &start, &end);
        paint_food(stdscr, &food);
        refresh();
        
        timeout(-1);
        c = getch();
        ungetch(c);
        
        curdir = newdir = KEY_DOWN;
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
                case 'q':
                    running = 0;
                    retry   = 0;
                    break;
            }
            
            t0 = clock_gettime_dbl();
            if( (t1-t0) <= 0 ){
                dir_change(newdir, &curdir);
                move_snake(&snake, curdir, &start, &end);
                if(check_snake(&snake, &food, &start, &end)){
                    clear();
                    running = 0;
                    break;
                }
                repaint_all(stdscr, &snake, &food, &start, &end);
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


void init_snake(snake_t* snake, int len, coord_t* start, coord_t* end){
    int i, y;
    coord_t middle;

    middle.x = (end->x - start->x)/2;
    middle.y = (end->y - start->y)/2;
    
    snake->len      = len;
    snake->head_pos = len-1;
    
    y = middle.y - len/2;
    for(i=0; i<len; ++i, ++y){
        (snake->body[i]).x = middle.x;
        (snake->body[i]).y = y;
    }
}


void init_food(food_t* food, int init_count, 
                snake_t* snake, coord_t* start, coord_t* end){
    int i;
    
    food->count = 0;
    
    for(i=0; i < init_count; ++i)
        add_food(food, snake, start, end);
}


void add_food(food_t* food, snake_t* snake, coord_t* start, coord_t* end){
    int i, k, redo;
    coord_t randc;
    
    while(1){
        redo = 0;
        
        randc.x = start->x + rand() % (end->x-start->x);
        randc.y = start->y + rand() % (end->y-start->y);
        
        for(i=0; i < snake->len; ++i){
            k = (snake->head_pos + SNAKE_MAX_LEN - i) % SNAKE_MAX_LEN;
            if( randc.x == snake->body[k].x &&
                randc.y == snake->body[k].y     ){
                redo = 1;
                break;
            }
        }
        
        if(redo)
            continue;
        
        for(i=0; i < food->count; ++i){
            if( randc.x == food->list[i].x &&
                randc.y == food->list[i].y  ){
                redo = 1;
                break;
            }
        }
        
        if(redo)
            continue;
        
        break;
    }
    
    food->list[food->count] = randc;
    food->count++;
}


void remove_food(food_t* food, int key){
    if(key != food->count-1)
        food->list[key] = food->list[food->count-1];
    food->count--;
}


void move_snake(snake_t* snake, int direction, coord_t* start, coord_t* end){
    coord_t *last_head = snake->body + snake->head_pos;
    
    snake->head_pos++;
    if(snake->head_pos == SNAKE_MAX_LEN)
        snake->head_pos = 0;
    
    snake->body[snake->head_pos] = *last_head;
    
    switch(direction){
        case KEY_LEFT:
            snake->body[snake->head_pos].x--;
            break;
        case KEY_UP:
            snake->body[snake->head_pos].y--;
            break;
        case KEY_RIGHT:
            snake->body[snake->head_pos].x++;
            break;
        case KEY_DOWN:
            snake->body[snake->head_pos].y++;
            break;
    }
}


int check_snake(snake_t* snake, food_t* food, coord_t* start, coord_t* end){
    int i, k;
    coord_t *head = snake->body + snake->head_pos;
    
    if(head->x < start->x || head->x > end->x)
        return 1;

    if(head->y < start->y || head->y > end->y)
        return 2;
    
    for(i=1; i < snake->len; ++i){
        k = (snake->head_pos + SNAKE_MAX_LEN - i) % SNAKE_MAX_LEN;
        if( head->x == snake->body[k].x &&
            head->y == snake->body[k].y     )
            return 3;
    }
    
    for(i=0; i < food->count; ++i){
        if(         food->list[i].x == head->x
                &&  food->list[i].y == head->y ){
            snake->len++;
            remove_food(food, i);
            add_food(food, snake, start, end);
            break;
        }
    }
    
    return 0;
}


void dir_change(int newdir, int* olddir){
    if(     (newdir == KEY_LEFT  && *olddir == KEY_RIGHT) 
        ||  (newdir == KEY_RIGHT && *olddir == KEY_LEFT )
        ||  (newdir == KEY_UP    && *olddir == KEY_DOWN )
        ||  (newdir == KEY_DOWN  && *olddir == KEY_UP   )
      ){
        return;
    }
    
    *olddir = newdir;
}


void paint_border(WINDOW* win, coord_t* start, coord_t* end){
    int i;
    
    for(i = start->x - 1; i <= end->x + 1; ++i){
        mvwaddch(win, start->y-1, i, BORDER_CHAR);
        mvwaddch(win, end->y  +1, i, BORDER_CHAR);
    }
    
    for(i = start->y; i <= end->y; ++i){
        mvwaddch(win, i, start->x-1, BORDER_CHAR);
        mvwaddch(win, i, end->x  +1, BORDER_CHAR);
    }
}


void paint_food(WINDOW* win, food_t* food){
    int i;
    
    for(i=0; i < food->count; ++i)
        mvwaddch(win, food->list[i].y, food->list[i].x, FOOD_CHAR);
        
}


void paint_snake(WINDOW* win, snake_t* snake, coord_t* start, coord_t* end){
    int i, k;
    
    mvwaddch( win, 
              snake->body[snake->head_pos].y,
              snake->body[snake->head_pos].x,
              HEAD_CHAR );
    
    for(i=1; i < snake->len; ++i){
        k = (snake->head_pos + SNAKE_MAX_LEN - i) % SNAKE_MAX_LEN;
        mvwaddch(win, snake->body[k].y, snake->body[k].x, BODY_CHAR);
    }
}


void repaint_all( WINDOW* win, snake_t* snake, food_t* food, 
                    coord_t* start, coord_t* end){
    wclear(win);
    paint_border(win, start, end);
    paint_food(win, food);
    paint_snake(win, snake, start, end);
}


double clock_gettime_dbl(){
    struct timespec ts;
    
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1.0e9;
}
