#include <QPainter>
#include <QFile>
#include "NesQt.h"
#include "nametablerenderer.h"
#include "nametableundo.h"
#include "rle.h"

using TileCoord = NametableRenderer::TileCoord;
using NESQT::GridSize;

NametableRenderer::NametableRenderer(Nametable &nametable, CHR &chr, ATTR &attr, PAL &pal, NESPalette &nes_palette, CHRRenderer &chr_renderer, QWidget *parent)
    : QWidget{parent}
    , nametable{nametable}
    , chr{chr}
    , attr{attr}
    , pal{pal}
    , nes_palette{nes_palette}
    , chr_renderer{chr_renderer}
    , m_image{new QImage(256,256,QImage::Format_RGB32)}
    , m_attr_checker{new QImage(64,64,QImage::Format_RGB32)}
    , redraw_queue{std::queue<TileCoord>()}
    , tileno_2_coord_map{std::vector<std::vector<TileCoord>>(1024)}
    , palno_2_coord_map{std::vector<std::vector<TileCoord>>(4)}
    , undo_stack{new QUndoStack}
    , m_timer{}
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(m_width,m_height);
    init_maps();
    init_tools();

    connect(&chr, SIGNAL (load()), this, SLOT (update_all()));
    connect(&chr, SIGNAL (changed_idx(int)), this, SLOT (update_tileno(int)));
    connect(&chr, SIGNAL (signal_swap_banks()), this, SLOT (update_all()));
    connect(&chr, SIGNAL (changed_current()), this, SLOT (slot_check_selected_only()));
    connect(&pal, SIGNAL (changed_palette(int)), this, SLOT (update_palno(int)));
    connect(&pal, SIGNAL (changed_background()), this, SLOT (update_all()));
    connect(&pal, SIGNAL (load()), this, SLOT (update_all()));
    connect(&nes_palette, SIGNAL (changed_ppumask()), this, SLOT (update_all()));
}

void NametableRenderer::init_maps()
{
    for (auto y = 0; y < 32; ++y) {
        for (auto x = 0; x < 32; ++x) {
            tileno_2_coord_map[0].push_back(TileCoord{x,y});
            if (x % 2 == 0 && y % 2 == 0) {
                palno_2_coord_map[0].push_back(TileCoord{x,y});
            }
        }
    }
}

void NametableRenderer::init_tools()
{
    intern.selector_pen = QPen{Qt::white};
    intern.selector_pen.setStyle(Qt::CustomDashLine);
    intern.selector_pen.setDashPattern({intern.selector_dash_len,2*intern.selector_dash_len});
    intern.selector_pen.setWidth(1);
    m_timer.start(EditorToolsInternal::timer_interval, this);
}

void NametableRenderer::render_tile(int tx, int ty)
{
    auto tileno = nametable.get(tx,ty);
    auto palette = attr.get(tx,ty);
    auto tile = chr.get_tile(tileno); /* TODO: cache used tiles */
    for (auto y = 0; y < 8; ++y) {
        for (auto x = 0; x < 8; ++x) {
            auto color = nes_palette[pal[palette*4 + tile[y*8+x]]];
            m_image->setPixelColor(QPoint(tx*8+x,ty*8+y),color);
        }
    }
    if (traits.attr_checker) {
        m_attr_checker->setPixelColor(QPoint(tx*2,      ty*2),nes_palette[pal[palette*4 + 0]]);
        m_attr_checker->setPixelColor(QPoint(tx*2 + 1,  ty*2),nes_palette[pal[palette*4 + 1]]);
        m_attr_checker->setPixelColor(QPoint(tx*2,      ty*2 + 1),nes_palette[pal[palette*4 + 2]]);
        m_attr_checker->setPixelColor(QPoint(tx*2 + 1,  ty*2 + 1),nes_palette[pal[palette*4 + 3]]);
    } else if (traits.selected_only) {
        m_attr_checker->setPixelColor(QPoint(tx*2,      ty*2),QColor(128,128,128));
        m_attr_checker->setPixelColor(QPoint(tx*2 + 1,  ty*2),QColor(96,96,96));
        m_attr_checker->setPixelColor(QPoint(tx*2,      ty*2 + 1),QColor(96,96,96));
        m_attr_checker->setPixelColor(QPoint(tx*2 + 1,  ty*2 + 1),QColor(128,128,128));
    }
}

/*
 * Update the tile at (x,y) from tileno_from to tileno_to.
 */
void NametableRenderer::update_xy(int x,int y, uchar tileno_from, uchar tileno_to)
{
    auto found = std::find(tileno_2_coord_map[tileno_from].begin(),tileno_2_coord_map[tileno_from].end(),TileCoord{static_cast<int>(x),static_cast<int>(y)});
    tileno_2_coord_map[tileno_from].erase(found);
    tileno_2_coord_map[tileno_to].push_back(TileCoord{static_cast<int>(x),static_cast<int>(y)});
    nametable.set(x,y,tileno_to);
}

/*
 * Update the 2x2-tile attribute block with upper-left corner at (x,y) from palno_from to palno_to.
 */
void NametableRenderer::update_attr_block(int px,int py, uchar palno_from, uchar palno_to)
{
    auto found = std::find(palno_2_coord_map[palno_from].begin(),palno_2_coord_map[palno_from].end(),TileCoord{static_cast<int>(px),static_cast<int>(py)});
    palno_2_coord_map[palno_from].erase(found);
    palno_2_coord_map[palno_to].push_back(TileCoord{static_cast<int>(px),static_cast<int>(py)});
    attr.set(px,py,palno_to);
}

/*
 * Redraw all tile slots with tile tileno
 */
void NametableRenderer::update_tileno(int tileno)
{
    for (auto p : tileno_2_coord_map[static_cast<uchar>(tileno)]) {
        redraw_queue.push(p);
    }
    update();
}

void NametableRenderer::update_palno(int palno)
{
    for (auto [x,y] : palno_2_coord_map[static_cast<uchar>(palno)]) {
        redraw_queue.push(TileCoord{x,y});
        redraw_queue.push(TileCoord{x+1,y});
        redraw_queue.push(TileCoord{x,y+1});
        redraw_queue.push(TileCoord{x+1,y+1});
    }
    update();
}

void NametableRenderer::update_all()
{
    for (int ty = 0; ty < 32; ++ty) {
        for (int tx = 0; tx < 32; ++tx) {
            redraw_queue.push(TileCoord{tx,ty});
        }
    }
    update();
}

void NametableRenderer::set_grid_size(NESQT::GridSize gs, bool checked)
{
    if (checked) {
        traits.gridsize = static_cast<NESQT::GridSize>(traits.gridsize | gs);
    } else {
        traits.gridsize = static_cast<NESQT::GridSize>(traits.gridsize & ~gs);
    }
    update();
}

void NametableRenderer::slot_attr_checker(int){
    traits.attr_checker = !traits.attr_checker;
    update_all();
}

void NametableRenderer::slot_selected_only(int)
{
    traits.selected_only = !traits.selected_only;
    update_all();
}

void NametableRenderer::slot_check_selected_only()
{
    if (traits.selected_only) {
        update_all();
    }
}

void NametableRenderer::draw_grids(QPainter &painter)
{
    if (!traits.gridsize) return;

    QPen pen {};
    pen.setWidth(0);
    if (traits.gridsize & GridSize::OneByOne) {
        pen.setColor({64,64,64});
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        for (int x = 0; x < m_width; x += 8) {
            painter.drawLine(QLine{QPoint{x,0},QPoint{x,static_cast<int>(m_height)}});
        }
        for (int y = 0; y < m_height; y += 8) {
            painter.drawLine(QLine{QPoint{0,y},QPoint{static_cast<int>(m_width),y}});
        }
    }
    if (traits.gridsize & GridSize::TwoByTwo) {
        pen.setColor({96,96,96});
        pen.setStyle(Qt::DashDotLine);
        painter.setPen(pen);
        for (int x = 0; x < m_width; x += 16) {
            painter.drawLine(QLine{QPoint{x,0},QPoint{x,static_cast<int>(m_height)}});
        }
        for (int y = 0; y < m_height; y += 16) {
            painter.drawLine(QLine{QPoint{0,y},QPoint{static_cast<int>(m_width),y}});
        }
    }
    if (traits.gridsize & GridSize::FourByFour) {
        pen.setColor({128,128,128});
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        for (int x = 0; x < m_width; x += 32) {
            painter.drawLine(QLine{QPoint{x,0},QPoint{x,static_cast<int>(m_height)}});
        }
        for (int y = 0; y < m_height; y += 32) {
            painter.drawLine(QLine{QPoint{0,y},QPoint{static_cast<int>(m_width),y}});
        }
    }
}

QVector<QPainter::PixmapFragment> NametableRenderer::make_pixmap_fragments(const QPixmap &pm)
{
    using PixmapFragment = QPainter::PixmapFragment;
    QVector<PixmapFragment> v {};
    for (auto [x,y] : tileno_2_coord_map[chr.current_tile()]) {
        v.push_back(PixmapFragment::create({(qreal)(x*8)+4,(qreal)(y*8)+4},{(qreal)(x*8),(qreal)(y*8),8,8}));
    }
    return v;
}

void NametableRenderer::paintEvent(QPaintEvent *event)
{
    QPainter p{this};
    while (!redraw_queue.empty()) {
        auto coord = redraw_queue.front();
        redraw_queue.pop();
        auto tx = coord.first, ty = coord.second;
        render_tile(tx,ty);
    }
    if (!traits.attr_checker && !traits.selected_only) {
        QTransform transform {};
        p.setTransform(transform.scale(3,3));
        p.drawImage(0,0,*m_image);
        draw_grids(p);
        if (selector.is_valid) {
            draw_selection(p);
        }
    } else if (traits.attr_checker) {
        p.drawImage(0,0,m_attr_checker->scaledToWidth(width()));
    } else if (traits.selected_only) {
        p.drawImage(0,0,m_attr_checker->scaledToWidth(width()));
        QTransform transform {};
        p.setTransform(transform.scale(3,3));
        auto pm = QPixmap::fromImage(*m_image);
        auto pixmapFrags = make_pixmap_fragments(pm);
        p.drawPixmapFragments(pixmapFrags.data(),pixmapFrags.size(),pm);
        draw_grids(p);
        if (selector.is_valid) {
            draw_selection(p);
        }
    }
}

#define ACTION_STROKE(_type,_x,_y,_fr,_to) \
do { \
    undo_stack->push(new StrokeUndo(*this,{ \
        .x      = (_x), \
        .y      = (_y), \
        .from   = (_fr), \
        .to     = (_to), \
        .type   = (_type) \
    },undo_count));   \
} while (0)
#define ACTION_SELECT() \
do { \
        undo_stack->push(new SelectUndo(*this)); \
} while (0)
#define ACTION_CANCEL_SELECT() \
do { \
    undo_stack->push(new SelectCancelUndo(*this)); \
} while (0)
#define ACTION_EYEDROPPER(_x,_y) \
do { \
    undo_stack->push(new EyedropperUndo(*this, { \
        .tile_from  = static_cast<uchar>(chr.current_tile()), \
        .tile_to    = nametable.get((_x),(_y)), \
        .pal_from   = static_cast<uchar>(pal.current_palette()), \
        .pal_to     = attr.get((_x),(_y)) \
    })); \
} while (0)
#define ACTION_FILL(_x,_y) \
do { \
    undo_stack->push(new FillUndo(*this, { \
        .x = (_x), \
        .y = (_y), \
        .from = nametable.get((_x),(_y)), \
        .to = static_cast<uchar>(chr.current_tile()) \
    })); \
} while (0)
#define ACTION_FILL_SELECTION() \
do { \
        undo_stack->push(new SelectFillUndo(*this,chr.current_tile(),pal.current_palette())); \
} while (0)
#define ACTION_PASTE(_x,_y) \
do { \
        undo_stack->push(new PasteUndo(*this, { (_x),(_y) })); \
} while (0)

/*
 * Non-selecting left mouse behaviors:
 * - Cancel a selection if a selection is visible.
 * AND ONE of the following:
 * - Eyedropper the color if ALT was held,
 * - Start or continue a stroke of tile and/or palette to the nametable.
 */
void NametableRenderer::left_mouse_handler(int x, int y, int modifiers)
{
    int scale = width() / 32;
    int tx = x / scale, ty = y / scale;
    auto px = tx & ~1, py = ty & ~1;
    if (selector.is_valid && !selector.is_drawing) {
        ACTION_CANCEL_SELECT();
    }
    if (modifiers & Qt::AltModifier) {
        ACTION_EYEDROPPER(tx,ty);
        return;
    }
    if (apply_tile() && (modifiers & Qt::ControlModifier)) {
        /* TODO: Fill key combo */
        ACTION_FILL(tx,ty);
        return;
    }
    if (auto from = attr.get(tx,ty)
        , to = static_cast<uchar>(pal.current_palette())
        ; from != to && apply_palette()) {
        ACTION_STROKE(StrokeType::Palette,px,py,from,to);
    }
    if (auto from = nametable.get(tx,ty)
        , to = static_cast<uchar>(chr.current_tile())
        ; from != to && apply_tile()) {
        ACTION_STROKE(StrokeType::Tile,tx,ty,from,to);
    }
}

/*
 * Middle mouse behaviors:
 * - If a selection is visible and finished, paste that selection to the
 *      nametable,  with the upper-left corner of the selection at the tile
 *      where the middle mouse button was pressed.
 */
void NametableRenderer::middle_mouse_handler(int x, int y, int modifiers)
{
    if (!selector.is_valid
        || selector.is_drawing
        || !traits.apply_tile && !traits.apply_palette) {
        return;
    }
    int scale = width() / 32;
    int tx = x / scale, ty = y / scale;
    ACTION_PASTE(tx,ty);
}

void NametableRenderer::selection_handler(int x, int y)
{
    auto scale = width() / 32;
    int tx = static_cast<int>(x) / scale, ty = static_cast<int>(y) / scale;
    selector.update(tx,ty);
}

void NametableRenderer::mouseReleaseEvent(QMouseEvent *event)
{
    undo_count++;
    if (selector.is_valid && !selector.has_moved) {
        selector = back;
        ACTION_CANCEL_SELECT();
    } else if (selector.is_drawing) {
        selector.is_drawing = false;
        ACTION_SELECT();
    }
}

void NametableRenderer::draw_selection(QPainter &painter)
{
    auto cur = selector.current;
    int x, y, w, h;
    if (cur.x0 <= cur.x1) {
        x = cur.x0 * 8;
        w = (cur.x1 - cur.x0 + 1)*8;
    } else {
        x = cur.x1 * 8;
        w = (cur.x0 - cur.x1 + 1)*8;
    }
    if (cur.y0 <= cur.y1) {
        y = cur.y0 * 8;
        h = (cur.y1 - cur.y0 + 1)*8;
    } else {
        y = cur.y1 * 8;
        h = (cur.y0 - cur.y1 + 1)*8;
    }
    auto rect = QRect{x,y,w,h};
    /*
     * Animate the dashes on the selection.
     */
    intern.selector_pen.setDashOffset(intern.selector_offset);
    painter.setPen(intern.selector_pen);
    painter.drawRect(rect);
}

void NametableRenderer::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x(), y = event->pos().y();
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        QWidget::mousePressEvent(event);
    } else if (event->modifiers() & Qt::ShiftModifier) {
        back = selector;
        selector.reset();
        selection_handler(x,y);
    } else if (event->buttons() == Qt::LeftButton) {
        left_mouse_handler(x,y,event->modifiers());
    } else if (event->buttons() == Qt::MiddleButton) {
        middle_mouse_handler(x,y,event->modifiers());
    }
}

void NametableRenderer::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x(), y = event->pos().y();
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        event->ignore();
        return;
    } else if (selector.is_drawing) {
        selector.has_moved = true;
        selection_handler(x,y);
    } else if (event->buttons() == Qt::LeftButton) {
        left_mouse_handler(x,y,0);
    }
    emit signal_status(compose_status(x,y),0);
}

void NametableRenderer::set_tool(ToolType type)
{
    traits.last_tool = traits.tool;
    traits.tool = type;
}

void NametableRenderer::undo_handler()
{
    undo_stack->undo();
}

void NametableRenderer::redo_handler()
{
    undo_stack->redo();
}

void NametableRenderer::fill_handler()
{
    if (!selector.is_valid
        || !apply_tile() && !apply_palette()) {
        return;
    }
    ACTION_FILL_SELECTION();
}

void NametableRenderer::keyPressEvent(QKeyEvent *event)
{
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    if (ctrl) {
        if (shift) {
            if (event->key() == Qt::Key_Z) {
                redo_handler();
            }
        } else {
            switch(event->key()) {
            case Qt::Key_F:
                fill_handler();
                break;
            case Qt::Key_Z:
                undo_handler();
                break;
            default:
                QWidget::keyPressEvent(event);
                return;
            }
        }
    } else {
        switch (event->key()) {
        case Qt::Key_Escape:
            if (selector.is_drawing) {
                selector.is_drawing = false;
                ACTION_SELECT();
            }
            if (selector.is_valid) {
                ACTION_CANCEL_SELECT();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
        }
    }
}

void NametableRenderer::keyReleaseEvent(QKeyEvent *event)
{
}

void NametableRenderer::timerEvent(QTimerEvent *event)
{
    if (selector.is_valid && event->timerId() == m_timer.timerId()) {
        intern.selector_offset -= intern.selector_interval;
        if (intern.selector_offset <= -3 * intern.selector_dash_len) {
            intern.selector_offset = 0;
        }
        update();
    } else {
        QWidget::timerEvent(event);
    }
}

void NametableRenderer::wheelEvent(QWheelEvent *event)
{
    bool shift = event->modifiers() & Qt::ShiftModifier;
    if (shift) {
        if (event->angleDelta().y() > 0) {
            chr.set_current_tile(chr.current_tile() == 0 ? chr.size() - 1 : chr.current_tile() - 1);
        } else {
            chr.set_current_tile(chr.current_tile() == chr.size() - 1 ? 0 : chr.current_tile() + 1);
        }
        return;
    }
    if (event->angleDelta().y() > 0) {
        pal.set_current_index(pal.current_index() == 0 ? pal.size() - 1 : pal.current_index() - 1);
    } else {
        pal.set_current_index(pal.current_index() == pal.size() - 1 ? 0 : pal.current_index() + 1);
    }
    update();
}

QString NametableRenderer::compose_status(int x, int y)
{
    QString s{};
    char buf[64] {};
    int scale = width() / 32;
    auto tx = x / scale, ty = y / scale;
    auto off = ty * nametable.width() + tx;
    auto attr_off = ty/4 * 8 + tx/4 + nametable.width() * nametable.height();

    std::snprintf(buf, sizeof(buf), "XY:%2d,%2d Off:$%04lx=$%02x AtXY:%2d,%2d AtOff %04lx.%d=%d ($)",tx,ty,off,nametable.get(tx,ty),tx/2,ty/2,attr_off,(tx&2)+(ty&2)*2,attr.get(tx,ty));
//        str="XY:"+IntToStr(tx)+","+IntToStr(ty)+" Off:$"+IntToHex(off,4)+"=$"+IntToHex(nameTable[off],2);
//        str+=" AtXY:"+IntToStr(tx/2)+","+IntToStr(ty/2)+" AtOff:$"+IntToHex(ty/4*8+tx/4+nameTableWidth*nameTableHeight,4)+"."+IntToStr((tx&2)+(ty&2)*2)+"="+IntToStr(attr.get(tx,ty))+" ($"+IntToHex(attrTable[ty/4*8+tx/4],2)+")";
//        if(nameSelection.left>=0) str+=" WH:"+IntToStr(nameSelection.right-nameSelection.left)+"x"+IntToStr(nameSelection.bottom-nameSelection.top);
//        str+=" MetaSpr:"+IntToStr(metaSpriteActive);
    s = QString{buf};
    return s;
}

std::vector<TileCoord> NametableRenderer::calculate_fill(int tx, int ty, uchar t_new)
{
    auto t_old = nametable.get(tx,ty);
    if (t_old == t_new) return std::vector<TileCoord>{};
    std::map<TileCoord,int> map;
    std::queue<TileCoord> queue;
    map[{tx,ty}] = 1;
    queue.push({tx,ty});
    for (; !queue.empty(); queue.pop()) {
        auto e = queue.front();
        auto x = e.first, y = e.second;
        if (x != 0) {
            if (auto n = TileCoord{x-1,y}; map.count(n) == 0) {
                if (x - 1 >= 0 && nametable.get(n.first,n.second) == t_old) {
                    queue.push(n);
                    map[n] = 1;
                }
            }
        }
        if (y != 0) {
            if (auto n = TileCoord{x,y-1}; map.count(n) == 0) {
                if (y - 1 >= 0 && nametable.get(n.first,n.second) == t_old) {
                    queue.push(n);
                    map[n] = 1;
                }
            }
        }
        if (auto n = TileCoord{x+1,y}; map.count(n) == 0) {
            if (x + 1 < nametable.width() && nametable.get(n.first,n.second) == t_old) {
                queue.push(n);
                map[n] = 1;
            }
        }
        if (auto n = TileCoord{x,y+1}; map.count(n) == 0) {
            if (y + 1 < nametable.height() && nametable.get(n.first,n.second) == t_old) {
                queue.push(n);
                map[n] = 1;
            }
        }
    }
    std::vector<TileCoord> v;
    for (const auto &[k,_] : map) {
        v.push_back(k);
    }
    return v;
}

QString NametableRenderer::as_6502_asm(bool selection)
{
    QString s{};
    int x0, x1, y0, y1;
    char buf[64];
    if (selection && selector.is_valid) {
        auto cur = selector.current;
        if (cur.x0 <= cur.x1) {
            x0 = cur.x0;
            x1 = cur.x1;
        } else {
            x0 = cur.x1;
            x1 = cur.x0;
        }
        if (cur.y0 <= cur.y1) {
            y0 = cur.y0;
            y1 = cur.y1;
        } else {
            y0 = cur.y1;
            y1 = cur.y0;
        }
    } else if (!selection) {
        x0 = 0, x1 = nametable.width() - 1;
        y0 = 0, y1 = nametable.height() - 1;
    } else {
        return s;
    }
    auto i = 0;
    for (auto y = y0; y < y1 + 1; ++y) {
        s += "    .byte ";
        for (auto x = x0; x < x1 + 1; ++x, ++i) {
            snprintf(buf, sizeof(buf), "$%2.2x", nametable.get(x,y));
            s += buf;
            if (x < x1) {
                s += ",";
            }
        }
        s += "\n";
    }
    return s;
}

QString NametableRenderer::as_c_code(bool selection, bool rle)
{
    QString s{};
    int x0, x1, y0, y1;
    char buf[64];
    if (selection && selector.is_valid) {
        auto cur = selector.current;
        if (cur.x0 <= cur.x1) {
            x0 = cur.x0;
            x1 = cur.x1;
        } else {
            x0 = cur.x1;
            x1 = cur.x0;
        }
        if (cur.y0 <= cur.y1) {
            y0 = cur.y0;
            y1 = cur.y1;
        } else {
            y0 = cur.y1;
            y1 = cur.y0;
        }
    } else if (!selection) {
        x0 = 0, x1 = nametable.width() - 1;
        y0 = 0, y1 = nametable.height() - 1;
    } else {
        return s;
    }
    if (rle) {
        int size = 0;
        unsigned char src[65536],dst[65536]; // TODO: magic number
        for (auto y = y0; y < y1 + 1; ++y) {
            for (auto x = x0; x < x1 + 1; ++x) {
                src[size++] = nametable.get(x,y);
            }
        }
        size = encode_rle(dst,src,size);
        if(size < 0) {
            NESQT::warn(tr("No unused tiles found, can't be encoded with RLE"));
            return QString{};
        }
        sprintf(buf,"const unsigned char nametable[%i]={\n",size);
        s += buf;
        for(int i = 0; i < size; ++i) {
            sprintf(buf,"0x%2.2x",dst[i]);
            s += buf;
            if(i < size-1) s += ",";
            if(((i&15)==15)||(i==size-1)) s += "\n";
        }
        s += "};";
        return s;
    }
    auto i = 0;
    snprintf(buf, sizeof(buf), "const unsigned char nametable[%i*%i] = {\n", x1 - x0 + 1, y1 - y0 + 1);
    s += buf;
    for (auto y = y0; y < y1 + 1; ++y) {
        s += "    ";
        for (auto x = x0; x < x1 + 1; ++x, ++i) {
            snprintf(buf, sizeof(buf), "0x%2.2x", nametable.get(x,y));
            s += buf;
            if (x < x1 || y < y1) {
                s += ",";
            }
        }
        s += "\n";
    }
    s += "};";
    return s;
}

void NametableRenderer::load_from_file(QString filename)
{
    unsigned char buf[1024]; /* TODO: magic number */
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        NESQT::warn(tr("Failed to open file"));
        return;
    }

    QDataStream ds(&f);

    if (ds.readRawData((char *) buf, sizeof(buf)) < sizeof(buf)) {
        NESQT::warn(tr("Failed to read 1K from Nametable file"));
        return;
    }

    nametable.load_from_file(buf, 960);

    attr.load_from_file(buf + 960);
}
