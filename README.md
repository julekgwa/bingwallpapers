# Bingwallpapers
![screenshot](https://raw.githubusercontent.com/julekgwa/bingwallpapers/script-only/bing.png)


![rpmbuild](https://copr.fedorainfracloud.org/coprs/julekgwa/Bingwallpapers/package/bingwallpapers/status_image/last_build.png)

## Installation 

#### Using Copr

```
sudo dnf copr enable julekgwa/Bingwallpapers 
sudo dnf install -y bingwallpapers
```

#### Using RPM

Download rpm file, [64bit](http://bingwallpapers.lekgoara.com/sources/bingwallpapers-x64.rpm) or for [i386 (32bit)](http://bingwallpapers.lekgoara.com/sources/bingwallpapers-i386.rpm)

Run the following command

```
sudo dnf install -y qt5 wget curl qt-x11
```

Install the application

```
sudo rpm -ivh bingwallpapers-x64.rpm or sudo rpm -ivh bingwallpapers-i386.rpm
```

## Development

[Install Qt 5.7+](https://www.qt.io/download)

Install the following

```
sudo dnf install -y qt5-devel yaml-cpp-devel gcc-c++ curl wget
```


## License

Bingwallpapers is distributed under the terms of the GNU General Public License, version 2 or later.