
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QLibraryInfo>
#include <QPluginLoader>
#include <QStyleHints>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
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
  a.setApplicationVersion("3.0");
  a.setOrganizationName("Mannes Technology");

  bool isDarkMode = a.styleHints()->colorScheme() == Qt::ColorScheme::Dark;
  if (isDarkMode) {
    // Apply dark theme
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
  MainWindow w;
  w.show();
  return a.exec();
}
