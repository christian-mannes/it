#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QProcess>
#include <QLibrary>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QRegularExpression>
#include <QWebEngineView>
#include <QUrl>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Function.h"
#include "paramsmodel.h"
#include "syntaxhighlightercpp.h"
#include "jupyter.h"

///////////////////////////////////////////////////////////////////////////////

#define CODE_TAB 0
#define IMAGE_TAB 1
#define HELP_TAB 2
#define NOTEBOOK_TAB 3

MainWindow *mainWindow = nullptr;
#define DEFAULT_COLORMAP "hot"
#define DEFAULT_FUNCTION_NAME "Mandi"

QString name2file(const QString &name) {
  QString result;
  for (int i = 0; i < name.length(); i++) {
    QChar c = name[i];
    if (c.isLetterOrNumber() || c == '_')
      result += c.toLower();
    else
      result += '_';
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(bool darkMode, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  mainWindow = this;
  ui->setupUi(this);
  function = nullptr;
  colormap = nullptr;
  state = nullptr;
  jupyter = nullptr;

  dylib = nullptr;
  createfun = nullptr;

  // New-style colormaps
  std::vector<std::string> newmaps;
  Colormap::getList(newmaps);
  for (const std::string &name: newmaps) {
    QString s(name.c_str());
    QAction* action = ui->menuColormap->addAction(s);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::on_chooseColormap);
  }
  ui->menuColormap->addSeparator();

  ui->actionAbout->setMenuRole(QAction::AboutRole); // MacOS About menu

  // Params model for parameters
  paramsmodel = new ParamsModel();
  ui->paramsTableView->setModel(paramsmodel);
  ui->paramsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  ui->itView->setFocus();
  connect(ui->itView, &ItView::renderFinished, this, &MainWindow::on_renderFinish);

  // Code editor/errors
  codeHasChanged = false;
  codeHasErrors = false;
  highlighter = new SyntaxHighlighterCPP(ui->codeEditor->document());
  connect(ui->codeEditor, &QPlainTextEdit::textChanged, this, [=]() {
    codeHasChanged = true;
  });
  connect(ui->errorsView, &QPlainTextEdit::cursorPositionChanged, this, [=]() {
    QTextCursor cursor = ui->errorsView->textCursor();
    int lineNumber = cursor.blockNumber(); // 0-based line number
    int displayLineNumber = lineNumber + 1; // 1-based for display
    qDebug() << "User clicked on line:" << displayLineNumber;
    QString line = cursor.block().text();
    qDebug() << "Line text:" << line;
    if (line.startsWith("ITFUN.cpp:")) {
      QRegularExpression regex(R"(:(\d+):(\d+):)");
      QRegularExpressionMatch match = regex.match(line);
      if (match.hasMatch()) {
        int lineno = match.captured(1).toInt() - 1 - 9; // length of prefix
        int column = match.captured(2).toInt() - 1;
        cursor = ui->codeEditor->textCursor();
        ui->codeEditor->setFocus();
        QTextBlock block = ui->codeEditor->document()->findBlockByLineNumber(lineno);
        if (block.isValid()) {
          cursor.setPosition(block.position());
          cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
          ui->codeEditor->setTextCursor(cursor);
          ui->codeEditor->ensureCursorVisible();
        }
      }
    }
  });
  ui->errorsView->hide();
  ui->debugView->hide();
  ui->actionShow_Functions->setChecked(true);
  exportDirectory = QDir::homePath();
  doSaveSettings = true;

  if (darkMode) {
    ui->actionCompile->setIcon(QIcon(":/icons/dark/hammer-outline.svg"));
    ui->actionStart->setIcon(QIcon(":/icons/dark/play-outline.svg"));
    ui->actionStop->setIcon(QIcon(":/icons/dark/close-outline.svg"));
    ui->actionBack->setIcon(QIcon(":/icons/dark/arrow-back-outline.svg"));
    ui->actionImage->setIcon(QIcon(":/icons/dark/image-outline.svg"));
    ui->actionCode->setIcon(QIcon(":/icons/dark/pencil-outline.svg"));
    ui->actionHelp->setIcon(QIcon(":/icons/dark/book-outline.svg"));
    ui->actionCheat_Sheet->setIcon(QIcon(":/icons/dark/help-circle-outline.svg"));
    ui->actionShow_Functions->setIcon(QIcon(":/icons/dark/folder-outline.svg"));
  }

  QTimer::singleShot(0, this, [this]() { // post-ui
    postInit();
  });
}

MainWindow::~MainWindow() {
  delete ui;
  delete highlighter;
}

void MainWindow::postInit() {
  loadSettings();
  if (filesDirectory.isEmpty()) {
    // first-time use, set up files directory and install color maps
    firstTimeUse(true);
  }

  // Classic-style color maps (from files)
  QString path = filesDirectory + "Maps";
  QDir dir(path);
  QStringList files = dir.entryList(QStringList() << "*.map", QDir::Files);
  foreach(QString filename, files) {
    QAction* action = ui->menuColormap->addAction(filename);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::on_chooseColormap);
  }

  treemodel = initFunctionList();
  ui->treeView->setTreeModel(treemodel);
  ui->treeView->expandAll();
  ui->treeView->preferredWidth = 100;
  ui->splitter->setStretchFactor(0, 0);
  ui->splitter->setStretchFactor(1, 1);
  connect(ui->treeView, &TreeView::itemDoubleClicked, this, &MainWindow::treeItemDoubleClicked);
  connect(ui->treeView, &TreeView::itemSelectionChanged, this, &MainWindow::treeSelectionChanged);
  connect(treemodel, &TreeModel::itemMoved, this, &MainWindow::treeItemMoved);
  connect(treemodel, &TreeModel::itemRenamed, this, &MainWindow::treeItemRenamed);
  connect(treemodel, &TreeModel::itemAdded, this, &MainWindow::treeItemAdded);
  connect(treemodel, &TreeModel::itemRemoved, this, &MainWindow::treeItemRemoved);
  ui->treeView->setHeaderHidden(true);


  setColormap(currColormap);
  if (savedFunction.isEmpty()) savedFunction = DEFAULT_FUNCTION_NAME;
  ui->treeView->selectItemByName(savedFunction, true);
  qDebug() << "Files directory:" << filesDirectory;
}

void MainWindow::show_about() {
  QString aboutText = QString(
      "<h2>%1</h2>"
      "<p>Version %2</p>"
      "<p>A Qt application for macOS</p>"
      "<p>Built with Qt %3</p>"
      "<p>Copyright © 2025 My Company</p>"
      "<p><a href='https://www.mycompany.com'>Visit our website</a></p>"
  ).arg(qApp->applicationName())
   .arg(qApp->applicationVersion())
   .arg(qVersion());

  QMessageBox aboutBox(this);
  aboutBox.setWindowTitle("About " + qApp->applicationName());
  aboutBox.setText(aboutText);
  aboutBox.setTextFormat(Qt::RichText);
  aboutBox.setStandardButtons(QMessageBox::Ok);

  // Set the application icon in the about box
  aboutBox.setIconPixmap(windowIcon().pixmap(64, 64));

  aboutBox.exec();
}

void MainWindow::loadSettings() {
  QSettings settings;
  if (settings.contains("currFunction")) savedFunction = settings.value("currFunction").toString();
  if (settings.contains("currColormap")) currColormap = settings.value("currColormap").toString();
  if (settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
  if (settings.contains("windowState")) {
    QByteArray wstate = settings.value("windowState").toByteArray();
    restoreState(wstate);
    if (windowState() & Qt::WindowMaximized) {
      setWindowState(windowState() | Qt::WindowMaximized);
    }
  }
  if (settings.contains("exportDirectory")) exportDirectory = settings.value("exportDirectory").toString();
  if (settings.contains("filesDirectory")) filesDirectory = settings.value("filesDirectory").toString();
}

void MainWindow::saveSettings() {
  if (doSaveSettings) {
    QSettings settings;
    settings.setValue("currFunction", currFunction);
    settings.setValue("currColormap", currColormap);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("isMaximized", isMaximized());
    settings.setValue("exportDirectory", exportDirectory);
    settings.setValue("filesDirectory", filesDirectory);
  }
}

void MainWindow::on_actionClear_Settings_triggered() {
  QSettings settings;
  settings.clear();
  doSaveSettings = false;
  QMessageBox msgBox;
  msgBox.setText("Your settings have been deleted");
  msgBox.exec();
}

void MainWindow::firstTimeUse(bool acceptLegacy) {
  qDebug() << "First time use...";
#ifdef Q_OS_APPLE
  firstTimeUseMac(acceptLegacy);
#endif
#ifdef Q_OS_WIN
  firstTimeUseWin(acceptLegacy);
#endif
}
void MainWindow::firstTimeUseWin(bool acceptLegacy) {

}

void MainWindow::firstTimeUseMac(bool acceptLegacy) {
#ifdef Q_OS_APPLE
  // Do we have the legacy directory ~/Library/Application Support/It ?
  QString legacyPath = QDir::homePath() + "/Library/Application Support/It";
  QDir legacyDir(legacyPath);
  bool doPrompt = true;
  if (legacyDir.exists() && acceptLegacy) { // use it on first time use (acceptLegacy)
    filesDirectory = legacyPath + "/";
    statusBar()->showMessage(QString("Your files directory is %1").arg(filesDirectory));
    doPrompt = false;
  }
  // If not, prompt user for a directory - repeat until we have one
  if (doPrompt) {
    QMessageBox msgBox;
    msgBox.setText("Please select a directory for your function files");
    msgBox.exec();
    while (true) {
      filesDirectory = QFileDialog::getExistingDirectory(this,
          "Please select a directory for your function files",
          QDir::homePath());
      if (filesDirectory.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("You must select a directory for your function files");
        msgBox.exec();
      } else {
        filesDirectory += "/";
        saveSettings();
        break;
      }
    }
  }
  statusBar()->showMessage(QString("Your files directory is %1").arg(filesDirectory));

  // Install color maps
  QFileInfo exe(QApplication::applicationDirPath() + "/It");
  QFileInfo mapsdir(filesDirectory + "maps");
  if (!mapsdir.exists() || mapsdir.lastModified() < exe.lastModified()) {
    qDebug() << "Installing maps";
    QString mapszip = QApplication::applicationDirPath() + "/../Resources/maps.zip";
    QProcess process;
    QStringList args;
    args << "-o" << mapszip << "-d" << filesDirectory;
    process.start("unzip", args);
    process.waitForFinished();
    qDebug() << "Installed maps" << (process.exitCode() == 0 ? "ok" : "FAIL");
  }

  // Install include files for compilation
  QFileInfo itdir(filesDirectory + "it");
  if (!itdir.exists() || itdir.lastModified() < exe.lastModified()) {
    qDebug() << "Installing include files";
    QString itzip = QApplication::applicationDirPath() + "/../Resources/it.zip";
    QProcess process;
    QStringList args;
      args << "-o" << itzip << "-d" << filesDirectory;
      process.start("unzip", args);
      process.waitForFinished();
      qDebug() << "Installed it" << (process.exitCode() == 0 ? "ok" : "FAIL");
  }

  QString nbdirpath = filesDirectory + "notebooks";
  QFileInfo nbdir(nbdirpath);
  if (!nbdir.exists()) {
    QDir fd(filesDirectory);
    fd.mkpath(nbdirpath);
  }

  QString buildpath = filesDirectory + "build";
  QFileInfo builddir(buildpath);
  if (!builddir.exists()) {
    QDir fd(filesDirectory);
    fd.mkpath(buildpath);
  }
#endif
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveSettings();
  // TODO: unsaved changes warning
  event->accept();
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionCode_triggered() { // show image
  ui->stackedWidget->setCurrentIndex(CODE_TAB);
}

void MainWindow::on_actionImage_triggered() { // show code editor
  ui->stackedWidget->setCurrentIndex(IMAGE_TAB);
}

void MainWindow::on_actionHelp_triggered() { // show help
  ui->stackedWidget->setCurrentIndex(HELP_TAB);
  ui->helpView->load(QUrl("https://mannes-tech.com/It/manual.html"));
}

void MainWindow::on_actionNotebook_triggered() { // show notebook
  if (jupyter == nullptr) {
    jupyter = new Jupyter(ui->notebookView, this);
    jupyter->startServer(filesDirectory + "notebooks");
    connect(jupyter, &Jupyter::serverReady, this, &MainWindow::on_jupyterReady);
    connect(jupyter, &Jupyter::serverFailed, this, &MainWindow::on_jupyterFailed);
    ui->stackedWidget->setCurrentIndex(NOTEBOOK_TAB);
  } else {
    ui->stackedWidget->setCurrentIndex(NOTEBOOK_TAB);
    jupyter->loadNotebook("");
  }
}
void MainWindow::on_jupyterReady() {
  qDebug() << "READY";
  jupyter->loadNotebook("");
}
void MainWindow::on_jupyterFailed() {
  qDebug() << "FAILED";
  statusBar()->showMessage("Failed to start Jupyter server. Is it installed?");
}

void MainWindow::on_chooseColormap() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) return;
  QString text = action->text();
  for (QAction* a : ui->menuColormap->actions()) {
    a->setChecked(a == action);
  }
  setColormap(text);
}

void MainWindow::on_actionNew_Function_triggered() {
  bool ok = false;
  QString fname = QInputDialog::getText(this,
    "New Function",
    "Function name:", QLineEdit::Normal,
    "", &ok);
  if (ok && !fname.isEmpty()) {
    QString src = QApplication::applicationDirPath() + "/../Resources/template.txt";
    QString cpp = filesDirectory + name2file(fname) + ".cpp";
    QFile::copy(src, cpp);
    ui->treeView->addItem(fname);
    //treemodel->selectItem(ui->treeView, fname, 0); // will call setFunction
    saveFunctionList();
  }
}

void MainWindow::on_actionSave_triggered() {
  if (currFunction.isEmpty()) return;
  if (!saveCode(currFunction, ui->codeEditor->toPlainText())) {
    QMessageBox msgBox;
    msgBox.setText(QString("Failed to save function %1").arg(currFunction)); // TODO: show why
    msgBox.exec();
    return;
  }
  codeHasChanged = false;
}

void MainWindow::on_actionRevert_to_saved_triggered() {
  if (currFunction.isEmpty()) return;
  codeHasChanged = false;
  codeHasErrors = false;
  QString file = filesDirectory + name2file(currFunction) + ".cpp";
  QFileInfo fileinfo(file);
  if (fileinfo.exists()) {
    QFile codefile(file);
    if (codefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream(&codefile);
      QString contents = stream.readAll();
      ui->codeEditor->setPlainText(contents);
    }
  }
}

void MainWindow::on_actionSave_As_triggered() {
  bool ok = false;
  QString fname = QInputDialog::getText(this,
    "New Function",
    "Function name:", QLineEdit::Normal,
    "", &ok);
  if (ok && !fname.isEmpty()) {
    QString src = filesDirectory + name2file(currFunction) + ".cpp";
    QString cpp = filesDirectory + name2file(fname) + ".cpp";
    QFile::copy(src, cpp);
    ui->treeView->addItem(fname);
    //treemodel->selectItem(ui->treeView, fname, 0); // will call setFunction
    saveFunctionList();
  }
}

void MainWindow::treeItemDoubleClicked(TreeItem* item) {
  qDebug() << "doubleclicked" << item->displayName();
  // double clicking just starts editing/renaming, ignore
}
void MainWindow::treeSelectionChanged(TreeItem* current, TreeItem* previous) {
  qDebug() << "selection" << previous->displayName() << "==>" << current->displayName();
  if (current) {
    QString name = current->displayName();
    if (current->isItem()) {
      setFunction(name, false);
    }
  }
}
void MainWindow::treeItemRenamed(TreeItem* item, const QString& oldName, const QString& newName) {
  if (item->isItem()) {
    QString src = filesDirectory + name2file(oldName) + ".cpp";
    QString dst = filesDirectory + name2file(newName) + ".cpp";
    qDebug() << "renamed" << src << dst;
    if (QFile::rename(src, dst)) {
      saveFunctionList();
    } else {
      qDebug() << "Could not rename";
    }
  } else {
    saveFunctionList();
  }
}
void MainWindow::treeItemMoved(TreeItem* item, TreeItem* oldParent, TreeItem* newParent) {
  saveFunctionList();
  qDebug() << "moved" << item->displayName();
}
void MainWindow::treeItemAdded(TreeItem* item, TreeItem* parent) {
  qDebug() << "added" << item->displayName();
}
void MainWindow::treeItemRemoved(TreeItem* item, TreeItem* parent) {
  QString src = filesDirectory + name2file(item->displayName()) + ".cpp";
  QFile::remove(src);
  qDebug() << "removed" << item->displayName() << src;
  saveFunctionList();
}

void MainWindow::on_actionDelete_triggered() {
  ui->treeView->deleteItem();
}

void MainWindow::on_xmin_le_textChanged(const QString &arg1) {}
void MainWindow::on_xmax_le_textChanged(const QString &arg1) {}
void MainWindow::on_ymin_le_textChanged(const QString &arg1) {}
void MainWindow::on_ymax_le_textChanged(const QString &arg1) {}

void MainWindow::on_pspace_radio_clicked() {
  if (function != nullptr && function->pspace == 0) {
    function = function->other;
    showDefaultCoordinates();
    paramsmodel->setFunction(function);
    ui->paramsTableView->show();
    if (state) state->function = function;
  }
}
void MainWindow::on_dspace_radio_clicked() {
  if (function != nullptr && function->pspace == 1) {
    function = function->other;
    showDefaultCoordinates();
    paramsmodel->setFunction(function);
    ui->paramsTableView->show();
    if (state) state->function = function;
  }
}

void MainWindow::on_actionSet_Files_Folder_triggered() {
  filesDirectory = "";
  firstTimeUse(false);
}


void MainWindow::on_actionShow_Functions_triggered() {
  if (ui->treeView->isHidden()) {
    ui->treeView->setHidden(false);
    ui->actionShow_Functions->setChecked(true);
  } else {
    ui->treeView->setHidden(true);
    ui->actionShow_Functions->setChecked(false);
  }
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionCompile_triggered() {
  codeHasChanged = true;
  setFunction(currFunction, false);
}

void MainWindow::on_actionStart_triggered() {
  if (codeHasChanged) {
    setFunction(currFunction, true); // reload and recompile if necessary
    return;
  }
  start();
}

void MainWindow::start() {
  double xmin, xmax, ymin, ymax;

  if (function == nullptr) return;
  if (colormap == nullptr) return;
  ui->stackedWidget->setCurrentIndex(IMAGE_TAB);

  if (state != nullptr && !ui->itView->selection.isEmpty()) { // auto-zoom
    QRect &sel = ui->itView->selection;
    xmin = state->X(sel.x());
    xmax = state->X(sel.x() + sel.width());
    ymin = state->Y(sel.y() + sel.height());
    ymax = state->Y(sel.y());
    ui->xmin_le->setText(QString::number(xmin));
    ui->xmax_le->setText(QString::number(xmax));
    ui->ymin_le->setText(QString::number(ymin));
    ui->ymax_le->setText(QString::number(ymax));
  } else {
    xmin = ui->xmin_le->text().toDouble();
    xmax = ui->xmax_le->text().toDouble();
    ymin = ui->ymin_le->text().toDouble();
    ymax = ui->ymax_le->text().toDouble();
  }
  ui->itView->selection.setSize(QSize(0, 0));

  int xres = ui->resolution_xres->text().toInt();
  int yres = (int)(xres * (ymax - ymin) / (xmax - xmin));

  if (state != nullptr) {
    history.push_back(state);
  }
  state = new State(function, colormap, xres, yres);
  state->setRange(xmin, xmax, ymin, ymax);
  state->setColormap(colormap);
  state->storeArgs(function);
  state->pspace = function->pspace;
  state->clear();
  function->state = state;

  ui->itView->thumbsize = ui->thumb_slider->value();
  ui->itView->singlethreaded = !ui->multithread_cb->isChecked();
  ui->itView->debug = ui->debug_cb->isChecked();
  ui->itView->annotate = ui->annotate_cb->isChecked();
  ui->itView->sandbox = ui->sandbox_cb->isChecked();

  ui->itView->startRender(function, state, colormap);
  ui->actionStop->setEnabled(false);
  ui->debugView->hide();
}

void MainWindow::on_thumb_slider_actionTriggered(int action) {
  ui->itView->thumbsize = ui->thumb_slider->value();
}

void MainWindow::on_actionStop_triggered() {
  ui->itView->stopRender();
  ui->actionStop->setEnabled(false);
  ui->actionBack->setEnabled(history.size() > 0);
  statusBar()->showMessage("Stopped.");
}

void MainWindow::on_renderFinish() {
  ui->actionStop->setEnabled(false);
  ui->actionBack->setEnabled(history.size() > 0);
  if (ui->itView->debug) {
    if (state->debugLines.size() > 0) {
      for (const std::string &line: state->debugLines) {
        ui->debugView->appendPlainText(QString(line.c_str()));
        qDebug() << line;
      }
      ui->debugView->show();
    }
  }
}

void MainWindow::on_actionBack_triggered() {
  if (ui->stackedWidget->currentIndex() == HELP_TAB) {
    ui->helpView->triggerPageAction(QWebEnginePage::Back);
  } else {
    ui->stackedWidget->setCurrentIndex(IMAGE_TAB);
    if (history.size() > 0) {
      if (state != nullptr) delete state;
      state = history.back();
      history.pop_back();
      ui->itView->restore(function, state, colormap);
      ui->xmin_le->setText(QString::number(state->xmin));
      ui->xmax_le->setText(QString::number(state->xmax));
      ui->ymin_le->setText(QString::number(state->ymin));
      ui->ymax_le->setText(QString::number(state->ymax));
      ui->resolution_xres->setText(QString::number(state->xres));
      ui->resolution_yres->setText(QString::number(state->yres));
      ui->pspace_radio->setChecked(state->pspace == 1); // set other?

      state->restoreArgs(function);
      paramsmodel->setFunction(function);
      ui->paramsTableView->show();
    }
    ui->actionBack->setEnabled(history.size() > 0);
  }
}

void MainWindow::on_slider_res_valueChanged(int value) {
  double xmin = ui->xmin_le->text().toDouble();
  double xmax = ui->xmax_le->text().toDouble();
  double ymin = ui->ymin_le->text().toDouble();
  double ymax = ui->ymax_le->text().toDouble();
  int xres = 50 * (value / 50);
  int yres = (int)(xres * (ymax - ymin) / (xmax - xmin));
  ui->slider_res->setValue(xres);
  ui->resolution_xres->setText(QString::number(xres));
  ui->resolution_yres->setText(QString::number(yres));
}

void MainWindow::on_thumb_slider_valueChanged(int value) {
  int v = 50 * (value / 50);
  ui->thumb_slider->setValue(v);
  ui->itView->thumbsize = v;
}

void MainWindow::on_resolution_xres_textChanged(const QString &arg1)
{
  int xres;
  try {
    xres = arg1.toInt();
    double xmin = ui->xmin_le->text().toDouble();
    double xmax = ui->xmax_le->text().toDouble();
    double ymin = ui->ymin_le->text().toDouble();
    double ymax = ui->ymax_le->text().toDouble();
    int yres = (int)(xres * (ymax - ymin) / (xmax - xmin));
    ui->slider_res->setValue(xres);
    ui->resolution_yres->setText(QString::number(yres));
  } catch (...) {

  }
}

void MainWindow::showDefaultCoordinates() {
  if (function == nullptr) return;
  ui->xmin_le->setText(QString::number(function->defxmin));
  ui->xmax_le->setText(QString::number(function->defxmax));
  ui->ymin_le->setText(QString::number(function->defymin));
  ui->ymax_le->setText(QString::number(function->defymax));
}

void MainWindow::on_actionExport_as_PNG_triggered() { ui->itView->exportToPNG(); }
void MainWindow::on_actionExport_as_SVG_triggered() { ui->itView->exportToSVG(); }
void MainWindow::on_actionExport_as_PDF_triggered() { ui->itView->exportToPDF(); }
void MainWindow::on_actionPrint_triggered() { ui->itView->print();}
void MainWindow::on_actionPrintPreview_triggered() { ui->itView->printPreview();}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::setColormap(const QString &name) {
  QString newColormap = name.isEmpty() ? DEFAULT_COLORMAP : name;
  if (colormap != nullptr) delete colormap;
  currColormap = newColormap;
  colormap = Colormap::getColormapByName(currColormap.toStdString());
  if (colormap == nullptr) {
    colormap = new Colormap();
    //QString path = QDir::homePath() + "/Library/Application Support/It/Maps/" + currColormap;
    QString path = filesDirectory + "Maps/" + currColormap;
    colormap->load(path.toStdString());
  }
  ui->preview->setColormap(colormap);
  ui->itView->setColormap(colormap);
}

TreeModel *MainWindow::initFunctionList() {
  TreeModel *model = new TreeModel(this);
  TreeItem *root = model->rootItem(), *folder = nullptr;

  // Built-in functions (TODO: add some more) - not read from file
  //treemodel->addFolder("Built-In");
  builtin.insert(DEFAULT_FUNCTION_NAME); // add to builtin set
  TreeItem *item = new TreeItem(DEFAULT_FUNCTION_NAME, TreeItem::Item);
  item->readOnly = true;
  root->appendChild(item);

  // Read f_list.txt file
  //QString path = QDir::homePath() + "/Library/Application Support/It/";
  QString path = filesDirectory;
  QString listpath = path + "f_list.txt";
  QFile inputFile(listpath);
  if (inputFile.open(QIODevice::ReadOnly)) {
    QTextStream in(&inputFile);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList pieces = line.split("\\");
      if (pieces.count() < 1) continue;
      if (pieces[0].startsWith("*")) { // folder
        folder = new TreeItem(pieces[0].mid(1), TreeItem::Folder);
        root->appendChild(folder);
      } else if (pieces.count() >= 2) { // name (piece[0]), filename (piece[1])
        QString name = pieces[0];
        QString file = pieces[1];
        if (file.endsWith(".cpp")) file = file.mid(0, file.length() - 4);
        if (file != name2file(name)) { // old style entry - copy file
          QString oldpath = path + file + ".cpp";
          QString newpath = path + name2file(name) + ".cpp";
          if (!QFile::exists(newpath)) {
            if (!QFile::copy(oldpath, newpath)) {
              qDebug() << "Renamed" << oldpath << "to" << newpath;
            } else {
              qDebug() << "Renamed" << oldpath << "to" << newpath;
            }
          }
        }
        TreeItem *item = new TreeItem(name, TreeItem::Item);
        if (folder) folder->appendChild(item); else root->appendChild(item);
      }
    }
    inputFile.close();
  }
  return model;
}

void MainWindow::saveFunctionList_(QTextStream &ts, TreeItem *item) {
  if (item->isFolder()) {
    if (item->parent())
      ts << "*" << item->displayName() << '\n';
    for (TreeItem *child : item->children()) {
      saveFunctionList_(ts, child);
    }
  } else {
    QString data = item->displayName();
    if (!builtin.contains(data))
      ts << data << '\\' << name2file(data) << ".cpp" << '\n';
  }
}

void MainWindow::saveFunctionList() {
  QString path = filesDirectory;
  QString listpath = path + "f_list.txt";
  QFile file(listpath);
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream ts(&file);
    TreeItem *root = treemodel->rootItem();
    saveFunctionList_(ts, root);
  }
  file.close();
}

void MainWindow::setFunction(const QString &newFunction, bool thenStart) {
  if (newFunction.isEmpty()) return;
  if (newFunction == currFunction) {
    if (codeHasChanged) { // recompile sets this to true
      if (!saveCode(newFunction, ui->codeEditor->toPlainText())) {
        QMessageBox msgBox;
        msgBox.setText(QString("Failed to save function %1").arg(newFunction)); // TODO: show why
        msgBox.exec();
        return;
      }
    } else {
      return; // nothing to do
    }
  }

  currFunction = newFunction;

  // Unload current function
  for (State *s: history) delete s;
  state = nullptr;
  history.clear();

  if (function != nullptr) {
    delete function->other;
    delete function;
    function = nullptr;
  }
  if (dylib != nullptr) {
    dylib->unload();
    dylib = nullptr;
  }

  // Load function code
  QString file, fname = name2file(newFunction);
  bool builtin_ = false;
  if (builtin.contains(newFunction)) {
    file = QApplication::applicationDirPath() + "/../Resources/builtin_" + fname + ".txt";
    builtin_ = true;
  } else {
    file = filesDirectory + fname + ".cpp";
  }
  QFileInfo fileinfo(file);
  if (fileinfo.exists()) {
    QFile codefile(file);
    if (codefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream(&codefile);
      QString contents = stream.readAll();
      ui->codeEditor->setPlainText(contents);
      if (!thenStart) ui->stackedWidget->setCurrentIndex(CODE_TAB);
    }
  } else {
    ui->codeEditor->setPlainText("");
    qDebug() << file << "not found";
    return;
  }

  // Compile and load from callback (async)
  QTimer::singleShot(0, this, [this,fname,builtin_,thenStart]() {
    if (!compileAndLoad(fname, builtin_, thenStart)) {
      QMessageBox msgBox;
      msgBox.setText(QString("Failed to load function %1").arg(currFunction));
      msgBox.exec();
    }
  });
}

bool MainWindow::saveCode(const QString &name, const QString &code) {
  QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  //QString path = home + "/Library/Application Support/It/" + name2file(name) + ".cpp";
  QString path = filesDirectory + name2file(name) + ".cpp";
  QFile file(path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    stream << code;
    file.close();
    qDebug() << "File written successfully to" << path; // TODO: popup
    return true;
  } else {
    qDebug() << "Failed to open file for writing:" << file.errorString();
    return false;
  }
}

bool MainWindow::compileAndLoad(const QString &fname, bool builtin_, bool thenStart) {
  qDebug() << "compileAndLoad" << fname; // pass fname2file(fname)
  if (builtin_) {
    function = createBuiltinFunction(currFunction.toStdString());
  } else {
    // TODO: directories on other platforms
    QString file = filesDirectory + fname + ".cpp";
    QString lib = filesDirectory + "build/" + fname + ".dylib";
    QString exe = QApplication::applicationDirPath() + "/It";
    QString comp = QApplication::applicationDirPath() + "/../Resources/compile_macos.sh";

    QFileInfo fileinfo(file);
    QFileInfo libinfo(lib);
    QFileInfo exeinfo(exe);
    codeHasChanged = false;
    codeHasErrors = false;

    int exitCode = 0;
    if (!libinfo.exists() || fileinfo.lastModified() > libinfo.lastModified() || exeinfo.lastModified() > fileinfo.lastModified()) {
      // Must compile
      QProcess proc;
      QStringList args;
      QString exe = QApplication::applicationDirPath() + "/It";
      args << comp << fname << filesDirectory << exe;
      qDebug() << "Will run: bash" << args;
      proc.start("bash", args);
      proc.waitForFinished();
      exitCode = proc.exitCode();
    } else {
      qDebug() << "No need to compile";
    }
    if (exitCode == 0) {
      qDebug() << "Compiled ok";
      dylib = new QLibrary(lib);
      if (dylib->load()) {
        createfun = (CreateFunction)dylib->resolve("_createFunction");
        if (createfun == nullptr) { qDebug() << "Could not resolve function"; return false; }
        function = createfun(1); // param space first
        function->other = createfun(0); // dyn space is other
        function->other->other = function;
      } else {
        qDebug() << "Could not load: " << dylib->errorString();
      }
    } else {
      codeHasErrors = true;
      qDebug() << "Could not compile, code:" << exitCode;
      QString errs = filesDirectory + "build/errors.txt";
      QStringList errors;
      QFile errsfile(errs);
      if (errsfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&errsfile);
        QString errors = stream.readAll();
        ui->errorsView->setPlainText(errors);
        ui->errorsView->show();
      }
      return false;
    }
  }

  function->defaults();
  function->other->defaults();

  int pspace = ui->pspace_radio->isChecked() ? 1 : 0;
  if (function->pspace != pspace) function = function->other;

  showDefaultCoordinates();
  paramsmodel->setFunction(function);
  ui->paramsTableView->show();

  ui->itView->clear();
  statusBar()->showMessage(QString("Loaded %1").arg(currFunction));

  if (thenStart) {
    start();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Help

void MainWindow::on_actionCheat_Sheet_triggered() {
  QMessageBox::information(this, "Cheat Sheet",
    "Mouse Actions:<br/><ul>"
      "<li>Left drag: set selection</li>"
      "<li>Left click inside selection: move selection</li>"
      "<li>Left click outside selection: clear selection</li>"
      "<li>Mouse wheel down: zoom in (click to reset)</li>"
      "<li>Shift-left-drag: thumbnail (in parameter space only)</li>"
      "<li>Alt-left-drag: draw </li>"
      "<li>Keys 1-9: set orbit length</li>"
      "<li>Key 0: reset orbit</li>"
      "<li>Key R: randomlize colors for selection and orbit</li>"
    "</ul>");
}

#define xstr(a) str(a)
#define str(a) #a

void MainWindow::on_actionAbout_triggered() {
  QMessageBox::information(this, "About It",
    QString(
      "<h1>It</h1>"
      "<p>Version: %1</p>"
      "<h3>Design:</h3>"
      "<p>&nbsp;&nbsp;&nbsp;Christian Mannes, Núria Fagella</p>"
      "<h3>Development:</h3>"
      "<p>&nbsp;&nbsp;&nbsp;Christian Mannes</p>"
      "<h3>Web Page:</h3>"
      "<p>&nbsp;&nbsp;&nbsp;https://www.mannes-tech.com/It/</p>"
      "<p><i>Copyright (c) 1998-2025 Mannes Technology. All rights reserved</i></p>").arg(xstr(APP_VERSION)));
}

///////////////////////////////////////////////////////////////////////////////

