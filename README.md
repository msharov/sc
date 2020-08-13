# SC

The Spreadsheet Calculator is a curses-based spreadsheet, written
in the ancient times by James Gosling. I forked this from the
7.16 release because it didn't compile properly for me. Since I
don't have much use for sc, I do not plan to do much other than
to keep it compiling. For a more actively developed version, try
[SC-IM](https://github.com/andmarti1424/sc-im).

Building requires [ncurses](https://invisible-island.net/ncurses/),
[bison](https://www.gnu.org/software/bison/), and a C11-supporting
compiler such as gcc 4.6.

```sh
configure --prefix=/usr && make install && sc
```
