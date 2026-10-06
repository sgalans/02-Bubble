#include "Bomb.h"


#define FUSE_TIME 2500 //ms
#define EXPLOSION_TIME 500 //ms

// bombs.png holds three 16x16 bomb frames, each a bit smaller than the
// previous one, so cycling them makes the bomb pulse. They are not on a
// regular 16 px grid, so each frame is addressed by its pixel offset.
#define SHEET_WIDTH 49.f
#define SHEET_HEIGHT 16.f
#define FRAME_SIZE 16.f
#define ROW_Y 0.f
#define FRAME_A_X 0.f
#define FRAME_B_X 17.f
#define FRAME_C_X 33.f


enum BombAnims
{
	PULSE
};


Bomb::Bomb()
{
	sprite = NULL;
	map = NULL;
}

Bomb::~Bomb()
{
	if(sprite != NULL)
		delete sprite;
}

void Bomb::init(const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap,
                Texture *spritesheet, ShaderProgram &shaderProgram)
{
	this->tile = tile;
	tileMapDispl = tileMapPos;
	map = tileMap;
	state = FUSE;
	timer = FUSE_TIME;

	sprite = Sprite::createSprite(glm::vec2(getSize()),
	                              glm::vec2(FRAME_SIZE / SHEET_WIDTH, FRAME_SIZE / SHEET_HEIGHT),
	                              spritesheet, &shaderProgram);
	sprite->setNumberAnimations(1);

		sprite->setAnimationSpeed(PULSE, 6);
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_A_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_C_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));
		sprite->addKeyframe(PULSE, glm::vec2(FRAME_B_X / SHEET_WIDTH, ROW_Y / SHEET_HEIGHT));

	sprite->changeAnimation(PULSE);
	sprite->setPosition(getPosition());
}

void Bomb::update(int deltaTime)
{
	if(state == DONE)
		return;

	timer -= deltaTime;
	if(state == FUSE)
	{
		sprite->update(deltaTime);
		if(timer <= 0)
			explode();
	}
	else if(state == EXPLODING && timer <= 0)
		state = DONE;
}

void Bomb::render()
{
	// The flames will be drawn here once the explosion is implemented
	if(state == FUSE)
		sprite->render();
}

// Also used to detonate it early (e.g. when another explosion reaches it)
void Bomb::explode()
{
	if(state != FUSE)
		return;
	map->clearBomb(tile.x, tile.y);
	state = EXPLODING;
	timer = EXPLOSION_TIME;
}
