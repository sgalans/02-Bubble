# Format dels nivells

Cada fila ha de tenir exactament l'amplada indicada a la capçalera, inclosos
els espais finals. `personalized.png` té una graella de **4×4** tiles de 16×16.
La capçalera ha d'indicar `4 4`. Els índexs de les peces 1–12 es conserven:
no cal canviar els codis de la graella. Els retalls especials segueixen
les coordenades de [../blocs.md](../blocs.md).

| Símbol | Significat |
| --- | --- |
| espai | Buit |
| `1`, `2` | Terra amb herba |
| `3` | Terra / tronc |
| `5` | Pedra indestructible |
| `a`, `b` | Blocs sòlids indestructibles de l'atles |
| `c` | Bloc sòlid destructible; pot activar el moviment d'un `a` adjacent |
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

Després de totes les files de la graella es poden afegir enllaços opcionals:

```text
BRIDGE cX cY aX aY destX destY
```

Les coordenades són de tile, començant per zero. En destruir el `c` amb una
explosió, l'`a` adjacent desapareix de l'origen i es col·loca immediatament al
destí, on continua sent sòlid i indestructible. El foc s'atura al `c` destruït,
igual que als arbustos. Sense enllaç, el `c` simplement desapareix.
Els enllaços han de tenir origen `a`, destí buit i `c` adjacent; els enllaços
invàlids o amb activador, origen o destí repetits s'ignoren.

Al nivell 2, els quatre `c` de (10, 1–4) mouen els `a` de (11, 1–4) a
(23, 4), (20, 4), (17, 4) i (14, 4), respectivament. Això forma un pont
amb dos tiles buits entre blocs, un tile per sota de la plataforma de sortida
per deixar espai suficient per saltar sota el sostre.
Els `c` adjacents pels costats (amunt, avall, esquerra o dreta) exploten en
cadena al mateix instant, encara que quedin fora de l'abast inicial de la bomba.
Cada bloc activa el seu enllaç i reprodueix els tres fotogrames nous de
`personalized.png`: (16, 48), (32, 48) i (48, 48), durant 600 ms.
L'explosió continua fent mal durant aquest temps. La cadena no
travessa espais buits ni blocs d'altres tipus, ni es propaga en diagonal.
Al nivell 2, una sola explosió contra qualsevol dels quatre `c` forma tot el pont.
Els blocs es restableixen en recarregar el nivell.

Al nivell 3, la cambra ocupa les columnes 9–22 i files 1–14 (coordenades
començant per zero): **14×14 tiles interiors**, amb marc de pedra a les
columnes 8 i 23 i files 0 i 15. L'únic accés és `L` a (23, 12).
La clau és al braç esquerre de la U i la sortida és dins de la cambra.
