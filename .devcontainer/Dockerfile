# .devcontainer/Dockerfile
FROM mcr.microsoft.com/devcontainers/base:noble

ARG VULKAN_SDK_VERSION=1.4.341.1
ENV VULKAN_SDK_VERSION=${VULKAN_SDK_VERSION}

ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем все зависимости одним слоем для оптимизации
RUN apt-get update && apt-get install -y \
    # Toolchain
    build-essential \
    clang \
    clangd \
    clang-format \
    clang-tidy \
    lld \
    llvm \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget \
    xz-utils \
    # SDL3 Wayland deps
    libwayland-dev \
    wayland-protocols \
    libwayland-egl-backend-dev \
    libwayland-client0 \
    libwayland-server0 \
    libwayland-cursor0 \
    libwayland-bin \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libdbus-1-dev \
    libibus-1.0-dev \
    libudev-dev \
    libdecor-0-dev \
    libpipewire-0.3-dev \
    libpulse-dev \
    libasound2-dev \
    libjack-dev \
    libsamplerate0-dev \
    libsndio-dev \
    # SDL3 X11 support
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxfixes-dev \
    libxi-dev \
    libxss-dev \
    libxtst-dev \
    # Дополнительные библиотеки для SDL3
    libaudio-dev \
    libfribidi-dev \
    libthai-dev \
    liburing-dev \
    # Vulkan
    libvulkan1 \
    vulkan-tools \
    libvulkan-dev \
    mesa-vulkan-drivers \
    vulkan-validationlayers \
    spirv-tools \
    glslang-tools \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    libdrm-dev \
    libgbm-dev \
    # Utils
    just \
    htop \
    ripgrep \
    fd-find \
    && rm -rf /var/lib/apt/lists/*

## Vulkan SDK
# Загружаем и устанавливаем Vulkan SDK из tarball
RUN VULKAN_SDK_MAJOR=$(echo ${VULKAN_SDK_VERSION} | cut -d. -f1-3) && \
    VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz" && \
    echo "Downloading Vulkan SDK ${VULKAN_SDK_VERSION}..." && \
    mkdir -p /tmp/vulkan-sdk && \
    wget -q --show-progress -O /tmp/vulkan-sdk.tar.xz "${VULKAN_SDK_URL}" && \
    echo "Extracting Vulkan SDK..." && \
    tar -xf /tmp/vulkan-sdk.tar.xz -C /tmp/vulkan-sdk && \
    mkdir -p ~/vulkan/${VULKAN_SDK_VERSION} && \
    mv /tmp/vulkan-sdk/${VULKAN_SDK_VERSION}/* ~/vulkan/${VULKAN_SDK_VERSION}/ && \
    rm -rf /tmp/vulkan-sdk /tmp/vulkan-sdk.tar.xz && \
    # Устанавливаем права на директорию для пользователя vscode
    mkdir -p /home/vscode/vulkan && \
    cp -r ~/vulkan/${VULKAN_SDK_VERSION} /home/vscode/vulkan/ && \
    chown -R vscode:vscode /home/vscode/vulkan

# Добавляем автоматическую загрузку окружения Vulkan SDK для интерактивных shell
RUN VULKAN_SDK_MAJOR=$(echo ${VULKAN_SDK_VERSION} | cut -d. -f1-3) && \
    echo "# Vulkan SDK ${VULKAN_SDK_VERSION}" >> /home/vscode/.bashrc && \
    echo "source /home/vscode/vulkan/${VULKAN_SDK_VERSION}/setup-env.sh" >> /home/vscode/.bashrc && \
    echo "source /home/vscode/vulkan/${VULKAN_SDK_VERSION}/setup-env.sh" >> /home/vscode/.profile && \
    # Добавляем также для root (на всякий случай)
    mkdir -p /root/vulkan && \
    cp -r /home/vscode/vulkan/${VULKAN_SDK_VERSION} /root/vulkan/ && \
    echo "source /root/vulkan/${VULKAN_SDK_VERSION}/setup-env.sh" >> /root/.bashrc

## Wine AppImage (wow64)
RUN WINE_APPIMAGE_URL="https://github.com/mmtrt/WINE_AppImage/releases/download/continuous-stable/wine-stable_11.0-x86_64.AppImage" && \
    mkdir -p /opt/wine && \
    wget -q --show-progress -O /opt/wine/wine.appimage "${WINE_APPIMAGE_URL}" && \
    chmod +x /opt/wine/wine.appimage && \
    # Извлекаем AppImage для лучшей производительности и доступа из любой папки
    cd /opt/wine && \
    ./wine.appimage --appimage-extract >/dev/null 2>&1 && \
    rm wine.appimage && \
    mv squashfs-root wine && \
    # Создаем симлинки в /usr/local/bin для глобального доступа
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/wine && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/wine64 && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/wineboot && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/winecfg && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/wineserver && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/winepath && \
    ln -sf /opt/wine/wine/AppRun /usr/local/bin/regedit

## fxc из Windows SDK
# Скачиваем NuGet пакет с Windows SDK, извлекаем fxc
RUN FXC_SDK_VERSION="10.0.22621.3233" && \
    mkdir -p /opt/fxc && \
    # Используем NuGet для получения бинарников без полной установки Windows SDK
    wget -q --show-progress -O /tmp/sdk.nupkg "https://www.nuget.org/api/v2/package/Microsoft.Windows.SDK.CPP.x64/${FXC_SDK_VERSION}" && \
    unzip -q /tmp/sdk.nupkg -d /tmp/sdk && \
    # Ищем fxc.exe в распакованном пакете
    find /tmp/sdk -name "fxc.exe" -exec cp {} /opt/fxc/ \; && \
    find /tmp/sdk -name "*.dll" -path "*/x64/*" -exec cp {} /opt/fxc/ \; && \
    rm -rf /tmp/sdk /tmp/sdk.nupkg && \
    # Регистрируем в Wine
    ln -sf /opt/fxc/fxc.exe /usr/local/bin/fxc && \
    chown -R vscode:vscode /opt/fxc

## Настраиваем пользователя и права
RUN usermod -a -G video,audio vscode && \
    chown -R vscode:vscode /home/vscode/vulkan

# Создаём префикс Wine
ENV WINEPREFIX=/home/vscode/.wine