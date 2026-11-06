#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"
#include "Audio.h"


#define HIGH_JUMP_HEIGHT_TILES 3
#define TRAMPOLINE_JUMP_HEIGHT_TILES 10
#define SHORT_JUMP_HEIGHT_TILES 2
#define HIGH_JUMP_DURATION 0.8f // seconds, returning to the launch height
#define SHORT_JUMP_DURATION 0.5f
#define MAX_VERTICAL_STEP_TIME (1.f / 120.f)
#define LEDGE_FALL_SPEED_TILES 6.f // initial downward speed when walking off support
#define MAX_FALL_SPEED_TILES 15.f // tiles per second
#define INVULNERABILITY_TIME 3000 // ms after respawning
#define DEATH_TIME 1500 // ms
#define DEATH_FRAMES 4
#define EXIT_TIME 1000 // ms
#define EXIT_ALIGN_TIME 300 // ms
#define BLINK_CYCLE 360 // ms
#define BLINK_VISIBLE_TIME 288 // 80% visible, 20% hidden
#define MAX_BOMBS 5
#define MAX_FIRE_POWER 5 // tiles
#define BASE_MOVE_SPEED 1.5f // pixels per frame
#define SPEED_UP_INCREMENT 0.25f
#define MAX_MOVE_SPEED 2.25f
#define PLAYER_SPRITE_WIDTH 16
#define PLAYER_HEIGHT 16
#define HITBOX_INSET_X 2



enum PlayerAnims
{
	STAND, MOVE, JUMP_UP, JUMP_DOWN, ENTER_EXIT
};


Player::Player()
{
	sprite = NULL;
	deathSprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
	delete deathSprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	verticalPosition = 0.f;
	verticalVelocity = 0.f;
	horizontalRemainder = 0.f;
	moveDirection = 0;
	shortJumpActive = false;
	facingRight = false;
	bJumpKeyPressed = false;
	bGodKeyPressed = false;
	bBombKeyPressed = false;
	bBombRequested = false;
	lives = 3;
	maxLives = 4; // Three normal hearts and one optional white bonus heart
	nBombs = 1;
	firePower = 1; // Fire Up items extend it
	moveSpeed = BASE_MOVE_SPEED;
	bombProof = false;
	invulnTime = 0;
	deathTime = blinkTime = 0;
	exiting = false;
	exitTime = 0;
	invensible = false;
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
	sizePlayer = glm::ivec2(PLAYER_SPRITE_WIDTH - 2 * HITBOX_INSET_X, PLAYER_HEIGHT);
	// Left-facing poses on the ragged sheet, cropped to 16x16 at y = 2.
	glm::vec2 frameSize(16.f / spritesheet.width(), 16.f / spritesheet.height());
	glm::vec2 standFrame(4.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 frameA(19.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 frameB(36.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 jumpUpFrame(86.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 jumpDownFrame(69.f / spritesheet.width(), 2.f / spritesheet.height());
	sprite = Sprite::createSprite(glm::vec2(PLAYER_SPRITE_WIDTH, PLAYER_HEIGHT), frameSize, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(5);
	
		sprite->setAnimationSpeed(STAND, 8);
		sprite->addKeyframe(STAND, standFrame);
		
		sprite->setAnimationSpeed(MOVE, 8);
		sprite->addKeyframe(MOVE, frameA);
		sprite->addKeyframe(MOVE, frameB);
		
		sprite->setAnimationSpeed(JUMP_UP, 8);
		sprite->addKeyframe(JUMP_UP, jumpUpFrame);
		
		sprite->setAnimationSpeed(JUMP_DOWN, 8);
		sprite->addKeyframe(JUMP_DOWN, jumpDownFrame);
		
	sprite->setAnimationSpeed(ENTER_EXIT, 1);
	sprite->addKeyframe(ENTER_EXIT, glm::vec2(19.f / spritesheet.width(), 20.f / spritesheet.height()));
	sprite->changeAnimation(STAND);
	sprite->setFlipX(false);
	// The death sequence on row 2 includes a 17-pixel-wide burst.
	deathSprite = Sprite::createSprite(glm::vec2(17.f, 18.f),
	                                  glm::vec2(17.f / spritesheet.width(), 18.f / spritesheet.height()),
	                                  &spritesheet, &shaderProgram);
	deathSprite->setNumberAnimations(DEATH_FRAMES);
	const float deathFrameX[DEATH_FRAMES] = { 54.f, 71.f, 89.f, 107.f };
	for(int frame = 0; frame < DEATH_FRAMES; frame++)
	{
		deathSprite->setAnimationSpeed(frame, 1);
		deathSprite->addKeyframe(frame, glm::vec2(deathFrameX[frame] / spritesheet.width(), 19.f / spritesheet.height()));
	}
	deathSprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	posPlayer = glm::ivec2(HITBOX_INSET_X, 0);
	spawnPoint = glm::vec2(0.f);
	updateSpritePosition();

}

void Player::update(int deltaTime)
{
	if(exiting)
	{
		exitTime += glm::clamp(deltaTime, 0, EXIT_TIME - exitTime);
		float align = glm::min(float(exitTime) / EXIT_ALIGN_TIME, 1.f);
		posPlayer = glm::ivec2(exitStart + (exitTarget - exitStart) * align);
		if(exitTime < EXIT_ALIGN_TIME)
			sprite->update(deltaTime);
		else
		{
			if(sprite->animation() != ENTER_EXIT)
				sprite->changeAnimation(ENTER_EXIT);
			sprite->setFlipX(false);
			float progress = float(exitTime - EXIT_ALIGN_TIME) / (EXIT_TIME - EXIT_ALIGN_TIME);
			sprite->setScale(1.f - progress * progress);
		}
		updateSpritePosition();
		return;
	}
	bool godKeyPressed = Game::instance().getKey(GLFW_KEY_G);
	if(lives > 0 && godKeyPressed && !bGodKeyPressed)
	{
		invensible = !invensible;
		blinkTime = 0;
	}
	bGodKeyPressed = godKeyPressed;
	if(isDying())
	{
		deathTime = max(0, deathTime - max(0, deltaTime));
		int frame = min((DEATH_TIME - deathTime) * DEATH_FRAMES / DEATH_TIME, DEATH_FRAMES - 1);
		deathSprite->changeAnimation(frame);
		// Held actions must be released before they can fire after respawning.
		bBombKeyPressed = Game::instance().getKey(GLFW_KEY_SPACE);
		bJumpKeyPressed = Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_W)
		                  || Game::instance().getKey(GLFW_KEY_S);
		if(deathTime == 0 && lives > 0)
		{
			setPosition(spawnPoint);
			sprite->changeAnimation(STAND);
			invulnTime = INVULNERABILITY_TIME;
			blinkTime = 0;
		}
		return;
	}
	// Out of lives: stay put and stop animating until the game handles it.
	if(lives <= 0)
		return;
	if(invulnTime > 0)
		invulnTime = max(0, invulnTime - deltaTime);
	blinkTime = (blinkTime + max(0, deltaTime) % BLINK_CYCLE) % BLINK_CYCLE;

	glm::ivec2 previousPos = posPlayer;
	bool highJumpKeyPressed = Game::instance().getKey(GLFW_KEY_UP)
		|| Game::instance().getKey(GLFW_KEY_W);
	bool jumpKeyPressed = highJumpKeyPressed || Game::instance().getKey(GLFW_KEY_S);
	bool grounded = false;
	bool supportedBeforeMove = false;
	if(verticalVelocity == 0.f)
	{
		glm::ivec2 groundProbe = posPlayer + glm::ivec2(0, 1);
		supportedBeforeMove = map->collisionMoveDown(groundProbe, sizePlayer, &groundProbe.y);
	}
	int directionX = 0;
	if(Game::instance().getKey(GLFW_KEY_LEFT) || Game::instance().getKey(GLFW_KEY_A))
		directionX = -1;
	else if(Game::instance().getKey(GLFW_KEY_RIGHT) || Game::instance().getKey(GLFW_KEY_D))
		directionX = 1;
	if(directionX != moveDirection)
	{
		horizontalRemainder = 0.f;
		moveDirection = directionX;
	}
	if(directionX != 0)
	{
		facingRight = directionX > 0;
		horizontalRemainder += moveSpeed;
		int steps = int(horizontalRemainder);
		horizontalRemainder -= steps;
		// One pixel at a time so higher speeds still stop flush against walls.
		for(int i = 0; i < steps; i++)
		{
			posPlayer.x += directionX;
			if(directionX < 0 ? map->collisionMoveLeft(posPlayer, sizePlayer)
			                  : map->collisionMoveRight(posPlayer, sizePlayer))
			{
				posPlayer.x -= directionX;
				horizontalRemainder = 0.f;
				break;
			}
		}
	}
	
	if(verticalVelocity >= 0.f)
	{
		glm::ivec2 groundProbe = posPlayer + glm::ivec2(0, 1);
		grounded = map->collisionMoveDown(groundProbe, sizePlayer, &groundProbe.y);
		if(grounded)
		{
			posPlayer.y = groundProbe.y;
			verticalPosition = float(posPlayer.y);
			verticalVelocity = 0.f;
		}
	}
	bool startJump = grounded && jumpKeyPressed && !bJumpKeyPressed;
	if(grounded)
		map->stepOnPlatforms(posPlayer, sizePlayer);
	if(supportedBeforeMove && !grounded)
	{
		// Skip the slow start from rest when stepping off a platform or bomb.
		verticalVelocity = LEDGE_FALL_SPEED_TILES * map->getTileSize();
		shortJumpActive = false;
	}
	if(grounded)
		shortJumpActive = startJump && !highJumpKeyPressed;

	// For total flight time T and height H: gravity = 8H/T^2, launch speed = -4H/T.
	const float jumpHeight = float((shortJumpActive ? SHORT_JUMP_HEIGHT_TILES : HIGH_JUMP_HEIGHT_TILES) * map->getTileSize());
	const float jumpDuration = shortJumpActive ? SHORT_JUMP_DURATION : HIGH_JUMP_DURATION;
	const float gravity = 8.f * jumpHeight / (jumpDuration * jumpDuration);
	const float maxFallSpeed = MAX_FALL_SPEED_TILES * map->getTileSize();
	if(startJump)
	{
		verticalVelocity = -4.f * jumpHeight / jumpDuration;
		grounded = false;
	}

	// Keep fractional motion and split time to check collisions near the apex too.
	float remainingTime = max(0, deltaTime) / 1000.f;
	while(remainingTime > 0.f && !grounded)
	{
		float stepTime = min(remainingTime, MAX_VERTICAL_STEP_TIME);
		remainingTime -= stepTime;
		float acceleratingTime = glm::clamp((maxFallSpeed - verticalVelocity) / gravity, 0.f, stepTime);
		verticalPosition += verticalVelocity * acceleratingTime + 0.5f * gravity * acceleratingTime * acceleratingTime;
		verticalVelocity = min(verticalVelocity + gravity * acceleratingTime, maxFallSpeed);
		verticalPosition += verticalVelocity * (stepTime - acceleratingTime);
		int targetY = int(floor(verticalPosition));
		int direction = targetY < posPlayer.y ? -1 : 1;
		// Sweep every pixel so fast movement cannot skip tiles or solid bombs.
		while(posPlayer.y != targetY)
		{
			posPlayer.y += direction;
			bool collided = direction < 0
				? map->collisionMoveUp(posPlayer, sizePlayer, &posPlayer.y)
				: map->collisionMoveDown(posPlayer, sizePlayer, &posPlayer.y);
			if(collided)
			{
				verticalPosition = float(posPlayer.y);
				verticalVelocity = 0.f;
				grounded = direction > 0;
				break;
			}
		}
	}
	if(grounded)
		map->stepOnPlatforms(posPlayer, sizePlayer);

	// Scene owns the bombs, so the player only asks for one on each key press
	if(Game::instance().getKey(GLFW_KEY_SPACE) && !bBombKeyPressed)
		bBombRequested = true;
	bBombKeyPressed = Game::instance().getKey(GLFW_KEY_SPACE);

	bJumpKeyPressed = jumpKeyPressed;

	// Airborne poses take priority over walking, even with no horizontal input.
	int animation = STAND;
	if(!grounded)
		animation = verticalVelocity < 0.f ? JUMP_UP : JUMP_DOWN;
	else if(posPlayer.x != previousPos.x)
		animation = MOVE;
	if(sprite->animation() != animation)
		sprite->changeAnimation(animation);
	sprite->setFlipX(facingRight);
	sprite->update(deltaTime);

	updateSpritePosition();
}

void Player::render()
{
	if(exiting)
	{
		if(!hasEnteredExit())
			sprite->render();
		return;
	}
	if(isDying() || lives <= 0)
	{
		deathSprite->render();
		return;
	}
	if(isInvulnerable() && !isBlinkVisible(blinkTime))
		return;
	sprite->render();
}

void Player::liftByTrampoline(int top, bool launch)
{
	if(!isAlive())
		return;
	int targetY = top - sizePlayer.y;
	int direction = targetY < posPlayer.y ? -1 : 1;
	while(posPlayer.y != targetY)
	{
		posPlayer.y += direction;
		bool collided = direction < 0
			? map->collisionMoveUp(posPlayer, sizePlayer, &posPlayer.y)
			: map->collisionMoveDown(posPlayer, sizePlayer, &posPlayer.y);
		if(collided)
			break;
	}
	verticalPosition = float(posPlayer.y);
	if(launch)
	{
		shortJumpActive = false;
		const float gravity = 8.f * HIGH_JUMP_HEIGHT_TILES * map->getTileSize() / (HIGH_JUMP_DURATION * HIGH_JUMP_DURATION);
		verticalVelocity = -sqrt(2.f * gravity * TRAMPOLINE_JUMP_HEIGHT_TILES * map->getTileSize());
	}
	updateSpritePosition();
}

bool Player::isBlinkVisible(int elapsedTime)
{
	return max(0, elapsedTime) % BLINK_CYCLE < BLINK_VISIBLE_TIME;
}

void Player::enterExit(const glm::vec2 &doorPosition)
{
	if(!isAlive())
		return;
	exiting = true;
	exitTime = 0;
	exitStart = glm::vec2(posPlayer);
	exitTarget = doorPosition - glm::vec2(tileMapDispl) + glm::vec2(HITBOX_INSET_X, 0);
	verticalVelocity = 0.f;
	horizontalRemainder = 0.f;
	bBombRequested = false;
	sprite->setScale(1.f);
	sprite->setFlipX(exitTarget.x > exitStart.x);
	sprite->changeAnimation(MOVE);
}

bool Player::hasEnteredExit() const
{
	return exiting && exitTime >= EXIT_TIME;
}

void Player::loseLife()
{
	if(!isAlive() || isInvulnerable())
		return;
	lives--;
	startDeath();
}

void Player::startDeath()
{
	deathTime = DEATH_TIME;
	invulnTime = 0;
	verticalVelocity = 0.f;
	horizontalRemainder = 0.f;
	bBombRequested = false;
	deathSprite->changeAnimation(0);
	Audio::instance().play(Audio::PLAYER_HURT);
}

void Player::loseAllLives()
{
	if(lives <= 0 || exiting)
		return;
	lives = 0;
	startDeath();
}

// Returns whether the player pressed the bomb key since the last call
bool Player::takeBombRequest()
{
	bool requested = bBombRequested && isAlive();
	bBombRequested = false;

	return requested;
}

// Buffs from picked-up items. Stats stop growing at their cap.
void Player::applyItem(Item::ItemType type)
{
	switch(type)
	{
	case Item::BOMB_UP:
		nBombs = min(nBombs + 1, MAX_BOMBS);
		break;
	case Item::FIRE_UP:
		firePower = min(firePower + 1, MAX_FIRE_POWER);
		break;
	case Item::SPEED_UP:
		moveSpeed = min(moveSpeed + SPEED_UP_INCREMENT, MAX_MOVE_SPEED);
		break;
	case Item::BOMB_PROOF:
		bombProof = true;
		break;
	default:
		break;
	}
}

Player::PowerUps Player::getPowerUps() const
{
	PowerUps powerUps = { nBombs, firePower, moveSpeed, bombProof };
	return powerUps;
}

int Player::getSpeedUpLevel() const
{
	return int(round((moveSpeed - BASE_MOVE_SPEED) / SPEED_UP_INCREMENT));
}

void Player::setPowerUps(const PowerUps &powerUps)
{
	nBombs = powerUps.nBombs;
	firePower = powerUps.firePower;
	moveSpeed = powerUps.moveSpeed;
	bombProof = powerUps.bombProof;
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = glm::ivec2(pos) + glm::ivec2(HITBOX_INSET_X, 0);
	verticalPosition = float(posPlayer.y);
	verticalVelocity = 0.f;
	horizontalRemainder = 0.f;
	moveDirection = 0;
	shortJumpActive = false;
	updateSpritePosition();
}

void Player::setSpawnPoint(const glm::vec2 &pos)
{
	spawnPoint = pos;
	setPosition(pos);
}

void Player::updateSpritePosition()
{
	sprite->setPosition(glm::vec2(tileMapDispl + posPlayer - glm::ivec2(HITBOX_INSET_X, 0)));
	deathSprite->setPosition(glm::vec2(tileMapDispl + posPlayer - glm::ivec2(HITBOX_INSET_X, 1)));
}



