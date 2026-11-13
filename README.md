# Bingwallpapers

Downloads Bing’s wallpaper of the day and sets it as a desktop wallpaper or a lock screen image.
The program can rotate over pictures in its directory in set intervals as well as delete old pictures after a set amount of time.

![Bingwallpapers window](docs/screenshot.jpg)


![rpmbuild](https://copr.fedorainfracloud.org/coprs/julekgwa/Bingwallpapers/package/bingwallpapers/status_image/last_build.png)

## Installation

#### Using Copr

```
sudo dnf copr enable julekgwa/Bingwallpapers
sudo dnf install -y bingwallpapers
```

## Usage

Bingwallpapers lives in the system tray and refreshes the wallpaper every day. Open its window
from the tray icon to choose the region, picture quality (Ultra HD 4K or Full HD), rotation and
download folder.

On GNOME there is no tray by default; install the AppIndicator extension to get one:

```
sudo dnf install -y gnome-shell-extension-appindicator
```

Without a tray, closing the window still leaves the app running in the background, so the
daily refresh keeps working. Start Bingwallpapers again to show the window, and use **Quit**
in the window to stop it.

Command line options (run `bingwallpapers --help` for the full list):

```
bingwallpapers --set          # download and set today's wallpaper
bingwallpapers --rotate       # switch to a random saved picture
bingwallpapers --4k off       # turn Ultra HD (4K) off; use "on" to turn it back on
bingwallpapers --clean 7      # remove pictures older than 7 days
```

## Development

Requires Qt 6.5+, CMake 3.21+ and yaml-cpp.

```
sudo dnf install -y cmake gcc-c++ qt6-qtbase-devel qt6-qtdeclarative-devel yaml-cpp-devel
```

Build and run

```
cmake -S src -B build
cmake --build build
./build/bingwallpapers
```

Run the tests

```
ctest --test-dir build --output-on-failure
```

Command line options: `bingwallpapers --help`

## License

Bingwallpapers is distributed under the terms of the GNU General Public License, version 2 or later.
