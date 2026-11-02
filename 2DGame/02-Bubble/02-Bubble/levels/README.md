# Format dels nivells

Cada fila ha de tenir exactament l'amplada indicada a la capçalera, inclosos
els espais finals. `personalized.png` té una graella de **3×3** tiles de 16×16.

| Símbol | Significat |
| --- | --- |
| espai | Buit |
| `1`, `2` | Terra amb herba |
| `3` | Terra / tronc |
| `5` | Pedra indestructible |
| `*` | Arbust destructible |
| `B`, `F`, `S`, `P` | Arbust amb Bomb Up, Fire Up, Speed Up o Bomb Proof |
| `K` | Arbust amb una clau |
| `L` | Porta amb pany; sòlida i immune a explosions |
| `D` | Sortida; s'obre quan no queden enemics |
| `@` | Inici del jugador |
| `W`, `R`, `Z` | Walker, Runner, Shooter |

La clau apareix en destruir l'arbust `K`. Cal recollir-la i tocar la porta `L`
per obrir el pas permanentment. Obrir una porta consumeix la clau.
La icona de la clau al HUD indica que el jugador la porta.
`K` al mapa és independent de la tecla de depuració `K`.

Al nivell 3, la cambra ocupa les columnes 9–22 i files 1–14 (coordenades
començant per zero): **14×14 tiles interiors**, amb marc de pedra a les
columnes 8 i 23 i files 0 i 15. L'únic accés és `L` a (23, 12).
La clau és al braç esquerre de la U i la sortida és dins de la cambra.
