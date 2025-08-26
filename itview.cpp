#include "itview.h"
#include "mainwindow.h"
#include <QPainter>
#include <QRunnable>
#include <QThread>
#include <QMouseEvent>
#include <QApplication>
#include <QStatusBar>

ItView::ItView(QWidget *parent) : QWidget{parent} {
  annotate = false;
  sandbox = false;
  thumbsize = 100;
  singlethreaded = false;
  rendering = false;
  selecting = 0;
  totalPixels = 0;
  zoom = 1.0;
  image = nullptr;
  thumbnail = nullptr;
  thumbstate = nullptr;
  threadPool = QThreadPool::globalInstance();
  cores = std::max(1, QThread::idealThreadCount());
  threadPool->setMaxThreadCount(cores);
  qDebug() << "Cores:" << cores;
  progressTimer = new QTimer(this);
  connect(progressTimer, &QTimer::timeout, this, &ItView::onProgressTimer);
  connect(this, &ItView::renderFinished, this, &ItView::onRenderFinished);
  setMouseTracking(true);
  orbit = 0;
}

ItView::~ItView() {
}

void ItView::clear() {
  if (image) delete image;
  image = nullptr;
  selection = QRect(0, 0, 0, 0);
  function = nullptr;
  if (thumbnail) delete thumbnail;
  thumbnail = nullptr;
  points.clear();
  update();
}

void ItView::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.translate(pan);
  painter.scale(zoom, zoom);

  if (image != nullptr) {
    painter.drawImage(0, 0, *image);
  }

  if (selection.width() * selection.height() != 0) {
    painter.setPen(QPen(QColor::fromRgbF(0, 1, 0), 2));
    painter.drawRect(selection);
  }

  if (rendering.load()) return;

  if (function) {
    double linew = 1.0;
    painter.setPen(QPen(QColor::fromRgbF(1, 1, 1), 2));
    QBrush brush;
    for (Annotation *a: function->annotations) {
      if (a->f == "DrawLine") {
        painter.drawLine(state->invX(a->p0), state->invY(a->p1), state->invX(a->p2), state->invY(a->p3));
      } else if (a->f == "SetLineWidth") {
        linew = a->p0;
      } else if (a->f == "SetStrokeColor") {
        painter.setPen(QPen(QColor((int)a->p0, (int)a->p1, (int)a->p2), linew));
      } else if (a->f == "SetFillColor") {
        brush = QBrush(QColor((int)a->p0, (int)a->p1, (int)a->p2));
      } else if (a->f == "DrawRect") {
        painter.drawRect(state->invX(a->p0), state->invY(a->p1), state->invX(a->p2), state->invY(a->p3));
      } else if (a->f == "FillRect") {
        painter.fillRect(QRect(state->invX(a->p0), state->invY(a->p1), state->invX(a->p2), state->invY(a->p3)), brush);
      } else if (a->f == "DrawEllipseInRect") {
        painter.drawEllipse(QRect(state->invX(a->p0), state->invY(a->p1), state->invX(a->p2), state->invY(a->p3)));
      } else if (a->f == "FillEllipseInRect") {
        // not supported
      } else if (a->f == "SetFont") {
        painter.setFont(QFont(QString(a->s.c_str()), a->p0));
      } else if (a->f == "DrawText") {
        painter.drawText(state->invX(a->p0), state->invY(a->p1), QString(a->s.c_str()));
      }
    }
  }

  if (thumbnail != nullptr) {
    painter.drawImage(0, 0, *thumbnail);
  }
  if (points.count() > 0) {
    int n = points.count();
    painter.setPen(QPen(QColor::fromRgbF(0.5, 1, 0.7), 2));
    for (int i = 1; i < n; i++) {
      painter.drawLine(points[i-1].x(), points[i-1].y(), points[i].x(), points[i].y());
    }
    if (orbit > 0 && state) {
      painter.setPen(QPen(QColor::fromRgbF(0.5, 0.6, 0.2), 2));
      double x = state->X(points[0].x());
      double y = state->Y(points[0].y());
      for (int i = 1; i < n; i++) {
        double x1 = state->X(points[i].x());
        double y1 = state->Y(points[i].y());
        complex z(x, y);
        complex z1(x1, y1);
        for (int j = 0; j < orbit; j++) {
          function->orbit(z);
          function->orbit(z1);
          painter.drawLine(state->invX(z.real()), state->invY(z.imag()), state->invX(z1.real()), state->invY(z1.imag()));
        }
        x = x1;
        y = y1;
      }
    }
  }
}

void ItView::deleteThumbnail() {
  if (thumbnail != nullptr) {
    delete thumbstate; thumbstate = nullptr;
    delete thumbnail; thumbnail = nullptr;
  }
}

void ItView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    zoom = 1.0;
    pan = QPoint(0, 0);
    if (Qt::AltModifier == QApplication::keyboardModifiers()) {
      points.append(event->pos());
    } else if (state != nullptr) {
      if (selecting == 0) {
        selection.setTopLeft(event->pos());
        selection.setSize(QSize(0, 0));
        selecting = 1;
      } else if (selection.contains(event->pos())) {
        selecting = 2;
        seldiff = QPoint(event->pos().x() - selection.left(), event->pos().y() - selection.top());
      } else {
        selecting = 0;
        selection.setSize(QSize(0, 0));
      }
      points.clear();
      update();
    }
  }
}

void ItView::mouseMoveEvent(QMouseEvent *event) {
  mousex = event->pos().x();
  mousey = event->pos().y();
  if ((event->buttons() & Qt::LeftButton)) {
    if (Qt::AltModifier == QApplication::keyboardModifiers()) {
      points.append(event->pos());
      update();
    } else if (state != nullptr) {
      if (selecting == 1) {
        selection.setBottomRight(event->pos());
        selection.setWidth(selection.height() * state->xres / state->yres);
        update();
      } else if (selecting == 2) {
        selection.moveTo(QPoint(mousex-seldiff.x(), mousey-seldiff.y()));
        update();
      }
    }
  } else if (Qt::ShiftModifier == QApplication::keyboardModifiers()) {
    if (function->pspace == 1) {
      if (thumbnail == nullptr) {
        thumbnail = new QImage(thumbsize, thumbsize, QImage::Format_RGB32);
        thumbstate = new State(function->other, colormap, thumbsize, thumbsize);
        thumbstate->getRangeFromFunction();
      }
      function->other->setParameter(state->X(mousex), state->Y(mousey));
      for (int y = 0; y < thumbsize; y++) {
        for (int x = 0; x < thumbsize; x++) {
          double pix = function->other->iterate_(thumbstate->X(x), thumbstate->Y(y));
          thumbnail->setPixel(x, y, colormap->getColor(pix));
        }
      }
      update();
    }
  } else {
    deleteThumbnail();
  }
  if (state) {
    double x = state->X(mousex);
    double y = state->Y(mousey);
    extern MainWindow *mainWindow;
    mainWindow->statusBar()->showMessage(QString("Mouse position: %1, %2").arg(x).arg(y));
  }
  if (orbit > 0 && state) {
    addOrbit();
  }
}

void ItView::addOrbit() {
  double x = state->X(mousex);
  double y = state->Y(mousey);
  complex z(x, y);
  function->ClearAnnotations();
  function->SetStrokeColor(255,127,255);
  for (int i = 0; i < orbit; i++) {
    function->orbit(z);
    function->DrawLine(x, y, z.real(), z.imag());
    x = z.real(); y = z.imag();
  }
  update();
}

void ItView::wheelEvent(QWheelEvent *event) {
  const double scaleFactor = 1.15;
  double factor = (event->angleDelta().y() > 0) ? scaleFactor : 1.0 / scaleFactor;

  QPointF point = event->position();
  QPointF worldPos = (point - pan) / zoom;
  zoom *= factor;
  // Adjust pan offset so the world point stays under the mouse
  pan = point - worldPos * zoom;

  update();
}

void ItView::keyPressEvent(QKeyEvent *event) {
  if (event->isAutoRepeat()) return;
  int keyPressed = event->key();
  if (keyPressed >= 49 && keyPressed < 58) { // 49="1"
    orbit = keyPressed - 48;
    addOrbit();
  } else {
    orbit = 0;
    if (function) function->ClearAnnotations();
    update();
  }
}

////////////////////////// Rendering Algo /////////////////////////////////////

class Tile : public QRunnable {
public:
  int x, y; // top left corner
  int w, h; // dimensions
  int hw, hh, w2, h2;
  Function *fun;  // copy of function - use for thread safety
  ItView *itview;
  int phase;
public:
  Tile(ItView *v, int x_, int y_, int w_, int h_, Function *f) {
    itview = v; fun = f; x = x_; y = y_; w = w_; h = h_;
    hw = w / 2; hh = h / 2; w2 = w - hw; h2 = h - hh;
    phase = 0;
    setAutoDelete(false);
  }
  ~Tile() { if (fun && fun->iscopy) delete fun; }
  void run() override { itview->renderTile(this); }
  inline int size() { return w * h; }
};

void ItView::startRender(Function *function_, State *state_, Colormap *colormap_) {
  function = function_;
  state = state_;
  colormap = colormap_;

  int h = state->getHeight();
  int w = state->getWidth();

  if (rendering.load())
    stopRender();
  rendering = true;

  if (image != nullptr) {
    if (image->width() != w || image->height() != h) {
      delete image;
      image = nullptr;
    }
  }
  if (image == nullptr) {
    image = new QImage(w, h, QImage::Format_RGB32);
  }
  resize(QSize(w, h));
  adjustSize();

  totalPixels = w * h;
  pendingPixels = totalPixels;
  elapsedTimer.start();

  qDebug() << "starting";
  function->setColors();

  if (singlethreaded) {
    Tile tile(this, 0, 0, w, h, function->copy_());
    tile.phase = 4; // calc all directly
    renderTile(&tile);
  } else {
#if 0 // STRIPES
    int th = h / cores;
    for (int y = 0; y < h; y += th) {
      int tile_h = std::min(th, h - y);
      Tile *tile = new Tile(this, 0, y, w, tile_h, function->copy_());
      tile->phase = 4;
      tiles.append(tile);
      threadPool->start(tile);
    }
#else
    const int ini_tile_size = 50;
    for (int y = 0; y < h; y += ini_tile_size) {
      for (int x = 0; x < w; x += ini_tile_size) {
        int tile_w = std::min(ini_tile_size, w - x);
        int tile_h = std::min(ini_tile_size, h - y);
        //qDebug() << "tile" << x << y << tile_w << tile_h;
        Tile *tile = new Tile(this, x, y, tile_w, tile_h, function->copy_());
        tiles.append(tile);
        threadPool->start(tile);
      }
    }
#endif
  }
  progressTimer->start(250);
  image->fill(Qt::GlobalColor::black);
  update();
}

void ItView::onProgressTimer() {
  if (!rendering.load()) progressTimer->stop();
  int pp = pendingPixels.load();
  int percent = 100 - ((100 * pp) / totalPixels);
  emit progressUpdated(percent);
  qDebug() << percent << "% done, pp =" << pp;
  map();
  update();
}

void ItView::stopRender() {
  if (!rendering.load()) return;
  rendering = false;
  progressTimer->stop();
  threadPool->waitForDone();
  for (Tile *tile: tiles) delete tile; tiles.clear();
  map();
  update();
}

void ItView::onRenderFinished() {
  double msec = elapsedTimer.elapsed();
  rendering = false;
  selecting = 0;
  progressTimer->stop();
  //threadPool->waitForDone();
  for (Tile *tile: tiles) delete tile; tiles.clear();
  if (sandbox) function->sandbox();
  if (annotate) function->annotate();
  map();
  update();
  extern MainWindow *mainWindow;
  qDebug() << "finished in " << msec << " msec";
  mainWindow->statusBar()->showMessage(QString("Finished in %1 ms (%2 cores)").arg(msec).arg(singlethreaded ? 1 : cores));
}

// 0---1---+
// |   |   |
// 2-- 3---+
// |   |   |
// +---+---+
void ItView::renderTile(Tile *tile) {
  if (tile->phase == 0) {
    double pix = tile->fun->iterate_(state->X(tile->x), state->Y(tile->y));
    state->setPixelRegion(tile->x, tile->y, pix, tile->w, tile->h);
  } else if (tile->phase == 1) {
    int x = tile->x + tile->hw;
    int y = tile->y;
    int w = tile->w2;
    int h = tile->hh;
    state->setPixelRegion(x, y, tile->fun->iterate_(state->X(x), state->Y(y)), w, h);
  } else if (tile->phase == 2) {
    int x = tile->x;
    int y = tile->y + tile->hh;
    int w = tile->hw;
    int h = tile->h2;
    state->setPixelRegion(x, y, tile->fun->iterate_(state->X(x), state->Y(y)), w, h);
  } else if (tile->phase == 3) {
    int x = tile->x + tile->hw;
    int y = tile->y + tile->hh;
    int w = tile->w2;
    int h = tile->h2;
    state->setPixelRegion(x, y, tile->fun->iterate_(state->X(x), state->Y(y)), w, h);
  } else { // final phase 4
    for (int y = tile->y; y < tile->y + tile->h; y++) {
      int idx = state->getPixelIndex(tile->x, y);
      for (int x = tile->x; x < tile->x + tile->w; x++) {
        if (!state->isSetAt(idx))
          state->setPixelAt(idx, tile->fun->iterate_(state->X(x), state->Y(y)));
        //QObject().thread()->usleep(50); // slow down
        idx++;
      }
    }
    int pp = pendingPixels.fetch_sub(tile->size()) - tile->size();
    //qDebug() << "renderTile" << tile->x << tile->y << tile->w << tile->h << "FULL" << pp;
    if (pp == 0) {
      emit renderFinished();
    }
    return;
  }
  tile->phase++;
  threadPool->start(tile);
}

void ItView::restore(Function *function_, State *state_, Colormap *colormap_) {
  function = function_;
  state = state_;
  colormap = colormap_;
  if (rendering.load())
    stopRender();

  int h = state->getHeight();
  int w = state->getWidth();

  if (image != nullptr) {
    if (image->width() != w || image->height() != h) {
      delete image;
      image = nullptr;
    }
  }
  if (image == nullptr) {
    image = new QImage(w, h, QImage::Format_RGB32);
  }
  resize(QSize(w, h));
  adjustSize();

  map();
  update();
}

void ItView::map() {
  int h = state->getHeight();
  int w = state->getWidth();
  const uchar *bits = image->bits();
  ibits = (int *)bits;
  int idx = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      double pix = state->getPixelAt(idx);
      ibits[idx++] = colormap->getColor(pix); //image->setPixel(x, y, colormap->getColor(pix));
    }
  }
}

void ItView::setColormap(Colormap *colormap_) {
  colormap = colormap_;
  if (rendering.load()) stopRender();
  if (image == nullptr) return;
  map();
  update();
}

///////////////////////////////////////////////////////////////////////////////
