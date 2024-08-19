#include <QPainter>
#include <QMouseEvent>
#include "chr.h"
#include "chrview.h"

CHRView::CHRView(CHR &chr, CHRRenderer &chr_renderer, QWidget *parent)
    : QWidget{parent}
    , chr{chr}
    , chr_renderer{chr_renderer}
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(384,384);
    setSizeIncrement(128,128);
    connect(&chr_renderer, SIGNAL (changed()), this, SLOT (update()));    
}

void CHRView::draw_grids(QPainter &painter)
{
    if (gs != NESQT::GridSize::None) {
        QPen pen{};
        pen.setColor({96,96,96});
        pen.setWidth(0);
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        for (int x = 0; x < width(); x += 8) {
            painter.drawLine(QLine{QPoint{x,0},QPoint{x,static_cast<int>(height())}});
        }
        for (int y = 0; y < height(); y += 8) {
            painter.drawLine(QLine{QPoint{0,y},QPoint{static_cast<int>(width()),y}});
        }
    }
}

void CHRView::set_grid_size(NESQT::GridSize g)
{
    gs = static_cast<NESQT::GridSize>(gs ^ g);
}


void CHRView::paintEvent(QPaintEvent *Event)
{
    QPainter painter{this};
    QTransform transform{};
    painter.setTransform(transform.scale(3,3));
    painter.drawImage(QPoint{0,0},chr_renderer.image());
    draw_grids(painter);

//    chr_renderer.render(painter);
    QPen pen_box{QColor(210,235,52)};
    QPen pen_shade{QColor(105,117,26)};
    painter.setTransform(QTransform{});
    size_t scale = width()/128;
    size_t ty = chr.current_tile() / 16, tx = chr.current_tile() % 16;
    painter.setPen(pen_box);
    painter.drawRect(QRect(scale*tx*8,scale*ty*8,scale*8-1,scale*8-1));
    painter.setPen(pen_shade);
    painter.drawRect(QRect(scale*tx*8+1,scale*ty*8+1,scale*8-3,scale*8-3));
}

void CHRView::mousePressEvent(QMouseEvent *event)
{
    size_t x = event->pos().x(), y = event->pos().y();
    size_t scale = width() / 16;
    size_t tx = x / scale, ty = y / scale;
    if (event->button() == Qt::LeftButton) {
        chr.set_current_tile(ty*16 + tx);
    }
}

void CHRView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
}

