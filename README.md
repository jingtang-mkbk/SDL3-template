# SDL3 Template

基于 SDL3 回调 API 的 C 项目模板，使用 Git 子模块 + CMake 管理依赖。

## 项目结构

```
.
├── main.c                  # 主程序（SDL3 回调模式）
├── CMakeLists.txt          # CMake 构建脚本
├── three-party/
│   ├── SDL/                # SDL3 核心库        (release-3.4.12)
│   ├── SDL_image/          # 图像加载扩展       (release-3.4.4)
│   ├── SDL_mixer/          # 音频混音扩展       (release-3.2.4)
│   └── SDL_ttf/            # 字体渲染扩展       (release-3.2.2)
├── .gitmodules             # 子模块配置
├── .gitignore              # Git 忽略规则
└── README.md
```

## 拉取项目后要做的事

### 1. 克隆项目（含子模块）

```bash
git clone --recurse-submodules <仓库地址>
```

如果已克隆但没有子模块：

```bash
git submodule update --init --recursive
```

子模块均已锁定在最新的 release tag，无需跟踪分支。

### 2. Windows 环境准备（MinGW）

SDL3 的 OpenGL ES 后端需要 ANGLE 头文件：

```bash
pacman -S mingw-w64-ucrt-x86_64-angleproject
```

### 3. 编译

项目使用 CMake `add_subdirectory` 将 SDL 源码作为子项目编译，**无需手动安装**：

```bash
cmake -S . -B build
cmake --build build
```

如需关闭可选子模块：

```bash
cmake -S . -B build -DUSE_SDL_IMAGE=OFF -DUSE_SDL_MIXER=OFF -DUSE_SDL_TTF=OFF
```

### 4. 运行

```bash
.\build\Debug\main.exe     # Windows (MinGW / MSVC)
./build/main               # Linux / macOS
```

按任意键或关闭窗口退出。

## 关于 main.c

使用 SDL3 的**回调 API**（`SDL_MAIN_USE_CALLBACKS`），无需手写事件循环：

| 回调函数           | 作用                             |
| ------------------ | -------------------------------- |
| `SDL_AppInit()`    | 启动时调用一次，创建窗口和渲染器 |
| `SDL_AppEvent()`   | 事件发生时调用（键盘、退出等）   |
| `SDL_AppIterate()` | 每帧调用，绘制画面               |
| `SDL_AppQuit()`    | 退出时调用一次，清理资源         |

帧率通过 `SDL_HINT_MAIN_CALLBACK_RATE` 固定在 60 FPS，不限显示器刷新率。

## 依赖说明

| 子模块    | 仓库                                                            | 说明                 |
| --------- | --------------------------------------------------------------- | -------------------- |
| SDL       | [libsdl-org/SDL](https://github.com/libsdl-org/SDL)             | 核心图形/输入/音频库 |
| SDL_image | [libsdl-org/SDL_image](https://github.com/libsdl-org/SDL_image) | PNG/JPEG 等图像加载  |
| SDL_mixer | [libsdl-org/SDL_mixer](https://github.com/libsdl-org/SDL_mixer) | 多格式音频播放       |
| SDL_ttf   | [libsdl-org/SDL_ttf](https://github.com/libsdl-org/SDL_ttf)     | TrueType 字体渲染    |

> SDL_image / SDL_mixer / SDL_ttf 为可选依赖，可通过 CMake option 关闭。
