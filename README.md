# ZMARBLE
ZMarble is an open source (very barebones) version of the multiply or release vids you may have seen on youtube

# Clone the repo:
```bash
git clone --depth=1 https://github.com/ZDev22/ZMarble
cd ZEngine
```
# Dependencies:
## LINUX:
#### Arch:
```bash
sudo pacman -Syu --noconfirm
sudo pacman -S gcc --noconfirm #C
sudo pacman -S libx11 libxrandr libxcursor xkbcommon mesa --noconfirm #X11
sudo pacman -S vulkan-headers vulkan-tools vulkan-icd-loader --noconfirm #Vulkan
# Intel GPU: sudo pacman -S vulkan-intel --noconfirm
# AMD GPU: sudo pacman -S vulkan-radeon --noconfirm
```
#### Debian:
```bash
sudo apt update
sudo apt install build-essential -y #C
sudo apt install libx11-dev libxrandr-dev libxkbcommon-dev libegl1-mesa-dev libxcursor-dev libxi-dev -y #X11
sudo apt install libvulkan-dev libvulkan1 mesa-vulkan-drivers vulkan-tools -y #Vulkan
```
#### Fedora:
```bash
sudo dnf update -y
sudo dnf install gcc -y #C
sudo dnf install libX11-devel libXrandr-devel libxkbcommon-devel mesa-libEGL-devel libXi-devel libXcursor-devel -y #X11
sudo dnf install vulkan-headers vulkan-tools vulkan-loader-devel -y #Vulkan
# Intel GPU: sudo dnf install vulkan-intel -y
# AMD GPU: sudo dnf install vulkan-radeon -y
```
#### Gentoo:
```bash
root # emerge --sync
sudo emerge --ask sys-devel/gcc -y #C
sudo emerge --ask x11-libs/libX11 x11-libs/libXrandr x11-libs/libXi x11-libs/libXcursor x11-libs/libxkbcommon mesa-libs/mesa -y #X11
sudo emerge --ask media-libs/vulkan-loader dev-util/vulkan-headers media-libs/vulkan-tools -y #Vulkan
# Intel GPU: sudo emerge --ask media-libs/vulkan-intel
# AMD GPU: sudo emerge --ask media-libs/vulkan-radeon
```
#### Leap:
```bash
sudo zypper refresh
sudo zypper update -y
sudo zypper install -t pattern devel_C -y #C
sudo zypper install libX11-devel libXrandr-devel libxkbcommon-devel Mesa-libEGL-devel libXi-devel libXcursor-devel -y #X11
sudo zypper install vulkan-headers vulkan-loader-devel vulkan-tools -y #Vulkan
# Intel GPU: sudo zypper install vulkan-intel -y
# AMD GPU: sudo zypper install vulkan-radeon -y
```
#### Alpine:
```bash
sudo apk update
sudo apk add build-base #C
sudo apk add libx11-dev libxrandr-dev libxkbcommon-dev libxi-dev libxcursor-dev mesa-egl-dev #X11
sudo apk add vulkan-headers vulkan-tools vulkan-loader-dev #Vulkan
# Intel GPU: sudo apk add vulkan-intel
# AMD GPU: sudo apk add vulkan-radeon
```
## MAC (beta):
- Download vulkan from: [https://vulkan.lunarg.com/sdk/home](https://vulkan.lunarg.com/sdk/home)
- Move vulkan into your home folder
- Run the following command to install moltenVK:
```bash
brew install molten-vk
```
## WINDOWS:
- Download the **Windows Installer** from [https://cmake.org/download/](https://cmake.org/download/)
- Download vulkan from: [https://vulkan.lunarg.com/sdk/home](https://vulkan.lunarg.com/sdk/home)
- Download msys2 from [https://www.msys2.org/](https://www.msys2.org/)
- Launch MSYS2 UCRT64 (the yellow one)
- Run the following commands to download a C compiler:
```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-make
pacman -S mingw-w64-ucrt-x86_64-vulkan-headers
pacman -S mingw-w64-ucrt-x86_64-vulkan-loader
pacman -S mingw-w64-ucrt-x86_64-shaderc
```
- Make sure to compile through the MSYS2 UCRT64 terminal

# Included dependencies:
[VULKAN (Graphics)](https://vulkan.lunarg.com/sdk/home)</br>
[RGFW (Window)](https://github.com/ColleagueRiley/RGFW)</br>
[MINIAUDIO (Audio) {Modified}](https://miniaud.io/)</br>
[STB-IMAGE (Image loader) {Modified}](https://github.com/nothings/stb)</br>
[STB-TRUETYPE (font text rasterizer) {Modified}](https://github.com/nothings/stb)

# Build:
```bash
make
```

# Run:
```bash
cd build
./main
```

