#include <sstream>
#include <iomanip>
#include "HUD.h"
#include "Game.h"



#define HEART_SIZE 32
#define HEART_FRAME_W 0.2f
#define HEART_FRAME_H 0.5f
#define HEART_CHANGE_TIME 1500 // ms
#define HEART_BLINK_INTERVAL 150 // ms per visible/empty phase

#define HUD_MARGIN 8
#define HEART_SPACING 4

#define FONT_FILE "fonts/PressStart2P-Regular.ttf"
// The font is drawn on an 8 pixel grid, so multiples of 8 keep it crisp
#define STAT_SIZE 16
#define STAT_SPACING 24 // between the hearts and the columns, and between columns
#define STAT_ROW_SPACING 20
#define SHADOW_OFFSET 2

// The time turns red and blinks when this little is left
#define TIME_WARNING 30000 // ms
#define BLINK_PERIOD 250


enum HeartAnims
{
	HEART_FULL = 0, HEART_EMPTY = 4, HEART_FLASH_RED, HEART_FLASH_WHITE, N_HEART_ANIMS
};

HUD::HUD()
{
	program = NULL;
	lifeSprite = NULL;
	keySprite = NULL;
	speedUpLevel = 0;
	bombProof = false;
	hasKey = false;
	lives = maxLives = nBombs = firePower = enemiesLeft = timeLeft = 0;
	currentTime = 0.0f;
	firstAnimatedHeart = lastAnimatedHeart = -1;
	heartAnimationTime = 0;
	gainingLife = false;
}

HUD::~HUD()
{
	if(lifeSprite != NULL)
		delete lifeSprite;
	delete keySprite;
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
	lifeSprite->setNumberAnimations(N_HEART_ANIMS);
	for(int frame = 0; frame < 5; frame++)
	{
		lifeSprite->setAnimationSpeed(frame, 1);
		lifeSprite->addKeyframe(frame, glm::vec2(frame * HEART_FRAME_W, 0.f));
	}
	lifeSprite->setAnimationSpeed(HEART_FLASH_RED, 1);
	lifeSprite->addKeyframe(HEART_FLASH_RED, glm::vec2(3 * HEART_FRAME_W, HEART_FRAME_H));
	lifeSprite->setAnimationSpeed(HEART_FLASH_WHITE, 1);
	lifeSprite->addKeyframe(HEART_FLASH_WHITE, glm::vec2(4 * HEART_FRAME_W, HEART_FRAME_H));

	keyTexture.loadFromFile("images/personalized.png", TEXTURE_PIXEL_FORMAT_RGBA);
	keyTexture.setMinFilter(GL_NEAREST);
	keyTexture.setMagFilter(GL_NEAREST);
	glm::vec2 keyFrame(16.f / keyTexture.width(), 16.f / keyTexture.height());
	keySprite = Sprite::createSprite(glm::vec2(HEART_SIZE), keyFrame, &keyTexture, program);
	keySprite->setNumberAnimations(1);
	keySprite->setAnimationSpeed(0, 1);
	keySprite->addKeyframe(0, glm::vec2(0.f, 1.f) * keyFrame);
	keySprite->changeAnimation(0);
	keySprite->setPosition(glm::vec2(SCREEN_WIDTH - HUD_MARGIN - HEART_SIZE, HUD_MARGIN));

	statText.init(FONT_FILE, STAT_SIZE, *program);
}

void HUD::update(int deltaTime, int lives, int maxLives, int nBombs, int firePower, int enemiesLeft, int timeLeft, bool hasKey,
                 int speedUpLevel, bool bombProof)
{
	currentTime += deltaTime;
	heartAnimationTime = glm::max(0, heartAnimationTime - deltaTime);
	if(lives != this->lives && this->maxLives == maxLives)
	{
		gainingLife = lives > this->lives;
		firstAnimatedHeart = glm::min(lives, this->lives);
		lastAnimatedHeart = glm::max(lives, this->lives) - 1;
		heartAnimationTime = HEART_CHANGE_TIME;
	}
	else if(lives != this->lives || maxLives != this->maxLives)
		heartAnimationTime = 0;
	this->lives = lives;
	if(this->maxLives != maxLives){
		//TODO: animation when max lives changes
	}
	this->maxLives = maxLives;
	this->nBombs = nBombs;
	this->firePower = firePower;
	this->enemiesLeft = enemiesLeft;
	this->timeLeft = timeLeft;
	this->hasKey = hasKey;
	this->speedUpLevel = speedUpLevel;
	this->bombProof = bombProof;
}

void HUD::render()
{
	renderLives();
	renderStats();
	if(hasKey)
		keySprite->render();
}

void HUD::renderLives()
{
	// One heart slot per possible life: full for lives left, empty for the rest
	for(int i = 0; i < maxLives; i++)
	{
		int frame = i < lives ? HEART_FULL : HEART_EMPTY;
		if(i >= firstAnimatedHeart && i <= lastAnimatedHeart && heartAnimationTime > 0)
		{
			int elapsed = HEART_CHANGE_TIME - heartAnimationTime;
			if(gainingLife)
				elapsed = HEART_CHANGE_TIME - 1 - elapsed;
			frame = HEART_EMPTY;
			// Full, three quarters, half, quarter: the atlas is not in fill order.
			static const int lossFrames[] = { HEART_FULL, 3, 1, 2 };
			if(elapsed < 80)
				frame = HEART_FLASH_WHITE;
			else if(elapsed < HEART_BLINK_INTERVAL)
				frame = HEART_FLASH_RED;
			else if((elapsed / HEART_BLINK_INTERVAL) % 2 == 0)
				frame = lossFrames[glm::min(elapsed * 4 / HEART_CHANGE_TIME, 3)];
		}
		lifeSprite->changeAnimation(frame);
		lifeSprite->setPosition(glm::vec2(HUD_MARGIN + i * (HEART_SIZE + HEART_SPACING), HUD_MARGIN));
		lifeSprite->render();
	}
}


void HUD::renderStats()
{
	// Show whole seconds rounded up, so 0:00 only appears when time is really over
	int seconds = (timeLeft + 999) / 1000;
	stringstream time, enemies, bombs, fire;
	time << "TIME " << seconds / 60 << ":" << setw(2) << setfill('0') << seconds % 60;
	enemies << "ENEMY " << enemiesLeft;
	bombs << "BOMB " << nBombs;
	fire << "FIRE " << firePower;

	glm::vec4 white(1.f, 1.f, 1.f, 1.f);
	glm::vec4 timeColor = white;
	if(timeLeft <= TIME_WARNING)
		timeColor = int(currentTime / BLINK_PERIOD) % 2 == 0 ? glm::vec4(1.f, 0.2f, 0.2f, 1.f) : white;

	// Three columns: time/enemies, bombs/fire, speed/bomb proof.
	glm::vec2 col1(HUD_MARGIN + maxLives * (HEART_SIZE + HEART_SPACING) - HEART_SPACING + STAT_SPACING, HUD_MARGIN);
	glm::vec2 row2(0.f, STAT_ROW_SPACING);
	renderText(time.str(), col1, timeColor);
	renderText(enemies.str(), col1 + row2, enemiesLeft == 0 ? glm::vec4(0.3f, 1.f, 0.3f, 1.f) : white);

	int firstColumnWidth = glm::max(statText.width("TIME 0:00"), statText.width(enemies.str()));
	glm::vec2 col2 = col1 + glm::vec2(firstColumnWidth + STAT_SPACING, 0.f);
	renderText(bombs.str(), col2, white);
	renderText(fire.str(), col2 + row2, white);

	int secondColumnWidth = glm::max(statText.width(bombs.str()), statText.width(fire.str()));
	glm::vec2 col3 = col2 + glm::vec2(secondColumnWidth + STAT_SPACING, 0.f);
	stringstream speed;
	speed << "SPEED " << speedUpLevel + 1;
	renderText(speed.str(), col3, white);
	renderText(bombProof ? "VEST ON" : "VEST OFF", col3 + row2,
	           bombProof ? glm::vec4(0.3f, 1.f, 0.3f, 1.f) : white);
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
}

void HUD::renderText(const string &str, const glm::vec2 &pos, const glm::vec4 &color)
{
	statText.render(str, pos + glm::vec2(SHADOW_OFFSET), glm::vec4(0.f, 0.f, 0.f, 1.f));
	statText.render(str, pos, color);
}
