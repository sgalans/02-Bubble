#include <GL/glew.h>
#include "Shooter.h"


// Tuning. The ball is meant to be easy to dodge, so keep SHOT_SPEED low.
#define SHOT_SPEED 50.f        // ball speed, in pixels per second
#define SHOT_COOLDOWN 2500     // ms between two shots
#define FIRST_SHOT_DELAY 1000  // ms before the first shot once the level starts
#define SHOT_RANGE_TILES 10    // only shoots when the player is this close
#define SHOOT_POSE_TIME 300    // ms the shooting frame stays on screen

#define FALL_STEP 4

#define SHOOTER_WIDTH 16
#define SHOOTER_HEIGHT 16

// Green blob row on bomberman.png, right below the runner's row. The first
// two frames are its idle bob; the third is the open-eyed pose used to shoot.
#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 16.f
#define FRAME_HEIGHT 16.f
#define ROW_Y 189.f
#define IDLE_A_X 2.f
#define IDLE_B_X 20.f
#define SHOOT_X 38.f


enum ShooterAnims
{
	IDLE, SHOOT
};


Shooter::Shooter()
{
	player = NULL;
	shotTimer = FIRST_SHOT_DELAY;
	shootPoseTimer = 0;
	shotRequested = false;
}

void Shooter::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);

	sprite = Sprite::createSprite(glm::ivec2(SHOOTER_WIDTH, SHOOTER_HEIGHT),
	                              glm::vec2(FRAME_WIDTH / SHEET_WIDTH, FRAME_HEIGHT / SHEET_HEIGHT),
	                              &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);

		sprite->setAnimationSpeed(IDLE, 4);
		sprite->addKeyframe(IDLE, glm::vec2(IDLE_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(IDLE, glm::vec2(IDLE_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(SHOOT, 1);
		sprite->addKeyframe(SHOOT, glm::vec2(SHOOT_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(IDLE);
	tileMapDispl = tileMapPos;
	sizeEnemy = glm::ivec2(SHOOTER_WIDTH, SHOOTER_HEIGHT);
	lives = 1;
	shotTimer = FIRST_SHOT_DELAY;
	shootPoseTimer = 0;
	shotRequested = false;
	updateSpritePosition();
}

void Shooter::setPlayer(const Player *target)
{
	player = target;
}

void Shooter::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(shootPoseTimer > 0)
	{
		shootPoseTimer -= deltaTime;
		if(shootPoseTimer <= 0)
			sprite->changeAnimation(IDLE);
	}

	if(shotTimer > 0)
		shotTimer -= deltaTime;
	else if(playerInRange())
	{
		// Aim from our centre at the player's centre as it is right now. The
		// ball does not follow the player afterwards.
		glm::vec2 center = getPosition() + glm::vec2(sizeEnemy) / 2.f;
		glm::vec2 target = player->getPosition() + glm::vec2(player->getSize()) / 2.f;
		glm::vec2 toTarget = target - center;
		if(glm::length(toTarget) > 0.f)
		{
			shotOrigin = center;
			shotVelocity = glm::normalize(toTarget) * SHOT_SPEED;
			shotRequested = true;
			shotTimer = SHOT_COOLDOWN;
			shootPoseTimer = SHOOT_POSE_TIME;
			sprite->changeAnimation(SHOOT);
		}
	}

	posEnemy.y += FALL_STEP;
	map->collisionMoveDown(posEnemy, sizeEnemy, &posEnemy.y);

	updateSpritePosition();
}

// Hands a pending shot to the scene: where the ball starts (centre, screen
// coordinates) and its velocity in pixels per second.
bool Shooter::takeShot(glm::vec2 &origin, glm::vec2 &velocity)
{
	if(!shotRequested)
		return false;
	shotRequested = false;
	origin = shotOrigin;
	velocity = shotVelocity;

	return true;
}

bool Shooter::playerInRange() const
{
	if(player == NULL || !player->isAlive())
		return false;

	glm::vec2 center = getPosition() + glm::vec2(sizeEnemy) / 2.f;
	glm::vec2 target = player->getPosition() + glm::vec2(player->getSize()) / 2.f;

	if(glm::length(target - center) > SHOT_RANGE_TILES * map->getTileSize())
		return false;

	return clearLineTo(target - glm::vec2(tileMapDispl));
}

// Walks the segment from our centre to target (map coordinates) in small
// steps and fails if any of them lands on a wall tile. Bombs do not block.
bool Shooter::clearLineTo(const glm::vec2 &target) const
{
	glm::vec2 from = glm::vec2(posEnemy) + glm::vec2(sizeEnemy) / 2.f;
	glm::vec2 delta = target - from;
	float step = map->getTileSize() / 4.f;
	int nSteps = int(glm::length(delta) / step) + 1;
	for(int i = 1; i <= nSteps; i++)
	{
		glm::vec2 point = from + delta * (float(i) / nSteps);
		glm::ivec2 tile = glm::ivec2(glm::floor(point / float(map->getTileSize())));
		if(map->isWall(tile.x, tile.y))
			return false;
	}

	return true;
}
