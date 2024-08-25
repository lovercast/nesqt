#include "nametable.h"

Nametable::Nametable(QObject *parent)
    : QObject{parent}
{
    data = QVector(w*h,(uchar)0);
}

Nametable::DataT Nametable::get(IndexT x, IndexT y) const
{
    return data[y*w + x];
}

void Nametable::set(IndexT x, IndexT y, DataT val)
{
    data[y*w + x] = val;
    emit changed();
}

Nametable::IndexT Nametable::width() const
{
    return w;
}

Nametable::IndexT Nametable::height() const
{
    return h;
}

void Nametable::fill(QVector<std::pair<size_t,size_t>> tiles, uchar tileno)
{
    for (auto [x,y] : tiles) {
        set(x,y,tileno);
    }
    emit changed();
}

void Nametable::load_from_file(DataT *buf, size_t sz)
{
    data.clear();
    for (int i = 0; i < sz; ++i) {
        data.push_back(buf[i]);
    }
    emit changed();
}
