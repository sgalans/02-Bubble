#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Game.h"
#include "Audio.h"


#define FADE_TIME 400 // ms for each half (to black, then back)
// A frame that loads a level takes long; capping the step keeps the fade-in from being skipped
#define FADE_MAX_STEP 33 // ms


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	Audio::instance().init();
	menu.init();
	initFade();
	state = MENU;
	Audio::instance().playMusic(Audio::MUSIC_TITLE);
	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		levelItems[i] = Player::ItemCounts();
		entryLives[i] = 3;
	}
}

bool Game::update(int deltaTime)
{
	// The game is frozen while a transition plays
	if(fadePhase != FADE_NONE)
	{
		updateFade(deltaTime);
		return bPlay;
	}

	if(state == MENU)
	{
		menu.update(deltaTime);
		int level = menu.takeChosenLevel();
		if(level != 0)
			startFade(ACTION_START_LEVEL, level);
	}
	else
	{
		scene->update(deltaTime);
		// After the last level the victory picture fades in instead of popping up
		if(scene->isWaitingForFinalScreen())
			startFade(ACTION_SHOW_FINAL_SCREEN);
		// The win screen gets its jingle once, without looping
		if(scene->isCleared())
			Audio::instance().playMusic(Audio::MUSIC_WORLD_CLEAR, false);
	}

	return bPlay;
}

void Game::render()
{
	// Clear the bars in black, then the logical game area in its usual colour.
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glEnable(GL_SCISSOR_TEST);
	glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	if(state == MENU)
		menu.render();
	else
		scene->render();
	renderFade();
}

void Game::keyPressed(int key)
{
	// Input is ignored during a transition so a second key cannot queue another switch
	if(fadePhase != FADE_NONE)
	{
		keys[key] = true;
		return;
	}
	// Debug: jump straight to any level, bypassing menu unlocks.
	if(key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
	{
		if(keys[key])
			return;
		keys[key] = true;
		int level = key - GLFW_KEY_1 + 1;
		std::ifstream levelFile(Menu::levelFile(level));
		if(!levelFile.is_open())
			return;
		startFade(ACTION_JUMP_TO_LEVEL, level);
		return;
	}
	if(key == GLFW_KEY_ESCAPE) // Escape code
	{
		// Escape leaves the level first, then steps back through the menu
		// screens, and quits only from the main menu.
		// Leaving from the win screen still counts the level as beaten.
		if(state == PLAYING)
		{
			startFade(ACTION_BACK_TO_MENU);
		}
		else if(!menu.back())
			bPlay = false;
		return;
	}
	// Enter on the win screen moves on to the next level
	if(state == PLAYING && scene->isCleared() && (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER))
	{
		startFade(ACTION_LEVEL_CLEARED);
		return;
	}
	// Debug: K wipes out every enemy, which also opens the exit door
	if(state == PLAYING && key == GLFW_KEY_K)
		scene->killAllEnemies();
	// Debug: P gives the player every power-up
	if(state == PLAYING && key == GLFW_KEY_P)
		scene->giveAllPowerUps();
	if(state == MENU)
		menu.keyPressed(key);
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}

void Game::startLevel(int level)
{
	const int levelMinutes[MENU_NUM_LEVELS] = { 3, 4, 5, 5, 3 };
	scene = new Scene();
	scene->init(Menu::levelFile(level), levelMinutes[level - 1] * 60 * 1000);
	scene->setFinalLevel(level == MENU_NUM_LEVELS);
	scene->setPowerUps(getStartingPowerUps(level));
	scene->setLives(getStartingLives(level));
	currentLevel = level;
	state = PLAYING;
	// Levels 1-4 are the forest; the last level is the boss fight
	Audio::instance().playMusic(level == MENU_NUM_LEVELS ? Audio::MUSIC_BOSS : Audio::MUSIC_FOREST);
}

Player::PowerUps Game::getStartingPowerUps(int level) const
{
	// Shared by level loading and the menu preview, with the same stat caps.
	Player::ItemCounts startingItems = {};
	for(int i = 0; i < level - 1; i++)
	{
		startingItems.bombs += levelItems[i].bombs;
		startingItems.fire += levelItems[i].fire;
		startingItems.speed += levelItems[i].speed;
	}
	return Player::powerUpsFromItems(startingItems);
}

void Game::backToMenu()
{
	if(scene->isCleared())
	{
		menu.unlockLevel(currentLevel + 1);
		saveLevelProgress();
	}
	delete scene;
	scene = NULL;
	state = MENU;
	Audio::instance().playMusic(Audio::MUSIC_TITLE);
}

// Unlocks the next level and goes straight into it. After the last level, or if
// the next level file is missing, it returns to the menu instead.
void Game::levelCleared()
{
	int next = currentLevel + 1;
	menu.unlockLevel(next);
	saveLevelProgress();
	delete scene;
	scene = NULL;
	if(next <= MENU_NUM_LEVELS && menu.isPlayable(next))
		startLevel(next);
	else
	{
		state = MENU;
		Audio::instance().playMusic(Audio::MUSIC_TITLE);
	}
}

void Game::saveLevelProgress()
{
	// Replace this level's contribution; replays never add to an old result.
	levelItems[currentLevel - 1] = scene->getCollectedItems();
	if(currentLevel < MENU_NUM_LEVELS)
		entryLives[currentLevel] = scene->getLives();
}

// Debug level jump: a beaten level still counts before the new one loads.
void Game::jumpToLevel(int level)
{
	if(scene != NULL && scene->isCleared())
	{
		menu.unlockLevel(currentLevel + 1);
		saveLevelProgress();
	}
	delete scene;
	scene = NULL;
	startLevel(level);
}

// Own shader and a 1x1 white texture, so the overlay works over both the menu and a level
void Game::initFade()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!vShader.isCompiled() || !fShader.isCompiled())
		std::cout << "Fade Shader Error" << std::endl;
	fadeProgram.init();
	fadeProgram.addShader(vShader);
	fadeProgram.addShader(fShader);
	fadeProgram.link();
	if(!fadeProgram.isLinked())
		std::cout << "Fade Shader Linking Error" << std::endl << fadeProgram.log() << std::endl;
	fadeProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();

	const unsigned char white[4] = { 255, 255, 255, 255 };
	fadeTexture.loadFromRGBABuffer(white, 1, 1);
	fadeTexture.setMinFilter(GL_NEAREST);
	fadeTexture.setMagFilter(GL_NEAREST);
	fadeSprite = Sprite::createSprite(glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.f, 1.f),
	                                  &fadeTexture, &fadeProgram);
	fadeSprite->setNumberAnimations(1);
	fadeSprite->setAnimationSpeed(0, 1);
	fadeSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	fadeSprite->changeAnimation(0);
	fadeSprite->setPosition(glm::vec2(0.f, 0.f));
}

void Game::startFade(FadeAction action, int level)
{
	if(fadePhase != FADE_NONE)
		return;
	// The confirm sound plays on the key press, not when the level loads
	bool startsLevel = action == ACTION_START_LEVEL || action == ACTION_JUMP_TO_LEVEL ||
	                   (action == ACTION_LEVEL_CLEARED && currentLevel < MENU_NUM_LEVELS &&
	                    menu.isPlayable(currentLevel + 1));
	if(startsLevel)
		Audio::instance().play(Audio::MENU_SELECT);
	fadePhase = FADE_OUT;
	fadeTime = 0;
	fadeAction = action;
	fadeLevel = level;
}

// Runs at full black, so the old screen is never seen being torn down
void Game::runFadeAction()
{
	switch(fadeAction)
	{
	case ACTION_START_LEVEL:
		startLevel(fadeLevel);
		break;
	case ACTION_JUMP_TO_LEVEL:
		jumpToLevel(fadeLevel);
		break;
	case ACTION_LEVEL_CLEARED:
		levelCleared();
		break;
	case ACTION_BACK_TO_MENU:
		if(state == PLAYING)
			backToMenu();
		break;
	case ACTION_SHOW_FINAL_SCREEN:
		if(scene != NULL)
		{
			scene->showFinalScreen();
			Audio::instance().playMusic(Audio::MUSIC_WORLD_CLEAR, false);
		}
		break;
	}
}

void Game::updateFade(int deltaTime)
{
	fadeTime += std::min(deltaTime, FADE_MAX_STEP);
	if(fadeTime < FADE_TIME)
		return;
	if(fadePhase == FADE_OUT)
	{
		runFadeAction();
		fadePhase = FADE_IN;
		fadeTime = 0;
	}
	else
		fadePhase = FADE_NONE;
}

// Black quad over the logical screen; the letterbox bars are already black
void Game::renderFade()
{
	if(fadePhase == FADE_NONE || fadeSprite == NULL)
		return;
	float t = std::min(1.f, float(fadeTime) / FADE_TIME);
	float alpha = (fadePhase == FADE_OUT) ? t : 1.f - t;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	fadeProgram.use();
	fadeProgram.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	fadeProgram.setUniform4f("color", 0.f, 0.f, 0.f, alpha);
	fadeSprite->render();
	glDisable(GL_BLEND);
}



