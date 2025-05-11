#ifndef MAINWINDOW_H // 防止头文件重复包含
#define MAINWINDOW_H

#include <QMainWindow>    // Qt主窗口基类
#include <QGridLayout>    // Qt网格布局
#include <QPushButton>    // 按钮控件
#include <QLabel>         // 标签控件
#include <QProgressBar>   // 进度条控件
#include <QSlider>        // 滑块控件
#include <QGridLayout>    // 网格布局
#include <QMenu>          // 菜单
#include <QAction>        // 动作
#include "algaegame.h"   // 游戏主逻辑类
#include <QMenuBar>       // 菜单栏
#include <QGroupBox>      // 分组框
#include <QCursor>        // 鼠标指针
#include <QtMultimedia/QMediaPlayer> // 多媒体播放器
#include <QtMultimedia/QAudioOutput> // 音频输出
#include <QSoundEffect>   // 音效
#include <QMap>           // Qt字典
#include <QScrollArea>    // 滚动区域

class CellWidget; // 前置声明，格子控件

// 主窗口类，负责UI和游戏交互
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // 构造函数
    ~MainWindow(); // 析构函数
    AlgaeGame* getGame() const { return m_game; } // 获取游戏指针
    bool isShadingPreviewEnabled() const { return m_showShadingPreview; } // 是否显示遮荫预览
    void playEffect(const QString& name); // 播放音效
    void playSoundEffect(const QString& resource); // 播放音效（备用）

protected:
    void keyPressEvent(QKeyEvent *event) override;   // 键盘按下事件
    void keyReleaseEvent(QKeyEvent *event) override; // 键盘释放事件

private slots:
    void onCellClicked(int row, int col);        // 单元格左键点击槽
    void onCellRightClicked(int row, int col);   // 单元格右键点击槽
    void onGameStateChanged();                   // 游戏状态变化槽
    void onResourcesChanged();                   // 资源变化槽
    void onProductionRatesChanged();             // 生产速率变化槽
    void showGameMenu();                         // 显示菜单
    void restartGame();                          // 重新开始
    void exitGame();                             // 退出游戏
    void showSettingsDialog();                   // 显示设置对话框
    void updateWinProgress();                    // 刷新通关进度
    void onGameWon();                            // 游戏胜利槽
    void showTraitDetailDialog();                 // 显示详细特性说明弹窗

private:
    AlgaeGame* m_game; // 游戏主逻辑指针

    // UI组件
    QWidget* m_centralWidget;      // 中央控件
    QGridLayout* m_gridLayout;     // 主网格布局
    QGridLayout* m_cellsLayout;    // 游戏格子布局
    QVector<QVector<CellWidget*>> m_cellWidgets; // 格子控件二维数组

    // 游戏控制按钮
    QPushButton* m_btnTypeA; // 选择A型藻类
    QPushButton* m_btnTypeB; // 选择B型藻类
    QPushButton* m_btnTypeC; // 选择C型藻类
    QPushButton* m_btnTypeD; // 选择D型藻类
    QPushButton* m_btnTypeE;

    // 资源显示
    QLabel* m_lblCarb;   // 糖类
    QLabel* m_lblLipid;  // 脂质
    QLabel* m_lblPro;    // 蛋白质
    QLabel* m_lblVit;    // 维生素

    // 生产速率显示
    QLabel* m_lblCarbRate;   // 糖类速率
    QLabel* m_lblLipidRate;  // 脂质速率
    QLabel* m_lblProRate;    // 蛋白质速率
    QLabel* m_lblVitRate;    // 维生素速率

    // 通关进度
    QProgressBar* m_progressBar; // 进度条

    // 菜单
    QMenu* m_gameMenu;       // 游戏菜单
    QAction* m_restartAction;// 重新开始动作
    QAction* m_settingsAction;// 设置动作
    QAction* m_exitAction;   // 退出动作

    // 胜利条件显示
    QGroupBox* m_winConditionGroup; // 条件分组
    QLabel* m_lblCarbCond;   // 糖类条件
    QLabel* m_lblLipidCond;  // 脂质条件
    QLabel* m_lblProCond;    // 蛋白质条件
    QLabel* m_lblVitCond;    // 维生素条件
    QLabel* m_lblCarbRateCond; // 糖类速率条件
    QLabel* m_lblLipidRateCond;// 脂质速率条件
    QLabel* m_lblProRateCond;  // 蛋白质速率条件
    QLabel* m_lblVitRateCond;  // 维生素速率条件

    // 藻类选择图标
    QLabel* m_iconTypeA; // A型图标
    QLabel* m_iconTypeB; // B型图标
    QLabel* m_iconTypeC; // C型图标
    QLabel* m_iconTypeD; // D型图标
    QLabel* m_iconTypeE;
    // 藻类鼠标指针
    QCursor m_cursorTypeA; // A型指针
    QCursor m_cursorTypeB; // B型指针
    QCursor m_cursorTypeC; // C型指针
    QCursor m_cursorTypeD; // D型指针
    QCursor m_cursorTypeE;

    QLabel* m_scoreLabel; // 实时分数栏
    int m_highScore = 0; // 最高分

    bool m_hasShownWinMsg = false; // 胜利提示是否已弹出
    bool m_showShadingPreview = false; // 是否显示遮荫预览

    QMediaPlayer* m_bgmPlayer = nullptr; // 背景音乐播放器
    QAudioOutput* m_bgmAudio = nullptr;  // 背景音乐输出
    QMediaPlayer* m_effectPlayer = nullptr; // 音效播放器
    QAudioOutput* m_effectAudio = nullptr;  // 音效输出
    double m_lastBgmProgress = -1.0; // 上次BGM进度

    QMap<QString, QSoundEffect*> m_soundEffects; // 音效字典

    QLabel* m_scoreHintLabel;   // 分数评价简介信息栏
    QLabel* m_scoreDetailLabel; // 分数组成详细说明

    QLabel* m_traitInfoLabel; // 植株特性信息栏
    QScrollArea* m_traitInfoScrollArea; // 特性信息滚动区域
    QPushButton* m_btnTraitDetail; // 详细特性说明按钮

    double m_effectVolume = 1.0; // 新增，音效音量（0.0~1.0）

    void setupUI();                // 初始化UI
    void setupGameGrid();          // 初始化网格
    void setupGameControls();      // 初始化控制按钮
    void setupResourceDisplay();   // 初始化资源显示
    void setupMenus();             // 初始化菜单
    void connectSignals();         // 连接信号槽
    void updateSelectedAlgaeButton(); // 刷新选中按钮
    void updateGridDisplay();         // 刷新网格显示
    void updateCellDisplay(int row, int col); // 刷新单元格显示
    void displayCellInfo(int row, int col);   // 显示单元格信息
    void initializeCellWidgets();             // 初始化格子控件
    void updateWinConditionLabels();          // 刷新胜利条件标签
    void updateScoreBar();                    // 刷新分数栏
    void playBGM(double progress);            // 播放背景音乐
};

// 网格格子控件，负责单元格UI显示与交互
class CellWidget : public QWidget {
    Q_OBJECT

public:
    CellWidget(int row, int col, QWidget* parent = nullptr); // 构造函数
    void setAlgaeCell(AlgaeCell* cell); // 设置对应的藻类单元格
    int getRow() const { return m_row; } // 获取行号
    int getCol() const { return m_col; } // 获取列号
    bool isHovered() const { return m_hovered; } // 是否悬浮

signals:
    void leftClicked(int row, int col);   // 左键点击信号
    void rightClicked(int row, int col);  // 右键点击信号
    void hovered(int row, int col);       // 悬浮信号

protected:
    void paintEvent(QPaintEvent* event) override;      // 绘制事件
    void mousePressEvent(QMouseEvent* event) override; // 鼠标点击事件
    void enterEvent(QEnterEvent* event) override;      // 鼠标进入事件
    void leaveEvent(QEvent* event) override;           // 鼠标离开事件

private:
    int m_row;           // 行号
    int m_col;           // 列号
    AlgaeCell* m_cell = nullptr; // 对应的藻类单元格
    bool m_hovered = false;      // 是否悬浮
};

#endif // MAINWINDOW_H
