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
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QDebug>
#include <QSoundEffect>
#include <QMap>
#include <QTemporaryFile>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>

// CellWidget Implementation
void MainWindow::onGameWon() {
    if (m_hasShownWinMsg) {
        return;
    }
    m_hasShownWinMsg = true;
    playEffect("victory.wav");
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
    // 检查是否完全达标
    bool isFullWin = m_game->getResources()->checkWinCondition();
    if (!isFullWin) {
        totalScore /= 2;
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
    if (!isFullWin) {
        msg += "\n注意：未完全达标，分数已减半。";
    }
    QMessageBox::information(this, tr("胜利!"), msg);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        showGameMenu();
        unsetCursor(); // 恢复默认指针
    }
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Space) {
        if (!m_showShadingPreview) {
            m_showShadingPreview = true;
            updateGridDisplay();
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Space) {
        if (m_showShadingPreview) {
            m_showShadingPreview = false;
            updateGridDisplay();
        }
    }
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::showGameMenu() {
    bool wasRunning = m_game->isGameRunning();
    if (wasRunning) {
        playEffect("qrc:/resources/pause.wav");
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
    musicSlider->setValue(static_cast<int>(m_bgmAudio->volume() * 100));
    musicValueLabel->setText(QString::number(musicSlider->value()));

    connect(musicSlider, &QSlider::valueChanged, [musicValueLabel, this](int value) {
        musicValueLabel->setText(QString::number(value));
        m_bgmAudio->setVolume(value / 100.0);
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
    sfxSlider->setValue(static_cast<int>(m_effectAudio->volume() * 100));
    sfxValueLabel->setText(QString::number(sfxSlider->value()));

    connect(sfxSlider, &QSlider::valueChanged, [sfxValueLabel](int value) {
        sfxValueLabel->setText(QString::number(value));
        // 不做任何音量设置
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
        // 只保存设置，不再重复设置音量
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
    // 蓝色渐变背景
    QColor bgTop(40, 80, 180);
    QColor bgBottom(10, 20, 60);
    QLinearGradient grad(rect().topLeft(), rect().bottomLeft());
    grad.setColorAt(0.0, bgTop);
    grad.setColorAt(1.0, bgBottom);
    painter.fillRect(rect(), grad);
    // 2. 光照渐变带（底部）
    if (m_cell) {
        GameGrid* grid = nullptr;
        if (m_cell->parentWidget()) {
            grid = qobject_cast<GameGrid*>(m_cell->parentWidget());
        }
        double light = 0.0;
        if (grid) light = grid->getLightAt(m_row);
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
        // 图标+文字
        QPixmap iconL(":/icons/light.png");
        if (!iconL.isNull()) painter.drawPixmap(lightRect.left(), lightRect.top()-2, 14, 14, iconL);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 7));
        painter.drawText(lightRect.adjusted(16,0,0,0), Qt::AlignLeft|Qt::AlignVCenter, QString("光:%1").arg(lightVal));
    }
    // 3. 遮光区可视化（半透明蓝灰，强度递减）
    if (m_cell && m_cell->isShadingVisible()) {
        int depth = 1 + (m_row % 3);
        int alpha = 40 + 30 * (depth-1);
        QColor shadeColor = QColor(60, 80, 120, alpha);
        painter.fillRect(cellRect, shadeColor);
    }
    // 4. 藻类图标更亮
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
            painter.setOpacity(0.4);
            painter.drawPixmap(cellRect.adjusted(2,2,2,2), shadow);
            painter.setOpacity(1.0);
            QPixmap brightPix = pix.scaled(cellRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QImage brightImg = brightPix.toImage();
            for(int y=0; y<brightImg.height(); ++y) for(int x=0; x<brightImg.width(); ++x) {
                QColor c = brightImg.pixelColor(x,y);
                c = c.lighter(130);
                brightImg.setPixelColor(x,y,c);
            }
            painter.drawPixmap(cellRect, QPixmap::fromImage(brightImg));
            QPen glowPen(QColor(220,240,255,220), 4);
            painter.setPen(glowPen);
            painter.drawRect(cellRect.adjusted(3,3,-3,-3));
            painter.restore();
        }
    }
    // 顶部中央：藻类状态（仅种植后显示）
    if (m_cell && m_cell->isOccupied()) {
        QString statusText;
        QColor statusColor;
        switch (m_cell->getStatus()) {
            case AlgaeCell::NORMAL: statusText = "正常"; statusColor = QColor(0,255,0); break;
            case AlgaeCell::RESOURCE_LOW: statusText = "资源低"; statusColor = QColor(255,165,0); break;
            case AlgaeCell::LIGHT_LOW: statusText = "光照低"; statusColor = QColor(255,0,0); break;
            case AlgaeCell::DYING: statusText = "濒死"; statusColor = QColor(255,0,128); break;
        }
        QFont font = painter.font();
        font.setPointSize(11);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QPen(statusColor, 2));
        QRect topRect(cellRect.left(), cellRect.top(), cellRect.width(), 22);
        // 状态图标+文字
        QPixmap iconS(":/icons/status.png");
        if (!iconS.isNull()) painter.drawPixmap(topRect.left(), topRect.top()+2, 16, 16, iconS);
        painter.drawText(topRect.adjusted(18,0,0,0), Qt::AlignLeft|Qt::AlignVCenter, statusText);
    }
    // 中央：未种植资源/可否种植标签（更显著）
    if (m_cell && !m_cell->isOccupied()) {
        GameGrid* grid = nullptr;
        if (m_cell->parentWidget()) {
            grid = qobject_cast<GameGrid*>(m_cell->parentWidget());
        }
        if (grid) {
            double n = grid->getNitrogenAt(m_row, m_col);
            double c = grid->getCarbonAt(m_row, m_col);
            double l = 0.0;
            AlgaeType::Type selType = AlgaeType::NONE;
            MainWindow* mw = nullptr;
            QWidget* w = grid->parentWidget();
            while (w && !mw) { mw = qobject_cast<MainWindow*>(w); w = w->parentWidget(); }
            if (mw) selType = mw->getGame()->getSelectedAlgaeType();
            if (selType != AlgaeType::NONE && m_hovered) {
                l = grid->getLightAtIfPlanted(m_row, m_col, selType);
            } else {
                l = grid->getLightAt(m_row);
            }
            // 图标+文字
            QPixmap iconN(":/icons/nitrogen.png");
            QPixmap iconC(":/icons/carbon.png");
            QPixmap iconL(":/icons/light.png");
            int iconY = cellRect.top()+cellRect.height()/2-18;
            int iconX = cellRect.left()+12;
            painter.drawPixmap(iconX, iconY, 14, 14, iconN);
            painter.drawText(iconX+16, iconY+12, QString::number((int)n));
            painter.drawPixmap(iconX+40, iconY, 14, 14, iconC);
            painter.drawText(iconX+56, iconY+12, QString::number((int)c));
            painter.drawPixmap(iconX+80, iconY, 14, 14, iconL);
            painter.drawText(iconX+96, iconY+12, QString::number((int)l));
            // 状态标签
            QString statusTag;
            QColor statusColor = Qt::white;
            if (selType != AlgaeType::NONE) {
                AlgaeType::Properties props = AlgaeType::getProperties(selType);
                bool lightOK = l >= props.lightRequiredPlant;
                bool resOK = AlgaeType::canAfford(selType, mw->getGame()->getResources()->getCarbohydrates(), mw->getGame()->getResources()->getLipids(), mw->getGame()->getResources()->getProteins(), mw->getGame()->getResources()->getVitamins());
                if (!lightOK) { statusTag = "光照不足"; statusColor = QColor(255,0,0); }
                else if (!resOK) { statusTag = "资源不足"; statusColor = QColor(255,165,0); }
                else { statusTag = "可种植"; statusColor = QColor(0,255,0); }
            }
            QRect tagRect(cellRect.left()+8, cellRect.top()+cellRect.height()/2+8, cellRect.width()-16, 28);
            if (!statusTag.isEmpty()) {
                QFont font2("Arial", 14, QFont::Bold);
                painter.setFont(font2);
                painter.setPen(statusColor);
                QColor bg = statusColor; bg.setAlpha(120);
                painter.setBrush(bg);
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(tagRect, 8, 8);
                painter.setPen(statusColor.darker(180));
                painter.drawText(tagRect, Qt::AlignCenter, statusTag);
            }
        }
    }
    if (m_hovered) {
        QColor hoverColor = QColor(255, 255, 0, 180);
        painter.setPen(QPen(hoverColor, 6, Qt::SolidLine));
        painter.drawRect(cellRect.adjusted(2, 2, -2, -2));
        QColor fillColor = QColor(255, 255, 180, 100);
        painter.fillRect(cellRect.adjusted(6, 6, -6, -6), fillColor);
    }
    painter.setPen(QPen(Qt::darkBlue, 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(Qt::darkGray);
    QFont smallFont = painter.font();
    smallFont.setPointSize(6);
    painter.setFont(smallFont);
    painter.drawText(rect().adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft,
                     QString::number(m_row) + "," + QString::number(m_col));

    // 5. 格外下方的高亮资源标注
    GameGrid* grid = nullptr;
    if (m_cell && m_cell->parentWidget()) {
        grid = qobject_cast<GameGrid*>(m_cell->parentWidget());
    }
    if (grid) {
        double n = grid->getNitrogenAt(m_row, m_col);
        double c = grid->getCarbonAt(m_row, m_col);
        double l = grid->getLightAt(m_row, m_col);

        QRect outRect(rect().left(), rect().bottom() + 2, rect().width(), 18);

        QFont font("Arial", 10, QFont::Bold);
        painter.setFont(font);

        // N高亮绿色
        painter.setPen(QColor(0, 220, 0));
        painter.drawText(outRect.adjusted(0, 0, -rect().width()*2/3, 0), Qt::AlignLeft | Qt::AlignVCenter, QString("N:%1").arg((int)n));
        // C高亮蓝色
        painter.setPen(QColor(30, 144, 255));
        painter.drawText(outRect.adjusted(rect().width()/3, 0, -rect().width()/3, 0), Qt::AlignHCenter | Qt::AlignVCenter, QString("C:%1").arg((int)c));
        // L高亮黄色
        painter.setPen(QColor(255, 215, 0));
        painter.drawText(outRect.adjusted(rect().width()*2/3, 0, 0, 0), Qt::AlignRight | Qt::AlignVCenter, QString("L:%1").arg((int)l));
    }
}

void CellWidget::mousePressEvent(QMouseEvent* event) {
    if (!m_cell) return;
    if (event->button() == Qt::LeftButton) {
        emit leftClicked(m_row, m_col);
    } else if (event->button() == Qt::RightButton) {
        emit rightClicked(m_row, m_col);
    }
}

void CellWidget::enterEvent(QEnterEvent* event) {
    Q_UNUSED(event);
    m_hovered = true;
    emit hovered(m_row, m_col);
    update();
    GameGrid* grid = nullptr;
    if (m_cell && m_cell->parentWidget()) {
        grid = qobject_cast<GameGrid*>(m_cell->parentWidget());
    }
    if (grid) {
        int shadeDepth = 2;
        for (int d = 1; d <= shadeDepth; ++d) {
            int r = m_row + d;
            if (r < grid->getRows()) {
                for (int c = m_col-1; c <= m_col+1; ++c) {
                    if (c >= 0 && c < grid->getCols()) {
                        AlgaeCell* cell = grid->getCell(r, c);
                        if (cell) cell->QWidget::update();
                    }
                }
            }
        }
    }
}

void CellWidget::leaveEvent(QEvent* event) {
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
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
    m_scoreLabel = new QLabel(this);
    m_winConditionGroup = new QGroupBox(this);
    m_lblCarbCond = new QLabel(this);
    m_lblLipidCond = new QLabel(this);
    m_lblProCond = new QLabel(this);
    m_lblVitCond = new QLabel(this);
    m_lblCarbRateCond = new QLabel(this);
    m_lblLipidRateCond = new QLabel(this);
    m_lblProRateCond = new QLabel(this);
    m_lblVitRateCond = new QLabel(this);
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
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmAudio = new QAudioOutput(this);
    m_bgmPlayer->setAudioOutput(m_bgmAudio);
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmAudio->setVolume(1.0);
    m_effectPlayer = new QMediaPlayer(this);
    m_effectAudio = new QAudioOutput(this);
    m_effectPlayer->setAudioOutput(m_effectAudio);
    m_effectAudio->setVolume(1.0);
    m_lastBgmProgress = -1.0;
    m_soundEffects.clear();
    setupUI();
    setupGameGrid();
    setupGameControls();
    setupResourceDisplay();
    setupMenus();
    connectSignals();
    initializeCellWidgets();

    setWindowTitle(tr("Algae - 水藻培养策略游戏"));
    setMinimumSize(1024, 768);
    showFullScreen(); // 启动全屏
    restartGame(); // 启动自动重开一局
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
    central->setStyleSheet("background: #222;");

    // 左侧：资源、速率、进度、说明
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);
    leftPanel->setStyleSheet("background: #222;");

    // 进度条
    QGroupBox* progressGroup = new QGroupBox("通关进度");
    QFont groupFont; groupFont.setBold(true); groupFont.setPointSize(12);
    progressGroup->setFont(groupFont);
    progressGroup->setStyleSheet("QGroupBox { color: #fff; font-size: 15px; font-weight: bold; border: 2px solid #444; border-radius: 10px; margin-top: 8px; background: rgba(40,40,40,0.8); }");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    m_progressBar->setMinimumHeight(28);
    m_progressBar->setStyleSheet("QProgressBar { background: #e3f2fd; color: #222; font-weight: bold; border-radius: 8px; } QProgressBar::chunk { background: #4caf50; border-radius: 8px; }");
    progressLayout->addWidget(m_progressBar);
    leftLayout->addWidget(progressGroup);

    // 分数栏
    m_scoreLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 20px; background: #e3f2fd; border-radius: 10px; padding: 6px;");
    leftLayout->addWidget(m_scoreLabel);
    // 新增分数简介信息栏
    m_scoreHintLabel = new QLabel(this);
    m_scoreHintLabel->setStyleSheet("color:#ff9800;font-size:16px;font-weight:bold;background:rgba(255,255,255,0.1);border-radius:8px;padding:4px;");
    leftLayout->addWidget(m_scoreHintLabel);
    // 新增分数组成详细说明栏
    m_scoreDetailLabel = new QLabel(this);
    m_scoreDetailLabel->setStyleSheet("color:#2196f3;font-size:13px;background:rgba(255,255,255,0.08);border-radius:8px;padding:4px;");
    m_scoreDetailLabel->setWordWrap(true);
    leftLayout->addWidget(m_scoreDetailLabel);

    // 资源
    QGroupBox* resourceGroup = new QGroupBox("资源"); resourceGroup->setFont(groupFont);
    resourceGroup->setStyleSheet("QGroupBox { color: #fff; font-size: 15px; font-weight: bold; border: 2px solid #444; border-radius: 10px; margin-top: 8px; background: rgba(40,40,40,0.8); }");
    QGridLayout* resourceLayout = new QGridLayout(resourceGroup);
    auto styleRes = "color: #fff; font-weight:bold; font-size:16px; background: rgba(30,30,30,0.7); border-radius: 8px; padding: 4px;";
    m_lblCarb->setStyleSheet(styleRes); m_lblLipid->setStyleSheet(styleRes); m_lblPro->setStyleSheet(styleRes); m_lblVit->setStyleSheet(styleRes);
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
    QGroupBox* rateGroup = new QGroupBox("生产速率"); rateGroup->setFont(groupFont);
    rateGroup->setStyleSheet("QGroupBox { color: #fff; font-size: 15px; font-weight: bold; border: 2px solid #444; border-radius: 10px; margin-top: 8px; background: rgba(40,40,40,0.8); }");
    QGridLayout* rateLayout = new QGridLayout(rateGroup);
    auto styleRate = "color: #b2ff59; font-weight:bold; font-size:15px; background: rgba(30,30,30,0.7); border-radius: 8px; padding: 4px;";
    m_lblCarbRate->setStyleSheet(styleRate); m_lblLipidRate->setStyleSheet(styleRate); m_lblProRate->setStyleSheet(styleRate); m_lblVitRate->setStyleSheet(styleRate);
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
    m_winConditionGroup->setFont(groupFont);
    m_winConditionGroup->setStyleSheet("QGroupBox { color: #fff; font-size: 15px; font-weight: bold; border: 2px solid #444; border-radius: 10px; margin-top: 8px; background: rgba(40,40,40,0.8); }");
    QFont condFont; condFont.setBold(true); condFont.setPointSize(11);
    m_lblCarbCond->setFont(condFont); m_lblLipidCond->setFont(condFont); m_lblProCond->setFont(condFont); m_lblVitCond->setFont(condFont);
    m_lblCarbRateCond->setFont(condFont); m_lblLipidRateCond->setFont(condFont); m_lblProRateCond->setFont(condFont); m_lblVitRateCond->setFont(condFont);
    m_lblCarbCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblLipidCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblProCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblVitCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblCarbRateCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblLipidRateCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblProRateCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    m_lblVitRateCond->setStyleSheet("color:#fff;background:rgba(30,30,30,0.7);border-radius:6px;padding:2px 8px;");
    leftLayout->addWidget(m_winConditionGroup);

    // 游戏说明
    QGroupBox* infoGroup = new QGroupBox("游戏说明"); infoGroup->setFont(groupFont);
    infoGroup->setStyleSheet("QGroupBox { color: #fff; font-size: 15px; font-weight: bold; border: 2px solid #444; border-radius: 10px; margin-top: 8px; background: rgba(40,40,40,0.8); }");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    QLabel* infoLabel = new QLabel("- 左键放置选中藻类\n- 右键删除已有藻类\n- 鼠标悬浮可查看格子资源\n- ESC 打开菜单\n\n胜利条件:\n- 糖类储备 ≥ 500\n- 脂质储备 ≥ 300\n- 蛋白质储备 ≥ 200\n- 维生素储备 ≥ 100\n- 达到目标生产效率");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("font-size:14px; color:#fff; background:rgba(30,30,30,0.7); border-radius:8px; padding:4px;");
    infoLayout->addWidget(infoLabel);
    leftLayout->addWidget(infoGroup);
    leftLayout->addStretch(1);

    // 中间：网格（自适应拉伸，最大化）
    QWidget* centerPanel = new QWidget;
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    QFrame* gridFrame = new QFrame(centerPanel);
    gridFrame->setFrameShape(QFrame::StyledPanel);
    gridFrame->setFrameShadow(QFrame::Sunken);
    m_cellsLayout->setSpacing(4);
    m_cellsLayout->setContentsMargins(10, 10, 10, 10);
    gridFrame->setLayout(m_cellsLayout);
    centerLayout->addWidget(gridFrame, 1); // 拉伸填满

    // 右侧：藻类选择与说明
    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(10);
    QGroupBox* controlGroup = new QGroupBox("藻类选择"); controlGroup->setFont(groupFont);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    QHBoxLayout* rowA = new QHBoxLayout(); rowA->addWidget(m_iconTypeA); rowA->addWidget(m_btnTypeA);
    QHBoxLayout* rowB = new QHBoxLayout(); rowB->addWidget(m_iconTypeB); rowB->addWidget(m_btnTypeB);
    QHBoxLayout* rowC = new QHBoxLayout(); rowC->addWidget(m_iconTypeC); rowC->addWidget(m_btnTypeC);
    controlLayout->addLayout(rowA); controlLayout->addLayout(rowB); controlLayout->addLayout(rowC);
    rightLayout->addWidget(controlGroup);
    // 藻类说明分组
    QGroupBox* infoA = new QGroupBox("藻类A属性"); infoA->setFont(groupFont);
    QVBoxLayout* infoALayout = new QVBoxLayout(infoA);
    QLabel* lblInfoA = new QLabel();
    lblInfoA->setTextFormat(Qt::RichText);
    lblInfoA->setText("<span style='color:#ffeb3b;font-weight:bold'>光照需求: 22/18/12<br>"
                      "种植消耗: 糖×10, 蛋白×5<br>"
                      "遮光效果: 下方1格, -8<br>"
                      "消耗: N×1/秒, C×8/秒<br>"
                      "产出: 糖×5/秒, 蛋白×2/秒<br>"
                      "特性: 同类相邻减产</span>");
    lblInfoA->setWordWrap(true); lblInfoA->setStyleSheet("font-size:15px; color:#333;"); infoALayout->addWidget(lblInfoA);
    rightLayout->addWidget(infoA);
    QGroupBox* infoB = new QGroupBox("藻类B属性"); infoB->setFont(groupFont);
    QVBoxLayout* infoBLayout = new QVBoxLayout(infoB);
    QLabel* lblInfoB = new QLabel();
    lblInfoB->setTextFormat(Qt::RichText);
    lblInfoB->setText("<span style='color:#ffeb3b;font-weight:bold'>光照需求: 18/14/10<br>"
                      "种植消耗: 糖×8, 脂质×6, 维生素×2<br>"
                      "遮光效果: 下方2格, 各-5<br>"
                      "消耗: N×2/秒, C×6/秒<br>"
                      "产出: 糖×3/秒, 脂质×4/秒, 维生素×1/秒<br>"
                      "特性: 提升左右恢复速率</span>");
    lblInfoB->setWordWrap(true); lblInfoB->setStyleSheet("font-size:15px; color:#333;"); infoBLayout->addWidget(lblInfoB);
    rightLayout->addWidget(infoB);
    QGroupBox* infoC = new QGroupBox("藻类C属性"); infoC->setFont(groupFont);
    QVBoxLayout* infoCLayout = new QVBoxLayout(infoC);
    QLabel* lblInfoC = new QLabel();
    lblInfoC->setTextFormat(Qt::RichText);
    lblInfoC->setText("<span style='color:#ffeb3b;font-weight:bold'>光照需求: 12/8/6<br>"
                      "种植消耗: 糖×5, 蛋白×2, 维生素×8<br>"
                      "遮光效果: 无<br>"
                      "消耗: N×2/秒, C×12/秒<br>"
                      "产出: 糖×3/秒, 蛋白×3/秒, 维生素×5/秒<br>"
                      "特性: 与B连接时糖减产</span>");
    lblInfoC->setWordWrap(true); lblInfoC->setStyleSheet("font-size:15px; color:#333;"); infoCLayout->addWidget(lblInfoC);
    rightLayout->addWidget(infoC);
    rightLayout->addStretch(1);

    // 三栏布局
    mainLayout->addWidget(leftPanel, 2);
    mainLayout->addWidget(centerPanel, 8); // 网格区域最大
    mainLayout->addWidget(rightPanel, 2);
}

void MainWindow::initializeCellWidgets() {
    m_cellWidgets.resize(m_game && m_game->getGrid() ? m_game->getGrid()->getRows() : 0);
    for (int row = 0; m_game && m_game->getGrid() && row < m_game->getGrid()->getRows(); ++row) {
        m_cellWidgets[row].resize(m_game->getGrid()->getCols());
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            CellWidget* cellWidget = new CellWidget(row, col, this);
            m_cellWidgets[row][col] = cellWidget;
            AlgaeCell* algaeCell = m_game->getGrid()->getCell(row, col);
            if (algaeCell) {
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
    if (!m_game || !m_game->getGrid()) return;
    for (int row = 0; row < m_game->getGrid()->getRows(); ++row) {
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            updateCellDisplay(row, col);
        }
    }
}

void MainWindow::updateCellDisplay(int row, int col) {
    if (!m_game || !m_game->getGrid()) return;
    if (row < m_cellWidgets.size() && col < m_cellWidgets[row].size()) {
        AlgaeCell* cell = m_game->getGrid()->getCell(row, col);
        if (cell) m_cellWidgets[row][col]->setAlgaeCell(cell);
    }
}

void MainWindow::displayCellInfo(int row, int col) {
    if (!m_game || !m_game->getGrid()) return;
    GameGrid* grid = m_game->getGrid();
    AlgaeCell* cell = grid->getCell(row, col);
    if (cell) {
        QString info = tr("位置: (%1,%2)  氮素: %.1f  二氧化碳: %.1f  光照: %.1f")
            .arg(row).arg(col)
            .arg(grid->getNitrogenAt(row, col))
            .arg(grid->getCarbonAt(row, col))
            .arg(grid->getLightAt(row));
        statusBar()->showMessage(info, 2000);
        QToolTip::showText(QCursor::pos(), info);
    }
}

void MainWindow::playSoundEffect(const QString& resource) {
    auto player = new QMediaPlayer(this);
    auto audio = new QAudioOutput(this);
    player->setAudioOutput(audio);
    audio->setVolume(1.0); // 固定最大音量
    player->setSource(QUrl(resource));
    player->play();
    connect(player, &QMediaPlayer::mediaStatusChanged, player, [player, audio](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia) {
            player->deleteLater();
            audio->deleteLater();
        }
    });
}

void MainWindow::onCellClicked(int row, int col) {
    bool success = m_game->plantAlgae(row, col);
    if (success) {
        statusBar()->showMessage(tr("放置藻类在 (%1,%2)").arg(row).arg(col), 2000);
        playEffect("planted.mp3");
    } else {
        playEffect("buzzer.wav");
    }
    updateCellDisplay(row, col);
}

void MainWindow::onCellRightClicked(int row, int col) {
    bool success = m_game->removeAlgae(row, col);
    if (success) {
        playEffect("displant.wav");
        statusBar()->showMessage(tr("移除藻类，恢复周围资源"), 2000);
    } else {
        playEffect("buzzer.wav");
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
    // 目标值
    const double WIN_CARB = 500.0;
    const double WIN_LIPID = 300.0;
    const double WIN_PRO = 200.0;
    const double WIN_VIT = 100.0;
    // 当前值
    double c = resources->getCarbohydrates();
    double l = resources->getLipids();
    double p = resources->getProteins();
    double v = resources->getVitamins();
    // 达标判断
    auto resStyle = [](bool ok) { return ok ? "color:#2ecc40;font-weight:bold;" : "color:#e67e22;"; };
    m_lblCarb->setText(QString("%1 / %2 %3").arg(QString::number(c, 'f', 1)).arg(WIN_CARB, 0, 'f', 0).arg(c >= WIN_CARB ? "✅" : "❌"));
    m_lblCarb->setStyleSheet(resStyle(c >= WIN_CARB));
    m_lblLipid->setText(QString("%1 / %2 %3").arg(QString::number(l, 'f', 1)).arg(WIN_LIPID, 0, 'f', 0).arg(l >= WIN_LIPID ? "✅" : "❌"));
    m_lblLipid->setStyleSheet(resStyle(l >= WIN_LIPID));
    m_lblPro->setText(QString("%1 / %2 %3").arg(QString::number(p, 'f', 1)).arg(WIN_PRO, 0, 'f', 0).arg(p >= WIN_PRO ? "✅" : "❌"));
    m_lblPro->setStyleSheet(resStyle(p >= WIN_PRO));
    m_lblVit->setText(QString("%1 / %2 %3").arg(QString::number(v, 'f', 1)).arg(WIN_VIT, 0, 'f', 0).arg(v >= WIN_VIT ? "✅" : "❌"));
    m_lblVit->setStyleSheet(resStyle(v >= WIN_VIT));
    updateWinProgress();
    updateWinConditionLabels();
    updateScoreBar();
    onProductionRatesChanged();
}

void MainWindow::onProductionRatesChanged() {
    GameResources* resources = m_game->getResources();
    // 目标速率
    const double TARGET_CARB_RATE = 50.0;
    const double TARGET_LIPID_RATE = 30.0;
    const double TARGET_PRO_RATE = 20.0;
    const double TARGET_VIT_RATE = 10.0;
    double cr = resources->getCarbRate();
    double lr = resources->getLipidRate();
    double pr = resources->getProRate();
    double vr = resources->getVitRate();
    auto rateStyle = [](bool ok) { return ok ? "color:#b2ff59;font-weight:bold;" : "color:#e67e22;"; };
    m_lblCarbRate->setText(QString("%1 / %2 %3").arg(QString::number(cr, 'f', 1)).arg(TARGET_CARB_RATE, 0, 'f', 0).arg(cr >= TARGET_CARB_RATE ? "✅" : "❌"));
    m_lblCarbRate->setStyleSheet(rateStyle(cr >= TARGET_CARB_RATE));
    m_lblLipidRate->setText(QString("%1 / %2 %3").arg(QString::number(lr, 'f', 1)).arg(TARGET_LIPID_RATE, 0, 'f', 0).arg(lr >= TARGET_LIPID_RATE ? "✅" : "❌"));
    m_lblLipidRate->setStyleSheet(rateStyle(lr >= TARGET_LIPID_RATE));
    m_lblProRate->setText(QString("%1 / %2 %3").arg(QString::number(pr, 'f', 1)).arg(TARGET_PRO_RATE, 0, 'f', 0).arg(pr >= TARGET_PRO_RATE ? "✅" : "❌"));
    m_lblProRate->setStyleSheet(rateStyle(pr >= TARGET_PRO_RATE));
    m_lblVitRate->setText(QString("%1 / %2 %3").arg(QString::number(vr, 'f', 1)).arg(TARGET_VIT_RATE, 0, 'f', 0).arg(vr >= TARGET_VIT_RATE ? "✅" : "❌"));
    m_lblVitRate->setStyleSheet(rateStyle(vr >= TARGET_VIT_RATE));
}

void MainWindow::updateWinProgress() {
    double progress = m_game->getResources()->getWinProgress();
    m_progressBar->setValue(static_cast<int>(progress * 100));
    playBGM(progress);
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
    // 新增分数简介信息栏内容
    QString hint;
    if (totalScore < 60) hint = "继续努力，优化藻类布局！";
    else if (totalScore < 80) hint = "良好，距离目标不远了！";
    else if (totalScore < 100) hint = "优秀，快达成极限生产！";
    else hint = "极限挑战，追求更高分数！";
    m_scoreHintLabel->setText(hint);
    // 新增分数组成详细说明
    QString detail = QString(
        "当前分数组成：\n"
        "资源得分：%1\n"
        "速率得分：%2\n"
        "总分：%3\n"
        "<span style='color:#888'>资源得分 = (糖/500 + 脂/300 + 蛋白/200 + 维生素/100) / 4 × 50<br>"
        "速率得分 = (糖速/50 + 脂速/30 + 蛋白速/20 + 维生素速/10) / 4 × 50<br>"
        "总分 = 资源得分 + 速率得分（满100分后只看速率得分）</span>"
    ).arg(QString::number(resourceScore, 'f', 1))
     .arg(QString::number(rateScore, 'f', 1))
     .arg(QString::number(totalScore));
    m_scoreDetailLabel->setText(detail);
}

void MainWindow::playBGM(double progress) {
    int bgmType = (progress < 0.5) ? 1 : 2;
    if ((m_lastBgmProgress < 0.5 && progress >= 0.5) || (m_lastBgmProgress >= 0.5 && progress < 0.5) || m_lastBgmProgress < 0) {
        if (bgmType == 1) {
            m_bgmPlayer->setSource(QUrl("qrc:/resources/st30f0n665joahrrvuj05fechvwkcv10/background_music1.wav"));
        } else {
            m_bgmPlayer->setSource(QUrl("qrc:/resources/st30f0n665joahrrvuj05fechvwkcv10/background_music2.wav"));
        }
        m_bgmPlayer->play();
    }
    m_lastBgmProgress = progress;
}

void MainWindow::playEffect(const QString& name) {
    QString path = "qrc:/resources/st30f0n665joahrrvuj05fechvwkcv10/" + name;
    playSoundEffect(path);
}
