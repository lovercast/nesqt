#pragma once
#ifndef NAMETABLERENDERER_H
#define NAMETABLERENDERER_H

#include <QWidget>
#include <QImage>
#include <QBasicTimer>
#include <QMouseEvent>
#include <QUndoStack>
#include <queue>
#include "nametable.h"
#include "attr.h"
#include "nespalette.h"
#include "pal.h"
#include "chr.h"
#include "chrrenderer.h"
#include "NesQt.h"

class NametableRenderer : public QWidget
{
    Q_OBJECT
public:
    enum class ToolType {
        Pen,
        Selector,
        Eyedropper,
    };
    using TileCoord = QPair<int,int>;
public:
    explicit NametableRenderer(Nametable &nametable, CHR &chr, ATTR &attr, PAL &pal, NESPalette &nes_palette, CHRRenderer &chr_renderer, QWidget *parent = nullptr);
    const QImage &image() const { return *m_image; }
    void render(QPainter &painter);
    bool apply_palette() const { return traits.apply_palette; }
    bool &apply_palette() { return traits.apply_palette; }
    bool apply_tile() const { return traits.apply_tile; }
    bool &apply_tile() { return traits.apply_tile; }
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void set_grid_size(NESQT::GridSize, bool);
    void set_tool(ToolType type);
    QString as_c_code(bool selection, bool rle);
    QString as_6502_asm(bool selection);
    void load_from_file(QString filename);

signals:
    void changed();
    void signal_status(QString, int);

private slots:
    void update_xy(int, int, uchar, uchar);
    void update_attr_block(int, int, uchar, uchar);
    void update_tileno(int);
    void update_palno(int);
//    void update_selection();
    void update_all();
    void slot_check_selected_only();
public slots:
    void slot_attr_checker(int);
    void slot_selected_only(int);

private:
    void init_maps();
    void init_tools();
    void render_tile(int tx, int ty);
    void left_mouse_handler(int x, int y, int modifiers);
    void middle_mouse_handler(int x, int y, int modifiers);
    void selection_handler(int x, int y);
    void redo_handler();
    void undo_handler();
    void fill_handler();

    void draw_grids(QPainter &);
    void draw_selection(QPainter &);

    QString compose_status(int x, int y);
    QVector<QPainter::PixmapFragment> make_pixmap_fragments(const QPixmap &pm);
    std::vector<TileCoord> calculate_fill(int tx, int ty, uchar t_new);
private:
    QImage *m_image;
    QImage *m_attr_checker;
    QBasicTimer m_timer;
    Nametable &nametable;
    CHR &chr;
    ATTR &attr;
    PAL &pal;
    NESPalette &nes_palette;
    CHRRenderer &chr_renderer;
    struct EditorTraits {
        bool apply_palette = true;
        bool apply_tile = true;
        bool attr_checker = false;
        bool selected_only = false;
        NESQT::GridSize gridsize = NESQT::GridSize::None;
        ToolType tool = ToolType::Pen;
        ToolType last_tool;
    } traits;
    struct EditorToolsInternal {
        QPen selector_pen;
        qreal selector_offset = 0;
        static constexpr qreal selector_interval = 1;
        static constexpr int timer_interval = 60;
        static constexpr qreal selector_dash_len = 2;
    } intern;
    std::queue<TileCoord> redraw_queue;
    std::vector<std::vector<TileCoord>> tileno_2_coord_map;
    std::vector<std::vector<TileCoord>> palno_2_coord_map;
    int viewport_width = 32;
    int viewport_height = 32;
    int m_width = 32*24;
    int m_height = 32*24;

    unsigned int undo_count = 0;
    QUndoStack *undo_stack;

    enum class StrokeType;
    struct StrokeCommand;
    struct FillCommand;
    using TileMapCommand = FillCommand;
    struct EyedropperCommand;
    struct SelectCommand {
        int x0;
        int y0;
        int x1;
        int y1;
    };
    class StrokeUndo;
    class FillUndo;
    class EyedropperUndo;
    class SelectUndo;
    class SelectFillUndo;
    class SelectCancelUndo;
    class PasteUndo;

    struct Selector {
        SelectCommand current;
        bool is_valid = false;
        bool has_moved = false;
        bool is_drawing = false;
        void reset()
        {
            current = {};
            is_valid = false;
            has_moved = false;
            is_drawing = false;
        }

        void update(int x, int y)
        {
            if (is_drawing) {
                if (x != current.x1) {
                    current.x1 = x;
                }
                if (y != current.y1) {
                    current.y1 = y;
                }
            } else {
                current = SelectCommand{x,y,x,y};
                is_valid = true;
                is_drawing = true;
            }
        }
    } selector, back;

};

#endif // NAMETABLERENDERER_H
