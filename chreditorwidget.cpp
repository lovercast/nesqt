#include "chreditorwidget.h"

using namespace std;

CHREditorWidget::CHREditorWidget(CHR &chr, PAL &pal, NESPalette &nes_palette, QWidget *parent)
    : QWidget(parent)
    , window_layout {new QVBoxLayout(this)}
    , frame {new QFrame()}
    , frame_layout {new QVBoxLayout(frame)}
    , chr{chr}
    , pal{pal}
    , nes_palette{nes_palette}
    , chrview {new CHREditView(chr,pal,nes_palette)}
    , tool_bar {new QToolBar()}
{
    resize(500,600); /* TODO: find optimal starting size */
    setWindowTitle(tr("CHR Editor"));
    setLayout(window_layout);
    window_layout->setMargin(4);
    frame->setFrameShape(QFrame::Box);
    frame->setFrameShadow(QFrame::Sunken);
    frame_layout->setMargin(0);
    frame_layout->setSpacing(0);
    frame->setLayout(frame_layout);
    window_layout->addWidget(frame);
    frame_layout->addWidget(chrview);

    auto act_hf = tool_bar->addAction(tr("HFLIP"), [&](){actionHFLIP();});
    auto act_vf = tool_bar->addAction(tr("VFLIP"), [&](){actionVFLIP();});
    auto act_cw = tool_bar->addAction(tr("CW"), [&](){actionCW();});
    auto act_cc = tool_bar->addAction(tr("CCW"), [&](){actionCCW();});
    act_hf->setToolTip(tr("Mirror horizontally"));
    act_vf->setToolTip(tr("Mirror vertically"));
    act_cw->setToolTip(tr("Rotate clockwise"));
    act_cc->setToolTip(tr("Rotate counterclockwise"));
    window_layout->addWidget(tool_bar, Qt::AlignCenter);
    connect(&pal, SIGNAL (changed()), chrview, SLOT (update()));
}


void CHREditorWidget::actionHFLIP()
{
    chr.mirror(true);
}

void CHREditorWidget::actionVFLIP()
{
    chr.mirror(false);
}

void CHREditorWidget::actionCW()
{
    chr.rotate(true);
}

void CHREditorWidget::actionCCW()
{
    chr.rotate(false);
}
