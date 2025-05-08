#include "algaegame.h"
#include <QDateTime>

AlgaeGame::AlgaeGame(QWidget* gridParent, QObject* parent) 
    : QObject(parent), 
      m_gameRunning(false),
      m_selectedAlgaeType(AlgaeType::TYPE_A),
      m_updateTimer(new QTimer(this)),
      m_lastUpdateTime(0),
      m_musicVolume(50),
      m_soundEffectsVolume(50) {

    // Pass gridParent (QWidget*) to GameGrid
    m_grid = new GameGrid(gridParent);
    m_resources = new GameResources(this);

    // Set up timer for game updates
    m_updateTimer->setInterval(50); // 20 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &AlgaeGame::update);

    // Initialize last update time
    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
}

AlgaeGame::~AlgaeGame() {
    // Stop the timer
    m_updateTimer->stop();
}

void AlgaeGame::setSelectedAlgaeType(AlgaeType::Type type) {
    if (m_selectedAlgaeType != type) {
        m_selectedAlgaeType = type;
        emit selectedAlgaeChanged();
        emit algaeTypeSelected(type); // 触发新信号
    }
}

void AlgaeGame::startGame() {
    if (!m_gameRunning) {
        m_gameRunning = true;
        m_updateTimer->start();
        emit gameStateChanged();
    }
}

void AlgaeGame::pauseGame() {
    if (m_gameRunning) {
        m_gameRunning = false;
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
    if (!m_gameRunning || m_selectedAlgaeType == AlgaeType::NONE) {
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
        // 获取种植成本（需在AlgaeType中新增获取成本的方法）
        double carbCost, lipidCost, proCost, vitCost;
        AlgaeType::getPlantingCost(m_selectedAlgaeType, carbCost, lipidCost, proCost, vitCost);

        // 扣除资源（使用成本值而非剩余值）
        m_resources->subtractCarbohydrates(carbCost);
        m_resources->subtractLipids(lipidCost);
        m_resources->subtractProteins(proCost);
        m_resources->subtractVitamins(vitCost);

        updateProductionRates();
        return true;
    }

    return false; // 新增：处理种植失败的情况
}

bool AlgaeGame::removeAlgae(int row, int col) {
    if (!m_gameRunning) {
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

void AlgaeGame::setMusicVolume(int volume) {
    m_musicVolume = qBound(0, volume, 100);
}

void AlgaeGame::setSoundEffectsVolume(int volume) {
    m_soundEffectsVolume = qBound(0, volume, 100);
}

void AlgaeGame::update() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_lastUpdateTime == 0) m_lastUpdateTime = currentTime;
    double deltaTime = (currentTime - m_lastUpdateTime) / 1000.0; // 转换为秒
    m_lastUpdateTime = currentTime;

    m_grid->update(deltaTime);  // 正确传递deltaTime
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

void AlgaeGame::onResourcesChanged() {
    // No action needed for now
}
