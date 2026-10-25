# Blocs del nivell 1

El tileset `images/level01_tiles.png` té 10 columnes × 4 files de peces de 16 × 16 píxels. La numeració comença a 1, d'esquerra a dreta i de dalt a baix. Els noms següents descriuen l'aspecte de cada peça.

Totes aquestes peces són sòlides, incloses les que semblen fullatge o pendents: el dibuix no canvia la forma rectangular de la col·lisió. Només `*` és destruïble. Un espai representa una casella buida; les majúscules continuen reservades a enemics i objectes.

Obre `blocs.html` al navegador per veure cada codi al costat de la seva imatge ampliada.

| Número | Codi al mapa | Aspecte |
|---|---|---|
| 1 | `1` | Barres verticals verdes |
| 2 | `2` | Trama diagonal verda |
| 3 | `3` | Roca verda arrodonida |
| 4 | `4` | Roca amb vora d'herba |
| 5 | `5` | Herba amb puntes penjants |
| 6 | `6` | Roca amb espiral vegetal (A) |
| 7 | `7` | Roca amb espiral vegetal (B) |
| 8 | `8` | Roca amb anella vegetal |
| 9 | `9` | Herba penjant, variant A |
| 10 | `a` | Herba penjant, variant B |
| 11 | `b` | Base de barres verticals |
| 12 | `c` | Trama de rombes |
| 13 | `d` | Herba amb arrels, extrem esquerre |
| 14 | `e` | Herba amb arrels, variant A |
| 15 | `f` | Herba amb arrels, variant B |
| 16 | `g` | Herba amb arrels, variant C |
| 17 | `h` | Herba amb arrels, variant D |
| 18 | `i` | Herba amb arrels, extrem dret |
| 19 | `j` | Maons verds rectangulars |
| 20 | `k` | Bloc llis amb vores dentades |
| 21 | `l` | Terra verda amb vora inferior |
| 22 | `m` | Terra marró amb pedres, variant A |
| 23 | `n` | Terra marró amb pedres, variant B |
| 24 | `o` | Maons fins en filades |
| 25 | `p` | Trama diagonal de roca, variant A |
| 26 | `q` | Trama diagonal de roca, variant B |
| 27 | `r` | Trama diagonal de roca, variant C |
| 28 | `s` | Maons horitzontals irregulars |
| 29 | `t` | Pedra de carreus grans |
| 30 | `u` | Roca verda clapejada |
| 31 | `v` | Trama de branques o esquerdes |
| 32 | `w` | Fullatge dens, variant A |
| 33 | `x` | Fullatge dens, variant B |
| 34 | `y` | Maons petits en vertical |
| 35 | `z` | Carreus rectangulars en vertical |
| 36 | `!` | Franges diagonals amples |
| 37 | `"` | Franges diagonals fines |
| 38 | `#` | Trama diagonal irregular |
| 39 | `$` | Mosaic de pedres irregulars |
| 40 | `%` | Trama de rombes amb molsa |
| — | `*` | Arbust sòlid destruïble (`SwampTiles/Bush_2_16x16.png`) |

Els quatre arbustos són a les caselles (columna, fila), comptant des d'1: **(7,15), (14,11), (2,9), (9,6)**. Bloquegen el moviment i la propagació del foc. Una explosió els elimina i s'atura en aquella casella.

Per amagar un power-up, substitueix `*` per `S` (patins), `F` (abast de foc), `B` (més bombes) o `P` (protecció contra bombes pròpies). La casella continua sent un arbust sòlid; l'objecte només es veu i es pot recollir quan l'arbust desapareix. `*` és un arbust buit. Al nivell 1, **(7,15)** amaga els patins i **(9,6)** amaga el Fire Up.

