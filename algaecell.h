#ifndef ALGAECELL_H
#define ALGAECELL_H

#include "algaetype.h"
#include <QObject>
#include <QPointF>
#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QString>

class GameGrid;

class AlgaeCell : public QWidget {
    Q_OBJECT

public:
    enum Status {
        NORMAL,
        RESOURCE_LOW,
        LIGHT_LOW,
        DYING
    };

    enum PlantResult {
        PLANT_SUCCESS,
        PLANT_OCCUPIED,
        PLANT_LIGHT_LOW,
        PLANT_LIGHT_INSUFFICIENT,
        PLANT_RESOURCE_LOW,
        PLANT_RESERVED
    };

    AlgaeCell(int row, int col, GameGrid* parent = nullptr);
    ~AlgaeCell();

    PlantResult plant(AlgaeType::Type type, double lightLevel, bool canAfford, bool canReserve);
    void remove();
    void update(double deltaTime);

    AlgaeType::Type getType() const { return m_type; }
    Status getStatus() const { return m_status; }

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }

    double getProductionMultiplier() const { return m_productionMultiplier; }

    bool isOccupied() const { return m_type != AlgaeType::NONE; }
    bool isDying() const { return m_status == DYING; }

    // Calculate production rates based on current conditions
    double getCarbProduction() const { return m_carbProduction; }
    double getLipidProduction() const { return m_lipidProduction; }
    double getProProduction() const { return m_proProduction; }
    double getVitProduction() const { return m_vitProduction; }

    // 新增：鼠标交互相关方法
    void setHovered(bool hovered);
    void setSelected(bool selected);
    bool isHovered() const { return m_isHovered; }
    bool isSelected() const { return m_isSelected; }
    void showShadingArea(bool show);
    bool isShadingAreaVisible() const { return m_showShadingArea; }
    void setShadingVisible(bool visible);
    bool isShadingVisible() const { return m_showShadingArea; }

    void setStatus(Status status);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void statusChanged(Status newStatus);
    void cellChanged();
    void algaeDied();
    void cellClicked(int row, int col);
    void cellHovered(int row, int col, bool entered);

private:
    int m_row;
    int m_col;
    GameGrid* m_grid;

    AlgaeType::Type m_type;
    AlgaeType::Properties m_properties;
    Status m_status;

    double m_productionMultiplier;
    double m_timeSinceLightLow;

    // 新增：鼠标交互相关属性
    bool m_isHovered;
    bool m_isSelected;
    bool m_showShadingArea = false;

    double m_carbProduction;
    double m_lipidProduction;
    double m_proProduction;
    double m_vitProduction;

    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    void updateStatus();
    void checkSpecialRules();
    void updateAppearance();
    void updateProductionRates();
};

#endif // ALGAECELL_H
