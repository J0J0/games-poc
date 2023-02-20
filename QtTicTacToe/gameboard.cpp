#include "gameboard.hpp"
#include "gamefield.hpp"

#include <QGridLayout>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QPoint>
#include <QPainter>
#include <QPen>


GameBoard::GameBoard() : player(GameField::P1){
    int i, j;
    GameField *gf;
    
    grid_layout = new QGridLayout;
    
    for(i=0; i < 3; ++i){
        for(j=0; j < 3; ++j){
            gf = new GameField(this);
            grid_layout->addWidget(gf, i, j);
            connect( gf, SIGNAL(field_set()), this, SLOT(next_player()) );
        }
    }
    
    setLayout(grid_layout);
    
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(2);
}

void GameBoard::next_player(){
    if(player == GameField::P1)
        player = GameField::P2;
    else
        player = GameField::P1;
}


void GameBoard::paintEvent(QPaintEvent *event){
    QPen pen;
    QPoint p0, p1;
    QPainter painter(this);
    
    pen = painter.pen();
    pen.setWidth(3);
    painter.setPen(pen);
    
    p0 = grid_layout->cellRect(0,0).topRight();
    p1 = grid_layout->cellRect(2,0).bottomRight();
    
    painter.drawLine(p0, p1);
    
    p0 = grid_layout->cellRect(0,1).topRight();
    p1 = grid_layout->cellRect(2,1).bottomRight();

    painter.drawLine(p0, p1);

    p0 = grid_layout->cellRect(0,0).bottomLeft();
    p1 = grid_layout->cellRect(0,2).bottomRight();
    
    painter.drawLine(p0, p1);
    
    p0 = grid_layout->cellRect(1,0).bottomLeft();
    p1 = grid_layout->cellRect(1,2).bottomRight();
    
    painter.drawLine(p0, p1);
}

/* 
void GameBoard::resizeEvent(QResizeEvent* event){
    int w, h;
    w = event->size().width();
    h = event->size().height();
    if(w > h){
        setMaximumWidth(h);
    }
    else
        setMaximumWidth(16777215);
    event->accept();
}
*/
