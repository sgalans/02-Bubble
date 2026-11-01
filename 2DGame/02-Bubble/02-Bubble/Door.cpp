#include "Door.h"


// Placeholder art: door.png holds two 16x16 frames side by side, closed then open
#define DOOR_FRAME_W 0.5f


enum DoorAnims
{
	CLOSED, OPEN
};


Door::Door()
{
	sprite = NULL;
	map = NULL;
}

Door::~Door()
{
	if(sprite != NULL)
		delete sprite;
}

void Door::init(const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap, ShaderProgram &shaderProgram)
{
	this->tile = tile;
	tileMapDispl = tileMapPos;
	map = tileMap;
	bOpen = false;

	spritesheet.loadFromFile("images/door.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
	sprite = Sprite::createSprite(glm::vec2(getSize()), glm::vec2(DOOR_FRAME_W, 1.f), &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);
	// Every animation needs a speed, even with one keyframe: Sprite::update loops forever at 0
	sprite->setAnimationSpeed(CLOSED, 1);
	sprite->setAnimationSpeed(OPEN, 1);
	sprite->addKeyframe(CLOSED, glm::vec2(0.f, 0.f));
	sprite->addKeyframe(OPEN, glm::vec2(DOOR_FRAME_W, 0.f));
	sprite->changeAnimation(CLOSED);
	sprite->setPosition(getPosition());
}

void Door::update(int deltaTime)
{
	sprite->update(deltaTime);
}

void Door::render()
{
	sprite->render();
}

void Door::open()
{
	if(bOpen)
		return;
	bOpen = true;
	sprite->changeAnimation(OPEN);
}
