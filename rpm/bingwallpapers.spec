%define name bingwallpapers
%define version %{lua: print(os.date("%Y.%m.%d"))}

Name:           %{name}
Version:        %{version}
Release:        1%{?dist}
Summary:        Set bing wallpaper as a desktop background image

License:        GPLv2+
URL:            bingwallpapers.lekgoara.com
Source0:        https://github.com/julekgwa/bingwallpapers/archive/master.tar.gz#/%{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.21
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel >= 6.5
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  yaml-cpp-devel
Requires:       qt6-qtbase >= 6.5
Requires:       qt6-qtdeclarative

%description
Downloads Bing’s wallpaper of the day and sets it as a desktop wallpaper or a lock screen image.
The program can rotate over pictures in its directory in set intervals as well as delete old pictures after a set amount of time.

%prep
%autosetup -n %{name}-master

%build
%global _vpath_srcdir src
%cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
%cmake_build

%install
%cmake_install
mkdir -p %{buildroot}/etc/xdg/autostart/
# Autostart keeps the window hidden in the system tray.
sed 's|^Exec=.*|Exec=/usr/bin/bingwallpapers --background|' \
    src/bingwallpapers.desktop > %{buildroot}/etc/xdg/autostart/bingwallpapers.desktop

%files
%license LICENSE
/usr/bin/bingwallpapers
/usr/share/bingwallpapers
/usr/share/applications/bingwallpapers.desktop
/etc/xdg/autostart/bingwallpapers.desktop

%changelog
* Fri Nov 13 2026 Junius LEKGWARA <phutigravel@gmail.com> - 2026.11
- Ported to Qt 6 and CMake
- Replaced the downloaded bing.sh helper script with built-in code; curl and wget are no longer needed
- Downloads the Ultra HD (4K) picture when Bing has one; new "Quality" switch to turn it off
- Also sets the dark-mode wallpaper on GNOME 42+, and supports KDE, Cinnamon and MATE backgrounds
- Redesigned the interface: wallpaper preview, tabs, light and dark themes
- Refreshing no longer freezes the window
- Fixed the next refresh time when the daily time had already passed
- Fixed deleting the wallpaper folder when no wallpaper was set
- Starting a second copy now shows the running window

* Sun Mar 8 2020 Junius LEKGWARA <phutigravel@gmail.com> - 2020-03-8
- updated bingwallpaper home page

* Tue May 1 2018 Junius LEKGWARA <phutigravel@gmail.com> - 2018.05
- display wallpaper copyright info, in a dialog box

* Mon Apr 2 2018 Junius LEKGWARA <phutigravel@gmail.com> - 2018.04
- Added network detection, when the user connects to the internet, the script will run.

* Fri Mar 30 2018 Junius LEKGWARA <phutigravel@gmail.com> - 2018.03
- Added command line support

* Mon Mar 12 2018 Junius LEKGWARA <phutigravel@gmail.com> - 1.0.4
- Enabled HighDpi Scaling
- Added auto daily refresh

* Wed Jan 31 2018 Junius LEKGWARA <phutigravel@gmail.com> - 1.0.3
- added custom download paths, user can select download directory of their choice

* Fri Jan 26 2018 Junius LEKGWARA <phutigravel@gmail.com> - 1.0.2
- added icons to system tray menu

* Thu Jan 25 2018 Junius LEKGWARA <phutigravel@gmail.com> - 1.0.1
- added window maximum width and height
- removed yaml-cpp library files and added it as a build require
- added refresh button to the start screen

* Fri Dec 29 2017 Junius LEKGWARA <phutigravel@gmail.com> - 1.0
- First Bingwallpapers release
