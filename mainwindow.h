#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "Function.h"
#include "Colormap.h"
#include "State.h"
#include "paramsmodel.h"
#include "tree.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Jupyter;
class QLibrary;
class SyntaxHighlighterCPP;
typedef Function *(*CreateFunction)(int pspace);
typedef void (*DeleteFunction)(void*);

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(bool dark, QWidget *parent = nullptr);
  ~MainWindow();
  void postInit();

public slots:
  void show_about();
  void on_actionCode_triggered();
  void on_actionImage_triggered();
  void treeItemDoubleClicked(TreeItem* item);
  void treeSelectionChanged(TreeItem* current, TreeItem* previous);
  void treeItemRenamed(TreeItem* item, const QString& oldName, const QString& newName);
  void treeItemMoved(TreeItem* item, TreeItem* oldParent, TreeItem* newParent);
  void treeItemAdded(TreeItem* item, TreeItem* parent);
  void treeItemRemoved(TreeItem* item, TreeItem* parent);

  void on_chooseColormap();
  void on_actionStart_triggered();
  void on_actionStop_triggered();
  void on_renderProgress(int p);
  void on_renderFinish();
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
  void on_actionExport_as_PNG_triggered();
  void on_actionExport_as_SVG_triggered();
  void on_actionExport_as_PDF_triggered();
  void on_actionPrint_triggered();
  void on_actionPrintPreview_triggered();
  void on_actionSet_Files_Folder_triggered();
  void on_actionClear_Settings_triggered();
  void on_thumb_slider_actionTriggered(int action);
  void on_actionCheat_Sheet_triggered();
  void on_actionHelp_triggered();
  void on_actionNotebook_triggered();
  void on_jupyterReady();
  void on_jupyterFailed();
  void on_actionNew_Function_triggered();
  void on_actionSave_triggered();
  void on_actionSave_As_triggered();
  void on_actionRevert_to_saved_triggered();
  void on_actionDelete_triggered();

  void on_thumb_slider_valueChanged(int value);

  void on_actionShow_Functions_triggered();

  void on_actionAbout_triggered();

  void on_resolution_xres_editingFinished();

  void on_thumbnailCheckBox_checkStateChanged(const Qt::CheckState &arg1);

  void on_thumnailAcceptButton_clicked();

private:
  Ui::MainWindow *ui;
  void closeEvent(QCloseEvent *event) override;
  void saveSettings();
  void loadSettings();
  void firstTimeUse(bool acceptLegacy);
  void firstTimeUseMac(bool acceptLegacy);
  void firstTimeUseWin(bool acceptLegacy);
  void firstTimeUseLinux(bool acceptLegacy);

  SyntaxHighlighterCPP *highlighter;

  QLibrary *dylib;
  QMap<QString,int> ver;
  CreateFunction createfun;
  DeleteFunction deletefun;

  TreeModel *initFunctionList();
  void start();
  void saveFunctionList();
  void saveFunctionList_(QTextStream &ts, TreeItem *item);
  bool saveCode(const QString &name, const QString &code);
  bool compileAndLoad(const QString &name, bool builtin_, bool thenStart);
  void setFunction(const QString &name, bool thenStart);
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
  TreeModel *treemodel;

  Jupyter *jupyter;
public:
  QString filesDirectory;
  QString resourceDirectory;
  QString exportDirectory;
  bool doSaveSettings;
};
#endif // MAINWINDOW_H
