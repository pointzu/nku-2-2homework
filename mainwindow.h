#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QGridLayout>
#include <QMenu>
#include <QAction>
#include "algaegame.h"
#include <QMenuBar>
#include <QGroupBox>
#include <QCursor>

class CellWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCellClicked(int row, int col);
    void onCellRightClicked(int row, int col);
    void onGameStateChanged();
    void onResourcesChanged();
    void onProductionRatesChanged();
    void showGameMenu();
    void restartGame();
    void exitGame();
    void showSettingsDialog();
    void updateWinProgress();
    void onGameWon();

private:
    AlgaeGame* m_game;

    // UI Components
    QWidget* m_centralWidget;
    QGridLayout* m_gridLayout;
    QGridLayout* m_cellsLayout;
    QVector<QVector<CellWidget*>> m_cellWidgets;

    // Game controls
    QPushButton* m_btnTypeA;
    QPushButton* m_btnTypeB;
    QPushButton* m_btnTypeC;

    // Resource display
    QLabel* m_lblCarb;
    QLabel* m_lblLipid;
    QLabel* m_lblPro;
    QLabel* m_lblVit;

    // Production rates display
    QLabel* m_lblCarbRate;
    QLabel* m_lblLipidRate;
    QLabel* m_lblProRate;
    QLabel* m_lblVitRate;

    // Win progress
    QProgressBar* m_progressBar;

    // Menu
    QMenu* m_gameMenu;
    QAction* m_restartAction;
    QAction* m_settingsAction;
    QAction* m_exitAction;

    // 胜利条件显示
    QGroupBox* m_winConditionGroup;
    QLabel* m_lblCarbCond;
    QLabel* m_lblLipidCond;
    QLabel* m_lblProCond;
    QLabel* m_lblVitCond;
    QLabel* m_lblCarbRateCond;
    QLabel* m_lblLipidRateCond;
    QLabel* m_lblProRateCond;
    QLabel* m_lblVitRateCond;

    // 藻类选择图标
    QLabel* m_iconTypeA;
    QLabel* m_iconTypeB;
    QLabel* m_iconTypeC;
    // 藻类鼠标指针
    QCursor m_cursorTypeA;
    QCursor m_cursorTypeB;
    QCursor m_cursorTypeC;

    QLabel* m_scoreLabel; // 实时分数栏
    int m_highScore = 0; // 最高分

    bool m_hasShownWinMsg = false; // 胜利提示是否已弹出

    void setupUI();
    void setupGameGrid();
    void setupGameControls();
    void setupResourceDisplay();
    void setupMenus();
    void connectSignals();
    void updateSelectedAlgaeButton();
    void updateGridDisplay();
    void updateCellDisplay(int row, int col);
    void displayCellInfo(int row, int col);
    void initializeCellWidgets();
    void updateWinConditionLabels();
    void updateScoreBar();
};

// Helper widget for grid cells
class CellWidget : public QWidget {
    Q_OBJECT

public:
    CellWidget(int row, int col, QWidget* parent = nullptr);
    void setAlgaeCell(AlgaeCell* cell);
    int getRow() const { return m_row; }
    int getCol() const { return m_col; }

signals:
    void leftClicked(int row, int col);
    void rightClicked(int row, int col);
    void hovered(int row, int col);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;

private:
    int m_row;
    int m_col;
    AlgaeCell* m_cell = nullptr;
};

#endif // MAINWINDOW_H
