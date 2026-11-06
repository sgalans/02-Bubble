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
| `V` | El mateix botó, girat 90° per situar-lo al terra; mateixa activació i recàrrega |
| `B`, `F`, `S`, `P` | Arbust amb Bomb Up, Fire Up, Speed Up o VEST (invulnerabilitat completa durant 17 s) |
| `K` | Arbust amb una clau |
| `L` | Porta amb pany; sòlida i immune a explosions |
| `D` | Sortida; s'obre quan no queden enemics |
| `@` | Inici del jugador |
| `W`, `R`, `Z` | Walker, Runner, Shooter |
| `X` | Boss (arbre de 2×2 tiles; la lletra marca el tile superior esquerre) |

VEST (`P`) protegeix de totes les explosions, enemics i projectils durant
17 segons. El jugador parpelleja i el HUD mostra els segons restants,
arrodonits cap amunt (`VEST 17` fins a `VEST 0`). Recollir-ne un altre
reinicia els 17 segons. No es traspassa entre nivells ni es restaura en
tornar a carregar un nivell. No evita la derrota per temps esgotat.

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

El temps límit és de **3 minuts als nivells 1 i 5**, **4 minuts al nivell 2**
i **5 minuts als nivells 3 i 4**. `Game::startLevel` passa aquest límit a
`Scene` en carregar el nivell, també amb les tecles de depuració `1`–`5`.

La sortida `D` s'obre quan s'han eliminat tots els enemics, també el boss.
En eliminar l'últim enemic apareix «EXIT!» durant 3 segons. També es recupera
una vida, fins al màxim de 4 (3 vermelles i una extra blanca), i apareix
«1 UP» durant el mateix temps, també si ja es tenen 4 vides. Els cors
vermells recuperats s'animen de buit a ple durant 1,5 segons, invertint
l'animació de pèrdua. La quarta vida, blanca, només parpelleja durant
1,5 segons en aparèixer o desaparèixer; es perd abans de les vermelles i
es conserva en passar de nivell. No es concedeix la vida si el jugador ja no té vides.
La recompensa només es dona una vegada; amb el boss, espera el final de
la seva animació de mort. En entrar a la
sortida oberta, el jugador s'alinea amb la porta, es gira d'esquena i
s'encongeix fins a desaparèixer durant 1 segon. El temps i la resta del joc
queden aturats durant aquesta entrada. Després apareix «YOU WIN»; al nivell 5
es mostra la imatge final `images/EndScreen.png`.
`Enter` continua al nivell següent o torna al menú després de l'últim nivell.
El progrés es conserva mentre el joc està obert, també en tornar al menú.
El selector mostra «START WITH» amb les vides, bombes simultànies, abast del
foc i nivell de velocitat d'entrada del nivell seleccionat. La velocitat base
es mostra com a 1, igual que al HUD. Els valors ja inclouen els límits.
Cada nivell guarda els Bomb Up, Fire Up i Speed Up recollits en l'última
partida completada, inclosos els recollits amb les millores al màxim.
En començar un nivell se sumen les aportacions dels nivells anteriors i
s'apliquen els límits del jugador; la seva pròpia aportació queda exclosa.
Repetir i completar un nivell substitueix el resultat anterior, sense sumar-hi
el nou. Abandonar-lo o perdre no modifica el resultat guardat. Si canvia el
resultat d'un nivell anterior, els següents recalculen els ítems en carregar-se.

Les vides d'entrada es guarden per separat: completar un nivell actualitza
les del següent, sense recalcular les dels altres nivells. El nivell 1 i els
nivells sense registre comencen amb 3 vides. Reintentar restaura les vides
d'entrada. Aquestes regles també s'apliquen a les tecles `1`–`5`. Les claus,
el VEST i la invulnerabilitat de reaparició no es guarden. Tancar el joc
esborra aquest progrés.

Les posicions de plataformes, claus, portes, núvols i altres elements són
les de les graelles `level01.txt`–`level05.txt`. Consulta el fitxer concret
per veure'n la distribució actual. Cada nivell conté la capçalera i la
graella; no utilitza enllaços de moviment addicionals després de les files.

## Boss volador (`X`)

`X` marca la cantonada superior esquerra inicial del cos de 2×2 tiles.
El boss s'incorpora sense teletransportar-se a la cantonada més propera
d'un recorregut creuat: dalt esquerra → baix dreta → dalt dreta → baix esquerra
→ dalt esquerra. Els trams són rectes i les diagonals es creuen al centre.
Les caselles de referència són a 4 tiles de cada vora del mapa; al nivell
actual de 24×16 són (4,4), (19,11), (19,4) i (4,11), respectivament.
El centre del cos de 2×2 se situa al centre de cada casella de referència:
la seva cantonada superior esquerra recorre (3,5; 3,5), (18,5; 10,5),
(18,5; 3,5) i (3,5; 10,5), en unitats de tile.
Així el recorregut del cos és simètric respecte del centre del mapa (12,8).
Les `x` minúscules del dibuix eren només guies i s'han substituït per espais;
no són marcadors de ruta vàlids. `X` continua sent l'inici del boss.

Vola a 31,5 píxels/segon, sense gravetat, i ignora els blocs, els arbustos,
les portes i les plataformes. Només les bombes bloquegen el seu cos complet,
incloses les acabades de posar: s'atura sense avançar la ruta i la reprèn
quan desapareixen. Manté 5 vides, les ràfegues de 3 boles i les pauses de
preparació, recuperació i dany. Durant aquestes pauses queda suspès a l'aire.
Els projectils continuen xocant amb blocs i bombes; les explosions poden
fer mal al boss i el contacte amb el seu cos fa mal al jugador.

En perdre l'última vida, el boss es queda quiet amb la postura de cop
durant 4 segons i deixa de disparar o fer mal per contacte. Dins dels seus
32×32 apareixen esclats dels tres fotogrames de partícules de 8×8 de
`bomberman.png`, a (223,540), (233,540) i (243,540).
A partir del segon 2, el cos parpelleja amb el mateix ritme que el jugador;
les partícules continuen visibles. Al segon 4 es retira el boss i s'obre
la sortida si no queden altres enemics. `K` també respecta aquesta animació.

## Trampolí (`T`)

El trampolí es posa amb `T` i ocupa una casella. En repòs fa 9 píxels
d'alçada, alineat amb la base del tile; els 7 píxels superiors són buits.
Col·loca el botó amb `U` (paret) o `V` (terra, textura girada 90°),
a la mateixa fila i exactament 4 columnes a la dreta: `T   U` o `T   V`.
El botó de terra es dibuixa a la base de la seva casella: posa el terra sòlid
a la fila de sota. No s'activa trepitjant-lo; tots dos funcionen amb explosions.
Una explosió que arriba al botó des de qualsevol direcció, o una
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
