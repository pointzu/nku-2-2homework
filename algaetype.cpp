#include "algaetype.h"

AlgaeType::Properties AlgaeType::getProperties(Type type) {
    Properties props;
    props.name = getTypeName(type);

    switch (type) {
    case NONE:
        props.lightRequiredPlant = 0;
        props.lightRequiredMaintain = 0;
        props.lightRequiredSurvive = 0;
        props.plantCostCarb = props.plantCostLipid = props.plantCostPro = props.plantCostVit = 0;
        props.shadingDepth = 0;
        props.shadingAmount = 0;
        props.consumeRateN = props.consumeRateC = 0;
        props.produceRateCarb = props.produceRateLipid = props.produceRatePro = props.produceRateVit = 0;
        props.imagePath = ":/images/empty.png";
        break;

    case TYPE_A:
        // 小型藻类A
        props.lightRequiredPlant = 10;
        props.lightRequiredMaintain = 8;
        props.lightRequiredSurvive = 5;
        props.plantCostCarb = 10;
        props.plantCostLipid = 0;
        props.plantCostPro = 5;
        props.plantCostVit = 0;
        props.shadingDepth = 1;
        props.shadingAmount = 5;
        props.consumeRateN = 1.0;
        props.consumeRateC = 8.0;
        props.produceRateCarb = 5.0;
        props.produceRateLipid = 0.0;
        props.produceRatePro = 2.0;
        props.produceRateVit = 0.0;
        props.imagePath = ":/images/algae_a.png";
        break;

    case TYPE_B:
        // 小型藻类B
        props.lightRequiredPlant = 12;
        props.lightRequiredMaintain = 10;
        props.lightRequiredSurvive = 6;
        props.plantCostCarb = 8;
        props.plantCostLipid = 6;
        props.plantCostPro = 0;
        props.plantCostVit = 2;
        props.shadingDepth = 2;
        props.shadingAmount = 3;
        props.consumeRateN = 2.0;
        props.consumeRateC = 6.0;
        props.produceRateCarb = 3.0;
        props.produceRateLipid = 4.0;
        props.produceRatePro = 0.0;
        props.produceRateVit = 1.0;
        props.imagePath = ":/images/algae_b.png";
        break;

    case TYPE_C:
        // 小型藻类C
        props.lightRequiredPlant = 8;
        props.lightRequiredMaintain = 6;
        props.lightRequiredSurvive = 4;
        props.plantCostCarb = 5;
        props.plantCostLipid = 0;
        props.plantCostPro = 2;
        props.plantCostVit = 8;
        props.shadingDepth = 0;
        props.shadingAmount = 0;
        props.consumeRateN = 2.0;
        props.consumeRateC = 12.0;
        props.produceRateCarb = 3.0;
        props.produceRateLipid = 0.0;
        props.produceRatePro = 3.0;
        props.produceRateVit = 5.0;
        props.imagePath = ":/images/algae_c.png";
        break;
    }

    return props;
}

QString AlgaeType::getTypeName(Type type) {
    switch (type) {
    case NONE:
        return QString("None");
    case TYPE_A:
        return QString("小型藻类A");
    case TYPE_B:
        return QString("小型藻类B");
    case TYPE_C:
        return QString("小型藻类C");
    default:
        return QString("Unknown");
    }
}

bool AlgaeType::canAfford(Type type, double carb, double lipid, double pro, double vit) {
    Properties props = getProperties(type);

    return carb >= props.plantCostCarb &&
           lipid >= props.plantCostLipid &&
           pro >= props.plantCostPro &&
           vit >= props.plantCostVit;
}

void AlgaeType::getPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit) {
    Properties props = getProperties(type);
    carb = static_cast<double>(props.plantCostCarb);
    lipid = static_cast<double>(props.plantCostLipid);
    pro = static_cast<double>(props.plantCostPro);
    vit = static_cast<double>(props.plantCostVit);
}

void AlgaeType::deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit) {
    // 实现资源扣除逻辑（例如根据藻类类型减去对应资源）
    Properties props = getProperties(type);
    carb -= props.plantCostCarb;
    lipid -= props.plantCostLipid;
    pro -= props.plantCostPro;
    vit -= props.plantCostVit;
}
QColor AlgaeType::getColor(Type type) {
    switch (type) {
    case TYPE_A: return QColor(50, 200, 50);  // 示例绿色
    case TYPE_B: return QColor(50, 50, 200);  // 示例蓝色
    default: return QColor(200, 200, 200);    // 默认灰色（无藻类）
    }
}
