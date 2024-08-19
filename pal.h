#pragma once
#ifndef PAL_H
#define PAL_H

#include <QObject>
#include "bgpaldefault.h"

class PAL : public QObject
{
    Q_OBJECT
public:
    explicit PAL(QObject *parent = nullptr);
    const int current_palette() const { return entryActive/4; }
    void set_current_palette(int pal) { entryActive = pal * 4 + current_relative(); emit changed(); }
    //int &current_palette() { emit changed(); return palActive; }
    const int current_index() const { return entryActive; }
    void set_current_index(int x) { entryActive = x; emit changed(); }
    const int current_color() const { return data[entryActive]; }
    void load_default_palette(BGPalette);
    const char operator[](size_t i) const { return data[i]; }
    const int current_scale() const { return scale; }
    const int current_relative() const { return entryActive % 4; }
    void set_current_relative(int x) { entryActive &= ~3; entryActive += x; emit changed(); }
    void switch_palette(BGPalette n);
    const int size() const { return palSize; }
signals:
    void changed();
    void changed_palette(int);
    void changed_background();
    void switched_palette();
    void load();
private slots:
    void slot_change_color(int color);
private:
    static constexpr int palSize = 16;
    BGPalette palActive = BGPalette::A;
    unsigned char data[16];
    std::vector<std::vector<uchar>> palettes;
    int entryActive;
    int scale = 20;
};

#endif // PAL_H
