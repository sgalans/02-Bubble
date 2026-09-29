#ifndef _WALKER_INCLUDE
#define _WALKER_INCLUDE


#include "Enemy.h"


// Enemy that walks along the floor and turns around when it hits a wall.

class Walker : public Enemy
{
public:
	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);

};


#endif // _WALKER_INCLUDE
