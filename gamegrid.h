#ifndef GAMEGRID_H
#define GAMEGRID_H

#include <QWidget>  // Changed from QObject to QWidget
#include <QVector>
#include "algaecell.h"
#include <QMouseEvent>  // Added for QMouseEvent
#include <QPaintEvent>  // Added for QPaintEvent

class GameGrid : public QWidget {  // Changed base class to QWidget
    Q_OBJECT

public:
    static const int ROWS = 10;
    static const int COLS = 8;

    explicit GameGrid(QWidget* parent = nullptr);  // Changed parent type to QWidget*
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

protected:
    int m_hoverRow = -1;  // 当前悬停行（初始无效）
    int m_hoverCol = -1;  // 当前悬停列（初始无效）
    const int CELL_SIZE = 64;  // 假设单元格尺寸为64x64像素
protected:
    void mouseMoveEvent(QMouseEvent* event) override;  // Valid override for QWidget
    void paintEvent(QPaintEvent* event) override;  // Added paintEvent override
};

#endif // GAMEGRID_H
