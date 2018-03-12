%define name bingwallpapers
%define version 1.0.4
%define build_timestamp %{lua: print(os.date("%Y%m%d"))}

Name:           %{name}
Version:        %{version}
Release:        %{build_timestamp}
Summary:        Set bing wallpaper as a desktop background image

License:        GPLv3+
URL:            bingwallpapers.lekgoara.com
Source0:        https://github.com/julekgwa/bingwallpapers/archive/master.tar.gz#/%{name}-%{version}-%{release}.tar.gz
BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-buildroot

BuildRequires:  qt5
BuildRequires:  qt5-devel
BuildRequires:  gcc-c++
BuildRequires:  yaml-cpp-devel
Requires:       qt5
Requires:       qt-x11
Requires:       curl
Requires:       wget

%description
Download and set bing's wallpaper of the day as a desktop wallpaper and screen lock image.
can set random wallpaper by choosing, an image from the downloaded images.

%global debug_package %{nil}
%define _binaries_in_noarch_packages_terminate_build   0

%prep
%autosetup -n %{name}-master


%build
lupdate-qt5 bingwallpapers.pro
lrelease-qt5 bingwallpapers.pro
qmake-qt5 "QT += qml quick widgets quickwidgets" "RESOURCES += qml.qrc images.qrc"
make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p %{buildroot}/usr/bin/
mkdir -p %{buildroot}/etc/xdg/autostart/
mkdir -p %{buildroot}/usr/share/bingwallpapers
mkdir -p %{buildroot}/usr/share/applications

cp %{_builddir}/%{name}-master/bingwallpapers %{buildroot}/usr/bin/
cp %{_builddir}/%{name}-master/bingwallpapers.png %{buildroot}/usr/share/bingwallpapers
cp %{_builddir}/%{name}-master/bingwallpapers.desktop %{buildroot}/usr/share/applications/
cp %{_builddir}/%{name}-master/bingwallpapers.desktop %{buildroot}/etc/xdg/autostart/

%files
/usr/bin/bingwallpapers
/usr/share/bingwallpapers
/usr/share/applications/bingwallpapers.desktop
/etc/xdg/autostart/bingwallpapers.desktop



%changelog
* Mon Mar 12 2018 Junius LEKGWARA <phutigravel@gmail.com> - 1.0.4
- Enabled HighDpi Scaling

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
