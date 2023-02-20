#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <QFrame>
#include <QGridLayout>
#include <QResizeEvent>

#include "gamefield.hpp"


//class GameBoard : public QWidget{
class GameBoard : public QFrame{
    Q_OBJECT
        
    public:
        QGridLayout *grid_layout;

        GameBoard();
        GameField::state_t player;
        
    public slots:
        void next_player();
    
    protected:
        //void resizeEvent(QResizeEvent *event);
        void paintEvent(QPaintEvent *event);
};

#endif
