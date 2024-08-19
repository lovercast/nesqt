#pragma once
#ifndef NAMETABLEUNDO_H
#define NAMETABLEUNDO_H

#include "nametablerenderer.h"

enum class NametableRenderer::StrokeType {
    Tile,
    Palette,
};

struct NametableRenderer::StrokeCommand {
    int x;
    int y;
    uchar from;
    uchar to;
    StrokeType type;
    bool operator==(StrokeCommand b) {
        return memcmp(this, &b, sizeof(StrokeCommand)) == 0;
    }
};

struct NametableRenderer::FillCommand {
    int x;
    int y;
    uchar from;
    uchar to;
    bool operator==(FillCommand b) {
        return memcmp(this, &b, sizeof(FillCommand)) == 0;
    }
};


struct NametableRenderer::EyedropperCommand {
    uchar tile_from;
    uchar tile_to;
    uchar pal_from;
    uchar pal_to;
};

class NametableRenderer::StrokeUndo : public QUndoCommand {
public:
    StrokeUndo(NametableRenderer &parent, StrokeCommand, int count);
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override;
    int count() const { return m_count; }
    int &count() { return m_count; }
private:
    NametableRenderer &parent;
    std::deque<StrokeCommand> buf;
    int m_count;
};

class NametableRenderer::FillUndo : public QUndoCommand {
public:
    FillUndo(NametableRenderer &parent, FillCommand f);
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    FillCommand f;
    std::vector<TileCoord> tiles;
};

class NametableRenderer::SelectFillUndo : public QUndoCommand {
public:
    using TileCoordFrom = std::tuple<int,int,uchar>;
    using PalCoordFrom = TileCoordFrom;
    SelectFillUndo(NametableRenderer &parent, uchar tileno_to, uchar palno_to);
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    uchar tileno_to;
    uchar palno_to;
    std::vector<TileCoordFrom> tiles;
    std::vector<PalCoordFrom> palettes;
};

class NametableRenderer::EyedropperUndo : public QUndoCommand {
public:
    EyedropperUndo(NametableRenderer &parent, EyedropperCommand e) : parent{parent}, e{e} {}
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    EyedropperCommand e;
};

class NametableRenderer::SelectUndo : public QUndoCommand {
public:
    SelectUndo(NametableRenderer &parent);
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    Selector selector;
    Selector back;
};

class NametableRenderer::SelectCancelUndo : public QUndoCommand {
public:
    SelectCancelUndo(NametableRenderer &parent);
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    Selector selector;
};

class NametableRenderer::PasteUndo : public QUndoCommand {
public:
    PasteUndo(NametableRenderer &parent, TileCoord coord);
    void undo() override;
    void redo() override;
private:
    NametableRenderer &parent;
    TileCoord coord;
    Selector selector;
    std::vector<TileMapCommand> tiles;
    std::vector<TileMapCommand> palettes;
};

#endif // NAMETABLEUNDO_H
