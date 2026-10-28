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
