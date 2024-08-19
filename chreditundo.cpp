#include "chreditundo.h"
#include "NesQt.h"


CHREditView::StrokeUndo::StrokeUndo(CHREditView &parent, StrokeCommand command, int count, int tile_id)
    : parent{parent}
    , m_count{count}
    , tile_id{tile_id}
{
    buf = std::deque<StrokeCommand>{command};
}

void CHREditView::StrokeUndo::undo()
{
    std::vector<CHR::PixelUpdate> pixels{};
    for (auto p = buf.cbegin(); p != buf.cend(); ++p) {
        pixels.push_back({p->x,p->y,p->from});
    }
    parent.update_pixels(pixels,tile_id);
    parent.update();
}

void CHREditView::StrokeUndo::redo()
{
    std::vector<CHR::PixelUpdate> pixels{};
    for (auto p = buf.cbegin(); p != buf.cend(); ++p) {
        pixels.push_back({p->x,p->y,p->to});
    }
    parent.update_pixels(pixels,tile_id);
    parent.update();
}

bool CHREditView::StrokeUndo::mergeWith(const QUndoCommand *other)
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

int CHREditView::StrokeUndo::id() const
{
    return NESQT::UndoTypes::CHREdit;
}

void CHREditView::EyedropperUndo::undo()
{
    parent.pal.set_current_relative(e.from);
}
void CHREditView::EyedropperUndo::redo()
{
    parent.pal.set_current_relative(e.to);
}
