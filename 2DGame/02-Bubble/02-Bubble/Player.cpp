#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define HIGH_JUMP_HEIGHT_TILES 5
#define SHORT_JUMP_HEIGHT_TILES 2
#define HIGH_JUMP_DURATION 0.8f // seconds, returning to the launch height
#define SHORT_JUMP_DURATION 0.5f
#define MAX_VERTICAL_STEP_TIME (1.f / 120.f)
#define INVULNERABILITY_TIME 2000 //ms



enum PlayerAnims
{
	STAND, MOVE, JUMP_UP, JUMP_DOWN
};


Player::Player()
{
	sprite = NULL;
	map = NULL;
}

Player::~Player()
{
	if (sprite != NULL)
		delete sprite;
}

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	verticalPosition = 0.f;
	verticalVelocity = 0.f;
	shortJumpActive = false;
	facingRight = false;
	bJumpKeyPressed = false;
	bGodKeyPressed = false;
	bBombKeyPressed = false;
	bBombRequested = false;
	lives = 2;
	maxLives = 3;
	nBombs = 1;
	firePower = 5; // set to 5 for easier testing
	invulnTime = 0;
	invensible = false;
	spritesheet.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
	sizePlayer = glm::ivec2(16, 16);
	// Left-facing poses on the ragged sheet, cropped to 16x16 at y = 2.
	glm::vec2 frameSize(16.f / spritesheet.width(), 16.f / spritesheet.height());
	glm::vec2 standFrame(4.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 frameA(19.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 frameB(36.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 jumpUpFrame(86.f / spritesheet.width(), 2.f / spritesheet.height());
	glm::vec2 jumpDownFrame(69.f / spritesheet.width(), 2.f / spritesheet.height());
	sprite = Sprite::createSprite(glm::vec2(sizePlayer), frameSize, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(4);
	
		sprite->setAnimationSpeed(STAND, 8);
		sprite->addKeyframe(STAND, standFrame);
		
		sprite->setAnimationSpeed(MOVE, 8);
		sprite->addKeyframe(MOVE, frameA);
		sprite->addKeyframe(MOVE, frameB);
		
		sprite->setAnimationSpeed(JUMP_UP, 8);
		sprite->addKeyframe(JUMP_UP, jumpUpFrame);
		
		sprite->setAnimationSpeed(JUMP_DOWN, 8);
		sprite->addKeyframe(JUMP_DOWN, jumpDownFrame);
		
	sprite->changeAnimation(STAND);
	sprite->setFlipX(false);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}

void Player::update(int deltaTime)
{
	// Out of lives: stay put and stop animating until the game handles it.
	if(lives <= 0)
		return;
	if(invulnTime > 0)
		invulnTime = max(0, invulnTime - deltaTime);

	glm::ivec2 previousPos = posPlayer;
	bool highJumpKeyPressed = Game::instance().getKey(GLFW_KEY_UP)
		|| Game::instance().getKey(GLFW_KEY_W);
	bool jumpKeyPressed = highJumpKeyPressed || Game::instance().getKey(GLFW_KEY_S);
	bool grounded = false;
	if(Game::instance().getKey(GLFW_KEY_LEFT) || Game::instance().getKey(GLFW_KEY_A))
	{
		facingRight = false;
		posPlayer.x -= 2;
		if(map->collisionMoveLeft(posPlayer, sizePlayer))
		{
			posPlayer.x += 2;
		}
	}
	else if(Game::instance().getKey(GLFW_KEY_RIGHT) || Game::instance().getKey(GLFW_KEY_D))
	{
		facingRight = true;
		posPlayer.x += 2;
		if(map->collisionMoveRight(posPlayer, sizePlayer))
		{
			posPlayer.x -= 2;
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
		shortJumpActive = startJump && !highJumpKeyPressed;

	// For total flight time T and height H: gravity = 8H/T^2, launch speed = -4H/T.
	const float jumpHeight = float((shortJumpActive ? SHORT_JUMP_HEIGHT_TILES : HIGH_JUMP_HEIGHT_TILES) * map->getTileSize());
	const float jumpDuration = shortJumpActive ? SHORT_JUMP_DURATION : HIGH_JUMP_DURATION;
	const float gravity = 8.f * jumpHeight / (jumpDuration * jumpDuration);
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
		verticalPosition += verticalVelocity * stepTime + 0.5f * gravity * stepTime * stepTime;
		verticalVelocity += gravity * stepTime;
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
	if(Game::instance().getKey(GLFW_KEY_G) && !bGodKeyPressed)
		invensible = !invensible;
	bGodKeyPressed = Game::instance().getKey(GLFW_KEY_G);

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

	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}

void Player::render()
{
	sprite->render();
}

void Player::loseLife()
{
	if(invulnTime <= 0 && !invensible){
		invulnTime = INVULNERABILITY_TIME;
		if(lives > 0)
			lives--;
	}
}

// Returns whether the player pressed the bomb key since the last call
bool Player::takeBombRequest()
{
	bool requested = bBombRequested;
	bBombRequested = false;

	return requested;
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	verticalPosition = float(posPlayer.y);
	verticalVelocity = 0.f;
	shortJumpActive = false;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




