#include <GL/glew.h>
#include "Walker.h"


#define WALKER_SPEED 1.0f
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
#define FRONT_A_X 2.f
#define FRONT_B_X 17.f


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

		// The side frames only exist facing right and Sprite cannot mirror a
		// quad, so both directions use the front view, which reads either way.
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(FRONT_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(FRONT_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(FRONT_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(FRONT_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(MOVE_LEFT);
	tileMapDispl = tileMapPos;
	sizeEnemy = glm::ivec2(WALKER_WIDTH, WALKER_HEIGHT);
	lives = 1;
	updateSpritePosition();
}

void Walker::update(int deltaTime)
{
	sprite->update(deltaTime);

	if(sprite->animation() == MOVE_LEFT)
	{
		posEnemy.x -= WALKER_SPEED;
		if(map->collisionMoveLeft(posEnemy, sizeEnemy))
		{
			posEnemy.x += WALKER_SPEED;
			sprite->changeAnimation(MOVE_RIGHT);
		}
	}
	else
	{
		posEnemy.x += WALKER_SPEED;
		if(map->collisionMoveRight(posEnemy, sizeEnemy))
		{
			posEnemy.x -= WALKER_SPEED;
			sprite->changeAnimation(MOVE_LEFT);
		}
	}

	posEnemy.y += FALL_STEP;
	map->collisionMoveDown(posEnemy, sizeEnemy, &posEnemy.y);

	updateSpritePosition();
}
