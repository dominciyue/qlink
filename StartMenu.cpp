#include "startmenu.h"
#include "gameboard.h"
#include <QPixmap>
#include <QPalette>
#include <QFileDialog>
#include <QApplication>
StartMenu::StartMenu(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StartMenu::setupUI()
{
    setWindowTitle("连连看 - 主菜单");
    setFixedSize(800, 600);  // 设置窗口大小

    // 设置背景图片
    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(":/back.jpg"));
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(0, 0, width(), height());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    newGameButton = new QPushButton("开始新游戏", this);
    loadGameButton = new QPushButton("载入游戏", this);
    exitButton = new QPushButton("退出游戏", this);

    setButtonStyle(newGameButton, ":/but.png");
    setButtonStyle(loadGameButton, ":/but.png");
    setButtonStyle(exitButton, ":/but.png");

    gameModeComboBox = new QComboBox(this);
    gameModeComboBox->addItem("单人模式");
    gameModeComboBox->addItem("双人模式");

    QHBoxLayout *newGameLayout = new QHBoxLayout();
    newGameLayout->addWidget(newGameButton);
    newGameLayout->addWidget(gameModeComboBox);

    mainLayout->addLayout(newGameLayout);
    mainLayout->addWidget(loadGameButton);
    mainLayout->addWidget(exitButton);

    connect(newGameButton, &QPushButton::clicked, this, &StartMenu::onNewGameClicked);
    connect(loadGameButton, &QPushButton::clicked, this, &StartMenu::onLoadGameClicked);
    connect(exitButton, &QPushButton::clicked, this, &StartMenu::onExitClicked);

    setLayout(mainLayout);
}

void StartMenu::setButtonStyle(QPushButton *button, const QString &iconPath)
{
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(64, 64));  // 设置图标大小
    button->setStyleSheet("QPushButton {"
                          "background-color: transparent;"
                          "border: none;"
                          "color: white;"
                          "font-size: 18px;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: rgba(255, 255, 255, 0.2);"
                          "}");
}

void StartMenu::onNewGameClicked()
{
    QString selectedMode = gameModeComboBox->currentText();
    GameBoard *gameBoard = new GameBoard(nullptr, selectedMode == "双人模式");
    gameBoard->show();
    this->close();
}

void StartMenu::onLoadGameClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "载入游戏", "", "游戏存档 (*.sav)");
    if (!fileName.isEmpty()) {
        GameBoard *gameBoard = new GameBoard(nullptr, false);  // 假设载入的游戏是单人模式
        gameBoard->loadGame(fileName);
        gameBoard->show();
        this->close();
    }
}

void StartMenu::onExitClicked()
{
    QApplication::quit();
}
