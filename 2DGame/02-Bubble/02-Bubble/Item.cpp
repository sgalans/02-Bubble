#include "Item.h"


// The power-up icons on bomberman.png are 16x16 squares laid out on an 18 px
// grid starting at (21, 117). Each entry is the pixel offset of one ItemType.
#define ICON_SIZE 16.f

static const glm::vec2 ICON_PIXELS[Item::N_TYPES] =
{
	glm::vec2(21.f, 117.f),  // BOMB_UP: bomb
	glm::vec2(39.f, 117.f),  // FIRE_UP: flame
	glm::vec2(57.f, 117.f),  // SPEED_UP: roller skates
	glm::vec2(75.f, 117.f)   // BOMB_PROOF: vest
};


enum ItemAnims
{
	IDLE
};


Item::Item()
{
	sprite = NULL;
	map = NULL;
}

Item::~Item()
{
	if(sprite != NULL)
		delete sprite;
}

void Item::init(ItemType type, const glm::ivec2 &tile, const glm::ivec2 &tileMapPos, TileMap *tileMap,
                Texture *sheet, ShaderProgram &shaderProgram)
{
	this->type = type;
	this->tile = tile;
	tileMapDispl = tileMapPos;
	map = tileMap;
	collected = false;

	glm::vec2 sheetSize(float(sheet->width()), float(sheet->height()));
	sprite = Sprite::createSprite(glm::vec2(getSize()), glm::vec2(ICON_SIZE) / sheetSize, sheet, &shaderProgram);
	sprite->setNumberAnimations(1);

		sprite->setAnimationSpeed(IDLE, 1);
		sprite->addKeyframe(IDLE, ICON_PIXELS[type] / sheetSize);

	sprite->changeAnimation(IDLE);
	sprite->setPosition(getPosition());
}

void Item::update(int deltaTime)
{
	sprite->update(deltaTime);
}

void Item::render()
{
	if(isAvailable())
		sprite->render();
}

// B = bomb up, F = fire up, S = speed up, P = bomb proof
bool Item::typeFromLetter(char letter, ItemType &type)
{
	switch(letter)
	{
	case 'B':
		type = BOMB_UP;
		return true;
	case 'F':
		type = FIRE_UP;
		return true;
	case 'S':
		type = SPEED_UP;
		return true;
	case 'P':
		type = BOMB_PROOF;
		return true;
	default:
		return false;
	}
}
