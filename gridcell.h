#ifndef GRIDCELL_H
#define GRIDCELL_H

#include "algaetype.h"

// 定义网格单元格结构
struct GridCell {
    AlgaeType::Type algaeType = AlgaeType::NONE;
    double light = 0.0;
    double nitrogen = 0.0;
    double carbon = 0.0;
};

#endif // GRIDCELL_H 