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

void Door::init(const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap, ShaderProgram &shaderProgram,
                bool keyGate)
{
	this->tile = tile;
	tileMapDispl = tileMapPos;
	map = tileMap;
	bOpen = false;
	this->keyGate = keyGate;

	spritesheet.loadFromFile(keyGate ? "images/personalized.png" : "images/door.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
	glm::vec2 frameSize = keyGate ? glm::vec2(16.f / spritesheet.width(), 16.f / spritesheet.height())
	                              : glm::vec2(DOOR_FRAME_W, 1.f);
	sprite = Sprite::createSprite(glm::vec2(getSize()), frameSize, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);
	// Every animation needs a speed, even with one keyframe: Sprite::update loops forever at 0
	sprite->setAnimationSpeed(CLOSED, 1);
	sprite->setAnimationSpeed(OPEN, 1);
	glm::vec2 closedFrame = keyGate ? glm::vec2(0.f, 32.f / spritesheet.height()) : glm::vec2(0.f);
	sprite->addKeyframe(CLOSED, closedFrame);
	sprite->addKeyframe(OPEN, keyGate ? closedFrame : glm::vec2(DOOR_FRAME_W, 0.f));
	sprite->changeAnimation(CLOSED);
	sprite->setPosition(getPosition());
}

void Door::update(int deltaTime)
{
	sprite->update(deltaTime);
}

void Door::render()
{
	if(!keyGate || !bOpen)
		sprite->render();
}

void Door::open()
{
	if(bOpen || (keyGate && !map->openKeyGate(tile.x, tile.y)))
		return;
	bOpen = true;
	sprite->changeAnimation(OPEN);
}
