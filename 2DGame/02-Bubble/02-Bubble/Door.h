#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// The level exit. It occupies a single cell of the tile map and starts closed.
// Scene opens it once every enemy is dead; touching it while open clears the level.
// A key gate is a separate solid passage; opening it removes its map collision.

class Door
{
public:
	Door();
	~Door();

public:
	void init(const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap, ShaderProgram &shaderProgram,
	          bool keyGate = false);
	void update(int deltaTime);
	void render();

	void open();
	bool isOpen() const { return bOpen; }
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl + tile * map->getTileSize()); }
	glm::ivec2 getSize() const { return glm::ivec2(map->getTileSize()); }

private:
	glm::ivec2 tileMapDispl, tile;
	bool bOpen, keyGate;
	Texture spritesheet;
	Sprite *sprite;
	TileMap *map;

};


#endif // _DOOR_INCLUDE
