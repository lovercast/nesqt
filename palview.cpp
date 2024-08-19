#include <QPainter>
#include <QMouseEvent>
#include "palview.h"

PALView::PALView(QWidget *parent, NESPalette& nes_palette, PAL &pal, int palno)
    : QWidget{parent}
    , pal{pal}
    , palno{palno}
    , nes_palette{nes_palette}
{
    if (palno >= 4 || palno < 0) { /* TODO: magic number */
        throw std::out_of_range("palette number");
    }
    setMouseTracking(true);
}

void PALView::paintEvent(QPaintEvent *Event)
{
    QPainter painter{this};
    int scale = pal.current_scale();
    char buf[3];
    std::snprintf(buf,sizeof(buf),"%d:",palno);
    painter.drawText(QRect(0,0,scale,scale),Qt::AlignLeft,QString{buf});
    resize(5*scale,scale);
    for (int i = 0; i < 4; ++i) {
        if (palno * 4 + i == pal.current_index()) {
            QPen pen_black{QColor(128,128,128)};
            painter.fillRect(i*scale+scale,0,scale,scale,Qt::white);
            painter.setPen(pen_black);
            painter.drawRect(i*scale+scale+1, 1, scale-3, scale-3);
            painter.fillRect(i*scale+scale+2,2,scale-4,scale-4,nes_palette[pal[palno * 4 + i]]);
        } else {
            painter.fillRect(i*scale+scale,0,scale,scale,nes_palette[pal[palno * 4 + i]]);
        }
    }
}

void PALView::mouseMoveEvent(QMouseEvent *Event)
{
    auto x = Event->pos().x();
    int scale = pal.current_scale();
    if (x < scale) {
        Event->ignore();
        return;
    }
    int cidx = (x-scale) / scale;
    if (cidx >= 0 && cidx < 4) {
        char buf0[2];
        char buf1[2];
        char buf2[5];
        char buf3[4];
        std::snprintf(buf0, sizeof(buf0), "%d", palno);
        std::snprintf(buf1, sizeof(buf1), "%d", cidx);
        std::snprintf(buf2, sizeof(buf2), "%4x", 0x3f00 + palno*4 + cidx);
        std::snprintf(buf3, sizeof(buf3), "$%02x", pal[palno*4 + cidx]);
        emit signal_status("Pal: " + QString{buf0}
                + "  Entry: " + QString{buf1}
                + "  Adr: " + QString{buf2}
                + "  Color: " + QString{buf3}, 0);
    }
}

void PALView::mousePressEvent(QMouseEvent *Event)
{
    auto x = Event->pos().x();
    int scale = pal.current_scale();
    if (x < scale) {
        Event->ignore();
        return;
    }
    int cidx = (x-scale) / scale;
    if (cidx >= 0 && cidx < 4) {
        pal.set_current_index(palno * 4 + cidx);
    } else {
        Event->ignore();
        return;
    }
}
