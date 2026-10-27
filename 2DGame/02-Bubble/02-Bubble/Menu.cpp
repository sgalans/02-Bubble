#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include "Menu.h"
#include "Game.h"


// Each glyph is 5x7 pixels, stored in a 6x8 cell so neighbours never bleed
#define GLYPH_W 5
#define GLYPH_H 7
#define CELL_W 6
#define CELL_H 8

#define TITLE_SCALE 5
#define ENTRY_SCALE 3
#define FOOTER_SCALE 2
#define TITLE_Y 60
#define FIRST_ENTRY_Y 170
#define ENTRY_SPACING 45
#define FOOTER_Y 430
#define BLINK_PERIOD 500


// Characters the font can draw, in atlas order. Anything else is drawn as a space.
static const char FONT_CHARS[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ>-.:!";

// One byte per row, top to bottom; bit 4 is the leftmost pixel
static const unsigned char FONT_ROWS[][GLYPH_H] =
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
	{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
	{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
	{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
	{0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
	{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
	{0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
	{0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
	{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
	{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
	{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
	{0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11}, // A
	{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
	{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
	{0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, // D
	{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
	{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // F
	{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
	{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
	{0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
	{0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
	{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
	{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
	{0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
	{0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // N
	{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
	{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
	{0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
	{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
	{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // S
	{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // V
	{0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, // W
	{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
	{0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}, // Y
	{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
	{0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, // >
	{0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // -
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}, // .
	{0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00}, // :
	{0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}  // !
};

static const int NUM_GLYPHS = sizeof(FONT_CHARS) - 1;


Menu::Menu()
{
	for(int i = 0; i < MENU_MAX_TEXT_SCALE; i++)
		glyph[i] = NULL;
	selected = 0;
	chosen = 0;
	currentTime = 0.0f;
}

Menu::~Menu()
{
	for(int i = 0; i < MENU_MAX_TEXT_SCALE; i++)
		if(glyph[i] != NULL)
			delete glyph[i];
	program.free();
}


void Menu::init()
{
	initShaders();
	initFont();

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

	renderTextCentered("SELECT LEVEL", TITLE_Y, TITLE_SCALE, glm::vec4(1.f, 0.8f, 0.2f, 1.f));

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
		renderTextCentered(label.str(), y, ENTRY_SCALE, color);

		// Blinking arrow to the left of the selected entry
		if(i == selected && int(currentTime / BLINK_PERIOD) % 2 == 0)
		{
			float textWidth = float(label.str().size() * CELL_W * ENTRY_SCALE);
			float x = (SCREEN_WIDTH - textWidth) / 2.f - 2 * CELL_W * ENTRY_SCALE;
			renderText(">", x, y, ENTRY_SCALE, color);
		}
	}

	renderTextCentered("UP DOWN: MOVE   ENTER: PLAY   ESC: EXIT", FOOTER_Y, FOOTER_SCALE,
	                   glm::vec4(0.8f, 0.8f, 0.8f, 1.f));
	program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
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

void Menu::renderText(const string &text, float x, float y, int scale, const glm::vec4 &color)
{
	Sprite *sprite = glyph[scale - 1];

	program.setUniform4f("color", color.r, color.g, color.b, color.a);
	for(unsigned int i = 0; i < text.size(); i++)
	{
		const char *found = strchr(FONT_CHARS, text[i]);
		int index = (found != NULL && text[i] != '\0') ? int(found - FONT_CHARS) : 0;
		if(index == 0)
			continue; // Spaces draw nothing
		sprite->changeAnimation(index);
		sprite->setPosition(glm::vec2(x + i * CELL_W * scale, y));
		sprite->render();
	}
}

void Menu::renderTextCentered(const string &text, float y, int scale, const glm::vec4 &color)
{
	float width = float(text.size() * CELL_W * scale - scale);
	renderText(text, (SCREEN_WIDTH - width) / 2.f, y, scale, color);
}

// Builds an RGBA atlas with every glyph side by side in a single row:
// white where the glyph has a pixel, transparent elsewhere, so the shader's
// color uniform tints the text.
void Menu::initFont()
{
	int width = NUM_GLYPHS * CELL_W;
	unsigned char *pixels = new unsigned char[width * CELL_H * 4];
	memset(pixels, 0, width * CELL_H * 4);

	for(int g = 0; g < NUM_GLYPHS; g++)
		for(int row = 0; row < GLYPH_H; row++)
			for(int col = 0; col < GLYPH_W; col++)
				if(FONT_ROWS[g][row] & (0x10 >> col))
				{
					unsigned char *p = &pixels[(row * width + g * CELL_W + col) * 4];
					p[0] = p[1] = p[2] = p[3] = 255;
				}

	fontTexture.loadFromRGBABuffer(pixels, width, CELL_H);
	fontTexture.setMinFilter(GL_NEAREST);
	fontTexture.setMagFilter(GL_NEAREST);
	delete[] pixels;

	glm::vec2 frameSize(float(GLYPH_W) / width, float(GLYPH_H) / CELL_H);
	for(int s = 0; s < MENU_MAX_TEXT_SCALE; s++)
	{
		int scale = s + 1;
		glyph[s] = Sprite::createSprite(glm::ivec2(GLYPH_W * scale, GLYPH_H * scale), frameSize, &fontTexture, &program);
		glyph[s]->setNumberAnimations(NUM_GLYPHS);
		for(int g = 0; g < NUM_GLYPHS; g++)
			glyph[s]->addKeyframe(g, glm::vec2(float(g * CELL_W) / width, 0.f));
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
