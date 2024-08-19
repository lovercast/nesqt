#pragma once
#ifndef CHREDITVIEW_H
#define CHREDITVIEW_H

#include <QWidget>
#include <QPainter>
#include <QUndoStack>
#include "chr.h"
#include "pal.h"
#include "nespalette.h"

class CHREditView : public QWidget
{
    Q_OBJECT
public:
    explicit CHREditView(CHR& chr, PAL &pal, NESPalette &nes_palette, QWidget *parent = nullptr);
private:
    void draw_tile_at(QPainter &painter, int xs,int ys, int, int, bool);
    void change_tile(int dx,int dy);
    void left_mouse_handler(int x, int y, int modifiers);
    void undo_handler();
    void redo_handler();
    int view_width() const { return chr.tile_width() * scale; }
    int view_height() const { return chr.tile_height() * scale; }
    int view_offset() const { return side / 4; }
    void keyPressEvent(QKeyEvent *Event) override;
    void mouseMoveEvent(QMouseEvent *Event) override;
    void mousePressEvent(QMouseEvent *Event) override;
    void mouseReleaseEvent(QMouseEvent *Event) override;
    void paintEvent(QPaintEvent *Event) override;
    void resizeEvent(QResizeEvent *Event) override;
    void wheelEvent(QWheelEvent *Event) override;
private slots:
    void update_pixels(std::vector<CHR::PixelUpdate>, int);
private:
    CHR &chr;
    PAL &pal;
    NESPalette &nes_palette;
    QUndoStack *undo_stack;
    unsigned int undo_count = 0;
    int pad = 8;
    int side;
    int scale;

private:
    struct StrokeCommand;
    struct EyedropperCommand;
    class StrokeUndo;
    class EyedropperUndo;
};

#endif // CHREDITVIEW_H
