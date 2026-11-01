Interface:
 - Menu principal [] //ara nomes es un selector de nivells, falta menu amb Jugar / Instruccions / Credits / Sortir
 - Instruccions []
 - Credits []
 - HUD vides [x]
 - HUD temps restant []
 - HUD enemics restants []
 - HUD bombes simultanies [] //HUD ja rep nBombs pero no el pinta
 - HUD mida explosio (fire) []
 - pantalla game over [] //perdre 3 vides -> tornar al menu
 - pantalla nivell superat / victoria final []

Enemy:
 - walker [x]
 - runner [x]
 - shooter [x]
 - Boss [] //nomes a l'ultim nivell

Nivells:
 - nivell1 [] //existeix level01.txt pero es de proves
 - nivell2 []
 - nivell3 []
 - nivell4 []
 - nivell5 [] //amb el boss
 - porta de sortida [] //nomes s'obre quan no queden enemics
 - superar nivell -> passar al seguent []
 - temps limit per nivell (i que fer quan s'acaba) []
 - posicio inicial del player llegida del fitxer de nivell [] //ara es fixa a Scene.cpp

Moviment:
 - fet [x]

Vides:
 - 3 vides [] //ara comenca amb lives = 2 (Player.cpp)
 - perdre vida en tocar enemic / projectil / explosio [x]
 - respawn / reiniciar nivell en perdre una vida []
 - perdre totes les vides -> tornar al menu [] //ara el player es queda quiet i no passa res

Objectes:
 - Bomb Up [x]
 - Fire Up [x]
 - Speed Up [x]
 - Bomb Proof [x]
 - Cor (protegeix d'un cop) [] //el pdf el menciona, opcional si ja en tenim 4
 - objectes surten en destruir tiles [] //ara es col·loquen directament al mapa

Gameplay:
 - posar Bombes [x]
 - Explotar bombes [x]
 - saltar sobre bombes [x]
 - bombes maten enemics i player [x]
 - bombes alineades amb els tiles [x]
 - reaccio en cadena [x]
 - tiles destructibles []

Tecles debug:
 - G god mode [x]
 - K matar tots els enemics i obrir porta []
 - 1-5 carregar nivell [] //nomes funciona des del menu, ha de funcionar tambe jugant

General (polish):
 - So efectes [x] //menu, bomba, explosio, item, mal
 - So: falten efectes [] //mort enemic, salt, porta, game over, victoria, boss
 - Musica de fons [] //Audio::playMusic existeix pero no es crida enlloc
 - Animacions [] //tantes com poguem (mort player/enemics, tile destruint-se, porta, HUD)
 - Transicions entre pantalles (fade) []

Lliurament:
 - info.txt (noms, funcionalitats, instruccions) []
 - carpeta Binari (exe + dlls + images, levels, sounds, fonts, shaders) []
 - carpeta Projecte compilable []
 - demo.avi d'1 minut (< 100 MB) []
 - zip NomCognoms.zip (< 200 MB) []
