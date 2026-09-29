#include <GL/glew.h>
#include "Walker.h"


#define WALKER_SPEED 2
#define FALL_STEP 4

// Source art is 13x16 px, drawn at 2x so the walker is as tall as the player.
#define WALKER_WIDTH 26
#define WALKER_HEIGHT 32

// bomberman.png is a ragged rip, so frames are addressed by pixel offset
// instead of a uniform grid. Row 10 spans y = 153..168 and holds, left to
// right: two front frames, two right-facing side frames, one back frame.
#define SHEET_WIDTH 253.f
#define SHEET_HEIGHT 632.f
#define FRAME_WIDTH 13.f
#define FRAME_HEIGHT 16.f
#define ROW_Y 153.f
#define SIDE_A_X 32.f
#define SIDE_B_X 46.f


enum WalkerAnims
{
	MOVE_LEFT, MOVE_RIGHT
};


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

		// The sheet only carries right-facing side frames, so both directions
		// share them until Sprite can mirror a quad horizontally.
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(SIDE_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(SIDE_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(SIDE_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(SIDE_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(MOVE_LEFT);
	tileMapDispl = tileMapPos;
	updateSpritePosition();
}

void Walker::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(sprite->animation() == MOVE_LEFT)
	{
		posEnemy.x -= WALKER_SPEED;
		if(map->collisionMoveLeft(posEnemy, glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT)))
		{
			posEnemy.x += WALKER_SPEED;
			sprite->changeAnimation(MOVE_RIGHT);
		}
	}
	else
	{
		posEnemy.x += WALKER_SPEED;
		if(map->collisionMoveRight(posEnemy, glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT)))
		{
			posEnemy.x -= WALKER_SPEED;
			sprite->changeAnimation(MOVE_LEFT);
		}
	}

	posEnemy.y += FALL_STEP;
	map->collisionMoveDown(posEnemy, glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT), &posEnemy.y);

	updateSpritePosition();
}
