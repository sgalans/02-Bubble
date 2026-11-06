#ifndef _BOSS_INCLUDE
#define _BOSS_INCLUDE


#include "Enemy.h"
#include "../Player.h"


// Flying 2x2 tree boss. It crosses diagonally between symmetric corner cells,
// stopping at bombs and while attacking or hurt. Scene owns its projectiles.

class Boss : public Enemy
{
public:
	Boss();
	~Boss();

	void init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram);
	void update(int deltaTime);
	void render();
	void takeHit();
	void kill();
	bool isReadyToRemove() const;
	bool isFlying() const { return true; }

	void setPlayer(const Player *target);
	bool takeShot(glm::vec2 &origin, glm::vec2 &velocity);
	int shotBounces() const { return 4; }

private:
	enum State { FLY, WINDUP, RECOVER, HURT, DYING };

	void fly(int deltaTime);
	glm::vec2 flightPoint(int index) const;
	bool touchesBomb(const glm::vec2 &pos) const;
	void fireSpread();
	void setState(State newState);

private:
	static const int MAX_SHOTS = 10;

	const Player *player;
	State state;
	int stateTimer;  // ms left in the current state (WINDUP, RECOVER, HURT)
	int attackTimer; // ms until the next attack while flying
	bool flightStarted;
	int flightTarget;
	glm::vec2 flightPosition;
	int livesLastFrame;
	glm::vec2 shotOrigins[MAX_SHOTS], shotVelocities[MAX_SHOTS];
	int nPendingShots;
	int deathElapsed;
	Sprite *particleSprite;
};


#endif // _BOSS_INCLUDE
