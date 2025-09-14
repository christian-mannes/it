#ifndef JUPYTER_H
#define JUPYTER_H

#if 0
#include <QObject>
#include <QProcess>
#include <QWebEngineView>
#include <QNetworkAccessManager>

class Jupyter : public QObject {
  Q_OBJECT

public:
  explicit Jupyter(QWebEngineView *v, QObject *parent_ = nullptr);

  void startServer(const QString &notebookDir = QString());
  void stopServer();
  void loadNotebook(const QString &notebookPath = QString());

signals:
  void serverReady();
  void serverFailed();
  void serverStopped();

private slots:
  void onJupyterOutput();
  void onJupyterFinished(int exitCode);
  void checkServerReady();

private:
  QWebEngineView *webView;
  QProcess *jupyterProcess;
  QNetworkAccessManager *networkManager;
};

QString findJupyterPath();
bool isJupyterAvailable();
#endif
#endif // JUPYTER_H
