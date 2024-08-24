#include "chr.h"
#include "NesQt.h"

#include <QFile>


CHR::CHR()
    : tileActive{0}
    , data{banks[0]}
    , tileid_2_tileno{tileid_2_tileno_maps[0]}
    , tileno_2_tileid{tileno_2_tileid_maps[0]}
{
    for (int i = 0; i < 64*256; ++i) {
        tileno_2_tileid_maps[0][i] = tileno_2_tileid_maps[1][i] = i;
        tileid_2_tileno_maps[0][i] = tileid_2_tileno_maps[1][i] = i;
    }
}

void CHR::load_from_file(QString filename)
{
    DataT buf[8192]; /* TODO: magic number */
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        NESQT::warn(tr("Failed to open file"));
        return;
    }

    QDataStream ds(&f);

    if (ds.readRawData((char *) buf, sizeof(buf)) < sizeof(buf)) {
        NESQT::warn(tr("Failed to read 8K from CHR file"));
        return;
    }
    DataT tile[64];
    for (IndexT ty = 0; ty < 16; ++ty) {
        for (IndexT tx = 0; tx < 16; ++tx) {
            IndexT tileno = ty*16 + tx;
            IndexT pp = tileno*16; // + bankActive
            for (IndexT y = 0; y < 8; ++y) {
                for (IndexT x = 0; x < 8; ++x) {
                    tile[y*8 + x] = (((buf[pp+y]<<x)&128)>>7)|(((buf[pp+y+8]<<x)&128)>>6);
                }
            }
            for (IndexT y = 0; y < 8; ++y) {
                std::copy(&tile[y*8],&tile[y*8 + 8],&banks[0][ty*128*8 + y*128 + tx*8]);
            }
        }
    }
    for (IndexT ty = 0; ty < 16; ++ty) {
        for (IndexT tx = 0; tx < 16; ++tx) {
            IndexT tileno = ty*16 + tx;
            IndexT pp = tileno*16 + 4096; // + bankActive
            for (IndexT y = 0; y < 8; ++y) {
                for (IndexT x = 0; x < 8; ++x) {
                    tile[y*8 + x] = (((buf[pp+y]<<x)&128)>>7)|(((buf[pp+y+8]<<x)&128)>>6);
                }
            }
            for (IndexT y = 0; y < 8; ++y) {
                std::copy(&tile[y*8],&tile[y*8 + 8],&banks[1][ty*128*8 + y*128 + tx*8]);
            }
        }
    }
    emit load();
    emit changed();
}

void CHR::set_current_tile(IndexT tile)
{
    tileActive = tile;
    emit changed();
    emit changed_current();
}

void CHR::scroll_left(void)
{
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    for (IndexT y = 0; y < 8; ++y) {
        IndexT idx = ty*128*8 + tx*8 + y*128;
        auto tmp = data[idx];
        memmove(&data[idx],&data[idx+1],7);
        data[idx+7] = tmp;
    }
    emit changed_idx(tileActive);
    emit changed();
}

void CHR::scroll_right(void)
{
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    for (IndexT y = 0; y < 8; ++y) {
        IndexT idx = ty*128*8 + tx*8 + y*128;
        auto tmp = data[idx+7];
        memmove(&data[idx+1],&data[idx],7);
        data[idx] = tmp;
    }
    emit changed_idx(tileActive);
    emit changed();
}

void CHR::scroll_up(void)
{
    DataT buf[8];
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    memcpy(buf,&data[ty*128*8 + tx*8],8);
    for (IndexT y = 1; y < 8; ++y) {
        IndexT idx = ty*128*8 + tx*8 + y*128;
        memmove(&data[idx-128],&data[idx],8);
    }
    memcpy(&data[ty*128*8 + tx*8 + 7*128],buf,8);
    emit changed_idx(tileActive);
    emit changed();
}

void CHR::scroll_down(void)
{
    DataT buf[8];
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    memcpy(buf,&data[ty*128*8 + tx*8 + 7*128],8);
    for (IndexT y = 7; y >=1; --y) {
        IndexT idx = ty*128*8 + tx*8 + y*128;
        memmove(&data[idx],&data[idx-128],8);
    }
    memcpy(&data[ty*128*8 + tx*8],buf,8);
    emit changed_idx(tileActive);
    emit changed();
}

void CHR::mirror(bool horizontal)
{
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    if (horizontal) {
        for (IndexT y = 0; y < 8; ++y) {
            for (IndexT i = 0; i < 4; ++i) {
                IndexT idx = ty*128*8 + tx*8 + y*128;
                std::swap(data[idx+i],data[idx+7-i]);
            }
        }
    } else {
        DataT tmp[8];
        for (IndexT y = 0; y < 4; ++y) {
            IndexT row1 = ty*128*8 + tx*8 + y*128;
            IndexT row2 = ty*128*8 + tx*8 + (7-y)*128;
            memcpy(tmp,&data[row1],8);
            memcpy(&data[row1],&data[row2],8);
            memcpy(&data[row2],tmp,8);
        }
    }
    emit changed_idx(tileActive);
    emit changed();
}

void CHR::rotate(bool clockwise)
{
    DataT tile[8][8], tile_flip[8][8];
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    for (IndexT y = 0; y < 8; ++y) {
        size_t idx = ty*128*8 + tx*8 + y*128;
        memcpy(tile[y],&data[idx],8);
    }
    for (IndexT i = 0; i < 8; ++i) {
        for (IndexT j = 0; j < 8; ++j) {
            tile_flip[i][j] = clockwise ? tile[7-j][i] : tile[j][7-i];
        }
    }
    for (IndexT y = 0; y < 8; ++y) {
        size_t idx = ty*128*8 + tx*8 + y*128;
        memcpy(&data[idx],tile_flip[y],8);
    }
    emit changed_idx(tileActive);
    emit changed();
}

std::vector<CHR::DataT> CHR::get_tile(const IndexT tileno) const {
    std::vector<DataT> v;
    IndexT ty = tileno / 16, tx = tileno % 16;
    for(IndexT y = 0; y < 8; ++y) {
        IndexT idx = ty*128*8 + tx*8 + y*128;
        std::copy(&data[idx],&data[idx+8],std::back_inserter(v));
    }
    return v;
}

void CHR::set_pixel(IndexT x, IndexT y, DataT val)
{
    set_pixel_at_tile(x,y,val,tileActive);
}

void CHR::set_pixel_at_tile(IndexT x, IndexT y, DataT val, IndexT tileno)
{
    IndexT ty = tileno / 16, tx = tileno % 16;
    IndexT idx = ty*128*8 + tx*8 + y*128 + x;
    data[idx] = val;
    emit changed_idx(tileno);
    emit changed();
}

void CHR::set_pixels_at_tile(std::vector<std::tuple<uchar,uchar,uchar>> pixels, IndexT tileno)
{
    IndexT ty = tileno / 16, tx = tileno % 16;
    for (auto [x,y,val] : pixels) {
        IndexT idx = ty*128*8 + tx*8 + y*128 + x;
        data[idx] = val;
    }
    emit changed_idx(tileno);
    emit changed();
}


CHR::DataT CHR::pixel(IndexT x, IndexT y) const
{
    IndexT ty = tileActive / 16, tx = tileActive % 16;
    IndexT idx = ty*128*8 + tx*8 + y*128 + x;
    return data[idx];
}

void CHR::swap_banks()
{
    bankActive = bankActive ? 0 : 1;
    data = banks[bankActive];
    tileid_2_tileno = tileid_2_tileno_maps[bankActive];
    emit signal_swap_banks();
    emit changed();
}

void CHR::slot_swap_banks()
{
    bankActive = bankActive ? 0 : 1;
    data = banks[bankActive];
    emit signal_swap_banks();
    emit changed();
}
