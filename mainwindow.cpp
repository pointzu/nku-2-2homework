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
#include<QApplication>
#include <QPixmap>
#include <QCursor>
#include<QPainterPath>
// CellWidget Implementation
void MainWindow::onGameWon() {
    if (m_hasShownWinMsg) {
        return;
    }
    m_hasShownWinMsg = true;
    // 评分计算
    double carb = m_game->getResources()->getCarbohydrates();
    double lipid = m_game->getResources()->getLipids();
    double pro = m_game->getResources()->getProteins();
    double vit = m_game->getResources()->getVitamins();
    double cr = m_game->getResources()->getCarbRate();
    double lr = m_game->getResources()->getLipidRate();
    double pr = m_game->getResources()->getProRate();
    double vr = m_game->getResources()->getVitRate();
    // 得分上限后只与速率相关
    double resourceScore = (carb/500.0 + lipid/300.0 + pro/200.0 + vit/100.0) / 4.0 * 50.0;
    double rateScore = (cr/50.0 + lr/30.0 + pr/20.0 + vr/10.0) / 4.0 * 50.0;
    int totalScore = static_cast<int>(resourceScore + rateScore + 0.5);
    if (totalScore >= 100) {
        resourceScore = 0.0;
        totalScore = static_cast<int>(rateScore + 0.5);
    }
    if (totalScore > m_highScore) m_highScore = totalScore;
    updateScoreBar();
    // 弹窗提示
    QString msg = tr("恭喜! 你已经成功建立了一个高效且可持续发展的藻类生态系统!\n\n"
        "你的最终成绩:\n"
        "- 糖类: %.1f (产量: %.1f/秒)\n"
        "- 脂质: %.1f (产量: %.1f/秒)\n"
        "- 蛋白质: %.1f (产量: %.1f/秒)\n"
        "- 维生素: %.1f (产量: %.1f/秒)\n"
        "\n评分: %3 分\n最高分: %4 分\n\n"
        "你可以继续挑战，尝试达到更高的资源饱和度和更优的生产组合!\n"
        "（继续游戏可自由优化布局，追求极限分数）"
        )
        .arg(carb).arg(cr)
        .arg(lipid).arg(lr)
        .arg(pro).arg(pr)
        .arg(vit).arg(vr)
        .arg(totalScore)
        .arg(m_highScore);
    QMessageBox::information(this, tr("胜利!"), msg);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        showGameMenu();
        unsetCursor(); // 恢复默认指针
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
    m_hasShownWinMsg = false; // 允许新一轮通关弹窗

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
    // settings.setValue("MusicVolume", m_game->getMusic());
    // settings.setValue("SFXVolume", m_game->getSoundEffects());

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
    // musicSlider->setValue(m_game->getMusic());
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
    // sfxSlider->setValue(m_game->getSoundEffects());
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
        // m_game->setMusicVolume(musicSlider->value());
        // m_game->setSoundEffectsVolume(sfxSlider->value());

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
CellWidget::CellWidget(int row, int col, QWidget* parent)
    : QWidget(parent)
    , m_row(row)
    , m_col(col)
    , m_cell(nullptr)
{
    setMinimumSize(50, 50);
    setMaximumSize(80, 80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void CellWidget::setAlgaeCell(AlgaeCell* cell) {
    m_cell = cell;
    update();
}

void CellWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int cellSize = qMin(width(), height()) - 4;
    QRect cellRect(width()/2 - cellSize/2, height()/2 - cellSize/2, cellSize, cellSize);
    // 1. 蓝色渐变分层背景
    int totalRows = 10; // 假设10行，可根据实际传入
    int rowIdx = m_row;
    double t = totalRows > 1 ? (double)rowIdx / (totalRows - 1) : 0.0;
    QColor topColor = QColor(150, 210, 255, 120 + 40 * (1-t)); // 上层更亮
    QColor bottomColor = QColor(10, 40, 120, 160 + 40 * t);    // 下层更深
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());
    grad.setColorAt(0.0, topColor);
    grad.setColorAt(1.0, bottomColor);
    painter.fillRect(rect(), grad);
    // 2. 光照渐变带（底部）
    if (m_cell) {
        double light = m_cell->parentWidget() ? static_cast<GameGrid*>(m_cell->parentWidget())->getLightAt(m_row) : 0.0;
        int lightBarH = 4;
        int lightBarW = width() - 8;
        QRect lightRect(4, height() - lightBarH - 2, lightBarW, lightBarH);
        int lightVal = qBound(0, static_cast<int>(light), 100);
        QLinearGradient lightGrad(lightRect.topLeft(), lightRect.topRight());
        lightGrad.setColorAt(0.0, QColor(120, 180, 255, 40));
        lightGrad.setColorAt(1.0, QColor(120, 180, 255, 80 + lightVal));
        painter.setBrush(lightGrad);
        painter.setPen(Qt::NoPen);
        painter.drawRect(lightRect);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 7));
        painter.drawText(lightRect, Qt::AlignCenter, QString("光:%1").arg(lightVal));
    }
    // 3. 遮光区可视化（半透明蓝灰，强度递减）
    if (m_cell && m_cell->isShadingVisible()) {
        int depth = 1 + (m_row % 3); // 可根据实际遮光深度
        int alpha = 40 + 30 * (depth-1); // 深度越大越深
        QColor shadeColor = QColor(60, 80, 120, alpha);
        painter.fillRect(cellRect, shadeColor);
    }
    // 4. 藻类图标（加阴影/发光描边）
    if (m_cell && m_cell->getType() != AlgaeType::NONE) {
        AlgaeType::Properties props = AlgaeType::getProperties(m_cell->getType());
        QPixmap pix(props.imagePath);
        if (!pix.isNull()) {
            QPixmap shadow = pix.scaled(cellRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QImage img = shadow.toImage();
            QPainterPath path;
            path.addRect(cellRect);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing, true);
            // 阴影
            painter.setOpacity(0.4);
            painter.drawPixmap(cellRect.adjusted(2,2,2,2), shadow);
            painter.setOpacity(1.0);
            // 图标本体
            painter.drawPixmap(cellRect, pix.scaled(cellRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            // 发光描边
            QPen glowPen(QColor(180,220,255,180), 3);
            painter.setPen(glowPen);
            painter.drawRect(cellRect.adjusted(3,3,-3,-3));
            painter.restore();
        }
    }
    // 5. 右下角小型数值标签（氮素/CO₂/光照）
    if (m_cell) {
        GameGrid* grid = m_cell->parentWidget() ? static_cast<GameGrid*>(m_cell->parentWidget()) : nullptr;
        if (grid) {
            double n = grid->getNitrogenAt(m_row, m_col);
            double c = grid->getCarbonAt(m_row, m_col);
            double l = grid->getLightAt(m_row);
            QString tag = QString("N:%1\nC:%2\nL:%3").arg((int)n).arg((int)c).arg((int)l);
            QRect tagRect(width()-38, height()-32, 36, 30);
            painter.setBrush(QColor(30,40,60,180));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(tagRect, 6, 6);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 7));
            painter.drawText(tagRect, Qt::AlignCenter, tag);
        }
    }
    painter.setPen(QPen(Qt::darkBlue, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(Qt::darkGray);
    QFont smallFont = painter.font();
    smallFont.setPointSize(6);
    painter.setFont(smallFont);
    painter.drawText(rect().adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft,
                     QString::number(m_row) + "," + QString::number(m_col));
    // 显示藻类图片
    if (m_cell && m_cell->getType() != AlgaeType::NONE) {
        AlgaeType::Properties props = AlgaeType::getProperties(m_cell->getType());
        QPixmap pix(props.imagePath);
        if (!pix.isNull()) {
            painter.drawPixmap(cellRect, pix);
        }
    }
    // 状态色块/边框
    if (m_cell) {
        QColor statusColor;
        switch (m_cell->getStatus()) {
            case AlgaeCell::NORMAL: statusColor = QColor(0, 200, 0, 80); break;
            case AlgaeCell::RESOURCE_LOW: statusColor = QColor(255, 165, 0, 120); break;
            case AlgaeCell::LIGHT_LOW: statusColor = QColor(255, 0, 0, 120); break;
            case AlgaeCell::DYING: statusColor = QColor(128, 0, 0, 180); break;
        }
        painter.setPen(QPen(statusColor, 3));
        painter.drawRect(cellRect.adjusted(2, 2, -2, -2));
    }
    // 状态文字
    if (m_cell) {
        QString statusText;
        switch (m_cell->getStatus()) {
            case AlgaeCell::NORMAL: statusText = "正常"; break;
            case AlgaeCell::RESOURCE_LOW: statusText = "资源低"; break;
            case AlgaeCell::LIGHT_LOW: statusText = "光照低"; break;
            case AlgaeCell::DYING: statusText = "濒死"; break;
        }
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setPointSize(8);
        painter.setFont(font);
        painter.drawText(cellRect, Qt::AlignBottom | Qt::AlignHCenter, statusText);
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
{
    m_progressBar = new QProgressBar(this);
    m_lblCarb = new QLabel(this);
    m_lblLipid = new QLabel(this);
    m_lblPro = new QLabel(this);
    m_lblVit = new QLabel(this);
    m_lblCarbRate = new QLabel(this);
    m_lblLipidRate = new QLabel(this);
    m_lblProRate = new QLabel(this);
    m_lblVitRate = new QLabel(this);
    m_btnTypeA = new QPushButton("小型藻类 A", this);
    m_btnTypeB = new QPushButton("小型藻类 B", this);
    m_btnTypeC = new QPushButton("小型藻类 C", this);
    m_iconTypeA = new QLabel(this);
    m_iconTypeB = new QLabel(this);
    m_iconTypeC = new QLabel(this);
    m_cellsLayout = new QGridLayout();
    m_game = new AlgaeGame(this);
    m_gridLayout = new QGridLayout();
    // 初始化鼠标指针
    QPixmap pixA(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png");
    QPixmap pixB(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_b.png");
    QPixmap pixC(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_c.png");
    m_cursorTypeA = QCursor(pixA.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeB = QCursor(pixB.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeC = QCursor(pixC.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_iconTypeA->setPixmap(pixA.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeB->setPixmap(pixB.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeC->setPixmap(pixC.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    setupUI();
    setupGameGrid();
    setupGameControls();
    setupResourceDisplay();
    setupMenus();
    connectSignals();
    initializeCellWidgets();

    setWindowTitle(tr("Algae - 水藻培养策略游戏"));
    setMinimumSize(800, 600);
    resize(1024, 768);

    statusBar()->showMessage(tr("选择藻类并点击网格放置，右键可移除藻类"));
}

MainWindow::~MainWindow() {
    // All Qt parent-child relationships will handle deletion
}

void MainWindow::setupUI() {
    // 主窗口三栏分区
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 左侧：资源、速率、进度、说明
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);

    // 进度条
    QGroupBox* progressGroup = new QGroupBox("通关进度");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    m_progressBar->setMinimumHeight(24);
    m_progressBar->setStyleSheet("QProgressBar {font-weight:bold;} QProgressBar::chunk {background:#4caf50;}");
    progressLayout->addWidget(m_progressBar);
    leftLayout->addWidget(progressGroup);

    // 分数栏
    m_scoreLabel = new QLabel("分数：0   最高分：0", this);
    m_scoreLabel->setStyleSheet("font-weight:bold; color:#1976d2; font-size:15px; padding:2px 0 2px 0;");
    leftLayout->addWidget(m_scoreLabel);

    // 资源
    QGroupBox* resourceGroup = new QGroupBox("资源");
    QGridLayout* resourceLayout = new QGridLayout(resourceGroup);
    resourceLayout->addWidget(new QLabel("糖类:"), 0, 0);
    resourceLayout->addWidget(m_lblCarb, 0, 1);
    resourceLayout->addWidget(new QLabel("脂质:"), 1, 0);
    resourceLayout->addWidget(m_lblLipid, 1, 1);
    resourceLayout->addWidget(new QLabel("蛋白质:"), 2, 0);
    resourceLayout->addWidget(m_lblPro, 2, 1);
    resourceLayout->addWidget(new QLabel("维生素:"), 3, 0);
    resourceLayout->addWidget(m_lblVit, 3, 1);
    leftLayout->addWidget(resourceGroup);

    // 生产速率
    QGroupBox* rateGroup = new QGroupBox("生产速率");
    QGridLayout* rateLayout = new QGridLayout(rateGroup);
    rateLayout->addWidget(new QLabel("糖类/秒:"), 0, 0);
    rateLayout->addWidget(m_lblCarbRate, 0, 1);
    rateLayout->addWidget(new QLabel("脂质/秒:"), 1, 0);
    rateLayout->addWidget(m_lblLipidRate, 1, 1);
    rateLayout->addWidget(new QLabel("蛋白质/秒:"), 2, 0);
    rateLayout->addWidget(m_lblProRate, 2, 1);
    rateLayout->addWidget(new QLabel("维生素/秒:"), 3, 0);
    rateLayout->addWidget(m_lblVitRate, 3, 1);
    leftLayout->addWidget(rateGroup);

    // 胜利条件分组
    m_winConditionGroup = new QGroupBox("胜利条件");
    QVBoxLayout* winCondLayout = new QVBoxLayout(m_winConditionGroup);
    m_lblCarbCond = new QLabel();
    m_lblLipidCond = new QLabel();
    m_lblProCond = new QLabel();
    m_lblVitCond = new QLabel();
    m_lblCarbRateCond = new QLabel();
    m_lblLipidRateCond = new QLabel();
    m_lblProRateCond = new QLabel();
    m_lblVitRateCond = new QLabel();
    winCondLayout->addWidget(m_lblCarbCond);
    winCondLayout->addWidget(m_lblLipidCond);
    winCondLayout->addWidget(m_lblProCond);
    winCondLayout->addWidget(m_lblVitCond);
    winCondLayout->addWidget(m_lblCarbRateCond);
    winCondLayout->addWidget(m_lblLipidRateCond);
    winCondLayout->addWidget(m_lblProRateCond);
    winCondLayout->addWidget(m_lblVitRateCond);
    leftLayout->addWidget(m_winConditionGroup);

    // 游戏说明
    QGroupBox* infoGroup = new QGroupBox("游戏说明");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    QLabel* infoLabel = new QLabel("- 左键放置选中藻类\n- 右键删除已有藻类\n- 鼠标悬浮可查看格子资源\n- ESC 打开菜单\n\n胜利条件:\n- 糖类储备 ≥ 500\n- 脂质储备 ≥ 300\n- 蛋白质储备 ≥ 200\n- 维生素储备 ≥ 100\n- 达到目标生产效率");
    infoLabel->setWordWrap(true);
    infoLayout->addWidget(infoLabel);
    leftLayout->addWidget(infoGroup);
    leftLayout->addStretch(1);

    // 中间：网格
    QWidget* centerPanel = new QWidget;
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    QFrame* gridFrame = new QFrame(centerPanel);
    gridFrame->setFrameShape(QFrame::StyledPanel);
    gridFrame->setFrameShadow(QFrame::Sunken);
    m_cellsLayout->setSpacing(2);
    m_cellsLayout->setContentsMargins(5, 5, 5, 5);
    gridFrame->setLayout(m_cellsLayout);
    centerLayout->addWidget(gridFrame);

    // 右侧：藻类选择与说明
    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(10);
    QGroupBox* controlGroup = new QGroupBox("藻类选择");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    // 图标+按钮并排
    QHBoxLayout* rowA = new QHBoxLayout();
    rowA->addWidget(m_iconTypeA);
    rowA->addWidget(m_btnTypeA);
    QHBoxLayout* rowB = new QHBoxLayout();
    rowB->addWidget(m_iconTypeB);
    rowB->addWidget(m_btnTypeB);
    QHBoxLayout* rowC = new QHBoxLayout();
    rowC->addWidget(m_iconTypeC);
    rowC->addWidget(m_btnTypeC);
    controlLayout->addLayout(rowA);
    controlLayout->addLayout(rowB);
    controlLayout->addLayout(rowC);
    rightLayout->addWidget(controlGroup);
    // 藻类A说明
    QGroupBox* infoA = new QGroupBox("藻类A属性");
    QVBoxLayout* infoALayout = new QVBoxLayout(infoA);
    QLabel* lblInfoA = new QLabel("光照需求: 10/8/5\n种植消耗: 糖×10, 蛋白×5\n遮光效果: 下方1格, -5\n消耗: N×1/秒, C×8/秒\n产出: 糖×5/秒, 蛋白×2/秒\n特性: 同类相邻减产");
    lblInfoA->setWordWrap(true);
    infoALayout->addWidget(lblInfoA);
    rightLayout->addWidget(infoA);
    // 藻类B说明
    QGroupBox* infoB = new QGroupBox("藻类B属性");
    QVBoxLayout* infoBLayout = new QVBoxLayout(infoB);
    QLabel* lblInfoB = new QLabel("光照需求: 12/10/6\n种植消耗: 糖×8, 脂质×6, 维生素×2\n遮光效果: 下方2格, 各-3\n消耗: N×2/秒, C×6/秒\n产出: 糖×3/秒, 脂质×4/秒, 维生素×1/秒\n特性: 提升左右恢复速率");
    lblInfoB->setWordWrap(true);
    infoBLayout->addWidget(lblInfoB);
    rightLayout->addWidget(infoB);
    // 藻类C说明
    QGroupBox* infoC = new QGroupBox("藻类C属性");
    QVBoxLayout* infoCLayout = new QVBoxLayout(infoC);
    QLabel* lblInfoC = new QLabel("光照需求: 8/6/4\n种植消耗: 糖×5, 蛋白×2, 维生素×8\n遮光效果: 无\n消耗: N×2/秒, C×12/秒\n产出: 糖×3/秒, 蛋白×3/秒, 维生素×5/秒\n特性: 与B连接时糖减产");
    lblInfoC->setWordWrap(true);
    infoCLayout->addWidget(lblInfoC);
    rightLayout->addWidget(infoC);
    rightLayout->addStretch(1);

    // 三栏布局
    mainLayout->addWidget(leftPanel, 2);
    mainLayout->addWidget(centerPanel, 5);
    mainLayout->addWidget(rightPanel, 3);

    // 底部操作提示栏
    QLabel* tipLabel = new QLabel("选择藻类并点击网格位置，右键可移除藻类", this);
    tipLabel->setStyleSheet("color: #aaa; font-size: 13px; padding: 4px;");
    statusBar()->addWidget(tipLabel, 1);
}

void MainWindow::initializeCellWidgets() {
    m_cellWidgets.resize(m_game->getGrid()->getRows());
    for (int row = 0; row < m_game->getGrid()->getRows(); ++row) {
        m_cellWidgets[row].resize(m_game->getGrid()->getCols());
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            CellWidget* cellWidget = new CellWidget(row, col, this);
            m_cellWidgets[row][col] = cellWidget;
            AlgaeCell* algaeCell = m_game->getGrid()->getCell(row, col);
            cellWidget->setAlgaeCell(algaeCell);
            m_cellsLayout->addWidget(cellWidget, row, col);
            connect(cellWidget, &CellWidget::leftClicked, this, &MainWindow::onCellClicked);
            connect(cellWidget, &CellWidget::rightClicked, this, &MainWindow::onCellRightClicked);
            connect(algaeCell, &AlgaeCell::cellChanged, this, [this, row, col]() {
                updateCellDisplay(row, col);
            });
        }
    }
}

void MainWindow::setupGameGrid() {
    // 此处不再new m_cellsLayout和m_gridLayout，也不再添加布局，只做数据相关初始化（如有需要可保留数据相关代码）
    // m_cellsLayout和m_gridLayout的初始化和布局已在setupUI中完成
}

void MainWindow::setupGameControls() {
    // Connect algae type selection buttons
    connect(m_btnTypeA, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_A);
        updateSelectedAlgaeButton();
        setCursor(m_cursorTypeA);
    });
    connect(m_btnTypeB, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_B);
        updateSelectedAlgaeButton();
        setCursor(m_cursorTypeB);
    });
    connect(m_btnTypeC, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_C);
        updateSelectedAlgaeButton();
        setCursor(m_cursorTypeC);
    });
    // Set initial selection
    m_game->setSelectedAlgaeType(AlgaeType::TYPE_A);
    updateSelectedAlgaeButton();
    setCursor(m_cursorTypeA);
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
    for (int row = 0; row < m_game->getGrid()->getRows(); ++row) {
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            updateCellDisplay(row, col);
        }
    }
}

void MainWindow::updateCellDisplay(int row, int col) {
    if (row < m_cellWidgets.size() && col < m_cellWidgets[row].size()) {
        m_cellWidgets[row][col]->setAlgaeCell(m_game->getGrid()->getCell(row, col));
    }
}

void MainWindow::displayCellInfo(int row, int col) {
    GameGrid* grid = m_game->getGrid();
    AlgaeCell* cell = grid->getCell(row, col);
    if (cell) {
        QString info = tr("位置: (%1,%2)  氮素: %.1f  二氧化碳: %.1f  光照: %.1f")
            .arg(row).arg(col)
            .arg(grid->getNitrogenAt(row, col))
            .arg(grid->getCarbonAt(row, col))
            .arg(grid->getLightAt(row));
        statusBar()->showMessage(info, 2000);
        // 仍然弹出tooltip
        QToolTip::showText(QCursor::pos(), info);
    }
}

void MainWindow::onCellClicked(int row, int col) {
    // Plant selected algae type on the cell
    bool success = m_game->plantAlgae(row, col);

    if (success) {
        statusBar()->showMessage(tr("放置藻类在 (%1,%2)").arg(row).arg(col), 2000);
        QToolTip::showText(QCursor::pos(), tr("种植成功！"));
    } else {
        statusBar()->showMessage(tr("无法放置藻类：资源不足或格子已被占用"), 2000);
    }
}

void MainWindow::onCellRightClicked(int row, int col) {
    // Remove algae from the cell
    bool success = m_game->removeAlgae(row, col);

    if (success) {
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
    m_lblCarb->setText(QString::number(resources->getCarbohydrates(), 'f', 1));
    m_lblLipid->setText(QString::number(resources->getLipids(), 'f', 1));
    m_lblPro->setText(QString::number(resources->getProteins(), 'f', 1));
    m_lblVit->setText(QString::number(resources->getVitamins(), 'f', 1));
    updateWinProgress();
    updateWinConditionLabels();
    updateScoreBar();
}

void MainWindow::onProductionRatesChanged() {
    GameResources* resources = m_game->getResources();
    m_lblCarbRate->setText(QString::number(resources->getCarbRate(), 'f', 1));
    m_lblLipidRate->setText(QString::number(resources->getLipidRate(), 'f', 1));
    m_lblProRate->setText(QString::number(resources->getProRate(), 'f', 1));
    m_lblVitRate->setText(QString::number(resources->getVitRate(), 'f', 1));
    updateWinConditionLabels();
    updateScoreBar();
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

// 刷新胜利条件标签
void MainWindow::updateWinConditionLabels() {
    GameResources* res = m_game->getResources();
    // 目标值
    const double WIN_CARB = 500.0;
    const double WIN_LIPID = 300.0;
    const double WIN_PRO = 200.0;
    const double WIN_VIT = 100.0;
    const double TARGET_CARB_RATE = 50.0;
    const double TARGET_LIPID_RATE = 30.0;
    const double TARGET_PRO_RATE = 20.0;
    const double TARGET_VIT_RATE = 10.0;
    // 当前值
    double c = res->getCarbohydrates();
    double l = res->getLipids();
    double p = res->getProteins();
    double v = res->getVitamins();
    double cr = res->getCarbRate();
    double lr = res->getLipidRate();
    double pr = res->getProRate();
    double vr = res->getVitRate();
    // 设置文本和颜色
    auto setLabel = [](QLabel* lbl, double val, double target, const QString& name) {
        bool ok = val >= target;
        lbl->setText(QString("%1：%2 / %3 %4").arg(name).arg(val, 0, 'f', 1).arg(target, 0, 'f', 1).arg(ok ? "✅" : "❌"));
        lbl->setStyleSheet(ok ? "color: #2ecc40; font-weight:bold;" : "color: #e67e22;");
    };
    setLabel(m_lblCarbCond, c, WIN_CARB, "糖类");
    setLabel(m_lblLipidCond, l, WIN_LIPID, "脂质");
    setLabel(m_lblProCond, p, WIN_PRO, "蛋白质");
    setLabel(m_lblVitCond, v, WIN_VIT, "维生素");
    setLabel(m_lblCarbRateCond, cr, TARGET_CARB_RATE, "糖类速率");
    setLabel(m_lblLipidRateCond, lr, TARGET_LIPID_RATE, "脂质速率");
    setLabel(m_lblProRateCond, pr, TARGET_PRO_RATE, "蛋白质速率");
    setLabel(m_lblVitRateCond, vr, TARGET_VIT_RATE, "维生素速率");
}

void MainWindow::updateScoreBar() {
    // 评分公式与onGameWon一致
    double carb = m_game->getResources()->getCarbohydrates();
    double lipid = m_game->getResources()->getLipids();
    double pro = m_game->getResources()->getProteins();
    double vit = m_game->getResources()->getVitamins();
    double cr = m_game->getResources()->getCarbRate();
    double lr = m_game->getResources()->getLipidRate();
    double pr = m_game->getResources()->getProRate();
    double vr = m_game->getResources()->getVitRate();
    double resourceScore = (carb/500.0 + lipid/300.0 + pro/200.0 + vit/100.0) / 4.0 * 50.0;
    double rateScore = (cr/50.0 + lr/30.0 + pr/20.0 + vr/10.0) / 4.0 * 50.0;
    int totalScore = static_cast<int>(resourceScore + rateScore + 0.5);
    if (totalScore >= 100) {
        resourceScore = 0.0;
        totalScore = static_cast<int>(rateScore + 0.5);
    }
    if (totalScore > m_highScore) m_highScore = totalScore;
    m_scoreLabel->setText(QString("分数：%1   最高分：%2").arg(totalScore).arg(m_highScore));
}
