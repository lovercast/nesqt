#pragma once
#ifndef CHRVIEW_H
#define CHRVIEW_H

#include <QWidget>
#include "chr.h"
#include "chrrenderer.h"
#include "NesQt.h"

class CHRView : public QWidget
{
    Q_OBJECT
public:
    explicit CHRView(CHR &chr, CHRRenderer &chr_renderer, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *Event) override;
    void mousePressEvent(QMouseEvent *Event) override;
    void mouseDoubleClickEvent(QMouseEvent *Event) override;
    void draw_grids(QPainter &painter);
    void set_grid_size(NESQT::GridSize);
signals:
    void doubleClicked();
private:
    CHR &chr;
    CHRRenderer &chr_renderer;
    NESQT::GridSize gs = NESQT::GridSize::None;
};

#endif // CHRVIEW_H
