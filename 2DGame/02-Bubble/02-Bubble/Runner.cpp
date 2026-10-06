#include <GL/glew.h>
#include "Runner.h"


#define WALKING_SPEED 1.0f
#define RUNNER_SPEED 2.0f
#define FALL_STEP 4

// How far ahead the runner can spot the player, in tiles.
#define SIGHT_RANGE_TILES 8

#define RUNNER_WIDTH 30
#define RUNNER_HEIGHT 32


#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 15.f
#define FRAME_HEIGHT 16.f

#define ROW_Y 171.f
#define WALK_A_X 2.f
#define WALK_B_X 19.f
#define WALK_C_X 36.f


enum RunnerAnims
{
	MOVE_LEFT, MOVE_RIGHT
};


Runner::Runner()
{
	direction = 0;
	chasing = false;
	player = NULL;
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

void Runner::update(int deltaTime)
{
	sprite->update(deltaTime);

	// Spotting the player starts a charge; losing sight of it mid-charge (it
	// jumped over us, or got out of reach) makes the runner turn around.
	if(seesPlayer())
		chasing = true;
	else if(chasing)
	{
		chasing = false;
		setDirection(1 - direction);
	}

	int speed = int(chasing ? RUNNER_SPEED : WALKING_SPEED);
	if(direction == 0)
	{
		posEnemy.x -= speed;
		if(map->collisionMoveLeft(posEnemy, sizeEnemy))
		{
			posEnemy.x += speed;
			chasing = false;
			setDirection(1);
		}
	}
	else
	{
		posEnemy.x += speed;
		if(map->collisionMoveRight(posEnemy, sizeEnemy))
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

// The runner only looks ahead: the player has to be in front of it, at about
// the same height and within range. Walls do not block its view.
bool Runner::seesPlayer() const
{
	if(player == NULL || !player->isAlive())
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
	return gap >= -sizeEnemy.x && gap <= SIGHT_RANGE_TILES * map->getTileSize();
}


void Runner::setDirection(int newDirection)
{
	if(newDirection == direction)
		return;
	direction = newDirection;
	sprite->changeAnimation(direction == 0 ? MOVE_LEFT : MOVE_RIGHT);
	sprite->setFlipX(direction == 1);
}
