#include "mainwindow.h" // 主窗口头文件
#include <QVBoxLayout>    // 垂直布局
#include <QHBoxLayout>    // 水平布局
#include <QGroupBox>      // 分组框
#include <QKeyEvent>      // 键盘事件
#include <QMessageBox>    // 消息框
#include <QInputDialog>   // 输入对话框
#include <QPainter>       // 绘图
#include <QMouseEvent>    // 鼠标事件
#include <QEnterEvent>    // 鼠标进入事件
#include <QToolTip>       // 工具提示
#include <QColor>         // 颜色
#include <QFont>          // 字体
#include <QStatusBar>     // 状态栏
#include <QSplitter>      // 分割器
#include <QFrame>         // 框架
#include <QTimer>         // 定时器
#include <QSettings>      // 设置
#include<QApplication>    // 应用程序
#include <QPixmap>        // 图片
#include <QCursor>        // 鼠标指针
#include<QPainterPath>    // 绘图路径
#include <QtMultimedia/QMediaPlayer> // 多媒体播放器
#include <QtMultimedia/QAudioOutput> // 音频输出
#include <QDebug>         // 调试输出
#include <QSoundEffect>   // 音效
#include <QMap>           // 字典
#include <QTemporaryFile> // 临时文件
#include <QFile>          // 文件
#include <QStandardPaths> // 标准路径
#include <QDir>           // 目录
#include <QUrl>           // URL
#include <QTextDocument>  // 文本文档
#include <QScrollArea>    // 滚动区域
#include <QPushButton>    // 按钮
#include <QDialog>

// =================== CellWidget实现部分 ===================
// 游戏胜利时的处理函数
void MainWindow::onGameWon() {
    if (m_hasShownWinMsg) { // 已弹出胜利提示则不再弹出
        return;
    }
    m_hasShownWinMsg = true;
    playEffect("victory.wav"); // 播放胜利音效
    // 评分计算
    double carb = m_game->getResources()->getCarbohydrates(); // 当前糖类
    double lipid = m_game->getResources()->getLipids();       // 当前脂质
    double pro = m_game->getResources()->getProteins();       // 当前蛋白质
    double vit = m_game->getResources()->getVitamins();       // 当前维生素
    double cr = m_game->getResources()->getCarbRate();        // 糖类速率
    double lr = m_game->getResources()->getLipidRate();       // 脂质速率
    double pr = m_game->getResources()->getProRate();         // 蛋白质速率
    double vr = m_game->getResources()->getVitRate();         // 维生素速率
    // 资源得分和速率得分
    double resourceScore = (carb/500.0 + lipid/300.0 + pro/200.0 + vit/100.0) / 4.0 * 50.0;
    double rateScore = (cr/50.0 + lr/30.0 + pr/20.0 + vr/10.0) / 4.0 * 50.0;
    int totalScore = static_cast<int>(resourceScore + rateScore + 0.5);
    if (totalScore >= 100) { // 满分后只看速率
        resourceScore = 0.0;
        totalScore = static_cast<int>(rateScore + 0.5);
    }
    // 检查是否完全达标
    bool isFullWin = m_game->getResources()->checkWinCondition();
    if (!isFullWin) {
        totalScore /= 2; // 未完全达标分数减半
    }
    if (totalScore > m_highScore) m_highScore = totalScore; // 更新最高分
    updateScoreBar(); // 刷新分数栏
    // 优化弹窗内容
    QString msg = tr("恭喜你通关！你已建立高效可持续的藻类生态系统！\n\n");
    msg += tr("【当前资源】\n");
    msg += tr("  糖类：%1 / 500\t脂质：%2 / 300\n  蛋白质：%3 / 200\t维生素：%4 / 100\n").arg(QString::number(carb, 'f', 1)).arg(QString::number(lipid, 'f', 1)).arg(QString::number(pro, 'f', 1)).arg(QString::number(vit, 'f', 1));
    msg += tr("【当前生产速率】\n");
    msg += tr("  糖类：%1 / 50/秒\t脂质：%2 / 30/秒\n  蛋白质：%3 / 20/秒\t维生素：%4 / 10/秒\n").arg(QString::number(cr, 'f', 1)).arg(QString::number(lr, 'f', 1)).arg(QString::number(pr, 'f', 1)).arg(QString::number(vr, 'f', 1));
    msg += tr("\n【分数构成】\n  资源得分：%1\n  速率得分：%2\n  总分：%3\n  最高分：%4\n").arg(QString::number(resourceScore, 'f', 1)).arg(QString::number(rateScore, 'f', 1)).arg(totalScore).arg(m_highScore);
    if (!isFullWin) {
        msg += tr("\n注意：未完全达标，分数已减半。\n");
    } else {
        msg += tr("\n已完全达标，可继续挑战更高分数！\n");
    }
    msg += tr("\n你可以继续优化布局，追求极限分数！\n");
    QMessageBox::information(this, tr("胜利！"), msg); // 弹出胜利提示
}

// 键盘按下事件处理
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) { // 按ESC打开菜单
        showGameMenu();
        unsetCursor(); // 恢复默认指针
    }
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Space) {
        if (!m_showShadingPreview) {
            m_showShadingPreview = true;
            updateGridDisplay(); // 显示遮荫预览
        }
    }
    QMainWindow::keyPressEvent(event); // 继续父类处理
}

// 键盘释放事件处理
void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Space) {
        if (m_showShadingPreview) {
            m_showShadingPreview = false;
            updateGridDisplay(); // 关闭遮荫预览
        }
    }
    QMainWindow::keyReleaseEvent(event);
}

// 显示游戏菜单
void MainWindow::showGameMenu() {
    bool wasRunning = m_game->isGameRunning(); // 记录游戏是否在运行
    if (wasRunning) {
        if (m_bgmPlayer) m_bgmPlayer->pause(); // 暂停背景音乐
        m_game->pauseGame(); // 暂停游戏
    }

    // 创建弹出菜单
    QMenu popupMenu(this);
    QAction* continueAction = nullptr;

    if (wasRunning) {
        continueAction = popupMenu.addAction(tr("继续游戏"));
    }

    QAction* restartAction = popupMenu.addAction(tr("重新开始"));
    QAction* settingsAction = popupMenu.addAction(tr("设置"));
    popupMenu.addSeparator();
    QAction* exitAction = popupMenu.addAction(tr("退出"));

    QAction* selectedAction = popupMenu.exec(QCursor::pos()); // 在鼠标处弹出

    if (selectedAction == continueAction) {
        m_game->startGame(); // 继续游戏
        if (m_bgmPlayer) m_bgmPlayer->play(); // 恢复背景音乐
    } else if (selectedAction == restartAction) {
        restartGame();
    } else if (selectedAction == settingsAction) {
        showSettingsDialog();
        if (wasRunning) {
            m_game->startGame(); // 设置后继续
            if (m_bgmPlayer) m_bgmPlayer->play(); // 恢复背景音乐
        }
    } else if (selectedAction == exitAction) {
        exitGame();
    } else if (wasRunning) {
        // 菜单取消但游戏原本在运行，恢复
        m_game->startGame();
        if (m_bgmPlayer) m_bgmPlayer->play(); // 恢复背景音乐
    }
}

// 重新开始游戏
void MainWindow::restartGame() {
    // 游戏进行中需确认
    if (m_game->isGameRunning()) {
        if (QMessageBox::question(this, tr("重新开始?"),
                                  tr("确定要重新开始游戏吗? 当前进度将丢失。"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }

    // 重置游戏
    m_game->resetGame();
    m_game->startGame();
    m_hasShownWinMsg = false; // 允许新一轮通关弹窗

    // 刷新UI
    updateGridDisplay();
    onResourcesChanged();
    onProductionRatesChanged();

    statusBar()->showMessage(tr("游戏已重新开始"), 2000);
    if (m_bgmPlayer) m_bgmPlayer->play();
}

// 退出游戏
void MainWindow::exitGame() {
    // 游戏进行中需确认
    if (m_game->isGameRunning()) {
        if (QMessageBox::question(this, tr("退出游戏?"),
                                  tr("确定要退出游戏吗? 未保存的进度将丢失。"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }

    // 保存设置
    QSettings settings("AlgaeGame", "Settings");
    // settings.setValue("MusicVolume", m_game->getMusic());
    // settings.setValue("SFXVolume", m_game->getSoundEffects());

    // 关闭应用
    QApplication::quit();
}

// 显示设置对话框
void MainWindow::showSettingsDialog() {
    // 暂停游戏
    bool wasRunning = m_game->isGameRunning();
    if (wasRunning) {
        m_game->pauseGame();
    }

    // 创建对话框
    QDialog dialog(this);
    dialog.setWindowTitle(tr("游戏设置"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // 背景音乐音量
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

    // 音效音量
    QHBoxLayout* sfxLayout = new QHBoxLayout();
    QLabel* sfxLabel = new QLabel(tr("音效音量:"));
    QSlider* sfxSlider = new QSlider(Qt::Horizontal);
    QLabel* sfxValueLabel = new QLabel();

    sfxSlider->setRange(0, 100);
    sfxSlider->setValue(static_cast<int>(m_effectVolume * 100));
    sfxValueLabel->setText(QString::number(sfxSlider->value()));

    connect(sfxSlider, &QSlider::valueChanged, [sfxValueLabel, this](int value) {
        sfxValueLabel->setText(QString::number(value));
        m_effectVolume = value / 100.0;
        m_effectAudio->setVolume(m_effectVolume);
    });

    sfxLayout->addWidget(sfxLabel);
    sfxLayout->addWidget(sfxSlider);
    sfxLayout->addWidget(sfxValueLabel);

    // 对话框按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // 添加控件到布局
    layout->addLayout(musicLayout);
    layout->addLayout(sfxLayout);
    layout->addWidget(buttonBox);

    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        // 只保存设置，不再重复设置音量
        QSettings settings("AlgaeGame", "Settings");
        settings.setValue("MusicVolume", musicSlider->value());
        settings.setValue("SFXVolume", sfxSlider->value());
    }

    // 恢复游戏
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

        // 第一行：N、C，第二行：L
        QRect outRect1(rect().left(), rect().bottom() - 44, rect().width(), 12); // N、C行更上移
        QRect outRect2(rect().left(), rect().bottom() - 28, rect().width(), 16); // L行更上移

        // N、C行，8号加粗
        QFont fontNC("Arial", 8, QFont::Bold);
        painter.setFont(fontNC);
        QString ncText = QString("<span style='color:#fff;'>N:%1 C:%2</span>")
                            .arg((int)n).arg((int)c);
        QTextDocument docNC;
        docNC.setHtml(QString("<div align='center' style='line-height:11px;margin:0;padding:0;'>%1</div>").arg(ncText));
        painter.save();
        painter.translate(outRect1.left(), outRect1.top());
        docNC.setTextWidth(outRect1.width());
        docNC.drawContents(&painter, QRectF(0, 0, outRect1.width(), outRect1.height()));
        painter.restore();

        // L行，10号加粗
        QFont fontL("Arial", 10, QFont::Bold);
        painter.setFont(fontL);
        QString lText = QString("<span style='color:#fff;'>L:%1</span>").arg((int)l);
        QTextDocument docL;
        docL.setHtml(QString("<div align='center' style='line-height:14px;margin:0;padding:0;'>%1</div>").arg(lText));
        painter.save();
        painter.translate(outRect2.left(), outRect2.top());
        docL.setTextWidth(outRect2.width());
        docL.drawContents(&painter, QRectF(0, 0, outRect2.width(), outRect2.height()));
        painter.restore();
    }

    // --- 藻类特性可视化 ---
    if (m_cell) {
        // A型相邻减产：左上角红色圆底白色粗体"-"
        if (m_cell->getType() == AlgaeType::TYPE_A && m_cell->isReducedByNeighborA()) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.left()+2, cellRect.top()+2, r, r);
            painter.setBrush(QColor(220,0,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "-");
            painter.restore();
        }
        // B型被加速：右上角绿色圆底白色粗体"+"
        if (m_cell->isBoostedByNeighborB()) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.right()-r-2, cellRect.top()+2, r, r);
            painter.setBrush(QColor(0,180,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "+");
            painter.restore();
        }
        // C型被B减产：右下角黄色圆底黑色粗体"!"
        if (m_cell->getType() == AlgaeType::TYPE_C && m_cell->isReducedByNeighborB()) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.right()-r-2, cellRect.bottom()-r-2, r, r);
            painter.setBrush(QColor(255,220,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::black);
            painter.drawText(markRect, Qt::AlignCenter, "!");
            painter.restore();
        }
        // D型被协同：左下角蓝色圆底白色粗体"★"
        if (m_cell->getType() == AlgaeType::TYPE_D && m_cell->isSynergizingNeighbor()) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.left()+2, cellRect.bottom()-r-2, r, r);
            painter.setBrush(QColor(0,120,255));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "★");
            painter.restore();
        }
        // 被D型协同的A/B/C型：右下角蓝色圆底白色粗体"↑"
        if (m_cell->isSynergizedByNeighbor() && m_cell->getType() != AlgaeType::TYPE_D) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.right()-r-2, cellRect.bottom()-r-2, r, r);
            painter.setBrush(QColor(0,120,255));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "↑");
            painter.restore();
        }
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

// =================== MainWindow实现部分 ===================
StartWindow::StartWindow(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Algae");
    setModal(true);
    setFixedSize(700, 480);
    // 只设置背景色，图片在paintEvent绘制
    setStyleSheet("");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    btnStart = new QPushButton("开始游戏", this);
    btnStart->setMinimumHeight(48);
    btnStart->setStyleSheet("font-size:28px;font-weight:bold;color:#fff;background:#43a047;border-radius:12px;margin:24px 0;");
    layout->addWidget(btnStart);
    btnInfo = new QPushButton("游戏说明", this);
    btnInfo->setMinimumHeight(36);
    btnInfo->setStyleSheet("font-size:22px;font-weight:bold;color:#fff;background:#1976d2;border-radius:12px;margin-bottom:12px;");
    layout->addWidget(btnInfo);
    connect(btnStart, &QPushButton::clicked, this, [this]() { accept(); });
}

void StartWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QPixmap bg(":/startbackground.jpg");
    if (!bg.isNull()) {
        painter.drawPixmap(rect(), bg);
    }
    QDialog::paintEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QPixmap bg(":/background.jpg");
    if (!bg.isNull()) {
        painter.drawPixmap(rect(), bg);
    }
    QMainWindow::paintEvent(event);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 启动界面
    StartWindow startWin;
    connect(startWin.btnInfo, &QPushButton::clicked, [&]() {
        QMessageBox::information(&startWin, "游戏说明",
            "欢迎来到水藻策略游戏！\n\n目标：\n- 合理布局不同类型藻类，提升资源产量，达成通关条件。\n\n操作：\n- 左键放置藻类，右键移除藻类。\n- 鼠标悬浮查看格子资源。\n- ESC 打开菜单。\n\n胜利条件：\n- 糖类≥500，脂质≥300，蛋白质≥200，维生素≥100，且生产速率达标。\n\n祝你游戏愉快！");
    });
    if (startWin.exec() != QDialog::Accepted) {
        qApp->exit();
        return;
    }

    m_progressBar = new QProgressBar(this); // 进度条
    m_lblCarb = new QLabel(this);           // 糖类标签
    m_lblLipid = new QLabel(this);          // 脂质标签
    m_lblPro = new QLabel(this);            // 蛋白质标签
    m_lblVit = new QLabel(this);            // 维生素标签
    m_lblCarbRate = new QLabel(this);       // 糖类速率标签
    m_lblLipidRate = new QLabel(this);      // 脂质速率标签
    m_lblProRate = new QLabel(this);        // 蛋白质速率标签
    m_lblVitRate = new QLabel(this);        // 维生素速率标签
    m_btnTypeA = new QPushButton("螺旋藻 Spirulina", this); // 螺旋藻按钮
    m_btnTypeB = new QPushButton("小球藻 Chlorella", this); // 小球藻按钮
    m_btnTypeC = new QPushButton("小型硅藻 Cyclotella", this); // 小型硅藻按钮
    m_btnTypeD = new QPushButton("裸藻 Euglena", this); // 裸藻按钮
    m_btnTypeE = new QPushButton("蓝藻 Cyanobacteria", this); // 蓝藻按钮
    m_iconTypeA = new QLabel(this); // A型图标
    m_iconTypeB = new QLabel(this); // B型图标
    m_iconTypeC = new QLabel(this); // C型图标
    m_iconTypeD = new QLabel(this); // D型图标
    m_iconTypeE = new QLabel(this); // E型图标
    m_cellsLayout = new QGridLayout(); // 网格布局
    m_game = new AlgaeGame(this);      // 游戏主逻辑
    m_gridLayout = new QGridLayout();  // 主网格布局
    m_scoreLabel = new QLabel(this);   // 分数栏
    m_winConditionGroup = new QGroupBox(this); // 胜利条件分组
    m_lblCarbCond = new QLabel(this);  // 糖类条件
    m_lblLipidCond = new QLabel(this); // 脂质条件
    m_lblProCond = new QLabel(this);   // 蛋白质条件
    m_lblVitCond = new QLabel(this);   // 维生素条件
    m_lblCarbRateCond = new QLabel(this); // 糖类速率条件
    m_lblLipidRateCond = new QLabel(this); // 脂质速率条件
    m_lblProRateCond = new QLabel(this);   // 蛋白质速率条件
    m_lblVitRateCond = new QLabel(this);   // 维生素速率条件
    // 初始化鼠标指针
    pixA = QPixmap(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png");
    pixB = QPixmap(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_b.png");
    pixC = QPixmap(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_c.png");
    pixD = QPixmap(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_d.png");
    pixE = QPixmap(":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_e.png");
    m_cursorTypeA = QCursor(pixA.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeB = QCursor(pixB.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeC = QCursor(pixC.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeD = QCursor(pixD.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_cursorTypeE = QCursor(pixE.scaled(32,32,Qt::KeepAspectRatio,Qt::SmoothTransformation), 0, 0);
    m_iconTypeA->setPixmap(pixA.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeB->setPixmap(pixB.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeC->setPixmap(pixC.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeD->setPixmap(pixD.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeE->setPixmap(pixE.scaled(24,24,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_bgmPlayer = new QMediaPlayer(this); // 背景音乐播放器
    m_bgmAudio = new QAudioOutput(this);  // 背景音乐输出
    m_bgmPlayer->setAudioOutput(m_bgmAudio);
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmAudio->setVolume(1.0);
    m_effectPlayer = new QMediaPlayer(this); // 音效播放器
    m_effectAudio = new QAudioOutput(this);  // 音效输出
    m_effectPlayer->setAudioOutput(m_effectAudio);
    m_effectAudio->setVolume(1.0);
    m_lastBgmProgress = -1.0;
    m_soundEffects.clear();
    // 新增：读取音效音量设置
    QSettings settings("AlgaeGame", "Settings");
    m_effectVolume = settings.value("SFXVolume", 100).toInt() / 100.0;
    m_effectAudio->setVolume(m_effectVolume);
    setupUI();                // 初始化UI
    setupGameGrid();          // 初始化网格
    setupGameControls();      // 初始化控制按钮
    setupResourceDisplay();   // 初始化资源显示
    setupMenus();             // 初始化菜单
    connectSignals();         // 连接信号槽
    initializeCellWidgets();  // 初始化格子控件

    setWindowTitle(tr("Algae")); // 设置窗口标题
    setMinimumSize(1024, 768); // 最小尺寸
    showFullScreen(); // 启动全屏
    restartGame(); // 启动自动重开一局
}

MainWindow::~MainWindow() {
    // 所有Qt父子关系会自动释放资源
}

// 初始化UI布局
void MainWindow::setupUI() {
    // 主窗口三栏分区
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    // 移除背景图片QSS，交由paintEvent处理
    // central->setStyleSheet("background-image: url('qrc:/background.jpg'); background-repeat: no-repeat; background-position: center; background-size: cover;");

    // 左侧：资源、速率、进度、说明
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);
    leftPanel->setStyleSheet("background: rgba(30,30,30,0.75); border-radius: 16px;");

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
    centerPanel->setStyleSheet("background: rgba(30,30,30,0); border-radius: 16px;");
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
    rightLayout->setSpacing(8); // 更紧凑
    QGroupBox* controlGroup = new QGroupBox("藻类选择"); controlGroup->setFont(groupFont);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    // 放大按钮和图标
    m_iconTypeA->setPixmap(pixA.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeB->setPixmap(pixB.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeC->setPixmap(pixC.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeD->setPixmap(pixD.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_iconTypeE->setPixmap(pixE.scaled(100,100,Qt::KeepAspectRatio,Qt::SmoothTransformation));
    m_btnTypeA->setMinimumHeight(72); m_btnTypeA->setStyleSheet("font-size:28px;font-weight:bold;");
    m_btnTypeB->setMinimumHeight(72); m_btnTypeB->setStyleSheet("font-size:28px;font-weight:bold;");
    m_btnTypeC->setMinimumHeight(72); m_btnTypeC->setStyleSheet("font-size:28px;font-weight:bold;");
    m_btnTypeD->setMinimumHeight(72); m_btnTypeD->setStyleSheet("font-size:28px;font-weight:bold;");
    m_btnTypeE->setMinimumHeight(72); m_btnTypeE->setStyleSheet("font-size:28px;font-weight:bold;");
    QHBoxLayout* rowA = new QHBoxLayout(); rowA->setSpacing(24); rowA->addWidget(m_iconTypeA); rowA->addWidget(m_btnTypeA);
    QHBoxLayout* rowB = new QHBoxLayout(); rowB->setSpacing(24); rowB->addWidget(m_iconTypeB); rowB->addWidget(m_btnTypeB);
    QHBoxLayout* rowC = new QHBoxLayout(); rowC->setSpacing(24); rowC->addWidget(m_iconTypeC); rowC->addWidget(m_btnTypeC);
    QHBoxLayout* rowD = new QHBoxLayout(); rowD->setSpacing(24); rowD->addWidget(m_iconTypeD); rowD->addWidget(m_btnTypeD);
    QHBoxLayout* rowE = new QHBoxLayout(); rowE->setSpacing(24); rowE->addWidget(m_iconTypeE); rowE->addWidget(m_btnTypeE);
    controlLayout->addLayout(rowA); controlLayout->addLayout(rowB); controlLayout->addLayout(rowC); controlLayout->addLayout(rowD); controlLayout->addLayout(rowE);
    // 让藻类选择区域占据右侧三分之一高度
    controlGroup->setMinimumHeight(360); // 可根据实际右侧高度调整
    rightLayout->addWidget(controlGroup);
    rightLayout->addStretch(1);
    // 新增：合并所有藻类属性为一个按钮，点击弹窗显示
    QPushButton* btnAllAlgaeInfo = new QPushButton("查看所有藻类属性", this);
    btnAllAlgaeInfo->setStyleSheet("font-size:22px;font-weight:bold;color:#fff;background:#388e3c;border-radius:12px;padding:14px 28px;margin:18px 0 18px 0;");
    connect(btnAllAlgaeInfo, &QPushButton::clicked, this, [this]() {
        QString info =
            "<h2 style='color:#111;'>🌿 所有藻类属性</h2>"
            "<b style='color:#111;'>螺旋藻 Spirulina</b><br>"
            "光照需求: 22/18/12<br>种植消耗: 糖×10, 蛋白×5<br>遮光效果: 下方1格, -8<br>消耗: N×1/秒, C×8/秒<br>产出: 糖×5/秒, 蛋白×2/秒<br>特性: 同类相邻减产<br><br>"
            "<b style='color:#111;'>小球藻 Chlorella</b><br>"
            "光照需求: 18/14/10<br>种植消耗: 糖×8, 脂质×6, 维生素×2<br>遮光效果: 下方2格, 各-5<br>消耗: N×2/秒, C×6/秒<br>产出: 糖×3/秒, 脂质×4/秒, 维生素×1/秒<br>特性: 提升左右恢复速率<br><br>"
            "<b style='color:#111;'>小型硅藻 Cyclotella</b><br>"
            "光照需求: 12/8/6<br>种植消耗: 糖×16, 蛋白×8, 维生素×24<br>遮光效果: 无<br>消耗: N×3/秒, C×18/秒<br>产出: 糖×1.5/秒, 蛋白×1.5/秒, 维生素×2.5/秒<br>特性: 与小球藻连接时糖减产<br><br>"
            "<b style='color:#111;'>裸藻 Euglena</b><br>"
            "光照需求: 16/12/8<br>种植消耗: 糖×12, 脂质×8, 蛋白×6, 维生素×6<br>遮光效果: 下方1格, -4<br>消耗: N×1.5/秒, C×7/秒<br>产出: 糖×2.5/秒, 脂质×1.5/秒, 蛋白×1.5/秒, 维生素×1.5/秒<br>特性: 与螺旋藻/小球藻/小型硅藻相邻时协同增益，双方产量+20%<br><br>"
            "<b style='color:#111;'>蓝藻 Cyanobacteria</b><br>"
            "光照需求: 8/4/2<br>种植消耗: 糖×10, 脂质×6, 蛋白×4, 维生素×4<br>遮光效果: 无<br>消耗: N×1/秒, C×5/秒<br>产出: 糖×2/秒, 脂质×1/秒, 蛋白×1/秒, 维生素×1/秒<br>特性: 极低光照生存，为周围格子+4光照<br>";
        QMessageBox box(this);
        box.setWindowTitle("所有藻类属性");
        QLabel* label = new QLabel(&box);
        label->setTextFormat(Qt::RichText);
        label->setText(info);
        label->setWordWrap(true);
        label->setStyleSheet("font-size:16px;color:#222;background:#fff;border-radius:8px;padding:16px;");
        label->setMinimumWidth(760);
        QScrollArea* scroll = new QScrollArea(&box);
        scroll->setWidget(label);
        scroll->setWidgetResizable(true);
        scroll->setMinimumWidth(780);
        scroll->setMinimumHeight(400);
        box.layout()->addWidget(scroll);
        box.setMinimumWidth(800);
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    });
    rightLayout->addWidget(btnAllAlgaeInfo);
    rightLayout->addStretch(1);
    // 删除可视化说明栏（m_traitInfoLabel和m_traitInfoScrollArea）
    // 放大详细特性说明按钮
    m_btnTraitDetail = new QPushButton("详细特性说明", this);
    m_btnTraitDetail->setStyleSheet("font-size:22px;font-weight:bold;color:#fff;background:#1976d2;border-radius:12px;padding:14px 28px;margin-bottom:12px;");
    connect(m_btnTraitDetail, &QPushButton::clicked, this, &MainWindow::showTraitDetailDialog);
    rightLayout->addWidget(m_btnTraitDetail);

    // 三栏布局
    mainLayout->addWidget(leftPanel, 2);
    mainLayout->addWidget(centerPanel, 8); // 网格区域最大
    mainLayout->addWidget(rightPanel, 2);
}

// =================== CellWidget初始化与信号连接 ===================
void MainWindow::initializeCellWidgets() {
    m_cellWidgets.resize(m_game && m_game->getGrid() ? m_game->getGrid()->getRows() : 0); // 按行数分配
    for (int row = 0; m_game && m_game->getGrid() && row < m_game->getGrid()->getRows(); ++row) {
        m_cellWidgets[row].resize(m_game->getGrid()->getCols()); // 按列数分配
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            CellWidget* cellWidget = new CellWidget(row, col, this); // 创建格子控件
            m_cellWidgets[row][col] = cellWidget;
            AlgaeCell* algaeCell = m_game->getGrid()->getCell(row, col); // 获取对应数据单元格
            if (algaeCell) {
                cellWidget->setAlgaeCell(algaeCell); // 绑定数据
                m_cellsLayout->addWidget(cellWidget, row, col); // 加入布局
                connect(cellWidget, &CellWidget::leftClicked, this, &MainWindow::onCellClicked); // 左键点击
                connect(cellWidget, &CellWidget::rightClicked, this, &MainWindow::onCellRightClicked); // 右键点击
                connect(algaeCell, &AlgaeCell::cellChanged, this, [this, row, col]() {
                    updateCellDisplay(row, col); // 数据变化时刷新显示
                });
                connect(cellWidget, &CellWidget::hovered, this, &MainWindow::displayCellInfo); // 悬浮显示资源信息
            }
        }
    }
}

// 初始化网格（数据相关）
void MainWindow::setupGameGrid() {
    // 此处不再new m_cellsLayout和m_gridLayout，也不再添加布局，只做数据相关初始化（如有需要可保留数据相关代码）
    // m_cellsLayout和m_gridLayout的初始化和布局已在setupUI中完成
}

// 初始化控制按钮逻辑
void MainWindow::setupGameControls() {
    // 连接藻类选择按钮
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
    connect(m_btnTypeD, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_D);
        updateSelectedAlgaeButton();
        setCursor(m_cursorTypeD);
    });
    connect(m_btnTypeE, &QPushButton::clicked, [this]() {
        m_game->setSelectedAlgaeType(AlgaeType::TYPE_E);
        updateSelectedAlgaeButton();
        setCursor(m_cursorTypeE);
    });
    // 默认选中A型
    m_game->setSelectedAlgaeType(AlgaeType::TYPE_A);
    updateSelectedAlgaeButton();
    setCursor(m_cursorTypeA);
}

// 初始化资源与速率显示
void MainWindow::setupResourceDisplay() {
    onResourcesChanged();
    onProductionRatesChanged();

    // 定时刷新通关进度
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, this, &MainWindow::updateWinProgress);
    progressTimer->start(500); // 每0.5秒刷新
}

// 初始化菜单
void MainWindow::setupMenus() {
    // 创建主菜单
    m_gameMenu = menuBar()->addMenu(tr("游戏"));

    // 创建菜单动作
    m_restartAction = new QAction(tr("重新开始"), this);
    m_settingsAction = new QAction(tr("设置"), this);
    m_exitAction = new QAction(tr("退出"), this);

    // 添加动作到菜单
    m_gameMenu->addAction(m_restartAction);
    m_gameMenu->addAction(m_settingsAction);
    m_gameMenu->addSeparator();
    m_gameMenu->addAction(m_exitAction);

    // 连接菜单动作
    connect(m_restartAction, &QAction::triggered, this, &MainWindow::restartGame);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitGame);
}

// 连接信号槽
void MainWindow::connectSignals() {
    // 游戏状态信号
    connect(m_game, &AlgaeGame::gameStateChanged, this, &MainWindow::onGameStateChanged);
    connect(m_game, &AlgaeGame::selectedAlgaeChanged, this, &MainWindow::updateSelectedAlgaeButton);
    connect(m_game, &AlgaeGame::gameWon, this, &MainWindow::onGameWon);

    // 资源信号
    connect(m_game->getResources(), &GameResources::resourcesChanged, this, &MainWindow::onResourcesChanged);
    connect(m_game->getResources(),&GameResources::productionRatesChanged, this, &MainWindow::onProductionRatesChanged);
    // 网格刷新信号
    connect(m_game->getGrid(), &GameGrid::gridUpdated, this, &MainWindow::updateGridDisplay);
}

// 刷新选中藻类按钮样式
void MainWindow::updateSelectedAlgaeButton() {
    // 重置按钮样式
    m_btnTypeA->setStyleSheet("");
    m_btnTypeB->setStyleSheet("");
    m_btnTypeC->setStyleSheet("");
    m_btnTypeD->setStyleSheet("");
    m_btnTypeE->setStyleSheet("");

    // 设置选中按钮样式
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
    case AlgaeType::TYPE_D:
        m_btnTypeD->setStyleSheet(selectedStyle);
        break;
    case AlgaeType::TYPE_E:
        m_btnTypeE->setStyleSheet(selectedStyle);
        break;
    default:
        break;
    }
}

// 刷新整个网格显示
void MainWindow::updateGridDisplay() {
    if (!m_game || !m_game->getGrid()) return;
    for (int row = 0; row < m_game->getGrid()->getRows(); ++row) {
        for (int col = 0; col < m_game->getGrid()->getCols(); ++col) {
            updateCellDisplay(row, col); // 刷新每个格子
        }
    }
}

// 刷新单个格子显示
void MainWindow::updateCellDisplay(int row, int col) {
    if (!m_game || !m_game->getGrid()) return;
    if (row < m_cellWidgets.size() && col < m_cellWidgets[row].size()) {
        AlgaeCell* cell = m_game->getGrid()->getCell(row, col);
        if (cell) m_cellWidgets[row][col]->setAlgaeCell(cell);
    }
}

// 显示单元格信息到状态栏和气泡
void MainWindow::displayCellInfo(int row, int col) {
    if (!m_game || !m_game->getGrid()) return;
    GameGrid* grid = m_game->getGrid();
    AlgaeCell* cell = grid->getCell(row, col);
    if (cell) {
        double light = grid->getLightAt(row, col);
        AlgaeType::Type selType = m_game->getSelectedAlgaeType();
        QString lightReqText;
        if (selType != AlgaeType::NONE) {
            auto props = AlgaeType::getProperties(selType);
            lightReqText = QString("（种植≥%1，维持≥%2，存活≥%3）")
                .arg(props.lightRequiredPlant)
                .arg(props.lightRequiredMaintain)
                .arg(props.lightRequiredSurvive);
            // 减益说明
            if (light < props.lightRequiredSurvive) {
                lightReqText += " [濒死：产量极低/死亡]";
            } else if (light < props.lightRequiredMaintain) {
                lightReqText += " [光照低：产量减半]";
            } else if (light < props.lightRequiredPlant) {
                lightReqText += " [可种植但产量降低]";
            } else {
                lightReqText += " [产量正常]";
            }
        }
        QString info = QString("位置: (%1,%2)  氮素: %3  二氧化碳: %4  光照: %5 %6")
            .arg(row)
            .arg(col)
            .arg(QString::number(grid->getNitrogenAt(row, col), 'f', 1))
            .arg(QString::number(grid->getCarbonAt(row, col), 'f', 1))
            .arg(QString::number(light, 'f', 1))
            .arg(lightReqText);
        statusBar()->showMessage(info, 2000);
        QToolTip::showText(QCursor::pos(), info);
    }
}

// 播放音效（临时播放器）
void MainWindow::playSoundEffect(const QString& resource) {
    auto player = new QMediaPlayer(this);
    auto audio = new QAudioOutput(this);
    audio->setVolume(m_effectVolume);
    player->setAudioOutput(audio);
    player->setSource(QUrl(resource));
    player->play();
    connect(player, &QMediaPlayer::mediaStatusChanged, player, [player, audio](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia) {
            player->deleteLater();
            audio->deleteLater();
        }
    });
}

// 单元格左键点击事件
void MainWindow::onCellClicked(int row, int col) {
    bool success = m_game->plantAlgae(row, col);
    if (success) {
        statusBar()->showMessage(tr("放置藻类在 (%1,%2)").arg(row).arg(col), 2000);
        playSoundEffect("qrc:/resources/planted.mp3");
    } else {
        playSoundEffect("qrc:/resources/buzzer.wav");
    }
    updateCellDisplay(row, col);
}

// 单元格右键点击事件
void MainWindow::onCellRightClicked(int row, int col) {
    bool success = m_game->removeAlgae(row, col);
    if (success) {
        playSoundEffect("qrc:/resources/displant.wav");
        statusBar()->showMessage(tr("移除藻类，恢复周围资源"), 2000);
    } else {
        playSoundEffect("qrc:/resources/buzzer.wav");
    }
}

// 游戏状态变化槽
void MainWindow::onGameStateChanged() {
    // 根据游戏状态刷新UI
    if (m_game->isGameRunning()) {
        statusBar()->showMessage(tr("游戏进行中"), 2000);
    } else {
        statusBar()->showMessage(tr("游戏暂停"), 2000);
    }
}

// 资源变化槽
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
    // 移除全局刷新，提高性能
    // onProductionRatesChanged();
}

// 生产速率变化槽
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
    // 不再全局刷新网格
}

// 刷新通关进度条
void MainWindow::updateWinProgress() {
    double progress = m_game->getResources()->getWinProgress();
    m_progressBar->setValue(static_cast<int>(progress * 100));
    playBGM(progress);
    // 根据进度改变颜色
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

// 刷新分数栏
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
    // 检查是否完全达标
    bool isFullWin = m_game->getResources()->checkWinCondition();
    if (!isFullWin) {
        totalScore /= 2; // 未完全达标分数减半
    }
    if (totalScore > m_highScore) m_highScore = totalScore;
    m_scoreLabel->setText(QString("分数：%1   最高分：%2").arg(totalScore).arg(m_highScore));
    // 新增分数简介信息栏内容
    QString hint;
    if (!isFullWin) hint = "当前未完全达标，分数已减半！";
    else if (totalScore < 60) hint = "继续努力，优化藻类布局！";
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
        "资源得分 = (糖/500 + 脂/300 + 蛋白/200 + 维生素/100) / 4 × 50\n"
        "速率得分 = (糖速/50 + 脂速/30 + 蛋白速/20 + 维生素速/10) / 4 × 50\n"
        "总分 = 资源得分 + 速率得分（满100分后只看速率得分）"
    ).arg(QString::number(resourceScore, 'f', 1))
     .arg(QString::number(rateScore, 'f', 1))
     .arg(QString::number(totalScore));
    m_scoreDetailLabel->setText(detail);
}

// 播放背景音乐，根据进度切换曲目
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

// 播放音效（主播放器）
void MainWindow::playEffect(const QString& name) {
    QString path = "qrc:/resources/st30f0n665joahrrvuj05fechvwkcv10/" + name;
    m_effectAudio->setVolume(m_effectVolume);
    m_effectPlayer->setSource(QUrl(path));
    m_effectPlayer->play();
}

// 详细特性说明弹窗实现
void MainWindow::showTraitDetailDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("植株详细特性说明");
    dlg.setMinimumSize(520, 520);
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QLabel* label = new QLabel(&dlg);
    label->setWordWrap(true);
    label->setText("<h2 style='color:#1976d2;'>🌿 植株详细特性说明</h2><hr>"
        "<b style='color:#e53935;font-size:17px;'>🔴➖ 螺旋藻 Spirulina</b><br>"
        "<ul style='margin-top:0;'>"
        "<li>若与同类相邻（上下左右），所有产量减半，格子左上角出现红色圆底➖。</li>"
        "<li>遮光深度1格，遮光强度8，下方格子光照降低。</li>"
        "<li>产出：糖×5/秒，蛋白×2/秒。</li>"
        "<li>消耗：N×1/秒，C×8/秒。</li>"
        "<li>适合单独种植或与小球藻/小型硅藻错开布局，避免 螺旋藻连片。</li>"
        "</ul>"
        "<b style='color:#43a047;font-size:17px;'>🟢➕ 小球藻 Chlorella</b><br>"
        "<ul style='margin-top:0;'>"
        "<li>被左右小球藻邻居加速，恢复速率翻倍，格子右上角出现绿色圆底➕。</li>"
        "<li>遮光深度2格，遮光强度5，下方两格光照降低。</li>"
        "<li>产出：糖×3/秒，脂质×4/秒，维生素×1/秒。</li>"
        "<li>消耗：N×2/秒，C×6/秒。</li>"
        "<li>适合横向连片种植，提升整体资源恢复速度。</li>"
        "</ul>"
        "<b style='color:#fbc02d;font-size:17px;'>🟡❗小型硅藻 Cyclotella</b><br>"
        "<ul style='margin-top:0;'>"
        "<li>与小球藻相邻时，糖产量减半，格子右下角出现黄色圆底❗。</li>"
        "<li>无遮光，不影响下方光照。</li>"
        "<li>产出：糖×1.5/秒，蛋白×1.5/秒，维生素×2.5/秒。</li>"
        "<li>消耗：N×3/秒，C×18/秒。</li>"
        "<li>适合与螺旋藻、小球藻错开混合，避免与小球藻直接相邻。</li>"
        "</ul>"
        "<b style='color:#2196f3;font-size:17px;'>🔵★裸藻 Euglena</b><br>"
        "<ul style='margin-top:0;'>"
        "<li>与螺旋藻/小球藻/小型硅藻相邻时，自己和邻居产量+20%，自身左下角出现蓝色圆底★，邻居右下角出现蓝色圆底↑。</li>"
        "<li>遮光深度1格，遮光强度4，下方格子光照降低。</li>"
        "<li>产出：糖×2.5/秒，脂质×1.5/秒，蛋白×1.5/秒，维生素×1.5/秒。</li>"
        "<li>消耗：N×1.5/秒，C×7/秒。</li>"
        "<li>适合作为布局核心，提升整体产量，建议与螺旋藻/小球藻/小型硅藻交错布局。</li>"
        "</ul>"
        "<b style='color:#ffeb3b;font-size:17px;'>🟣🔘 蓝藻 Cyanobacteria</b><br>"
        "<ul style='margin-top:0;'>"
        "<li>极低光照生存，为周围格子+4光照。</li>"
        "<li>无遮光，不影响下方光照。</li>"
        "<li>产出：糖×2/秒，脂质×1/秒，蛋白×1/秒，维生素×1/秒。</li>"
        "<li>消耗：N×1/秒，C×5/秒。</li>"
        "<li>适合作为辅助，提升整体产量。</li>"
        "</ul>"
        "<hr>"
        "<b style='color:#1976d2;'>机制补充与策略建议：</b><br>"
        "<ul style='margin-top:0;'>"
        "<li><b>遮光机制：</b> 上方藻类会降低下方格子的光照，影响种植和产量。 螺旋藻遮1格，小球藻 遮2格，小型硅藻 无遮光，裸藻遮1格，蓝藻 无遮光。</li>"
        "<li><b>资源消耗与恢复：</b> 每种藻类消耗氮/碳不同，B型横向连片可提升左右格的资源恢复速度。</li>"
        "<li><b>特性高亮：</b> 当格子被特性影响时，会在对应角落显示彩色圆形+符号，便于直观识别。</li>"
        "<li><b>布局建议：</b> 合理错开螺旋藻/小球藻/小型硅藻/裸藻/蓝藻，利用小球藻加速、裸藻协同、蓝藻补光，避免螺旋藻连片和小型硅藻与小球藻直接相邻，可最大化资源产出。</li>"
        "<li><b>示例：</b> 螺旋藻-蓝藻-小球藻-裸藻-小型硅藻 这种横向布局，蓝藻可补光，裸藻可协同两侧螺旋藻/小球藻/小型硅藻，提升整体产量。</li>"
        "</ul>"
        "<hr>"
                   "<b style='color:#1976d2;font-size:17px;'>常见状态说明与应对措施</b><br>"
                   "<ul style='margin-top:0;'>"
                   "<li><b style='color:#00bcd4;'>光照无法种植</b>（提示“光照不足”）<br>"
                   "<b>产生原因：</b> 当前格子的光照低于所选藻类的“种植所需光照”阈值，通常是因为上方有遮光藻类或本行本身光照较低。<br>"
                   "<b>影响：</b> 无法在该格种植该类型藻类。<br>"
                   "<b>改变措施：</b> 可尝试移除上方遮光藻类、选择对光照要求更低的藻类，或在光照更高的格子种植。</li>"
                   "<li><b style='color:#e67e22;'>资源不足</b>（提示“资源不足”）<br>"
                   "<b>产生原因：</b> 当前全局资源（糖类、脂质、蛋白质、维生素）不足以支付所选藻类的种植消耗。<br>"
                   "<b>影响：</b> 无法种植该藻类。<br>"
                   "<b>改变措施：</b> 通过已有藻类持续生产资源，或移除部分消耗较高的藻类，积累资源后再种植。</li>"
                   "<li><b style='color:#d32f2f;'>濒危状态</b>（格子高亮/提示“濒死”）<br>"
                   "<b>产生原因：</b> 已种植藻类的格子，当前光照低于“存活所需最低光照”阈值，或长时间资源极低。<br>"
                   "<b>影响：</b> 藻类产量极低甚至为零，若持续濒危可能自动死亡被移除。<br>"
                   "<b>改变措施：</b> 移除上方遮光藻类、调整布局提升光照，或补充资源，避免藻类死亡。</li>"
                   "<li><b style='color:#ff9800;'>光照低</b>（格子高亮/提示“光照低”）<br>"
                   "<b>产生原因：</b> 已种植藻类的格子，当前光照低于“维持所需光照”但高于“存活所需最低光照”。<br>"
                   "<b>影响：</b> 藻类产量降低（通常为正常产量的一半），但不会立即死亡。<br>"
                   "<b>改变措施：</b> 优化上方遮光藻类布局，提升本格光照，或选择对光照要求更低的藻类。</li>"
                   "</ul>"
                   "<hr>"
                   "<b>其它说明：</b><br>"
                   "- 鼠标悬浮格子可查看当前资源、光照、可否种植等信息。<br>"
                   "- 详细数值和生产速率请参考右侧属性栏。<br>"
                   "- 所有特性均可在主界面格子上实时高亮显示。"
                   );
    label->setStyleSheet("font-size:15px;color:#222;background:rgba(255,255,255,0.95);border-radius:8px;padding:12px;");
    QScrollArea* scroll = new QScrollArea(&dlg);
    scroll->setWidget(label);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll);
    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(btnBox);
    dlg.exec();
}
