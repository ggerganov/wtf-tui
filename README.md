# wtf-tui

[![Actions Status](https://github.com/ggerganov/wtf-tui/workflows/CI/badge.svg)](https://github.com/ggerganov/wtf-tui/actions)
[![wtf-tui on Snapcraft](https://snapcraft.io/wtf-tui/badge.svg)](https://snapcraft.io/wtf-tui)
[![Snap Status](https://build.snapcraft.io/badge/ggerganov/wtf-tui.svg)](https://build.snapcraft.io/user/ggerganov/wtf-tui)
[![wtf-tui v0.1 badge][changelog-badge]][changelog]
[![MIT License Badge][license-badge]][license]

Easily create [WTF](https://wtfutil.com) configurations in your terminal

[![wtf-tui-demo](https://asciinema.org/a/VUKWZM70PxRCHueyPFXy9smU8.svg)](https://asciinema.org/a/VUKWZM70PxRCHueyPFXy9smU8)

## Details

This is a simple tool that facilitates creating YAML configurations for the [WTF terminal dashboard](https://wtfutil.com). The text-based UI is implemented with [ImTui](https://github.com/ggerganov/imtui). All of the originally supported WTF modules are available for configuring. Also, the UI allows easy positioning and resizing of the created modules.

## Installing

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-black.svg)](https://snapcraft.io/wtf-tui)

### Linux

```bash
sudo snap install wtf-tui
```

## Building from source

### Linux and Mac:

```bash
git clone https://github.com/ggerganov/wtf-tui --recursive
cd wtf-tui
mkdir build && cd build
cmake ..
make

./bin/wtfutil-tui
```

### Emscripten:

```bash
git clone https://github.com/ggerganov/wtf-tui --recursive
cd wtf-tui
mkdir build && cd build
emconfigure cmake ..
make
```

## Live demo in the browser

Emscripten port: [wtf-tui.ggerganov.com](https://wtf-tui.ggerganov.com/) *(not suitable for mobile devices)*

[changelog]: ./CHANGELOG.md
[changelog-badge]: https://img.shields.io/badge/changelog-wtf-tui%20v0.1-dummy
[license]: ./LICENSE
[version-badge]: https://img.shields.io/badge/version-0.1-blue.svg
[license-badge]: https://img.shields.io/badge/license-MIT-blue.svg
