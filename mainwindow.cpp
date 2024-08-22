#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <format>
#include <QLabel>
#include <QClipboard>
#include <QFileDialog>
#include "chr.h"
#include "palview.h"
#include "chrrenderer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , chr(new CHR)
    , tab_widget {new QTabWidget}
    , vlayout_tab_status {new QVBoxLayout}
    , vlayout_tile_pal {new QVBoxLayout}
    , gb_tileset {new QGroupBox}
    , gb_palettes {new QGroupBox}
    , status_bar {new QStatusBar}
    , hlayout_palettes {new QHBoxLayout}
    , vlayout_palettes {new QVBoxLayout}
    , vlayout_tileset {new QVBoxLayout}
    , nes_palette {new NESPalette}
    , pal {new PAL}
    , palview_grid {new QGridLayout}
    , tb_ppumask {new QToolBar}
    , gb_ppumask {new QGroupBox}
    , hlayout_ppumask {new QHBoxLayout}
    , button_ppum_r {new QToolButton}
    , button_ppum_g {new QToolButton}
    , button_ppum_b {new QToolButton}
    , button_ppum_m {new QToolButton}
    , toolbar_bg_palette {new QToolBar}
    , hlayout_apply_ppum {new QHBoxLayout}
    , check_apply_palettes {new QCheckBox}
    , check_apply_tiles {new QCheckBox}
    , vlayout_bg_pal_default {new QVBoxLayout}
    , chr_renderer {new CHRRenderer(*chr, *pal, *nes_palette)}
    , chr_view {new CHRView(*chr,*chr_renderer)}
    , attr {new ATTR}
    , nametable {new Nametable}
    , nametable_renderer {new NametableRenderer(*nametable, *chr, *attr, *pal, *nes_palette, *chr_renderer)}
    , nametable_view {new NametableView(*nametable_renderer)}
    , glayout_tileset_menu {new QGridLayout}
    , tb_chrbank {new QToolBar(tr("Pattern table"))}
    , tb_display_grid {new QToolBar(tr("Grid"))}
    , check_attr_checker {new QCheckBox(tr("Attr checker"))}
    , check_selected_only {new QCheckBox(tr("Selected only"))}
    , check_type_in {new QCheckBox(tr("Type in"))}
    , tool_button_palette_a {new QToolButton()}
    , tool_button_palette_b {new QToolButton()}
    , tool_button_palette_c {new QToolButton()}
    , tool_button_palette_d {new QToolButton()}
{
    ui->setupUi(this);

    chr_editor = new CHREditorWidget(*chr,*pal,*nes_palette);
    nes_palette_view = new NESPaletteView(nullptr, *nes_palette, *pal);
    pal0_view = new PALView(nullptr,*nes_palette,*pal,0);
    pal1_view = new PALView(nullptr,*nes_palette,*pal,1);
    pal2_view = new PALView(nullptr,*nes_palette,*pal,2);
    pal3_view = new PALView(nullptr,*nes_palette,*pal,3);
    tab_widget->addTab(nametable_renderer, "Nametable");
    ui->horizontalLayout->addLayout(vlayout_tab_status);
    ui->horizontalLayout->addLayout(vlayout_tile_pal);
    vlayout_tab_status->addWidget(tab_widget);
    vlayout_tab_status->addWidget(status_bar);
    auto font = new QFont("Monospace");
    font->setStyleHint(QFont::Monospace);
    status_bar->setFont(*font);
    gb_palettes->setTitle("Palettes");
    vlayout_tile_pal->addWidget(gb_tileset,0);
    vlayout_tile_pal->addWidget(gb_palettes,1);
    init_tileset_menu();
    init_palette_menu();
    init_menu_actions();
    setStatusBar(status_bar);

    connect(chr_view, SIGNAL (doubleClicked()), this, SLOT (slotCHREditor()));
    connect(nametable_renderer, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));

}

void MainWindow::init_menu_actions()
{
    connect(ui->actionViewCHREditor, SIGNAL(triggered()), this, SLOT(slotCHREditor()));
    connect(ui->actionSwap_banks, SIGNAL (triggered()), chr, SLOT (slot_swap_banks()));
    connect(ui->action_selection_to_c_code, SIGNAL (triggered()), this, SLOT (slot_selection_to_c_code()));
    connect(ui->action_selection_to_c_rle, SIGNAL (triggered()), this, SLOT (slot_selection_to_c_rle()));
    connect(ui->action_selection_to_asm, SIGNAL(triggered()), this, SLOT(slot_selection_to_asm()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(slot_open()));
}

void MainWindow::init_tileset_menu()
{
    gb_tileset->setTitle("Tileset");
    gb_tileset->setLayout(vlayout_tileset);
    vlayout_tileset->addWidget(chr_view);
    vlayout_tileset->addLayout(glayout_tileset_menu);
    glayout_tileset_menu->addWidget(check_apply_tiles, 0,0);
    glayout_tileset_menu->addWidget(tb_chrbank, 0,1);
    glayout_tileset_menu->addWidget(check_attr_checker, 1,0);
    glayout_tileset_menu->addWidget(check_selected_only, 1,1);
    glayout_tileset_menu->addWidget(check_type_in, 2,0);
    glayout_tileset_menu->addWidget(tb_display_grid, 2,1);

    check_apply_tiles->setText(tr("Apply tiles"));
    check_apply_tiles->setChecked(true);
    check_apply_tiles->setToolTip(tr("Enable tile drawing in the nametable"));
    connect(check_apply_tiles, SIGNAL (stateChanged(int)), this, SLOT (slot_check_apply_tiles(int)));
    check_attr_checker->setChecked(false);
    check_attr_checker->setToolTip(tr("[A] Show checker grid to see attributes in the nametable"));
    connect(check_attr_checker, SIGNAL (stateChanged(int)), this, SLOT (slot_check_attr_checker(int)));
    check_selected_only->setChecked(false);
    check_selected_only->setToolTip(tr("Display only selected tiles in the nametable"));
    connect(check_selected_only, SIGNAL (stateChanged(int)), this, SLOT (slot_check_selected_only(int)));

    auto label_chrbank = new QLabel(tr("Pattern table "));
    tb_chrbank->addWidget(label_chrbank);
    auto act_chrbank_a = tb_chrbank->addAction(tr("A"));
    auto act_chrbank_b = tb_chrbank->addAction(tr("B"));
    act_chrbank_a->setToolTip(tr("Select first 4K of 8K CHR"));
    act_chrbank_b->setToolTip(tr("Select second 4K of 8K CHR"));
    connect(act_chrbank_a, SIGNAL (triggered(bool)), this, SLOT(slot_swap_banks(bool)));
    connect(act_chrbank_b, SIGNAL (triggered(bool)), this, SLOT(slot_swap_banks(bool)));
    act_chrbank_a->setCheckable(true);
    act_chrbank_b->setCheckable(true);
    act_chrbank_a->setChecked(true);
    static_cast<QToolButton*>(tb_chrbank->widgetForAction(act_chrbank_a))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_chrbank->widgetForAction(act_chrbank_b))->setAutoRaise(false);

    auto button_group_chrbank = new QButtonGroup;
    button_group_chrbank->setExclusive(true);
    button_group_chrbank->addButton(static_cast<QToolButton*>(tb_chrbank->widgetForAction(act_chrbank_a)));
    button_group_chrbank->addButton(static_cast<QToolButton*>(tb_chrbank->widgetForAction(act_chrbank_b)));

    auto label_display_grid = new QLabel(tr("Grid "));
    tb_display_grid->addWidget(label_display_grid);
    act_grid_all = tb_display_grid->addAction(tr("All"));
    act_grid_1 = tb_display_grid->addAction(tr("1x"));
    act_grid_2 = tb_display_grid->addAction(tr("2x"));
    act_grid_4 = tb_display_grid->addAction(tr("4x"));

    connect(act_grid_all, SIGNAL (triggered(bool)), this, SLOT (slot_grid_all(bool)));
    connect(act_grid_1, SIGNAL (triggered(bool)), this, SLOT (slot_grid_1(bool)));
    connect(act_grid_2, SIGNAL (triggered(bool)), this, SLOT (slot_grid_2(bool)));
    connect(act_grid_4, SIGNAL (triggered(bool)), this, SLOT (slot_grid_4(bool)));

    act_grid_all->setCheckable(true);
    act_grid_1->setCheckable(true);
    act_grid_2->setCheckable(true);
    act_grid_4->setCheckable(true);
    static_cast<QToolButton*>(tb_display_grid->widgetForAction(act_grid_all))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_display_grid->widgetForAction(act_grid_1))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_display_grid->widgetForAction(act_grid_2))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_display_grid->widgetForAction(act_grid_4))->setAutoRaise(false);
}

void MainWindow::init_palette_menu()
{
    gb_palettes->setLayout(hlayout_palettes);
    hlayout_palettes->addLayout(vlayout_palettes);
    vlayout_palettes->addLayout(palview_grid);
    vlayout_palettes->addWidget(nes_palette_view);
    vlayout_palettes->setSpacing(2);
    connect(nes_palette_view, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));
    palview_grid->addWidget(pal0_view,0,0);
    palview_grid->addWidget(pal1_view,0,1);
    palview_grid->addWidget(pal2_view,1,0);
    palview_grid->addWidget(pal3_view,1,1);
    palview_grid->setSpacing(2);
    connect(pal0_view, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));
    connect(pal1_view, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));
    connect(pal2_view, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));
    connect(pal3_view, SIGNAL (signal_status(QString,int)), this, SLOT (slot_status(QString,int)));
    connect(ui->action_Pal_A, SIGNAL (triggered()), this, SLOT (slot_default_palette_A()));
    connect(ui->action_Pal_B, SIGNAL (triggered()), this, SLOT (slot_default_palette_B()));
    connect(ui->action_Pal_C, SIGNAL (triggered()), this, SLOT (slot_default_palette_C()));
    connect(ui->action_Pal_D, SIGNAL (triggered()), this, SLOT (slot_default_palette_D()));
    connect(ui->action_Pal_Grayscale, SIGNAL (triggered()), this, SLOT (slot_default_palette_Grayscale()));
    connect(pal, SIGNAL (changed()), pal0_view, SLOT (update()));
    connect(pal, SIGNAL (changed()), pal1_view, SLOT (update()));
    connect(pal, SIGNAL (changed()), pal2_view, SLOT (update()));
    connect(pal, SIGNAL (changed()), pal3_view, SLOT (update()));
    connect(nes_palette, SIGNAL (changed()), pal0_view, SLOT (update()));
    connect(nes_palette, SIGNAL (changed()), pal1_view, SLOT (update()));
    connect(nes_palette, SIGNAL (changed()), pal2_view, SLOT (update()));
    connect(nes_palette, SIGNAL (changed()), pal3_view, SLOT (update()));

    auto act_ppum_r = tb_ppumask->addAction(tr("R"), [&](){nes_palette->set_ppu_mask(PPUMask::Red);});
    auto act_ppum_g = tb_ppumask->addAction(tr("G"), [&](){nes_palette->set_ppu_mask(PPUMask::Green);});
    auto act_ppum_b = tb_ppumask->addAction(tr("B"), [&](){nes_palette->set_ppu_mask(PPUMask::Blue);});
    auto act_ppum_m = tb_ppumask->addAction(tr("M"), [&](){nes_palette->set_ppu_mask(PPUMask::Grayscale);});
    static_cast<QToolButton*>(tb_ppumask->widgetForAction(act_ppum_r))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_ppumask->widgetForAction(act_ppum_g))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_ppumask->widgetForAction(act_ppum_b))->setAutoRaise(false);
    static_cast<QToolButton*>(tb_ppumask->widgetForAction(act_ppum_m))->setAutoRaise(false);
    act_ppum_r->setCheckable(true);
    act_ppum_g->setCheckable(true);
    act_ppum_b->setCheckable(true);
    act_ppum_m->setCheckable(true);
    act_ppum_r->setToolTip(tr("Color emphasis for red channel"));
    act_ppum_g->setToolTip(tr("Color emphasis for green channel"));
    act_ppum_b->setToolTip(tr("Color emphasis for blue channel"));
    act_ppum_m->setToolTip(tr("Grayscale mode"));
    tb_ppumask->setToolButtonStyle(Qt::ToolButtonTextOnly);
    hlayout_apply_ppum->addWidget(check_apply_palettes);
    hlayout_apply_ppum->addWidget(tb_ppumask);
    check_apply_palettes->setText(tr("Apply palettes"));
    check_apply_palettes->setToolTip(tr("Enable palette drawing in the nametable"));
    check_apply_palettes->setChecked(true);
    connect(check_apply_palettes, SIGNAL (stateChanged(int)), this, SLOT (slot_check_apply_palettes(int)));
    vlayout_palettes->addLayout(hlayout_apply_ppum);

    auto button_group_palettes = new QButtonGroup;

    hlayout_palettes->addLayout(vlayout_bg_pal_default);

    vlayout_bg_pal_default->addWidget(toolbar_bg_palette);

    toolbar_bg_palette->setOrientation(Qt::Vertical);
    auto act_palette_a = toolbar_bg_palette->addAction(tr("A"),[&](){pal->switch_palette(BGPalette::A);});
    auto act_palette_b = toolbar_bg_palette->addAction(tr("B"),[&](){pal->switch_palette(BGPalette::B);});
    auto act_palette_c = toolbar_bg_palette->addAction(tr("C"),[&](){pal->switch_palette(BGPalette::C);});
    auto act_palette_d = toolbar_bg_palette->addAction(tr("D"),[&](){pal->switch_palette(BGPalette::D);});
    static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_a))->setAutoRaise(false);
    static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_b))->setAutoRaise(false);
    static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_c))->setAutoRaise(false);
    static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_d))->setAutoRaise(false);
    act_palette_a->setCheckable(true);
    act_palette_b->setCheckable(true);
    act_palette_c->setCheckable(true);
    act_palette_d->setCheckable(true);
    act_palette_a->setChecked(true);
    button_group_palettes->setExclusive(true);
    button_group_palettes->addButton(static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_a)));
    button_group_palettes->addButton(static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_b)));
    button_group_palettes->addButton(static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_c)));
    button_group_palettes->addButton(static_cast<QToolButton*>(toolbar_bg_palette->widgetForAction(act_palette_d)));
}


MainWindow::~MainWindow()
{
    delete status_bar;
    delete gb_palettes;
    delete gb_tileset;
    delete vlayout_tile_pal;
    delete vlayout_tab_status;
    delete tab_widget;
    delete chr_editor;
    delete chr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    chr_editor->close();
    event->accept();
}

void MainWindow::slotCHREditor()
{
    if (chr_editor->isVisible()) {
        chr_editor->raise();
    } else {
        chr_editor->show();
    }
};

void MainWindow::slot_status(QString msg, int timeout = 0)
{
    statusBar()->showMessage(msg,timeout);
}

void MainWindow::slot_default_palette_A()
{

    pal->load_default_palette(BGPalette::A);
}

void MainWindow::slot_default_palette_B()
{
    pal->load_default_palette(BGPalette::B);
}

void MainWindow::slot_default_palette_C()
{
    pal->load_default_palette(BGPalette::C);
}

void MainWindow::slot_default_palette_D()
{
    pal->load_default_palette(BGPalette::D);
}

void MainWindow::slot_default_palette_Grayscale()
{
    pal->load_default_palette(BGPalette::Grayscale);
}

void MainWindow::slot_palette_A(bool checked)
{
    if (checked)
        pal->switch_palette(BGPalette::A);
}

void MainWindow::slot_palette_B(bool checked)
{
    if (checked)
        pal->switch_palette(BGPalette::B);
}

void MainWindow::slot_palette_C(bool checked)
{
    if (checked)
        pal->switch_palette(BGPalette::C);
}

void MainWindow::slot_palette_D(bool checked)
{
    if (checked)
        pal->switch_palette(BGPalette::D);
}

void MainWindow::slot_check_apply_palettes(int state)
{
    switch (state) {
    case Qt::Unchecked:
        nametable_renderer->apply_palette() = false;
        break;
    case Qt::PartiallyChecked:
    case Qt::Checked:
        nametable_renderer->apply_palette() = true;
        break;
    }
}

void MainWindow::slot_check_apply_tiles(int state)
{
    switch (state) {
    case Qt::Unchecked:
        nametable_renderer->apply_tile() = false;
        break;
    case Qt::PartiallyChecked:
    case Qt::Checked:
        nametable_renderer->apply_tile() = true;
        break;
    }
}

void MainWindow::slot_swap_banks(bool checked)
{
    if (checked)
        chr->swap_banks();
}

void MainWindow::slot_grid_all(bool checked)
{
    if (act_grid_1->isChecked() != checked)
        act_grid_1->trigger();
    if (act_grid_2->isChecked() != checked)
        act_grid_2->trigger();
    if (act_grid_4->isChecked() != checked)
        act_grid_4->trigger();
}

void MainWindow::slot_grid_1(bool checked)
{
    if (!checked && act_grid_all->isChecked()) {
        act_grid_all->setChecked(false);
    } else if (checked && act_grid_2->isChecked() && act_grid_4->isChecked()) {
        act_grid_all->setChecked(true);
    }
    chr_view->set_grid_size(NESQT::GridSize::OneByOne);
    nametable_renderer->set_grid_size(NESQT::GridSize::OneByOne, checked);
}

void MainWindow::slot_grid_2(bool checked)
{
    if (!checked && act_grid_all->isChecked()) {
        act_grid_all->setChecked(false);
    } else if (checked && act_grid_1->isChecked() && act_grid_4->isChecked()) {
        act_grid_all->setChecked(true);
    }
    chr_view->set_grid_size(NESQT::GridSize::TwoByTwo);
    nametable_renderer->set_grid_size(NESQT::GridSize::TwoByTwo, checked);
}

void MainWindow::slot_grid_4(bool checked)
{
    if (!checked && act_grid_all->isChecked()) {
        act_grid_all->setChecked(false);
    } else if (checked && act_grid_1->isChecked() && act_grid_2->isChecked()) {
        act_grid_all->setChecked(true);
    }
    chr_view->set_grid_size(NESQT::GridSize::FourByFour);
    nametable_renderer->set_grid_size(NESQT::GridSize::FourByFour, checked);
}

void MainWindow::slot_check_attr_checker(int checkState)
{
    if (checkState == Qt::Checked && check_selected_only->checkState() == Qt::Checked) {
        check_selected_only->setCheckState(Qt::Unchecked);
    }
    nametable_renderer->slot_attr_checker(checkState);
}

void MainWindow::slot_check_selected_only(int checkState)
{
    if (checkState == Qt::Checked && check_attr_checker->checkState() == Qt::Checked) {
        check_attr_checker->setCheckState(Qt::Unchecked);
    }
    nametable_renderer->slot_selected_only(checkState);
}

void MainWindow::slot_selection_to_c_code()
{
    QString s = nametable_renderer->as_c_code(true, false);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(s);
}

void MainWindow::slot_selection_to_c_rle()
{
    QString s = nametable_renderer->as_c_code(true, true);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(s);
}

void MainWindow::slot_selection_to_asm()
{
    QString s = nametable_renderer->as_6502_asm(true);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(s);
}

void MainWindow::slot_open()
{
    QString filter(tr("All supported files (*.chr *.bin *.nam *map *.rle *.pal)"));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), filter);

    // Check what type of file and validate
    QString ext;
    auto q = nullptr;
    for (auto c : fileName) {
        if (c == '.') {
            ext.clear();
        }
        ext.push_back(c);
    }
    if (ext.isEmpty() || !ext.startsWith('.')) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 tr("Invalid filename"));
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }

    if (ext == ".chr") {
        NESQT::not_implemented();
    } else if (ext == ".bin") {
        NESQT::not_implemented();
    } else if (ext == ".nam") {
        NESQT::not_implemented();
    } else if (ext == ".map") {
        NESQT::not_implemented();
    } else if (ext == ".rle") {
        NESQT::not_implemented();
    } else if (ext == ".pal") {
        NESQT::not_implemented();
    } else {
        QMessageBox::information(this, tr("Unable to open file"),
                                 tr(std::format("Invalid file extension: {}", ext.toStdString()).c_str()));
    }
}

void MainWindow::slot_warn()
{
    NESQT::warn("warn!");
}
