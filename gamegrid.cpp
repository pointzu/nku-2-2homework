#include "gamegrid.h" // 游戏网格头文件
#include <QRandomGenerator> // Qt随机数生成器
#include <QApplication>     // Qt应用程序类
#include"mainwindow.h"    // 主窗口头文件
#include <QPainter>
GameGrid::GameGrid(QWidget* parent)
    : QWidget(parent)
    , m_layout(new QGridLayout(this)) // 创建网格布局
    , m_rows(10)  // 默认行数
    , m_cols(8)   // 默认列数
    , m_selectedAlgaeType(AlgaeType::NONE) // 初始无选中藻类
    , m_produceTimer(0.0) // 产出计时器归零
{
    m_layout->setSpacing(2); // 设置格子间距
    m_layout->setContentsMargins(2, 2, 2, 2); // 设置边距
    setLayout(m_layout); // 应用布局
    initialize(m_rows, m_cols); // 初始化网格
}

void GameGrid::initialize(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    createCells();        // 创建所有单元格
    initializeGrid();     // 初始化网格结构
    initializeResources();// 初始化资源
}

void GameGrid::createCells()
{
    clearCells(); // 先清空原有单元格
    m_cells.resize(m_rows); // 调整行数
    for (int row = 0; row < m_rows; ++row) {
        m_cells[row].resize(m_cols); // 调整列数
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = new AlgaeCell(row, col, this); // 创建单元格
            m_cells[row][col] = cell;
            m_layout->addWidget(cell, row, col); // 加入布局
            if (cell) {
                connect(cell, &AlgaeCell::cellClicked, this, &GameGrid::onCellClicked); // 连接点击信号
                connect(cell, &AlgaeCell::cellHovered, this, &GameGrid::onCellHovered); // 连接悬浮信号
            }
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
        updateCursor();      // 更新鼠标指针
        updateShadingAreas();// 更新遮荫区
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
        AlgaeCell* cell = getCell(row, col);
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

    // 只有主窗口允许预览时才显示遮荫区
    MainWindow* mw = nullptr;
    QWidget* w = parentWidget();
    while (w && !mw) { mw = qobject_cast<MainWindow*>(w); w = w->parentWidget(); }
    if (!mw || !mw->isShadingPreviewEnabled()) return;

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
                            AlgaeCell* cell = getCell(r, c);
                            if (cell) {
                                cell->showShadingArea(true);
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
    if (getCell(row, col))
        emit cellClicked(row, col); // 发射信号
}

void GameGrid::onCellHovered(int row, int col, bool entered)
{
    if (getCell(row, col))
        emit cellHovered(row, col, entered); // 发射信号
    updateShadingAreas(); // 悬浮时刷新遮荫区
}

GameGrid::~GameGrid() {
    clearCells();
}

AlgaeCell* GameGrid::getCell(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        if (m_cells[row][col])
            return m_cells[row][col];
    }
    return nullptr;
}

double GameGrid::getLightAt(int row, int col) const {
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        return m_baseLight[row] - calculateShadingAt(row, col); // 基础光照减遮光
    }
    return 0.0;
}

double GameGrid::getLightAt(int row) const {
    return getLightAt(row, 0);
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

// 网格整体更新，每帧调用
void GameGrid::update(double deltaTime) {
    // 1. 先消耗局部资源
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            AlgaeCell* cell = m_cells[row][col];
            if (cell->isOccupied()) {
                AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
                double nNeed = props.consumeRateN * deltaTime; // 需要消耗的氮
                double cNeed = props.consumeRateC * deltaTime; // 需要消耗的碳
                if (m_nitrogen[row][col] < nNeed || m_carbon[row][col] < cNeed) {
                    cell->setStatus(AlgaeCell::RESOURCE_LOW); // 资源不足
                } else {
                    cell->setStatus(AlgaeCell::NORMAL); // 正常
                    m_nitrogen[row][col] -= nNeed;
                    m_carbon[row][col] -= cNeed;
                }
            } else {
                cell->setStatus(AlgaeCell::NORMAL); // 空格子状态正常
            }
        }
    }
    // 2. 遮光区域可视化（同前）
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->setShadingVisible(false); // 先全部隐藏
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
                        m_cells[targetRow][col]->setShadingVisible(true); // 下方格子显示遮荫
                    }
                }
            }
        }
    }
    // 3. 产出资源逻辑：每10秒产出一次
    m_produceTimer += deltaTime;
    if (m_produceTimer >= 10.0) {
        double totalCarb = 0, totalLipid = 0, totalPro = 0, totalVit = 0;
        for (int row = 0; row < m_rows; ++row) {
            for (int col = 0; col < m_cols; ++col) {
                AlgaeCell* cell = m_cells[row][col];
                if (cell->isOccupied()) {
                    if (cell->getStatus() == AlgaeCell::NORMAL || cell->getStatus() == AlgaeCell::RESOURCE_LOW) {
                        totalCarb += cell->getCarbProduction() * 10.0; // 10秒产量
                        totalLipid += cell->getLipidProduction() * 10.0;
                        totalPro += cell->getProProduction() * 10.0;
                        totalVit += cell->getVitProduction() * 10.0;
                    }
                }
            }
        }
        // 通过信号或直接调用GameResources接口加资源（需适配你的架构）
        emit produceResources(totalCarb, totalLipid, totalPro, totalVit); // 发射产出信号
        m_produceTimer = 0.0; // 计时器归零
    }
    // 4. 更新全局资源
    emit resourcesChanged(); // 通知UI刷新
    // 5. 更新所有格子（状态刷新）
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->update(deltaTime); // 单元格自身刷新
        }
    }
    emit gridChanged(); // 通知网格变化
}

// 重置网格和资源
void GameGrid::reset() {
    // Reset all cells
    for (int row = 0; row < m_rows; ++row) {
        for (int col = 0; col < m_cols; ++col) {
            m_cells[row][col]->remove(); // 移除所有藻类
        }
    }

    // Reset resources
    initializeResources(); // 重新初始化资源

    emit gridChanged();
    emit resourcesChanged();
}

// 计算某格的遮光总量
int GameGrid::calculateShadingAt(int row, int col) const {
    int totalShading = 0;
    // 只考虑本列上方的遮荫
    for (int r = 0; r < row; ++r) {
        AlgaeCell* cell = m_cells[r][col];
        if (cell->isOccupied()) {
            AlgaeType::Properties props = AlgaeType::getProperties(cell->getType());
            int distanceRows = row - r;
            if (distanceRows <= props.shadingDepth) {
                totalShading += props.shadingAmount;
                if (cell->getType() == AlgaeType::TYPE_A) {
                    totalShading += 3; // A型藻类额外遮光
                }
            }
        }
    }
    return totalShading;
}

// 移除藻类时奖励资源
void GameGrid::applyRemoveBonus(int row, int col) {
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

// 初始化网格结构（重新分配单元格对象）
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

// 初始化资源，包括光照、氮、碳及其恢复速率
void GameGrid::initializeResources() {
    // 更陡峭的光照梯度（顶部到下方）
    m_baseLight.resize(m_rows);
    m_baseLight[0] = 30;
    m_baseLight[1] = 28;
    m_baseLight[2] = 26;
    m_baseLight[3] = 24;
    m_baseLight[4] = 22;
    m_baseLight[5] = 20;
    m_baseLight[6] = 18;
    m_baseLight[7] = 16;
    m_baseLight[8] = 14;
    m_baseLight[9] = 12;

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

// 更新资源（氮、碳）随时间变化
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

// 计算特殊效果（如B型藻类提升左右格恢复速率）
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

// 预判种植后某格的光照强度
double GameGrid::getLightAtIfPlanted(int row, int col, AlgaeType::Type type) const {
    int simulatedShading = calculateShadingAt(row, col); // 现有遮光
    if (type != AlgaeType::NONE) {
        AlgaeType::Properties props = AlgaeType::getProperties(type);
        for (int d = 1; d <= props.shadingDepth; ++d) {
            int targetRow = row + d;
            if (targetRow < m_rows) {
                simulatedShading += props.shadingAmount; // 预加新藻类遮光
            }
        }
    }
    return m_baseLight[row] - simulatedShading; // 返回预判光照
}

void GameGrid::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    QRect rect = this->rect();

    // N、C行
    QRect outRect1(rect.left(), rect.bottom() - 38, rect.width(), 12);
    QFont fontNC("Arial", 8, QFont::Bold);
    painter.setFont(fontNC);
    QString ncText = QString("<span style='color:#fff;'>N:%1&nbsp;&nbsp;C:%2</span>")
                        .arg((int)m_nitrogen[0][0]).arg((int)m_carbon[0][0]);
    QTextDocument docNC;
    docNC.setHtml(QString("<div align='center' style='line-height:12px;margin:0;padding:0;'>%1</div>").arg(ncText));
    painter.save();
    painter.translate(outRect1.left(), outRect1.top());
    docNC.setTextWidth(outRect1.width());
    docNC.drawContents(&painter, QRectF(0, 0, outRect1.width(), outRect1.height()));
    painter.restore();

    // L行
    QRect outRect2(rect.left(), rect.bottom() - 24, rect.width(), 16);
    QFont fontL("Arial", 11, QFont::Bold);
    painter.setFont(fontL);
    QString lText = QString("<span style='color:#00ff66;'>L:%1</span>").arg((int)getLightAt(0)); // 绿色高亮
    QTextDocument docL;
    docL.setHtml(QString("<div align='center' style='line-height:15px;margin:0;padding:0;'>%1</div>").arg(lText));
    painter.save();
    painter.translate(outRect2.left(), outRect2.top());
    docL.setTextWidth(outRect2.width());
    docL.drawContents(&painter, QRectF(0, 0, outRect2.width(), outRect2.height()));
    painter.restore();
}
