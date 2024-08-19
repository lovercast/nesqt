#include "chrrenderer.h"
#include <QColor>
#include <QImage>

CHRRenderer::CHRRenderer(const CHR &chr, const PAL &pal, const NESPalette &nes_palette, QObject *parent)
    : QObject{parent}
    , chr{chr}
    , pal{pal}
    , nes_palette{nes_palette}
    , image_{new QImage}
{
    connect(&chr, SIGNAL (changed()), this, SLOT (update()));
    connect(&pal, SIGNAL (changed()), this, SLOT (update()));
    connect(&nes_palette, SIGNAL (changed()), this, SLOT (update()));
    update();
}

void CHRRenderer::update()
{
    const auto& map = chr.map();
    uchar header[] = {'P','6',' ','1','2','8',' ','1','2','8',' ','2','5','5',' '};
    uchar data[sizeof(header) + 64*256*3];
    std::copy(header, header+sizeof(header), data);
    uchar *offset = &data[sizeof(header)];
    for (int i = 0; i < chr.bank_size(); ++i) {
        auto color = nes_palette[pal[pal.current_palette()*4 + map[i]]];
        auto r = static_cast<uchar>(color.red());
        auto g = static_cast<uchar>(color.green());
        auto b = static_cast<uchar>(color.blue());

        offset[i*3 + 0] = r;
        offset[i*3 + 1] = g;
        offset[i*3 + 2] = b;
    }
    image_->loadFromData(data, sizeof(data));
    emit changed();
}

