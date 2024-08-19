#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QGroupBox>
#include <QButtonGroup>
#include <QToolButton>
#include <QCheckBox>
#include "nametable.h"
#include "nametablerenderer.h"
#include "nametableview.h"
#include "chreditorwidget.h"
#include "nespaletteview.h"
#include "pal.h"
#include "palview.h"
#include "nespalette.h"
#include "chrrenderer.h"
#include "chrview.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
//    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void buttonClicked();

private slots:
    void slotCHREditor();
    void slot_status(QString msg, int timeout);
    void slot_default_palette_A();
    void slot_default_palette_B();
    void slot_default_palette_C();
    void slot_default_palette_D();
    void slot_default_palette_Grayscale();
    void slot_check_apply_tiles(int state);
    void slot_check_apply_palettes(int state);
    void slot_palette_A(bool);
    void slot_palette_B(bool);
    void slot_palette_C(bool);
    void slot_palette_D(bool);
    void slot_swap_banks(bool checked);
    void slot_grid_all(bool);
    void slot_grid_1(bool);
    void slot_grid_2(bool);
    void slot_grid_4(bool);
    void slot_check_attr_checker(int checkState);
    void slot_check_selected_only(int checkState);
    void slot_selection_to_c_code();
    void slot_selection_to_c_rle();
    void slot_selection_to_asm();
    void slot_warn();

private:
    Ui::MainWindow *ui;
    CHR *chr;
    CHREditorWidget *chr_editor;
    QTabWidget *tab_widget;
    QVBoxLayout *vlayout_tile_pal;
    QVBoxLayout *vlayout_tab_status;
    QGroupBox *gb_tileset;
    QGroupBox *gb_palettes;
    QStatusBar *status_bar;
    QHBoxLayout *hlayout_palettes;
    QVBoxLayout *vlayout_palettes;
    QVBoxLayout *vlayout_tileset;
    NESPaletteView *nes_palette_view;
    PAL *pal;
    PALView *pal_view;
    QGridLayout *palview_grid;
    PALView *pal0_view;
    PALView *pal1_view;
    PALView *pal2_view;
    PALView *pal3_view;
    NESPalette *nes_palette;
    QToolBar *tb_ppumask;
    QHBoxLayout *hlayout_ppumask;
    QGroupBox *gb_ppumask;
    QToolButton *button_ppum_r;
    QToolButton *button_ppum_g;
    QToolButton *button_ppum_b;
    QToolButton *button_ppum_m;
    QToolBar *toolbar_bg_palette;
    QHBoxLayout *hlayout_apply_ppum;
    QCheckBox *check_apply_palettes;
    QCheckBox *check_apply_tiles;
    QVBoxLayout *vlayout_bg_pal_default;
    CHRRenderer *chr_renderer;
    CHRView *chr_view;
    ATTR *attr;
    Nametable *nametable;
    NametableRenderer *nametable_renderer;
    NametableView *nametable_view;

    QGridLayout *glayout_tileset_menu;
    QToolBar *tb_chrbank;
    QToolBar *tb_display_grid;

    QCheckBox *check_attr_checker;
    QCheckBox *check_selected_only;
    QCheckBox *check_type_in;
    QToolButton *tool_button_palette_a;
    QToolButton *tool_button_palette_b;
    QToolButton *tool_button_palette_c;
    QToolButton *tool_button_palette_d;
    QAction *act_grid_all;
    QAction *act_grid_1;
    QAction *act_grid_2;
    QAction *act_grid_4;
private:
    void init_tileset_menu();
    void init_palette_menu();
    void init_menu_actions();
};

void palette_calc(void);
#endif // MAINWINDOW_H
