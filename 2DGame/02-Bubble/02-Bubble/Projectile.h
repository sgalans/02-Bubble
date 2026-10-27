#ifndef _PROJECTILE_INCLUDE
#define _PROJECTILE_INCLUDE


#include "Sprite.h"
#include "TileMap.h"


// A small ball fired by a Shooter. It flies in a straight line, ignoring
// gravity, until it hits a wall, a bomb or the player.
// Scene owns the projectiles and shares one sprite sheet between all of them.

class Projectile
{
public:
	Projectile();
	~Projectile();

public:
	// center is in screen coordinates, velocity in pixels per second
	void init(const glm::vec2 &center, const glm::vec2 &velocity, const glm::ivec2 &tileMapPos,
	          TileMap *tileMap, Texture *sheet, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();

	glm::vec2 getPosition() const { return glm::vec2(tileMapDispl) + pos; }
	glm::ivec2 getSize() const { return size; }
	void destroy() { done = true; }
	bool isDone() const { return done; }

private:
	glm::ivec2 tileMapDispl, size;
	glm::vec2 pos, velocity; // pos is the top-left corner in map coordinates
	bool done;
	Sprite *sprite;
	TileMap *map;

};


#endif // _PROJECTILE_INCLUDE
