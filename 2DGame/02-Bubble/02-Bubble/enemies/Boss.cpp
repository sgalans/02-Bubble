#include <cmath>
#include <GL/glew.h>
#include "Boss.h"


// Tuning
#define BOSS_LIVES 8
#define REAR_FIRE_HITS 4         // after this many hits it also fires backwards
#define BOSS_FLIGHT_SPEED 31.5f  // pixels per second
#define FLIGHT_CORNERS 4
#define FLIGHT_MARGIN 4.f        // corner marker cells from each map edge
#define ATTACK_COOLDOWN 3500     // ms of flying between two attacks
#define FIRST_ATTACK_DELAY 2000  // ms before the first attack
#define WINDUP_TIME 700          // ms holding the attack pose before firing
#define RECOVER_TIME 500         // ms standing still after firing
#define HURT_TIME 600            // ms flinching after an explosion hit
#define ATTACK_RANGE_TILES 12    // only attacks when the player is this close
#define SHOT_SPEED 60.f          // ball speed, in pixels per second
#define SHOT_COUNT 3             // balls per attack, at most Boss::MAX_SHOTS
#define SHOT_SPREAD 0.35f        // radians between two neighbouring balls

#define BOSS_WIDTH 32
#define BOSS_HEIGHT 32

#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 32.f
#define FRAME_HEIGHT 32.f
#define ROW_Y 453.f
#define IDLE_A_X 2.f
#define IDLE_B_X 36.f
#define ATTACK_X 70.f
#define HURT_X 104.f
#define BOSS_DEATH_TIME 4000 // ms before the exit may open
#define DEATH_BLINK_START 2000 // ms
#define PARTICLE_SIZE 8
#define PARTICLE_COUNT 6


enum BossAnims
{
	IDLE, ATTACK, HURT_FACE
};


Boss::Boss()
{
	player = NULL;
	state = FLY;
	stateTimer = 0;
	attackTimer = FIRST_ATTACK_DELAY;
	flightStarted = false;
	flightTarget = 0;
	nPendingShots = 0;
	livesLastFrame = BOSS_LIVES;
	deathElapsed = 0;
	particleSprite = NULL;
}

Boss::~Boss()
{
	delete particleSprite;
}

void Boss::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	// Frames sit 2 px apart on the sheet, so filtering would bleed neighbours in.
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	sprite = Sprite::createSprite(glm::ivec2(BOSS_WIDTH, BOSS_HEIGHT),
	                              glm::vec2(FRAME_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(3);

		sprite->setAnimationSpeed(IDLE, 3);
		sprite->addKeyframe(IDLE, glm::vec2(IDLE_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(IDLE, glm::vec2(IDLE_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(ATTACK, 1);
		sprite->addKeyframe(ATTACK, glm::vec2(ATTACK_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(HURT_FACE, 1);
		sprite->addKeyframe(HURT_FACE, glm::vec2(HURT_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(IDLE);
	tileMapDispl = tileMapPos;
	sizeEnemy = glm::ivec2(BOSS_WIDTH, BOSS_HEIGHT);
	lives = BOSS_LIVES;
	livesLastFrame = lives;
	state = FLY;
	stateTimer = 0;
	attackTimer = FIRST_ATTACK_DELAY;
	flightStarted = false;
	flightTarget = 0;
	nPendingShots = 0;
	updateSpritePosition();
	deathElapsed = 0;
	particleSprite = Sprite::createSprite(glm::vec2(PARTICLE_SIZE),
	                                     glm::vec2(float(PARTICLE_SIZE) / spritesheet.width(), float(PARTICLE_SIZE) / spritesheet.height()),
	                                     &spritesheet, &shaderProgram);
	particleSprite->setNumberAnimations(3);
	for(int frame = 0; frame < 3; frame++)
	{
		particleSprite->setAnimationSpeed(frame, 1);
		particleSprite->addKeyframe(frame, glm::vec2((223.f + 10.f * frame) / spritesheet.width(), 540.f / spritesheet.height()));
	}
}

void Boss::setPlayer(const Player *target)
{
	player = target;
}

void Boss::update(int deltaTime)
{
	if(isDead())
	{
		if(state != DYING)
			setState(DYING);
		deathElapsed += glm::clamp(deltaTime, 0, BOSS_DEATH_TIME - deathElapsed);
		return;
	}
	sprite->update(deltaTime);

	// Losing a life since the last frame means an explosion just hit us.
	// Flinching cancels a wind-up in progress.
	if(lives < livesLastFrame)
		setState(HURT);
	livesLastFrame = lives;

	switch(state)
	{
	case FLY:
		fly(deltaTime);
		attackTimer -= deltaTime;
		if(attackTimer <= 0 && player != NULL && player->isAlive())
		{
			glm::vec2 center = getPosition() + glm::vec2(sizeEnemy) / 2.f;
			glm::vec2 target = player->getPosition() + glm::vec2(player->getSize()) / 2.f;
			if(glm::length(target - center) <= ATTACK_RANGE_TILES * map->getTileSize())
				setState(WINDUP);
		}
		break;
	case WINDUP:
		stateTimer -= deltaTime;
		if(stateTimer <= 0)
		{
			fireSpread();
			setState(RECOVER);
		}
		break;
	case RECOVER:
	case HURT:
		stateTimer -= deltaTime;
		if(stateTimer <= 0)
			setState(FLY);
		break;
	}

	updateSpritePosition();
}

void Boss::setState(State newState)
{
	state = newState;
	switch(state)
	{
	case FLY:
		attackTimer = ATTACK_COOLDOWN;
		sprite->changeAnimation(IDLE);
		break;
	case WINDUP:
		stateTimer = WINDUP_TIME;
		sprite->changeAnimation(ATTACK);
		break;
	case RECOVER:
		stateTimer = RECOVER_TIME;
		sprite->changeAnimation(IDLE);
		break;
	case HURT:
		stateTimer = HURT_TIME;
		sprite->changeAnimation(HURT_FACE);
		break;
	case DYING:
		deathElapsed = 0;
		nPendingShots = 0;
		sprite->changeAnimation(HURT_FACE);
		break;
	}
}

void Boss::takeHit()
{
	Enemy::takeHit();
	if(isDead() && state != DYING)
		setState(DYING);
}

void Boss::kill()
{
	Enemy::kill();
	if(state != DYING)
		setState(DYING);
}

bool Boss::isReadyToRemove() const
{
	return isDead() && deathElapsed >= BOSS_DEATH_TIME;
}

void Boss::render()
{
	if(!isDead())
	{
		Enemy::render();
		return;
	}
	if(isReadyToRemove())
		return;
	if(deathElapsed < DEATH_BLINK_START || Player::isBlinkVisible(deathElapsed - DEATH_BLINK_START))
		sprite->render();
	// Stagger short three-frame bursts; their full 8x8 bounds stay inside the body.
	for(int i = 0; i < PARTICLE_COUNT; i++)
	{
		int pulse = (deathElapsed + i * 83) / 100;
		int frame = pulse % 5;
		if(frame >= 3)
			continue;
		unsigned int seed = (static_cast<unsigned int>(i) + 1u) * 73856093u
		                    ^ static_cast<unsigned int>(pulse / 5) * 19349663u;
		glm::vec2 offset(float(seed % (BOSS_WIDTH - PARTICLE_SIZE + 1)),
		                 float((seed / 25) % (BOSS_HEIGHT - PARTICLE_SIZE + 1)));
		particleSprite->changeAnimation(frame);
		particleSprite->setPosition(getPosition() + offset);
		particleSprite->render();
	}
}

// Centre the 2x2 body on symmetric corner cells: TL -> BR -> TR -> BL.
glm::vec2 Boss::flightPoint(int index) const
{
	glm::vec2 halfBody = glm::vec2(sizeEnemy) / 2.f;
	glm::vec2 nearCorner = glm::vec2(FLIGHT_MARGIN + 0.5f) * float(map->getTileSize()) - halfBody;
	glm::vec2 farCorner = (glm::vec2(map->getMapSize()) - glm::vec2(FLIGHT_MARGIN + 0.5f))
	                     * float(map->getTileSize()) - halfBody;
	switch(index % FLIGHT_CORNERS)
	{
	case 0: return nearCorner;
	case 1: return farCorner;
	case 2: return glm::vec2(farCorner.x, nearCorner.y);
	default: return glm::vec2(nearCorner.x, farCorner.y);
	}
}

bool Boss::touchesBomb(const glm::vec2 &pos) const
{
	int left = int(floor(pos.x / map->getTileSize()));
	int right = int(floor((pos.x + sizeEnemy.x - 0.001f) / map->getTileSize()));
	int top = int(floor(pos.y / map->getTileSize()));
	int bottom = int(floor((pos.y + sizeEnemy.y - 0.001f) / map->getTileSize()));
	for(int y = top; y <= bottom; y++)
	{
		for(int x = left; x <= right; x++)
		{
			if(map->hasBomb(x, y))
				return true;
		}
	}
	return false;
}

void Boss::fly(int deltaTime)
{
	if(!flightStarted)
	{
		flightPosition = glm::vec2(posEnemy);
		float nearest = -1.f;
		for(int i = 0; i < FLIGHT_CORNERS; i++)
		{
			float distance = glm::length(flightPoint(i) - flightPosition);
			if(nearest < 0.f || distance < nearest)
			{
				nearest = distance;
				flightTarget = i;
			}
		}
		flightStarted = true;
	}
	float remaining = BOSS_FLIGHT_SPEED * glm::max(deltaTime, 0) / 1000.f;
	while(remaining > 0.f)
	{
		glm::vec2 target = flightPoint(flightTarget);
		glm::vec2 delta = target - flightPosition;
		float distance = glm::length(delta);
		if(distance < 0.001f)
		{
			flightTarget = (flightTarget + 1) % FLIGHT_CORNERS;
			continue;
		}
		// Sweep the whole body in subpixel steps, including fresh passable bombs.
		float step = glm::min(remaining, glm::min(distance, 0.5f));
		glm::vec2 next = flightPosition + delta * (step / distance);
		if(touchesBomb(next))
			break;
		flightPosition = next;
		remaining -= step;
	}
	posEnemy = glm::ivec2(glm::floor(flightPosition));
}

// Queues a fan of balls centred on the player's current position. Scene
// collects them one by one through takeShot.
void Boss::fireSpread()
{
	nPendingShots = 0;
	if(player == NULL || !player->isAlive())
		return;

	glm::vec2 center = getPosition() + glm::vec2(sizeEnemy) / 2.f;
	glm::vec2 target = player->getPosition() + glm::vec2(player->getSize()) / 2.f;
	glm::vec2 toTarget = target - center;
	if(glm::length(toTarget) == 0.f)
		return;

	float baseAngle = atan2(toTarget.y, toTarget.x);
	// Once badly hurt, a mirrored fan also flies away from the player.
	int fans = (BOSS_LIVES - lives >= REAR_FIRE_HITS) ? 2 : 1;
	int count = glm::min(SHOT_COUNT, MAX_SHOTS / fans);
	for(int fan = 0; fan < fans; fan++)
	{
		float fanAngle = baseAngle + fan * 3.14159265f;
		for(int i = 0; i < count; i++)
		{
			float angle = fanAngle + (i - (count - 1) / 2.f) * SHOT_SPREAD;
			shotOrigins[nPendingShots] = center;
			shotVelocities[nPendingShots] = glm::vec2(cos(angle), sin(angle)) * SHOT_SPEED;
			nPendingShots++;
		}
	}
}

// Hands one pending ball to the scene per call: where it starts (centre,
// screen coordinates) and its velocity in pixels per second.
bool Boss::takeShot(glm::vec2 &origin, glm::vec2 &velocity)
{
	if(nPendingShots == 0)
		return false;
	nPendingShots--;
	origin = shotOrigins[nPendingShots];
	velocity = shotVelocities[nPendingShots];

	return true;
}
