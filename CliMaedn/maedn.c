#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __unix__
void cls(void){ system("clear"); }
#else
void cls(void){ for(int i=0;i<150;i++) printf("\n"); }
#endif

void discardline(void){ char c; while( (c=getchar())!='\n' && c != EOF ); }


typedef struct {
    char *name;
    int tokens[4];
    unsigned int home;
} player_t;
#define INIT_TOKENS(p) (p).tokens[0] = -1; \
                       (p).tokens[1] = -1; \
                       (p).tokens[2] = -1; \
                       (p).tokens[3] = -1; 

#define HAS_TOKEN_ON_FIELD(p) ( ((p).tokens[0] >= 0) || \
                                ((p).tokens[1] >= 0) || \
                                ((p).tokens[2] >= 0) || \
                                ((p).tokens[3] >= 0) )

int dice(void){ return 1+rand()%6; }


#define BOARDLEN (4*(5+4+1))

void print_board(char*b){
    cls();
    for(int i=0;i<11;i++){
        printf("%11.11s\n", b+11*i);
    }
    printf("\n");
}

static const unsigned int gb_link[BOARDLEN] = { 4,5,6,17,28,39,50,51,52,53,54,65,76,75,
74,73,72,83,94,105,116,115,114,103,92,81,70,69,68,67,66,55,44,45,46,47,48,37,26,15 };
#define gb(n) gb_all[ gb_link[n] ]
#define gbX(b,n) b[ gb_link[n] ]

static const char pl_tokens[4][4] = { {'1','2','3','4'}, {'a','b','c','d'},
                                      {'A','B','C','D'}, {'p','q','r','s'} };

#define IS_PL_TOKEN(i,c) ( (pl_tokens[i][0] == c) || \
                           (pl_tokens[i][1] == c) || \
                           (pl_tokens[i][2] == c) || \
                           (pl_tokens[i][3] == c) )


void clear_all(char *b){ 
    for(int i=0;i<11*11;i++)
        b[i] = ' ';
}

void paint_board(char *b){ 
    for(int i=0;i<BOARDLEN;i++)
        gbX(b,i) = '#';
}

enum { MOVE_NOT_MOVEABLE = 0, MOVE_REGULAR, MOVE_LEAVE_BASE, MOVE_ENTER_HOME };

int token_moveable(int tok_i, int pl_i, int steps, char *b, player_t *pl){
    int tok, move;

    tok = pl[pl_i].tokens[tok_i];
    if(tok == -2) // token already home
        return MOVE_NOT_MOVEABLE;
    else if(tok == -1){ // token at base
        if(steps!=6)
            return MOVE_NOT_MOVEABLE;
        else{
            move = 2+10*pl_i;
            if(IS_PL_TOKEN(pl_i,gbX(b,move)))
                return MOVE_NOT_MOVEABLE;
            else
                return MOVE_LEAVE_BASE;
        }
    }else{ // tok >= 0
        move = (tok + steps) % BOARDLEN;
        
        if( (move > 1+10*pl_i) // move would go beyond starting point
            && ((BOARDLEN+tok-(2+10*pl_i)) % BOARDLEN >= 34) // token near home
        ){ // can go home?
            move -= 2+10*pl_i;
            if(move > 3) // too far
                return MOVE_NOT_MOVEABLE;
            else if(pl[pl_i].home & (1<<move)) // home place already taken
                return MOVE_NOT_MOVEABLE;
            else // can go in
                return MOVE_ENTER_HOME;
        }else{ // check regular move
            if(IS_PL_TOKEN(pl_i,gbX(b,move)))
                return MOVE_NOT_MOVEABLE;
            else
                return MOVE_REGULAR;
        }
    }
}

void move_token(char *b, player_t *pl, int pl_count, int pl_i, int tok_i, int steps){
// assumes that the arguments induce a valid move (check with above function first!)
    int j, k, tok, move;
    char c_tok, c_move;
    tok = pl[pl_i].tokens[tok_i];
    if(tok == -1)
        move = 2+10*pl_i;
    else // tok >= 0
        move = (tok + steps) % BOARDLEN;
    
    c_tok  = pl_tokens[pl_i][tok_i];
    c_move = gbX(b,move);
    
    if(gbX(b,move) != '#'){ // throw out other player's token
        for(j=0; c_move!=c_tok && j<pl_count; j++){
            if(j == pl_i)
                continue;
            
            for(k=0; k<4; k++){
                if(pl[j].tokens[k] == move){
                    pl[j].tokens[k] = -1;
                    c_move = c_tok;
                    break;
                }
            }
        }
    }
    
    if(tok >= 0)
        gbX(b,tok) = '#';
    gbX(b,move) = c_tok;
    
    pl[pl_i].tokens[tok_i] = move;
}

void move_token_home(char *b, player_t *pl, int pl_i, int tok_i, int steps){
// assumes that the arguments induce a valid move (check with above function first!)
    int k, tok;
    
    tok = pl[pl_i].tokens[tok_i];
    k = (tok + steps) % BOARDLEN - (2+10*pl_i);
    
    pl[pl_i].home |= (1<<k);
    gbX(b,tok) = '#';
    pl[pl_i].tokens[tok_i] = -2;
}

// expects the names of the players as command line arguments
int main(int argc, char *pl_names[]){
    int i, k;
    int pl_count = argc-1;
    int playing, retry, d, t;
    char gb_all[11*11];
    player_t p[4];
    int moveable[4], mcount, m_k;
    
    if(pl_count < 2)
        return 1;
    
    srand(time(NULL));

    for(i=0; i<pl_count; i++){
        p[i].name = pl_names[i+1];
        INIT_TOKENS(p[i]);
        p[i].home = 0;
    }
    
    clear_all(gb_all);
    paint_board(gb_all);
    
    print_board(gb_all);
    
    playing = 1;
    i = 0;
    while(playing){
        retry = 0;
        do{
            printf("player %s, roll the dice.", p[i].name);
            discardline();
            d = dice();
            printf("player %s got a %d.\n", p[i].name, d);
        }while( (d != 6) && (!HAS_TOKEN_ON_FIELD(p[i])) && (++retry != 3) );
        
        for(k=3, mcount=0; k>=0; k--){
            moveable[k] = token_moveable(k, i, d, gb_all, p);
            if(moveable[k]){
                mcount++;
                m_k = k;
            }
        }
        
        if(mcount == 0){
            printf("player %s can't do anything.\n", p[i].name);
            discardline();
            i = (i+1) % pl_count;
            continue;
        }else if(mcount == 1){ // only one choice, don't ask
            t = m_k;
            printf("player %s can only move '%c'.\n", p[i].name, pl_tokens[i][t]);
            discardline();
        }else{
            printf("Choose a token: ");
            while(1){
                do{
                    scanf("%d", &t);
                    discardline();
                }while(t<1 || t>4);
                t--;
                    
                if(moveable[t])
                    break;
                
                printf("invalid move, choose again: ");
            }
        }
        
        switch(moveable[t]){
            case MOVE_REGULAR:
                move_token(gb_all, p, pl_count, i, t, d);
                break;
            case MOVE_LEAVE_BASE:
                move_token(gb_all, p, pl_count, i, t, 0);
                break;
            case MOVE_ENTER_HOME:
                move_token_home(gb_all, p, i, t, d);
                if(p[i].home == 0x0F){
                    playing = 0;
                    d = 6; // this is kind of a hack to leave
                           // the main loop without touching i
                }
                break;
                
            case MOVE_NOT_MOVEABLE:
            default:
                // should never happen
                return 2;
        }
        
        print_board(gb_all);
        
        if(d!=6)
            i = (i+1) % pl_count;
    }
    
    printf("player %s wins!\n\n", p[i].name);
    
    return 0;
}
