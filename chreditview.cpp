#include <QMouseEvent>
#include "chreditview.h"
#include "chreditundo.h"

CHREditView::CHREditView(CHR& chr, PAL &pal, NESPalette &nes_palette, QWidget *parent)
    : QWidget{parent}
    , chr{chr}
    , side {qMin(width(), height())}
    , scale {(side - 2*pad) / 16}
    , pal{pal}
    , nes_palette{nes_palette}
    , undo_stack{new QUndoStack}
{
    setFocusPolicy(Qt::StrongFocus);
    connect(&chr, SIGNAL(changed()), this, SLOT(update()));
}

void CHREditView::draw_tile_at(QPainter &painter, int x,int y, int tileno, int scale, bool pgrid=true)
{
    if(tileno >= 0 && tileno < 256) {
        auto tile = chr.get_tile(tileno);
        for (int dy = 0; dy < chr.tile_height(); ++dy) {
            for (int dx = 0; dx < chr.tile_width(); ++dx) {
                int col = tile.at(dy * chr.tile_width() + dx);
                auto color = nes_palette[pal[pal.current_palette()*4 + col]];
                painter.fillRect(x + dx * scale, y + dy * scale, scale-(pgrid ? 1 : 0), scale-(pgrid ? 1 : 0), color);
            }
        }
    }
}

void CHREditView::change_tile(int dx,int dy)
{
    int tileActive = chr.current_tile();

    int tx = tileActive % 16 + dx;
    int ty = tileActive / 16 + dy;

    if(tx < 0 || tx > 15 || ty < 0 || ty > 15) return;
    chr.set_current_tile(tileActive + (dy * 16 + dx));
}

void CHREditView::resizeEvent(QResizeEvent *Event)
{
    side = qMin(width(), height());
    scale = (side - 2*pad) / 16;
}

void CHREditView::paintEvent(QPaintEvent *Event)
{
    QPainter painter{this};
    auto tileActive = chr.current_tile();
    int tx = tileActive % 16;
    int ty = tileActive / 16;

    draw_tile_at(painter, -side/4, -side/4,  ty>0&&tx>0  ?tileActive-17:-1, scale);
    draw_tile_at(painter, side/4, -side/4,  ty>0 ?tileActive-16:-1, scale);
    draw_tile_at(painter, 3*side/4, -side/4,  ty>0&&tx<15 ?tileActive-15:-1, scale);

    draw_tile_at(painter, -side/4, side/4,  tx>0 ? tileActive-1 :-1, scale);
    draw_tile_at(painter, side/4, side/4,  tileActive, scale);
    draw_tile_at(painter, 3*side/4, side/4,  tx<15 ? tileActive+1 :-1, scale);

    draw_tile_at(painter, -side/4, 3*side/4,  ty<15 && tx>0 ?tileActive+15:-1, scale);
    draw_tile_at(painter, side/4, 3*side/4,  ty<15 ? tileActive+16:-1, scale);
    draw_tile_at(painter, 3*side/4, 3*side/4,  ty<15 && tx<15 ? tileActive+17:-1, scale);
}

#define ACTION_EYEDROPPER(_fr,_to) do \
    undo_stack->push(new EyedropperUndo(*this, {.from = (_fr), .to = (_to)})); \
while (0)
#define ACTION_STROKE(_tx,_ty,_fr,_to) do \
    undo_stack->push(new StrokeUndo(*this, { \
        .x = static_cast<uchar>(_tx), \
        .y = static_cast<uchar>(_ty), \
        .from = static_cast<uchar>(_fr), \
        .to = static_cast<uchar>(_to)} \
    , undo_count, chr.current_tile())); \
while (0)

void CHREditView::left_mouse_handler(int x, int y, int modifiers)
{
    auto tx = (x - side/4) / scale, ty = (y - side/4) / scale;
    if (auto pix = static_cast<uchar>(chr.pixel(tx,ty))
        , pen = static_cast<uchar>(pal.current_relative())
        ; pix != pen) {
        if (modifiers & Qt::AltModifier) {
            ACTION_EYEDROPPER(pen, pix);
        } else {
            ACTION_STROKE(tx, ty, pix, pen);
        }
    }
}

void CHREditView::mousePressEvent(QMouseEvent *event)
{
    int x = event->localPos().x();
    int y = event->localPos().y();
    int dx = 0, dy = 0;
    /*
     * If not over the current tile,
     * change the current tile in that direction
     */
    if (x < view_offset()) {
        dx = -1;
    } else if (x >= view_offset() + view_width()) {
        dx = +1;
    }
    if (y < view_offset()) {
        dy = -1;
    } else if (y >= view_offset() + view_height()) {
        dy = +1;
    }
    if (dx || dy) {
        change_tile(dx, dy);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        left_mouse_handler(x,y,event->modifiers());
        return;
    }
    /* Unreachable */
    QWidget::mousePressEvent(event);
}

void CHREditView::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->localPos().x();
    int y = event->localPos().y();
    /*
     * If not over current tile, pass.
     */
    if (x < view_offset() || x >= view_offset() + view_width() || y < view_offset() || y >= view_offset() + view_height()) {
        QWidget::mouseMoveEvent(event);
    } else if (event->buttons() == Qt::LeftButton) {
        left_mouse_handler(x,y,0);
    }
}

void CHREditView::update_pixels(std::vector<CHR::PixelUpdate> pixels, int tileid)
{
    auto tileno = chr.tileno_from_tileid(tileid);
    chr.set_pixels_at_tile(pixels, tileno);
}

void CHREditView::wheelEvent(QWheelEvent *Event)
{
    if (Event->angleDelta().y() > 0) {
        chr.set_current_tile(chr.current_tile() == 0 ? 255 : chr.current_tile() - 1);
    } else {
        chr.set_current_tile(chr.current_tile() == 255 ? 0 : chr.current_tile() + 1);
    }
    update();
}

void CHREditView::undo_handler()
{
    undo_stack->undo();
}

void CHREditView::redo_handler()
{
    undo_stack->redo();
}

void CHREditView::keyPressEvent(QKeyEvent *event)
{
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    if (ctrl) {
        if (shift) {
            if (event->key() == Qt::Key_Z) {
                redo_handler();
            } else {
                goto pass;
            }
        } else {
            switch(event->key()) {
            case Qt::Key_Z:
                undo_handler();
                break;
            default:
                goto pass;
            }
        }
    } else {
        goto pass;
    }
    return;
pass:
    QWidget::keyPressEvent(event);
}

void CHREditView::mouseReleaseEvent(QMouseEvent *event)
{
    undo_count++;
}

