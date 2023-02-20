#include "gameboard.hpp"
#include "gamefield.hpp"

#include <QSizePolicy>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>


GameField::GameField(GameBoard* gb) : state(NONE), game_board(gb){
    QSizePolicy sp;
    
    setMinimumSize(100,100);
    
    sp.setHorizontalPolicy(QSizePolicy::Preferred);
    sp.setVerticalPolicy(  QSizePolicy::Minimum);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

void GameField::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        try_set_player(game_board->player);
    }
}

void GameField::try_set_player(state_t pl){
    if(state == NONE){
        state = pl;
        update();
        emit field_set();
    }
}

void GameField::reset(){
    state = NONE;
    update();
}

void GameField::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    
    if(state == P1){
        painter.drawLine(6,6,width()-6,height()-6);
        painter.drawLine(width()-6,6,6,height()-6);
    }else if(state == P2){
        painter.drawEllipse(6,6,width()-12,height()-12);
    }else{
        // don't paint anything
    }
}

QSize GameField::sizeHint(){
    return QSize(100,100);
}

int GameField::heightForWidth(int w) const{
    return w;
}
