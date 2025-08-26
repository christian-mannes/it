#ifndef COLORMAPPREVIEW_H
#define COLORMAPPREVIEW_H

#include <QWidget>
#include <QImage>
#include "Colormap.h"

class ColormapPreview : public QWidget
{
  Q_OBJECT
public:
  explicit ColormapPreview(QWidget *parent = nullptr);
  void setColormap(Colormap *m);
protected:
  void paintEvent(QPaintEvent *event) override;

signals:

private:
  Colormap *colormap;
  QImage *image;
  bool dirty;
};

#endif // COLORMAPPREVIEW_H
