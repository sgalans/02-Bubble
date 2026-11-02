Interface:
 - Menu principal [] //ara nomes es un selector de nivells, falta menu amb Jugar / Instruccions / Credits / Sortir
 - Instruccions []
 - Crèdits []
 - HUD vides [x]
 - HUD temps restant [x] //vermell i parpellejant els ultims 30s
 - HUD enemics restants [x]
 - HUD bombes simultanies [x]
 - HUD mida explosio (fire) [x]
 - pantalla game over [x] //"YOU ARE DEAD" + ESC torna al menu
 - pantalla nivell superat [x]
 - pantalla victoria final (despres del boss) []

Enemy:
 - walker [x]
 - runner [x]
 - shooter [x]
 - Boss [] 

Nivells:
 - nivell1 [x] 
 - millorar nivell 1 []
 - nivell2 [x]
 - millorar nivell 2 []
 - nivell3 [x] // U de 32x32, cambra interior 14x14, clau K i porta L
 - millorar nivell 3 []
 - nivell4 []
 - nivell5 [] 
 - memòria dels power-ups entre nivells consecutius [x]
 - pensar en què fer amb els power-ups acomulats en cas de perdre una vida o morir
 - porta de sortida [x] -> falta trobat textura per la porta
 - superar nivell -> passar al seguent [x] 
 - recordar nivells superats al menu [x] 
 - compte enrere del nivell [x] 
 - que passa quan el temps arriba a 0 [] 
 - posicio inicial del player llegida del fitxer de nivell [x] // @; sense marcador conserva l'inici per defecte

Moviment:
 - fet [x]
 - reduir velocitat màxima de caiguda [x]
 - reduir velocitat de moviment de tot [x]
 - reduir velocitat que dóna el power up speed [x]

Vides:
 - 3 vides [x] 
 - perdre vida en tocar enemic / projectil / explosio [x]
 - respawn / reiniciar nivell en perdre una vida []


Objectes:
 - Bomb Up [x]
 - Fire Up [x]
 - Speed Up [x]
 - Bomb Proof [x]
 - Clau amagada en arbust K [x] // obre una porta L; icona al HUD
 - Cor (protegeix d'un cop) [] opcional
 - objectes surten en destruir tiles [x]

Gameplay:
 - posar Bombes [x]
 - Explotar bombes [x]
 - saltar sobre bombes [x]
 - bombes maten enemics i player [x]
 - bombes alineades amb els tiles [x]
 - reaccio en cadena [x]
 - tiles destructibles [x]
 - portes amb pany [x] // bloquegen moviment i explosions fins a fer servir la clau
 - millora de càmera [x]

Tecles debug:
 - G god mode [x]
 - K matar tots els enemics i obrir porta []
 - 1-5 carregar nivell [] //nomes funciona des del menu, ha de funcionar tambe jugant

General (polish):
 - So efectes [x] //menu, bomba, explosio, item, mal
 - So: falten efectes [] //mort enemic, salt, porta, game over, victoria, boss
 - Musica de fons [] //Audio::playMusic existeix pero no es crida enlloc
 - Textura definitiva de la porta []
 - Animacions [] //tantes com poguem (mort player/enemics, tile destruint-se, porta, HUD)
 - Transicions entre pantalles (fade) []

Lliurament:
 - info.txt (noms, funcionalitats, instruccions) []
 - carpeta Binari (exe + dlls + images, levels, sounds, fonts, shaders) []
 - carpeta Projecte compilable []
 - demo.avi d'1 minut (< 100 MB) []
 - zip NomCognoms.zip (< 200 MB) []
