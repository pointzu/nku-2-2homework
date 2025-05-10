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
        playSound("qrc:/sounds/buzzer.wav");
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
            playSound("qrc:/sounds/buzzer.wav");
            m_type = type;
            m_properties = props;
            m_status = RESOURCE_LOW;
            m_productionMultiplier = 0.0;
            emit cellChanged();
            return AlgaeCell::PLANT_RESERVED;
        } else {
            playSound("qrc:/sounds/buzzer.wav");
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
    playSound("qrc:/../resources/st30f0n665joahrrvuj05fechvwkcv10/planted.wav");
    return AlgaeCell::PLANT_SUCCESS;
}

void AlgaeCell::remove() {
    if (isOccupied()) {
        playSound("qrc:/sounds/displant.wav");
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
                statusColor = QColor(255, 165, 0);
                break;
            case LIGHT_LOW:
                statusColor = QColor(255, 0, 0);
                break;
            case DYING:
                statusColor = QColor(128, 0, 0);
                break;
            default:
                break;
        }
        painter.setPen(QPen(statusColor, 2));
        painter.drawRect(cellRect.adjusted(1, 1, -1, -1));
    }

    // 悬浮高亮
    if (m_isHovered) {
        QColor hoverColor = QColor(255, 255, 0, 120);
        painter.setPen(QPen(hoverColor, 5, Qt::SolidLine));
        painter.drawRect(cellRect.adjusted(2, 2, -2, -2));
        QColor fillColor = QColor(255, 255, 180, 80);
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
