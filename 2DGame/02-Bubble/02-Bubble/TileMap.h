#ifndef _TILE_MAP_INCLUDE
#define _TILE_MAP_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "ShaderProgram.h"


// Class Tilemap is capable of loading a tile map from a text file in a very
// simple format (see level01.txt for an example). With this information
// it builds a single VBO that contains all tiles. As a result the render
// method draws the whole map independently of what is visible.


class TileMap
{

public:
	// Values stored in the bomb mirror map. A freshly placed bomb is passable
	// so the player is not trapped inside it; it turns solid once he leaves it.
	enum BombCell { NO_BOMB = 0, BOMB_SOLID = 1, BOMB_PASSABLE = 2 };

private:
	TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

public:
	// Tile maps can only be created inside an OpenGL context
	static TileMap *createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program);

	~TileMap();

	void render() const;
	void free();
	
	int getTileSize() const { return tileSize; }
	int getBlockSize() const { return blockSize; }
	glm::ivec2 getPosition() const { return position; }
	glm::ivec2 getMapSize() const { return mapSize; }

	bool collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const;
	bool collisionMoveDown(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;
	bool collisionMoveUp(const glm::ivec2 &pos, const glm::ivec2 &size, int *posY) const;

	// Bomb mirror map, in tile coordinates
	bool setBomb(int x, int y);
	void clearBomb(int x, int y);
	bool hasBomb(int x, int y) const;
	void solidifyBombs(const glm::ivec2 &pos, const glm::ivec2 &size);

private:
	bool loadLevel(const string &levelFile);
	void prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program);
	bool insideMap(int x, int y) const;
	bool isSolid(int x, int y) const;

private:
	GLuint vao;
	GLuint vbo;
	GLint posLocation, texCoordLocation;
	int nTiles;
	glm::ivec2 position, mapSize, tilesheetSize;
	int tileSize, blockSize;
	Texture tilesheet;
	glm::vec2 tileTexSize;
	int *map;
	int *bombMap;

};


#endif // _TILE_MAP_INCLUDE


