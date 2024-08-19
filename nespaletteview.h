#pragma once
#ifndef NESPALETTEVIEW_H
#define NESPALETTEVIEW_H

#include "nespalette.h"
#include "pal.h"

#include <QWidget>

class NESPaletteView : public QWidget
{
    Q_OBJECT
public:
    explicit NESPaletteView(QWidget *parent, NESPalette& nes_palette, PAL& pal);
    void paintEvent(QPaintEvent *Event) override;
    void mouseMoveEvent(QMouseEvent *Event) override;
    void mousePressEvent(QMouseEvent *Event) override;

signals:
    void signal_status(QString,int);
    void signal_mousepress(int);
private:
    PAL& pal;
    NESPalette& nes_palette;
    int scale;
    int nrows;
    int row_len;
};

#endif // NESPALETTEVIEW_H
