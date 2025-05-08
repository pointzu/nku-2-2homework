#include "gamegrid.h"
#include <QRandomGenerator>
#include <QApplication>

GameGrid::GameGrid(QWidget* parent)
    : QWidget(parent)
    , m_layout(new QGridLayout(this))
    , m_rows(10)  // 默认值
    , m_cols(8)   // 默认值
    , m_selectedAlgaeType(AlgaeType::NONE)
{
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(2, 2, 2, 2);
    setLayout(m_layout);
    initialize(m_rows, m_cols);
}

void GameGrid::initialize(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    createCells();
    initializeGrid();
    initializeResources();
}

void GameGrid::createCells()
{
    clearCells();
    m_cells.resize(m_rows);
    for (int row = 0; row < m_rows; ++row) {
        m_cells[row].resize(m_cols);
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = new AlgaeCell(row, col, this);
            m_cells[row][col] = cell;
            m_layout->addWidget(cell, row, col);
            connect(cell, &AlgaeCell::cellClicked, this, &GameGrid::onCellClicked);
            connect(cell, &AlgaeCell::cellHovered, this, &GameGrid::onCellHovered);
        }
    }
}

void GameGrid::clearCells()
{
    for (auto& row : m_cells) {
        for (auto cell : row) {
            if (cell) {
                delete cell;
            }
        }
    }
    m_cells.clear();
}

void GameGrid::setSelectedAlgaeType(AlgaeType::Type type)
{
    if (m_selectedAlgaeType != type) {
        m_selectedAlgaeType = type;
        updateCursor();
        updateShadingAreas();
    }
}

void GameGrid::updateCursor()
{
    if (m_selectedAlgaeType != AlgaeType::NONE) {
        AlgaeType::Properties props = AlgaeType::getProperties(m_selectedAlgaeType);
        QPixmap cursorPixmap(props.cursorImagePath);
        if (!cursorPixmap.isNull()) {
            m_algaeCursor = QCursor(cursorPixmap, -1, -1);
            setCursor(m_algaeCursor);
        }
    } else {
        setCursor(m_defaultCursor);
    }
}

void GameGrid::showShadingArea(int row, int col, bool show)
{
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        AlgaeCell* cell = m_cells[row][col];
        if (cell) {
            cell->showShadingArea(show);
        }
    }
}

void GameGrid::updateShadingAreas()
{
    // 清除所有遮荫区域
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            if (m_cells[row][col]) {
                m_cells[row][col]->showShadingArea(false);
            }
        }
    }

    // 如果有选中的藻类类型，显示遮荫区域
    if (m_selectedAlgaeType != AlgaeType::NONE) {
        AlgaeType::Properties props = AlgaeType::getProperties(m_selectedAlgaeType);
        for (int row = 0; row < m_rows; ++row) {
            for (int col = 0; col < m_cols; ++col) {
                if (m_cells[row][col] && m_cells[row][col]->isHovered()) {
                    // 显示遮荫区域
                    for (int r = qMax(0, row - props.shadingDepth); 
                         r <= qMin(m_rows - 1, row + props.shadingDepth); ++r) {
                        for (int c = qMax(0, col - props.shadingDepth);
                             c <= qMin(m_cols - 1, col + props.shadingDepth); ++c) {
                            if (m_cells[r][c]) {
                                m_cells[r][c]->showShadingArea(true);
                            }
                        }
                    }
                }
            }
        }
    }
}

void GameGrid::onCellClicked(int row, int col)
{
    emit cellClicked(row, col);
}

void GameGrid::onCellHovered(int row, int col, bool entered)
{
    emit cellHovered(row, col, entered);
    updateShadingAreas();
}

GameGrid::~GameGrid() {
    clearCells();
}

AlgaeCell* GameGrid::getCell(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_cells[row][col];
    }
    return nullptr;
}

double GameGrid::getLightAt(int row) const {
    if (row >= 0 && row < m_rows) {
        return m_baseLight[row] - calculateShadingAt(row, 0);
    }
    return 0.0;
}

double GameGrid::getNitrogenAt(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_nitrogen[row][col];
    }
    return 0.0;
}

double GameGrid::getCarbonAt(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_carbon[row][col];
    }
    return 0.0;
}

double GameGrid::getNitrogenRegenRate(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_nitrogenRegen[row][col];
    }
    return 0.0;
}

double GameGrid::getCarbonRegenRate(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_carbonRegen[row][col];
    }
    return 0.0;
}

void GameGrid::update(double deltaTime) {
    // 1. 先消耗局部资源
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];
            if (cell->isOccupied()) {
                AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
                double nNeed = props.consumeRateN * deltaTime;
                double cNeed = props.consumeRateC * deltaTime;
                // 资源不足时状态变差
                if (m_nitrogen[row][col] < nNeed || m_carbon[row][col] < cNeed) {
                    cell->setStatus(AlgaeCell::RESOURCE_LOW);
                } else {
                    cell->setStatus(AlgaeCell::NORMAL);
                    m_nitrogen[row][col] -= nNeed;
                    m_carbon[row][col] -= cNeed;
                }
            } else {
                cell->setStatus(AlgaeCell::NORMAL);
            }
        }
    }
    // 2. 遮光区域可视化（同前）
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->setShadingVisible(false);
        }
    }
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];
            if (cell->isOccupied()) {
                AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
                int depth = props.shadingDepth;
                for (int d = 1; d <= depth; ++d) {
                    int targetRow = row + d;
                    if (targetRow < m_rows) {
                        m_cells[targetRow][col]->setShadingVisible(true);
                    }
                }
            }
        }
    }
    // 3. 产出全局资源（只统计正常/资源低状态的产出）
    double totalCarb = 0, totalLipid = 0, totalPro = 0, totalVit = 0;
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];
            if (cell->isOccupied()) {
                if (cell->getStatus() == AlgaeCell::NORMAL || cell->getStatus() == AlgaeCell::RESOURCE_LOW) {
                    totalCarb += cell->getCarbProduction() * deltaTime;
                    totalLipid += cell->getLipidProduction() * deltaTime;
                    totalPro += cell->getProProduction() * deltaTime;
                    totalVit += cell->getVitProduction() * deltaTime;
                }
            }
        }
    }
    // 4. 更新全局资源
    emit resourcesChanged(); // 通知UI刷新
    // 5. 更新所有格子（状态刷新）
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->update(deltaTime);
        }
    }
    emit gridChanged();
}

void GameGrid::reset() {
    // Reset all cells
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->remove();
        }
    }

    // Reset resources
    initializeResources();

    emit gridChanged();
    emit resourcesChanged();
}

int GameGrid::calculateShadingAt(int row, int col) const {
    int totalShading = 0;

    // Check all cells above for shading effects
    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            AlgaeCell* cell = m_cells[r][c];

            if (cell->isOccupied()) {
                AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());

                // Calculate distance from shading cell to target cell
                int distanceRows = row - r;

                // Apply shading if within shading depth
                if (distanceRows <= props.shadingDepth) {
                    totalShading += props.shadingAmount;

                    // Special rule for Type A: extra shading
                    if (cell->getType() == AlgaeType::TYPE_A) {
                        totalShading += 3; // Extra shading from Type A
                    }
                }
            }
        }
    }

    return totalShading;
}

void GameGrid::applyRemoveBonus(int row, int col) {
    // Bonus resources when removing algae
    // Center cell gets more, surrounding cells get less

    // Center cell
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        m_nitrogen[row][col] += 10;
        m_carbon[row][col] += 15;
    }

    // Surrounding 8 cells
    for (int r = row-1; r <= row+1; r++) {
        for (int c = col-1; c <= col+1; c++) {
            if (r >= 0 && r < m_rows && c >= 0 && c < m_cols && (r != row || c != col)) {
                m_nitrogen[r][c] += 5;
                m_carbon[r][c] += 10;
            }
        }
    }

    emit resourcesChanged();
}

void GameGrid::initializeGrid() {
    m_cells.resize(m_rows);

    for (int row = 0; row < m_rows; ++row) {
        m_cells[row].resize(m_cols);

        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col] = new AlgaeCell(row, col, this);

            // Connect signals
            connect(m_cells[row][col], &AlgaeCell::cellChanged, this, [=]() {
                emit cellChanged(row, col);
            });
        }
    }
}

void GameGrid::initializeResources() {
    // Initialize light levels (decreasing from top to bottom, then constant)
    m_baseLight.resize(m_rows);
    m_baseLight[0] = 100;
    m_baseLight[1] = 76;
    m_baseLight[2] = 58;
    m_baseLight[3] = 44;
    m_baseLight[4] = 34;
    m_baseLight[5] = 26;
    m_baseLight[6] = 20;
    m_baseLight[7] = 15;
    m_baseLight[8] = 15;
    m_baseLight[9] = 15;

    // Initialize nitrogen and carbon with random values
    m_nitrogen.resize(m_rows);
    m_nitrogenRegen.resize(m_rows);
    m_carbon.resize(m_rows);
    m_carbonRegen.resize(m_rows);

    for (int row = 0; row < m_rows; ++row) {
        m_nitrogen[row].resize(m_cols);
        m_nitrogenRegen[row].resize(m_cols);
        m_carbon[row].resize(m_cols);
        m_carbonRegen[row].resize(m_cols);

        for (int col = 0; col < m_cols; ++col) {
            // Random initial values
            m_nitrogen[row][col] = QRandomGenerator::global()->bounded(10, 18); // 10-17
            m_carbon[row][col] = QRandomGenerator::global()->bounded(30, 46);   // 30-45

            // Random regeneration rates
            m_nitrogenRegen[row][col] = QRandomGenerator::global()->bounded(40, 61) / 10.0; // 4-6/s
            m_carbonRegen[row][col] = QRandomGenerator::global()->bounded(150, 301) / 10.0; // 15-30/s
        }
    }
}

void GameGrid::updateResources(double deltaTime) {
    // Update nitrogen and carbon based on regen rates and consumption
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];

            // Calculate consumption if cell is occupied
            double nitrogenConsumption = 0;
            double carbonConsumption = 0;

            if (cell->isOccupied()) {
                AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
                nitrogenConsumption = props.consumeRateN * deltaTime;
                carbonConsumption = props.consumeRateC * deltaTime;

                // Adjust for cell's status
                if (cell->getStatus() == AlgaeCell::RESOURCE_LOW ||
                    cell->getStatus() == AlgaeCell::LIGHT_LOW) {
                    nitrogenConsumption *= 0.5;
                    carbonConsumption *= 0.5;
                }
            }

            // Apply regeneration and consumption
            double nitrogenRegen = m_nitrogenRegen[row][col] * deltaTime;
            double carbonRegen = m_carbonRegen[row][col] * deltaTime;

            m_nitrogen[row][col] = qMin(30.0, m_nitrogen[row][col] + nitrogenRegen - nitrogenConsumption);
            m_carbon[row][col] = qMin(80.0, m_carbon[row][col] + carbonRegen - carbonConsumption);

            // Ensure values don't go below 0
            m_nitrogen[row][col] = qMax(0.0, m_nitrogen[row][col]);
            m_carbon[row][col] = qMax(0.0, m_carbon[row][col]);
        }
    }

    emit resourcesChanged();
}

void GameGrid::calculateSpecialEffects() {
    // Backup original regen rates
    QVector<QVector<double>> origNitrogenRegen = m_nitrogenRegen;
    QVector<QVector<double>> origCarbonRegen = m_carbonRegen;

    // Apply special effects
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];

            if (cell->isOccupied()) {
                // Special rule for Type B: enhance regeneration in adjacent cells
                if (cell->getType() == AlgaeType::TYPE_B) {
                    // Left cell
                    if (col > 0) {
                        m_nitrogenRegen[row][col-1] = origNitrogenRegen[row][col-1] * 2.0;
                        m_carbonRegen[row][col-1] = origCarbonRegen[row][col-1] * 2.0;
                    }

                    // Right cell
                    if (col < m_cols-1) {
                        m_nitrogenRegen[row][col+1] = origNitrogenRegen[row][col+1] * 2.0;
                        m_carbonRegen[row][col+1] = origCarbonRegen[row][col+1] * 2.0;
                    }
                }
            }
        }
    }
}
