#ifndef GAMERESOURCES_H // 防止头文件重复包含
#define GAMERESOURCES_H

#include <QObject> // Qt对象基类

// 游戏资源管理类，负责管理糖类、脂质、蛋白质、维生素及其生产速率
class GameResources : public QObject {
    Q_OBJECT

public:
    explicit GameResources(QObject* parent = nullptr); // 构造函数

    // 资源获取函数
    double getCarbohydrates() const { return m_carbohydrates; } // 获取糖类
    double getLipids() const { return m_lipids; }               // 获取脂质
    double getProteins() const { return m_proteins; }           // 获取蛋白质
    double getVitamins() const { return m_vitamins; }           // 获取维生素

    // 生产速率获取函数
    double getCarbRate() const { return m_carbRate; }     // 获取糖类速率
    double getLipidRate() const { return m_lipidRate; }   // 获取脂质速率
    double getProRate() const { return m_proRate; }       // 获取蛋白质速率
    double getVitRate() const { return m_vitRate; }       // 获取维生素速率

    // 资源增加函数
    void addCarbohydrates(double amount); // 增加糖类
    void addLipids(double amount);        // 增加脂质
    void addProteins(double amount);      // 增加蛋白质
    void addVitamins(double amount);      // 增加维生素

    // 资源减少函数
    void subtractCarbohydrates(double amount); // 减少糖类
    void subtractLipids(double amount);        // 减少脂质
    void subtractProteins(double amount);      // 减少蛋白质
    void subtractVitamins(double amount);      // 减少维生素

    // 生产速率设置函数
    void setCarbRate(double rate);   // 设置糖类速率
    void setLipidRate(double rate);  // 设置脂质速率
    void setProRate(double rate);    // 设置蛋白质速率
    void setVitRate(double rate);    // 设置维生素速率

    // 游戏状态相关
    void update(double deltaTime); // 随时间更新资源
    void reset();                  // 重置资源和速率

    // 胜利条件判断
    double getWinProgress() const; // 获取通关进度（0~1）
    bool checkWinCondition() const; // 检查是否达成胜利条件

signals:
    void resourcesChanged();         // 资源变化信号
    void productionRatesChanged();   // 生产速率变化信号

private:
    // 当前资源量
    double m_carbohydrates; // 糖类
    double m_lipids;        // 脂质
    double m_proteins;      // 蛋白质
    double m_vitamins;      // 维生素

    // 生产速率
    double m_carbRate;   // 糖类速率
    double m_lipidRate;  // 脂质速率
    double m_proRate;    // 蛋白质速率
    double m_vitRate;    // 维生素速率

    // 胜利条件阈值
    const double WIN_CARB = 500.0;   // 糖类目标
    const double WIN_LIPID = 300.0;  // 脂质目标
    const double WIN_PRO = 200.0;    // 蛋白质目标
    const double WIN_VIT = 100.0;    // 维生素目标

    // 生产速率目标
    const double TARGET_CARB_RATE = 50.0;   // 糖类速率目标
    const double TARGET_LIPID_RATE = 30.0;  // 脂质速率目标
    const double TARGET_PRO_RATE = 20.0;    // 蛋白质速率目标
    const double TARGET_VIT_RATE = 10.0;    // 维生素速率目标
};

#endif // GAMERESOURCES_H
