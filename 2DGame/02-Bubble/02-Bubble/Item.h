#ifndef _ITEM_INCLUDE
#define _ITEM_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// A power-up that occupies a single cell of the tile map and does not fall.
// It stays hidden until its bush is destroyed, then waits for player contact.
// Scene applies its effect, marks it collected and deletes it.
// Scene owns the items and shares one icon sheet between all of them.

class Item
{
public:
	enum ItemType { BOMB_UP, FIRE_UP, SPEED_UP, BOMB_PROOF, N_TYPES };

public:
	Item();
	~Item();

public:
	void init(ItemType type, const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap,
	          Texture *sheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	// Level file letter for each type. Returns false for letters that are not items.
	static bool typeFromLetter(char letter, ItemType &type);

	ItemType getType() const { return type; }
	glm::ivec2 getTile() const { return tile; }
	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl + tile * map->getTileSize()); }
	glm::ivec2 getSize() const { return glm::ivec2(map->getTileSize()); }
	void collect() { collected = true; }
	bool isCollected() const { return collected; }
	bool isAvailable() const { return !collected && !map->isWall(tile.x, tile.y); }

private:
	ItemType type;
	glm::ivec2 tileMapDispl, tile;
	bool collected;
	Sprite *sprite;
	TileMap *map;

};


#endif // _ITEM_INCLUDE
