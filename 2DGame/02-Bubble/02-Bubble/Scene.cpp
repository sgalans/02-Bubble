#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25

#define CAMERA_WIDTH_TILES 10


Scene::Scene()
{
	map = NULL;
	player = NULL;
}

Scene::~Scene()
{
	texProgram.free();
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
}


void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/level01.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	cameraSize = glm::vec2(CAMERA_WIDTH_TILES * map->getBlockSize(),
	                        CAMERA_WIDTH_TILES * map->getBlockSize() * float(SCREEN_HEIGHT) / float(SCREEN_WIDTH));
	updateCamera();
	currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
	updateCamera();
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
	player->render();
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



