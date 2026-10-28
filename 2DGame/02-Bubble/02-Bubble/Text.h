#ifndef _TEXT_INCLUDE
#define _TEXT_INCLUDE


#include <string>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Sprite.h"
#include "Texture.h"


#define TEXT_FIRST_CHAR 32  // space
#define TEXT_LAST_CHAR 126  // ~
#define TEXT_NUM_CHARS (TEXT_LAST_CHAR - TEXT_FIRST_CHAR + 1)


// Text draws strings with a TrueType font loaded through FreeType.
// init() rasterizes the printable ASCII characters at one pixel size into a
// single texture, so each Text object is one font at one size.
// Positions are in the current projection's units (screen pixels for the HUD/menu).


class Text
{

public:
	Text();
	~Text();

	bool init(const std::string &fontFile, int size, ShaderProgram &program);

	// pos is the top-left corner of the line
	void render(const std::string &str, const glm::vec2 &pos, const glm::vec4 &color);
	void renderCentered(const std::string &str, float y, float screenWidth, const glm::vec4 &color);
	int width(const std::string &str) const;
	int getSize() const { return size; }

private:
	struct Glyph
	{
		Sprite *sprite; // NULL for characters with no pixels, like space
		glm::ivec2 bearing; // Offset from the pen position (x) and baseline (y) to the bitmap's top-left
		int advance;
	};

	ShaderProgram *program;
	Texture atlas;
	Glyph glyphs[TEXT_NUM_CHARS];
	int size, ascender;

};


#endif // _TEXT_INCLUDE
