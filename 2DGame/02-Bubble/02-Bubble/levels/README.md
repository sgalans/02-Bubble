# Format dels nivells

Cada fila ha de tenir exactament l'amplada indicada a la capçalera, inclosos
els espais finals. `personalized.png` té una graella de **4×5** tiles de 16×16.
La capçalera ha d'indicar `4 5`. La peça clara abans codificada amb `b`
ara és la peça 13 (`d`); els nivells s'han adaptat per conservar-ne l'aspecte.
Els retalls especials segueixen les coordenades de [../blocs.md](../blocs.md).

| Símbol | Significat |
| --- | --- |
| espai | Buit |
| `1`, `2` | Terra amb herba |
| `3` | Terra / tronc |
| `5` | Pedra indestructible |
| `a` | Plataforma temporal: desapareix en trepitjar-la i torna a aparèixer |
| `b`, `c`, `d` | Blocs sòlids indestructibles de l'atles |
| `*` | Arbust destructible |
| `B`, `F`, `S`, `P` | Arbust amb Bomb Up, Fire Up, Speed Up o Bomb Proof |
| `K` | Arbust amb una clau |
| `L` | Porta amb pany; sòlida i immune a explosions |
| `D` | Sortida; s'obre quan no queden enemics |
| `@` | Inici del jugador |
| `W`, `R`, `Z` | Walker, Runner, Shooter |
| `X` | Boss (arbre de 2×2 tiles; la lletra marca el tile superior esquerre) |

La clau apareix en destruir l'arbust `K`. Cal recollir-la i tocar la porta `L`
per obrir el pas permanentment. Obrir una porta consumeix la clau.
La icona de la clau al HUD indica que el jugador la porta.
`K` al mapa és independent de la tecla de depuració `K`.

Els `a` desapareixen 1 segon després de trepitjar-los per sobre, amb els
fotogrames 10→11→12. Continuen sòlids durant aquest segon. Després de 2 segons
absents, reapareixen amb els mateixos fotogrames al revés durant 1 segon.
Recuperen la col·lisió al final; si la casella està ocupada pel jugador,
un enemic o una bomba, esperen que quedi lliure. Cada bloc té el seu compte
independent i no es reinicia en trepitjar-lo de nou mentre està actiu.

Al nivell 2, el pont està disponible des del començament: quatre blocs `a`
a (14, 4), (17, 4), (20, 4) i (23, 4), amb coordenades començant per zero.
Hi ha dos tiles buits entre blocs i són un tile per sota de la plataforma de
sortida, per deixar espai per saltar sota el sostre. No hi ha activadors ni
blocs que es moguin; el nivell només conté la graella, sense enllaços addicionals.

Al nivell 3, la cambra ocupa les columnes 9–22 i files 1–14 (coordenades
començant per zero): **14×14 tiles interiors**, amb marc de pedra a les
columnes 8 i 23 i files 0 i 15. L'únic accés és `L` a (23, 12).
La clau és al braç esquerre de la U i la sortida és dins de la cambra.
