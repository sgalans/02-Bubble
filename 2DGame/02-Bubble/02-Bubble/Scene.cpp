#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "Audio.h"
#include "enemies/Walker.h"
#include "enemies/Runner.h"
#include "enemies/Shooter.h"
#include "enemies/Boss.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 2
#define INIT_PLAYER_Y_TILES 14

#define CAMERA_HEIGHT_TILES 10

#define FONT_FILE "fonts/PressStart2P-Regular.ttf"
// The font is drawn on an 8 pixel grid, so multiples of 8 keep it crisp
#define END_TITLE_SIZE 32
#define END_HINT_SIZE 16
#define END_LINE_SPACING 24
#define EXIT_NOTICE_TIME 3000 // ms

// Final victory picture (160x80), drawn at 4x and centred vertically
#define END_SCREEN_FILE "images/EndScreen.png"
#define END_SCREEN_SCALE 4


Scene::Scene()
{
	map = NULL;
	player = NULL;
	door = NULL;
	hasKey = false;
	showKeyHint = false;
	exitNoticeTime = 0;
	hud = NULL;
	skySprite = NULL;
	cloudSprite = NULL;
	endScreenSprite = NULL;
}

Scene::~Scene()
{
	delete skySprite;
	delete cloudSprite;
	delete endScreenSprite;
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
	for(unsigned int i = 0; i < enemies.size(); i++)
		delete enemies[i];
	for(unsigned int i = 0; i < bombs.size(); i++)
		delete bombs[i];
	for(unsigned int i = 0; i < items.size(); i++)
		delete items[i];
	for(unsigned int i = 0; i < projectiles.size(); i++)
		delete projectiles[i];
	if(door != NULL)
		delete door;
	for(unsigned int i = 0; i < keyGates.size(); i++)
		delete keyGates[i];
	if(hud != NULL)
		delete hud;
}


void Scene::init(const string &levelFile, int timeLimit)
{
	initShaders();
	map = TileMap::createTileMap(levelFile, glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setSpawnPoint(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);

	// Shared by every bomb so placing one does not reload the images from disk
	bombTexture.loadFromFile("images/bombs.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombTexture.setMinFilter(GL_NEAREST);
	bombTexture.setMagFilter(GL_NEAREST);
	explosionTexture.loadFromFile("images/explosions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionTexture.setMinFilter(GL_NEAREST);
	explosionTexture.setMagFilter(GL_NEAREST);
	// Power-up icons, shared by every item and also used for the shooter's
	// balls. Loaded before spawning them.
	itemTexture.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	itemTexture.setMinFilter(GL_NEAREST);
	itemTexture.setMagFilter(GL_NEAREST);
	personalizedTexture.loadFromFile("images/personalized.png", TEXTURE_PIXEL_FORMAT_RGBA);
	personalizedTexture.setMinFilter(GL_NEAREST);
	personalizedTexture.setMagFilter(GL_NEAREST);
	// Row 3 holds sky, then the two adjacent halves of a cloud.
	glm::vec2 atlasSize(float(personalizedTexture.width()), float(personalizedTexture.height()));
	float tileSize = float(map->getTileSize());
	skySprite = Sprite::createSprite(glm::vec2(tileSize), glm::vec2(16.f) / atlasSize,
	                                 &personalizedTexture, &texProgram);
	skySprite->setNumberAnimations(1);
	skySprite->setAnimationSpeed(0, 1);
	skySprite->addKeyframe(0, glm::vec2(32.f, 48.f) / atlasSize);
	skySprite->changeAnimation(0);
	cloudSprite = Sprite::createSprite(glm::vec2(2.f * tileSize, tileSize), glm::vec2(32.f, 16.f) / atlasSize,
	                                   &personalizedTexture, &texProgram);
	cloudSprite->setNumberAnimations(1);
	cloudSprite->setAnimationSpeed(0, 1);
	cloudSprite->addKeyframe(0, glm::vec2(48.f, 48.f) / atlasSize);
	cloudSprite->changeAnimation(0);

	spawnFromLevel();

	hud = new HUD();
	hud->init(texProgram);
	endTitleText.init(FONT_FILE, END_TITLE_SIZE, texProgram);
	endHintText.init(FONT_FILE, END_HINT_SIZE, texProgram);

	cameraSize = glm::vec2(CAMERA_HEIGHT_TILES * map->getTileSize() * float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
	                        CAMERA_HEIGHT_TILES * map->getTileSize());
	updateCamera();
	currentTime = 0.0f;
	timeLeft = timeLimit;
	cleared = false;
}

// Creates entities and decorations from the level's spawn markers:
// W = Walker, R = Runner, Z = Shooter, X = Boss, D = exit, L = key gate, @ = player
// C marks clouds; Item::typeFromLetter identifies bushes hiding items.
// TileMap handles a platforms, T trampolines and U buttons separately.
// The letter marks the entity's top-left tile.
void Scene::spawnFromLevel()
{
	cloudTiles.clear();
	const vector<TileMap::Spawn> &spawns = map->getSpawns();
	for(unsigned int i = 0; i < spawns.size(); i++)
	{
		if(spawns[i].type == '@')
		{
			player->setSpawnPoint(glm::vec2(spawns[i].tile * map->getTileSize()));
			continue;
		}

		if(spawns[i].type == 'C')
		{
			cloudTiles.push_back(spawns[i].tile);
			continue;
		}

		Item::ItemType itemType;
		if(Item::typeFromLetter(spawns[i].type, itemType))
		{
			spawnItem(itemType, spawns[i].tile);
			continue;
		}

		if(spawns[i].type == 'L')
		{
			Door *gate = new Door();
			gate->init(spawns[i].tile, glm::ivec2(SCREEN_X, SCREEN_Y), map, texProgram, true);
			keyGates.push_back(gate);
			continue;
		}

		if(spawns[i].type == 'D')
		{
			if(door != NULL)
				cout << "More than one door in level file, keeping the first" << endl;
			else
			{
				door = new Door();
				door->init(spawns[i].tile, glm::ivec2(SCREEN_X, SCREEN_Y), map, texProgram);
			}
			continue;
		}

		Enemy *enemy;
		switch(spawns[i].type)
		{
		case 'W':
			enemy = new Walker();
			break;
		case 'R':
		{
			Runner *runner = new Runner();
			runner->setPlayer(player);
			enemy = runner;
			break;
		}
		case 'Z':
		{
			Shooter *shooter = new Shooter();
			shooter->setPlayer(player);
			enemy = shooter;
			break;
		}
		case 'X':
		{
			Boss *boss = new Boss();
			boss->setPlayer(player);
			enemy = boss;
			break;
		}
		default:
			cout << "Unknown letter '" << spawns[i].type << "' in level file" << endl;
			continue;
		}
		enemy->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
		enemy->setPosition(glm::vec2(spawns[i].tile * map->getTileSize()));
		enemy->setTileMap(map);
		enemies.push_back(enemy);
	}
}

void Scene::spawnItem(Item::ItemType type, const glm::ivec2 &tile)
{
	Item *item = new Item();
	Texture *sheet = type == Item::KEY ? &personalizedTexture : &itemTexture;
	item->init(type, tile, glm::ivec2(SCREEN_X, SCREEN_Y), map, sheet, texProgram);
	items.push_back(item);
}

// The player picks up every item he overlaps
void Scene::collectItems()
{
	if(!player->isAlive())
		return;
	for(unsigned int i = 0; i < items.size(); i++)
	{
		if(items[i]->isAvailable() &&
		   boxesOverlap(player->getPosition(), player->getSize(), items[i]->getPosition(), items[i]->getSize()))
		{
			if(items[i]->getType() == Item::KEY)
				hasKey = true;
			else
				player->applyItem(items[i]->getType());
			items[i]->collect();
			Audio::instance().play(Audio::ITEM_PICKUP);
		}
	}
}

void Scene::removeCollectedItems()
{
	for(unsigned int i = 0; i < items.size(); )
	{
		if(items[i]->isCollected())
		{
			delete items[i];
			items.erase(items.begin() + i);
		}
		else
			i++;
	}
}

void Scene::spawnProjectile(const glm::vec2 &origin, const glm::vec2 &velocity)
{
	Projectile *projectile = new Projectile();
	projectile->init(origin, velocity, glm::ivec2(SCREEN_X, SCREEN_Y), map, &itemTexture, texProgram);
	projectiles.push_back(projectile);
}

// Balls hurt the player on contact and vanish, as they do against walls and
// bombs (Projectile::update handles those).
void Scene::updateProjectiles(int deltaTime)
{
	for(unsigned int i = 0; i < projectiles.size(); i++)
	{
		projectiles[i]->update(deltaTime);
		if(!projectiles[i]->isDone() && player->isAlive() &&
		   boxesOverlap(player->getPosition(), player->getSize(), projectiles[i]->getPosition(), projectiles[i]->getSize()))
		{
			player->loseLife();
			projectiles[i]->destroy();
		}
	}

	for(unsigned int i = 0; i < projectiles.size(); )
	{
		if(projectiles[i]->isDone())
		{
			delete projectiles[i];
			projectiles.erase(projectiles.begin() + i);
		}
		else
			i++;
	}
}

void Scene::update(int deltaTime)
{
	// Everything stops on the win screen
	if(cleared)
		return;
	if(player->isExiting())
	{
		player->update(deltaTime);
		updateCamera();
		cleared = player->hasEnteredExit();
		return;
	}
	currentTime += deltaTime;
	exitNoticeTime = glm::max(0, exitNoticeTime - deltaTime);
	// The clock pauses while dying; exiting is handled above. Timeout removes
	// all lives and plays the death animation before showing the defeat screen.
	if(player->isAlive())
	{
		timeLeft = glm::max(0, timeLeft - deltaTime);
		if(timeLeft == 0)
			player->loseAllLives();
	}
	// Map updates precede movement so disappearing support takes effect immediately.
	vector<glm::ivec4> occupiedBoxes;
	if(player->isAlive())
		occupiedBoxes.push_back(glm::ivec4(glm::ivec2(player->getPosition()) - map->getPosition(), player->getSize()));
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(!enemies[i]->isDead() && !enemies[i]->isFlying())
			occupiedBoxes.push_back(glm::ivec4(glm::ivec2(enemies[i]->getPosition()) - map->getPosition(), enemies[i]->getSize()));
	}
	int ridingTrampoline = -1;
	vector<int> enemyTrampolines(enemies.size(), -1);
	vector<int> previousSpringTops;
	const vector<TileMap::Trampoline> &springs = map->getTrampolines();
	glm::ivec2 playerPos = glm::ivec2(player->getPosition()) - map->getPosition();
	for(unsigned int i = 0; i < springs.size(); i++)
	{
		int left = springs[i].tile.x * map->getTileSize();
		int top = map->getTrampolineTop(springs[i]);
		previousSpringTops.push_back(top);
		for(unsigned int j = 0; j < enemies.size(); j++)
		{
			if(enemies[j]->isDead() || enemies[j]->isFlying())
				continue;
			glm::ivec2 pos = glm::ivec2(enemies[j]->getPosition()) - map->getPosition();
			glm::ivec2 size = enemies[j]->getSize();
			if(pos.y + size.y == top && pos.x < left + map->getTileSize() && left < pos.x + size.x)
				enemyTrampolines[j] = int(i);
		}
		if(player->isStandingOnSurface(map->getTrampolineTop(springs[i])) &&
		   playerPos.x < left + map->getTileSize() && left < playerPos.x + player->getSize().x)
		{
			ridingTrampoline = int(i);
		}
	}
	map->update(deltaTime, occupiedBoxes);
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		int spring = enemyTrampolines[i];
		if(spring >= 0)
		{
			int shift = map->getTrampolineTop(springs[spring]) - previousSpringTops[spring];
			enemies[i]->setPosition(enemies[i]->getPosition() - glm::vec2(map->getPosition()) + glm::vec2(0, shift));
		}
	}
	if(ridingTrampoline >= 0)
	{
		const TileMap::Trampoline &spring = springs[ridingTrampoline];
		player->liftByTrampoline(map->getTrampolineTop(spring), spring.launched);
	}
	player->update(deltaTime);
	if(player->takeBombRequest())
		tryPlayerBomb();
	for(unsigned int i = 0; i < items.size(); i++)
		items[i]->update(deltaTime);
	collectItems();
	removeCollectedItems();
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->update(deltaTime);
		enemies[i]->updateHitCooldown(deltaTime);
		if(!enemies[i]->isDead() && player->isAlive() && playerTouches(enemies[i]))
			player->loseLife();
		glm::vec2 shotOrigin, shotVelocity;
		// The boss can fire several balls at once
		while(enemies[i]->takeShot(shotOrigin, shotVelocity))
			spawnProjectile(shotOrigin, shotVelocity);
	}
	updateProjectiles(deltaTime);
	updateBombs(deltaTime);
	applyExplosionDamage();
	removeDeadEnemies();
	updateDoor(deltaTime);
	updateCamera();
	hud->update(deltaTime, player->getLives(), player->getMaxLives(), player->getNBombs(),
	            player->getFirePower(), int(enemies.size()), timeLeft, hasKey,
	            player->getSpeedUpLevel(), player->getVestTime());
}

// Drops a bomb on the cell at the player's feet, horizontally centred on him.
// Bombs ignore gravity, so this also works in mid-air.
void Scene::tryPlayerBomb()
{
	if(activeBombs() >= player->getNBombs())
		return;

	glm::ivec2 pos = glm::ivec2(player->getPosition()) - map->getPosition();
	glm::ivec2 size = player->getSize();
	glm::ivec2 tile((pos.x + size.x / 2) / map->getTileSize(), (pos.y + size.y - 1) / map->getTileSize());
	placeBomb(tile, player->getFirePower());
}

// Only bombs still burning their fuse count against the player's limit
int Scene::activeBombs() const
{
	int count = 0;
	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		if(bombs[i]->getState() == Bomb::FUSE)
			count++;
	}

	return count;
}

// Puts a bomb on the given cell if it is free. Returns false otherwise.
bool Scene::placeBomb(const glm::ivec2 &tile, int range)
{
	if(!map->setBomb(tile.x, tile.y))
		return false;
	Bomb *bomb = new Bomb();
	bomb->init(tile, range, glm::ivec2(SCREEN_X, SCREEN_Y), map, &bombTexture, &explosionTexture, texProgram);
	bombs.push_back(bomb);
	Audio::instance().play(Audio::BOMB_PLACE);

	return true;
}

void Scene::updateBombs(int deltaTime)
{
	for(unsigned int i = 0; i < bombs.size(); i++)
	{
		bombs[i]->update(deltaTime);

		// A fresh bomb stays passable until whoever was on it has walked out
		glm::ivec2 tile = bombs[i]->getTile();
		if(map->isBombPassable(tile.x, tile.y) && !someoneInside(bombs[i]))
			map->solidifyBomb(tile.x, tile.y);
	}
	chainExplosions();

	for(unsigned int i = 0; i < bombs.size(); )
	{
		if(bombs[i]->isDone())
		{
			delete bombs[i];
			bombs.erase(bombs.begin() + i);
		}
		else
			i++;
	}
}

// Bombs reached by a flame explode at once. Repeats until nothing changes so
// a whole chain goes off in the same frame.
void Scene::chainExplosions()
{
	bool changed = true;
	while(changed)
	{
		changed = false;
		for(unsigned int i = 0; i < bombs.size(); i++)
		{
			if(bombs[i]->getState() != Bomb::FUSE)
				continue;
			for(unsigned int j = 0; j < bombs.size(); j++)
			{
				if(bombs[j]->flameCovers(bombs[i]->getTile()))
				{
					bombs[i]->explode();
					changed = true;
					break;
				}
			}
		}
	}
}

// Flames hurt everybody, the player included. Player::loseLife already
// ignores hits while he is invulnerable or in god mode.
void Scene::applyExplosionDamage()
{
	for(unsigned int b = 0; b < bombs.size(); b++)
	{
		if(bombs[b]->getState() != Bomb::EXPLODING)
			continue;
		if(player->isAlive() && bombs[b]->flameTouches(player->getPosition(), player->getSize()))
			player->loseLife();
		for(unsigned int i = 0; i < enemies.size(); i++)
		{
			if(bombs[b]->flameTouches(enemies[i]->getPosition(), enemies[i]->getSize()))
				enemies[i]->takeHit();
		}
	}
}

void Scene::removeDeadEnemies()
{
	bool hadEnemies = !enemies.empty();
	for(unsigned int i = 0; i < enemies.size(); )
	{
		if(enemies[i]->isReadyToRemove())
		{
			delete enemies[i];
			enemies.erase(enemies.begin() + i);
		}
		else
			i++;
	}
	if(hadEnemies && enemies.empty())
	{
		exitNoticeTime = EXIT_NOTICE_TIME;
		if(player->getLives() > 0)
			player->setLives(player->getLives() + 1);
	}
}

// With no enemies left, the exit opens. Entering it starts the player's
// exit animation; update() marks the level cleared once that finishes.
void Scene::updateDoor(int deltaTime)
{
	showKeyHint = false;
	glm::ivec2 playerFeet = glm::ivec2(player->getPosition()) - map->getPosition();
	playerFeet += glm::ivec2(player->getSize().x / 2, player->getSize().y - 1);
	glm::ivec2 playerTile = playerFeet / map->getTileSize();
	for(unsigned int i = 0; i < keyGates.size(); i++)
	{
		Door *gate = keyGates[i];
		gate->update(deltaTime);
		// The closed gate is solid, so include flush contact in the overlap test.
		if(!gate->isOpen() && hasKey && player->isAlive() &&
		   boxesOverlap(player->getPosition() - glm::vec2(1.f), player->getSize() + glm::ivec2(2),
		                gate->getPosition(), gate->getSize()))
		{
			gate->open();
			if(gate->isOpen())
			{
				hasKey = false;
				Audio::instance().play(Audio::MENU_SELECT);
			}
		}
		if(!gate->isOpen() && !hasKey && player->isAlive())
		{
			glm::ivec2 gateTile = (glm::ivec2(gate->getPosition()) - map->getPosition()) / map->getTileSize();
			glm::ivec2 distance = glm::abs(playerTile - gateTile);
			if(distance.x + distance.y == 1)
				showKeyHint = true;
		}
	}

	if(door == NULL)
		return;
	door->update(deltaTime);
	if(!door->isOpen() && enemies.empty())
		door->open();
	if(door->isOpen() && player->isAlive())
	{
		glm::vec2 center = player->getPosition() + glm::vec2(player->getSize()) / 2.f;
		glm::vec2 doorMin = door->getPosition(), doorMax = doorMin + glm::vec2(door->getSize());
		if(center.x >= doorMin.x && center.x < doorMax.x && center.y >= doorMin.y && center.y < doorMax.y)
		{
			player->enterExit(door->getPosition());
			showKeyHint = false;
			exitNoticeTime = 0;
		}
	}
}

// K kills every enemy and clears projectiles. The boss finishes its death
// animation before removal; updateDoor opens the exit once the list is empty.
void Scene::killAllEnemies()
{
	if(cleared)
		return;
	for(unsigned int i = 0; i < enemies.size(); i++)
		enemies[i]->kill();
	removeDeadEnemies();
	for(unsigned int i = 0; i < projectiles.size(); i++)
		projectiles[i]->destroy();
}

bool Scene::playerTouches(const Enemy *enemy) const
{
	return boxesOverlap(player->getPosition(), player->getSize(), enemy->getPosition(), enemy->getSize());
}

bool Scene::someoneInside(const Bomb *bomb) const
{
	if(boxesOverlap(player->getPosition(), player->getSize(), bomb->getPosition(), bomb->getSize()))
		return true;
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		if(!enemies[i]->isDead() && boxesOverlap(enemies[i]->getPosition(), enemies[i]->getSize(), bomb->getPosition(), bomb->getSize()))
			return true;
	}

	return false;
}

bool Scene::boxesOverlap(const glm::vec2 &posA, const glm::ivec2 &sizeA,
                         const glm::vec2 &posB, const glm::ivec2 &sizeB)
{
	return posA.x < posB.x + sizeB.x && posB.x < posA.x + sizeA.x &&
	       posA.y < posB.y + sizeB.y && posB.y < posA.y + sizeA.y;
}

void Scene::updateCamera()
{
	glm::vec2 worldMin = glm::vec2(map->getPosition());
	glm::vec2 worldMax = worldMin + glm::vec2(map->getMapSize()) * float(map->getTileSize());
	glm::vec2 playerCenter = player->getPosition() + glm::vec2(player->getSize()) / 2.f;

	cameraPos = playerCenter - cameraSize / 2.f;
	// Centre small maps; follow the player when the map exceeds the viewport.
	for(int axis = 0; axis < 2; axis++)
	{
		if(worldMax[axis] - worldMin[axis] <= cameraSize[axis])
			cameraPos[axis] = (worldMin[axis] + worldMax[axis] - cameraSize[axis]) / 2.f;
		else
			cameraPos[axis] = glm::clamp(cameraPos[axis], worldMin[axis], worldMax[axis] - cameraSize[axis]);
	}
}

void Scene::renderBackground()
{
	const int tileSize = map->getTileSize();
	const int firstX = int(floor(cameraPos.x / tileSize));
	const int firstY = int(floor(cameraPos.y / tileSize));
	const int lastX = int(ceil((cameraPos.x + cameraSize.x) / tileSize));
	const int lastY = int(ceil((cameraPos.y + cameraSize.y) / tileSize));
	for(int y = firstY; y < lastY; y++)
	{
		for(int x = firstX; x < lastX; x++)
		{
			skySprite->setPosition(glm::vec2(x * tileSize, y * tileSize));
			skySprite->render();
		}
	}
	// C marks the left half; both halves are drawn as one decorative sprite.
	for(unsigned int i = 0; i < cloudTiles.size(); i++)
	{
		cloudSprite->setPosition(glm::vec2(map->getPosition() + cloudTiles[i] * tileSize));
		cloudSprite->render();
	}
}

void Scene::setFinalLevel(bool final)
{
	delete endScreenSprite;
	endScreenSprite = NULL;
	if(!final)
		return;
	endScreenTexture.loadFromFile(END_SCREEN_FILE, TEXTURE_PIXEL_FORMAT_RGBA);
	endScreenTexture.setMinFilter(GL_NEAREST);
	endScreenTexture.setMagFilter(GL_NEAREST);
	glm::vec2 size(float(END_SCREEN_SCALE * endScreenTexture.width()),
	               float(END_SCREEN_SCALE * endScreenTexture.height()));
	endScreenSprite = Sprite::createSprite(size, glm::vec2(1.f, 1.f), &endScreenTexture, &texProgram);
	endScreenSprite->setNumberAnimations(1);
	endScreenSprite->setAnimationSpeed(0, 1);
	endScreenSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	endScreenSprite->changeAnimation(0);
	endScreenSprite->setPosition(glm::vec2((SCREEN_WIDTH - size.x) / 2.f, (SCREEN_HEIGHT - size.y) / 2.f));
}

// Beating the last level: the picture on a black background with the title over it
void Scene::renderFinalScreen()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // Game's normal clear colour

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	texProgram.setUniformMatrix4f("modelview", glm::mat4(1.0f));
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	endScreenSprite->render();

	float imageTop = (SCREEN_HEIGHT - END_SCREEN_SCALE * endScreenTexture.height()) / 2.f;
	float imageBottom = SCREEN_HEIGHT - imageTop;
	endTitleText.renderCentered("YOU WIN", imageTop + END_LINE_SPACING, SCREEN_WIDTH,
	                            glm::vec4(1.f, 0.85f, 0.2f, 1.f));
	endHintText.renderCentered("PRESS ENTER TO RETURN TO MAIN MENU",
	                           imageBottom + (imageTop - END_HINT_SIZE) / 2.f, SCREEN_WIDTH,
	                           glm::vec4(1.f, 1.f, 1.f, 1.f));
}

void Scene::render()
{
	glm::mat4 modelview;

	if(cleared && endScreenSprite != NULL)
	{
		renderFinalScreen();
		return;
	}

	texProgram.use();
	projection = glm::ortho(cameraPos.x, cameraPos.x + cameraSize.x, cameraPos.y + cameraSize.y, cameraPos.y);
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	renderBackground();
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	if(door != NULL)
		door->render();
	for(unsigned int i = 0; i < keyGates.size(); i++)
		keyGates[i]->render();
	for(unsigned int i = 0; i < items.size(); i++)
		items[i]->render();
	for(unsigned int i = 0; i < bombs.size(); i++)
		bombs[i]->render();
	player->render();
	for(unsigned int i = 0; i < enemies.size(); i++)
		enemies[i]->render();
	for(unsigned int i = 0; i < projectiles.size(); i++)
		projectiles[i]->render();

	// HUD pass: screen-space projection so it does not scroll with the camera
	texProgram.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	texProgram.setUniformMatrix4f("modelview", glm::mat4(1.0f));
	hud->render();
	if(cleared)
		renderEndScreen("YOU WIN", glm::vec4(1.f, 0.85f, 0.2f, 1.f), "PRESS ENTER TO CONTINUE");
	else if(player->isGameOver())
		renderEndScreen("YOU ARE DEAD", glm::vec4(1.f, 0.f, 0.f, 1.f), "PRESS ESC TO RETURN TO MAIN MENU");
	else if(player->isAlive())
	{
		float noticeY = (SCREEN_HEIGHT - END_TITLE_SIZE) / 2.f;
		if(exitNoticeTime > 0)
		{
			endTitleText.renderCentered("EXIT!", noticeY + 2.f, SCREEN_WIDTH, glm::vec4(0.f, 0.f, 0.f, 1.f));
			endTitleText.renderCentered("EXIT!", noticeY, SCREEN_WIDTH, glm::vec4(1.f, 0.85f, 0.2f, 1.f));
			float lifeUpY = noticeY - END_TITLE_SIZE - END_LINE_SPACING;
			endTitleText.renderCentered("1 UP", lifeUpY + 2.f, SCREEN_WIDTH, glm::vec4(0.f, 0.f, 0.f, 1.f));
			endTitleText.renderCentered("1 UP", lifeUpY, SCREEN_WIDTH, glm::vec4(0.3f, 1.f, 0.3f, 1.f));
		}
		if(showKeyHint)
		{
			float hintY = noticeY + END_TITLE_SIZE + END_LINE_SPACING;
			endHintText.renderCentered("You need a key", hintY + 2.f, SCREEN_WIDTH, glm::vec4(0.f, 0.f, 0.f, 1.f));
			endHintText.renderCentered("You need a key", hintY, SCREEN_WIDTH, glm::vec4(1.f));
		}
	}
}

// Title centred on screen with a smaller white hint under it. Game::keyPressed
// handles the keys: Escape goes back to the menu, Enter continues after a win.
void Scene::renderEndScreen(const string &title, const glm::vec4 &titleColor, const string &hint)
{
	float titleY = (SCREEN_HEIGHT - END_TITLE_SIZE - END_LINE_SPACING - END_HINT_SIZE) / 2.f;
	endTitleText.renderCentered(title, titleY, SCREEN_WIDTH, titleColor);
	endHintText.renderCentered(hint, titleY + END_TITLE_SIZE + END_LINE_SPACING, SCREEN_WIDTH,
	                           glm::vec4(1.f, 1.f, 1.f, 1.f));
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



