#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "HUD.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	void update(int deltaTime);
	void render();

private:
	void initShaders();
	void updateCamera();
	bool playerTouches(const Enemy *enemy) const;

private:
	TileMap *map;
	Player *player;
	std::vector<Enemy *> enemies;
	int aliveEnemies;
	HUD *hud;
	int score;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	glm::vec2 cameraPos, cameraSize;

};


#endif // _SCENE_INCLUDE

