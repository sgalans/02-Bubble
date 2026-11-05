# Format dels nivells

Cada fila ha de tenir exactament l'amplada indicada a la capçalera, inclosos
els espais finals. `personalized.png` té una graella de **5×5** tiles de 16×16.
La capçalera ha d'indicar `5 5`. El catàleg actual de peces i marcadors és
[../blocs.md](../blocs.md).
Els retalls especials segueixen les coordenades de [../blocs.md](../blocs.md).

El cel (peça 18) cobreix automàticament tot el fons. Els núvols es posen
manualment amb `C` a la graella: cada `C` dibuixa les peces 19–20 juntes,
ocupant visualment la seva casella i la de la dreta. Deixa un espai a la dreta
del marcador per mostrar el núvol complet (`C `); no cal un segon marcador.
Cap de les dues meitats té col·lisió. Sense `C`, només es veu el cel.
El núvol queda darrere dels blocs i les entitats. Substitueix espais per `C`
sense afegir columnes a la fila. L'atles continua sent de 80×80 píxels: `5 5`.

| Símbol | Significat |
| --- | --- |
| espai | Buit |
| `C` | Núvol decoratiu de 2×1 tiles; marca la meitat esquerra, sense col·lisió |
| `1`, `2` | Terra amb herba |
| `3` | Terra / tronc |
| `5` | Pedra indestructible |
| `a` | Plataforma temporal: desapareix en trepitjar-la i torna a aparèixer |
| `b`, `c`, `d` | Blocs sòlids indestructibles de l'atles |
| `*` | Arbust destructible |
| `T` | Trampolí: alçades 9/12/16 píxels, impuls de 10 tiles; activat pel botó |
| `U` | Botó sense col·lisió: activa el `T` exactament 4 tiles a l'esquerra |
| `B`, `F`, `S`, `P` | Arbust amb Bomb Up, Fire Up, Speed Up o Bomb Proof |
| `K` | Arbust amb una clau |
| `L` | Porta amb pany; sòlida i immune a explosions |
| `D` | Sortida; s'obre quan no queden enemics |
| `@` | Inici del jugador |
| `W`, `R`, `Z` | Walker, Runner, Shooter |
| `X` | Boss (arbre de 2×2 tiles; la lletra marca el tile superior esquerre) |

La clau apareix en destruir l'arbust `K`. Cal recollir-la i tocar la porta `L`
per obrir el pas permanentment. Obrir una porta consumeix la clau.
La porta `L` continua visible amb la textura oberta i deixa de tenir col·lisió.
La icona de la clau al HUD indica que el jugador la porta.
`K` al mapa és independent de la tecla de depuració `K`.

Els `a` desapareixen 1 segon després de trepitjar-los per sobre, amb els
fotogrames 10→11→12. Continuen sòlids durant aquest segon. Després de 2 segons
absents, reapareixen amb els mateixos fotogrames al revés durant 1 segon.
Recuperen la col·lisió al final; si la casella està ocupada pel jugador,
un enemic o una bomba, esperen que quedi lliure. Cada bloc té el seu compte
independent i no es reinicia en trepitjar-lo de nou mentre està actiu.

## Sortida i distribució dels nivells

La sortida `D` s'obre quan s'han eliminat tots els enemics, també el boss.
En eliminar l'últim enemic apareix «EXIT!» durant 3 segons. En entrar a la
sortida oberta, el jugador s'alinea amb la porta, es gira d'esquena i
s'encongeix fins a desaparèixer durant 1 segon. El temps i la resta del joc
queden aturats durant aquesta entrada. Després apareix «YOU WIN»; al nivell 5
es mostra la imatge final `images/EndScreen.png`.
`Enter` continua al nivell següent o torna al menú després de l'últim nivell.

Les posicions de plataformes, claus, portes, núvols i altres elements són
les de les graelles `level01.txt`–`level05.txt`. Consulta el fitxer concret
per veure'n la distribució actual. Cada nivell conté la capçalera i la
graella; no utilitza enllaços de moviment addicionals després de les files.

## Trampolí (`T`)

El trampolí es posa amb `T` i ocupa una casella. En repòs fa 9 píxels
d'alçada, alineat amb la base del tile; els 7 píxels superiors són buits.
Col·loca el botó amb `U`, a la mateixa fila i exactament 4 columnes a la dreta:
`T   U`. Una explosió que arriba al botó des de qualsevol direcció, o una
bomba que explota a la seva casella, activa el trampolí i comprimeix el botó.
El botó torna a estar preparat després d'1 segon; durant aquest temps ignora
nous impactes. No té col·lisió, no es destrueix ni atura les flames.
La connexió botó/trampolí funciona encara que hi hagi blocs entremig;
els blocs sí que poden tapar el botó i impedir que l'explosió l'assoleixi.
Si no hi ha `T` exactament al destí, només s'anima el botó.
El trampolí atura les flames i no s'activa mai per una explosió directa.
L'animació és 9→12→16→12→9 píxels. Es manté completament estès durant 1 segon abans de baixar; el cicle complet dura aproximadament 1,4 segons.
El jugador es manté sobre la superfície i, en arribar als 16 píxels, rep
un impuls de 10 tiles d'alçada des de la superfície elevada, sense prémer salt.
Els sostres aturen l'ascens. Sense jugador a sobre, s'anima igualment.
Un nou impacte mentre està actiu no reinicia l'animació. Els enemics segueixen
la superfície però no reben l'impuls. Deixa espai lliure a sobre per al salt.
