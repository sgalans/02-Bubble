#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"
#include "Game.h"


#define FONT_FILE "fonts/PressStart2P-Regular.ttf"
// The font is drawn on an 8 pixel grid, so multiples of 8 keep it crisp
#define TITLE_SIZE 32
#define ENTRY_SIZE 24
#define FOOTER_SIZE 16
#define TITLE_Y 60
#define FIRST_ENTRY_Y 170
#define ENTRY_SPACING 45
#define FOOTER_Y 430
#define BLINK_PERIOD 500


Menu::Menu()
{
	selected = 0;
	chosen = 0;
	currentTime = 0.0f;
}

Menu::~Menu()
{
	program.free();
}


void Menu::init()
{
	initShaders();
	titleText.init(FONT_FILE, TITLE_SIZE, program);
	entryText.init(FONT_FILE, ENTRY_SIZE, program);
	footerText.init(FONT_FILE, FOOTER_SIZE, program);

	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		ifstream fin(levelFile(i + 1).c_str());
		available[i] = fin.good();
	}
	// Start on the first level that can be played
	selected = 0;
	while(selected < MENU_NUM_LEVELS - 1 && !available[selected])
		selected++;
	chosen = 0;
	currentTime = 0.0f;
}

void Menu::update(int deltaTime)
{
	currentTime += deltaTime;
}

void Menu::render()
{
	program.use();
	program.setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	program.setUniformMatrix4f("modelview", glm::mat4(1.0f));
	program.setUniform2f("texCoordDispl", 0.f, 0.f);

	titleText.renderCentered("SELECT LEVEL", TITLE_Y, SCREEN_WIDTH, glm::vec4(1.f, 0.8f, 0.2f, 1.f));

	for(int i = 0; i < MENU_NUM_LEVELS; i++)
	{
		stringstream label;
		label << "LEVEL " << (i + 1);
		float y = float(FIRST_ENTRY_Y + i * ENTRY_SPACING);

		glm::vec4 color;
		if(!available[i])
			color = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
		else if(i == selected)
			color = glm::vec4(1.f, 1.f, 0.3f, 1.f);
		else
			color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		entryText.renderCentered(label.str(), y, SCREEN_WIDTH, color);

		// Blinking arrow to the left of the selected entry
		if(i == selected && int(currentTime / BLINK_PERIOD) % 2 == 0)
		{
			float x = (SCREEN_WIDTH - entryText.width(label.str())) / 2.f - entryText.width("> ");
			entryText.render(">", glm::vec2(x, y), color);
		}
	}

	footerText.renderCentered("ARROWS: MOVE  ENTER: PLAY  ESC: EXIT", FOOTER_Y, SCREEN_WIDTH,
	                          glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
}

void Menu::keyPressed(int key)
{
	switch(key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		moveSelection(-1);
		break;
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		moveSelection(1);
		break;
	case GLFW_KEY_ENTER:
	case GLFW_KEY_KP_ENTER:
	case GLFW_KEY_SPACE:
		if(available[selected])
			chosen = selected + 1;
		break;
	default:
		// Number keys jump straight into that level
		if(key >= GLFW_KEY_1 && key < GLFW_KEY_1 + MENU_NUM_LEVELS && available[key - GLFW_KEY_1])
		{
			selected = key - GLFW_KEY_1;
			chosen = selected + 1;
		}
		break;
	}
}

int Menu::takeChosenLevel()
{
	int level = chosen;
	chosen = 0;
	return level;
}

string Menu::levelFile(int level)
{
	stringstream file;
	file << "levels/level" << setw(2) << setfill('0') << level << ".txt";
	return file.str();
}

// Moves to the next available level in the given direction, wrapping around
void Menu::moveSelection(int step)
{
	for(int i = 1; i <= MENU_NUM_LEVELS; i++)
	{
		int candidate = ((selected + step * i) % MENU_NUM_LEVELS + MENU_NUM_LEVELS) % MENU_NUM_LEVELS;
		if(available[candidate])
		{
			selected = candidate;
			currentTime = 0.0f; // Restart the blink so the arrow shows right away
			return;
		}
	}
}

void Menu::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	program.init();
	program.addShader(vShader);
	program.addShader(fShader);
	program.link();
	if(!program.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << program.log() << endl << endl;
	}
	program.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}
