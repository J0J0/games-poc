#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QObject>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "gameboard.hpp"


int main(int argc, char *argv[])
{
    int i;
    QApplication app(argc, argv);
    QMainWindow  mainwin;
    QWidget      *cwid  = new QWidget;
    QHBoxLayout  *hbox  = new QHBoxLayout;
    QVBoxLayout  *vbox  = new QVBoxLayout;
    GameBoard    *board = new GameBoard;
    QPushButton  *reset_button = new QPushButton("reset");
    
    mainwin.setCentralWidget(cwid);
    
    vbox->addStretch();
    vbox->addWidget(board, 0);
    vbox->addStretch();
    
    hbox->addWidget(reset_button, 0);
    hbox->addSpacing(20);
    hbox->addLayout(vbox, 1);
    
    cwid->setLayout(hbox);
    
    for(i=0; i < board->grid_layout->count(); ++i)
        QObject::connect( reset_button, SIGNAL(clicked()), 
                          board->grid_layout->itemAt(i)->widget(), SLOT(reset()) );
    
    mainwin.show();
    mainwin.resize(500,400);  // FIXME: need this to make cellRect useable
                              //        (see QGridLayout::cellRect doc)
    mainwin.resize(400,400);
    
    return app.exec();
}
