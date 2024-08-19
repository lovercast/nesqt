#pragma once
#ifndef NESPALETTE_H
#define NESPALETTE_H
#include <QObject>

enum class PPUMask {
    Grayscale = 0x01,
    Blue = 0x20,
    Green = 0x40,
    Red = 0x80,
};

class NESPalette : public QObject
{
    Q_OBJECT
public:
    explicit NESPalette(QObject *parent = nullptr);
    const QColor& operator[](size_t i) const {return out_palette[i];}
    const auto size() const { return out_palette.size(); }
    void set_ppu_mask(PPUMask mask);
signals:
    void changed();
    void changed_ppumask();
private slots:
    void slot_ppum_r();
    void slot_ppum_g();
    void slot_ppum_b();
    void slot_ppum_m();
private:
    QVector<QColor> base_palette;
    QVector<QColor> out_palette;
    int ppu_mask;
};

#endif // NESPALETTE_H
