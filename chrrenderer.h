#pragma once
#ifndef CHRRENDERER_H
#define CHRRENDERER_H

#include <QObject>
#include <QPainter>
#include "chr.h"
#include "pal.h"
#include "nespalette.h"

class CHRRenderer : public QObject
{
    Q_OBJECT
public:
    explicit CHRRenderer(const CHR &chr,
                         const PAL &pal,
                         const NESPalette &nes_palette,
                         QObject *parent = nullptr);
    const QImage& image() const {return *image_;}

signals:
    void changed();
    void changed_idx(int);
private slots:
    void update(void);
private:
    /**
     * @brief Update the tile at index.
     */
    std::unique_ptr<QPixmap> render_tile(int idx);
private:
    const CHR &chr;
    const PAL &pal;
    const NESPalette &nes_palette;
    QImage *image_;
};

#endif // CHRRENDERER_H
