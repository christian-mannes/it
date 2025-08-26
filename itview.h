#ifndef ITVIEW_H
#define ITVIEW_H

#include <QWidget>
#include <QImage>
#include <QThreadPool>
#include <QRunnable>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include "Function.h"
#include "Colormap.h"
#include "State.h"
#include <atomic>

class Tile;

class ItView : public QWidget {
  Q_OBJECT
public:
  explicit ItView(QWidget *parent = nullptr);
  ~ItView();
signals:
  void progressUpdated(int percentage);
  void renderFinished();
public slots:
  void onProgressTimer();
  void onRenderFinished();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
public:
  bool annotate;
  bool sandbox;
  int thumbsize;
  bool singlethreaded;
public:
  void clear();
  void startRender(Function *function, State *state, Colormap *colormap);
  void stopRender();
  void restore(Function *function, State *state, Colormap *colormap);
  void setColormap(Colormap *colormap);
  Tile *getTile();
  void renderTile(Tile *tile);
  void deleteThumbnail();
  void addOrbit();
  QRect selection;
  QPoint seldiff;
  int selecting;
  int cores;
private:
  std::atomic<bool> rendering;
  std::atomic<int> pendingPixels;
  int totalPixels;
  QImage *image;
  int *ibits;
  QImage *thumbnail;
  QThreadPool *threadPool;
  QElapsedTimer elapsedTimer;
  QTimer *progressTimer;
  Function *function;
  State *state;
  State *thumbstate;
  Colormap *colormap;
  int orbit;
  int mousex;
  int mousey;
  double zoom;
  QPointF pan;
  QList<QPoint> points;
  QList<Tile*> tiles;
  void map();
};

#endif // ITVIEW_H
