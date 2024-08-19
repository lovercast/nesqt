#include "pal.h"

PAL::PAL(QObject *parent)
    : QObject{parent}
    , data{0}
    , entryActive{0}
    , palettes{std::vector<std::vector<uchar>>(5)}
{
    memcpy(data, &bg_pal_default[16], 16);
    for (int i = 0; i < 4; ++i) {
        auto p = &bg_pal_default[i*16 + 16];
        std::copy(p, p+16, std::back_inserter(palettes[i]));
    }
}

void PAL::switch_palette(BGPalette n)
{
    size_t idx = static_cast<uchar>(n) - 1;
    std::copy(data,data+16,palettes[static_cast<size_t>(palActive)-1].begin());
    std::copy(palettes[idx].begin(),palettes[idx].end(),data);
    palActive = n;
    emit load();
}

void PAL::load_default_palette(BGPalette n)
{
    memcpy(data, &bg_pal_default[static_cast<size_t>(n)*16], 16);
    emit changed();
    emit load();
}

void PAL::slot_change_color(int color)
{
    // transparent colors must all be the same
    if (entryActive % 4 == 0) {
        data[0] = color;
        data[4] = color;
        data[8] = color;
        data[12] = color;
        emit changed_background();
    } else {
        data[entryActive] = color;
        emit changed_palette(current_palette());
    }
    emit changed();
}
