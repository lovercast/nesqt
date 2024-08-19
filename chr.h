#pragma once
#ifndef CHR_H
#define CHR_H

#include <QObject>
#include <vector>
//#include "pal.h"

class CHR : public QObject
{
    using DataT = uchar;
    using IndexT = size_t;
    Q_OBJECT
public:
    CHR();
    std::vector<DataT> get_tile(const IndexT tile) const;
    const IndexT tile_width() const { return tilew; }
    const IndexT tile_height() const { return tileh; }
    const IndexT current_tile() const { return tileActive; }
    //IndexT &current_tile() { emit changed(); return tileActive; }
    void set_current_tile(IndexT tile);
    void scroll_left(void);
    void scroll_right(void);
    void scroll_up(void);
    void scroll_down(void);
    void mirror(bool horizontal=true);
    void rotate(bool clockwise=true);
    void load_from_file(const char *filename);
//    QPixmap &&to_pixmap(PAL &pal);
    const auto &map() const { return data; }
    /*
     * Change a pixel value in the active tile.
     */
//    DataT &pixel(IndexT x, IndexT y);
    DataT pixel(IndexT x, IndexT y) const;
    void set_pixel(IndexT x, IndexT y, DataT val);
    void set_pixel_at_tile(IndexT x, IndexT y, DataT val, IndexT tileno);
    using PixelUpdate = std::tuple<uchar,uchar,uchar>;
    void set_pixels_at_tile(std::vector<PixelUpdate> pixels, IndexT tileno);

    IndexT bank_size() const { return 64 * 256; }
    void swap_banks();
    constexpr int size() const { return m_size; }
    DataT tileno_from_tileid(IndexT tile_id) const { return tileid_2_tileno[tile_id]; }
signals:
    void changed();
    void changed_current();
    void changed_xy(int,int);
    void changed_idx(int);
    void load();
    void signal_swap_banks();

private slots:
    void slot_swap_banks();
private:
    static constexpr int m_size = 256;
    const IndexT tilew = 8;
    const IndexT tileh = 8;
    DataT *data;
    DataT banks[2][64*256] = {0};
    DataT bank0[64*256] = {0};
    DataT bank1[64*256] = {0};
    IndexT tileActive = 0;
    IndexT bankActive = 0;
    DataT *tileno_2_tileid;
    DataT *tileid_2_tileno;
    DataT tileno_2_tileid_maps[2][64*256];
    DataT tileid_2_tileno_maps[2][64*256];
};

#endif // CHR_H
