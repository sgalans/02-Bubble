#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE


#include <vector>
#include "Sprite.h"
#include "TileMap.h"


// A bomb occupies a single cell of the tile map and does not fall. It marks
// its cell in the map's bomb mirror while the fuse burns, then explodes in a
// cross of flames that reaches 'range' cells in each direction.
// Scene owns the bombs and deletes them once they reach DONE.

class Bomb
{
public:
	enum BombState { FUSE, EXPLODING, DONE };

	// Columns of explosions.png, in order
	enum FlamePiece { CENTER, ARM_H, ARM_V, END_UP, END_DOWN, END_LEFT, END_RIGHT, N_PIECES };

	struct Flame
	{
		glm::ivec2 tile;
		FlamePiece piece;
	};

public:
	Bomb();
	~Bomb();

public:
	void init(const glm::ivec2 &tile, int range, const glm::ivec2 &tileMapPos, TileMap *tileMap,
	          Texture *bombSheet, Texture *flameSheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	void explode();

	glm::ivec2 getTile() const { return tile; }
	glm::vec2 getPosition() const { return tilePosition(tile); }
	glm::ivec2 getSize() const { return glm::ivec2(map->getTileSize()); }
	BombState getState() const { return state; }
	bool isDone() const { return state == DONE; }

	const std::vector<Flame> &getFlames() const { return flames; }
	bool flameCovers(const glm::ivec2 &cell) const;

private:
	void spreadFlames();
	glm::vec2 tilePosition(const glm::ivec2 &cell) const { return glm::vec2(tileMapDispl + cell * map->getTileSize()); }

private:
	glm::ivec2 tileMapDispl, tile;
	int range;
	BombState state;
	int timer;
	Sprite *sprite, *flameSprite;
	TileMap *map;
	std::vector<Flame> flames;

};


#endif // _BOMB_INCLUDE
