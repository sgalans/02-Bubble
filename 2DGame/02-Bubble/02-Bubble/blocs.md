# Blocs i marcadors dels nivells

El tileset actual és [images/personalized.png](images/personalized.png):
**64×80 píxels**, amb **4 columnes × 5 files** de tiles de **16×16**.
La numeració comença a 1, d'esquerra a dreta i de dalt a baix.

## Peces del tileset

Les coordenades de píxel indiquen la cantonada superior esquerra de cada peça.

| Peça | Píxel (x, y) | Aspecte | Ús al mapa |
| --- | --- | --- | --- |
| 1 | (0, 0) | Terra amb herba, variant A | `1`: bloc sòlid |
| 2 | (16, 0) | Terra amb herba, variant B | `2`: bloc sòlid |
| 3 | (32, 0) | Terra sense herba / tronc | `3`: bloc sòlid |
| 4 | (48, 0) | Arbust | `*` o una lletra d'objecte |
| 5 | (0, 16) | Pedra | `5`: bloc sòlid indestructible |
| 6 | (16, 16) | Clau | `K`: amagada dins d'un arbust |
| 7 | (32, 16) | Porta amb pany | `L`: pas bloquejat fins a fer servir la clau |
| 8 | (48, 16) | Sortida tancada | `D`: mentre queden enemics |
| 9 | (0, 32) | Sortida oberta | `D`: quan no queden enemics |
| 10 | (16, 32) | Maons vermellosos | `a`: plataforma que desapareix en trepitjar-la |
| 11 | (32, 32) | Maons, variant B | `b`: bloc sòlid |
| 12 | (48, 32) | Maons, variant C | `c`: bloc sòlid |
| 13 | (0, 48) | Terra / pedra clara | `d`: bloc sòlid |
| 14 | (16, 48) | Bloc gris amb marca vermella | Sense ús actual |
| 15–17 | (32, 48), (48, 48), (0, 64) | Fotogrames d'explosió antics | Sense ús actual |
| 18 | (16, 64) | Cel | Fons decoratiu de `Scene`, sense col·lisió |
| 19–20 | (32, 64), (48, 64) | Meitats esquerra i dreta del núvol | `C`: peça de 32×16 al fons, sense col·lisió |

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
Els quatre `a` del pont del nivell 2 ja són als seus destins en començar
i desapareixen temporalment quan el jugador els trepitja.
El bloc gris i els seus fotogrames d'explosió es conserven a la imatge,
però no s'utilitzen als nivells ni tenen cap mecànica especial.

## Marcadors del mapa

| Codi | Significat |
| --- | --- |
| espai | Casella buida |
| `C` | Núvol decoratiu de 2×1 tiles, sense col·lisió; marca la meitat esquerra |
| `@` | Posició inicial del jugador |
| `W` | Walker |
| `R` | Runner |
| `Z` | Shooter |
| `D` | Sortida del nivell: s'obre quan no queden enemics |
| `L` | Porta amb pany: bloqueja el moviment i les explosions |
| `*` | Arbust sense objecte |
| `B` | Arbust amb Bomb Up: +1 bomba simultània |
| `F` | Arbust amb Fire Up: +1 tile d'abast de l'explosió |
| `S` | Arbust amb Speed Up: augmenta la velocitat |
| `P` | Arbust amb Bomb Proof: protegeix de les bombes pròpies |
| `K` | Arbust amb una clau |

Els arbustos bloquegen el moviment i el foc. Una explosió els destrueix
i s'atura en aquella casella. L'objecte amagat apareix quan desapareix
l'arbust i es recull en tocar-lo.

La clau es mostra al HUD mentre el jugador la porta. Tocar una porta `L`
amb la clau obre el pas permanentment i consumeix la clau. Les explosions
no poden obrir aquesta porta. `D` és la sortida del nivell i funciona
independentment del pany.

## Capçalera dels nivells

Després de la mida del mapa, indica aquestes dades per utilitzar el tileset actual:

```text
16 16              -- Tile size & block size
images/personalized.png -- Tilesheet
4 5                -- Number of tiles in tilesheet
```

Cada fila de la graella ha de tenir exactament l'amplada declarada del mapa,
inclosos els espais finals. Vegeu [levels/README.md](levels/README.md)
per al format dels nivells i la distribució de la cambra del nivell 3.
