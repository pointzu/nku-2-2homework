#include "algaecell.h"
#include "gamegrid.h"
#include <QPainter>
#include <QMouseEvent>

AlgaeCell::AlgaeCell(int row, int col, GameGrid* parent)
    : QWidget(parent)
    , m_row(row)
    , m_col(col)
    , m_grid(parent)
    , m_type(AlgaeType::NONE)
    , m_status(NORMAL)
    , m_productionMultiplier(1.0)
    , m_timeSinceLightLow(0.0)
    , m_isHovered(false)
    , m_isSelected(false)
    , m_showShadingArea(false)
    , m_carbProduction(0.0)
    , m_lipidProduction(0.0)
    , m_proProduction(0.0)
    , m_vitProduction(0.0)
{
    setMouseTracking(true);
    setMinimumSize(60, 60);
    m_properties = AlgaeType::getProperties(m_type);
}

AlgaeCell::~AlgaeCell() {
}

bool AlgaeCell::plant(AlgaeType::Type type) {
    if (isOccupied()) {
        return false;
    }
    double lightLevel = m_grid->getLightAt(m_row);
    AlgaeType::Properties props = AlgaeType::getProperties(type);
    if (lightLevel < props.lightRequiredPlant) {
        return false;
    }
    m_type = type;
    m_properties = props;
    m_status = NORMAL;
    m_productionMultiplier = 1.0;
    m_timeSinceLightLow = 0.0;
    emit cellChanged();
    return true;
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

    // 绘制单元格背景
    QRect cellRect = rect();
    painter.fillRect(cellRect, QColor(240, 240, 240));

    // 绘制遮荫区域
    if (m_showShadingArea && m_type != AlgaeType::NONE) {
        QColor shadingColor = m_properties.shadingColor;
        shadingColor.setAlpha(100);
        painter.fillRect(cellRect, shadingColor);
    }

    // 绘制藻类
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

    // 绘制状态指示器
    if (m_status != NORMAL) {
        QColor statusColor;
        switch (m_status) {
            case RESOURCE_LOW:
                statusColor = QColor(255, 165, 0); // 橙色
                break;
            case LIGHT_LOW:
                statusColor = QColor(255, 0, 0);   // 红色
                break;
            case DYING:
                statusColor = QColor(128, 0, 0);   // 深红色
                break;
            default:
                break;
        }
        painter.setPen(QPen(statusColor, 2));
        painter.drawRect(cellRect.adjusted(1, 1, -1, -1));
    }

    // 悬浮高亮
    if (m_isHovered) {
        QColor hoverColor = QColor(255, 255, 0, 120); // 更亮的黄色
        painter.setPen(QPen(hoverColor, 5, Qt::SolidLine));
        painter.drawRect(cellRect.adjusted(2, 2, -2, -2));
        QColor fillColor = QColor(255, 255, 180, 80); // 明显的半透明色块
        painter.fillRect(cellRect.adjusted(6, 6, -6, -6), fillColor);
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
    double statusMultiplier = 1.0;
    switch (m_status) {
        case LIGHT_LOW:
            statusMultiplier = 0.5;
            break;
        case RESOURCE_LOW:
            statusMultiplier = 0.7;
            break;
        case DYING:
            statusMultiplier = 0.0;
            break;
        default:
            statusMultiplier = 1.0;
            break;
    }
    m_carbProduction = props.produceRateCarb * statusMultiplier;
    m_lipidProduction = props.produceRateLipid * statusMultiplier;
    m_proProduction = props.produceRatePro * statusMultiplier;
    m_vitProduction = props.produceRateVit * statusMultiplier;
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
    double currentLight = m_grid->getLightAt(m_row);
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
