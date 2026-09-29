#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// Abstract base for every enemy. It owns the sprite, the tile map reference
// and the position; each subclass decides how it moves and what it can do.

class Enemy
{
public:
	Enemy();
	virtual ~Enemy();

public:
	virtual void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram) = 0;
	virtual void update(int deltaTime) = 0;
	virtual void render();

	void setTileMap(TileMap *tileMap);
	void setPosition(const glm::vec2 &pos);
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl.x + posEnemy.x, tileMapDispl.y + posEnemy.y); }
	glm::ivec2 getSize() const { return sizeEnemy; }

protected:
	void updateSpritePosition();

protected:
	glm::ivec2 tileMapDispl, posEnemy, sizeEnemy;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;
	int lives;

};


#endif // _ENEMY_INCLUDE
