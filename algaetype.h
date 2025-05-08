// algaetype.h
#ifndef ALGAETYPE_H
#define ALGAETYPE_H

#include <QString>
#include<QColor>
class AlgaeType {
public:
    enum Type { NONE, TYPE_A, TYPE_B, TYPE_C };


    struct Properties {
        QString name;
        int lightRequiredPlant;
        int lightRequiredMaintain;
        int lightRequiredSurvive;
        int minLight;         // 最低光照需求
        double minNitrogen;   // 新增：最低氮需求
        double minCarbon;     // 新增：最低碳需求
        int plantCostCarb;
        int plantCostLipid;
        int plantCostPro;
        int plantCostVit;
        int shadingDepth;
        int shadingAmount;
        double consumeRateN;
        double consumeRateC;
        double produceRateCarb;
        double produceRateLipid;
        double produceRatePro;
        double produceRateVit;
        QString imagePath;
    };

    static Properties getProperties(Type type);
    static QString getTypeName(Type type);
    static QColor getColor(Type type); // 新增颜色获取方法
    static bool canAfford(Type type, double carb, double lipid, double pro, double vit);
    static void getPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit);
    static void deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit); // 新增声明
};


#endif // ALGAETYPE_H
