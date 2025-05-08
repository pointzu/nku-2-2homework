#include "algaecell.h"
#include <QPainter>
#include "gamegrid.h"

AlgaeCell::AlgaeCell(int row, int col, GameGrid* parent)
    : QObject(parent), m_row(row), m_col(col), m_grid(parent),
    m_type(AlgaeType::NONE), m_status(NORMAL),
    m_productionMultiplier(1.0), m_timeSinceLightLow(0.0) {

    m_properties = AlgaeType::getProperties(m_type);
}

AlgaeCell::~AlgaeCell() {
}

bool AlgaeCell::plant(AlgaeType::Type type) {
    // Already occupied
    if (isOccupied()) {
        return false;
    }

    // Get light level for this position
    double lightLevel = m_grid->getLightAt(m_row);

    // Get properties for the algae type
    AlgaeType::Properties props = AlgaeType::getProperties(type);

    // Check if there's enough light
    if (lightLevel < props.lightRequiredPlant) {
        return false;
    }

    // Set type and properties
    m_type = type;
    m_properties = props;
    m_status = NORMAL;
    m_productionMultiplier = 1.0;
    m_timeSinceLightLow = 0.0;

    emit cellChanged();

    return true;
}

void AlgaeCell::remove() {
    if (m_type != AlgaeType::NONE) {
        m_type = AlgaeType::NONE;
        m_properties = AlgaeType::getProperties(m_type);
        m_status = NORMAL;
        m_productionMultiplier = 1.0;
        m_timeSinceLightLow = 0.0;

        // Apply removal bonus to surrounding cells
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

    // 更新生长阶段（最大1.0，随时间线性增长）
    m_growthStage = qMin(m_growthStage + deltaTime * 0.1, 1.0);  // 0.1为生长速率系数
    
    updateStatus(deltaTime);
    // checkSpecialRules();  // 假设存在特殊规则检查（如B型藻类增强邻居）
}

// Apply special rules based on

// In algaecell.h, update the declaration to match:
// void paint(QPainter* painter, int cellSize) const;

// In algaecell.cpp, correct the implementation:
void AlgaeCell::paint(QPainter* painter, int cellSize) const {
    if (isOccupied()) {
        painter->setBrush(AlgaeType::getColor(m_type));
        painter->drawRect(m_col * cellSize, m_row * cellSize, cellSize, cellSize); // Fixed coordinates (row/col)
    }
}


// 修复糖类产出计算
double AlgaeCell::getCarbProduction() const {
    if (m_status == AlgaeCell::DYING) return 0.0;
    return m_properties.produceRateCarb * m_growthStage;  // 修正：使用 produceRateCarb
}

// 修复脂质产出计算
double AlgaeCell::getLipidProduction() const {
    if (m_status == AlgaeCell::DYING) return 0.0;
    return m_properties.produceRateLipid * m_growthStage;
}

// 修复蛋白质产出计算
double AlgaeCell::getProProduction() const {
    if (m_status == AlgaeCell::DYING) return 0.0;
    return m_properties.produceRatePro * m_growthStage;
}

// 修复维生素产出计算
double AlgaeCell::getVitProduction() const {
    if (m_status == AlgaeCell::DYING) return 0.0;
    return m_properties.produceRateVit * m_growthStage;
}

// 修复状态更新方法（参数匹配）
void AlgaeCell::updateStatus(double deltaTime) {
    double currentLight = m_grid->getLightAt(m_row);
    double currentNitrogen = m_grid->getNitrogenAt(m_row, m_col);  // 获取当前氮含量
    double currentCarbon = m_grid->getCarbonAt(m_row, m_col);     // 获取当前碳含量

    // 检查光照不足（需在AlgaeType::Properties中添加minLight）
    if (currentLight < m_properties.minLight) {
        m_status = LIGHT_LOW;
        m_timeSinceLightLow += deltaTime;
        if (m_timeSinceLightLow > 5.0) {  // 5秒光照不足进入死亡状态
            m_status = DYING;
        }
    }
    // 检查资源不足（需在AlgaeType::Properties中添加minNitrogen/minCarbon）
    else if (currentNitrogen < m_properties.minNitrogen || currentCarbon < m_properties.minCarbon) {
        m_status = RESOURCE_LOW;
    }
    // 正常状态
    else {
        m_status = NORMAL;
        m_timeSinceLightLow = 0.0;  // 重置光照不足计时
    }
}
