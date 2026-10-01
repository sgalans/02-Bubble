#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl.x + posPlayer.x, tileMapDispl.y + posPlayer.y); }
	glm::ivec2 getSize() const { return sizePlayer; }

	void loseLife();
	int getLives() const { return lives; }
	int getMaxLives() const { return maxLives; }
	int getNBombs() const { return nBombs; }
	bool isAlive() const { return lives > 0; }
	bool isInvulnerable() const { return invulnTime > 0 || invensible; }

private:
	bool bJumping;
	bool bJumpKeyPressed;
	glm::ivec2 tileMapDispl, posPlayer, sizePlayer;
	int jumpAngle, startY;
	int lives, maxLives;
	int invulnTime;
	int nBombs;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	bool invensible;

};


#endif // _PLAYER_INCLUDE


