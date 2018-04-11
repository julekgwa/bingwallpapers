# Bingwallpapers

Downloads Bing’s wallpaper of the day and sets it as a desktop wallpaper or a lock screen image.
The program can rotate over pictures in its directory in set intervals as well as delete old pictures after a set amount of time.

![screenshot](https://raw.githubusercontent.com/julekgwa/bingwallpapers/script-only/bing.png)


![rpmbuild](https://copr.fedorainfracloud.org/coprs/julekgwa/Bingwallpapers/package/bingwallpapers/status_image/last_build.png)

## Installation

#### Using Copr

```
sudo dnf copr enable julekgwa/Bingwallpapers
sudo dnf install -y bingwallpapers
```

#### Using RPM

Download rpm file, [bingwallpapers](http://bingwallpapers.lekgoara.com/sources/bingwallpapers.noarch.rpm)

Install the application

```
$ sudo dnf install bingwallpapers.noarch.rpm
```

## Development

[Install Qt 5.7+](https://www.qt.io/download)

Install the following

```
sudo dnf install -y qt5-devel yaml-cpp-devel gcc-c++ curl wget
```


## License

Bingwallpapers is distributed under the terms of the GNU General Public License, version 2 or later.
