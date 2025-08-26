#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "Function.h"
#include "Colormap.h"
#include "State.h"
#include "paramsmodel.h"
#include "filesmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLibrary;
class SyntaxHighlighterCPP;
typedef Function *(*CreateFunction)(int pspace);
typedef void (*DestroyFunction)(void);

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void on_actionCode_triggered();
  void on_actionImage_triggered();
  void on_choose_function(const QModelIndex& current, const QModelIndex& previous);
  void on_choose_colormap_map();
  void on_choose_colormap_fun();
  void on_actionStart_triggered();
  void on_actionStop_triggered();
  void on_render_finish();
  void on_slider_res_valueChanged(int value);
  void on_resolution_xres_textChanged(const QString &arg1);
  void on_xmin_le_textChanged(const QString &arg1);
  void on_xmax_le_textChanged(const QString &arg1);
  void on_ymin_le_textChanged(const QString &arg1);
  void on_ymax_le_textChanged(const QString &arg1);
  void on_pspace_radio_clicked();
  void on_dspace_radio_clicked();
  void on_actionBack_triggered();

private slots:
  void on_actionCompile_triggered();
  void show_about();

private:
  Ui::MainWindow *ui;
  void closeEvent(QCloseEvent *event) override;
  void saveSettings();
  void loadSettings();

  SyntaxHighlighterCPP *highlighter;

  QLibrary *dylib;
  CreateFunction createfun;
  TreeModel *initFunctionList();
  bool saveCode(const QString &name, const QString &code);
  bool compileAndLoad(const QString &name);
  void setFunction(const QString &name);
  QString currFunction; // "Mandi"
  QString savedFunction; // "Mandi"
  void showDefaultCoordinates();
  QSet<QString> builtin;
  Function *function;
  bool codeHasChanged;
  bool codeHasErrors;

  QString currColormap;
  Colormap *colormap;
  void setColormap(const QString &name);

  State *state;
  std::vector<State*> history;
  ParamsModel *paramsmodel;
  FilesModel *filesmodel;
  TreeModel *treemodel;
};
#endif // MAINWINDOW_H
