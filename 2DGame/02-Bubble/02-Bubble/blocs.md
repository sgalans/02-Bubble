# Blocs i marcadors dels nivells

El tileset actual és [images/personalized.png](images/personalized.png):
**80×80 píxels**, amb **5 columnes × 5 files** de tiles de **16×16**.
La numeració comença a 1, d'esquerra a dreta i de dalt a baix.
Aquest fitxer és el catàleg actual de textures i marcadors; el format i les
regles d'edició dels mapes són a [levels/README.md](levels/README.md).

## Peces del tileset

Les coordenades de píxel indiquen la cantonada superior esquerra de cada peça.

| Índex | Codi al mapa | Píxel (x, y) | Peça i ús |
| --- | --- | --- | --- |
| 1–3 | `1`, `2`, `3` | (0, 0), (16, 0), (32, 0) | Terra sòlida. |
| 4 | `*`, `B`, `F`, `S`, `P`, `K` | (48, 0) | Arbust destructible, buit o amb objecte. |
| 5 | `5` | (64, 0) | Pedra indestructible. |
| 6 | `K` | (0, 16) | Clau amagada en un arbust. |
| 7 | `L` | (16, 16) | Porta amb pany. |
| 8 | `D` | (32, 16) | Sortida tancada. |
| 9 | `D`, `L` oberta | (48, 16) | Porta oberta. |
| 10 | `a` | (64, 16) | Plataforma de maons temporal. |
| 11–12 | `b`, `c` | (0, 32), (16, 32) | Fotogrames dels maons; estàtics si s'escriuen al mapa. |
| 13 | `d` | (32, 32) | Terra / pedra clara sòlida. |
| 14 | — | (48, 32) | Bloc gris antic, sense ús. |
| 15–17 | — | (64, 32), (0, 48), (16, 48) | Explosió antiga, sense ús. |
| 18 | — | (32, 48) | Cel decoratiu. |
| 19–20 | `C` | (48, 48), (64, 48) | Núvol de 2×1 tiles, sense col·lisió. |
| 21–23 | `T` | (0, 64), (16, 64), (32, 64) | Trampolí: 9, 12 i 16 píxels d'alçada. |
| 24–25 | `U`, `V` | (48, 64), (64, 64) | Botó preparat i comprimit; `V` gira la textura 90° per al terra. Sense col·lisió. |

El cel es repeteix per cobrir tota la vista dels nivells. Col·loca els núvols
manualment amb `C` a la graella, deixant un espai a la dreta (`C `).
El marcador indica la meitat esquerra; les dues textures es dibuixen juntes
abans del mapa i les entitats, sense col·lisió. No hi ha núvols automàtics.
Utilitza `C`, no els índexs de textura, que crearien blocs sòlids.

Els codis de tile positius creen blocs sòlids indestructibles, excepte `a`.
En trepitjar un `a` per sobre, el jugador activa una animació de desaparició
10→11→12 que dura 1 segon. El bloc continua sent sòlid fins que desapareix.
Després de 2 segons absent, reapareix durant 1 segon amb 12→11→10.
Recupera la col·lisió en acabar, esperant si hi ha el jugador, un enemic o
una bomba dins de la casella. Tornar-lo a trepitjar no reinicia el compte.
Les explosions no l'activen ni el destrueixen. `b` i `c` escrits al mapa
continuen sent blocs estàtics, encara que es facin servir com a fotogrames.
Per posar un **arbust destructible**, utilitza `*`, no `4`.
La clau i les portes funcionals es creen amb `K`, `L` i `D`;
els codis `6`, `7`, `8` i `9` només creen blocs estàtics.
Els `a` ja són a les posicions indicades a cada mapa en començar el nivell;
desapareixen temporalment quan el jugador els trepitja. No es mouen de casella.
El bloc gris i els seus fotogrames d'explosió es conserven a la imatge,
però no s'utilitzen als nivells ni tenen cap mecànica especial.

## Trampolí (`T`)

El trampolí es posa amb `T` i ocupa una casella. En repòs fa 9 píxels
d'alçada, alineat amb la base del tile; els 7 píxels superiors són buits.
S'activa amb un botó `U` (paret) o `V` (terra) exactament 4 tiles a la dreta
i a la mateixa fila. `V` gira les dues textures 90° i es recolza a la base
de la casella; posa el terra sòlid a la fila de sota. No s'activa trepitjant-lo.
Una explosió que arriba al botó des de qualsevol direcció, inclòs el centre,
el comprimeix durant 1 segon i activa el trampolí. El botó no té col·lisió,
no atura el foc i no es destrueix. Els impactes durant la recàrrega s'ignoren.
El vincle amb el trampolí funciona encara que hi hagi blocs entremig;
l'explosió sí que necessita arribar fins al botó. Sense `T` al destí,
només es comprimeix el botó. Un trampolí actiu no reinicia el seu cicle.
El trampolí bloqueja les flames i no es destrueix; cap explosió directa l'activa.
L'animació és 9→12→16→12→9 píxels. Es manté completament estès durant 1 segon abans de baixar; el cicle complet dura aproximadament 1,4 segons.
El jugador es manté sobre la superfície i, en arribar als 16 píxels, rep
un impuls de 10 tiles d'alçada des de la superfície elevada, sense prémer salt.
Els sostres aturen l'ascens. Sense jugador a sobre, s'anima igualment.
Un nou impacte mentre està actiu no reinicia l'animació. Els enemics segueixen
la superfície però no reben l'impuls. Deixa espai lliure a sobre per al salt.

## Marcadors del mapa

| Codi | Significat |
| --- | --- |
| espai | Casella buida |
| `T` | Trampolí activat pel botó de 4 tiles a la dreta; impulsa 10 tiles |
| `U` | Botó sense col·lisió: una explosió activa el `T` de 4 tiles a l'esquerra |
| `V` | Versió de terra del botó `U`, girada 90°; mateix comportament |
| `C` | Núvol decoratiu de 2×1 tiles, sense col·lisió; marca la meitat esquerra |
| `@` | Posició inicial del jugador |
| `W` | Walker |
| `R` | Runner |
| `Z` | Shooter |
| `X` | Boss de 2×2 tiles; marca la cantonada superior esquerra |
| `D` | Sortida del nivell: s'obre quan no queden enemics |
| `L` | Porta amb pany: bloqueja el moviment i les explosions |
| `*` | Arbust sense objecte |
| `B` | Arbust amb Bomb Up: +1 bomba simultània |
| `F` | Arbust amb Fire Up: +1 tile d'abast de l'explosió |
| `S` | Arbust amb Speed Up: augmenta la velocitat |
| `P` | Arbust amb VEST: invulnerabilitat completa durant 17 segons; recollir-ne un altre reinicia el compte |
| `K` | Arbust amb una clau |

Els arbustos bloquegen el moviment i el foc. Una explosió els destrueix
i s'atura en aquella casella. L'objecte amagat apareix quan desapareix
l'arbust i es recull en tocar-lo.

La clau es mostra al HUD mentre el jugador la porta. Tocar una porta `L`
amb la clau obre el pas permanentment i consumeix la clau. La porta conserva
la textura oberta (peça 9) i deixa de tenir col·lisió. Les explosions no poden
obrir-la. `D` és la sortida del nivell i funciona independentment del pany:
s'obre quan no queden enemics. Entrar-hi inicia una animació d'1 segon;
en acabar es mostra la victòria del nivell o la pantalla final del joc.

## Capçalera dels nivells

Després de la mida del mapa, indica aquestes dades per utilitzar el tileset actual:

```text
16 16              -- Tile size & block size
images/personalized.png -- Tilesheet
5 5                -- Number of tiles in tilesheet
```

Cada fila de la graella ha de tenir exactament l'amplada declarada del mapa,
inclosos els espais finals. Vegeu [levels/README.md](levels/README.md)
per al format dels nivells. La distribució concreta és a cada `levelXX.txt`.
