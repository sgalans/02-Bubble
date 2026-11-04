#ifndef _RUNNER_INCLUDE
#define _RUNNER_INCLUDE


#include "Enemy.h"
#include "../Player.h"

// Enemy that charges at the player and turns at walls and platform edges.

class Runner : public Enemy
{
public:
	Runner();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);

	void setPlayer(const Player *target);

private:
	bool hasFloorBelow(int x, int width) const;
	bool seesPlayer() const;
	void setDirection(int newDirection);

private:
    int direction; // 0 = left, 1 = right
	bool chasing;
	const Player *player;
};

#endif 
