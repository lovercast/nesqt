#pragma once
#ifndef NAMETABLEVIEW_H
#define NAMETABLEVIEW_H

#include <QWidget>
#include "nametablerenderer.h"

class NametableView : public QWidget
{
    Q_OBJECT
public:
    explicit NametableView(NametableRenderer &nametable_renderer, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
private:
    NametableRenderer &nametable_renderer;
};

#endif // NAMETABLEVIEW_H
