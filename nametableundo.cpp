#include "nametableundo.h"

NametableRenderer::StrokeUndo::StrokeUndo(NametableRenderer &parent, StrokeCommand command, int count)
    : parent{parent}
    , m_count{count}
{
    buf = std::deque<StrokeCommand>{command};
}

void NametableRenderer::StrokeUndo::undo()
{
    for (auto p = buf.crbegin(); p != buf.crend(); ++p) {
        if (p->type == StrokeType::Tile) {
            parent.update_xy(p->x,p->y,p->to,p->from);
            parent.redraw_queue.push({p->x,p->y});
        } else if (p->type == StrokeType::Palette) {
            parent.update_attr_block(p->x,p->y,p->to,p->from);
            parent.redraw_queue.push({p->x,     p->y});
            parent.redraw_queue.push({p->x+1,   p->y});
            parent.redraw_queue.push({p->x,     p->y+1});
            parent.redraw_queue.push({p->x+1,   p->y+1});
        }
    }
    parent.update();
}

void NametableRenderer::StrokeUndo::redo()
{
    for (auto p = buf.cbegin(); p != buf.cend(); ++p) {
        if (p->type == StrokeType::Tile) {
            parent.update_xy(p->x,p->y,p->from,p->to);
            parent.redraw_queue.push({p->x,p->y});
        } else if (p->type == StrokeType::Palette) {
            parent.update_attr_block(p->x,p->y,p->from,p->to);
            parent.redraw_queue.push({p->x,     p->y});
            parent.redraw_queue.push({p->x+1,   p->y});
            parent.redraw_queue.push({p->x,     p->y+1});
            parent.redraw_queue.push({p->x+1,   p->y+1});
        }
    }
    parent.update();
}

bool NametableRenderer::StrokeUndo::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) {
        return false;
    }
    auto p = static_cast<const StrokeUndo*>(other);
    if (p->count() != count()) {
        return false;
    }
    buf.emplace_back(p->buf.front());
    return true;
}

int NametableRenderer::StrokeUndo::id() const
{
    return NESQT::UndoTypes::Nametable;
}

NametableRenderer::FillUndo::FillUndo(NametableRenderer &parent, FillCommand f)
    : parent{parent}
    , f{f}
{
    tiles = parent.calculate_fill(f.x,f.y,f.to);
}

void NametableRenderer::FillUndo::undo()
{
    for (auto [x,y] : tiles) {
        parent.update_xy(x,y,f.to,f.from);
        parent.redraw_queue.push({x,y});
    }
    parent.update();
}

void NametableRenderer::FillUndo::redo()
{
    for (auto [x,y] : tiles) {
        parent.update_xy(x,y,f.from,f.to);
        parent.redraw_queue.push({x,y});
    }
    parent.update();
}

NametableRenderer::SelectFillUndo::SelectFillUndo(NametableRenderer &parent, uchar tileno_to, uchar palno_to)
    : parent{parent}
    , tileno_to{tileno_to}
    , palno_to{palno_to}
{
    tiles = std::vector<TileCoordFrom>{};
    palettes = std::vector<PalCoordFrom>{};
    int x0, x1, y0, y1;
    auto cur = parent.selector.current;
    if (cur.x0 <= cur.x1) {
        x0 = cur.x0;
        x1 = cur.x1;
    } else {
        x0 = cur.x1;
        x1 = cur.x0;
    }
    if (cur.y0 <= cur.y1) {
        y0 = cur.y0;
        y1 = cur.y1;
    } else {
        y0 = cur.y1;
        y1 = cur.y0;
    }
    if (parent.apply_tile()) {
        for (auto y = y0; y < y1 + 1; ++y) {
            for (auto x = x0; x < x1 + 1; ++x) {
                auto tileno_from = parent.nametable.get(x,y);
                if (tileno_from != tileno_to) {
                    tiles.push_back({x,y,tileno_from});
                }
            }
        }
    }
    if (parent.apply_palette()) {
        auto y_end = (y1 % 2 == 0) ? y1 + 2 : y1 + 1;
        for (auto y = y0; y < y_end; y += 2) {
            auto x_end = (x1 % 2 == 0) ? x1 + 2 : x1 + 1;
            for (auto x = x0; x < x_end; x += 2) {
                auto palno_from = parent.attr.get(x,y);
                if (palno_from != palno_to) {
                    palettes.push_back({x & ~1,y & ~1,palno_from});
                }
            }
        }
    }
}

void NametableRenderer::SelectFillUndo::undo()
{
    for (auto [x,y,from] : tiles) {
        parent.update_xy(x,y,tileno_to,from);
        parent.redraw_queue.push({x,y});
    }
    for (auto [x,y,from] : palettes) {
        parent.update_attr_block(x,y,palno_to, from);
        parent.redraw_queue.push({x,y});
        parent.redraw_queue.push({x+1,y});
        parent.redraw_queue.push({x,y+1});
        parent.redraw_queue.push({x+1,y+1});
    }
    parent.update();
}

void NametableRenderer::SelectFillUndo::redo()
{
    for (auto [x,y,from] : tiles) {
        parent.update_xy(x,y,from,tileno_to);
        parent.redraw_queue.push({x,y});
    }
    for (auto [x,y,from] : palettes) {
        parent.update_attr_block(x,y,from, palno_to);
        parent.redraw_queue.push({x,y});
        parent.redraw_queue.push({x+1,y});
        parent.redraw_queue.push({x,y+1});
        parent.redraw_queue.push({x+1,y+1});
    }
    parent.update();
}

void NametableRenderer::EyedropperUndo::undo()
{
    parent.chr.set_current_tile(e.tile_from);
    parent.pal.set_current_palette(e.pal_from);
}

void NametableRenderer::EyedropperUndo::redo()
{
    parent.chr.set_current_tile(e.tile_to);
    parent.pal.set_current_palette(e.pal_to);
}

NametableRenderer::SelectUndo::SelectUndo(NametableRenderer &parent)
    : parent{parent}
    , selector{parent.selector}
    , back{parent.back}
{
}

void NametableRenderer::SelectUndo::undo()
{
    parent.selector = back;
    parent.update();
}

void NametableRenderer::SelectUndo::redo()
{
    parent.selector = selector;
    parent.update();
}

NametableRenderer::SelectCancelUndo::SelectCancelUndo(NametableRenderer &parent)
    : parent{parent}
    , selector{parent.selector}
{
}

void NametableRenderer::SelectCancelUndo::undo()
{
    parent.selector = selector;
    parent.update();
}

void NametableRenderer::SelectCancelUndo::redo()
{
    parent.selector.reset();
    parent.update();
}

NametableRenderer::PasteUndo::PasteUndo(NametableRenderer &parent, TileCoord coord)
    : parent{parent}
    , selector{parent.selector}
    , coord{coord}
    , tiles{std::vector<TileMapCommand>{}}
    , palettes{std::vector<TileMapCommand>{}}
{
    int x0, x1, y0, y1;
    auto cur = selector.current;
    if (cur.x0 <= cur.x1) {
        x0 = cur.x0;
        x1 = cur.x1;
    } else {
        x0 = cur.x1;
        x1 = cur.x0;
    }
    if (cur.y0 <= cur.y1) {
        y0 = cur.y0;
        y1 = cur.y1;
    } else {
        y0 = cur.y1;
        y1 = cur.y0;
    }
    /*
     * Tiles and palettes must be copied separately because
     * the user may have caused them to be misaligned.
     */
    if (parent.apply_tile()) {
        auto x_base = coord.first, y_base = coord.second;
        for (auto y = y0; y < y1 + 1; ++y) {
            auto y_prime = y_base + y - y0;
            if (y_prime >= parent.viewport_height) {
                break;
            }
            for (auto x = x0; x < x1 + 1; ++x) {
                auto x_prime = x_base + x - x0;
                if (x_prime >= parent.viewport_width) {
                    break;
                }
                if (auto from = parent.nametable.get(x_prime,y_prime)
                    , to = parent.nametable.get(x,y)
                    ; from != to) {
                    tiles.push_back({x_prime,y_prime,from,to});
                }
            }
        }
    }
    if (parent.apply_palette()) {
        auto x_base = coord.first & ~1, y_base = coord.second & ~1;
        auto pal_x0 = x0 & ~1, pal_x1 = x1 & ~1, pal_y0 = y0 & ~1, pal_y1 = y1 & ~1;
        for (auto y = pal_y0; y < pal_y1 + 1; y += 2) {
            auto y_prime = y_base + y - pal_y0;
            if (y_prime >= parent.viewport_height) {
                break;
            }
            for (auto x = x0; x < x1 + 1; x += 2) {
                auto x_prime = x_base + x - x0;
                if (x_prime >= parent.viewport_width) {
                    break;
                }
                if (auto from = parent.attr.get(x_prime,y_prime)
                    , to = parent.attr.get(x,y)
                    ; from != to) {
                    palettes.push_back({x_prime,y_prime,from,to});
                }
            }
        }
    }
}

void NametableRenderer::PasteUndo::undo()
{
    for (auto [x,y,from,to] : tiles) {
        parent.update_xy(x,y,to,from);
        parent.redraw_queue.push({x,y});
    }
    for (auto [x,y,from,to] : palettes) {
        parent.update_attr_block(x,y,to,from);
        parent.redraw_queue.push({x,y});
        parent.redraw_queue.push({x+1,y});
        parent.redraw_queue.push({x,y+1});
        parent.redraw_queue.push({x+1,y+1});
    }
    parent.update();
}

void NametableRenderer::PasteUndo::redo()
{
    for (auto [x,y,from,to] : tiles) {
        parent.update_xy(x,y,from,to);
        parent.redraw_queue.push({x,y});
    }
    for (auto [x,y,from,to] : palettes) {
        parent.update_attr_block(x,y,from,to);
        parent.redraw_queue.push({x,y});
        parent.redraw_queue.push({x+1,y});
        parent.redraw_queue.push({x,y+1});
        parent.redraw_queue.push({x+1,y+1});
    }
    parent.update();
}
