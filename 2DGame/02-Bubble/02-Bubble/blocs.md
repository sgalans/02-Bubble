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
| 10 | (16, 32) | Maons vermellosos | `a`: bloc sòlid |
| 11 | (32, 32) | Maons, variant B | `b`: bloc sòlid |
| 12 | (48, 32) | Maons, variant C | `c`: bloc sòlid |
| 13 | (0, 48) | Terra / pedra clara | `d`: bloc sòlid |
| 14 | (16, 48) | Bloc gris amb marca vermella | Sense ús actual |
| 15–17 | (32, 48), (48, 48), (0, 64) | Fotogrames d'explosió antics | Sense ús actual |
| 18–20 | (16, 64), (32, 64), (48, 64) | Caselles buides | No utilitzar com a blocs |

Tots els codis de tile positius creen blocs sòlids indestructibles.
Per posar un **arbust destructible**, utilitza `*`, no `4`.
La clau i les portes funcionals es creen amb `K`, `L` i `D`;
els codis `6`, `7`, `8` i `9` només creen blocs estàtics.
El pont del nivell 2 és permanent: els quatre `a` ja són als seus destins.
El bloc gris i els seus fotogrames d'explosió es conserven a la imatge,
però no s'utilitzen als nivells ni tenen cap mecànica especial.

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
4 5                -- Number of tiles in tilesheet
```

Cada fila de la graella ha de tenir exactament l'amplada declarada del mapa,
inclosos els espais finals. Vegeu [levels/README.md](levels/README.md)
per al format dels nivells i la distribució de la cambra del nivell 3.
