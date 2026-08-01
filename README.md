# SDL3 Template

基于 SDL3 回调 API 的 C 项目模板，使用 Git 子模块 + CMake 管理依赖。

## 项目结构

```
.
├── CMakeLists.txt              # CMake 构建脚本
├── src/
│   ├── main.c                  # 主程序入口（SDL3 回调模式）
│   ├── data.h                  # 公共数据类型和头文件聚合
│   ├── platform/
│   │   └── platform.c/h        # 平台相关（窗口尺寸）
│   ├── manager/
│   │   ├── manager.c/h         # 管理器聚合（统一 init/deinit）
│   │   ├── font_manager.c/h    # 字体管理器（stb_ds 哈希缓存）
│   │   ├── image_manager.c/h   # 图片管理器（stb_ds 哈希缓存）
│   │   └── music_manager.c/h   # 音频管理器（支持多音轨）
│   ├── ui/
│   │   ├── text.c/h            # UI 文字控件
│   │   └── image.c/h           # UI 图片控件
│   ├── scenes/
│   │   ├── main_scene/         # 主场景（Switch 按钮、背景图、BGM）
│   │   └── minesweeper_scene/  # 扫雷场景（JSON 驱动布局）
│   ├── utils/
│   │   └── utils.c/h           # 工具函数（颜色解析）
│   └── assets/                 # 资源文件（图片、字体、音乐、JSON）
├── three-party/
│   ├── SDL/                    # SDL3 核心库        (release-3.4.12)
│   ├── SDL_image/              # 图像加载扩展       (release-3.4.4)
│   ├── SDL_mixer/              # 音频混音扩展       (release-3.2.4)
│   ├── SDL_ttf/                # 字体渲染扩展       (release-3.2.2)
│   ├── cJSON/                  # JSON 解析库
│   └── stb_ds.h                # 单头文件哈希表/动态数组
├── .gitmodules                 # 子模块配置
└── README.md
```

## 架构说明

### 管理器系统（Manager）

统一管理各子系统的生命周期，通过位标志按需初始化：

```c
manager.init(MANAGER_FLAG_FONT | MANAGER_FLAG_IMAGE | MANAGER_FLAG_MUSIC);

// 使用
manager.get_managers()->font_manager->load("font.ttf");
manager.get_managers()->image_manager->load(renderer, "img.png");
manager.get_managers()->music_manager->play("bgm.ogg");

// 退出时自动清理
manager.deinit();
```

### 场景系统（Scene）

基于函数指针的场景切换机制：

| 回调        | 作用         |
| ----------- | ------------ |
| `init()`    | 场景初始化   |
| `event()`   | 事件处理     |
| `iterate()` | 每帧渲染     |
| `deinit()`  | 场景退出清理 |

通过 `state->switch_scene(state, "scene_name")` 切换场景。

### UI 控件

- **Text** — 文字渲染控件，自动缓存纹理，支持字号和颜色设置
- **Image** — 图片渲染控件，从 image_manager 加载纹理，自动获取尺寸

## 拉取项目后要做的事

### 1. 克隆项目（含子模块）

```bash
git clone --recurse-submodules <仓库地址>
```

如果已克隆但没有子模块：

```bash
git submodule update --init --recursive
```

### 2. Windows 环境准备（MSYS2 UCRT64）

```bash
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-freetype
```

### 3. 编译

```bash
/ucrt64/bin/cmake -S . -B build -G "Unix Makefiles"
/ucrt64/bin/cmake --build build -j 12
```

### 4. 运行

```bash
.\build\Debug\main.exe
```

按 Back 按钮返回主场景，关闭窗口退出。

## 依赖说明

| 子模块    | 仓库                                                            | 说明                 |
| --------- | --------------------------------------------------------------- | -------------------- |
| SDL       | [libsdl-org/SDL](https://github.com/libsdl-org/SDL)             | 核心图形/输入/音频库 |
| SDL_image | [libsdl-org/SDL_image](https://github.com/libsdl-org/SDL_image) | PNG/SVG 等图像加载   |
| SDL_mixer | [libsdl-org/SDL_mixer](https://github.com/libsdl-org/SDL_mixer) | 多格式音频播放       |
| SDL_ttf   | [libsdl-org/SDL_ttf](https://github.com/libsdl-org/SDL_ttf)     | TrueType 字体渲染    |
| cJSON     | [DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)         | JSON 解析            |
| stb_ds    | [nothings/stb](https://github.com/nothings/stb)                 | 哈希表/动态数组      |
