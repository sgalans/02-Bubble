#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "Audio.h"


void Game::init()
{
	bPlay = true;
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	Audio::instance().init();
	menu.init();
	state = MENU;
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
	if(key == GLFW_KEY_ESCAPE) // Escape code
	{
		// Escape leaves the level first, and quits only from the menu.
		// Leaving from the win screen still counts the level as beaten.
		if(state == PLAYING)
		{
			if(scene->isCleared())
				menu.unlockLevel(currentLevel + 1);
			backToMenu();
		}
		else
			bPlay = false;
		return;
	}
	// Enter on the win screen moves on to the next level
	if(state == PLAYING && scene->isCleared() && (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER))
	{
		levelCleared();
		return;
	}
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
	Audio::instance().play(Audio::MENU_SELECT);
	scene = new Scene();
	scene->init(Menu::levelFile(level));
	currentLevel = level;
	state = PLAYING;
}

void Game::backToMenu()
{
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
	delete scene;
	scene = NULL;
	if(next <= MENU_NUM_LEVELS && menu.isPlayable(next))
		startLevel(next);
	else
		state = MENU;
}



