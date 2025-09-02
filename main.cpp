
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QLibraryInfo>
#include <QPluginLoader>

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
  MainWindow w;
  w.show();
  return a.exec();
}
