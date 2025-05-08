// algaetype.h
#ifndef ALGAETYPE_H
#define ALGAETYPE_H

#include <QString>
#include <QColor>

class AlgaeType {
public:
    enum Type { NONE, TYPE_A, TYPE_B, TYPE_C };

    struct Properties {
        QString name;
        int lightRequiredPlant;
        int lightRequiredMaintain;
        int lightRequiredSurvive;
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
        QString hoverImagePath;    // 鼠标悬停时的图片
        QString selectedImagePath; // 选中时的图片
        QString cursorImagePath;   // 鼠标指针图片
        QColor shadingColor;       // 遮荫区域的颜色
    };

    static Properties getProperties(Type type);
    static QString getTypeName(Type type);
    static bool canAfford(Type type, double carb, double lipid, double pro, double vit);
    static void deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit);
};

#endif // ALGAETYPE_H
