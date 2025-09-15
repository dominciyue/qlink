#include "gameboard.h"
#include <QGridLayout>
#include <QRandomGenerator>
#include <QIcon>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsLineItem>
#include <QStackedLayout>

void GameBoard::loadImages()
{
    blockImages.resize(3);  // 假设有3种不同的方块
    blockImages[0] = QPixmap("://p1.jpg").scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    blockImages[1] = QPixmap("://p2.jpg").scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    blockImages[2] = QPixmap("://p3.jpg").scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 确保图片加载成功
    for (const auto& img : blockImages) {
        if (img.isNull()) {
            qWarning() << "Failed to load block image";
        }
    }
}

void GameBoard::drawMap(QGridLayout *layout)
{
    for (int i = 0; i < buttons.size(); ++i) {
        for (int j = 0; j < buttons[i].size(); ++j) {
            layout->removeWidget(buttons[i][j]);
        }
    }

    buttons.resize(rows);

    for (int i = 0; i < rows; ++i) {
        buttons[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            QPushButton *button = new QPushButton();
            button->setFixedSize(CELL_SIZE, CELL_SIZE);

            int blockType = map[i][j];
            if (blockType == -1) {
                // 空地
                button->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
            } else if (blockType == -2) {
                // 道具
                for (const auto &prop : props) {
                    if (prop.row == i && prop.col == j) {
                        button->setStyleSheet(getPropStyleSheet(prop.type));
                        button->setText(getPropText(prop.type));
                        break;
                    }
                }
            } else if (blockType >= 0 && blockType < blockImages.size()) {
                // 普通方块
                button->setIcon(QIcon(blockImages[blockType]));
                button->setIconSize(QSize(CELL_SIZE-2, CELL_SIZE-2));
            }

            buttons[i][j] = button;
            layout->addWidget(button, i, j);
        }
    }

    this->connectButtons();
}

void GameBoard::keyPressEvent(QKeyEvent *event)
{

    if (event->key() == Qt::Key_P) {
        if (isPaused) {
            resumeGame();
        } else {
            pauseGame();
        }
        event->accept();
        return;  // 添加这行，确保在处理 P 键后立即返回
    }

    if (!isPaused) {
        int dx = 0, dy = 0;
        int player = 0;

        switch (event->key()) {
        // 玩家1的控制键
        case Qt::Key_Left:
            dx = -1;
            player = 1;

            break;
        case Qt::Key_Right:
            dx = 1;
            player = 1;
            break;
        case Qt::Key_Up:
            dy = -1;
            player = 1;
            break;
        case Qt::Key_Down:
            dy = 1;
            player = 1;
            break;

            // 玩家2的控制键
        case Qt::Key_A:
            dx = -1;
            player = 2;
            break;
        case Qt::Key_D:
            dx = 1;
            player = 2;
            break;
        case Qt::Key_W:
            dy = -1;
            player = 2;
            break;
        case Qt::Key_S:
            dy = 1;
            player = 2;
            break;

        default:
            QWidget::keyPressEvent(event);
            return;
        }

        if (player == 0 || (player == 2 && !isTwoPlayerMode)) {
            return;
        }
        movePlayer(player, dx, dy);
        event->accept();
    }
}

void GameBoard::generateMap()
{
    int totalCells = (rows - 2) * (cols - 2);  // 不包括边界

    QVector<int> allItems;
    int propCount = totalCells / 10;  // 10% 的格子是道具
    int blockPairCount = (totalCells - propCount) / 2;

    // 添加道具
    for (int i = 0; i < propCount; ++i) {
        allItems.push_back(-2);  // 使用 -2 表示道具
    }

    // 添加可消除方块
    for (int i = 0; i < blockPairCount; ++i) {
        int blockType = QRandomGenerator::global()->bounded(3);  // 生成0-2的随机数
        allItems.push_back(blockType);
        allItems.push_back(blockType);  // 每种类型都添加两次，确保可以配对
    }

    // 打乱方块和道具顺序
    for (int i = allItems.size() - 1; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        qSwap(allItems[i], allItems[j]);
    }

    // 填充地图
    map.resize(rows);
    for (int i = 0; i < rows; ++i) {
        map[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) {
                map[i][j] = -1;  // 边界上没有方块
            } else {
                map[i][j] = allItems.takeLast();
                if (map[i][j] == -2) {  // 如果是道具
                    PropType propType;
                    if (isTwoPlayerMode) {
                        propType = static_cast<PropType>(QRandomGenerator::global()->bounded(5) + 1);
                        if (propType == PropType::Flash) {
                            propType = PropType::Dizzy;  // 替换 Flash 为 Dizzy
                        }
                    } else {
                        propType = static_cast<PropType>(QRandomGenerator::global()->bounded(4) + 1);
                    }
                    props.push_back({propType, i, j});
                }
            }
        }
    }
}

bool GameBoard::isMapSolvable()
{

    return true;
}

int GameBoard::countBlockType(int type)
{
    int count = 0;
    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            if (map[i][j] == type) {
                count++;
            }
        }
    }
    return count;
}

void GameBoard::movePlayer(int player, int dx, int dy)
{
    qDebug() << "Moving player" << player << "dx:" << dx << "dy:" << dy;

    if ((player == 1 && isPlayer1Frozen) || (player == 2 && isPlayer2Frozen)) {
        qDebug() << "Player" << player << "is frozen and cannot move";
        return;  // 玩家被冻结，无法移动
    }

    if ((player == 1 && isPlayer1Dizzy) || (player == 2 && isPlayer2Dizzy)) {
        qDebug() << "Player" << player << "is dizzy, reversing movement";
        dx = -dx;
        dy = -dy;
    }

    int &playerRow = (player == 1) ? player1Row : player2Row;
    int &playerCol = (player == 1) ? player1Col : player2Col;

    int newRow = playerRow + dy;
    int newCol = playerCol + dx;

    if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
        if (map[newRow][newCol] == -2) {
            // 触发道具效果
            for (auto it = props.begin(); it != props.end(); ++it) {
                if (it->row == newRow && it->col == newCol) {
                    currentPlayer = player;  // 设置当前玩家
                    activateProp(it->type);
                    props.erase(it);
                    break;
                }
            }
            map[newRow][newCol] = -1;
            buttons[newRow][newCol]->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
            buttons[newRow][newCol]->setText("");
        } else if (map[newRow][newCol] >= 0) {
            activateBlock(player, newRow, newCol);
        }

        playerRow = newRow;
        playerCol = newCol;
        updatePlayersPosition();
    }
}

void GameBoard::activateBlock(int player, int row, int col)
{
    QPushButton *block = buttons[row][col];
    if (block && !block->isHidden()) {
        if (isBlockActivated) {
            buttons[lastActivatedBlock.first][lastActivatedBlock.second]->setStyleSheet("");
        }

        block->setStyleSheet("background-color: yellow; border: 2px solid black;");

        if (isBlockActivated) {
            if (map[row][col] == map[lastActivatedBlock.first][lastActivatedBlock.second] &&
                (row != lastActivatedBlock.first || col != lastActivatedBlock.second)) {

                // 检查是否可以用两个或以内的转折连接
                QVector<QPoint> path = findPath(lastActivatedBlock.first, lastActivatedBlock.second, row, col);

                if (!path.isEmpty()) {
                    // 消除方块
                    buttons[row][col]->hide();
                    buttons[lastActivatedBlock.first][lastActivatedBlock.second]->hide();

                    // 绘制连接线
                    drawConnectionLine(lastActivatedBlock.first, lastActivatedBlock.second, row, col);

                    map[row][col] = -1;
                    map[lastActivatedBlock.first][lastActivatedBlock.second] = -1;
                    isBlockActivated = false;
                    addScore(player, 2);

                    // 检查游戏是否结束
                    if (isGameFinished()) {
                        endGame("恭喜！您已清除所有方块！");
                        return;
                    }

                    // 如果游戏没有结束，检查是否还有可以连接的方块对
                    if (!hasMatchingPairs()) {
                        shuffleBlocks();
                    }
                } else {
                    // 如果不能连接，更新lastActivatedBlock
                    lastActivatedBlock = {row, col};
                }
            } else {
                lastActivatedBlock = {row, col};
            }
        } else {
            isBlockActivated = true;
            lastActivatedBlock = {row, col};
        }
    }
}
// 在 GameBoard.cpp 中实现 focusOutEvent 方法
void GameBoard::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    QTimer::singleShot(0, this, SLOT(setFocus()));
}

GameBoard::GameBoard(QWidget *parent, bool isTwoPlayerMode)
    : QWidget(parent), isTwoPlayerMode(isTwoPlayerMode),
    player1Score(0), player2Score(0), isBlockActivated1(false), isBlockActivated2(false),
    player1(nullptr), player2(nullptr), remainingTime(GAME_DURATION), isPaused(false),
    scene(nullptr), view(nullptr),isPlayer1Frozen(false), isPlayer2Frozen(false),
    isPlayer1Dizzy(false), isPlayer2Dizzy(false),hintTimer(nullptr)  // 初始化为 nullptr
{
    qDebug() << "GameBoard constructor called with isTwoPlayerMode:" << isTwoPlayerMode;

    rows = GRID_SIZE;
    cols = GRID_SIZE;
    loadImages();
    generateMap();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建游戏地图
    QWidget *gameMapWidget = new QWidget(this);
    gridLayout = new QGridLayout(gameMapWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    drawMap(gridLayout);

    gameMapWidget->setFixedSize(CELL_SIZE * cols, CELL_SIZE * rows);

    // 创建场景和视图用于绘制连接线
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, CELL_SIZE * cols, CELL_SIZE * rows);
    view = new QGraphicsView(scene, this);
    view->setStyleSheet("background: transparent;");
    view->setFrameStyle(QFrame::NoFrame);
    view->setFixedSize(CELL_SIZE * cols, CELL_SIZE * rows);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 将所有按钮添加到场景中
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QPushButton *button = buttons[i][j];
            QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
            proxy->setWidget(button);
            proxy->setPos(j * CELL_SIZE, i * CELL_SIZE);
            proxy->setZValue(button->property("zValue").toInt());
            //scene->addItem(proxy);
        }
    }
    propSpawnTimer = new QTimer(this);
    connect(propSpawnTimer, &QTimer::timeout, this, &GameBoard::spawnProp);
    propSpawnTimer->start(30000);  // 每30秒生成一个道具

    mainLayout->addLayout(gridLayout);

    // 创建一个堆叠布局来放置游戏地图和连接线视图
    QStackedLayout *stackedLayout = new QStackedLayout();
    stackedLayout->addWidget(view);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    mainLayout->addLayout(stackedLayout);

    freezeTimer = new QTimer(this);
    dizzyTimer = new QTimer(this);
    connect(freezeTimer, &QTimer::timeout, this, [this]() { unfreezePlayer(currentPlayer); });
    connect(dizzyTimer, &QTimer::timeout, this, [this]() { undizzyPlayer(currentPlayer); });

    // 创建玩家标签
    player1 = new QGraphicsEllipseItem(0, 0, PLAYER_SIZE, PLAYER_SIZE);
    player1->setBrush(QBrush(Qt::yellow));
    player1->setPen(QPen(Qt::black));
    scene->addItem(player1);

    if (isTwoPlayerMode) {
        player2 = new QGraphicsEllipseItem(0, 0, PLAYER_SIZE, PLAYER_SIZE);
        player2->setBrush(QBrush(Qt::blue));
        player2->setPen(QPen(Qt::black));
        scene->addItem(player2);
        player2Row = rows - 2;
        player2Col = cols - 2;
    }

    player1Row = 1;
    player1Col = 1;

    updatePlayersPosition();


    // 创建信息显示区域
    infoWidget = new QWidget(this);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);

    timerLabel = new QLabel(this);
    timerLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(timerLabel);

    if (isTwoPlayerMode) {
        playerTurnLabel = new QLabel(this);
        player1ScoreLabel = new QLabel(this);
        player2ScoreLabel = new QLabel(this);
        infoLayout->addWidget(playerTurnLabel);
        infoLayout->addWidget(player1ScoreLabel);
        infoLayout->addWidget(player2ScoreLabel);
        updatePlayerTurnLabel();
    } else {
        player1ScoreLabel = new QLabel(this);
        infoLayout->addWidget(player1ScoreLabel);
    }

    mainLayout->addWidget(infoWidget);

    // 设置暂停菜单
    setupPauseMenu();

    // 添加暂停菜单到主布局
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setFixedSize(CELL_SIZE * cols, CELL_SIZE * rows + infoWidget->sizeHint().height() + buttonLayout->sizeHint().height());
    // 创建游戏计时器
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameBoard::updateTimer);
    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(false);
    installEventFilter(this);
    updateScoreLabels();

    setFocus();
    startGame();
}

bool GameBoard::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "Mouse button press event filtered.";
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        mousePressEvent(mouseEvent);
    }
    return QWidget::eventFilter(obj, event);
}

void GameBoard::updatePlayersPosition()
{
    if (player1) {
        int x1 = player1Col * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
        int y1 = player1Row * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
        player1->setPos(x1, y1);
        player1->setZValue(Z_PLAYER);
    }
    if (isTwoPlayerMode && player2) {
        int x2 = player2Col * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
        int y2 = player2Row * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2;
        player2->setPos(x2, y2);
        player2->setZValue(Z_PLAYER);
    }

    scene->update();
}
void GameBoard::switchPlayer()
{
    if (isTwoPlayerMode) {
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
        updatePlayerTurnLabel();
    }
}

void GameBoard::updatePlayerTurnLabel()
{
    if (isTwoPlayerMode) {
        playerTurnLabel->setText(QString("当前玩家: %1").arg(currentPlayer));
    }
}

void GameBoard::updateScoreLabels()
{
    if (isTwoPlayerMode) {
        if (player1ScoreLabel) player1ScoreLabel->setText(QString("玩家1得分: %1").arg(player1Score));
        if (player2ScoreLabel) player2ScoreLabel->setText(QString("玩家2得分: %1").arg(player2Score));
    } else {
        if (player1ScoreLabel) player1ScoreLabel->setText(QString("得分: %1").arg(player1Score));
    }
}


bool GameBoard::isGameFinished()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {  // 如果还有非空白、非道具的方块
                return false;
            }
        }
    }
    return true;
}
void GameBoard::checkGameEnd()
{
    if (isGameFinished() || !hasMatchingPairs()) {
        QString message;
        if (isTwoPlayerMode) {
            if (player1Score > player2Score) {
                message = "游戏结束！玩家1获胜！";
            } else if (player2Score > player1Score) {
                message = "游戏结束！玩家2获胜！";
            } else {
                message = "游戏结束！平局！";
            }
        } else {
            message = "游戏结束！";
        }
        message += QString("\n玩家1得分: %1").arg(player1Score);
        if (isTwoPlayerMode) {
            message += QString("\n玩家2得分: %2").arg(player2Score);
        }

        endGame(message);
    }
}
void GameBoard::addScore(int player, int points)
{
    if (player == 1) {
        player1Score += points;
    } else {
        player2Score += points;
    }
    updateScoreLabels();
}
void GameBoard::startGame()
{
    remainingTime = GAME_DURATION;
    updateTimer();
    gameTimer->start(1000); // 每秒更新一次
}
void GameBoard::updateTimer()
{
    if (remainingTime > 0) {
        remainingTime--;
        int minutes = remainingTime / 60;
        int seconds = remainingTime % 60;
        timerLabel->setText(QString("剩余时间: %1:%2")
                                .arg(minutes, 2, 10, QChar('0'))
                                .arg(seconds, 2, 10, QChar('0')));
    } else {
        endGame("时间到！");
    }
}
void GameBoard::endGame(const QString &reason)
{
    gameTimer->stop();
    QString message = reason + "\n";
    if (isTwoPlayerMode) {
        if (player1Score > player2Score) {
            message += "玩家1获胜！";
        } else if (player2Score > player1Score) {
            message += "玩家2获胜！";
        } else {
            message += "平局！";
        }
    } else {
        message += QString("你的得分: %1").arg(player1Score);
    }
    QMessageBox::information(this, "游戏结束", message);

    //添加一些代码来禁用游戏板或重置游戏
    setEnabled(false);  // 禁用整个游戏板
}
bool GameBoard::hasMatchingPairs()
{
    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            if (map[i][j] != -1) {
                for (int x = 1; x < rows - 1; ++x) {
                    for (int y = 1; y < cols - 1; ++y) {
                        if ((i != x || j != y) && map[i][j] == map[x][y]) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void GameBoard::setupPauseMenu()
{
    pauseButton = new QPushButton("暂停", this);
    saveButton = new QPushButton("保存", this);
    loadButton = new QPushButton("载入", this);

    connect(pauseButton, &QPushButton::clicked, this, &GameBoard::pauseGame);
    connect(saveButton, &QPushButton::clicked, this, &GameBoard::onSaveButtonClicked);
    connect(loadButton, &QPushButton::clicked, this, &GameBoard::onLoadButtonClicked);

    saveButton->setEnabled(false);
    loadButton->setEnabled(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(pauseButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);

    // 将按钮布局添加到主布局
    // 注意：这里假设你的主布局是 QVBoxLayout，如果不是，可能需要调整
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->addLayout(buttonLayout);
    }
}

void GameBoard::pauseGame()
{
    qDebug() << "pauseGame " << "isPaused: " << isPaused ;

    if (!isPaused) {
        gameTimer->stop();
        isPaused = true;
        pauseButton->setText("继续");
        saveButton->setEnabled(true);
    } else {
        resumeGame();
    }
}

void GameBoard::resumeGame()
{
    gameTimer->start();
    isPaused = false;
    pauseButton->setText("暂停");
    saveButton->setEnabled(false);
}

void GameBoard::onSaveButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存游戏", "", "游戏存档 (*.sav)");
    if (!fileName.isEmpty()) {
        saveGame(fileName);
    }
}

void GameBoard::onLoadButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "载入游戏", "", "游戏存档 (*.sav)");
    if (!fileName.isEmpty()) {
        loadGame(fileName);
    }
}

void GameBoard::saveGame(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_5_15);

        // 保存游戏模式
        out << isTwoPlayerMode;

        // 保存地图大小和内容
        out << rows << cols;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                out << map[i][j];
            }
        }

        // 保存玩家位置
        out << player1Row << player1Col;
        if (isTwoPlayerMode) {
            out << player2Row << player2Col;
        }

        // 保存分数
        out << player1Score;
        if (isTwoPlayerMode) {
            out << player2Score;
        }

        // 保存剩余时间
        out << remainingTime;

        // 保存道具信息
        out << props.size();
        for (const auto &prop : props) {
            out << static_cast<int>(prop.type) << prop.row << prop.col;
        }

        file.close();
        qDebug() << "Game saved successfully.";
    } else {
        qDebug() << "Failed to save game.";
    }
}

void GameBoard::clearCurrentMap()
{
    // 删除所有现有的按钮
    for (auto &row : buttons) {
        for (auto &button : row) {
            if (button) {
                delete button;
            }
        }
    }
    buttons.clear();

    // 清除布局
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(layout());
    if (gridLayout) {
        QLayoutItem *child;
        while ((child = gridLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        delete gridLayout;
    }

    // 创建新的网格布局
    QGridLayout *newLayout = new QGridLayout(this);
    newLayout->setSpacing(0);
    newLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(newLayout);
}

void GameBoard::loadGame(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_15);

        try {
            // 读取游戏模式
            in >> isTwoPlayerMode;
            qDebug() << "Loaded game mode:" << (isTwoPlayerMode ? "Two Player" : "Single Player");

            // 读取地图大小和内容
            int loadedRows, loadedCols;
            in >> loadedRows >> loadedCols;
            qDebug() << "Loaded map size:" << loadedRows << "x" << loadedCols;

            // 检查地图大小是否合法
            if (loadedRows <= 0 || loadedRows > MAX_ROWS || loadedCols <= 0 || loadedCols > MAX_COLS) {
                throw std::runtime_error("Invalid map size in save file.");
            }

            // 调整地图大小
            resizeMap(loadedRows, loadedCols);

            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    in >> map[i][j];
                }
            }

            // 读取玩家位置
            in >> player1Row >> player1Col;
            qDebug() << "Player 1 position:" << player1Row << "," << player1Col;
            if (isTwoPlayerMode) {
                in >> player2Row >> player2Col;
                qDebug() << "Player 2 position:" << player2Row << "," << player2Col;
            }

            // 检查玩家位置是否合法
            if (player1Row < 0 || player1Row >= rows || player1Col < 0 || player1Col >= cols ||
                (isTwoPlayerMode && (player2Row < 0 || player2Row >= rows || player2Col < 0 || player2Col >= cols))) {
                throw std::runtime_error("Invalid player position in save file.");
            }

            // 读取分数
            in >> player1Score;
            qDebug() << "Player 1 score:" << player1Score;
            if (isTwoPlayerMode) {
                in >> player2Score;
                qDebug() << "Player 2 score:" << player2Score;
            }

            // 读取剩余时间
            in >> remainingTime;
            qDebug() << "Remaining time:" << remainingTime;

            // 读取道具信息
            qsizetype propCount;
            in >> propCount;
            qDebug() << "Number of props:" << propCount;
            props.clear();
            for (int i = 0; i < propCount; ++i) {
                int type, row, col;
                in >> type >> row >> col;
                if (row >= 0 && row < rows && col >= 0 && col < cols) {
                    props.push_back({static_cast<PropType>(type), row, col});
                    qDebug() << "Loaded prop:" << type << "at" << row << "," << col;
                } else {
                    qDebug() << "Skipped invalid prop at position:" << row << "," << col;
                }
            }

            file.close();
            qDebug() << "Game loaded successfully.";

            // 重新初始化游戏界面
            initializeGameBoard();

            loadPlayersPosition();
            updateUI();
            for (const auto &prop : props) {
                updateBlockAppearance(prop.row, prop.col);
            }
            updateBlockAppearance(player1Row, player1Col);
            if (isTwoPlayerMode) {
                updateBlockAppearance(player2Row, player2Col);
            }
        } catch (const std::exception& e) {
            qDebug() << "Error loading game:" << e.what();
            file.close();
            return;
        }
    } else {
        qDebug() << "Failed to open game file.";
    }
    updatePlayerPositions();

    if (scene) {
        scene->update();
    }
    // 强制更新所有元素
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            updateBlockAppearance(i, j);
        }
    }

    // 更新玩家位置
    updatePlayerPositions();

    // 更新道具显示
    for (const auto &prop : props) {
        updateBlockAppearance(prop.row, prop.col);
    }

    // 强制重绘
    update();
}

void GameBoard::updatePlayerPositions()
{
    if (player1) {
        player1->setPos(player1Col * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2, player1Row * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2);
        player1->setZValue(1000);
    }
    if (isTwoPlayerMode && player2) {
        player2->setPos(player2Col * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2, player2Row * CELL_SIZE + (CELL_SIZE - PLAYER_SIZE) / 2);
        player2->setZValue(1000);
    }
}

void GameBoard::resizeMap(int newRows, int newCols)
{
    rows = newRows;
    cols = newCols;
    map.resize(rows);
    for (int i = 0; i < rows; ++i) {
        map[i].resize(cols);
    }
}

void GameBoard::initializeGameBoard()
{
    qDebug() << "Initializing game board...";

    // 清除现有的按钮
    for (auto &row : buttons) {
        for (auto &button : row) {
            if (button) {
                delete button;
                button = nullptr;
            }
        }
    }
    buttons.clear();

    // 创建新的按钮
    this->drawMap(gridLayout);

    /*
    buttons.resize(rows);
    for (int i = 0; i < rows; ++i) {
        buttons[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            buttons[i][j] = new QPushButton(this);
            buttons[i][j]->setGeometry(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            buttons[i][j]->show();
        }
    }*/

    // 重新设置游戏板大小
    //setFixedSize(cols * CELL_SIZE, rows * CELL_SIZE);
    setFixedSize(CELL_SIZE * cols, CELL_SIZE * rows + infoWidget->sizeHint().height() + buttonLayout->sizeHint().height());

    // 初始化或重置其他游戏元素
    if (!player1ScoreLabel) {
        player1ScoreLabel = new QLabel(this);
    }
    player1ScoreLabel->setGeometry(10, rows * CELL_SIZE + 10, 200, 30);
    player1ScoreLabel->show();

    if (isTwoPlayerMode) {
        if (!player2ScoreLabel) {
            player2ScoreLabel = new QLabel(this);
        }
        player2ScoreLabel->setGeometry(cols * CELL_SIZE - 210, rows * CELL_SIZE + 10, 200, 30);
        player2ScoreLabel->show();
    }

    // 创建或更新时间标签
    if (!timerLabel) {
        timerLabel = new QLabel(this);
    }
    timerLabel->setGeometry((cols * CELL_SIZE - 100) / 2, rows * CELL_SIZE + 10, 100, 30);
    timerLabel->show();
    qDebug() << "Game board initialized.";
}

void GameBoard::connectButtons()
{
    qDebug() << "connectButtons";

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            connect(buttons[i][j], &QPushButton::pressed, this, [this, i, j]() {
                handleButtonClick(i, j);
                });
        }
    }
}

void GameBoard::handleButtonClick(int row, int col)
{
    qDebug() << "Button clicked at row:" << row << "col:" << col;

    if (isFlashActive) {
        // 处理 Flash 模式下的点击
        if (canReachPosition(player1Row, player1Col, row, col)) {
            movePlayerToNearestEmptyCell(1, row, col);
            if (map[row][col] >= 0) {
                activateBlock(1, row, col);
            }
        }
    } else {
        // 处理普通模式下的点击
        int dx = row - player1Row;
        int dy = col - player1Col;

        if ((abs(dx) == 1 && dy == 0) || (dx == 0 && abs(dy) == 1)) {
            movePlayer(1, dx, dy);
        }
    }

    updateUI();
}

void GameBoard::loadPlayersPosition()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            updateBlockAppearance(i, j);
        }
    }

    updatePlayerAppearance(player1Row, player1Col);
    if (isTwoPlayerMode) {
        updatePlayerAppearance(player2Row, player2Col);
    }
}

void GameBoard::updateScoreDisplay()
{
    qDebug() << "Entering updateScoreDisplay()";

    if (!player1ScoreLabel) {
        qDebug() << "Player 1 score label is null. Creating new label.";
        player1ScoreLabel = new QLabel(this);
        player1ScoreLabel->setGeometry(10, rows * CELL_SIZE + 10, 200, 30);
    }

    qDebug() << "Updating player 1 score:" << player1Score;
    player1ScoreLabel->setText(QString("玩家1分数: %1").arg(player1Score));
    player1ScoreLabel->show();

    if (isTwoPlayerMode) {
        if (!player2ScoreLabel) {
            qDebug() << "Player 2 score label is null. Creating new label.";
            player2ScoreLabel = new QLabel(this);
            player2ScoreLabel->setGeometry(cols * CELL_SIZE - 210, rows * CELL_SIZE + 10, 200, 30);
        }

        qDebug() << "Updating player 2 score:" << player2Score;
        player2ScoreLabel->setText(QString("玩家2分数: %1").arg(player2Score));
        player2ScoreLabel->show();
    } else {
        qDebug() << "Single player mode detected";
        if (player2ScoreLabel) {
            qDebug() << "Hiding player 2 score label";
            player2ScoreLabel->hide();
        } else {
            qDebug() << "Player 2 score label is null in single player mode";
        }
    }

    qDebug() << "Exiting updateScoreDisplay()";
}

void GameBoard::loadTimer()
{
    qDebug() << "Entering updateTimer()";

    if (!timerLabel) {
        qDebug() << "Timer label is null. Creating new label.";
        timerLabel = new QLabel(this);
        timerLabel->setGeometry((cols * CELL_SIZE - 100) / 2, rows * CELL_SIZE + 10, 100, 30);
    }

    qDebug() << "Updating remaining time:" << remainingTime;
    int minutes = remainingTime / 60;
    int seconds = remainingTime % 60;
    timerLabel->setText(QString("剩余时间: %1:%2")
                            .arg(minutes, 2, 10, QChar('0'))
                            .arg(seconds, 2, 10, QChar('0')));
    timerLabel->show();

    qDebug() << "Exiting updateTimer()";
}

void GameBoard::updateUI()
{
    qDebug() << "Entering updateUI()";
      qDebug() << buttons[0][0];
    // 更新方块显示
    qDebug() << "Updating block appearances...";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i < buttons.size() && j < buttons[i].size() && buttons[i][j]) {
                updateBlockAppearance(i, j);
            } else {
                qDebug() << "Warning: Button not found at" << i << "," << j;
            }
        }
    }
    qDebug() << "Block appearances updated.";

    // 更新玩家位置
    qDebug() << "Updating player positions...";
    loadPlayersPosition();
    qDebug() << "Player positions updated.";

    // 更新分数显示
    qDebug() << "Updating score display...";
    updateScoreDisplay();
    qDebug() << "Score display updated.";

    // 更新剩余时间显示
    qDebug() << "Updating timer...";
    loadTimer();
    qDebug() << "Timer updated.";

    // 更新道具显示
    qDebug() << "Updating props display...";
    for (const auto &prop : props) {
        if (prop.row >= 0 && prop.row < rows && prop.col >= 0 && prop.col < cols) {
            updateBlockAppearance(prop.row, prop.col);
            qDebug() << "Updated prop display at" << prop.row << "," << prop.col;
        } else {
            qDebug() << "Warning: Invalid prop position" << prop.row << "," << prop.col;
        }
    }
    qDebug() << "Props display updated.";

    // 刷新整个游戏板
    qDebug() << "Refreshing game board...";
    update();

    qDebug() << "Exiting updateUI()";
}

void GameBoard::updateAllBlockAppearances()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            updateBlockAppearance(i, j);
        }
    }
}

void GameBoard::setupGame()
{
    qDebug() << "Setting up game with isTwoPlayerMode:" << isTwoPlayerMode;
    // 生成地图
    generateMap();

    // 设置暂停菜单
    setupPauseMenu();

    // 开始游戏
    startGame();

    // 设置焦点策略
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

void GameBoard::updateGameBoard()
{
    // 更新玩家位置
    updatePlayersPosition();

    // 更新分数标签
    updateScoreLabels();

    // 更新计时器标签
    updateTimer();

    // 更新玩家回合标签
    updatePlayerTurnLabel();

    // 更新所有方块的外观
    updateAllBlockAppearances();

    // 更新暂停状态
    if (isPaused) {
        pauseGame();
    } else {
        resumeGame();
    }
}
void GameBoard::serializeGame(QDataStream &out)
{
    out << isTwoPlayerMode << rows << cols << player1Score << player2Score
        << player1Row << player1Col << player2Row << player2Col
        << remainingTime << currentPlayer << isPaused;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out << map[i][j];
        }
    }

    // 序列化道具信息
    out << props.size();
    for (const auto &prop : props) {
        out << static_cast<int>(prop.type) << prop.row << prop.col;
    }
}

void GameBoard::deserializeGame(QDataStream &in)
{
    in >> isTwoPlayerMode >> rows >> cols >> player1Score >> player2Score
        >> player1Row >> player1Col >> player2Row >> player2Col
        >> remainingTime >> currentPlayer >> isPaused;

    map.resize(rows);
    for (int i = 0; i < rows; ++i) {
        map[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            in >> map[i][j];
        }
    }

    // 反序列化道具信息
    int propCount;
    in >> propCount;
    props.clear();
    for (int i = 0; i < propCount; ++i) {
        int type, row, col;
        in >> type >> row >> col;
        props.push_back({static_cast<PropType>(type), row, col});
    }
}

void GameBoard::drawConnectionLine(int row1, int col1, int row2, int col2)
{
    clearConnectionLines();

    QVector<QPoint> path = findPath(row1, col1, row2, col2);
    if (path.isEmpty() || !scene) {
        return;
    }

    QPainterPath painterPath;
    QPoint start = path[0];
    painterPath.moveTo(start.x() * CELL_SIZE + CELL_SIZE / 2, start.y() * CELL_SIZE + CELL_SIZE / 2);

    for (int i = 1; i < path.size(); ++i) {
        QPoint end = path[i];
        painterPath.lineTo(end.x() * CELL_SIZE + CELL_SIZE / 2, end.y() * CELL_SIZE + CELL_SIZE / 2);
    }

    QPen pen(Qt::red, 3);
    QGraphicsPathItem *pathItem = scene->addPath(painterPath, pen);
    if (pathItem) {
        pathItem->setZValue(Z_CONNECTION_LINE);
    }

    scene->update();  // 更新整个场景
    view->viewport()->update();  // 更新视图的视口

    QTimer::singleShot(500, this, &GameBoard::clearConnectionLines);
}

void GameBoard::clearConnectionLines()
{
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        if (dynamic_cast<QGraphicsPathItem*>(item)) {
            scene->removeItem(item);
            delete item;
        }
    }
    scene->update();  // 更新整个场景
    view->viewport()->update();  // 更新视图的视口
}

bool GameBoard::allBlocksCleared()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {  // 如果还有非空白、非道具的方块
                return false;
            }
        }
    }
    return true;
}

QVector<QPoint> GameBoard::findPathBFS(int row1, int col1, int row2, int col2)
{
    struct PathNode {
        int row, col;
        QVector<QPoint> path;
        int turns;
        int direction; // 0: 初始, 1: 水平, 2: 垂直
    };

    QQueue<PathNode> queue;
    QVector<QVector<QVector<bool>>> visited(rows, QVector<QVector<bool>>(cols, QVector<bool>(3, false)));

    queue.enqueue({row1, col1, {{col1, row1}}, 0, 0});
    visited[row1][col1][0] = true;

    while (!queue.isEmpty()) {


        PathNode current = queue.dequeue();
        qDebug() << queue.size() << " " << current.row << "," << current.col << " " << row2 << "," << col2;

        if (current.row == row2 && current.col == col2) {
            qDebug() << "path size: " << current.path.size();
            return current.path;
        }

        static const int dr[] = {0, 0, -1, 1};
        static const int dc[] = {-1, 1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];
            int newDirection = (i < 2) ? 1 : 2;
            int newTurns = current.turns + (current.direction != newDirection && current.direction != 0);

            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
                !visited[newRow][newCol][newDirection] &&
                (isEmptyOrBorder(newRow, newCol) || (newRow == row2 && newCol == col2)) &&
                newTurns <= 2) {

                bool pathClear = true;
                if (newDirection == 1) { // 水平移动
                    int minCol = qMin(newCol, current.col);
                    int maxCol = qMax(newCol, current.col);
                    for (int c = minCol + 1; c < maxCol; ++c) {
                        if (!isEmptyOrBorder(current.row, c) && !(current.row == row2 && c == col2)) {
                            pathClear = false;
                            break;
                        }
                    }
                } else { // 垂直移动
                    int minRow = qMin(newRow, current.row);
                    int maxRow = qMax(newRow, current.row);
                    for (int r = minRow + 1; r < maxRow; ++r) {
                        if (!isEmptyOrBorder(r, current.col) && !(r == row2 && current.col == col2)) {
                            pathClear = false;
                            break;
                        }
                    }
                }

                if (pathClear) {
                    QVector<QPoint> newPath = current.path;
                    newPath.append(QPoint(newCol, newRow));
                    queue.enqueue({newRow, newCol, newPath, newTurns, newDirection});
                    visited[newRow][newCol][newDirection] = true;
                }
            }
        }
    }

    return QVector<QPoint>();
}

QVector<QPoint> GameBoard::findPath(int row1, int col1, int row2, int col2)
{
    qDebug() << "Finding path between (" << row1 << "," << col1 << ") and (" << row2 << "," << col2 << ")";

    // 检查是否可以连接
    if (!canConnect(row1, col1, row2, col2)) {
        qDebug() << "Cannot connect these blocks";
        return QVector<QPoint>();
    }

    return findPathBFS(row1, col1, row2, col2);
}

bool GameBoard::isEmptyOrBorder(int row, int col)
{
    return (row >= 0 && row < rows && col >= 0 && col < cols && map[row][col] == -1);
}
bool GameBoard::canConnect(int row1, int col1, int row2, int col2)
{
    // 检查是否是同一个方块
    if (row1 == row2 && col1 == col2) return false;

    // 检查是否是相同类型的方块
    if (map[row1][col1] != map[row2][col2] || map[row1][col1] == -1) return false;

    // 使用新的路径查找算法
    QVector<QPoint> path = findPathBFS(row1, col1, row2, col2);
    return !path.isEmpty();
}

bool GameBoard::checkStraightLine(int row1, int col1, int row2, int col2)
{
    if (row1 == row2) {
        int minCol = qMin(col1, col2);
        int maxCol = qMax(col1, col2);
        for (int col = minCol + 1; col < maxCol; ++col) {
            if (!isEmptyOrBorder(row1, col)) return false;
        }
    } else if (col1 == col2) {
        int minRow = qMin(row1, row2);
        int maxRow = qMax(row1, row2);
        for (int row = minRow + 1; row < maxRow; ++row) {
            if (!isEmptyOrBorder(row, col1)) return false;
        }
    } else {
        return false;
    }
    return true;
}

void GameBoard::spawnProp()
{
    QVector<PropType> availableProps;
    if (isTwoPlayerMode) {
        availableProps = {PropType::PlusOneSecond, PropType::Shuffle, PropType::Hint, PropType::Freeze, PropType::Dizzy};
        qDebug() << "Available props for two-player mode:";
    } else {
        availableProps = {PropType::PlusOneSecond, PropType::Shuffle, PropType::Hint, PropType::Flash};
        qDebug() << "Available props for single-player mode:";
    }

    // 输出可用道具
    for (const auto& prop : availableProps) {
        qDebug() << "  -" << getPropText(prop);
    }

    PropType propType = availableProps[QRandomGenerator::global()->bounded(availableProps.size())];
    qDebug() << "Selected prop type:" << getPropText(propType);

    // 找到一个空的位置来放置道具
    int row, col;
    do {
        row = QRandomGenerator::global()->bounded(rows);
        col = QRandomGenerator::global()->bounded(cols);
    } while (map[row][col] != -1);  // -1 表示空位置

    // 在地图上标记道具位置
    map[row][col] = -2;  // -2 表示道具

    // 添加道具到道具列表
    props.push_back({propType, row, col});

    // 更新方块外观
    updateBlockAppearance(row, col);

    // 如果道具生成在玩家位置上，立即激活道具
    if ((row == player1Row && col == player1Col) || (isTwoPlayerMode && row == player2Row && col == player2Col)) {
        activateProp(propType);
        // 移除已激活的道具
        props.removeIf([row, col](const Prop& prop) { return prop.row == row && prop.col == col; });
        map[row][col] = -1;  // 将位置标记为空
        updateBlockAppearance(row, col);
    }

    // 调试输出
    qDebug() << "Spawned prop:" << getPropText(propType) << "at" << row << "," << col << "isTwoPlayerMode:" << isTwoPlayerMode;
}
QString GameBoard::getPropText(PropType type)
{
    switch (type) {
    case PropType::PlusOneSecond:
        return "+1s";
    case PropType::Shuffle:
        return "Shuffle";
    case PropType::Hint:
        return "Hint";
    case PropType::Flash:
        return "Flash";
    case PropType::Freeze:
        return "Freeze";
    case PropType::Dizzy:
        return "Dizzy";
    default:
        return "";
    }
}

QString GameBoard::getPropStyleSheet(PropType type)
{
    switch (type) {
    case PropType::PlusOneSecond:
        return "background-color: lightblue; border: 2px solid blue;";
    case PropType::Shuffle:
        return "background-color: lightgreen; border: 2px solid green;";
    case PropType::Hint:
        return "background-color: lightyellow; border: 2px solid yellow;";
    case PropType::Flash:
        return "background-color: lightpink; border: 2px solid red;";
    case PropType::Freeze:
        return "background-color: cyan; border: 2px solid darkblue;";
    case PropType::Dizzy:
        return "background-color: magenta; border: 2px solid purple;";
    default:
        return "background-color: white; border: 2px solid black;";
    }
}

void GameBoard::activateProp(PropType type)
{
    qDebug() << "Activating prop:" << getPropText(type) << "isTwoPlayerMode:" << isTwoPlayerMode << "currentPlayer:" << currentPlayer;

    switch (type) {
    case PropType::PlusOneSecond:
        plusOneSecond();
        break;
    case PropType::Shuffle:
        shuffleBlocks();
        break;
    case PropType::Hint:
         startHint();
        break;

    case PropType::Flash:
        if (!isTwoPlayerMode) {
            startFlash();
        } else {
            qDebug() << "Flash prop not available in two-player mode";
        }
        break;
    case PropType::Freeze:
        if (isTwoPlayerMode) {
            freezePlayer(currentPlayer == 1 ? 2 : 1);
        } else {
            qDebug() << "Freeze prop not available in single-player mode";
        }
        break;
    case PropType::Dizzy:
        if (isTwoPlayerMode) {
            dizzyPlayer(currentPlayer == 1 ? 2 : 1);
        } else {
            qDebug() << "Dizzy prop not available in single-player mode";
        }
        break;
    default:
        qDebug() << "Unknown prop type:" << static_cast<int>(type);
        break;
    }
}

void GameBoard::plusOneSecond()
{
    remainingTime += 30;
    updateTimer();
}

void GameBoard::shuffleBlocks()
{
    QVector<int> blockTypes;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {
                blockTypes.push_back(map[i][j]);
            }
        }
    }

    std::random_shuffle(blockTypes.begin(), blockTypes.end());

    int index = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {
                map[i][j] = blockTypes[index++];
                updateBlockAppearance(i, j);
            }
        }
    }
}

void GameBoard::startHint()
{
    if (!hintBlocks.isEmpty()) {
        stopHint(); // 如果已经有提示在显示，先停止它
    }

    // 查找可以连接的方块对
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {
                for (int m = 0; m < rows; ++m) {
                    for (int n = 0; n < cols; ++n) {
                        if ((i != m || j != n) && map[i][j] == map[m][n]) {
                            QVector<QPoint> path = findPath(i, j, m, n);
                            qDebug() << "path found, " << i << "," << j << " to " << m <<","<<n;

                            if (!path.isEmpty()) {
                                hintBlocks.push_back({i, j});
                                hintBlocks.push_back({m, n});
                                highlightHintBlocks();

                                // 设置定时器，5秒后停止提示
                                if (hintTimer) {
                                    hintTimer->stop();
                                    disconnect(hintTimer, &QTimer::timeout, this, &GameBoard::stopHint);
                                    delete hintTimer;
                                }

                                hintTimer = new QTimer(this);
                                connect(hintTimer, &QTimer::timeout, this, &GameBoard::stopHint);
                                hintTimer->setSingleShot(true);
                                hintTimer->start(5000);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // 如果没有找到可连接的方块对，显示提示信息
    //QMessageBox::information(this, "提示", "当前没有可连接的方块对");
    qDebug() << "当前没有可连接的方块对";
}

void GameBoard::stopHint()
{
    qDebug() << "stopHint";

    if (hintTimer) {
        hintTimer->stop();
        delete hintTimer;
        hintTimer = nullptr;
    }
    clearHintHighlight();
}

void GameBoard::highlightHintBlocks()
{


    clearHintHighlight();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] >= 0) {
                for (int x = i; x < rows; ++x) {
                    for (int y = (x == i ? j + 1 : 0); y < cols; ++y) {
                        if (map[x][y] == map[i][j] && canConnect(i, j, x, y)) {
                            hintBlocks = {{i, j}, {x, y}};

                            qDebug() << "highlightHintBlocks " << i << "," << j << " to " << x <<"," <<y;

                            buttons[i][j]->setStyleSheet("background-color: lightgreen; border: 2px solid black;");
                            buttons[x][y]->setStyleSheet("background-color: lightgreen; border: 2px solid black;");

                            return;
                        }
                    }
                }
            }
        }
    }
}

void GameBoard::clearHintHighlight()
{
    for (const auto &block : hintBlocks) {
        buttons[block.first][block.second]->setStyleSheet("");
    }
    hintBlocks.clear();
}

void GameBoard::startFlash()
{
    qDebug() << "Starting Flash mode.";
    isFlashActive = true;
    flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, this, &GameBoard::stopFlash);
    flashTimer->start(5000);  // 5秒
}

void GameBoard::stopFlash()
{
    qDebug() << "Stopping Flash mode.";
    isFlashActive = false;
    if (flashTimer) {
        flashTimer->stop();
        delete flashTimer;
        flashTimer = nullptr;
    }
}

void GameBoard::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Mouse press event detected. isFlashActive:" << isFlashActive;

    if (isFlashActive) {
        int col = event->pos().x() / CELL_SIZE;
        int row = event->pos().y() / CELL_SIZE;
        qDebug() << "Mouse click position - row:" << row << "col:" << col;

        if (row >= 0 && row < rows && col >= 0 && col < cols) {
            qDebug() << "Position is within bounds.";
            if (canReachPosition(player1Row, player1Col, row, col)) {
                qDebug() << "Position is reachable.";
                movePlayerToNearestEmptyCell(1, row, col);
                if (map[row][col] >= 0) {
                    qDebug() << "Activating block at row:" << row << "col:" << col;
                    activateBlock(1, row, col);
                }
            } else {
                qDebug() << "Position is not reachable.";
            }
        } else {
            qDebug() << "Position is out of bounds.";
        }
    } else {
        qDebug() << "Flash is not active.";
    }

    QWidget::mousePressEvent(event);
    setFocus();
    event->accept();
}

bool GameBoard::canReachPosition(int startRow, int startCol, int endRow, int endCol)
{
    qDebug() << "Checking if position is reachable from (" << startRow << "," << startCol << ") to (" << endRow << "," << endCol << ")";

    QQueue<QPair<int, int>> queue;
    QVector<QVector<bool>> visited(rows, QVector<bool>(cols, false));

    queue.enqueue({startRow, startCol});
    visited[startRow][startCol] = true;

    while (!queue.isEmpty()) {
        QPair<int, int> current = queue.dequeue();
        int r = current.first;
        int c = current.second;

        if (r == endRow && c == endCol) {
            qDebug() << "Position is reachable.";
            return true;
        }

        static const int dr[] = {-1, 1, 0, 0};
        static const int dc[] = {0, 0, -1, 1};

        for (int i = 0; i < 4; ++i) {
            int newRow = r + dr[i];
            int newCol = c + dc[i];

            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
                !visited[newRow][newCol] && isEmptyOrBorder(newRow, newCol)) {
                queue.enqueue({newRow, newCol});
                visited[newRow][newCol] = true;
            }
        }
    }

    qDebug() << "Position is not reachable.";
    return false;
}

void GameBoard::movePlayerToNearestEmptyCell(int player, int targetRow, int targetCol)
{
    qDebug() << "Moving player" << player << "to nearest empty cell around (" << targetRow << "," << targetCol << ")";

    static const int dr[] = {0, -1, 1, 0, 0};
    static const int dc[] = {0, 0, 0, -1, 1};

    for (int i = 0; i < 5; ++i) {
        int newRow = targetRow + dr[i];
        int newCol = targetCol + dc[i];

        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols &&
            isEmptyOrBorder(newRow, newCol)) {
            if (player == 1) {
                player1Row = newRow;
                player1Col = newCol;
            } else {
                player2Row = newRow;
                player2Col = newCol;
            }
            updatePlayersPosition();
            qDebug() << "Player" << player << "moved to (" << newRow << "," << newCol << ")";
            return;
        }
    }

    qDebug() << "No empty cell found around (" << targetRow << "," << targetCol << ")";
}

void GameBoard::updateBlockAppearance(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || !buttons[row][col]) {
        return;
    }

    QPushButton *button = buttons[row][col];
    int blockType = map[row][col];

    if (blockType == -1) {
        // 空白方块
        button->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
        button->setIcon(QIcon());
        button->setText("");
    } else if (blockType == -2) {
        // 道具
        for (const auto &prop : props) {
            if (prop.row == row && prop.col == col) {
                button->setStyleSheet(getPropStyleSheet(prop.type));
                button->setText(getPropText(prop.type));
                button->setIcon(QIcon());
                break;
            }
        }
    } else if (blockType >= 0 && blockType < blockImages.size()) {
        // 普通方块
        button->setStyleSheet("");
        button->setIcon(QIcon(blockImages[blockType]));
        button->setIconSize(QSize(CELL_SIZE-2, CELL_SIZE-2));
        button->setText("");
    }
}


void GameBoard::updatePlayerAppearance(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || !buttons[row][col]) {
        qDebug() << "Invalid position or button in updatePlayerAppearance:" << row << "," << col;
        return;
    }

    QPushButton *button = buttons[row][col];
    if (row == player1Row && col == player1Col) {
        button->setStyleSheet("background-color: rgba(255, 0, 0, 128); border: 2px solid darkred;");
    }
    else if (isTwoPlayerMode && row == player2Row && col == player2Col) {
        button->setStyleSheet("background-color: rgba(0, 0, 255, 128); border: 2px solid darkblue;");
        player2 = new QGraphicsEllipseItem(0, 0, PLAYER_SIZE, PLAYER_SIZE);
        player2->setBrush(QBrush(Qt::blue));
        player2->setPen(QPen(Qt::black));
        scene->addItem(player2);
    }
}

void GameBoard::freezePlayer(int player)
{
    qDebug() << "Freezing player" << player;
    if (player == 1) {
        isPlayer1Frozen = true;
    } else {
        isPlayer2Frozen = true;
    }
    QTimer::singleShot(3000, this, [this, player]() {
        unfreezePlayer(player);
    });
}

void GameBoard::dizzyPlayer(int player)
{
    qDebug() << "Dizzying player" << player;
    if (player == 1) {
        isPlayer1Dizzy = true;
    } else {
        isPlayer2Dizzy = true;
    }
    QTimer::singleShot(10000, this, [this, player]() {
        undizzyPlayer(player);
    });
}

void GameBoard::unfreezePlayer(int player)
{
    if (player == 1) {
        isPlayer1Frozen = false;
    } else {
        isPlayer2Frozen = false;
    }
}


void GameBoard::undizzyPlayer(int player)
{
    qDebug() << "Undizzying player" << player;
    if (player == 1) {
        isPlayer1Dizzy = false;
    } else {
        isPlayer2Dizzy = false;
    }
}

void GameBoard::reversePlayerMovement(int player, int &dx, int &dy)
{
    dx = -dx;
    dy = -dy;
}

