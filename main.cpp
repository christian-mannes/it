#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setApplicationName("It");
  a.setApplicationDisplayName("It");
  a.setApplicationVersion("3.0");
  a.setOrganizationName("Mannes Technology");
  MainWindow w;
  w.show();
  return a.exec();
}
