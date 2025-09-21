#include "colormappreview.h"
#include <QPainter>

ColormapPreview::ColormapPreview(QWidget *parent)
  : QWidget{parent}
{
  colormap = nullptr;
  image = nullptr;
  dirty = true;
  progress = 100;
}

void ColormapPreview::setColormap(Colormap *m) {
  colormap = m;
  dirty = true;
  update();
}

void ColormapPreview::setProgress(int p) {
  progress = p;
  update();
}

void ColormapPreview::paintEvent(QPaintEvent *event) {
  if (progress < 100) {
    QPainter painter(this);
    QBrush brush(QColor(0, 128, 0));
    QRect rect(0, 0, progress * width() / 100, height());
    painter.fillRect(rect, brush);
    return;
  }

  if (colormap == nullptr) return;
  int w = width(), h = height();
  if (image != nullptr) {
    if (image->width() != w || image->height() != h) {
      delete image;
      image = nullptr;
    }
  }
  if (image == nullptr) {
    image = new QImage(w, h, QImage::Format_RGB32);
    dirty = true;
  }

  if (dirty) {
    QPainter painter(this);
    for (int x = 0; x < w; x++) {
      int col = colormap->getColor((double)x / w);
      for (int y = 0; y < h; y++) {
        image->setPixel(x, y, col);
      }
    }
    painter.drawImage(0, 0, *image);
  }
}
