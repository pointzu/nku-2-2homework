#ifndef ALGAEGAME_H
#define ALGAEGAME_H

#include <QObject>
#include <QTimer>
#include "gamegrid.h"
#include "gameresources.h"
#include "algaetype.h"

class AlgaeGame : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameStateChanged)
    Q_PROPERTY(AlgaeType::Type selectedAlgaeType READ getSelectedAlgaeType WRITE setSelectedAlgaeType NOTIFY selectedAlgaeChanged)

public:
    explicit AlgaeGame(QWidget* parent = nullptr);
    ~AlgaeGame();

    // Game state
    bool isGameRunning() const { return m_isGameRunning; }

    // Algae selection
    AlgaeType::Type getSelectedAlgaeType() const { return m_selectedAlgaeType; }
    void setSelectedAlgaeType(AlgaeType::Type type);

    // Game grid access
    GameGrid* getGrid() const { return m_grid; }

    // Resources access
    GameResources* getResources() const { return m_resources; }

    // Game actions
    void startGame();
    void pauseGame();
    void resetGame();

    // Cell interactions
    bool plantAlgae(int row, int col);
    bool removeAlgae(int row, int col);

    // Settings
    void setMusicVolume(int volume);
    void setSoundEffectsVolume(int volume);

    AlgaeCell::PlantResult getLastPlantResult() const { return m_lastPlantResult; }

public slots:
    void update();
    void onGridChanged();
    void onResourcesChanged();

signals:
    void gameStateChanged();
    void selectedAlgaeChanged();
    void gameWon();
    void resourcesUpdated();

private:
    bool m_isGameRunning;
    AlgaeType::Type m_selectedAlgaeType;

    QTimer* m_updateTimer;
    qint64 m_lastUpdateTime;

    GameGrid* m_grid;
    GameResources* m_resources;

    AlgaeCell::PlantResult m_lastPlantResult = AlgaeCell::PLANT_SUCCESS;

    // 删除音乐相关成员
    // int m_musicVolume;
    // int m_soundEffectsVolume;

    void updateProductionRates();
};

#endif // ALGAEGAME_H
