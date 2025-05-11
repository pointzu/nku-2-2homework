// algaetype.h
#ifndef ALGAETYPE_H // 防止头文件重复包含
#define ALGAETYPE_H

#include <QString> // Qt字符串类
#include <QColor>  // Qt颜色类

// 藻类类型及属性定义类
class AlgaeType {
public:
    // 藻类类型枚举
    enum Type { NONE, TYPE_A, TYPE_B, TYPE_C, TYPE_D, TYPE_E };

    // 藻类属性结构体
    struct Properties {
        QString name;              // 名称
        int lightRequiredPlant;    // 种植所需光照
        int lightRequiredMaintain; // 维持所需光照
        int lightRequiredSurvive;  // 存活所需最低光照
        int plantCostCarb;         // 种植消耗糖类
        int plantCostLipid;        // 种植消耗脂质
        int plantCostPro;          // 种植消耗蛋白质
        int plantCostVit;          // 种植消耗维生素
        int shadingDepth;          // 遮光深度（几格）
        int shadingAmount;         // 遮光强度
        double consumeRateN;       // 每秒消耗氮
        double consumeRateC;       // 每秒消耗碳
        double produceRateCarb;    // 每秒产糖
        double produceRateLipid;   // 每秒产脂质
        double produceRatePro;     // 每秒产蛋白
        double produceRateVit;     // 每秒产维生素
        QString imagePath;         // 图标路径
        QString hoverImagePath;    // 鼠标悬停图片
        QString selectedImagePath; // 选中图片
        QString cursorImagePath;   // 鼠标指针图片
        QColor shadingColor;       // 遮荫区颜色
    };

    // 获取指定类型的属性
    static Properties getProperties(Type type);
    // 获取类型名称
    static QString getTypeName(Type type);
    // 判断资源是否足够种植
    static bool canAfford(Type type, double carb, double lipid, double pro, double vit);
    // 扣除种植消耗
    static void deductPlantingCost(Type type, double& carb, double& lipid, double& pro, double& vit);
};

#endif // ALGAETYPE_H
