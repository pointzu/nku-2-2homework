#ifndef ALGAECELL_H
#define ALGAECELL_H

#include "algaetype.h"
#include <QObject>
#include <QPointF>
#include <QPainter> // Add this line

class GameGrid;

class AlgaeCell : public QObject {
    Q_OBJECT

public:
    enum Status {
        NORMAL,
        RESOURCE_LOW,
        LIGHT_LOW,
        DYING
    };

    AlgaeCell(int row, int col, GameGrid* parent = nullptr);
    ~AlgaeCell();

    bool plant(AlgaeType::Type type);
    void remove();
    void update(double deltaTime);
    void paint(QPainter* painter, int cellSize) const; // Add cellSize parameter

    // 以下成员函数应放在类内部
    AlgaeType::Type getType() const { return m_type; }
    Status getStatus() const { return m_status; }
    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    double getProductionMultiplier() const { return m_productionMultiplier; }
    bool isOccupied() const { return m_type != AlgaeType::NONE; }
    bool isDying() const { return m_status == DYING; }

    // Calculate production rates based on current conditions
    double getCarbProduction() const;
    double getLipidProduction() const;
    double getProProduction() const;
    double getVitProduction() const;

signals:
    void statusChanged(Status newStatus);
    void cellChanged();
    void algaeDied();

private:
    int m_row;
    int m_col;
    GameGrid* m_grid;

    AlgaeType::Type m_type;
    AlgaeType::Properties m_properties;
    Status m_status;

    double m_productionMultiplier;
    double m_timeSinceLightLow;
    double m_growthStage;  // 新增：生长阶段变量

    void updateStatus(double deltaTime);  // 修改：添加参数声明
    void checkSpecialRules();
};

#endif // ALGAECELL_H
