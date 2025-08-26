#include <QDir>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QProcess>
#include <QLibrary>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QRegularExpression>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Function.h"
#include "paramsmodel.h"
#include "syntaxhighlightercpp.h"
///////////////////////////////////////////////////////////////////////////////

#define IMAGE_TAB 1
#define CODE_TAB 0

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  mainWindow = this;
  ui->setupUi(this);
  function = nullptr;
  colormap = nullptr;
  state = nullptr;

  dylib = nullptr;
  createfun = nullptr;

  // Classic-style color maps (from files)
  QString path = QDir::homePath() + "/Library/Application Support/It/Maps";
  QDir dir(path);
  QStringList files = dir.entryList(QStringList() << "*.map", QDir::Files);
  foreach(QString filename, files) {
    QAction* action = ui->menuColormap->addAction(filename);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::on_choose_colormap_map);
  }
  ui->menuColormap->addSeparator();
  // New-style colormaps
  std::vector<std::string> newmaps;
  Colormap::getList(newmaps);
  for (const std::string &name: newmaps) {
    QString s(name.c_str());
    QAction* action = ui->menuColormap->addAction(s);
    action->setCheckable(true);
    connect(action, &QAction::triggered, this, &MainWindow::on_choose_colormap_fun);
  }

  // Tree model for functions treeview
  treemodel = initFunctionList();
  ui->treeView->setModel(treemodel);
  ui->treeView->show();
  ui->treeView->expandAll();
  connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::on_choose_function);

  // Params model for parameters
  paramsmodel = new ParamsModel();
  ui->paramsTableView->setModel(paramsmodel);
  ui->paramsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  ui->itView->setFocus();
  connect(ui->itView, &ItView::renderFinished, this, &MainWindow::on_render_finish);

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

  QTimer::singleShot(0, this, [this]() { // post-ui
    loadSettings();
    setColormap(currColormap);
    if (savedFunction.isEmpty()) savedFunction = DEFAULT_FUNCTION_NAME;
    treemodel->selectItem(ui->treeView, savedFunction, 0); // will call setFunction
  });
}

MainWindow::~MainWindow() {
  delete ui;
  delete highlighter;
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
  if (settings.contains("currFunction"))
    savedFunction = settings.value("currFunction").toString();
  if (settings.contains("currColormap"))
    currColormap = settings.value("currColormap").toString();

  if (settings.contains("geometry")) {
    restoreGeometry(settings.value("geometry").toByteArray());
  }
  if (settings.contains("windowState")) {
    QByteArray wstate = settings.value("windowState").toByteArray();
    restoreState(wstate);
    if (windowState() & Qt::WindowMaximized) {
      setWindowState(windowState() | Qt::WindowMaximized);
    }
  }
}

void MainWindow::saveSettings() {
  QSettings settings;
  settings.setValue("currFunction", currFunction);
  settings.setValue("currColormap", currColormap);
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.setValue("isMaximized", isMaximized());
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

void MainWindow::on_choose_colormap_map() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) return;
  QString text = action->text();
  for (QAction* a : ui->menuColormap->actions()) {
    a->setChecked(a == action);
  }
  setColormap(text);
}

void MainWindow::on_choose_colormap_fun() {
  QAction* action = qobject_cast<QAction*>(sender());
  if (!action) return;
  QString text = action->text();
  for (QAction* a : ui->menuColormap->actions()) {
    a->setChecked(a == action);
  }
  setColormap(text);
}

void MainWindow::on_choose_function(const QModelIndex& current, const QModelIndex& previous) {
  Q_UNUSED(previous);
  if (current.isValid()) {
    QString name = current.data(Qt::DisplayRole).toString();
    setFunction(name);
  }
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

///////////////////////////////////////////////////////////////////////////////

void MainWindow::on_actionCompile_triggered() {
  codeHasChanged = true;
  setFunction(currFunction);
}

void MainWindow::on_actionStart_triggered() {
  if (codeHasChanged) {
    setFunction(currFunction); // reload and recompile if necessary
  }
  if (function == nullptr) return;
  if (colormap == nullptr) return;
  ui->stackedWidget->setCurrentIndex(IMAGE_TAB);

  double xmin, xmax, ymin, ymax;

  if (!ui->itView->selection.isEmpty()) { // auto-zoom
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
  ui->itView->singlethreaded = ui->singlethreaded_cb->isChecked();
  ui->itView->annotate = ui->annotate_cb->isChecked();
  ui->itView->sandbox = ui->sandbox_cb->isChecked();

  ui->itView->startRender(function, state, colormap);
  ui->actionStop->setEnabled(false);
}

void MainWindow::on_actionStop_triggered() {
  ui->itView->stopRender();
  ui->actionStop->setEnabled(false);
  ui->actionBack->setEnabled(history.size() > 0);
  statusBar()->showMessage("Stopped.");
}

void MainWindow::on_render_finish() {
  ui->actionStop->setEnabled(false);
  ui->actionBack->setEnabled(history.size() > 0);
}

void MainWindow::on_actionBack_triggered() {
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

void MainWindow::on_slider_res_valueChanged(int value)
{
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

void MainWindow::on_resolution_xres_textChanged(const QString &arg1)
{
  int xres, yres;
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

///////////////////////////////////////////////////////////////////////////////

void MainWindow::setColormap(const QString &name) {
  QString newColormap = name.isEmpty() ? DEFAULT_COLORMAP : name;
  if (colormap != nullptr) delete colormap;
  currColormap = newColormap;
  colormap = Colormap::getColormapByName(currColormap.toStdString());
  if (colormap == nullptr) {
    colormap = new Colormap();
    QString path = QDir::homePath() + "/Library/Application Support/It/Maps/" + currColormap;
    colormap->load(path.toStdString());
  }
  ui->preview->setColormap(colormap);
  ui->itView->setColormap(colormap);
}

TreeModel *MainWindow::initFunctionList() {
  TreeModel *treemodel = new TreeModel(QStringList() << "Functions");

  // Built-in functions (TODO: add some more)
  treemodel->addFolder("Built-In");
  builtin.insert(DEFAULT_FUNCTION_NAME);
  treemodel->addEntry(DEFAULT_FUNCTION_NAME);

  // Read f_list.txt file
  QString path = QDir::homePath() + "/Library/Application Support/It/";
  QString listpath = path + "f_list.txt";
  QFile inputFile(listpath);
  if (inputFile.open(QIODevice::ReadOnly)) {
    QTextStream in(&inputFile);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList pieces = line.split("\\");
      if (pieces.count() < 1) continue;
      if (pieces[0].startsWith("*")) { // folder
        treemodel->addFolder(pieces[0].mid(1));
      } else if (pieces.count() >= 2) { // name (piece[0]), filename (piece[1])
        QString name = pieces[0];
        QString file = pieces[1];
        if (file.endsWith(".cpp")) file = file.mid(0, file.length() - 4);
        if (file != name2file(name)) { // old style entry - rename file
          QString oldpath = path + file + ".cpp";
          QString newpath = path + name2file(name) + ".cpp";
          if (!QFile::exists(newpath)) {
            if (!QFile::rename(oldpath, newpath)) {
              qDebug() << "Renamed" << oldpath << "to" << newpath;
            } else {
              qDebug() << "Renamed" << oldpath << "to" << newpath;
            }
          }
        }
        treemodel->addEntry(name);
      }
    }
    inputFile.close();
  }
  return treemodel;
}

void MainWindow::setFunction(const QString &newFunction) {
  qDebug() << "setFunction" << newFunction;
  if (newFunction.isEmpty()) return;
  if (newFunction == currFunction) {
    if (codeHasChanged) { // and has been saved... (autosave, TODO)
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

  // Load new function
  if (builtin.contains(newFunction)) {
    function = createBuiltinFunction(newFunction.toStdString());
    ui->codeEditor->setPlainText("");
    QTimer::singleShot(0, this, [this]() { codeHasChanged = false; });
  } else {
    if (!compileAndLoad(name2file(newFunction))) {
      ui->stackedWidget->setCurrentIndex(CODE_TAB);
      QMessageBox msgBox;
      msgBox.setText(QString("Failed to load function %1").arg(newFunction)); // TODO: show why
      msgBox.exec();
      return;
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
}

bool MainWindow::saveCode(const QString &name, const QString &code) {
  QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString path = home + "/Library/Application Support/It/" + name2file(name) + ".cpp";
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

bool MainWindow::compileAndLoad(const QString &fname) {
  qDebug() << "compileAndLoad" << fname; // pass fname2file(fname)
  // TODO: directories on other platforms
  QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString file = home + "/Library/Application Support/It/" + fname + ".cpp";
  QString lib = home + "/Library/Application Support/It/build/" + fname + ".dylib";
  QString comp = home + "/Code/it3/compile_macos.sh";
  QFileInfo fileinfo(file);
  QFileInfo libinfo(lib);
  codeHasChanged = false;
  codeHasErrors = false;
  if (fileinfo.exists()) {
    QFile codefile(file);
    if (codefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream stream(&codefile);
      QString contents = stream.readAll();
      ui->codeEditor->setPlainText(contents);
      QTimer::singleShot(0, this, [this]() { codeHasChanged = false; });
    }
  } else {
    qDebug() << file << "not found";
    return false;
  }
  int exitCode = 0;
  if (!libinfo.exists() || fileinfo.lastModified() > libinfo.lastModified()) { // must compile, TODO: older than our own executable
    QProcess proc;
    QStringList args;
    args << comp << fname;
    qDebug() << "bash" << args;
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
    return true;
  } else {
    codeHasErrors = true;
    qDebug() << "Could not compile" << exitCode;
    QString errs = home + "/Library/Application Support/It/build/errors.txt";
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

///////////////////////////////////////////////////////////////////////////////

