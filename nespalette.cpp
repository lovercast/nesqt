#include <QColor>
#include "emphasis.h"
#include "nespalette.h"
#include "nespalette_data.h"

NESPalette::NESPalette(QObject *parent)
    : QObject{parent}
    , ppu_mask{0}
{
    for (int i = 0; i < sizeof(nes_palette_ntsc)/sizeof(unsigned char); i+=3) {
        base_palette.push_back(QColor(nes_palette_ntsc[i],nes_palette_ntsc[i+1],nes_palette_ntsc[i+2]));
    }
    out_palette = base_palette;
}

void NESPalette::set_ppu_mask(PPUMask mask)
{
    ppu_mask ^= static_cast<int>(mask);
    if (!ppu_mask) {
        for(int i = 0; i < size(); ++i) {
            out_palette[i] = base_palette[i];
        }
    } else if (ppu_mask & static_cast<int>(PPUMask::Grayscale)) {
        for(int i = 0; i < size(); ++i) {
            out_palette[i] = base_palette[i & 0xf0];
        }
    } else {
        for(int i = 0; i < size(); ++i) {
            double r = ((double)base_palette[i].red());
            double g = ((double)base_palette[i].green());
            double b = ((double)base_palette[i].blue());
            r = r * (emphasis[ppu_mask >> 5][0] / 100.0);
            g = g * (emphasis[ppu_mask >> 5][1] / 100.0);
            b = b * (emphasis[ppu_mask >> 5][2] / 100.0);
            r = qMin(r,255.0);
            g = qMin(g,255.0);
            b = qMin(b,255.0);
            out_palette[i] = QColor((int)(r),(int)(g),(int)(b));
        }
    }
    emit changed();
    emit changed_ppumask();
}

void NESPalette::slot_ppum_r()
{
    set_ppu_mask(PPUMask::Red);
}

void NESPalette::slot_ppum_g()
{
    set_ppu_mask(PPUMask::Green);
}

void NESPalette::slot_ppum_b()
{
    set_ppu_mask(PPUMask::Blue);
}

void NESPalette::slot_ppum_m()
{
    set_ppu_mask(PPUMask::Grayscale);
}
