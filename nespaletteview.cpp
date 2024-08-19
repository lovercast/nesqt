#include <QPainter>
#include <QMouseEvent>
#include "nespaletteview.h"

NESPaletteView::NESPaletteView(QWidget *parent, NESPalette& nes_palette, PAL& pal)
    : QWidget{parent}
    , nes_palette{nes_palette}
    , pal{pal}
    , scale{20} /* TODO: magic number */
    , nrows{4}
{

    row_len = nes_palette.size() / nrows;
    setMinimumSize((row_len - 2)*scale,nrows*scale);
    connect(&pal, SIGNAL (changed()), this, SLOT (update()));
    connect(this, SIGNAL (signal_mousepress(int)), &pal, SLOT (slot_change_color(int)));
    connect(&nes_palette, SIGNAL (changed()), this, SLOT (update()));
    setMouseTracking(true);
}

void NESPaletteView::paintEvent(QPaintEvent *Event)
{
    QPainter painter{this};
    auto current_color = pal.current_color();
    for (int i = 0; i < nrows; ++i) {
        for (int j = 0; j < row_len-2; ++j) {
            auto cidx = i * row_len + j;
            if (current_color == cidx) {
                QPen pen_black{QColor(128,128,128)};
                painter.fillRect(scale*j,scale*i,scale,scale,Qt::white);
                painter.setPen(pen_black);
                painter.drawRect(scale*j+1, scale*i+1, scale-3, scale-3);
                painter.fillRect(scale*j+2,scale*i+2,scale-4,scale-4,nes_palette[cidx]);
            } else if (current_color == 0x0f && i == 0 && j == row_len - 2 - 1) {
                QPen pen_black{QColor(128,128,128)};
                painter.fillRect(scale*j,scale*i,scale,scale,Qt::white);
                painter.setPen(pen_black);
                painter.drawRect(scale*j+1, scale*i+1, scale-3, scale-3);
                painter.fillRect(scale*j+2,scale*i+2,scale-4,scale-4,nes_palette[cidx]);
            } else {
                painter.fillRect(scale*j,scale*i,scale,scale,nes_palette[cidx]);
            }
        }
    }
}

void NESPaletteView::mouseMoveEvent(QMouseEvent *Event)
{
    auto x = Event->pos().x(), y = Event->pos().y();
    int cidx = (x/scale) + 16 * (y/scale);
    if (cidx == 0x0D || cidx == 0x1D) cidx = 0x0F;
    if (cidx >= 0 && cidx < nrows*row_len) {
        char buf[4];
        std::snprintf(buf, sizeof(buf), "$%02x", cidx);
        emit signal_status("Color: " + QString{buf}, 0);
    }
}

void NESPaletteView::mousePressEvent(QMouseEvent *Event)
{
    auto x = Event->pos().x(), y = Event->pos().y();
    int cidx = (x/scale) + 16 * (y/scale);
    if (cidx == 0x0D || cidx == 0x1D) cidx = 0x0F;
    if (cidx >= 0 && cidx < nrows*row_len) {
        emit signal_mousepress(cidx);
    }
}
