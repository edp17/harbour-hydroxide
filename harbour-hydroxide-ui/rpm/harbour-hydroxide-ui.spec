Name:       harbour-hydroxide-ui
Version:    1.0.0
Release:    0
Summary:    UI controller for Hydroxide (Proton Mail bridge) on Sailfish OS
License:    MIT
URL:        https://github.com/emersion/hydroxide
Group:      Applications/Internet

# UI depends on the backend/service package you already built
Requires:   harbour-hydroxide
Requires:   sailfishsilica-qt5 >= 1.0.0
Requires:   qt5-qtdeclarative
Requires:   systemd

BuildRequires:  cmake
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  sailfishsilica-qt5-devel
BuildRequires:  libsailfishapp-devel
BuildRequires:  pkgconfig(Qt5Concurrent)

%description
Simple Sailfish OS UI to control the harbour-hydroxide service and to run
"hydroxide auth" and copy the generated bridge password to clipboard.

%prep
# nothing to unpack (in-tree build)

%build
%cmake .
%cmake_build

%install
%cmake_install

# Ensure standard Sailfish locations (in case CMakeLists differs)
install -d %{buildroot}%{_datadir}/applications
install -d %{buildroot}%{_datadir}/icons/hicolor/256x256/apps

# Desktop file (if not already installed by CMake)
if [ -f harbour-hydroxide-ui.desktop ]; then
  install -m 0644 harbour-hydroxide-ui.desktop %{buildroot}%{_datadir}/applications/
fi

# Icon (if not already installed by CMake)
if [ -f icons/harbour-hydroxide-ui.png ]; then
  install -m 0644 icons/harbour-hydroxide-ui.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/
fi

%files
%{_bindir}/harbour-hydroxide-ui
%{_datadir}/harbour-hydroxide-ui/
%{_datadir}/applications/harbour-hydroxide-ui.desktop
%{_datadir}/icons/hicolor/256x256/apps/harbour-hydroxide-ui.png

%changelog
* Tue Mar 03 2026 Miklós - 0.1.0-1
- Initial UI package