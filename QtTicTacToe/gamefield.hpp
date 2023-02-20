#ifndef GAMEFIELD_H
#define GAMEFIELD_H

class GameBoard;

#include <QWidget>


class GameField : public QWidget{
    Q_OBJECT
        
    public:
        enum state_t { NONE, P1, P2 };

        GameField(GameBoard* gb);
        QSize sizeHint();
        int heightForWidth(int w) const;
        
    public slots:
        void reset();
        
    signals:
        void field_set();
        
    protected:
        void mousePressEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *event);
        
    private:
        state_t state;
        GameBoard *game_board;
        
        void try_set_player(state_t pl);
};

#endif
