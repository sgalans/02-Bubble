#include <GL/glew.h>
#include "Walker.h"


#define WALKER_SPEED 0.3f
#define FALL_STEP 2

// Draw and collide at the source art's native size.
#define WALKER_WIDTH 13
#define WALKER_HEIGHT 16

// bomberman.png is a ragged rip, so frames are addressed by pixel offset
// instead of a uniform grid. The walker's row spans y = 153..168.
#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 13.f
#define FRAME_HEIGHT 16.f
#define ROW_Y 153.f
#define FRONT_A_X 2.f
#define FRONT_B_X 17.f
#define DEATH_WIDTH 15.f
#define DEATH_TIME 600


enum WalkerAnims
{
	MOVE_LEFT, MOVE_RIGHT
};


Walker::Walker() : deathSprite(NULL), deathElapsed(0)
{
}

Walker::~Walker()
{
	delete deathSprite;
}

void Walker::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	// Frames sit 2-3 px apart on the sheet, so filtering would bleed neighbours in.
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	sprite = Sprite::createSprite(glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT),
	                              glm::vec2(FRAME_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

		// Both directions use the front view, which reads either way.
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(FRONT_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(FRONT_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(FRONT_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(FRONT_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	// Death frames are wider than the walking frames; keep the collision size.
	deathSprite = Sprite::createSprite(glm::vec2(DEATH_WIDTH, FRAME_HEIGHT),
	                                  glm::vec2(DEATH_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                                  &spritesheet, &shaderProgram);
	deathSprite->setNumberAnimations(1);
	deathSprite->setAnimationSpeed(0, 5);
	deathSprite->addKeyframe(0, glm::vec2(75.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->addKeyframe(0, glm::vec2(92.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->addKeyframe(0, glm::vec2(109.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->changeAnimation(0);
	deathElapsed = 0;

	sprite->changeAnimation(MOVE_LEFT);
	tileMapDispl = tileMapPos;
	sizeEnemy = glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT);
	lives = 1;
	updateSpritePosition();
}

// True when the tile row right under the walker's feet is solid below the
// given column span. Probes with a copy of the position so nothing moves.
bool Walker::hasFloorBelow(int x, int width) const
{
	glm::ivec2 probePos(x, posEnemy.y + 1);
	int probeY = probePos.y;
	return map->collisionMoveDown(probePos, glm::ivec2(width, sizeEnemy.y), &probeY);
}

void Walker::update(int deltaTime)
{
	if(isDead())
	{
		int elapsed = glm::clamp(deltaTime, 0, DEATH_TIME - deathElapsed);
		deathElapsed += elapsed;
		deathSprite->update(elapsed);
		return;
	}

	sprite->update(deltaTime);

	// Only guard edges while standing, so a walker spawned mid-air still drops.
	bool onGround = hasFloorBelow(posEnemy.x, sizeEnemy.x);
	horizontalRemainder += WALKER_SPEED;
	int steps = int(horizontalRemainder);
	horizontalRemainder -= steps;

	if(sprite->animation() == MOVE_LEFT)
	{
		posEnemy.x -= steps;
		if(map->collisionMoveLeft(posEnemy, sizeEnemy) || (onGround && !hasFloorBelow(posEnemy.x, 1)))
		{
			posEnemy.x += steps;
			sprite->changeAnimation(MOVE_RIGHT);
		}
	}
	else
	{
		posEnemy.x += steps;
		if(map->collisionMoveRight(posEnemy, sizeEnemy) || (onGround && !hasFloorBelow(posEnemy.x + sizeEnemy.x - 1, 1)))
		{
			posEnemy.x -= steps;
			sprite->changeAnimation(MOVE_LEFT);
		}
	}

	posEnemy.y += FALL_STEP;
	map->collisionMoveDown(posEnemy, sizeEnemy, &posEnemy.y);

	updateSpritePosition();
}

void Walker::startDeath()
{
	deathElapsed = 0;
	deathSprite->changeAnimation(0);
}

void Walker::takeHit()
{
	if(isDead())
		return;
	Enemy::takeHit();
	if(isDead())
		startDeath();
}

void Walker::kill()
{
	if(isDead())
		return;
	Enemy::kill();
	startDeath();
}

bool Walker::isReadyToRemove() const
{
	return isDead() && deathElapsed >= DEATH_TIME;
}

void Walker::render()
{
	if(!isDead())
		Enemy::render();
	else if(!isReadyToRemove())
	{
		deathSprite->setPosition(getPosition() + glm::vec2((WALKER_WIDTH - DEATH_WIDTH) / 2.f, 0.f));
		deathSprite->render();
	}
}
