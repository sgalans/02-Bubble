#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include "Game.h"
#include "Audio.h"


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	Audio::instance().init();
	menu.init();
	state = MENU;
	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		levelItems[i] = Player::ItemCounts();
		entryLives[i] = 3;
	}
}

bool Game::update(int deltaTime)
{
	if(state == MENU)
	{
		menu.update(deltaTime);
		int level = menu.takeChosenLevel();
		if(level != 0)
			startLevel(level);
	}
	else
		scene->update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(state == MENU)
		menu.render();
	else
		scene->render();
}

void Game::keyPressed(int key)
{
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
		if(scene != NULL && scene->isCleared())
		{
			menu.unlockLevel(currentLevel + 1);
			saveLevelProgress();
		}
		delete scene;
		scene = NULL;
		startLevel(level);
		return;
	}
	if(key == GLFW_KEY_ESCAPE) // Escape code
	{
		// Escape leaves the level first, then steps back through the menu
		// screens, and quits only from the main menu.
		// Leaving from the win screen still counts the level as beaten.
		if(state == PLAYING)
		{
			backToMenu();
		}
		else if(!menu.back())
			bPlay = false;
		return;
	}
	// Enter on the win screen moves on to the next level
	if(state == PLAYING && scene->isCleared() && (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER))
	{
		levelCleared();
		return;
	}
	// Debug: K wipes out every enemy, which also opens the exit door
	if(state == PLAYING && key == GLFW_KEY_K)
		scene->killAllEnemies();
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
	Audio::instance().play(Audio::MENU_SELECT);
	scene = new Scene();
	scene->init(Menu::levelFile(level), levelMinutes[level - 1] * 60 * 1000);
	scene->setFinalLevel(level == MENU_NUM_LEVELS);
	scene->setPowerUps(getStartingPowerUps(level));
	scene->setLives(getStartingLives(level));
	currentLevel = level;
	state = PLAYING;
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
		state = MENU;
}

void Game::saveLevelProgress()
{
	// Replace this level's contribution; replays never add to an old result.
	levelItems[currentLevel - 1] = scene->getCollectedItems();
	if(currentLevel < MENU_NUM_LEVELS)
		entryLives[currentLevel] = scene->getLives();
}



