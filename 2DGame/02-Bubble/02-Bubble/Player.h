#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"
#include "Item.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{
public:
	struct PowerUps
	{
		int nBombs, firePower;
		float moveSpeed;
		bool bombProof;
	};

	Player();
	~Player();

public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	
	void setTileMap(TileMap *tileMap);
	// Position uses the sprite's top-left; getPosition returns the hitbox's top-left.
	void setPosition(const glm::vec2 &pos);
	// Moves the player there and remembers it as the respawn point after losing a life.
	void setSpawnPoint(const glm::vec2 &pos);
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl.x + posPlayer.x, tileMapDispl.y + posPlayer.y); }
	glm::ivec2 getSize() const { return sizePlayer; }

	void loseLife();
	// Takes every life at once, ignoring invulnerability and god mode (used when time runs out)
	void loseAllLives();
	int getLives() const { return lives; }
	int getMaxLives() const { return maxLives; }
	int getNBombs() const { return nBombs; }
	int getFirePower() const { return firePower; }
	// Own bombs do not hurt him (other bombs still do)
	bool isBombProof() const { return bombProof; }
	bool isAlive() const { return lives > 0 && !isDying(); }
	bool isDying() const { return deathTime > 0; }
	bool isGameOver() const { return lives <= 0 && !isDying(); }
	bool isInvulnerable() const { return isDying() || invulnTime > 0 || invensible; }
	bool takeBombRequest();
	void applyItem(Item::ItemType type);
	PowerUps getPowerUps() const;
	void setPowerUps(const PowerUps &powerUps);

private:
	void updateSpritePosition();
	void startDeath();

private:
	bool facingRight;
	bool bJumpKeyPressed;
	bool bGodKeyPressed;
	bool bBombKeyPressed;
	bool bBombRequested;
	glm::ivec2 tileMapDispl, posPlayer, sizePlayer;
	glm::vec2 spawnPoint; // sprite top-left, same convention as setPosition
	float verticalPosition, verticalVelocity, horizontalRemainder;
	int moveDirection;
	bool shortJumpActive;
	int lives, maxLives;
	int invulnTime;
	int deathTime, blinkTime;
	int nBombs;
	int firePower; // flame length in tiles
	float moveSpeed; // pixels per frame
	bool bombProof;
	Texture spritesheet;
	Sprite *sprite, *deathSprite;
	TileMap *map;
	bool invensible;

};


#endif // _PLAYER_INCLUDE


