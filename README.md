# 藻类策略游戏（algaeplus）

## 项目简介

本项目是一个基于 Qt 的水藻生态策略游戏，玩家通过合理布局不同类型的藻类，提升资源产量，达成通关条件。游戏具有丰富的策略性和可视化效果，适合教学、科研或兴趣开发。

## 主要功能
- 多种藻类类型，每种具有独特属性和特性
- 资源管理（糖类、脂质、蛋白质、维生素）与生产速率
- 遮光、协同、加速等多种格子特性机制
- 胜利条件与评分系统
- 友好的图形化界面，支持全屏
- 背景音乐与音效

## 编译与运行

### 依赖环境
- Qt 6（推荐 6.4 及以上，需包含 Widgets 和 Multimedia 模块）
- CMake 3.16 及以上
- C++17 编译器（如 MinGW 64-bit、MSVC、GCC 等）

### 编译步骤（以 Windows 为例）
1. 安装 Qt 和 Qt Creator，确保安装了 Widgets、Multimedia 模块。
2. 使用 Qt Creator 打开 `algaeplus` 文件夹下的 `CMakeLists.txt`。
3. 配置合适的编译套件（如 Desktop Qt 6.8.1 MinGW 64-bit）。
4. 点击构建并运行。

或命令行编译：
```bash
cd algaeplus
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH=你的Qt安装路径
cmake --build .
```

## 玩法说明
- 左键点击网格放置选中的藻类，右键点击移除藻类。
- 鼠标悬浮格子可查看当前资源、光照、可否种植等信息。
- ESC 打开菜单，可暂停、重新开始、设置音量等。
- 达成所有资源和生产速率目标即可胜利。

### 藻类类型与特性
- **螺旋藻（A型）**：遮光强，产糖高，同类相邻减产。
- **小球藻（B型）**：提升左右格资源恢复速率，产脂质。
- **小型硅藻（C型）**：产维生素，与B型相邻糖减产。
- **裸藻（D型）**：与A/B/C型相邻时协同增益，产量+20%。
- **蓝藻（E型）**：极低光照生存，为周围格子+4光照。

详细属性可在游戏右侧点击"查看所有藻类属性"按钮查看。

## 主要文件说明
- `main.cpp`：程序入口
- `mainwindow.h/cpp`：主窗口与UI逻辑
- `algaegame.h/cpp`：游戏主逻辑
- `gamegrid.h/cpp`：网格与格子管理
- `algaecell.h/cpp`：单元格（藻类）逻辑
- `algaetype.h/cpp`：藻类类型与属性定义
- `gameresources.h/cpp`：资源管理
- `SoundManager.h/cpp`：音效管理
- `resources.qrc`、`image.qrc`、`sound.qrc`：资源文件

## 资源与素材
- 所有图片、音效均存放于 `../resources/` 目录及其子目录下。
- 资源文件通过 Qt 的 `.qrc` 资源系统管理。

## 贡献与许可
- 本项目仅供学习与交流，欢迎二次开发与改进。
- 如有建议或 bug，欢迎 issue 或 PR。 