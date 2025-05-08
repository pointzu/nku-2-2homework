#include "algaegame.h"
#include <QDateTime>

AlgaeGame::AlgaeGame(QWidget* parent)
    : QObject(parent)
    , m_grid(new GameGrid(parent))
    , m_resources(new GameResources(this))
    , m_selectedAlgaeType(AlgaeType::NONE)
    , m_isGameRunning(false)
    , m_updateTimer(new QTimer(this))
    , m_lastUpdateTime(QDateTime::currentMSecsSinceEpoch())
{
    // 初始化游戏
    m_grid->initialize(10, 8);  // 10行8列的网格
    
    // 连接信号
    connect(m_grid, &GameGrid::gridChanged, this, &AlgaeGame::onGridChanged);
    connect(m_grid, &GameGrid::resourcesChanged, this, &AlgaeGame::onResourcesChanged);
    connect(m_grid, &GameGrid::cellClicked, this, [this](int row, int col) {
        if (m_selectedAlgaeType != AlgaeType::NONE) {
            plantAlgae(row, col);
        } else {
            removeAlgae(row, col);
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
    
    // 设置定时器
    m_updateTimer->setInterval(200);  // 200ms 刷新一次
    connect(m_updateTimer, &QTimer::timeout, this, &AlgaeGame::update);
}

AlgaeGame::~AlgaeGame() {
    // Stop the timer
    m_updateTimer->stop();
}

void AlgaeGame::setSelectedAlgaeType(AlgaeType::Type type) {
    if (m_selectedAlgaeType != type) {
        m_selectedAlgaeType = type;
        emit selectedAlgaeChanged();
    }
}

void AlgaeGame::startGame() {
    if (!m_isGameRunning) {
        m_isGameRunning = true;
        m_updateTimer->start();
        emit gameStateChanged();
    }
}

void AlgaeGame::pauseGame() {
    if (m_isGameRunning) {
        m_isGameRunning = false;
        m_updateTimer->stop();
        emit gameStateChanged();
    }
}

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

bool AlgaeGame::plantAlgae(int row, int col) {
    if (!m_isGameRunning || m_selectedAlgaeType == AlgaeType::NONE) {
        return false;
    }

    // Check if we can afford to plant
    double carb = m_resources->getCarbohydrates();
    double lipid = m_resources->getLipids();
    double pro = m_resources->getProteins();
    double vit = m_resources->getVitamins();

    if (!AlgaeType::canAfford(m_selectedAlgaeType, carb, lipid, pro, vit)) {
        return false;
    }

    // Try to plant
    AlgaeCell* cell = m_grid->getCell(row, col);
    if (cell && cell->plant(m_selectedAlgaeType)) {
        // Deduct resources
        AlgaeType::deductPlantingCost(m_selectedAlgaeType, carb, lipid, pro, vit);
        m_resources->subtractCarbohydrates(carb - m_resources->getCarbohydrates());
        m_resources->subtractLipids(lipid - m_resources->getLipids());
        m_resources->subtractProteins(pro - m_resources->getProteins());
        m_resources->subtractVitamins(vit - m_resources->getVitamins());

        // Update production rates
        updateProductionRates();

        return true;
    }

    return false;
}

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
        // pauseGame(); // 不再自动暂停
        emit gameWon();
    }
}

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

void AlgaeGame::onGridChanged() {
    // 更新资源生产速率
    updateProductionRates();
    
    // 检查游戏状态
    if (m_resources->checkWinCondition()) {
        // pauseGame(); // 不再自动暂停
        emit gameWon();
    }
}

void AlgaeGame::onResourcesChanged() {
    // 更新UI显示
    emit resourcesUpdated();
}
