#include <GL/glew.h>
#include "Runner.h"


#define WALKING_SPEED 0.45f
#define RUNNER_SPEED 0.9f
#define FALL_STEP 3

// How far ahead the runner can spot the player, in tiles.
#define SIGHT_RANGE_TILES 8

#define RUNNER_WIDTH 15
#define RUNNER_HEIGHT 16


#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 15.f
#define FRAME_HEIGHT 16.f

#define ROW_Y 171.f
#define WALK_A_X 2.f
#define WALK_B_X 19.f
#define WALK_C_X 36.f
#define DEATH_WIDTH 16.f
#define DEATH_TIME 800


enum RunnerAnims
{
	MOVE_LEFT, MOVE_RIGHT
};


Runner::Runner()
{
	direction = 0;
	chasing = false;
	player = NULL;
	deathSprite = NULL;
	deathElapsed = 0;
}

Runner::~Runner()
{
	delete deathSprite;
}

void Runner::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	// Frames sit 2-3 px apart on the sheet, so filtering would bleed neighbours in.
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	sprite = Sprite::createSprite(glm::ivec2(RUNNER_WIDTH, RUNNER_HEIGHT),
	                              glm::vec2(FRAME_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

		// Both directions share the left-facing frames; MOVE_RIGHT is drawn mirrored.
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(WALK_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(WALK_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(WALK_C_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(WALK_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(WALK_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(WALK_C_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	// The breakup frames span 16 pixels; keep the walking collision size.
	deathSprite = Sprite::createSprite(glm::vec2(DEATH_WIDTH, FRAME_HEIGHT),
	                                  glm::vec2(DEATH_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                                  &spritesheet, &shaderProgram);
	deathSprite->setNumberAnimations(1);
	deathSprite->setAnimationSpeed(0, 5);
	deathSprite->addKeyframe(0, glm::vec2(53.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->addKeyframe(0, glm::vec2(69.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->addKeyframe(0, glm::vec2(86.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->addKeyframe(0, glm::vec2(104.f / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
	deathSprite->changeAnimation(0);
	deathElapsed = 0;

	sprite->changeAnimation(MOVE_LEFT);
	sprite->setFlipX(false);
	tileMapDispl = tileMapPos;
	sizeEnemy = glm::ivec2(RUNNER_WIDTH, RUNNER_HEIGHT);
	lives = 1;
    direction = 0; // Start moving left
	chasing = false;
	updateSpritePosition();
}

void Runner::setPlayer(const Player *target)
{
	player = target;
}

bool Runner::hasFloorBelow(int x, int width) const
{
	glm::ivec2 probePos(x, posEnemy.y + 1);
	int probeY = probePos.y;
	return map->collisionMoveDown(probePos, glm::ivec2(width, sizeEnemy.y), &probeY);
}

void Runner::update(int deltaTime)
{
	if(isDead())
	{
		int elapsed = glm::clamp(deltaTime, 0, DEATH_TIME - deathElapsed);
		deathElapsed += elapsed;
		deathSprite->update(elapsed);
		return;
	}

	sprite->update(deltaTime);
	// Only guard edges while standing, so airborne runners can still land.
	bool onGround = hasFloorBelow(posEnemy.x, sizeEnemy.x);

	// Losing sight ends the charge; walls and ledges still control direction.
	chasing = seesPlayer();

	horizontalRemainder += chasing ? RUNNER_SPEED : WALKING_SPEED;
	int speed = int(horizontalRemainder);
	horizontalRemainder -= speed;
	if(direction == 0)
	{
		posEnemy.x -= speed;
		if(map->collisionMoveLeft(posEnemy, sizeEnemy) || (onGround && !hasFloorBelow(posEnemy.x, 1)))
		{
			posEnemy.x += speed;
			chasing = false;
			setDirection(1);
		}
	}
	else
	{
		posEnemy.x += speed;
		if(map->collisionMoveRight(posEnemy, sizeEnemy) || (onGround && !hasFloorBelow(posEnemy.x + sizeEnemy.x - 1, 1)))
		{
			posEnemy.x -= speed;
			chasing = false;
			setDirection(0);
		}
	}

	posEnemy.y += FALL_STEP;
	map->collisionMoveDown(posEnemy, sizeEnemy, &posEnemy.y);

	updateSpritePosition();
}

void Runner::startDeath()
{
	chasing = false;
	deathElapsed = 0;
	deathSprite->setFlipX(direction == 1);
	deathSprite->changeAnimation(0);
}

void Runner::takeHit()
{
	if(isDead())
		return;
	Enemy::takeHit();
	if(isDead())
		startDeath();
}

void Runner::kill()
{
	if(isDead())
		return;
	Enemy::kill();
	startDeath();
}

bool Runner::isReadyToRemove() const
{
	return isDead() && deathElapsed >= DEATH_TIME;
}

void Runner::render()
{
	if(!isDead())
		Enemy::render();
	else if(!isReadyToRemove())
	{
		deathSprite->setPosition(getPosition() + glm::vec2((RUNNER_WIDTH - DEATH_WIDTH) / 2.f, 0.f));
		deathSprite->render();
	}
}

// The runner only looks ahead: the player has to be in front of it, at about
// the same height and within range, with no map blocks between them.
bool Runner::seesPlayer() const
{
	if(player == NULL || !player->isAlive() || player->isInvulnerable())
		return false;

	// Work in map coordinates, the same space as posEnemy.
	glm::ivec2 playerPos = glm::ivec2(player->getPosition()) - tileMapDispl;
	glm::ivec2 playerSize = player->getSize();

	bool sameHeight = playerPos.y < posEnemy.y + sizeEnemy.y && posEnemy.y < playerPos.y + playerSize.y;
	if(!sameHeight)
		return false;

	int gap;
	if(direction == 0)
		gap = posEnemy.x - (playerPos.x + playerSize.x);
	else
		gap = playerPos.x - (posEnemy.x + sizeEnemy.x);
	// A negative gap down to -size means the boxes overlap; further than that
	// and the player is behind us.
	if(gap < -sizeEnemy.x || gap > SIGHT_RANGE_TILES * map->getTileSize())
		return false;

	// Cast horizontally through the height shared by both hitboxes.
	int top = glm::max(posEnemy.y, playerPos.y);
	int bottom = glm::min(posEnemy.y + sizeEnemy.y, playerPos.y + playerSize.y);
	int row = (top + (bottom - top) / 2) / map->getTileSize();
	int from = (posEnemy.x + sizeEnemy.x / 2) / map->getTileSize();
	int to = (playerPos.x + playerSize.x / 2) / map->getTileSize();
	for(int x = glm::min(from, to); x <= glm::max(from, to); x++)
	{
		if(map->isWall(x, row))
			return false;
	}
	return true;
}


void Runner::setDirection(int newDirection)
{
	if(newDirection == direction)
		return;
	direction = newDirection;
	sprite->changeAnimation(direction == 0 ? MOVE_LEFT : MOVE_RIGHT);
	sprite->setFlipX(direction == 1);
}
