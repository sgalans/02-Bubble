#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// A bomb occupies a single cell of the tile map and does not fall. It marks
// its cell in the map's bomb mirror while the fuse burns, then explodes.
// Scene owns the bombs and deletes them once they reach DONE.

class Bomb
{
public:
	enum BombState { FUSE, EXPLODING, DONE };

public:
	Bomb();
	~Bomb();

public:
	void init(const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap,
	          Texture *spritesheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	void explode();

	glm::ivec2 getTile() const { return tile; }
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl + tile * map->getTileSize()); }
	glm::ivec2 getSize() const { return glm::ivec2(map->getTileSize()); }
	BombState getState() const { return state; }
	bool isDone() const { return state == DONE; }

private:
	glm::ivec2 tileMapDispl, tile;
	BombState state;
	int timer;
	Sprite *sprite;
	TileMap *map;

};


#endif // _BOMB_INCLUDE
