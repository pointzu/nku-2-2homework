#include "gameresources.h" // 资源管理头文件
#include <QtMath>            // Qt数学函数

// 构造函数，初始化资源和速率
GameResources::GameResources(QObject* parent) : QObject(parent) {
    reset(); // 调用重置函数，初始化所有资源
}

// 增加糖类资源
void GameResources::addCarbohydrates(double amount) {
    m_carbohydrates += amount;
    emit resourcesChanged(); // 发射资源变化信号
}

// 增加脂质资源
void GameResources::addLipids(double amount) {
    m_lipids += amount;
    emit resourcesChanged();
}

// 增加蛋白质资源
void GameResources::addProteins(double amount) {
    m_proteins += amount;
    emit resourcesChanged();
}

// 增加维生素资源
void GameResources::addVitamins(double amount) {
    m_vitamins += amount;
    emit resourcesChanged();
}

// 减少糖类资源
void GameResources::subtractCarbohydrates(double amount) {
    m_carbohydrates = qMax(0.0, m_carbohydrates - amount); // 不低于0
    emit resourcesChanged();
}

// 减少脂质资源
void GameResources::subtractLipids(double amount) {
    m_lipids = qMax(0.0, m_lipids - amount);
    emit resourcesChanged();
}

// 减少蛋白质资源
void GameResources::subtractProteins(double amount) {
    m_proteins = qMax(0.0, m_proteins - amount);
    emit resourcesChanged();
}

// 减少维生素资源
void GameResources::subtractVitamins(double amount) {
    m_vitamins = qMax(0.0, m_vitamins - amount);
    emit resourcesChanged();
}

// 设置糖类生产速率
void GameResources::setCarbRate(double rate) {
    if (m_carbRate != rate) {
        m_carbRate = rate;
        emit productionRatesChanged(); // 发射速率变化信号
    }
}

// 设置脂质生产速率
void GameResources::setLipidRate(double rate) {
    if (m_lipidRate != rate) {
        m_lipidRate = rate;
        emit productionRatesChanged();
    }
}

// 设置蛋白质生产速率
void GameResources::setProRate(double rate) {
    if (m_proRate != rate) {
        m_proRate = rate;
        emit productionRatesChanged();
    }
}

// 设置维生素生产速率
void GameResources::setVitRate(double rate) {
    if (m_vitRate != rate) {
        m_vitRate = rate;
        emit productionRatesChanged();
    }
}

// 随时间增量更新资源
void GameResources::update(double deltaTime) {
    // 按当前生产速率增加资源
    m_carbohydrates += m_carbRate * deltaTime;
    m_lipids += m_lipidRate * deltaTime;
    m_proteins += m_proRate * deltaTime;
    m_vitamins += m_vitRate * deltaTime;

    emit resourcesChanged();
}

// 重置所有资源和速率为初始值
void GameResources::reset() {
    // 初始资源
    m_carbohydrates = 50.0;
    m_lipids = 30.0;
    m_proteins = 20.0;
    m_vitamins = 10.0;

    // 初始生产速率为0
    m_carbRate = 0.0;
    m_lipidRate = 0.0;
    m_proRate = 0.0;
    m_vitRate = 0.0;

    emit resourcesChanged();
    emit productionRatesChanged();
}

// 获取通关进度（以生产速率为主，资源量为辅）
double GameResources::getWinProgress() const {
    // 以生产速率为主，资源量为辅
    double carbRateProgress = qMin(1.0, m_carbRate / TARGET_CARB_RATE);
    double lipidRateProgress = qMin(1.0, m_lipidRate / TARGET_LIPID_RATE);
    double proRateProgress = qMin(1.0, m_proRate / TARGET_PRO_RATE);
    double vitRateProgress = qMin(1.0, m_vitRate / TARGET_VIT_RATE);
    double rateProgress = (carbRateProgress + lipidRateProgress + proRateProgress + vitRateProgress) / 4.0;
    // 资源量仅作辅助（不拖慢进度，但资源为0时进度不满）
    double carbProgress = m_carbohydrates > 0 ? 1.0 : 0.0;
    double lipidProgress = m_lipids > 0 ? 1.0 : 0.0;
    double proProgress = m_proteins > 0 ? 1.0 : 0.0;
    double vitProgress = m_vitamins > 0 ? 1.0 : 0.0;
    double resourceOK = (carbProgress + lipidProgress + proProgress + vitProgress) / 4.0;
    return qMin(rateProgress, resourceOK);
}

// 检查是否达成胜利条件（资源和速率均达标）
bool GameResources::checkWinCondition() const {
    // 检查所有资源是否达标
    bool resourcesOK = (m_carbohydrates >= WIN_CARB) &&
                       (m_lipids >= WIN_LIPID) &&
                       (m_proteins >= WIN_PRO) &&
                       (m_vitamins >= WIN_VIT);

    // 检查所有生产速率是否达标
    bool ratesOK = (m_carbRate >= TARGET_CARB_RATE) &&
                   (m_lipidRate >= TARGET_LIPID_RATE) &&
                   (m_proRate >= TARGET_PRO_RATE) &&
                   (m_vitRate >= TARGET_VIT_RATE);

    // 两者都需满足
    return resourcesOK && ratesOK;
}
