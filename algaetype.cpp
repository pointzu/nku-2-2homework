#include "algaetype.h" // 藻类类型定义头文件
#include <QColor>        // Qt颜色类

// 获取指定类型的属性
AlgaeType::Properties AlgaeType::getProperties(Type type) {
    Properties props;
    switch (type) {
        case TYPE_A:
            props.name = "螺旋藻 Spirulina";
            props.lightRequiredPlant = 22;      // 种植所需光照
            props.lightRequiredMaintain = 18;   // 维持所需光照
            props.lightRequiredSurvive = 12;    // 存活所需光照
            props.plantCostCarb = 20;           // 种植消耗糖类
            props.plantCostLipid = 0;           // 种植消耗脂质
            props.plantCostPro = 10;            // 种植消耗蛋白质
            props.plantCostVit = 0;             // 种植消耗维生素
            props.shadingDepth = 1;             // 遮光深度
            props.shadingAmount = 8;            // 遮光强度
            props.consumeRateN = 1;             // 每秒消耗氮
            props.consumeRateC = 8;             // 每秒消耗碳
            props.produceRateCarb = 5;          // 每秒产糖
            props.produceRateLipid = 0;         // 每秒产脂质
            props.produceRatePro = 2;           // 每秒产蛋白
            props.produceRateVit = 0;           // 每秒产维生素
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png"; // 图标
            props.hoverImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png";
            props.selectedImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_a.png";
            props.cursorImagePath = "";
            props.shadingColor = QColor(0, 0, 0, 50); // 遮荫色
            break;

        case TYPE_B:
            props.name = "小球藻 Chlorella";
            props.lightRequiredPlant = 18;
            props.lightRequiredMaintain = 14;
            props.lightRequiredSurvive = 10;
            props.plantCostCarb = 16;
            props.plantCostLipid = 12;
            props.plantCostPro = 0;
            props.plantCostVit = 4;
            props.shadingDepth = 2;
            props.shadingAmount = 5;
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
            props.name = "小型硅藻 Cyclotella";
            props.lightRequiredPlant = 12;
            props.lightRequiredMaintain = 8;
            props.lightRequiredSurvive = 6;
            props.plantCostCarb = 10;
            props.plantCostLipid = 0;
            props.plantCostPro = 4;
            props.plantCostVit = 16;
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

        case TYPE_D:
            props.name = "裸藻 Euglena";
            props.lightRequiredPlant = 16;
            props.lightRequiredMaintain = 12;
            props.lightRequiredSurvive = 8;
            props.plantCostCarb = 12;
            props.plantCostLipid = 8;
            props.plantCostPro = 6;
            props.plantCostVit = 6;
            props.shadingDepth = 1;
            props.shadingAmount = 4;
            props.consumeRateN = 1.5;
            props.consumeRateC = 7;
            props.produceRateCarb = 2.5;
            props.produceRateLipid = 1.5;
            props.produceRatePro = 1.5;
            props.produceRateVit = 1.5;
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_d.png";
            props.hoverImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_d.png";
            props.selectedImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_d.png";
            props.cursorImagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_d.png";
            props.shadingColor = QColor(0, 120, 255, 40);
            break;

        case TYPE_E:
            props.name = "蓝藻 Cyanobacteria";
            props.lightRequiredPlant = 8;
            props.lightRequiredMaintain = 4;
            props.lightRequiredSurvive = 2;
            props.plantCostCarb = 10;
            props.plantCostLipid = 6;
            props.plantCostPro = 4;
            props.plantCostVit = 4;
            props.shadingDepth = 0;
            props.shadingAmount = 0;
            props.consumeRateN = 1.0;
            props.consumeRateC = 5.0;
            props.produceRateCarb = 2.0;
            props.produceRateLipid = 1.0;
            props.produceRatePro = 1.0;
            props.produceRateVit = 1.0;
            props.imagePath = ":/resources/st30f0n665joahrrvuj05fechvwkcv10/type_e.png";
            props.hoverImagePath = props.imagePath;
            props.selectedImagePath = props.imagePath;
            props.cursorImagePath = props.imagePath;
            props.shadingColor = QColor(255, 215, 0, 80);
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

// 获取类型名称
QString AlgaeType::getTypeName(Type type) {
    return getProperties(type).name;
}

// 判断资源是否足够种植
bool AlgaeType::canAfford(Type type, double carb, double lipid, double pro, double vit) {
    Properties props = getProperties(type);
    return carb >= props.plantCostCarb &&
           lipid >= props.plantCostLipid &&
           pro >= props.plantCostPro &&
           vit >= props.plantCostVit;
}

// 扣除种植消耗
void AlgaeType::deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit) {
    Properties props = getProperties(type);
    carb -= props.plantCostCarb;
    lipid -= props.plantCostLipid;
    pro -= props.plantCostPro;
    vit -= props.plantCostVit;
}
