{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  # Derleme zamanı araçları (CMake, Derleyiciler, Hook'lar) buraya:
  nativeBuildInputs = [
    pkgs.cmake
    pkgs.pkg-config          # Kütüphaneleri bulmak için gerekli olabilir
    pkgs.qt6.wrapQtAppsHook  # <-- EN ÖNEMLİ KISIM: Qt ortam değişkenlerini otomatik ayarlar
  ];

  # Runtime (çalışma zamanı) ve Linklenecek kütüphaneler buraya:
  buildInputs = [
    pkgs.qt6.qtbase
    pkgs.qt6.qtsvg
    #pkgs.qt6.qttools
    #pkgs.qt6.qtwebchannel
    #pkgs.qt6.qtwebengine
    #pkgs.qt6.qt5compat
    pkgs.libGL              # Qt Gui genellikle OpenGL'e ihtiyaç duyar
    #pkgs.zlib
  ];

  # Manuel export yapmana gerek yok, wrapQtAppsHook ve mkShell bunu halleder.
  # Ancak özel bir durum varsa shellHook ekleyebilirsin.
  shellHook = ''
    echo "Qt6 Geliştirme Ortamı Hazır"
  '';
}
