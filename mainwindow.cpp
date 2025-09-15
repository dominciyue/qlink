#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QAction>

MainWindow::MainWindow(QWidget *parent, bool isTwoPlayerMode)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isTwoPlayerMode(isTwoPlayerMode)
{
    ui->setupUi(this);

    gameBoard = new GameBoard(this, isTwoPlayerMode);
    gameBoard->setFocusPolicy(Qt::StrongFocus);
    gameBoard->setFocus();
    setCentralWidget(gameBoard);
    resize(800, 600);

    // 添加保存和加载游戏的按钮
    QAction *saveAction = new QAction("保存游戏", this);
    QAction *loadAction = new QAction("载入游戏", this);

    // 使用 lambda 表达式连接信号和槽
    connect(saveAction, &QAction::triggered, this, [this]() { saveGame(); });
    connect(loadAction, &QAction::triggered, this, [this]() { loadGame(); });

    ui->menubar->addAction(saveAction);
    ui->menubar->addAction(loadAction);
}

void MainWindow::saveGame()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存游戏", "", "游戏存档 (*.sav)");
    if (!fileName.isEmpty()) {
        gameBoard->saveGame(fileName);
    }
}

void MainWindow::loadGame()
{
    QString fileName = QFileDialog::getOpenFileName(this, "载入游戏", "", "游戏存档 (*.sav)");
    if (!fileName.isEmpty()) {
        loadGame(fileName);
    }
}

void MainWindow::loadGame(const QString &fileName)
{
    gameBoard->loadGame(fileName);
}

MainWindow::~MainWindow()
{
    delete ui;
}
