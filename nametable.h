#pragma once
#ifndef NAMETABLE_H
#define NAMETABLE_H

#include <QObject>

class Nametable : public QObject
{
    Q_OBJECT
    using DataT = uchar;
    using IndexT = size_t;
public:
    static constexpr IndexT MAX_WIDTH = 4096;
    static constexpr IndexT MAX_HEIGHT = 4096;
    static constexpr IndexT MAX_SIZE = MAX_WIDTH*MAX_HEIGHT;

public:
    explicit Nametable(QObject *parent = nullptr);
    DataT get(IndexT x, IndexT y) const;
    void set(IndexT x, IndexT y, DataT val);
    IndexT width() const;
    IndexT height() const;
    void fill(QVector<std::pair<size_t,size_t>>, uchar val);

signals:
    void changed();
    void changed_idx(int);
private:
    QVector<DataT> data;
    IndexT w = 32;
    IndexT h = 32;
};

#endif // NAMETABLE_H
