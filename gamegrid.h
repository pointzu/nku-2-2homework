#ifndef GAMEGRID_H
#define GAMEGRID_H

#include <QWidget>
#include <QGridLayout>
#include <vector>
#include "algaecell.h"
#include "algaetype.h"

class GameGrid : public QWidget {
    Q_OBJECT

public:
    explicit GameGrid(QWidget* parent = nullptr);
    ~GameGrid();

    void initialize(int rows, int cols);
    void update(double deltaTime);
    void reset();

    AlgaeCell* getCell(int row, int col) const;
    bool plantAlgae(int row, int col, AlgaeType::Type type);
    void removeAlgae(int row, int col);

    // 新增：鼠标交互相关方法
    void setSelectedAlgaeType(AlgaeType::Type type);
    AlgaeType::Type getSelectedAlgaeType() const { return m_selectedAlgaeType; }
    void updateCursor();
    void showShadingArea(int row, int col, bool show);

    // Grid properties
    int getRows() const { return m_rows; }
    int getCols() const { return m_cols; }

    // Light and resources
    double getLightAt(int row) const;
    double getLightAt(int row, int col) const;
    double getNitrogenAt(int row, int col) const;
    double getCarbonAt(int row, int col) const;
    double getNitrogenRegenRate(int row, int col) const;
    double getCarbonRegenRate(int row, int col) const;

    // Calculate combined effects
    int calculateShadingAt(int row, int col) const;
    void applyRemoveBonus(int row, int col);

    // 新增：悬浮预判种植后光照
    double getLightAtIfPlanted(int row, int col, AlgaeType::Type type) const;

signals:
    void cellChanged(int row, int col);
    void algaePlanted(int row, int col, AlgaeType::Type type);
    void algaeRemoved(int row, int col);
    void gridChanged();
    void resourcesChanged();
    void gridUpdated();
    void cellClicked(int row, int col);
    void cellHovered(int row, int col, bool entered);

private slots:
    void onCellClicked(int row, int col);
    void onCellHovered(int row, int col, bool entered);

private:
    QGridLayout* m_layout;
    std::vector<std::vector<AlgaeCell*>> m_cells;
    int m_rows;
    int m_cols;

    // 新增：鼠标交互相关属性
    AlgaeType::Type m_selectedAlgaeType;
    QCursor m_defaultCursor;
    QCursor m_algaeCursor;

    QVector<double> m_baseLight;
    QVector<QVector<double>> m_nitrogen;
    QVector<QVector<double>> m_nitrogenRegen;
    QVector<QVector<double>> m_carbon;
    QVector<QVector<double>> m_carbonRegen;

    void createCells();
    void clearCells();
    void updateShadingAreas();
    void initializeGrid();
    void initializeResources();
    void updateResources(double deltaTime);
    void calculateSpecialEffects();
};

#endif // GAMEGRID_H
