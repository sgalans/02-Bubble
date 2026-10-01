#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include "Sprite.h"
#include "Texture.h"


// HUD draws the in-game interface (lives, score, ...) on top of the scene.
// It does not own any game state: Scene passes the current values every frame.
// It is rendered in screen space, so it does not move with the camera.


class HUD
{
public:
	HUD();
	~HUD();

public:
	void init(ShaderProgram &shaderProgram);
	void update(int deltaTime, int lives,int maxLives, int nBombs,int aliveEnemies);
	void render();

private:
	void renderLives();
	void renderRemainingEnemys();

private:
	ShaderProgram *program;
	int lives, maxLives,nBombs,numEnemys;
	Texture lifeTexture;
	Sprite *lifeSprite;
	Texture enemyTexture;
	Sprite *enemySprite;

};


#endif // _HUD_INCLUDE
