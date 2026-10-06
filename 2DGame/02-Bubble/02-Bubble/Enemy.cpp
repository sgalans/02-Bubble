#include "Enemy.h"


#define HIT_COOLDOWN 1000 //ms, longer than an explosion lasts


Enemy::Enemy()
{
	sprite = NULL;
	map = NULL;
	lives = 1;
	hitCooldown = 0;
}

void Enemy::takeHit()
{
	if(hitCooldown > 0 || lives <= 0)
		return;
	lives--;
	hitCooldown = HIT_COOLDOWN;
}

void Enemy::updateHitCooldown(int deltaTime)
{
	if(hitCooldown > 0)
		hitCooldown = glm::max(0, hitCooldown - deltaTime);
}

Enemy::~Enemy()
{
	if (sprite != NULL)
		delete sprite;
}

void Enemy::render()
{
	sprite->render();
}

void Enemy::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Enemy::setPosition(const glm::vec2 &pos)
{
	posEnemy = pos;
	updateSpritePosition();
}

void Enemy::updateSpritePosition()
{
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y)));
}
