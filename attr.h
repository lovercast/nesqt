#pragma once
#ifndef ATTR_H
#define ATTR_H

#include <QObject>
#include "nametable.h"

class ATTR : public QObject
{
    Q_OBJECT
    using DataT = uchar;
    using IndexT = size_t;
    static constexpr IndexT MAX_SIZE = (Nametable::MAX_WIDTH/4)*(Nametable::MAX_HEIGHT/4);
public:
    explicit ATTR(QObject *parent = nullptr);
    DataT get(IndexT x, IndexT y) const;
    void set(IndexT x, IndexT y, IndexT val);
signals:
    void changed();
    void changed_idx(int);
private:
    QVector<IndexT> data;
    IndexT w = 16;
    IndexT h = 16;
};

#endif // ATTR_H
