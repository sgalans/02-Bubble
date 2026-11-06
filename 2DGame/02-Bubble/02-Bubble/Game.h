#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Menu.h"
#include "Sprite.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


// Game is a singleton (a class with a single instance) that represents our whole application


class Game
{

private:
	Game() { scene = NULL; currentLevel = 0; fadeSprite = NULL; fadePhase = FADE_NONE; }
	
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
	Player::PowerUps getStartingPowerUps(int level) const;
	int getStartingLives(int level) const { return entryLives[level - 1]; }

private:
	void startLevel(int level);
	void backToMenu();
	void levelCleared();
	void saveLevelProgress();
	void jumpToLevel(int level);

	// Screen transitions: fade to black, run the pending action, fade back in.
	// Every level switch goes through startFade instead of switching directly.
	enum FadePhase { FADE_NONE, FADE_OUT, FADE_IN };
	enum FadeAction { ACTION_START_LEVEL, ACTION_JUMP_TO_LEVEL, ACTION_LEVEL_CLEARED,
	                  ACTION_BACK_TO_MENU, ACTION_SHOW_FINAL_SCREEN };
	void initFade();
	void startFade(FadeAction action, int level = 0);
	void runFadeAction();
	void updateFade(int deltaTime);
	void renderFade();

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

	FadePhase fadePhase;
	int fadeTime; // ms into the current phase
	FadeAction fadeAction; // Runs when the screen is fully black
	int fadeLevel; // Level for the START/JUMP actions
	ShaderProgram fadeProgram;
	Texture fadeTexture; // 1x1 white, tinted black with the fade alpha
	Sprite *fadeSprite; // Covers the whole logical screen

};


#endif // _GAME_INCLUDE


