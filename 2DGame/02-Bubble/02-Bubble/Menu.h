#ifndef _MENU_INCLUDE
#define _MENU_INCLUDE


#include <string>
#include "ShaderProgram.h"
#include "Text.h"


#define MENU_NUM_LEVELS 5


// Menu is the start screen: it lists the levels and lets the player pick one.
// Text is drawn with FreeType through the Text class.
// Levels whose file does not exist yet are shown greyed out and cannot be picked.


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

private:
	void initShaders();
	void moveSelection(int step);

private:
	ShaderProgram program;
	Text titleText, entryText, footerText;
	bool available[MENU_NUM_LEVELS];
	int selected, chosen;
	float currentTime;

};


#endif // _MENU_INCLUDE
