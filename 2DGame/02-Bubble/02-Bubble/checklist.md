Interface:
 - Menu principal [x] //PLAY / INSTRUCTIONS / CREDITS + selector de nivells
 - Instruccions [x]
 - Crèdits [x] //noms dels dos integrants
 - HUD vides [x] //pèrdua animada durant 1,5 s: flaix blanc/vermell, parpelleig i fotogrames de buidatge fins al cor buit
 - HUD temps restant [x] //vermell i parpellejant els ultims 30s
 - HUD enemics restants [x]
 - HUD bombes simultanies [x]
 - HUD mida explosio (fire) [x]
 - pantalla game over [x] //"YOU ARE DEAD" + ESC torna al menu
 - pantalla nivell superat [x] //animació d'entrada a la sortida d'1 s, després "YOU WIN" + ENTER passa al seguent
 - pantalla victoria final (despres del boss) [x] //images/EndScreen.png amb "YOU WIN" a sobre; ENTER/ESC torna al menu

Enemy:
 - walker [x]
 - runner [x]
 - shooter [x]
 - Boss [x] //arbre volador 2x2 (X), recorregut diagonal creuat entre quatre cantonades simètriques; ignora blocs, el frenen les bombes; dispara 3 boles

Nivells:
 - nivell1 [x] 
 - millorar nivell 1 [x]
 - nivell2 [x]
 - millorar nivell 2 [x]
 - nivell3 [x]
 - millorar nivell 3 [x]
 - nivell4 [x]
 - nivell5 [x]
 - memòria dels power-ups entre nivells consecutius [x]
 - ara mateix si tens items acumulats però surts al menú els items acumulats es perden -> mirar què fer []
 - porta de sortida [x] //textures de porta tancada/oberta i porta amb pany a personalized.png
 - superar nivell -> passar al seguent [x] 
 - recordar nivells superats al menu [x] 
 - compte enrere del nivell [x] //nivells 1 i 5: 180 s; nivell 2: 240 s; nivells 3 i 4: 300 s
 - posicio inicial del player llegida del fitxer de nivell [x] // @

Moviment:
 - fet [x]
 - reduir velocitat màxima de caiguda [x]
 - reduir velocitat de moviment de tot [x]
 - reduir velocitat que dóna el power up speed [x]

Vides:
 - 3 vides inicials [x] //1 UP pot afegir una quarta vida blanca; només parpelleja en aparèixer/desaparèixer
 - perdre vida en tocar enemic / projectil / explosio [x]
 - respawn en perdre una vida [x] //animació de mort d'1,5 s; torna al punt d'inici amb 3 s d'invulnerabilitat; el nivell no es reinicia
 - conservar les vides restants entre nivells [x] //continua amb les mateixes vides i power-ups; una partida nova comença amb 3 vides
 - recuperar una vida en eliminar l'últim enemic [x] //màxim 4 (3 vermelles + 1 blanca); "1 UP" durant 3 s també al màxim; animació inversa als cors vermells, blinking al blanc; no reviu un game over
 - temps esgotat [x] //ara mata del tot (game over); valorar si nomes ha de treure una vida

Objectes:
 - Bomb Up [x]
 - Fire Up [x]
 - Speed Up [x]
 - VEST [x] //invulnerabilitat completa 17 s amb parpelleig i compte enrere al HUD; un altre VEST reinicia a 17 s
 - Clau amagada en arbust K [x] // obre una porta L; icona al HUD
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
 - plataformes temporals 'a' [x]
 - millora de càmera [x]

Tecles debug:
 - G god mode [x]
 - K matar tots els enemics i obrir porta [x]
 - 1-5 carregar nivell [x] //funciona des del menu i jugant (Game::keyPressed)

General (polish):
 - So efectes [x] //menu, bomba, explosio, item, mal
 - So: falten efectes [] //mort enemic, salt, porta, perdre vida, game over, victoria, boss
 - Musica de fons [] //Audio::playMusic existeix pero no es crida enlloc i no hi ha cap fitxer de musica a sounds/
 - Textura definitiva de la porta [x]
 - Fons amb cel i núvols [x]
 - Mort del boss [x] //postura de cop + partícules 4 s; parpelleig des del segon 2; la sortida espera que acabi
 - Mort del Walker [x] //3 fotogrames de bomberman.png durant 0,6 s, immòbil i sense fer mal; la sortida espera que acabi
 - Mort del Runner [x] //4 fotogrames de bomberman.png durant 0,8 s, immòbil i sense fer mal; la sortida espera que acabi
 - Mort del Shooter [x] //3 fotogrames de bomberman.png durant 0,6 s, immòbil i sense disparar ni fer mal; la sortida espera que acabi
 - Animacions dels enemics [x] //morts del Walker, Runner, Shooter i boss
 - Transicions entre pantalles (fade) []
 - Revisar bugs abans del lliurament [] //cada bug resta punts

Lliurament:
 - info.txt (noms, funcionalitats, instruccions) []
 - carpeta Binari (exe + dlls + images, levels, sounds, fonts, shaders) [] //compilar en Release
 - carpeta Projecte compilable []
 - demo.avi d'1 minut (< 100 MB) []
 - zip NomCognoms.zip (< 200 MB) []
