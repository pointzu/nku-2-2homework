#include "algaegame.h"      // 游戏主逻辑头文件
#include "mainwindow.h"     // 主窗口头文件
#include <QDateTime>         // Qt时间类

// AlgaeGame构造函数，初始化成员变量和游戏网格、资源
AlgaeGame::AlgaeGame(QWidget* parent)
    : QObject(parent)
    , m_grid(new GameGrid(parent)) // 创建游戏网格
    , m_resources(new GameResources(this)) // 创建资源管理器
    , m_selectedAlgaeType(AlgaeType::NONE) // 初始无选中藻类
    , m_isGameRunning(false) // 游戏初始为暂停
    , m_updateTimer(new QTimer(this)) // 创建定时器
    , m_lastUpdateTime(QDateTime::currentMSecsSinceEpoch()) // 记录当前时间
{
    // 初始化游戏网格，10行8列
    m_grid->initialize(10, 8);  // 10行8列的网格
    
    // 连接网格信号到游戏槽函数
    connect(m_grid, &GameGrid::gridChanged, this, &AlgaeGame::onGridChanged);
    connect(m_grid, &GameGrid::resourcesChanged, this, &AlgaeGame::onResourcesChanged);
    connect(m_grid, &GameGrid::cellClicked, this, [this](int row, int col) {
        if (m_selectedAlgaeType != AlgaeType::NONE) {
            plantAlgae(row, col); // 有选中藻类则种植
        } else {
            removeAlgae(row, col); // 否则移除
        }
    });
    
    // 新增：监听所有AlgaeCell的cellChanged信号，自动刷新速率
    for (int row = 0; row < m_grid->getRows(); ++row) {
        for (int col = 0; col < m_grid->getCols(); ++col) {
            AlgaeCell* cell = m_grid->getCell(row, col);
            if (cell) {
                connect(cell, &AlgaeCell::cellChanged, this, &AlgaeGame::onGridChanged);
            }
        }
    }
    
    // 设置定时器，200ms刷新一次
    m_updateTimer->setInterval(200);  // 200ms 刷新一次
    connect(m_updateTimer, &QTimer::timeout, this, &AlgaeGame::update);
}

// 析构函数，停止定时器
AlgaeGame::~AlgaeGame() {
    m_updateTimer->stop();
}

// 设置当前选中的藻类类型
void AlgaeGame::setSelectedAlgaeType(AlgaeType::Type type) {
    if (m_selectedAlgaeType != type) {
        m_selectedAlgaeType = type;
        emit selectedAlgaeChanged(); // 通知UI
    }
}

// 开始游戏，启动定时器
void AlgaeGame::startGame() {
    if (!m_isGameRunning) {
        m_isGameRunning = true;
        m_updateTimer->start();
        emit gameStateChanged(); // 通知UI
    }
}

// 暂停游戏，停止定时器
void AlgaeGame::pauseGame() {
    if (m_isGameRunning) {
        m_isGameRunning = false;
        m_updateTimer->stop();
        emit gameStateChanged(); // 通知UI
    }
}

// 重置游戏，重置网格和资源，恢复初始状态
void AlgaeGame::resetGame() {
    pauseGame();

    // Reset grid and resources
    m_grid->reset();
    m_resources->reset();

    // Reset selected algae type
    m_selectedAlgaeType = AlgaeType::TYPE_A;

    emit selectedAlgaeChanged();
    emit gameStateChanged();
}

// 在指定位置种植藻类
bool AlgaeGame::plantAlgae(int row, int col) {
    if (!m_isGameRunning || m_selectedAlgaeType == AlgaeType::NONE) {
        return false;
    }
    double carb = m_resources->getCarbohydrates();
    double lipid = m_resources->getLipids();
    double pro = m_resources->getProteins();
    double vit = m_resources->getVitamins();
    double light = m_grid->getLightAt(row);
    bool canAfford = AlgaeType::canAfford(m_selectedAlgaeType, carb, lipid, pro, vit);
    if (!canAfford) {
        // 资源不足，不能种植
        return false;
    }
    bool canReserve = true;
    AlgaeCell* cell = m_grid->getCell(row, col);
    if (cell) {
        AlgaeCell::PlantResult result = cell->plant(m_selectedAlgaeType, light, canAfford, canReserve);
        m_lastPlantResult = result;
        if (result == AlgaeCell::PLANT_SUCCESS) {
            AlgaeType::Properties props = AlgaeType::getProperties(m_selectedAlgaeType);
            m_resources->subtractCarbohydrates(props.plantCostCarb);
            m_resources->subtractLipids(props.plantCostLipid);
            m_resources->subtractProteins(props.plantCostPro);
            m_resources->subtractVitamins(props.plantCostVit);
            updateProductionRates();
            return true;
        }
        updateProductionRates();
        return false;
    }
    return false;
}

// 移除指定位置的藻类
bool AlgaeGame::removeAlgae(int row, int col) {
    if (!m_isGameRunning) {
        return false;
    }

    AlgaeCell* cell = m_grid->getCell(row, col);
    if (cell && cell->isOccupied()) {
        cell->remove();
        updateProductionRates();
        return true;
    }

    return false;
}

// 删除音量设置函数
// void AlgaeGame::setMusicVolume(int volume) {
//     m_musicVolume = qBound(0, volume, 100);
// }

// void AlgaeGame::setSoundEffectsVolume(int volume) {
//     m_soundEffectsVolume = qBound(0, volume, 100);
// }

// 游戏主循环，每帧调用
void AlgaeGame::update() {
    if (!m_isGameRunning) {
        return;
    }
    // 计算时间增量
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    double deltaTime = (currentTime - m_lastUpdateTime) / 1000.0; // 转换为秒
    m_lastUpdateTime = currentTime;
    // 更新网格（更新所有单元格）
    m_grid->update(deltaTime);
    // 根据生产速率更新资源
    m_resources->update(deltaTime);
    // 检查胜利条件
    if (m_resources->checkWinCondition()) {
        emit gameWon();
    }
    // 新增：每帧刷新UI网格和胜利条件栏
    emit m_grid->gridUpdated();
    emit m_resources->resourcesChanged();
}

// 计算所有单元格的生产速率总和，并更新资源
void AlgaeGame::updateProductionRates() {
    // Calculate total production rates from all cells
    double totalCarb = 0.0;
    double totalLipid = 0.0;
    double totalPro = 0.0;
    double totalVit = 0.0;

    for (int row = 0; row < m_grid->getRows(); ++row) {
        for (int col = 0; col < m_grid->getCols(); ++col) {
            AlgaeCell* cell = m_grid->getCell(row, col);
            if (cell && cell->isOccupied()) {
                totalCarb += cell->getCarbProduction();
                totalLipid += cell->getLipidProduction();
                totalPro += cell->getProProduction();
                totalVit += cell->getVitProduction();
            }
        }
    }

    // Update resources with new rates
    m_resources->setCarbRate(totalCarb);
    m_resources->setLipidRate(totalLipid);
    m_resources->setProRate(totalPro);
    m_resources->setVitRate(totalVit);
}

// 网格变化时自动刷新生产速率和胜利判定
void AlgaeGame::onGridChanged() {
    // 更新资源生产速率
    updateProductionRates();
    
    // 检查游戏状态
    if (m_resources->checkWinCondition()) {
        // pauseGame(); // 不再自动暂停
        emit gameWon();
    }
}

// 资源变化时通知UI
void AlgaeGame::onResourcesChanged() {
    // 更新UI显示
    emit resourcesUpdated();
}
