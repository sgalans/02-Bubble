# Blocs i marcadors dels nivells

El tileset actual és [images/personalized.png](images/personalized.png):
**64×64 píxels**, amb **4 columnes × 4 files** de tiles de **16×16**.
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
| 10 | (16, 32) | Maons vermellosos | `a`: bloc sòlid |
| 11 | (32, 32) | Terra / pedra clara | `b`: bloc sòlid |
| 12 | (48, 32) | Bloc gris amb una marca diagonal vermella | `c`: bloc sòlid destructible, activador del pont |
| 13 | (0, 48) | Casella buida | No utilitzar com a bloc |
| 14–16 | (16, 48), (32, 48), (48, 48) | Animació d'explosió del `c` | Tres fotogrames en ordre durant 600 ms |

Els blocs numèrics tenen col·lisió rectangular completa i són indestructibles.
El `c` és destructible amb explosions i atura el foc. Si té un enllaç `BRIDGE`
al final del nivell, mou el seu `a` adjacent al destí configurat; el `a` segueix
sent sòlid i indestructible. Tots els `c` connectats pels costats exploten en
cadena al mateix instant, cadascun amb les seves flames i el seu moviment.
Vegeu [levels/README.md](levels/README.md).
Per posar un **arbust destructible**, utilitza `*`, no `4`: escriure `4`
només dibuixa l'arbust com un bloc indestructible. Igualment, `6` i `7`
només dibuixen blocs estàtics; la clau i la porta funcionals es creen amb `K` i `L`.
La sortida es col·loca amb `D` i canvia automàticament de la peça 8 a la 9
quan no queden enemics; escriure `8` o `9` només crea un bloc estàtic.

## Marcadors del mapa

| Codi | Significat |
| --- | --- |
| espai | Casella buida |
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
4 4                -- Number of tiles in tilesheet
```

Cada fila de la graella ha de tenir exactament l'amplada declarada del mapa,
inclosos els espais finals. Vegeu [levels/README.md](levels/README.md)
per al format dels nivells i la distribució de la cambra del nivell 3.
