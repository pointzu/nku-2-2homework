#include "gameresources.h"
#include <QtMath>

GameResources::GameResources(QObject* parent) : QObject(parent) {
    reset();
}

void GameResources::addCarbohydrates(double amount) {
    m_carbohydrates += amount;
    emit resourcesChanged();
}

void GameResources::addLipids(double amount) {
    m_lipids += amount;
    emit resourcesChanged();
}

void GameResources::addProteins(double amount) {
    m_proteins += amount;
    emit resourcesChanged();
}

void GameResources::addVitamins(double amount) {
    m_vitamins += amount;
    emit resourcesChanged();
}

void GameResources::subtractCarbohydrates(double amount) {
    m_carbohydrates = qMax(0.0, m_carbohydrates - amount);
    emit resourcesChanged();
}

void GameResources::subtractLipids(double amount) {
    m_lipids = qMax(0.0, m_lipids - amount);
    emit resourcesChanged();
}

void GameResources::subtractProteins(double amount) {
    m_proteins = qMax(0.0, m_proteins - amount);
    emit resourcesChanged();
}

void GameResources::subtractVitamins(double amount) {
    m_vitamins = qMax(0.0, m_vitamins - amount);
    emit resourcesChanged();
}

void GameResources::setCarbRate(double rate) {
    if (m_carbRate != rate) {
        m_carbRate = rate;
        emit productionRatesChanged();
    }
}

void GameResources::setLipidRate(double rate) {
    if (m_lipidRate != rate) {
        m_lipidRate = rate;
        emit productionRatesChanged();
    }
}

void GameResources::setProRate(double rate) {
    if (m_proRate != rate) {
        m_proRate = rate;
        emit productionRatesChanged();
    }
}

void GameResources::setVitRate(double rate) {
    if (m_vitRate != rate) {
        m_vitRate = rate;
        emit productionRatesChanged();
    }
}

void GameResources::update(double deltaTime) {
    // Add resources based on current production rates
    m_carbohydrates += m_carbRate * deltaTime;
    m_lipids += m_lipidRate * deltaTime;
    m_proteins += m_proRate * deltaTime;
    m_vitamins += m_vitRate * deltaTime;

    emit resourcesChanged();
}

void GameResources::reset() {
    // Initial resources
    m_carbohydrates = 50.0;
    m_lipids = 30.0;
    m_proteins = 20.0;
    m_vitamins = 10.0;

    // Initial production rates (0)
    m_carbRate = 0.0;
    m_lipidRate = 0.0;
    m_proRate = 0.0;
    m_vitRate = 0.0;

    emit resourcesChanged();
    emit productionRatesChanged();
}

double GameResources::getWinProgress() const {
    // Calculate resource progress (as percentage)
    double carbProgress = qMin(1.0, m_carbohydrates / WIN_CARB);
    double lipidProgress = qMin(1.0, m_lipids / WIN_LIPID);
    double proProgress = qMin(1.0, m_proteins / WIN_PRO);
    double vitProgress = qMin(1.0, m_vitamins / WIN_VIT);

    // Calculate production rate progress
    double carbRateProgress = qMin(1.0, m_carbRate / TARGET_CARB_RATE);
    double lipidRateProgress = qMin(1.0, m_lipidRate / TARGET_LIPID_RATE);
    double proRateProgress = qMin(1.0, m_proRate / TARGET_PRO_RATE);
    double vitRateProgress = qMin(1.0, m_vitRate / TARGET_VIT_RATE);

    // Total progress is average of all factors
    double resourceProgress = (carbProgress + lipidProgress + proProgress + vitProgress) / 4.0;
    double rateProgress = (carbRateProgress + lipidRateProgress + proRateProgress + vitRateProgress) / 4.0;

    // Overall progress is minimum of both components (must satisfy both)
    return qMin(resourceProgress, rateProgress);
}

bool GameResources::checkWinCondition() const {
    // Check if all resources have reached their win thresholds
    bool resourcesOK = (m_carbohydrates >= WIN_CARB) &&
                       (m_lipids >= WIN_LIPID) &&
                       (m_proteins >= WIN_PRO) &&
                       (m_vitamins >= WIN_VIT);

    // Check if all production rates have reached their targets
    bool ratesOK = (m_carbRate >= TARGET_CARB_RATE) &&
                   (m_lipidRate >= TARGET_LIPID_RATE) &&
                   (m_proRate >= TARGET_PRO_RATE) &&
                   (m_vitRate >= TARGET_VIT_RATE);

    // Must satisfy both conditions
    return resourcesOK && ratesOK;
}
