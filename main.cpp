
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#endif

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QLibraryInfo>
#include <QPluginLoader>
#include <QStyleHints>
#include "mainwindow.h"

#define xstr(a) str(a)
#define str(a) #a

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
#ifdef _WIN32
    if (AllocConsole()) {
      freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
      freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
      freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
      std::cout.clear();
      std::cerr.clear();
      std::cin.clear();
  }
#endif
#ifdef _DEBUG
    qDebug() << "Qt app using DEBUG runtime";
#else
    qDebug() << "Qt app using RELEASE runtime";
#endif
#if 0
  qDebug() << "=== Qt Plugin Debugging ===";
  qDebug() << "Application dir path:" << QCoreApplication::applicationDirPath();
  qDebug() << "Library paths:" << QCoreApplication::libraryPaths();
  qDebug() << "Plugin paths:" << QLibraryInfo::path(QLibraryInfo::PluginsPath);

  // Check if the cocoa plugin file exists and is readable
  QString cocoaPath = QCoreApplication::applicationDirPath() + "/../PlugIns/platforms/libqcocoa.dylib";
  qDebug() << "Cocoa plugin path:" << cocoaPath;
  qDebug() << "Cocoa plugin exists:" << QFile::exists(cocoaPath);

  // Try to load the plugin directly
  QPluginLoader loader(cocoaPath);
  qDebug() << "Plugin metadata:" << loader.metaData();
  qDebug() << "Plugin load error:" << loader.errorString();
#endif
#ifdef Q_OS_MACOS
  QString pluginPath = QCoreApplication::applicationDirPath() + "/../PlugIns";
  QCoreApplication::addLibraryPath(pluginPath);
#endif
  a.setApplicationName("It");
  a.setApplicationDisplayName("It");
  a.setApplicationVersion(xstr(APP_VERSION));
  a.setOrganizationName("Mannes Technology");
#ifdef Q_OS_WIN
  app.setWindowIcon(QIcon(":/icons/icon.ico"));
#endif
#ifdef Q_OS_LINUX
  a.setWindowIcon(QIcon(":/icons/icon_48x48.png"));
  //QIcon appIcon;
  //appIcon.addFile(":/icons/icon_32x32.png", QSize(32,32));
  //appIcon.addFile(":/icons/icon_48x48.png", QSize(48,48));
  //appIcon.addFile(":/icons/icon_64x64.png", QSize(64,64));
  //a.setWindowIcon(appIcon);
#endif
  bool isDarkMode = a.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
  if (isDarkMode) {
    a.setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    a.setPalette(darkPalette);
  }
  MainWindow w(isDarkMode);
  w.show();
  return a.exec();
}
