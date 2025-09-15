#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include "gameboard.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, bool isTwoPlayerMode = false);
    ~MainWindow();

public slots:
    void saveGame();
    void loadGame();
    void loadGame(const QString &fileName);  // 新增加的方法

private:
    Ui::MainWindow *ui;
    GameBoard *gameBoard;
    bool isTwoPlayerMode;
};

#endif // MAINWINDOW_H
