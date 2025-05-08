#ifndef ALGAEGAME_H
#define ALGAEGAME_H

#include <QObject>
#include <QTimer>
#include "gamegrid.h"
#include "gameresources.h"
#include "algaetype.h"

class AlgaeGame : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool gameRunning READ isGameRunning NOTIFY gameStateChanged)
    Q_PROPERTY(AlgaeType::Type selectedAlgaeType READ getSelectedAlgaeType WRITE setSelectedAlgaeType NOTIFY selectedAlgaeChanged)

public:
    explicit AlgaeGame(QObject* parent = nullptr);
    ~AlgaeGame();

    // Game state
    bool isGameRunning() const { return m_gameRunning; }

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

public slots:
    void update();
    void onResourcesChanged();
signals:
    void gameStateChanged();
    void selectedAlgaeChanged();
    void gameWon();

private:
    bool m_gameRunning;
    AlgaeType::Type m_selectedAlgaeType;

    QTimer* m_updateTimer;
    qint64 m_lastUpdateTime;

    GameGrid* m_grid;
    GameResources* m_resources;

    int m_musicVolume;
    int m_soundEffectsVolume;

    void updateProductionRates();
};

#endif // ALGAEGAME_H
