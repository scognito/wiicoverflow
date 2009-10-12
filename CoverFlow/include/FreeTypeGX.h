/* 
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled
 * FreeType parsable font into a GX texture for Wii homebrew development.
 * Copyright (C) 2008 Armin Tamzarian
 * 
 * This file is part of FreeTypeGX.
 * 
 * FreeTypeGX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FreeTypeGX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with FreeTypeGX.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \mainpage FreeTypeGX
 *
 * \section sec_intro Introduction
 * 
 * FreeTypeGX is a wrapper class for libFreeType which renders a compiled FreeType parsable font into a GX texture for Wii homebrew development.
 * <br>
 * FreeTypeGX is written in C++ and makes use of a selectable pre-buffered or buffer-on-demand methodology to allow fast and efficient printing of text to the EFB.  
 * <p>
 * This library was developed in-full by Armin Tamzarian with the support of developers in \#wiibrew on EFnet. 
 * 
 * \section sec_installation_source Installation (Source Code)
 * 
 * -# Ensure that you have the <a href = "http://www.tehskeen.com/forums/showthread.php?t=9404">libFreeType</a> Wii library installed in your development environment with the library added to your Makefile where appropriate.
 * -# Ensure that you have the <a href = "http://code.google.com/p/metaphrasis">Metaphrasis</a> library installed in your development environment with the library added to your Makefile where appropriate.
 * -# Extract the FreeTypeGX archive.
 * -# Copy the contents of the <i>src</i> directory into your project's development path.
 * -# Include the FreeTypeGX header file in your code using syntax such as the following:
 * \code
 * #include "FreeTypeGX.h"
 * \endcode
 * 
 * \section sec_installation_library Installation (Library)
 * 
 * -# Ensure that you have the <a href = "http://www.tehskeen.com/forums/showthread.php?t=9404">libFreeType</a> Wii library installed in your development environment with the library added to your Makefile where appropriate.
 * -# Ensure that you have the <a href = "http://code.google.com/p/metaphrasis">Metaphrasis</a> library installed in your development environment with the library added to your Makefile where appropriate.
 * -# Extract the FreeTypeGX archive.
 * -# Copy the contents of the <i>lib</i> directory into your <i>devKitPro/libogc</i> directory.
 * -# Include the FreeTypeGX header file in your code using syntax such as the following:
 * \code
 * #include "FreeTypeGX.h"
 * \endcode
 * 
 * \section sec_freetypegx_prerequisites FreeTypeGX Prerequisites
 * 
 * Before you begin using FreeTypeGX in your project you must ensure that the desired font in compiled into your project. For this example I will assume you are building your project with a Makefile using devKitPro evironment and are attempting to include a font whose filename is rursus_compact_mono.ttf. 
 *
 * -# Copy the font into a directory which will be processed by the project's Makefile. If you are unsure about where you should place your font just copy the it into your project's source directory.
 * \n\n
 * -# Modify the Makefile to convert the font into an object file:
 * \code
 * %.ttf.o : %.ttf
 * 	@echo $(notdir $<)
 * 	$(bin2o)
 * \endcode
 * \n
 * -# Include the font object's generated header file in your source code:
 * \code
 * #include "rursus_compact_mono_ttf.h"
 * \endcode
 * This header file defines the two variables that you will need for use within your project:
 * \code
 * extern const u8 rursus_compact_mono_ttf[];	A pointer to the font buffer within the compiled project.
 * extern const u32 rursus_compact_mono_ttf_size;	The size of the font's buffer in bytes.
 * \endcode
 * 
 * \section sec_freetypegx_usage FreeTypeGX Usage
 * 
 * -# Within the file you included the FreeTypeGX.h header create an instance object of the FreeTypeGX class:
 * \code
 * FreeTypeGX *freeTypeGX = new FreeTypeGX();
 * \endcode
 * Alternately you can specify a texture format to which you would like to render the font characters. Note that the default value for this parameter is GX_TF_RGBA8.
 * \code
 * FreeTypeGX *freeTypeGX = new FreeTypeGX(GX_TF_RGB565);
 * \endcode
 * Furthermore, you can also specify a positional format as defined in your graphics subsystem initialization. Note that the default value for this parameter is GX_POS_XYZ.
 * \code
 * FreeTypeGX *freeTypeGX = new FreeTypeGX(GX_TF_RGB565, GX_POS_XY);
 * \endcode
 * \n
 * Currently supported textures are:
 * \li <i>GX_TF_I4</i>
 * \li <i>GX_TF_I8</i>
 * \li <i>GX_TF_IA4</i>
 * \li <i>GX_TF_IA8</i>
 * \li <i>GX_TF_RGB565</i>
 * \li <i>GX_TF_RGB5A3</i>
 * \li <i>GX_TF_RGBA8</i>
 * 
 * \n
 * Currently supported position formats are:
 * \li <i>GX_POS_XY</i>
 * \li <i>GX_POS_XYZ</i>
 * 
 * \n
 * -# Using the allocated FreeTypeGX instance object call the loadFont function to load the font from the compiled buffer and specify the desired point size. Note that this function can be called multiple times to load a new:
 * \code
 * fontSystem->loadFont(rursus_compact_mono_ttf, rursus_compact_mono_ttf_size, 64);
 * \endcode
 * Alternately you can specify a flag which will load and cache all available font glyphs immidiately. Note that on large font sets enabling this feature could take a significant amount of time. 
 * \code
 * fontSystem->loadFont(rursus_compact_mono_ttf, rursus_compact_mono_ttf_size, 64, true);
 * \endcode
 * \n
 * -# Using the allocated FreeTypeGX instance object call the drawText function to print a string at the specified screen X and Y coordinates to the current EFB:
 * \code
 * freeTypeGX->drawText(10, 25, _TEXT("FreeTypeGX Rocks!"));
 * \endcode
 * Alternately you can specify a <i>GXColor</i> object you would like to apply to the printed characters:
 * \code
 * freeTypeGX->drawText(10, 25, _TEXT("FreeTypeGX Rocks!"),
 *                      (GXColor){0xff, 0xee, 0xaa, 0xff});
 * \endcode
 * Furthermore you can also specify a group of styling parameters which will modify the positioning or style of the text:
 * \code
 * freeTypeGX->drawText(10, 25, _TEXT("FreeTypeGX Rocks!"),
 *                      (GXColor){0xff, 0xee, 0xaa, 0xff},
 *                      FTGX_JUSTIFY_CENTER | FTGX_ALIGN_BOTTOM | FTGX_STYLE_UNDERLINE);
 * \endcode
 * \n
 * Currently style parameters are:
 * \li <i>FTGX_JUSTIFY_LEFT</i>
 * \li <i>FTGX_JUSTIFY_CENTER</i>
 * \li <i>FTGX_JUSTIFY_RIGHT</i>
 * \li <i>FTGX_ALIGN_TOP</i>
 * \li <i>FTGX_ALIGN_MIDDLE</i>
 * \li <i>FTGX_ALIGN_BOTTOM</i>
 * \li <i>FTGX_STYLE_UNDERLINE</i>
 * \li <i>FTGX_STYLE_STRIKE</i>
 * 
 * \section sec_license License
 * 
 * FreeTypeGX is distributed under the GNU Lesser General Public License.
 *    
 * \section sec_contact Contact
 * 
 * If you have any suggestions, questions, or comments regarding this library feel free to e-mail me at tamzarian1989 [at] gmail [dawt] com.
 */

#ifndef FREETYPEGX_H_
#define FREETYPEGX_H_

#include <gccore.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include <Metaphrasis.h>

#include <malloc.h>
#include <string.h>
#include <wchar.h>
#include <map>

/*! \struct ftgxCharData_
 * 
 * Font face character glyph relevant data structure.
 */
typedef struct ftgxCharData_ {
	uint16_t glyphAdvanceX;	/**< Character glyph X coordinate advance in pixels. */
	uint16_t glyphIndex;	/**< Charachter glyph index in the font face. */

	uint16_t textureWidth;	/**< Texture width in pixels/bytes. */
	uint16_t textureHeight;	/**< Texture glyph height in pixels/bytes. */

	uint16_t renderOffsetY;	/**< Texture Y axis bearing offset. */
	uint16_t renderOffsetMax;	/**< Texture Y axis bearing maximum value. */
	uint16_t renderOffsetMin;	/**< Texture Y axis bearing minimum value. */

	uint32_t* glyphDataTexture;	/**< Glyph texture bitmap data buffer. */
} ftgxCharData;

/*! \struct ftgxDataOffset_
 * 
 * Offset structure which hold both a maximum and minimum value.
 */
typedef struct ftgxDataOffset_ {
	uint16_t max;	/**< Maximum data offset. */
	uint16_t min;	/**< Minimum data offset. */
} ftgxDataOffset;

#define _TEXT(t) L ## t /**< Unicode helper macro. */

#define FTGX_NULL				0x0000
#define FTGX_JUSTIFY_LEFT		0x0001
#define FTGX_JUSTIFY_CENTER		0x0002
#define FTGX_JUSTIFY_RIGHT		0x0004

#define FTGX_ALIGN_TOP			0x0010
#define FTGX_ALIGN_MIDDLE		0x0020
#define FTGX_ALIGN_BOTTOM		0x0040

#define FTGX_STYLE_UNDERLINE	0x0100
#define FTGX_STYLE_STRIKE		0x0200

const GXColor ftgxWhite = (GXColor){0xff, 0xff, 0xff, 0xff}; /**< Constant color value used only to sanitize Doxygen documentation. */

/*! \class FreeTypeGX
 * \brief Wrapper class for the libFreeType library with GX rendering.
 * \author Armin Tamzarian
 * \version 0.2.3
 * 
 * FreeTypeGX acts as a wrapper class for the libFreeType library. It supports precaching of transformed glyph data into
 * a specified texture format. Rendering of the data to the EFB is accomplished through the application of high performance
 * GX texture functions resulting in high throughput of string rendering.  
 */
class FreeTypeGX {

	private:
		FT_Library ftLibrary;	/**< FreeType FT_Library instance. */
		FT_Face ftFace;			/**< FreeType reusable FT_Face typographic object. */
		FT_GlyphSlot ftSlot;	/**< FreeType reusable FT_GlyphSlot glyph container object. */
		FT_UInt ftPointSize;	/**< Requested size of the rendered font. */
		bool ftKerningEnabled;	/**< Flag indicating the availability of font kerning data. */
		
		uint8_t textureFormat;		/**< Defined texture format of the target EFB. */
		uint8_t positionFormat;		/**< Defined position format of the texture. */
		std::map<wchar_t, ftgxCharData> fontData; /**< Map which holds the glyph data structures for the corresponding characters. */

		static uint16_t adjustTextureWidth(uint16_t textureWidth, uint8_t textureFormat);
		static uint16_t adjustTextureHeight(uint16_t textureHeight, uint8_t textureFormat);

		static uint16_t getStyleOffsetWidth(uint16_t width, uint16_t format);
		static uint16_t getStyleOffsetHeight(ftgxDataOffset offset, uint16_t format);

		static void copyTextureToFramebuffer(GXTexObj *texObj, uint8_t positionFormat, uint16_t texWidth, uint16_t texHeight, int16_t screenX, int16_t screenY, GXColor color);
		static void copyFeatureToFramebuffer(uint8_t positionFormat, uint16_t featureWidth, uint16_t featureHeight, int16_t screenX, int16_t screenY, GXColor color);

		void unloadFont();
		ftgxCharData *cacheGlyphData(wchar_t charCode);
		uint16_t cacheGlyphDataComplete();
		void loadGlyphData(FT_Bitmap *bmp, ftgxCharData *charData);
		void drawTextFeature(uint16_t x, uint16_t y, uint16_t width, ftgxDataOffset offsetData, uint16_t format, GXColor color);
		
	public:
		FreeTypeGX(uint8_t textureFormat = GX_TF_RGBA8, uint8_t positionFormat = GX_POS_XYZ);
		~FreeTypeGX();

		static wchar_t* charToWideChar(char* p);

		uint16_t loadFont(uint8_t* fontBuffer, FT_Long bufferSize, FT_UInt pointSize, bool cacheAll = false);
		uint16_t loadFont(const uint8_t* fontBuffer, FT_Long bufferSize, FT_UInt pointSize, bool cacheAll = false);
		
		uint16_t drawText(uint16_t x, uint16_t y, wchar_t *text, GXColor color = ftgxWhite, uint16_t textStyling = FTGX_NULL);
		uint16_t drawText(uint16_t x, uint16_t y, wchar_t const *text, GXColor color = ftgxWhite, uint16_t textStyling = FTGX_NULL);

		uint16_t getWidth(wchar_t *text);
		uint16_t getWidth(wchar_t const *text);
		uint16_t getHeight(wchar_t *text);
		uint16_t getHeight(wchar_t const *text);
		ftgxDataOffset getOffset(wchar_t *text);
		ftgxDataOffset getOffset(wchar_t const *text);
		// CoverFloader needs
		uint16_t drawTextWithShadow(uint16_t x, uint16_t y, wchar_t *text, GXColor color = ftgxWhite, GXColor shadowColor = ftgxWhite, uint16_t textStyling = FTGX_NULL);
	
};

#endif /* FREETYPEGX_H_ */
