#ifndef GAMEGRID_H
#define GAMEGRID_H

#include <QObject>
#include <QVector>
#include "algaecell.h"

class GameGrid : public QObject {
    Q_OBJECT

public:
    static const int ROWS = 10;
    static const int COLS = 8;

    explicit GameGrid(QObject* parent = nullptr);
    ~GameGrid();

    // Cell access
    AlgaeCell* getCell(int row, int col);

    // Grid properties
    int getRows() const { return ROWS; }
    int getCols() const { return COLS; }

    // Light and resources
    double getLightAt(int row) const;
    double getNitrogenAt(int row, int col) const;
    double getCarbonAt(int row, int col) const;
    double getNitrogenRegenRate(int row, int col) const;
    double getCarbonRegenRate(int row, int col) const;

    // Game actions
    void update(double deltaTime);
    void reset();

    // Calculate combined effects
    // 修改函数声明为const
    int calculateShadingAt(int row, int col) const; // 添加const
    void applyRemoveBonus(int row, int col);

signals:
    void gridChanged();
    void cellChanged(int row, int col);
    void resourcesChanged();
    void gridUpdated(); // 添加此信号声明

private:
    QVector<QVector<AlgaeCell*>> m_cells;
    QVector<double> m_baseLight;
    QVector<QVector<double>> m_nitrogen;
    QVector<QVector<double>> m_nitrogenRegen;
    QVector<QVector<double>> m_carbon;
    QVector<QVector<double>> m_carbonRegen;

    void initializeGrid();
    void initializeResources();
    void updateResources(double deltaTime);
    void calculateSpecialEffects();
};

#endif // GAMEGRID_H
