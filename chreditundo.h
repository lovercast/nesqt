#pragma once
#ifndef CHREDITUNDO_H
#define CHREDITUNDO_H
#include <deque>
#include "chreditview.h"

struct CHREditView::StrokeCommand {
    uchar x;
    uchar y;
    uchar from;
    uchar to;
    bool operator==(StrokeCommand b) {
        return memcmp(this, &b, sizeof(StrokeCommand)) == 0;
    }
};

struct CHREditView::EyedropperCommand {
    uchar from;
    uchar to;
};

class CHREditView::StrokeUndo : public QUndoCommand {
public:
    StrokeUndo(CHREditView &parent, StrokeCommand, int count, int tile_id);
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override;
    int count() const { return m_count; }
    int &count() { return m_count; }
private:
    CHREditView &parent;
    std::deque<StrokeCommand> buf;
    int m_count;
    int tile_id;
};

class CHREditView::EyedropperUndo : public QUndoCommand {
public:
    EyedropperUndo(CHREditView &parent, EyedropperCommand e) : parent{parent}, e{e} {}
    void undo() override;
    void redo() override;
private:
    CHREditView &parent;
    EyedropperCommand e;
};

#endif // CHREDITUNDO_H
