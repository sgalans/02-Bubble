#include "HUD.h"
#include "Game.h"


// hearts.png is a 5x2 grid of 32x32 hearts
#define HEART_SIZE 32
#define HEART_FRAME_W 0.2f
#define HEART_FRAME_H 0.5f

#define HUD_MARGIN 8
#define HEART_SPACING 4

// Enemy counter icon: the walker's front frame from bomberman.png (13x16 px),
// drawn at 2x like the walker itself. Offsets match Walker.cpp.
#define ENEMY_SHEET_W 253.f
#define ENEMY_SHEET_H 632.f
#define ENEMY_FRAME_X 2.f
#define ENEMY_FRAME_Y 153.f
#define ENEMY_FRAME_W 13.f
#define ENEMY_FRAME_H 16.f
#define ENEMY_ICON_W 26
#define ENEMY_ICON_H 32
#define ENEMY_SPACING 4
#define ROW_SPACING 4


enum HeartAnims
{
	HEART_FULL, HEART_EMPTY
};

HUD::HUD()
{
	program = NULL;
	lifeSprite = NULL;
	enemySprite = NULL;
	lives = maxLives = nBombs = numEnemys = 0;
}

HUD::~HUD()
{
	if(lifeSprite != NULL)
		delete lifeSprite;
	if(enemySprite != NULL)
		delete enemySprite;
}


void HUD::init(ShaderProgram &shaderProgram)
{
	program = &shaderProgram;

	lifeTexture.loadFromFile("images/hearts.png", TEXTURE_PIXEL_FORMAT_RGBA);
	lifeTexture.setMinFilter(GL_NEAREST);
	lifeTexture.setMagFilter(GL_NEAREST);

	lifeSprite = Sprite::createSprite(glm::ivec2(HEART_SIZE, HEART_SIZE),
	                                  glm::vec2(HEART_FRAME_W, HEART_FRAME_H),
	                                  &lifeTexture, program);
	lifeSprite->setNumberAnimations(2);
	lifeSprite->addKeyframe(HEART_FULL, glm::vec2(0.f, 0.f));                 // top row, first
	lifeSprite->addKeyframe(HEART_EMPTY, glm::vec2(4 * HEART_FRAME_W, 0.f));  // top row, last

	enemyTexture.loadFromFile("images/bomberman.png", TEXTURE_PIXEL_FORMAT_RGBA);
	// Frames sit 2-3 px apart on the sheet, so filtering would bleed neighbours in.
	enemyTexture.setMinFilter(GL_NEAREST);
	enemyTexture.setMagFilter(GL_NEAREST);

	enemySprite = Sprite::createSprite(glm::ivec2(ENEMY_ICON_W, ENEMY_ICON_H),
	                                   glm::vec2(ENEMY_FRAME_W / ENEMY_SHEET_W, ENEMY_FRAME_H / ENEMY_SHEET_H),
	                                   &enemyTexture, program);
	enemySprite->setNumberAnimations(1);
	enemySprite->addKeyframe(0, glm::vec2(ENEMY_FRAME_X / ENEMY_SHEET_W, ENEMY_FRAME_Y / ENEMY_SHEET_H));
	enemySprite->changeAnimation(0);
}

void HUD::update(int deltaTime, int lives,int maxLives, int nBombs,int aliveEnemies)
{
	if(this->lives != lives){
		//TODO: animation when a life is lost
	}
	this->lives = lives;
	if(this->maxLives != maxLives){
		//TODO: animation when max lives changes
	}
	this->maxLives = maxLives;
	this->nBombs = nBombs;
	this->numEnemys = aliveEnemies;
	
}

void HUD::render()
{
	renderLives();
	renderRemainingEnemys();

}

void HUD::renderLives()
{
	// One heart slot per possible life: full for lives left, empty for the rest
	for(int i = 0; i < maxLives; i++)
	{
		lifeSprite->changeAnimation(i < lives ? HEART_FULL : HEART_EMPTY);
		lifeSprite->setPosition(glm::vec2(HUD_MARGIN + i * (HEART_SIZE + HEART_SPACING), HUD_MARGIN));
		lifeSprite->render();
	}
}

void HUD::renderRemainingEnemys()
{
	// One enemy icon per enemy still alive, in a row just below the hearts
	float y = float(HUD_MARGIN + HEART_SIZE + ROW_SPACING);
	for(int i = 0; i < numEnemys; i++)
	{
		enemySprite->setPosition(glm::vec2(HUD_MARGIN + i * (ENEMY_ICON_W + ENEMY_SPACING), y));
		enemySprite->render();
	}
}


