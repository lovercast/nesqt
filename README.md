# NesQt

Cross-platform Qt port of Shiru's NES Screen Tool.

## Prereqs
- Qt 5 or 6
- cmake

## Building and Running
```sh
git clone git@github.com:lovercast/nesqt.git
mkdir build-NesQt
cd build-NesQt
export CMAKE_PREFIX_PATH={Path to Qt installation}/{Version Number}/{Target}/lib/cmake/
cmake -S ../NesQt -B .
make
./NesQt
```

## Tileset editor

- [x] LMB   - set tile as current
- [x] LMBx2 - open the tile in the CHR editor
- [ ] Shift+LMB - select a rectangular group of tiles
- [ ] Ctrl+LMB  - select random set of tiles
- [ ] RMB       - cancel selection

- [x] Ctrl+Z   - undo
- [ ] Ctrl+X   - cut tile graphics
- [ ] Ctrl+C   - copy tile graphics
- [ ] Ctrl+V   - paste tile graphics

- [x] Only half of 8K CHR memory is visible at a time, you can switch between the
halfs using 'A' and 'B' buttons under the tileset.

- [ ] You can drag and drop tiles in the tileset using right mouse button.
This will also update nametable indexes, so no visible changes will be
done, just two tiles in the tileset will get exchanged. This allows to
rearrange order of tiles easily.

- [x] 'Apply tiles' button allows to change tile numbers in the nametable. You can't
draw anything in the nametable when this button is not pressed down.

- [x] 'Attr checker' button allows to check nametable attributes with a special
checker tile. Warning, you still can draw tiles in this mode.

- [x] 'Selected only' button disabled display for any tiles except the ones currently
selected in the tileset (one or a few). It helps to visually see location and
amount of the tile entries in the nametable.

- [ ] 'Type in' button allows to enter a sequence of tiles into the nametable at
given position (click to set) either by clicking tiles in the tileset, or
using keyboard. In this case tileset should contain a font with ASCII encoding,
with space being tile 0.

'Grid All', '1x', '2x', '4x' buttons controls grid displaying. There are grids
of three sizes that could be used at once or in any combination.

## Palette editor

- [x] LMB selects current color and palette, or assigns a color to the current color
in the current palette. RMB allows to drag and drop colors between the main
palette or BG palettes.
- [ ] The mouse wheel can also be used to cycle through the palettes if the main
program window has focus.
- [x] 'Apply palettes' button enables applying current palette to the nametable.
- [x] 'R','G','B','M' buttons controls the color emphasis bits that alters the
palette.
- [x] 'A','B','C','D' buttons is four palette slots. You can load up to four palettes
and switch between them at any time. Please be aware that Palette Save/Load
only works for active palette. The slots needed for better convinience when
a few palettes needs to be designed to be used with the same graphics.
- [ ] When Num Lock is off, you can use Numpad keys to switch between sub-palletes
(1 and 3), sub-palette entries (7 and 9), and change current color (8,4,6,2).

## TODO
- [ ] File IO
- [ ] Metasprites
- [ ] Session

