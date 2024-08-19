#pragma once
#ifndef PALVIEW_H
#define PALVIEW_H

#include <QWidget>
#include "pal.h"
#include "nespalette.h"

class PALView : public QWidget
{
    Q_OBJECT
public:
    explicit PALView(QWidget *parent, NESPalette& nes_palette, PAL &pal, int palno);
    void paintEvent(QPaintEvent *Event) override;
    void mouseMoveEvent(QMouseEvent *Event) override;
    void mousePressEvent(QMouseEvent *Event) override;
signals:
    void signal_status(QString, int);
private:
    NESPalette& nes_palette;
    PAL& pal;
    const int palno;
};

#endif // PALVIEW_H
