#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <string>
#include "ShaderProgram.h"
#include "Text.h"
#include "Sprite.h"


#define MENU_NUM_LEVELS 5



class Menu
{

public:
	Menu();
	~Menu();

	void init();
	void update(int deltaTime);
	void render();

	void keyPressed(int key);
	// Escape: goes back one screen. Returns false on the main screen, so the game can quit
	bool back();

	// Returns the chosen level (1..MENU_NUM_LEVELS) once, or 0 if none was chosen
	int takeChosenLevel();

	static std::string levelFile(int level);

	// Unlocks the given level (1..MENU_NUM_LEVELS) and selects it, so after
	// beating a level the next one is ready to play from the menu
	void unlockLevel(int level);
	// The level file exists and it has been unlocked
	bool isPlayable(int level) const;

private:
	enum Screen { MAIN, LEVELS, INSTRUCTIONS, CREDITS };

	void initShaders();
	void moveSelection(int step);
	void mainKeyPressed(int key);
	void levelsKeyPressed(int key);
	void renderMain();
	void renderLevels();
	void renderStartingStats();
	void renderTextScreen(const std::string &title, const std::string &body);
	void renderArrow(const std::string &label, float y, const glm::vec4 &color);
	void renderBackground();
	// Centred inside the right-hand column, clear of the character in the background
	void renderInColumn(Text &text, const std::string &str, float y, const glm::vec4 &color);
	void renderShadowed(Text &text, const std::string &str, const glm::vec2 &pos, const glm::vec4 &color);

private:
	ShaderProgram program;
	Text titleText, entryText, footerText;
	Texture itemTexture, heartTexture;
	Sprite *itemIcon, *heartIcon;
	Texture backgroundTexture;
	Sprite *backgroundSprite;
	bool available[MENU_NUM_LEVELS]; // The level file exists
	bool unlocked[MENU_NUM_LEVELS]; // Level 1 always, the rest once the previous one is beaten
	Screen screen;
	int mainSelected; // Entry highlighted on the main screen
	int selected, chosen;
	float currentTime;

};


#endif // _MENU_INCLUDE
