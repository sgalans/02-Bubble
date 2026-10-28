#include <iostream>
#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Text.h"


// Empty columns between glyphs in the atlas so neighbours never bleed
#define ATLAS_PADDING 1


Text::Text()
{
	program = NULL;
	size = ascender = 0;
	for(int i = 0; i < TEXT_NUM_CHARS; i++)
		glyphs[i].sprite = NULL;
}

Text::~Text()
{
	for(int i = 0; i < TEXT_NUM_CHARS; i++)
		if(glyphs[i].sprite != NULL)
			delete glyphs[i].sprite;
}


// Rasterizes every printable ASCII character and packs them side by side in a
// single-row RGBA atlas: white, with the glyph coverage as alpha, so the
// shader's color uniform tints the text. FreeType is only needed here.
bool Text::init(const string &fontFile, int size, ShaderProgram &program)
{
	this->program = &program;
	this->size = size;

	FT_Library library;
	FT_Face face;
	if(FT_Init_FreeType(&library))
	{
		cout << "Could not init FreeType" << endl;
		return false;
	}
	if(FT_New_Face(library, fontFile.c_str(), 0, &face))
	{
		cout << "Could not load font " << fontFile << endl;
		FT_Done_FreeType(library);
		return false;
	}
	FT_Set_Pixel_Sizes(face, 0, size);
	ascender = int(face->size->metrics.ascender >> 6);

	// First pass: measure the atlas
	int atlasWidth = 0, atlasHeight = 1;
	for(int c = TEXT_FIRST_CHAR; c <= TEXT_LAST_CHAR; c++)
	{
		if(FT_Load_Char(face, c, FT_LOAD_RENDER))
			continue;
		atlasWidth += face->glyph->bitmap.width + ATLAS_PADDING;
		atlasHeight = glm::max(atlasHeight, int(face->glyph->bitmap.rows));
	}

	// Second pass: copy each bitmap into the atlas and remember where it went
	unsigned char *pixels = new unsigned char[atlasWidth * atlasHeight * 4];
	memset(pixels, 0, atlasWidth * atlasHeight * 4);
	vector<int> atlasX(TEXT_NUM_CHARS, 0);
	vector<glm::ivec2> bitmapSize(TEXT_NUM_CHARS, glm::ivec2(0));
	int x = 0;
	for(int c = TEXT_FIRST_CHAR; c <= TEXT_LAST_CHAR; c++)
	{
		Glyph &glyph = glyphs[c - TEXT_FIRST_CHAR];
		glyph.advance = 0;
		glyph.bearing = glm::ivec2(0);
		if(FT_Load_Char(face, c, FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot slot = face->glyph;
		glyph.advance = int(slot->advance.x >> 6);
		glyph.bearing = glm::ivec2(slot->bitmap_left, slot->bitmap_top);
		atlasX[c - TEXT_FIRST_CHAR] = x;
		bitmapSize[c - TEXT_FIRST_CHAR] = glm::ivec2(slot->bitmap.width, slot->bitmap.rows);
		for(int row = 0; row < int(slot->bitmap.rows); row++)
			for(int col = 0; col < int(slot->bitmap.width); col++)
			{
				unsigned char *p = &pixels[(row * atlasWidth + x + col) * 4];
				p[0] = p[1] = p[2] = 255;
				p[3] = slot->bitmap.buffer[row * slot->bitmap.pitch + col];
			}
		x += slot->bitmap.width + ATLAS_PADDING;
	}

	atlas.loadFromRGBABuffer(pixels, atlasWidth, atlasHeight);
	atlas.setMinFilter(GL_NEAREST);
	atlas.setMagFilter(GL_NEAREST);
	delete[] pixels;
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	// One quad per character, sized to its bitmap
	for(int i = 0; i < TEXT_NUM_CHARS; i++)
	{
		glm::ivec2 bmp = bitmapSize[i];
		if(bmp.x == 0 || bmp.y == 0)
			continue;
		glyphs[i].sprite = Sprite::createSprite(glm::vec2(bmp),
		                                        glm::vec2(float(bmp.x) / atlasWidth, float(bmp.y) / atlasHeight),
		                                        &atlas, &program);
		glyphs[i].sprite->setNumberAnimations(1);
		glyphs[i].sprite->addKeyframe(0, glm::vec2(float(atlasX[i]) / atlasWidth, 0.f));
		glyphs[i].sprite->changeAnimation(0);
	}
	return true;
}

void Text::render(const string &str, const glm::vec2 &pos, const glm::vec4 &color)
{
	program->setUniform4f("color", color.r, color.g, color.b, color.a);
	float penX = pos.x, baseline = pos.y + ascender;
	for(unsigned int i = 0; i < str.size(); i++)
	{
		int c = (unsigned char)str[i];
		if(c < TEXT_FIRST_CHAR || c > TEXT_LAST_CHAR)
			continue;
		const Glyph &glyph = glyphs[c - TEXT_FIRST_CHAR];
		if(glyph.sprite != NULL)
		{
			glyph.sprite->setPosition(glm::vec2(penX + glyph.bearing.x, baseline - glyph.bearing.y));
			glyph.sprite->render();
		}
		penX += glyph.advance;
	}
	program->setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
}

void Text::renderCentered(const string &str, float y, float screenWidth, const glm::vec4 &color)
{
	render(str, glm::vec2((screenWidth - width(str)) / 2.f, y), color);
}

int Text::width(const string &str) const
{
	int w = 0;
	for(unsigned int i = 0; i < str.size(); i++)
	{
		int c = (unsigned char)str[i];
		if(c >= TEXT_FIRST_CHAR && c <= TEXT_LAST_CHAR)
			w += glyphs[c - TEXT_FIRST_CHAR].advance;
	}
	return w;
}
