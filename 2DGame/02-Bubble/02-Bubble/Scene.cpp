#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"
#include "Walker.h"
#include "Runner.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25

#define INIT_WALKER_X_TILES 10
#define INIT_WALKER_Y_TILES 25

#define CAMERA_WIDTH_TILES 10


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

	Walker *walker = new Walker();
	walker->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	walker->setPosition(glm::vec2(INIT_WALKER_X_TILES * map->getTileSize(), INIT_WALKER_Y_TILES * map->getTileSize()));
	walker->setTileMap(map);
	enemies.push_back(walker);

	Runner *runner = new Runner();
	runner->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	runner->setPosition(glm::vec2((INIT_WALKER_X_TILES + 2) * map->getTileSize(), INIT_WALKER_Y_TILES * map->getTileSize()));
	runner->setTileMap(map);
	runner->setPlayer(player);
	enemies.push_back(runner);

	// Shared by every bomb so placing one does not reload the images from disk
	bombTexture.loadFromFile("images/bombs.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombTexture.setMinFilter(GL_NEAREST);
	bombTexture.setMagFilter(GL_NEAREST);
	explosionTexture.loadFromFile("images/explosions.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionTexture.setMinFilter(GL_NEAREST);
	explosionTexture.setMagFilter(GL_NEAREST);

	hud = new HUD();
	hud->init(texProgram);

	cameraSize = glm::vec2(CAMERA_WIDTH_TILES * map->getBlockSize(),
	                        CAMERA_WIDTH_TILES * map->getBlockSize() * float(SCREEN_HEIGHT) / float(SCREEN_WIDTH));
	updateCamera();
	currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
	if(player->takeBombRequest())
		tryPlayerBomb();
	for(unsigned int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->update(deltaTime);
		enemies[i]->updateHitCooldown(deltaTime);
		if(player->isAlive() && playerTouches(enemies[i]))
			player->loseLife();
	}
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
	glm::vec2 playerCenter = player->getPosition() + glm::vec2(16.f, 16.f);

	cameraPos = playerCenter - cameraSize / 2.f;
	cameraPos.x = glm::clamp(cameraPos.x, worldMin.x, glm::max(worldMin.x, worldMax.x - cameraSize.x));
	cameraPos.y = glm::clamp(cameraPos.y, worldMin.y, glm::max(worldMin.y, worldMax.y - cameraSize.y));
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
	for(unsigned int i = 0; i < bombs.size(); i++)
		bombs[i]->render();
	player->render();
	for(unsigned int i = 0; i < enemies.size(); i++)
		enemies[i]->render();

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



