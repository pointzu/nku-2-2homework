#include "gamegrid.h"
#include <QRandomGenerator>
#include <QPainter> // Add this line

GameGrid::GameGrid(QWidget* parent) : QWidget(parent) {
    initializeGrid();
    initializeResources();
}

GameGrid::~GameGrid() {
    // Clean up cells
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            delete m_cells[row][col];
        }
    }
}

AlgaeCell* GameGrid::getCell(int row, int col) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        return m_cells[row][col];
    }
    return nullptr;
}

double GameGrid::getLightAt(int row) const {
    if (row >= 0 && row < ROWS) {
        return m_baseLight[row] - calculateShadingAt(row, 0);
    }
    return 0.0;
}

double GameGrid::getNitrogenAt(int row, int col) const {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        return m_nitrogen[row][col];
    }
    return 0.0;
}

double GameGrid::getCarbonAt(int row, int col) const {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        return m_carbon[row][col];
    }
    return 0.0;
}

double GameGrid::getNitrogenRegenRate(int row, int col) const {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        return m_nitrogenRegen[row][col];
    }
    return 0.0;
}

double GameGrid::getCarbonRegenRate(int row, int col) const {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        return m_carbonRegen[row][col];
    }
    return 0.0;
}

void GameGrid::update(double deltaTime) {
    // Update resources
    updateResources(deltaTime);

    // Calculate special effects (B type enhances neighboring cells, etc.)
    calculateSpecialEffects();

    // Update all cells
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            m_cells[row][col]->update(deltaTime);
        }
    }

    emit gridChanged();
}

void GameGrid::reset() {
    // Reset all cells
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
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

    // 仅检查当前列的上方行（同一列的藻类遮荫影响更直接）
    for (int r = 0; r < row; ++r) {
        AlgaeCell* cell = m_cells[r][col];  // 仅当前列

        if (cell->isOccupied()) {
            AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
            int distanceRows = row - r;

            if (distanceRows <= props.shadingDepth) {
                totalShading += props.shadingAmount;
                if (cell->getType() == AlgaeType::TYPE_A) {  // 类型A额外遮荫
                    totalShading += 3;
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
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        m_nitrogen[row][col] += 10;
        m_carbon[row][col] += 15;
    }

    // Surrounding 8 cells
    for (int r = row-1; r <= row+1; r++) {
        for (int c = col-1; c <= col+1; c++) {
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS && (r != row || c != col)) {
                m_nitrogen[r][c] += 5;
                m_carbon[r][c] += 10;
            }
        }
    }

    emit resourcesChanged();
}

void GameGrid::initializeGrid() {
    m_cells.resize(ROWS);

    for (int row = 0; row < ROWS; ++row) {
        m_cells[row].resize(COLS);

        for (int col = 0; col < COLS; ++col) {
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
    m_baseLight.resize(ROWS);
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
    m_nitrogen.resize(ROWS);
    m_nitrogenRegen.resize(ROWS);
    m_carbon.resize(ROWS);
    m_carbonRegen.resize(ROWS);

    for (int row = 0; row < ROWS; ++row) {
        m_nitrogen[row].resize(COLS);
        m_nitrogenRegen[row].resize(COLS);
        m_carbon[row].resize(COLS);
        m_carbonRegen[row].resize(COLS);

        for (int col = 0; col < COLS; ++col) {
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
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
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
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
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
                    if (col < COLS-1) {
                        m_nitrogenRegen[row][col+1] = origNitrogenRegen[row][col+1] * 2.0;
                        m_carbonRegen[row][col+1] = origCarbonRegen[row][col+1] * 2.0;
                    }
                }
            }
        }
    }
}

void GameGrid::mouseMoveEvent(QMouseEvent* event) {
    // 计算悬停的行列（修正 Qt 6 弃用 API）
    int cellWidth = width() / COLS;
    int cellHeight = height() / ROWS;
    // 使用 position() 获取坐标并转换为整数
    m_hoverCol = static_cast<int>(event->position().x()) / cellWidth;
    m_hoverRow = static_cast<int>(event->position().y()) / cellHeight;
     QWidget::update(); // 触发 QWidget 的重绘方法（无参数）
    QWidget::mouseMoveEvent(event);
}

void GameGrid::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    // 绘制网格（示例逻辑）
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            AlgaeCell* cell = getCell(row, col);
            if (cell) {
                cell->paint(&painter, CELL_SIZE); // 使用修改后的 paint 函数
            }
        }
    }

    // 绘制遮光区域（半透明灰色覆盖）
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int shading = calculateShadingAt(row, col);
            if (shading > 0) {
                int alpha = qMin(shading * 8, 200);
                painter.setBrush(QColor(0, 0, 0, alpha));
                painter.drawRect(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            }
        }
    }

    // 绘制悬浮高亮（半透明绿色矩形）
    if (m_hoverRow != -1 && m_hoverCol != -1) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 255, 0, 80));
        painter.drawRect(m_hoverCol * CELL_SIZE, m_hoverRow * CELL_SIZE, CELL_SIZE, CELL_SIZE);
    }

    QWidget::paintEvent(event);
}
