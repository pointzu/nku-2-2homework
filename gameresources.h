#ifndef GAMERESOURCES_H
#define GAMERESOURCES_H

#include <QObject>

class GameResources : public QObject {
    Q_OBJECT

public:
    explicit GameResources(QObject* parent = nullptr);

    // Resource getters
    double getCarbohydrates() const { return m_carbohydrates; }
    double getLipids() const { return m_lipids; }
    double getProteins() const { return m_proteins; }
    double getVitamins() const { return m_vitamins; }

    // Production rate getters
    double getCarbRate() const { return m_carbRate; }
    double getLipidRate() const { return m_lipidRate; }
    double getProRate() const { return m_proRate; }
    double getVitRate() const { return m_vitRate; }

    // Resource modifiers
    void addCarbohydrates(double amount);
    void addLipids(double amount);
    void addProteins(double amount);
    void addVitamins(double amount);

    void subtractCarbohydrates(double amount);
    void subtractLipids(double amount);
    void subtractProteins(double amount);
    void subtractVitamins(double amount);

    // Production rate modifiers
    void setCarbRate(double rate);
    void setLipidRate(double rate);
    void setProRate(double rate);
    void setVitRate(double rate);

    // Game state
    void update(double deltaTime);
    void reset();

    // Win condition check
    double getWinProgress() const;
    bool checkWinCondition() const;

signals:
    void resourcesChanged();
    void productionRatesChanged();

private:
    // Current resource amounts
    double m_carbohydrates;
    double m_lipids;
    double m_proteins;
    double m_vitamins;

    // Production rates
    double m_carbRate;
    double m_lipidRate;
    double m_proRate;
    double m_vitRate;

    // Win condition thresholds
    const double WIN_CARB = 500.0;
    const double WIN_LIPID = 300.0;
    const double WIN_PRO = 200.0;
    const double WIN_VIT = 100.0;

    // Target production rates for win
    const double TARGET_CARB_RATE = 50.0;
    const double TARGET_LIPID_RATE = 30.0;
    const double TARGET_PRO_RATE = 20.0;
    const double TARGET_VIT_RATE = 10.0;
};

#endif // GAMERESOURCES_H
