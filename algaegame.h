#ifndef ALGAEGAME_H // 防止头文件重复包含
#define ALGAEGAME_H

#include <QObject>      // Qt对象基类
#include <QTimer>       // Qt定时器
#include "gamegrid.h" // 游戏网格类
#include "gameresources.h" // 资源管理类
#include "algaetype.h"     // 藻类类型定义

// 游戏主逻辑类，负责管理网格、资源、状态、信号等
class AlgaeGame : public QObject {
    Q_OBJECT

    // Qt属性系统，支持QML或属性绑定
    Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameStateChanged)
    Q_PROPERTY(AlgaeType::Type selectedAlgaeType READ getSelectedAlgaeType WRITE setSelectedAlgaeType NOTIFY selectedAlgaeChanged)

public:
    explicit AlgaeGame(QWidget* parent = nullptr); // 构造函数
    ~AlgaeGame(); // 析构函数

    // 游戏状态
    bool isGameRunning() const { return m_isGameRunning; } // 是否正在运行

    // 藻类选择
    AlgaeType::Type getSelectedAlgaeType() const { return m_selectedAlgaeType; } // 获取当前选中藻类
    void setSelectedAlgaeType(AlgaeType::Type type); // 设置选中藻类

    // 网格访问
    GameGrid* getGrid() const { return m_grid; } // 获取网格指针

    // 资源访问
    GameResources* getResources() const { return m_resources; } // 获取资源指针

    // 游戏操作
    void startGame();  // 开始游戏
    void pauseGame();  // 暂停游戏
    void resetGame();  // 重置游戏

    // 单元格交互
    bool plantAlgae(int row, int col);   // 种植藻类
    bool removeAlgae(int row, int col);  // 移除藻类

    // 设置（已注释掉音量相关）
    void setMusicVolume(int volume);         // 设置音乐音量（已废弃）
    void setSoundEffectsVolume(int volume);  // 设置音效音量（已废弃）

    AlgaeCell::PlantResult getLastPlantResult() const { return m_lastPlantResult; } // 获取上次种植结果

public slots:
    void update();           // 游戏主循环
    void onGridChanged();    // 网格变化槽
    void onResourcesChanged(); // 资源变化槽

signals:
    void gameStateChanged();      // 游戏状态变化信号
    void selectedAlgaeChanged();  // 选中藻类变化信号
    void gameWon();               // 游戏胜利信号
    void resourcesUpdated();      // 资源刷新信号

private:
    bool m_isGameRunning;           // 游戏是否运行中
    AlgaeType::Type m_selectedAlgaeType; // 当前选中藻类

    QTimer* m_updateTimer;          // 游戏主循环定时器
    qint64 m_lastUpdateTime;        // 上次更新时间戳

    GameGrid* m_grid;               // 游戏网格指针
    GameResources* m_resources;     // 资源管理指针

    AlgaeCell::PlantResult m_lastPlantResult = AlgaeCell::PLANT_SUCCESS; // 上次种植结果

    // 删除音乐相关成员
    // int m_musicVolume;
    // int m_soundEffectsVolume;

    void updateProductionRates(); // 更新生产速率
};

#endif // ALGAEGAME_H
