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

std::vector<ATTR::DataT> ATTR::save_to_file() {
    std::vector<DataT> buf(64);
    for (int y = 0; y < h; y += 2) {
        for (int x = 0; x < w; x += 2) {
            char topleft = data[y * w + x];
            char topright = data[y * w + x + 1];
            char bottomleft = data[(y + 1) * w + x];
            char bottomright = data[(y + 1) * w + x + 1];
            buf.push_back((bottomright << 6) | (bottomleft << 4) | (topright << 2) | topleft);
        }
    }
    return buf;
}


void ATTR::load_from_file(DataT buf[]) {
    auto p = data.begin();
    for (int y = 0; y < h; y += 2) {
        for (int x = 0; x < w; x += 2) {
            auto val = *p++;
            data[y * w + x] = val & 0x3;
            data[y * w + x + 1] = (val >> 2) & 0x3;
            data[(y + 1) * w + x] = (val >> 4) & 0x3;
            data[(y + 1) * w + x + 1] = (val >> 6) & 0x3;
        }
    }
}

