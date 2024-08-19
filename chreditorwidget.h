#pragma once
#ifndef CHREDITORWIDGET_H
#define CHREDITORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QToolBar>
#include "chr.h"
#include "pal.h"
#include "nespalette.h"
#include "chreditview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CHREditorWidget; }
QT_END_NAMESPACE

class CHREditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CHREditorWidget(CHR &chr, PAL &pal, NESPalette &nes_palette, QWidget *parent = nullptr);

signals:
private:
    void actionHFLIP();
    void actionVFLIP();
    void actionCW();
    void actionCCW();
private:
    QVBoxLayout* window_layout;
    QFrame* frame;
    QVBoxLayout* frame_layout;
    CHR &chr;
    PAL &pal;
    NESPalette &nes_palette;
    CHREditView* chrview;
    QToolBar* tool_bar;
};

#endif // CHREDITORWIDGET_H
