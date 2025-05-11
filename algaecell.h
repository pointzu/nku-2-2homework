#ifndef ALGAECELL_H // 防止头文件重复包含
#define ALGAECELL_H

#include "algaetype.h" // 藻类类型定义
#include <QObject>      // Qt对象基类
#include <QPointF>      // Qt二维点
#include <QWidget>      // Qt控件基类
#include <QMediaPlayer> // 多媒体播放器
#include <QAudioOutput> // 音频输出
#include <QString>      // Qt字符串

class GameGrid; // 前置声明，网格类

// 藻类单元格控件，负责单格的状态、交互与显示
class AlgaeCell : public QWidget {
    Q_OBJECT

public:
    // 单元格状态枚举
    enum Status {
        NORMAL,         // 正常
        RESOURCE_LOW,   // 资源不足
        LIGHT_LOW,      // 光照不足
        DYING           // 濒死
    };

    // 种植结果枚举
    enum PlantResult {
        PLANT_SUCCESS,           // 种植成功
        PLANT_OCCUPIED,          // 已被占用
        PLANT_LIGHT_LOW,         // 光照略低
        PLANT_LIGHT_INSUFFICIENT,// 光照严重不足
        PLANT_RESOURCE_LOW,      // 资源不足
        PLANT_RESERVED           // 资源预定
    };

    AlgaeCell(int row, int col, GameGrid* parent = nullptr); // 构造函数
    ~AlgaeCell(); // 析构函数

    PlantResult plant(AlgaeType::Type type, double lightLevel, bool canAfford, bool canReserve); // 种植藻类
    void remove();      // 移除藻类
    void update(double deltaTime); // 刷新状态

    AlgaeType::Type getType() const { return m_type; } // 获取类型
    Status getStatus() const { return m_status; }      // 获取状态

    int getRow() const { return m_row; } // 获取行号
    int getCol() const { return m_col; } // 获取列号

    double getProductionMultiplier() const { return m_productionMultiplier; } // 获取产量倍率

    bool isOccupied() const { return m_type != AlgaeType::NONE; } // 是否被占用
    bool isDying() const { return m_status == DYING; }            // 是否濒死

    // 计算当前产量
    double getCarbProduction() const { return m_carbProduction; }     // 糖产量
    double getLipidProduction() const { return m_lipidProduction; }   // 脂产量
    double getProProduction() const { return m_proProduction; }       // 蛋白产量
    double getVitProduction() const { return m_vitProduction; }       // 维生素产量

    // 新增：鼠标交互相关方法
    void setHovered(bool hovered);   // 设置悬浮
    void setSelected(bool selected); // 设置选中
    bool isHovered() const { return m_isHovered; }   // 是否悬浮
    bool isSelected() const { return m_isSelected; } // 是否选中
    void showShadingArea(bool show); // 显示/隐藏遮荫区
    bool isShadingAreaVisible() const { return m_showShadingArea; } // 遮荫区是否可见
    void setShadingVisible(bool visible); // 设置遮荫区可见
    bool isShadingVisible() const { return m_showShadingArea; }     // 遮荫区是否可见

    void setStatus(Status status); // 设置状态

    // --- 特性可视化状态 ---
    bool m_isReducedByNeighborA = false;
    bool m_isBoostedByNeighborB = false;
    bool m_isReducedByNeighborB = false;
    void setReducedByNeighborA(bool v) { m_isReducedByNeighborA = v; }
    bool isReducedByNeighborA() const { return m_isReducedByNeighborA; }
    void setBoostedByNeighborB(bool v) { m_isBoostedByNeighborB = v; }
    bool isBoostedByNeighborB() const { return m_isBoostedByNeighborB; }
    void setReducedByNeighborB(bool v) { m_isReducedByNeighborB = v; }
    bool isReducedByNeighborB() const { return m_isReducedByNeighborB; }
    // D型协同特性
    bool m_isSynergizedByNeighbor = false; // 被D型协同
    bool m_isSynergizingNeighbor = false;  // D型正在协同别人
    void setSynergizedByNeighbor(bool v) { m_isSynergizedByNeighbor = v; }
    bool isSynergizedByNeighbor() const { return m_isSynergizedByNeighbor; }
    void setSynergizingNeighbor(bool v) { m_isSynergizingNeighbor = v; }
    bool isSynergizingNeighbor() const { return m_isSynergizingNeighbor; }
    // E型加光特性
    bool m_isLightedByE = false; // 被E型加光
    void setLightedByE(bool v) { m_isLightedByE = v; }
    bool isLightedByE() const { return m_isLightedByE; }

protected:
    void paintEvent(QPaintEvent* event) override;      // 绘制事件
    void enterEvent(QEnterEvent* event) override;      // 鼠标进入事件
    void leaveEvent(QEvent* event) override;           // 鼠标离开事件
    void mousePressEvent(QMouseEvent* event) override; // 鼠标点击事件

signals:
    void statusChanged(Status newStatus); // 状态变化信号
    void cellChanged();                   // 单元格变化信号
    void algaeDied();                     // 藻类死亡信号
    void cellClicked(int row, int col);   // 单元格点击信号
    void cellHovered(int row, int col, bool entered); // 单元格悬浮信号

private:
    int m_row;           // 行号
    int m_col;           // 列号
    GameGrid* m_grid;    // 所属网格指针

    AlgaeType::Type m_type;         // 当前藻类类型
    AlgaeType::Properties m_properties; // 当前属性
    Status m_status;                 // 当前状态

    double m_productionMultiplier;   // 产量倍率
    double m_timeSinceLightLow;      // 光照低计时

    // 新增：鼠标交互相关属性
    bool m_isHovered;        // 是否悬浮
    bool m_isSelected;       // 是否选中
    bool m_showShadingArea = false; // 遮荫区是否显示

    double m_carbProduction;   // 糖产量
    double m_lipidProduction;  // 脂产量
    double m_proProduction;    // 蛋白产量
    double m_vitProduction;    // 维生素产量

    QMediaPlayer* m_player;      // 播放器
    QAudioOutput* m_audioOutput; // 音频输出

    void updateStatus();         // 刷新状态
    void checkSpecialRules();    // 检查特殊规则
    void updateAppearance();     // 刷新外观
    void updateProductionRates();// 刷新产量
};

#endif // ALGAECELL_H
