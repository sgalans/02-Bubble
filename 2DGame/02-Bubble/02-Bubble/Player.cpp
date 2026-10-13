#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4
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
	bJumping = false;
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
	
	if(bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			bJumping = false;
			posPlayer.y = startY;
			glm::ivec2 groundProbe = posPlayer + glm::ivec2(0, FALL_STEP);
			grounded = map->collisionMoveDown(groundProbe, sizePlayer, &groundProbe.y);
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
			{
				grounded = map->collisionMoveDown(posPlayer, sizePlayer, &posPlayer.y);
				bJumping = !grounded;
			}
			else if(map->collisionMoveUp(posPlayer, sizePlayer, &posPlayer.y))
				jumpAngle = 180 - jumpAngle;
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		grounded = map->collisionMoveDown(posPlayer, sizePlayer, &posPlayer.y);
		if(grounded)
		{
			if((Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_W)) && !bJumpKeyPressed)
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
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

	bJumpKeyPressed = Game::instance().getKey(GLFW_KEY_UP) || Game::instance().getKey(GLFW_KEY_W);

	// Airborne poses take priority over walking, even with no horizontal input.
	int animation = STAND;
	if(bJumping)
		animation = jumpAngle < 90 ? JUMP_UP : JUMP_DOWN;
	else if(!grounded)
		animation = JUMP_DOWN;
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
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}




