#ifndef _BOSS_INCLUDE
#define _BOSS_INCLUDE


#include "Enemy.h"
#include "../Player.h"


// Big tree enemy, two tiles wide and two tall. It paces slowly along its
// platform and every few seconds stops, winds up and throws a fan of balls
// at the player. It takes several explosions to bring down and flinches for
// a moment after each one. Scene creates and owns the balls, as for Shooter.

class Boss : public Enemy
{
public:
	Boss();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);

	void setPlayer(const Player *target);
	bool takeShot(glm::vec2 &origin, glm::vec2 &velocity);

private:
	enum State { WALK, WINDUP, RECOVER, HURT };

	bool hasFloorBelow(int x, int width) const;
	void walk();
	void fireSpread();
	void setState(State newState);

private:
	static const int MAX_SHOTS = 5;

	const Player *player;
	State state;
	int stateTimer;  // ms left in the current state (WINDUP, RECOVER, HURT)
	int attackTimer; // ms until the next attack while walking
	int direction;   // -1 = left, 1 = right
	int livesLastFrame;
	glm::vec2 shotOrigins[MAX_SHOTS], shotVelocities[MAX_SHOTS];
	int nPendingShots;
};


#endif // _BOSS_INCLUDE
