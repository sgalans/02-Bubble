#include "Projectile.h"


// Small orange ball near the bottom right of bomberman.png
#define BALL_SIZE 8
#define BALL_PIXEL_X 229.f
#define BALL_PIXEL_Y 530.f


enum ProjectileAnims
{
	FLY
};


Projectile::Projectile()
{
	sprite = NULL;
	map = NULL;
	done = false;
}

Projectile::~Projectile()
{
	if(sprite != NULL)
		delete sprite;
}

void Projectile::init(const glm::vec2 &center, const glm::vec2 &velocity, const glm::ivec2 &tileMapPos,
                      TileMap *tileMap, Texture *sheet, ShaderProgram &shaderProgram)
{
	tileMapDispl = tileMapPos;
	map = tileMap;
	size = glm::ivec2(BALL_SIZE);
	pos = center - glm::vec2(tileMapDispl) - glm::vec2(size) / 2.f;
	this->velocity = velocity;
	done = false;

	glm::vec2 sheetSize(float(sheet->width()), float(sheet->height()));
	sprite = Sprite::createSprite(glm::vec2(size), glm::vec2(float(BALL_SIZE)) / sheetSize, sheet, &shaderProgram);
	sprite->setNumberAnimations(1);

		sprite->setAnimationSpeed(FLY, 1);
		sprite->addKeyframe(FLY, glm::vec2(BALL_PIXEL_X, BALL_PIXEL_Y) / sheetSize);

	sprite->changeAnimation(FLY);
	sprite->setPosition(getPosition());
}

void Projectile::update(int deltaTime)
{
	if(done)
		return;

	pos += velocity * (deltaTime / 1000.f);
	sprite->setPosition(getPosition());

	
	glm::ivec2 tile = glm::ivec2(pos + glm::vec2(size) / 2.f) / map->getTileSize();
	if(map->isWall(tile.x, tile.y) || map->hasBomb(tile.x, tile.y))
		done = true;
}

void Projectile::render()
{
	if(!done)
		sprite->render();
}
