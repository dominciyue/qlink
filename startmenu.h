#ifndef STARTMENU_H
#define STARTMENU_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

// 前向声明
class GameBoard;

class StartMenu : public QWidget
{
    Q_OBJECT

public:
    explicit StartMenu(QWidget *parent = nullptr);

private slots:
    void onNewGameClicked();
    void onLoadGameClicked();
    void onExitClicked();

private:
    QPushButton *newGameButton;
    QPushButton *loadGameButton;
    QPushButton *exitButton;
    QComboBox *gameModeComboBox;
    QLabel *backgroundLabel;

    void setupUI();
    void setButtonStyle(QPushButton *button, const QString &iconPath);
};

#endif // STARTMENU_H
