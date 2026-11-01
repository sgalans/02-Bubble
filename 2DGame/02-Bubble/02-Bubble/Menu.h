#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <string>
#include "ShaderProgram.h"
#include "Text.h"


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

	// Returns the chosen level (1..MENU_NUM_LEVELS) once, or 0 if none was chosen
	int takeChosenLevel();

	static std::string levelFile(int level);

	// Unlocks the given level (1..MENU_NUM_LEVELS) and selects it, so after
	// beating a level the next one is ready to play from the menu
	void unlockLevel(int level);
	// The level file exists and it has been unlocked
	bool isPlayable(int level) const;

private:
	void initShaders();
	void moveSelection(int step);

private:
	ShaderProgram program;
	Text titleText, entryText, footerText;
	bool available[MENU_NUM_LEVELS]; // The level file exists
	bool unlocked[MENU_NUM_LEVELS]; // Level 1 always, the rest once the previous one is beaten
	int selected, chosen;
	float currentTime;

};


#endif // _MENU_INCLUDE
