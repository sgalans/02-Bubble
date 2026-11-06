#ifndef _HUD_INCLUDE
#define _HUD_INCLUDE


#include <string>
#include "Sprite.h"
#include "Texture.h"
#include "Text.h"


// HUD draws lives, time, enemies, BOMB/FIRE/SPEED levels, VEST and the held key.
// It does not own any game state: Scene passes the current values every frame.
// It is rendered in screen space, so it does not move with the camera.


class HUD
{
public:
	HUD();
	~HUD();

public:
	void init(ShaderProgram &shaderProgram);
	void update(int deltaTime, int lives, int maxLives, int nBombs, int firePower, int enemiesLeft, int timeLeft, bool hasKey,
	            int speedUpLevel, bool bombProof);
	void render();

private:
	void renderLives();
	void renderStats();
	// Draws the string with a dark drop shadow so it stays readable over any tile
	void renderText(const std::string &str, const glm::vec2 &pos, const glm::vec4 &color);

private:
	ShaderProgram *program;
	int lives, maxLives, nBombs, firePower, enemiesLeft, timeLeft;
	int firstLostHeart, lastLostHeart, heartLossTime; // Lost slots share the same animation
	float currentTime;
	bool hasKey;
	int speedUpLevel;
	bool bombProof;
	Texture lifeTexture;
	Texture keyTexture;
	Sprite *lifeSprite;
	Sprite *keySprite;
	Text statText;

};


#endif // _HUD_INCLUDE
