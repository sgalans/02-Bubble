#include "Door.h"


// Tiles 7-9 hold the key gate, closed exit and open exit in the 4x5 atlas.


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

	spritesheet.loadFromFile("images/personalized.png", TEXTURE_PIXEL_FORMAT_RGBA);
	spritesheet.setMinFilter(GL_NEAREST);
	spritesheet.setMagFilter(GL_NEAREST);
	glm::vec2 frameSize(16.f / spritesheet.width(), 16.f / spritesheet.height());
	sprite = Sprite::createSprite(glm::vec2(getSize()), frameSize, &spritesheet, &shaderProgram);
	sprite->setNumberAnimations(2);
	// Every animation needs a speed, even with one keyframe: Sprite::update loops forever at 0
	sprite->setAnimationSpeed(CLOSED, 1);
	sprite->setAnimationSpeed(OPEN, 1);
	glm::vec2 closedFrame = glm::vec2(keyGate ? 2.f : 3.f, 1.f) * frameSize;
	sprite->addKeyframe(CLOSED, closedFrame);
	sprite->addKeyframe(OPEN, glm::vec2(0.f, 2.f) * frameSize);
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
	if(bOpen || (keyGate && !map->openKeyGate(tile.x, tile.y)))
		return;
	bOpen = true;
	sprite->changeAnimation(OPEN);
}
