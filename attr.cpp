#include "attr.h"

ATTR::ATTR(QObject *parent)
    : QObject{parent}
{
    data = QVector(w*h,(size_t)0);
}

ATTR::DataT ATTR::get(IndexT x, IndexT y) const
{
    y /= 2;
    x /= 2;
    return data[y*w + x];
}

void ATTR::set(IndexT x, IndexT y, IndexT val)
{
    y /= 2;
    x /= 2;
    data[y*w + x] = val;
    emit changed();
}
