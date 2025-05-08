#include "algaetype.h"
#include <QColor>

AlgaeType::Properties AlgaeType::getProperties(Type type) {
    Properties props;
    switch (type) {
        case TYPE_A:
            props.name = "小型藻类 A";
            props.lightRequiredPlant = 10;
            props.lightRequiredMaintain = 8;
            props.lightRequiredSurvive = 5;
            props.plantCostCarb = 10;
            props.plantCostLipid = 0;
            props.plantCostPro = 5;
            props.plantCostVit = 0;
            props.shadingDepth = 1;
            props.shadingAmount = 5;
            props.consumeRateN = 1;
            props.consumeRateC = 8;
            props.produceRateCarb = 5;
            props.produceRateLipid = 0;
            props.produceRatePro = 2;
            props.produceRateVit = 0;
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png";
            props.hoverImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png";
            props.selectedImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png";
            props.cursorImagePath = "";
            props.shadingColor = QColor(0, 0, 0, 50);
            break;

        case TYPE_B:
            props.name = "小型藻类 B";
            props.lightRequiredPlant = 12;
            props.lightRequiredMaintain = 10;
            props.lightRequiredSurvive = 6;
            props.plantCostCarb = 8;
            props.plantCostLipid = 6;
            props.plantCostPro = 0;
            props.plantCostVit = 2;
            props.shadingDepth = 2;
            props.shadingAmount = 3;
            props.consumeRateN = 2;
            props.consumeRateC = 6;
            props.produceRateCarb = 3;
            props.produceRateLipid = 4;
            props.produceRatePro = 0;
            props.produceRateVit = 1;
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_b.png";
            props.hoverImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_b.png";
            props.selectedImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_b.png";
            props.cursorImagePath = "";
            props.shadingColor = QColor(0, 0, 0, 30);
            break;

        case TYPE_C:
            props.name = "小型藻类 C";
            props.lightRequiredPlant = 8;
            props.lightRequiredMaintain = 6;
            props.lightRequiredSurvive = 4;
            props.plantCostCarb = 5;
            props.plantCostLipid = 0;
            props.plantCostPro = 2;
            props.plantCostVit = 8;
            props.shadingDepth = 0;
            props.shadingAmount = 0;
            props.consumeRateN = 2;
            props.consumeRateC = 12;
            props.produceRateCarb = 3;
            props.produceRateLipid = 0;
            props.produceRatePro = 3;
            props.produceRateVit = 5;
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_c.png";
            props.hoverImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_c.png";
            props.selectedImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_c.png";
            props.cursorImagePath = "";
            props.shadingColor = QColor(0, 0, 0, 0);
            break;

        default:
            props.name = "";
            props.lightRequiredPlant = 0;
            props.lightRequiredMaintain = 0;
            props.lightRequiredSurvive = 0;
            props.plantCostCarb = 0;
            props.plantCostLipid = 0;
            props.plantCostPro = 0;
            props.plantCostVit = 0;
            props.shadingDepth = 0;
            props.shadingAmount = 0;
            props.consumeRateN = 0;
            props.consumeRateC = 0;
            props.produceRateCarb = 0;
            props.produceRateLipid = 0;
            props.produceRatePro = 0;
            props.produceRateVit = 0;
            props.imagePath = "";
            props.hoverImagePath = "";
            props.selectedImagePath = "";
            props.cursorImagePath = "";
            props.shadingColor = QColor(0, 0, 0, 0);
            break;
    }
    return props;
}

QString AlgaeType::getTypeName(Type type) {
    return getProperties(type).name;
}

bool AlgaeType::canAfford(Type type, double carb, double lipid, double pro, double vit) {
    Properties props = getProperties(type);
    return carb >= props.plantCostCarb &&
           lipid >= props.plantCostLipid &&
           pro >= props.plantCostPro &&
           vit >= props.plantCostVit;
}

void AlgaeType::deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit) {
    Properties props = getProperties(type);
    carb -= props.plantCostCarb;
    lipid -= props.plantCostLipid;
    pro -= props.plantCostPro;
    vit -= props.plantCostVit;
}
