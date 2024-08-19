#pragma once
#ifndef TILE_H
#define TILE_H

#include <QObject>

template<size_t W, size_t H>
class Tile : public QObject
{
    uchar data[W * H];
public:
    explicit Tile();
    constexpr size_t width() const { return W; }
    constexpr size_t height() const { return H; }
signals:

};

#endif // TILE_H
