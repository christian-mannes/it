#include "jupyter.h"
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTimer>
#include <QDir>

QString findJupyterPath() {
  QStringList possiblePaths = {
    "/opt/homebrew/bin/jupyter",     // Homebrew Apple Silicon
    "/usr/local/bin/jupyter",        // Homebrew Intel
    "/usr/bin/jupyter",              // System install
    QDir::homePath() + "/.local/bin/jupyter"  // pip user install
  };
  for (const QString &path : possiblePaths) {
    if (QFile::exists(path)) {
      return path;
    }
  }
  return QString();
}

bool isJupyterAvailable() {
  QString jupyterPath = findJupyterPath();
  if (jupyterPath.isEmpty()) {
      return false;
  }
  QProcess process;
  process.start(jupyterPath, QStringList() << "--version");
  process.waitForFinished(3000);
  return process.exitCode() == 0;
}

Jupyter::Jupyter(QWebEngineView *webView_, QObject *parent_) : QObject(parent_), webView(webView_) {
  jupyterProcess = nullptr;
  networkManager = new QNetworkAccessManager(this);
}

void Jupyter::startServer(const QString &notebookDir) {
  if (jupyterProcess && jupyterProcess->state() == QProcess::Running) {
    qDebug() << "Jupyter server already running";
    return;
  }
  jupyterProcess = new QProcess(this);
  QString workDir = notebookDir.isEmpty() ? QDir::homePath() : notebookDir;
  jupyterProcess->setWorkingDirectory(workDir);

  QStringList arguments;
  arguments << "lab"
            << "--no-browser"           // Don't open browser
            << "--port=8888"            // Fixed port
            << "--ip=127.0.0.1"         // Localhost only
            << "--allow-root"           // Allow if running as root
            << "--NotebookApp.token=''" // Disable token for simplicity
            << "--NotebookApp.password=''" // Disable password
            << "--NotebookApp.disable_check_xsrf=True"; // Disable XSRF

  connect(jupyterProcess, &QProcess::readyReadStandardOutput, this, &Jupyter::onJupyterOutput);
  connect(jupyterProcess, &QProcess::finished, this, &Jupyter::onJupyterFinished);

  QString path = findJupyterPath();
  if (path.isEmpty()) {
    qDebug() << "Jupyter is not available";
    emit serverFailed();
    return;
  }
  jupyterProcess->start(path, arguments);

  if (!jupyterProcess->waitForStarted(5000)) {
    qDebug() << "Failed to start Jupyter:" << jupyterProcess->errorString();
    emit serverFailed();
    return;
  }

  // Wait a moment then check if server is ready
  QTimer::singleShot(3000, this, &Jupyter::checkServerReady);
}

void Jupyter::stopServer() {
  if (jupyterProcess && jupyterProcess->state() == QProcess::Running) {
    jupyterProcess->terminate();
    if (!jupyterProcess->waitForFinished(5000)) {
        jupyterProcess->kill();
    }
    delete jupyterProcess;
    jupyterProcess = nullptr;
  }
}

void Jupyter::loadNotebook(const QString &notebookPath) {
  QString url = "http://127.0.0.1:8888/lab";
  if (!notebookPath.isEmpty()) {
    url += "/tree/" + notebookPath;
  }
  webView->load(QUrl(url));
}

void Jupyter::onJupyterOutput() {
  QByteArray data = jupyterProcess->readAllStandardOutput();
  QString output = QString::fromUtf8(data);
  qDebug() << "Jupyter:" << output;

  // Look for server ready message
  if (output.contains("is available at") || output.contains("Use Control-C")) {
    emit serverReady();
  }
}

void Jupyter::onJupyterFinished(int exitCode) {
  qDebug() << "Jupyter process finished with code:" << exitCode;
  emit serverStopped();
}

void Jupyter::checkServerReady() {
  // Test if server is responding
  QNetworkRequest request(QUrl("http://127.0.0.1:8888/api/status"));
  QNetworkReply *reply = networkManager->get(request);

  connect(reply, &QNetworkReply::finished, [this, reply]() {
    if (reply->error() == QNetworkReply::NoError) {
      qDebug() << "Jupyter server is ready";
      emit serverReady();
    } else {
      qDebug() << "Server not ready, retrying...";
      QTimer::singleShot(1000, this, &Jupyter::checkServerReady);
    }
    reply->deleteLater();
  });
}
