#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "Bomb.h"
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
	void spawnEnemies();
	void updateCamera();
	void tryPlayerBomb();
	bool placeBomb(const glm::ivec2 &tile, int range);
	void chainExplosions();
	void applyExplosionDamage();
	void removeDeadEnemies();
	int activeBombs() const;
	void updateBombs(int deltaTime);
	bool playerTouches(const Enemy *enemy) const;
	bool someoneInside(const Bomb *bomb) const;
	static bool boxesOverlap(const glm::vec2 &posA, const glm::ivec2 &sizeA,
	                         const glm::vec2 &posB, const glm::ivec2 &sizeB);

private:
	TileMap *map;
	Player *player;
	std::vector<Enemy *> enemies;
	std::vector<Bomb *> bombs;
	Texture bombTexture, explosionTexture;
	HUD *hud;
	int score;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	glm::vec2 cameraPos, cameraSize;

};


#endif // _SCENE_INCLUDE

