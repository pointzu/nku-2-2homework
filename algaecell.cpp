#include "algaecell.h" // 引入藻类单元格头文件
#include "gamegrid.h"  // 引入网格头文件
#include <QPainter>    // Qt绘图类
#include <QMouseEvent> // Qt鼠标事件
#include <QToolTip>    // Qt提示框
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>
#include <QFile>
#include <QSoundEffect>

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

    // 5. 格外下方的资源标注
    if (m_grid) {
        double n = m_grid->getNitrogenAt(m_row, m_col);
        double c = m_grid->getCarbonAt(m_row, m_col);
        double l = m_grid->getLightAt(m_row, m_col);
        QString resText = QString("N:%1  C:%2  L:%3").arg((int)n).arg((int)c).arg((int)l);
        QFont resFont("Arial", 8);
        QPainter* p = &painter;
        p->setFont(resFont);
        p->setPen(QColor(180,180,180));
        QRect outRect(rect().left(), rect().bottom()+2, rect().width(), 14);
        p->drawText(outRect, Qt::AlignCenter, resText);
    }

    // --- 藻类特性可视化 ---
    if (isOccupied()) {
        // 螺旋藻（Spirulina）相关
        // 小球藻（Chlorella）相关
        // 小型硅藻（Cyclotella）相关
        // 裸藻（Euglena）相关
        // 蓝藻（Cyanobacteria）相关
        // A型相邻减产：左上角红色圆底白色粗体"-"
        if (m_type == AlgaeType::TYPE_A && isReducedByNeighborA()) {
            painter.save();
            int r = 18;
            QRect markRect(rect().left()+2, rect().top()+2, r, r);
            painter.setBrush(QColor(220,0,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "-");
            painter.restore();
        }
        // B型被加速：右上角绿色圆底白色粗体"+"
        if (isBoostedByNeighborB()) {
            painter.save();
            int r = 18;
            QRect markRect(rect().right()-r-2, rect().top()+2, r, r);
            painter.setBrush(QColor(0,180,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "+");
            painter.restore();
        }
        // C型被B减产：右下角黄色圆底黑色粗体"!"
        if (m_type == AlgaeType::TYPE_C && isReducedByNeighborB()) {
            painter.save();
            int r = 18;
            QRect markRect(rect().right()-r-2, rect().bottom()-r-2, r, r);
            painter.setBrush(QColor(255,220,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::black);
            painter.drawText(markRect, Qt::AlignCenter, "!");
            painter.restore();
        }
        // E型自身右上角金色圆底"☀"
        if (m_type == AlgaeType::TYPE_E) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.right()-r-2, cellRect.top()+2, r, r);
            painter.setBrush(QColor(255,215,0));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(14); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "☀");
            painter.restore();
        }
        // 被E型加光的格子左下角蓝色圆底"+L"
        if (isLightedByE() && m_type != AlgaeType::TYPE_E) {
            painter.save();
            int r = 18;
            QRect markRect(cellRect.left()+2, cellRect.bottom()-r-2, r, r);
            painter.setBrush(QColor(0,180,255));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(markRect);
            QFont f = painter.font(); f.setPointSize(12); f.setBold(true); painter.setFont(f);
            painter.setPen(Qt::white);
            painter.drawText(markRect, Qt::AlignCenter, "+L");
            painter.restore();
        }
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

    // 基础产量
    m_carbProduction = props.produceRateCarb * m_productionMultiplier;
    m_lipidProduction = props.produceRateLipid * m_productionMultiplier;
    m_proProduction = props.produceRatePro * m_productionMultiplier;
    m_vitProduction = props.produceRateVit * m_productionMultiplier;

    // A型相邻减产：所有产量减半
    if (m_type == AlgaeType::TYPE_A && isReducedByNeighborA()) {
        m_carbProduction *= 0.5;
        m_lipidProduction *= 0.5;
        m_proProduction *= 0.5;
        m_vitProduction *= 0.5;
    }
    // C型被B减产：糖产量减半
    if (m_type == AlgaeType::TYPE_C && isReducedByNeighborB()) {
        m_carbProduction *= 0.5;
    }
    // D型协同：自身或被协同时产量提升20%
    if ((m_type == AlgaeType::TYPE_D && isSynergizingNeighbor()) || isSynergizedByNeighbor()) {
        m_carbProduction *= 1.2;
        m_lipidProduction *= 1.2;
        m_proProduction *= 1.2;
        m_vitProduction *= 1.2;
    }
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
