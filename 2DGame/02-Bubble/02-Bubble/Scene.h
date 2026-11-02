#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Enemy.h"
#include "Bomb.h"
#include "Item.h"
#include "Door.h"
#include "Projectile.h"
#include "HUD.h"
#include "Text.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init(const string &levelFile);
	void update(int deltaTime);
	void render();

	// True once the player has walked into the open door. The level freezes and shows the win screen.
	bool isCleared() const { return cleared; }
	Player::PowerUps getPowerUps() const { return player->getPowerUps(); }
	void setPowerUps(const Player::PowerUps &powerUps) { player->setPowerUps(powerUps); }

private:
	void initShaders();
	void spawnFromLevel();
	void spawnItem(Item::ItemType type, const glm::ivec2 &tile);
	void collectItems();
	void removeCollectedItems();
	void spawnProjectile(const glm::vec2 &origin, const glm::vec2 &velocity);
	void updateProjectiles(int deltaTime);
	void updateCamera();
	void updateDoor(int deltaTime);
	void renderEndScreen(const string &title, const glm::vec4 &titleColor, const string &hint);
	void tryPlayerBomb();
	bool placeBomb(const glm::ivec2 &tile, int range, bool playerOwned);
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
	std::vector<Item *> items;
	Door *door; // NULL if the level file has no D
	std::vector<Door *> keyGates;
	bool hasKey;
	std::vector<Projectile *> projectiles;
	Texture bombTexture, explosionTexture, itemTexture, personalizedTexture;
	HUD *hud;
	Text endTitleText, endHintText; // Death and win screens
	int score;
	ShaderProgram texProgram;
	float currentTime;
	int timeLeft; // ms until the level time limit runs out
	bool cleared;
	glm::mat4 projection;
	glm::vec2 cameraPos, cameraSize;

};


#endif // _SCENE_INCLUDE

