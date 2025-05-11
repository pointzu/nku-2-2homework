#include "algaecell.h" // 引入藻类单元格头文件
#include "gamegrid.h"  // 引入网格头文件
#include "mainwindow.h" // 引入主窗口头文件
#include <QPainter>    // Qt绘图类
#include <QMouseEvent> // Qt鼠标事件
#include <QToolTip>    // Qt提示框
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>
#include <QFile>

// 藻类单元格构造函数
AlgaeCell::AlgaeCell(int row, int col, GameGrid* parent)
    : QWidget(parent) // 继承QWidget
    , m_row(row)      // 行号
    , m_col(col)      // 列号
    , m_grid(parent)  // 所属网格
    , m_type(AlgaeType::NONE) // 初始无藻类
    , m_status(NORMAL)        // 初始状态正常
    , m_productionMultiplier(1.0) // 生产倍率
    , m_timeSinceLightLow(0.0)    // 光照低计时
    , m_isHovered(false)          // 是否悬浮
    , m_isSelected(false)         // 是否选中
    , m_showShadingArea(false)    // 是否显示遮荫区
    , m_carbProduction(0.0)       // 糖产量
    , m_lipidProduction(0.0)      // 脂产量
    , m_proProduction(0.0)        // 蛋白产量
    , m_vitProduction(0.0)        // 维生素产量
    , m_player(nullptr)
    , m_audioOutput(nullptr)
{
    setMouseTracking(true); // 启用鼠标跟踪
    setMinimumSize(60, 60); // 最小尺寸
    m_properties = AlgaeType::getProperties(m_type); // 获取属性
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(1.0); // 1.0为最大音量，0.0为静音
}

// 析构函数
AlgaeCell::~AlgaeCell() {
}

// 种植结果枚举
enum PlantResult {
    PLANT_SUCCESS,           // 种植成功
    PLANT_OCCUPIED,          // 已被占用
    PLANT_LIGHT_LOW,         // 光照略低
    PLANT_LIGHT_INSUFFICIENT,// 光照不足
    PLANT_RESOURCE_LOW,      // 资源不足
    PLANT_RESERVED           // 资源预定
};

// 种植函数
AlgaeCell::PlantResult AlgaeCell::plant(AlgaeType::Type type, double lightLevel, bool canAfford, bool canReserve) {
    if (isOccupied()) {
        playSound("qrc:/../resources/buzzer.wav");
        return AlgaeCell::PLANT_OCCUPIED;
    }
    AlgaeType::Properties props = AlgaeType::getProperties(type); // 获取属性
    if (lightLevel < props.lightRequiredPlant) { // 光照不足
        if (lightLevel >= props.lightRequiredMaintain) { // 允许缓慢生长
            m_type = type;
            m_properties = props;
            m_status = LIGHT_LOW;
            m_productionMultiplier = 0.5;
            m_timeSinceLightLow = 0.0;
            emit cellChanged();
            return AlgaeCell::PLANT_LIGHT_LOW;
        } else { // 完全不能种植
            emit cellChanged();
            return AlgaeCell::PLANT_LIGHT_INSUFFICIENT;
        }
    }
    if (!canAfford) {
        if (canReserve) {
            m_type = type;
            m_properties = props;
            m_status = RESOURCE_LOW;
            m_productionMultiplier = 0.0;
            emit cellChanged();
            return AlgaeCell::PLANT_RESERVED;
        } else {
            return AlgaeCell::PLANT_RESOURCE_LOW;
        }
    }
    // 正常种植
    m_type = type;
    m_properties = props;
    m_status = NORMAL;
    m_productionMultiplier = 1.0;
    m_timeSinceLightLow = 0.0;
    emit cellChanged();
    return AlgaeCell::PLANT_SUCCESS;
}

void AlgaeCell::remove() {
    if (isOccupied()) {
        playSound("qrc:/../resources/displant.wav");
        m_type = AlgaeType::NONE;
        m_properties = AlgaeType::getProperties(m_type);
        m_status = NORMAL;
        m_productionMultiplier = 1.0;
        m_timeSinceLightLow = 0.0;
        if (m_grid) {
            m_grid->applyRemoveBonus(m_row, m_col);
        }
        emit cellChanged();
    }
}

void AlgaeCell::update(double deltaTime) {
    if (!isOccupied()) {
        return;
    }
    updateStatus();
}

void AlgaeCell::setHovered(bool hovered)
{
    if (m_isHovered != hovered) {
        m_isHovered = hovered;
        emit cellHovered(m_row, m_col, hovered);
        updateAppearance();
    }
}

void AlgaeCell::setSelected(bool selected)
{
    if (m_isSelected != selected) {
        m_isSelected = selected;
        updateAppearance();
    }
}

void AlgaeCell::showShadingArea(bool show)
{
    if (m_showShadingArea != show) {
        m_showShadingArea = show;
       QWidget:: update();
    }
}

void AlgaeCell::setShadingVisible(bool visible) {
    if (m_showShadingArea != visible) {
        m_showShadingArea = visible;
        QWidget::update();
    }
}

void AlgaeCell::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect cellRect = rect();
    // 1. 背景色随光照强度变化
    double light = m_grid ? m_grid->getLightAt(m_row, m_col) : 0.0;
    double maxLight = 30.0, minLight = 0.0;
    double ratio = (light - minLight) / (maxLight - minLight);
    ratio = qBound(0.0, ratio, 1.0);
    QColor bgColor = QColor::fromHsv(210, 30, 220 * ratio + 30 * (1 - ratio));
    painter.fillRect(cellRect, bgColor);

    // 2. 遮荫区颜色随遮光强度变化
    if (m_showShadingArea && m_type != AlgaeType::NONE) {
        int shade = m_grid ? m_grid->calculateShadingAt(m_row, m_col) : 0;
        int alpha = qBound(30, 30 + shade * 10, 180);
        QColor shadingColor = m_properties.shadingColor;
        shadingColor.setAlpha(alpha);
        painter.fillRect(cellRect, shadingColor);
    }

    // 3. 藻类图片
    if (m_type != AlgaeType::NONE) {
        QString imagePath = m_properties.imagePath;
        if (m_isSelected) {
            imagePath = m_properties.selectedImagePath;
        } else if (m_isHovered) {
            imagePath = m_properties.hoverImagePath;
        }
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            painter.drawPixmap(cellRect, pixmap);
        }
    }

    // 4. 光照不足高亮/闪烁边框
    if (m_status == LIGHT_LOW || m_status == DYING) {
        QColor statusColor = (m_status == DYING) ? QColor(255,0,128) : QColor(255,0,0);
        int penWidth = 4;
        painter.setPen(QPen(statusColor, penWidth));
        painter.drawRect(cellRect.adjusted(2,2,-2,-2));
    } else if (m_status == RESOURCE_LOW) {
        painter.setPen(QPen(QColor(255,165,0), 3));
        painter.drawRect(cellRect.adjusted(2,2,-2,-2));
    }

    // 5. 悬浮高亮和光照数值/可种植状态
    if (m_isHovered) {
        QColor hoverColor = QColor(255, 255, 0, 120);
        painter.setPen(QPen(hoverColor, 3, Qt::DashLine));
        painter.drawRect(cellRect.adjusted(4, 4, -4, -4));
        // 显示光照数值和可种植状态
        AlgaeType::Type selType = m_type;
        if (m_grid && !isOccupied()) {
            MainWindow* mw = nullptr;
            QWidget* w = m_grid->parentWidget();
            while (w && !mw) { mw = qobject_cast<MainWindow*>(w); w = w->parentWidget(); }
            if (mw) selType = mw->getGame()->getSelectedAlgaeType();
        }
        QString info;
        if (selType != AlgaeType::NONE) {
            AlgaeType::Properties props = AlgaeType::getProperties(selType);
            if (light < props.lightRequiredPlant) {
                info = "光照不足";
            } else {
                info = "可种植";
            }
            info += QString(" 光照:%1").arg((int)light);
        } else {
            info = QString("光照:%1").arg((int)light);
        }
        painter.setPen(Qt::black);
        painter.setFont(QFont("微软雅黑", 10, QFont::Bold));
        painter.drawText(cellRect.adjusted(0,0,0,-cellRect.height()/2), Qt::AlignCenter, info);
    }

    // 5. 网格右侧竖排高亮显示资源数值（氮、碳、光照）
    if (m_grid) {
        double n = m_grid->getNitrogenAt(m_row, m_col);
        double c = m_grid->getCarbonAt(m_row, m_col);
        double l = m_grid->getLightAt(m_row, m_col);
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        int margin = 4;
        int spacing = 18;
        int x = cellRect.right() - 48 + margin; // 靠右侧
        int y = cellRect.top() + margin;
        painter.setPen(QColor("#00e676"));
        painter.drawText(x, y + spacing * 0, QString("N:%1").arg(n, 0, 'f', 1));
        painter.setPen(QColor("#29b6f6"));
        painter.drawText(x, y + spacing * 1, QString("C:%1").arg(c, 0, 'f', 1));
        painter.setPen(QColor("#ffd600"));
        painter.drawText(x, y + spacing * 2, QString("L:%1").arg(l, 0, 'f', 1));
    }
}

void AlgaeCell::enterEvent(QEnterEvent* event)
{
    setHovered(true);
    QWidget::update();
    QWidget::enterEvent(event);
}

void AlgaeCell::leaveEvent(QEvent* event)
{
    setHovered(false);
    QWidget::update();
    QWidget::leaveEvent(event);
}

void AlgaeCell::mousePressEvent(QMouseEvent* event)
{
    if (!m_grid) return;
    if (event->button() == Qt::LeftButton) {
        emit cellClicked(m_row, m_col);
    }
    QWidget::mousePressEvent(event);
}

void AlgaeCell::updateAppearance()
{
    QWidget::update();
}

void AlgaeCell::updateProductionRates() {
    if (!isOccupied()) {
        m_carbProduction = 0.0;
        m_lipidProduction = 0.0;
        m_proProduction = 0.0;
        m_vitProduction = 0.0;
        return;
    }
    AlgaeType::Properties props = AlgaeType::getProperties(m_type);
    double light = m_grid ? m_grid->getLightAt(m_row, m_col) : 0.0;
    // 产量倍率与光照线性关联
    double ratio = (light - props.lightRequiredSurvive) / (props.lightRequiredPlant - props.lightRequiredSurvive);
    ratio = qBound(0.0, ratio, 1.0);
    m_productionMultiplier = ratio;
    m_carbProduction = props.produceRateCarb * m_productionMultiplier;
    m_lipidProduction = props.produceRateLipid * m_productionMultiplier;
    m_proProduction = props.produceRatePro * m_productionMultiplier;
    m_vitProduction = props.produceRateVit * m_productionMultiplier;
}

void AlgaeCell::setStatus(Status status) {
    if (m_status != status) {
        m_status = status;
        updateProductionRates();
        updateAppearance();
    }
}

void AlgaeCell::updateStatus() {
    if (!isOccupied()) {
        return;
    }
    double currentLight = m_grid->getLightAt(m_row, m_col);
    AlgaeType::Properties props = AlgaeType::getProperties(m_type);
    Status newStatus = NORMAL;
    if (currentLight < props.lightRequiredSurvive) {
        newStatus = DYING;
        m_timeSinceLightLow += 1.0;
    } else if (currentLight < props.lightRequiredMaintain) {
        newStatus = LIGHT_LOW;
        m_timeSinceLightLow = 0.0;
    } else if (currentLight < props.lightRequiredPlant) {
        newStatus = RESOURCE_LOW;
        m_timeSinceLightLow = 0.0;
    } else {
        m_timeSinceLightLow = 0.0;
    }
    if (m_status != newStatus) {
        m_status = newStatus;
        emit statusChanged(m_status);
        updateProductionRates();
    }
}

void AlgaeCell::checkSpecialRules() {
    if (!isOccupied()) {
        return;
    }

    // 检查特殊规则
    if (m_type == AlgaeType::TYPE_A && m_status == LIGHT_LOW) {
        m_timeSinceLightLow += 1.0;
        if (m_timeSinceLightLow >= 5.0) {  // 5秒后死亡
            remove();
            emit algaeDied();
        }
    }
}

void AlgaeCell::playSound(const QString& resourcePath) {
    m_player->stop();
    m_player->setSource(QUrl(resourcePath));
    m_audioOutput->setVolume(1.0);
    m_player->play();
    qDebug() << "播放音效:" << resourcePath << "状态:" << m_player->error() << m_player->errorString();
}
