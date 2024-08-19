#include "nametableview.h"

NametableView::NametableView(NametableRenderer &nametable_renderer, QWidget *parent)
    : QWidget(parent)
    , nametable_renderer{nametable_renderer}
{
    setMinimumSize(32*24,32*24);
}

void NametableView::paintEvent(QPaintEvent *event)
{
//    QPainter p{this};
//    nametable_renderer.render(p, width());
//    QPainter::PixmapFragment arr[1];
//    arr[0] = QPainter::PixmapFragment::create(QPointF{0,0},QRectF{0,0,10,10});
//    p.drawPixmapFragments(arr, 1, QPixmap::fromImage(nametable_renderer.image()));
//    p.drawImage(QPoint{0,0},nametable_renderer.image().scaledToWidth(width()));
//    p.fillRect(0,0,10,10,Qt::blue);
//    QTransform transform;
//    transform.scale(3,3);
//    p.setTransform(transform);
//    nametable_renderer.render(p);
//    p.drawImage(0,0,nametable_renderer.image().scaledToWidth(width()));
}
