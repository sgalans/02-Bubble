#ifndef _SHOOTER_INCLUDE
#define _SHOOTER_INCLUDE


#include "Enemy.h"
#include "Player.h"


// Enemy that stays where it is and, while the player is in range and no wall
// stands between them, every so
// often fires a slow ball straight at where the player is at that moment.
// The shooter only asks for the shot; Scene creates and owns the ball.

class Shooter : public Enemy
{
public:
	Shooter();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);

	void setPlayer(const Player *target);
	bool takeShot(glm::vec2 &origin, glm::vec2 &velocity);

private:
	bool playerInRange() const;
	bool clearLineTo(const glm::vec2 &target) const;

private:
	const Player *player;
	int shotTimer;      // ms until the next shot is allowed
	int shootPoseTimer; // ms left showing the shooting frame
	bool shotRequested;
	glm::vec2 shotOrigin, shotVelocity;
};


#endif // _SHOOTER_INCLUDE
