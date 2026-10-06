#include "HUD.h"
#include "Game.h"


// hearts.png is a 5x2 grid of 32x32 hearts
#define HEART_SIZE 32
#define HEART_FRAME_W 0.2f
#define HEART_FRAME_H 0.5f

#define HUD_MARGIN 8
#define HEART_SPACING 4


enum HeartAnims
{
	HEART_FULL, HEART_EMPTY
};

HUD::HUD()
{
	program = NULL;
	lifeSprite = NULL;
	lives = maxLives = nBombs = 0;
}

HUD::~HUD()
{
	if(lifeSprite != NULL)
		delete lifeSprite;
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
}

void HUD::update(int deltaTime, int lives,int maxLives, int nBombs)
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
	
}

void HUD::render()
{
	renderLives();

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


