#ifndef GAMEBOARD_H
#define GAMEBOARD_H
#include <QWidget>
#include <QVector>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>
#include <QPixmap>
#include <QGridLayout>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QQueue>
#include <QGraphicsProxyWidget>
#include <QGraphicsEllipseItem>

class GameBoard : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoard(QWidget *parent = nullptr, bool isTwoPlayerMode = false);
    enum class PropType {
        None,
        PlusOneSecond,
        Shuffle,
        Hint,
        Flash,  // 仅在单人模式中使用
        Freeze, // 仅在双人模式中使用
        Dizzy   // 仅在双人模式中使用
    };
    void setupGame();
    void runTests(); // 新增的测试方法

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void onSaveButtonClicked();
    void onLoadButtonClicked();
    void handleButtonClick(int row, int col);
    void updateTimer();
    void pauseGame();
    void resumeGame();
public slots:
    void saveGame(const QString &fileName);
    void loadGame(const QString &fileName);

private:
    static const int MAX_COLS = 15;
    static const int MAX_ROWS = 15;
    bool checkStraightLine(int row1, int col1, int row2, int col2);
    void generateMap();
    void drawMap(QGridLayout *layout);
    void movePlayer(int player, int dx, int dy);
    void activateBlock(int player, int row, int col);
    void checkAndRemoveBlocks();
    void loadImages();
    bool isGameFinished();
    //void updatePlayersPosition();
    void updatePlayersPosition();
    bool isMapSolvable();
    int countBlockType(int type);
    bool isTwoPlayerMode;
    int currentPlayer;
    QLabel *playerTurnLabel;
    void switchPlayer();
    void updatePlayerTurnLabel();
    int rows;
    int cols;
    QVector<QVector<int>> map;
    QVector<QVector<QPushButton*>> buttons;
    QVector<QPixmap> blockImages;
    QPair<int, int> lastActivatedBlock;
    bool isBlockActivated;
    static const int CELL_SIZE = 50;  // 每个格子的大小
    static const int PLAYER_SIZE = 30;  // 玩家图标的大小
    static const int GRID_SIZE = 14;  // 网格的大小（行数和列数）
    QLabel *player1ScoreLabel;
    QLabel *player2ScoreLabel;
    int player1Score;
    int player2Score;
    void updateScoreLabels();
    void addScore(int player, int points);
    void checkGameEnd();
    QGraphicsEllipseItem *player1;
    QGraphicsEllipseItem *player2;
    int player1Row;
    int player1Col;
    int player2Row;
    int player2Col;
    QPair<int, int> lastActivatedBlock1;
    QPair<int, int> lastActivatedBlock2;
    bool isBlockActivated1;
    bool isBlockActivated2;
    QTimer *gameTimer;
    QLabel *timerLabel;
    int remainingTime;
    static const int GAME_DURATION = 300; // 游戏时长（秒）
    void startGame();
    void endGame(const QString &reason);
    bool hasMatchingPairs();
    QPushButton *pauseButton;
    QPushButton *saveButton;
    QPushButton *loadButton;
    bool isPaused;
    void setupPauseMenu();
    void serializeGame(QDataStream &out);
    void deserializeGame(QDataStream &in);
    QGraphicsScene *scene;
    QGraphicsView *view;
    void drawConnectionLine(int row1, int col1, int row2, int col2);
    void clearConnectionLines();
    QVector<QPoint> findPath(int row1, int col1, int row2, int col2);
    bool canConnect(int row1, int col1, int row2, int col2);
    bool isEmptyOrBorder(int row, int col);
    bool allBlocksCleared();
    QVector<QPoint> findPathBFS(int row1, int col1, int row2, int col2);
    struct PathNode {
        int row, col;
        QVector<QPoint> path;
        int turns;
    };
    static const int Z_BACKGROUND = 0;
    static const int Z_NORMAL_BLOCK = 1;
    static const int Z_BORDER_BLOCK = 2;
    static const int Z_PLAYER = 98;
    static const int Z_CONNECTION_LINE = 99;
    struct Prop {
        PropType type;
        int row;
        int col;
    };
    QString getPropText(PropType type);
    QVector<Prop> props;
    QTimer *propSpawnTimer;
    QTimer *hintTimer;
    QTimer *flashTimer;
    bool isFlashActive;
    QVector<QPair<int, int>> hintBlocks;
    // 在类定义中添加以下公共方法
    void spawnProp();
    void activateProp(PropType type);
    void plusOneSecond();
    void shuffleBlocks();
    void startHint();
    void stopHint();
    void startFlash();
    void stopFlash();
    void clearCurrentMap();
    void highlightHintBlocks();
    void clearHintHighlight();
    bool canReachPosition(int startRow, int startCol, int endRow, int endCol);
    void movePlayerToNearestEmptyCell(int player, int targetRow, int targetCol);
    void updateBlockAppearance(int row, int col);
    QString getPropStyleSheet(PropType type);
    QTimer *freezeTimer;
    QTimer *dizzyTimer;
    bool isPlayer1Frozen;
    bool isPlayer2Frozen;
    bool isPlayer1Dizzy;
    bool isPlayer2Dizzy;

    // 添加新的方法
    void freezePlayer(int player);
    void unfreezePlayer(int player);
    void dizzyPlayer(int player);
    void undizzyPlayer(int player);
    void reversePlayerMovement(int player, int &dx, int &dy);
    void updateGameBoard();
    void updateAllBlockAppearances();
    void setupUI();
    void updateUI();
    void loadPlayersPosition();
    void loadTimer();
    void resizeMap(int newRows, int newCols);
    void initializeGameBoard();
    void connectButtons();
    void updatePlayerAppearance(int row, int col);
    void updatePlayerPositions();
    void updateScoreDisplay();

    QWidget *infoWidget;
    QHBoxLayout *buttonLayout;
    QGridLayout *gridLayout;
    friend class TestGameBoard;
    void testActivateBlock();
    void testFindPath();
    void testIsGameFinished();
    void testHasMatchingPairs();
};

#endif // GAMEBOARD_H
