#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "Walker.h"
#include "Runner.h"
#include "Shooter.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 2
#define INIT_PLAYER_Y_TILES 14

#define CAMERA_HEIGHT_TILES 20


Scene::Scene()
{
	map = NULL;
	player = NULL;
	hud = NULL;
}

Scene::~Scene()
{
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
	if(hud != NULL)
		delete hud;
}


void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
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

	spawnFromLevel();

	hud = new HUD();
	hud->init(texProgram);

	cameraSize = glm::vec2(CAMERA_HEIGHT_TILES * map->getTileSize() * float(SCREEN_WIDTH) / float(SCREEN_HEIGHT),
	                        CAMERA_HEIGHT_TILES * map->getTileSize());
	updateCamera();
	currentTime = 0.0f;
}

// Creates one enemy or item per letter found in the level file:
// W = Walker, R = Runner, Z = Shooter. Item letters mark bushes hiding power-ups.
// The letter marks the entity's top-left tile.
void Scene::spawnFromLevel()
{
	const vector<TileMap::Spawn> &spawns = map->getSpawns();
	for(unsigned int i = 0; i < spawns.size(); i++)
	{
		Item::ItemType itemType;
		if(Item::typeFromLetter(spawns[i].type, itemType))
		{
			spawnItem(itemType, spawns[i].tile);
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
	item->init(type, tile, glm::ivec2(SCREEN_X, SCREEN_Y), map, &itemTexture, texProgram);
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
			player->applyItem(items[i]->getType());
			items[i]->collect();
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
	currentTime += deltaTime;
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
		if(player->isAlive() && playerTouches(enemies[i]))
			player->loseLife();
		glm::vec2 shotOrigin, shotVelocity;
		if(enemies[i]->takeShot(shotOrigin, shotVelocity))
			spawnProjectile(shotOrigin, shotVelocity);
	}
	updateProjectiles(deltaTime);
	updateBombs(deltaTime);
	applyExplosionDamage();
	removeDeadEnemies();
	updateCamera();
	hud->update(deltaTime, player->getLives(), player->getMaxLives(), player->getNBombs());
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
	placeBomb(tile, player->getFirePower(), true);
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
bool Scene::placeBomb(const glm::ivec2 &tile, int range, bool playerOwned)
{
	if(!map->setBomb(tile.x, tile.y))
		return false;
	Bomb *bomb = new Bomb();
	bomb->init(tile, range, glm::ivec2(SCREEN_X, SCREEN_Y), map, &bombTexture, &explosionTexture, texProgram);
	bomb->setPlayerOwned(playerOwned);
	bombs.push_back(bomb);

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
		bool harmless = bombs[b]->isPlayerOwned() && player->isBombProof();
		if(player->isAlive() && !harmless && bombs[b]->flameTouches(player->getPosition(), player->getSize()))
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
	for(unsigned int i = 0; i < enemies.size(); )
	{
		if(enemies[i]->isDead())
		{
			delete enemies[i];
			enemies.erase(enemies.begin() + i);
		}
		else
			i++;
	}
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
		if(boxesOverlap(enemies[i]->getPosition(), enemies[i]->getSize(), bomb->getPosition(), bomb->getSize()))
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

void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	projection = glm::ortho(cameraPos.x, cameraPos.x + cameraSize.x, cameraPos.y + cameraSize.y, cameraPos.y);
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
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



