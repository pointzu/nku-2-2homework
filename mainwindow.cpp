#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QToolTip>
#include <QColor>
#include <QFont>
#include <QStatusBar>
#include <QSplitter>
#include <QFrame>
#include <QTimer>
#include <QSettings>

// CellWidget Implementation
CellWidget::CellWidget(int row, int col, QWidget* parent)
    : QWidget(parent)
    , m_row(row)
    , m_col(col)
    , m_algaeType(AlgaeType::NONE)
    , m_status(AlgaeCell::NORMAL)
{
    setMinimumSize(50, 50);
    setMaximumSize(80, 80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    void MainWindow::onGameWon() {
        // Stop the game
        m_game->pauseGame();

        // Play win sound
        if (m_sfxWin->isLoaded()) {
            m_sfxWin->play();
        }

        // Show victory message
        QMessageBox::information(this, tr("胜利!"),
                                 tr("恭喜! 你已经成功建立了一个高效且可持续发展的藻类生态系统!\n\n"
                                    "你的最终成绩:\n"
                                    "- 糖类: %.1f (产量: %.1f/秒)\n"
                                    "- 脂质: %.1f (产量: %.1f/秒)\n"
                                    "- 蛋白质: %.1f (产量: %.1f/秒)\n"
                                    "- 维生素: %.1f (产量: %.1f/秒)")
                                     .arg(m_game->getResources()->getCarbohydrates())
                                     .arg(m_game->getResources()->getCarbRate())
                                     .arg(m_game->getResources()->getLipids())
                                     .arg(m_game->getResources()->getLipidRate())
                                     .arg(m_game->getResources()->getProteins())
                                     .arg(m_game->getResources()->getProRate())
                                     .arg(m_game->getResources()->getVitamins())
                                     .arg(m_game->getResources()->getVitRate()));

        // Ask if player wants to restart
        if (QMessageBox::question(this, tr("重新开始?"),
                                  tr("你想再玩一局吗?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            restartGame();
        }
    }

    void MainWindow::keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Escape) {
            showGameMenu();
        }

        QMainWindow::keyPressEvent(event);
    }

    void MainWindow::showGameMenu() {
        // Pause the game
        bool wasRunning = m_game->isGameRunning();
        if (wasRunning) {
            m_game->pauseGame();
        }

        // Show game menu
        QMenu popupMenu(this);
        QAction* continueAction = nullptr;

        if (wasRunning) {
            continueAction = popupMenu.addAction(tr("继续游戏"));
        }

        QAction* restartAction = popupMenu.addAction(tr("重新开始"));
        QAction* settingsAction = popupMenu.addAction(tr("设置"));
        popupMenu.addSeparator();
        QAction* exitAction = popupMenu.addAction(tr("退出"));

        QAction* selectedAction = popupMenu.exec(QCursor::pos());

        if (selectedAction == continueAction) {
            m_game->startGame(); // Resume game
        } else if (selectedAction == restartAction) {
            restartGame();
        } else if (selectedAction == settingsAction) {
            showSettingsDialog();
            if (wasRunning) {
                m_game->startGame(); // Resume game after settings
            }
        } else if (selectedAction == exitAction) {
            exitGame();
        } else if (wasRunning) {
            // If menu was canceled but game was running, resume it
            m_game->startGame();
        }
    }

    void MainWindow::restartGame() {
        // Ask for confirmation if game is in progress
        if (m_game->isGameRunning()) {
            if (QMessageBox::question(this, tr("重新开始?"),
                                      tr("确定要重新开始游戏吗? 当前进度将丢失。"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                return;
            }
        }

        // Reset game
        m_game->resetGame();
        m_game->startGame();

        // Update UI
        updateGridDisplay();
        onResourcesChanged();
        onProductionRatesChanged();

        statusBar()->showMessage(tr("游戏已重新开始"), 2000);
    }

    void MainWindow::exitGame() {
        // Ask for confirmation if game is in progress
        if (m_game->isGameRunning()) {
            if (QMessageBox::question(this, tr("退出游戏?"),
                                      tr("确定要退出游戏吗? 未保存的进度将丢失。"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
                return;
            }
        }

        // Save settings
        QSettings settings("AlgaeGame", "Settings");
        settings.setValue("MusicVolume", m_game->getMusic());
        settings.setValue("SFXVolume", m_game->getSoundEffects());

        // Close application
        QApplication::quit();
    }

    void MainWindow::showSettingsDialog() {
        // Pause game
        bool wasRunning = m_game->isGameRunning();
        if (wasRunning) {
            m_game->pauseGame();
        }

        // Create dialog
        QDialog dialog(this);
        dialog.setWindowTitle(tr("游戏设置"));
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Music volume
        QHBoxLayout* musicLayout = new QHBoxLayout();
        QLabel* musicLabel = new QLabel(tr("背景音乐音量:"));
        QSlider* musicSlider = new QSlider(Qt::Horizontal);
        QLabel* musicValueLabel = new QLabel();

        musicSlider->setRange(0, 100);
        musicSlider->setValue(m_game->getMusic());
        musicValueLabel->setText(QString::number(musicSlider->value()));

        connect(musicSlider, &QSlider::valueChanged, [musicValueLabel](int value) {
            musicValueLabel->setText(QString::number(value));
        });

        musicLayout->addWidget(musicLabel);
        musicLayout->addWidget(musicSlider);
        musicLayout->addWidget(musicValueLabel);

        // Sound effects volume
        QHBoxLayout* sfxLayout = new QHBoxLayout();
        QLabel* sfxLabel = new QLabel(tr("音效音量:"));
        QSlider* sfxSlider = new QSlider(Qt::Horizontal);
        QLabel* sfxValueLabel = new QLabel();

        sfxSlider->setRange(0, 100);
        sfxSlider->setValue(m_game->getSoundEffects());
        sfxValueLabel->setText(QString::number(sfxSlider->value()));

        connect(sfxSlider, &QSlider::valueChanged, [sfxValueLabel](int value) {
            sfxValueLabel->setText(QString::number(value));
        });

        sfxLayout->addWidget(sfxLabel);
        sfxLayout->addWidget(sfxSlider);
        sfxLayout->addWidget(sfxValueLabel);

        // Dialog buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        // Add widgets to layout
        layout->addLayout(musicLayout);
        layout->addLayout(sfxLayout);
        layout->addWidget(buttonBox);

        // Show dialog
        if (dialog.exec() == QDialog::Accepted) {
            // Apply settings
            m_game->setMusicVolume(musicSlider->value());
            m_game->setSoundEffectsVolume(sfxSlider->value());

            // Save settings
            QSettings settings("AlgaeGame", "Settings");
            settings.setValue("MusicVolume", musicSlider->value());
            settings.setValue("SFXVolume", sfxSlider->value());
        }

        // Resume game if it was running before
        if (wasRunning) {
            m_game->startGame();
        }
    }
}

void CellWidget::setAlgaeType(AlgaeType::Type type) {
    if (m_algaeType != type) {
        m_algaeType = type;
        update();
    }
}

void CellWidget::setStatus(AlgaeCell::Status status) {
    if (m_status != status) {
        m_status = status;
        update();
    }
}

void CellWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate cell dimensions
    int cellSize = qMin(width(), height()) - 4;
    QRect cellRect(width()/2 - cellSize/2, height()/2 - cellSize/2, cellSize, cellSize);

    // Draw cell background
    QColor bgColor = QColor(200, 240, 255, 100); // Light blue for water
    painter.fillRect(rect(), bgColor);

    // Draw border
    painter.setPen(QPen(Qt::darkBlue, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    // Draw grid position
    painter.setPen(Qt::darkGray);
    QFont smallFont = painter.font();
    smallFont.setPointSize(6);
    painter.setFont(smallFont);
    painter.drawText(rect().adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft,
                     QString::number(m_row) + "," + QString::number(m_col));

    // Draw algae content if not empty
    if (m_algaeType != AlgaeType::NONE) {
        QColor algaeColor;
        QString algaeText;

        switch (m_algaeType) {
        case AlgaeType::TYPE_A:
            algaeColor = QColor(30, 180, 30); // Green
            algaeText = "A";
            break;
        case AlgaeType::TYPE_B:
            algaeColor = QColor(30, 120, 180); // Blue-green
            algaeText = "B";
            break;
        case AlgaeType::TYPE_C:
            algaeColor = QColor(180, 120, 30); // Orange-brown
            algaeText = "C";
            break;
        default:
            break;
        }

        // Apply status effects to the color
        switch (m_status) {
        case AlgaeCell::RESOURCE_LOW:
            algaeColor = algaeColor.darker(150);
            break;
        case AlgaeCell::LIGHT_LOW:
            algaeColor = algaeColor.darker(200);
            break;
        case AlgaeCell::DYING:
            algaeColor = QColor(150, 150, 150); // Gray for dying
            break;
        default:
            break;
        }

        // Draw algae circle
        painter.setBrush(algaeColor);
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(cellRect);

        // Draw algae text
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(14);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(cellRect, Qt::AlignCenter, algaeText);
    }
}

void CellWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit leftClicked(m_row, m_col);
    } else if (event->button() == Qt::RightButton) {
        emit rightClicked(m_row, m_col);
    }
}

void CellWidget::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event);
    emit hovered(m_row, m_col);
}

// MainWindow Implementation
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_game(new AlgaeGame(this))
{
    setupUI();
    setupGameGrid();
    setupGameControls();
    setupResourceDisplay();
    setupMenus();
    setupAudio();
    connectSignals();

    // Initialize game state
    onGameStateChanged();
    onResourcesChanged();
    onProductionRatesChanged();

    setWindowTitle(tr("Algae - 水藻培养策略游戏"));
    setMinimumSize(800, 600);
    resize(1024, 768);

    statusBar()->showMessage(tr("选择藻类并点击网格放置，右键可移除藻类"));
}

MainWindow::~MainWindow() {
    // All Qt parent-child relationships will handle deletion
}

void MainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_gridLayout = new QGridLayout(m_centralWidget);

    // Create main layout with 3 parts
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, m_centralWidget);

    // 1. Left panel for resources and info
    QWidget* leftPanel = new QWidget(mainSplitter);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    // Progress indicator
    QGroupBox* winGroup = new QGroupBox(tr("通关进度"), leftPanel);
    QVBoxLayout* winLayout = new QVBoxLayout(winGroup);
    m_progressBar = new QProgressBar(winGroup);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    winLayout->addWidget(m_progressBar);
    leftLayout->addWidget(winGroup);

    // Resources display
    QGroupBox* resourceGroup = new QGroupBox(tr("资源"), leftPanel);
    QGridLayout* resourceLayout = new QGridLayout(resourceGroup);
    resourceLayout->setColumnStretch(0, 1);
    resourceLayout->setColumnStretch(1, 1);

    // Resource labels
    m_lblCarb = new QLabel(resourceGroup);
    m_lblLipid = new QLabel(resourceGroup);
    m_lblPro = new QLabel(resourceGroup);
    m_lblVit = new QLabel(resourceGroup);

    resourceLayout->addWidget(new QLabel(tr("糖类:")), 0, 0);
    resourceLayout->addWidget(m_lblCarb, 0, 1);
    resourceLayout->addWidget(new QLabel(tr("脂质:")), 1, 0);
    resourceLayout->addWidget(m_lblLipid, 1, 1);
    resourceLayout->addWidget(new QLabel(tr("蛋白质:")), 2, 0);
    resourceLayout->addWidget(m_lblPro, 2, 1);
    resourceLayout->addWidget(new QLabel(tr("维生素:")), 3, 0);
    resourceLayout->addWidget(m_lblVit, 3, 1);

    leftLayout->addWidget(resourceGroup);

    // Production rates
    QGroupBox* rateGroup = new QGroupBox(tr("生产速率"), leftPanel);
    QGridLayout* rateLayout = new QGridLayout(rateGroup);
    rateLayout->setColumnStretch(0, 1);
    rateLayout->setColumnStretch(1, 1);

    // Rate labels
    m_lblCarbRate = new QLabel(rateGroup);
    m_lblLipidRate = new QLabel(rateGroup);
    m_lblProRate = new QLabel(rateGroup);
    m_lblVitRate = new QLabel(rateGroup);

    rateLayout->addWidget(new QLabel(tr("糖类/秒:")), 0, 0);
    rateLayout->addWidget(m_lblCarbRate, 0, 1);
    rateLayout->addWidget(new QLabel(tr("脂质/秒:")), 1, 0);
    rateLayout->addWidget(m_lblLipidRate, 1, 1);
    rateLayout->addWidget(new QLabel(tr("蛋白质/秒:")), 2, 0);
    rateLayout->addWidget(m_lblProRate, 2, 1);
    rateLayout->addWidget(new QLabel(tr("维生素/秒:")), 3, 0);
    rateLayout->addWidget(m_lblVitRate, 3, 1);

    leftLayout->addWidget(rateGroup);

    // Instructions
    QGroupBox* infoGroup = new QGroupBox(tr("游戏说明"), leftPanel);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    QLabel* infoLabel = new QLabel(tr(
                                       "- 左键放置选中藻类\n"
                                       "- 右键删除已有藻类\n"
                                       "- 鼠标悬浮可查看格子资源\n"
                                       "- ESC 打开菜单\n\n"
                                       "胜利条件:\n"
                                       "- 糖类储备 ≥ 500\n"
                                       "- 脂质储备 ≥ 300\n"
                                       "- 蛋白质储备 ≥ 200\n"
                                       "- 维生素储备 ≥ 100\n"
                                       "- 达到目标生产效率"
                                       ), infoGroup);
    infoLayout->addWidget(infoLabel);
    leftLayout->addWidget(infoGroup);

    leftLayout->addStretch(1);

    // 2. Center panel for grid
    QWidget* centerPanel = new QWidget(mainSplitter);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);

    QFrame* gridFrame = new QFrame(centerPanel);
    gridFrame->setFrameShape(QFrame::StyledPanel);
    gridFrame->setFrameShadow(QFrame::Sunken);

    m_cellsLayout = new QGridLayout(gridFrame);
    m_cellsLayout->setSpacing(2);
    m_cellsLayout->setContentsMargins(5, 5, 5, 5);

    centerLayout->addWidget(gridFrame);

    // 3. Right panel for controls
    QWidget* rightPanel = new QWidget(mainSplitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    QGroupBox* controlGroup = new QGroupBox(tr("藻类选择"), rightPanel);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);

    m_btnTypeA = new QPushButton(tr("小型藻类 A"), controlGroup);
    m_btnTypeB = new QPushButton(tr("小型藻类 B"), controlGroup);
    m_btnTypeC = new QPushButton(tr("小型藻类 C"), controlGroup);

    // Configure button appearance
    QFont btnFont = m_btnTypeA->font();
    btnFont.setBold(true);
    m_btnTypeA->setFont(btnFont);
    m_btnTypeB->setFont(btnFont);
    m_btnTypeC->setFont(btnFont);

    m_btnTypeA->setMinimumHeight(50);
    m_btnTypeB->setMinimumHeight(50);
    m_btnTypeC->setMinimumHeight(50);

    // Add green color to button A
    QPalette palA = m_btnTypeA->palette();
    palA.setColor(QPalette::Button, QColor(200, 255, 200));
    m_btnTypeA->setPalette(palA);
    m_btnTypeA->setAutoFillBackground(true);
    m_btnTypeA->setFlat(true);

    // Add blue-green color to button B
    QPalette palB = m_btnTypeB->palette();
    palB.setColor(QPalette::Button, QColor(200, 220, 255));
    m_btnTypeB->setPalette(palB);
    m_btnTypeB->setAutoFillBackground(true);
    m_btnTypeB->setFlat(true);

    // Add orange color to button C
    QPalette palC = m_btnTypeC->palette();
    palC.setColor(QPalette::Button, QColor(255, 220, 180));
    m_btnTypeC->setPalette(palC);
    m_btnTypeC->setAutoFillBackground(true);
    m_btnTypeC->setFlat(true);

    controlLayout->addWidget(m_btnTypeA);
    controlLayout->addWidget(m_btnTypeB);
    controlLayout->addWidget(m_btnTypeC);

    // Add algae information
    QGroupBox* infoA = new QGroupBox(tr("藻类 A 属性"), controlGroup);
    QVBoxLayout* infoALayout = new QVBoxLayout(infoA);
    QLabel* lblInfoA = new QLabel(tr(
                                      "光照需求: 10/8/5\n"
                                      "种植消耗: 糖×10, 蛋白×5\n"
                                      "遮光效果: 下方1格, -5\n"
                                      "消耗: N×1/秒, C×8/秒\n"
                                      "产出: 糖×5/秒, 蛋白×2/秒\n"
                                      "特性: 同类相邻减产"
                                      ), infoA);
    infoALayout->addWidget(lblInfoA);
    controlLayout->addWidget(infoA);

    QGroupBox* infoB = new QGroupBox(tr("藻类 B 属性"), controlGroup);
    QVBoxLayout* infoBLayout = new QVBoxLayout(infoB);
    QLabel* lblInfoB = new QLabel(tr(
                                      "光照需求: 12/10/6\n"
                                      "种植消耗: 糖×8, 脂质×6, 维生素×2\n"
                                      "遮光效果: 下方2格, 各-3\n"
                                      "消耗: N×2/秒, C×6/秒\n"
                                      "产出: 糖×3/秒, 脂质×4/秒, 维生素×1/秒\n"
                                      "特性: 提升左右恢复速率"
                                      ), infoB);
    infoBLayout->addWidget(lblInfoB);
    controlLayout->addWidget(infoB);

    QGroupBox* infoC = new QGroupBox(tr("藻类 C 属性"), controlGroup);
    QVBoxLayout* infoCLayout = new QVBoxLayout(infoC);
    QLabel* lblInfoC = new QLabel(tr(
                                      "光照需求: 8/6/4\n"
                                      "种植消耗: 糖×5, 蛋白×2, 维生素×8\n"
                                      "遮光效果: 无\n"
                                      "消耗: N×2/秒, C×12/秒\n"
                                      "产出: 糖×3/秒, 蛋白×3/秒, 维生素×5/秒\n"
                                      "特性: 与B连接时糖减产"
                                      ), infoC);
    infoCLayout->addWidget(lblInfoC);
    controlLayout->addWidget(infoC);

    rightLayout->addWidget(controlGroup);
    rightLayout->addStretch(1);

    // Set splitter sizes
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 2);  // Left panel
    mainSplitter->setStretchFactor(1, 5);  // Center panel (grid)
    mainSplitter->setStretchFactor(2, 2);  // Right panel

    m_gridLayout->addWidget(mainSplitter, 0, 0);
}

void MainWindow::initializeCellWidgets() {
    // Clear existing widgets if any
    for (auto& row : m_cellWidgets) {
        for (auto* widget : row) {
            delete widget;
        }
    }
    m_cellWidgets.clear();

    // Initialize cell widgets based on game grid size
    m_cellWidgets.resize(GameGrid::ROWS);
    for (int row = 0; row < GameGrid::ROWS; ++row) {
        m_cellWidgets[row].resize(GameGrid::COLS);
        for (int col = 0; col < GameGrid::COLS; ++col) {
            CellWidget* cellWidget = new CellWidget(row, col, this);
            m_cellsLayout->addWidget(cellWidget, row, col);
            m_cellWidgets[row][col] = cellWidget;

            // Connect cell widget signals
            connect(cellWidget, &CellWidget::leftClicked, this, &MainWindow::onCellClicked);
            connect(cellWidget, &CellWidget::rightClicked, this, &MainWindow::onCellRightClicked);
            connect(cellWidget, &CellWidget::hovered, this, &MainWindow::displayCellInfo);
        }
    }
}

void MainWindow::setupGameGrid() {
    initializeCellWidgets();
    updateGridDisplay();
}

void MainWindow::setupGameControls() {
    // Connect algae type selection buttons
    connect(m_btnTypeA, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_A);
        updateSelectedAlgaeButton();
    });

    connect(m_btnTypeB, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_B);
        updateSelectedAlgaeButton();
    });

    connect(m_btnTypeC, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_C);
        updateSelectedAlgaeButton();
    });

    // Set initial selection
    m_game->setSelectedAlgaeType(AlgaeType::TYPE_A);
    updateSelectedAlgaeButton();
}

void MainWindow::setupResourceDisplay() {
    onResourcesChanged();
    onProductionRatesChanged();

    // Setup timer for win progress updates
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::updateWinProgress);
    progressTimer->start(500); // Update every half second
}

void MainWindow::setupMenus() {
    // Create main menu
    m_gameMenu = menuBar()->addMenu(tr("游戏"));

    // Create menu actions
    m_restartAction = new QAction(tr("重新开始"), this);
    m_settingsAction = new QAction(tr("设置"), this);
    m_exitAction = new QAction(tr("退出"), this);

    // Add actions to menu
    m_gameMenu->addAction(m_restartAction);
    m_gameMenu->addAction(m_settingsAction);
    m_gameMenu->addSeparator();
    m_gameMenu->addAction(m_exitAction);

    // Connect menu actions
    connect(m_restartAction, &QAction::triggered, this, &MainWindow::restartGame);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitGame);
}

void MainWindow::setupAudio() {
    // Initialize background music
    m_backgroundMusic = new QMediaPlayer(this);

    // Initialize sound effects
    m_sfxPlace = new QSoundEffect(this);
    m_sfxRemove = new QSoundEffect(this);
    m_sfxWin = new QSoundEffect(this);

    // Load audio files - these would need to be properly set in a real implementation
    // m_backgroundMusic->setSource(QUrl::fromLocalFile(":/sounds/background.wav"));
    // m_sfxPlace->setSource(QUrl::fromLocalFile(":/sounds/place.wav"));
    // m_sfxRemove->setSource(QUrl::fromLocalFile(":/sounds/remove.wav"));
    // m_sfxWin->setSource(QUrl::fromLocalFile(":/sounds/win.wav"));

    // Load volume settings
    QSettings settings("AlgaeGame", "Settings");
    int musicVolume = settings.value("MusicVolume", 50).toInt();
    int sfxVolume = settings.value("SFXVolume", 70).toInt();

    m_game->setMusicVolume(musicVolume);
    m_game->setSoundEffectsVolume(sfxVolume);
}

void MainWindow::connectSignals() {
    // Connect game state signals
    connect(m_game, &AlgaeGame::gameStateChanged, this, &MainWindow::onGameStateChanged);
    connect(m_game, &AlgaeGame::selectedAlgaeChanged, this, &MainWindow::updateSelectedAlgaeButton);
    connect(m_game, &AlgaeGame::gameWon, this, &MainWindow::onGameWon);

    // Connect resource signals
    connect(m_game->getResources(), &GameResources::resourcesChanged, this, &MainWindow::onResourcesChanged);
    connect(m_game->getResources(),&GameResources::productionRatesChanged, this, &MainWindow::onProductionRatesChanged);
    // Connect grid update signal
    connect(m_game->getGrid(), &GameGrid::gridUpdated, this, &MainWindow::updateGridDisplay);
}

void MainWindow::updateSelectedAlgaeButton() {
    // Reset button styles
    m_btnTypeA->setStyleSheet("");
    m_btnTypeB->setStyleSheet("");
    m_btnTypeC->setStyleSheet("");

    // Set the selected button style
    QString selectedStyle = "background-color: rgba(255, 255, 100, 180); border: 2px solid black;";

    switch (m_game->getSelectedAlgaeType()) {
    case AlgaeType::TYPE_A:
        m_btnTypeA->setStyleSheet(selectedStyle);
        break;
    case AlgaeType::TYPE_B:
        m_btnTypeB->setStyleSheet(selectedStyle);
        break;
    case AlgaeType::TYPE_C:
        m_btnTypeC->setStyleSheet(selectedStyle);
        break;
    default:
        break;
    }
}

void MainWindow::updateGridDisplay() {
    for (int row = 0; row < GameGrid::ROWS; ++row) {
        for (int col = 0; col < GameGrid::COLS; ++col) {
            updateCellDisplay(row, col);
        }
    }
}

void MainWindow::updateCellDisplay(int row, int col) {
    AlgaeCell* cell = m_game->getGrid()->getCell(row, col);
    if (cell && row < m_cellWidgets.size() && col < m_cellWidgets[row].size()) {
        m_cellWidgets[row][col]->setAlgaeType(cell->getType());
        m_cellWidgets[row][col]->setStatus(cell->getStatus());
    }
}

void MainWindow::displayCellInfo(int row, int col) {
    GameGrid* grid = m_game->getGrid();
    AlgaeCell* cell = grid->getCell(row, col);

    if (cell) {
        QString info = tr("位置: (%1,%2)\n").arg(row).arg(col);

        // Add resource info
        info += tr("氮素: %.1f [+%.1f/s]\n")
                    .arg(grid->getNitrogenAt(row, col))
                    .arg(grid->getNitrogenRegenRate(row, col));

        info += tr("二氧化碳: %.1f [+%.1f/s]\n")
                    .arg(grid->getCarbonAt(row, col))
                    .arg(grid->getCarbonRegenRate(row, col));

        info += tr("光照: %.1f\n").arg(grid->getLightAt(row));

        // If cell is occupied, add algae info
        if (cell->isOccupied()) {
            info += tr("藻类类型: %1\n").arg(AlgaeType::getTypeName(cell->getType()));

            QString statusText;
            switch (cell->getStatus()) {
            case AlgaeCell::NORMAL:
                statusText = tr("正常");
                break;
            case AlgaeCell::RESOURCE_LOW:
                statusText = tr("资源不足");
                break;
            case AlgaeCell::LIGHT_LOW:
                statusText = tr("光照不足");
                break;
            case AlgaeCell::DYING:
                statusText = tr("濒临死亡");
                break;
            }
            info += tr("状态: %1\n").arg(statusText);

            // Add production info
            if (cell->getStatus() != AlgaeCell::DYING) {
                info += tr("产出:\n");
                double carbProd = cell->getCarbProduction();
                double lipidProd = cell->getLipidProduction();
                double proProd = cell->getProProduction();
                double vitProd = cell->getVitProduction();

                if (carbProd > 0) info += tr("  糖类: %.1f/s\n").arg(carbProd);
                if (lipidProd > 0) info += tr("  脂质: %.1f/s\n").arg(lipidProd);
                if (proProd > 0) info += tr("  蛋白质: %.1f/s\n").arg(proProd);
                if (vitProd > 0) info += tr("  维生素: %.1f/s\n").arg(vitProd);
            }
        }

        // Show tooltip with cell info
        QToolTip::showText(QCursor::pos(), info);
    }
}

void MainWindow::onCellClicked(int row, int col) {
    // Plant selected algae type on the cell
    bool success = m_game->plantAlgae(row, col);

    if (success) {
        // Play sound
        if (m_sfxPlace->isLoaded()) {
            m_sfxPlace->play();
        }
        statusBar()->showMessage(tr("放置藻类在 (%1,%2)").arg(row).arg(col), 2000);
    } else {
        statusBar()->showMessage(tr("无法放置藻类：资源不足或格子已被占用"), 2000);
    }
}

void MainWindow::onCellRightClicked(int row, int col) {
    // Remove algae from the cell
    bool success = m_game->removeAlgae(row, col);

    if (success) {
        // Play sound
        if (m_sfxRemove->isLoaded()) {
            m_sfxRemove->play();
        }
        statusBar()->showMessage(tr("移除藻类，恢复周围资源"), 2000);
    }
}

void MainWindow::onGameStateChanged() {
    // Update UI based on game state
    if (m_game->isGameRunning()) {
        statusBar()->showMessage(tr("游戏进行中"), 2000);
    } else {
        statusBar()->showMessage(tr("游戏暂停"), 2000);
    }
}

void MainWindow::onResourcesChanged() {
    GameResources* resources = m_game->getResources();

    // Update resource display
    m_lblCarb->setText(QString::number(resources->getCarbohydrates(), 'f', 1));
    m_lblLipid->setText(QString::number(resources->getLipids(), 'f', 1));
    m_lblPro->setText(QString::number(resources->getProteins(), 'f', 1));
    m_lblVit->setText(QString::number(resources->getVitamins(), 'f', 1));

    // Update win progress
    updateWinProgress();
}

void MainWindow::onProductionRatesChanged() {
    GameResources* resources = m_game->getResources();

    // Update rate display
    m_lblCarbRate->setText(QString::number(resources->getCarbRate(), 'f', 1));
    m_lblLipidRate->setText(QString::number(resources->getLipidRate(), 'f', 1));
    m_lblProRate->setText(QString::number(resources->getProRate(), 'f', 1));
    m_lblVitRate->setText(QString::number(resources->getVitRate(), 'f', 1));
}

void MainWindow::updateWinProgress() {
    double progress = m_game->getResources()->getWinProgress();
    m_progressBar->setValue(static_cast<int>(progress * 100));

    // Change color based on progress
    QString styleSheet;
    if (progress < 0.3) {
        styleSheet = "QProgressBar { text-align: center; } QProgressBar::chunk { background-color: #ff3333; }";
    } else if (progress < 0.7) {
        styleSheet = "QProgressBar { text-align: center; } QProgressBar::chunk { background-color: #ffcc33; }";
    } else {
        styleSheet = "QProgressBar { text-align: center; } QProgressBar::chunk { background-color: #33cc33; }";
    }
    m_progressBar->setStyleSheet(styleSheet);
}
