#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Menu.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() { scene = NULL; currentLevel = 0; }
	
public:
	static Game &instance()
	{
		static Game G;
	
		return G;
	}
	
	void init();
	bool update(int deltaTime);
	void render();
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;

private:
	void startLevel(int level);
	void backToMenu();
	void levelCleared();
	void saveLevelProgress();

private:
	enum GameState { MENU, PLAYING };

	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST+1]; // Store key states so that 
							    // we can have access at any time
	GameState state;
	Menu menu;
	Scene *scene; // Only exists while a level is being played
	int currentLevel; // Level being played, 1..MENU_NUM_LEVELS
	Player::ItemCounts levelItems[MENU_NUM_LEVELS]; // Latest completed attempt of each level
	int entryLives[MENU_NUM_LEVELS]; // Lives at entry, independent of item recalculation

};


#endif // _GAME_INCLUDE


