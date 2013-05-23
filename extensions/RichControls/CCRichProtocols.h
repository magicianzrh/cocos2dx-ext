/****************************************************************************
 Copyright (c) 2013 Kevin Sun and RenRen Games

 email:happykevins@gmail.com
 http://wan.renren.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#ifndef __CC_RICHBASE_H__
#define __CC_RICHBASE_H__

#include "dfont/dfont_manager.h" // import dfont system
#include <cocos2d.h>
#include "ExtensionMacros.h"

#define CCRICH_DEBUG 0 // dump debug info

//
//	Rich Controls
//	- TODO: CCRichArea, CCRichScrollView...
//	- TODO: support CCLabelTTF, CCLabelAtlas, CCLabelBMF
//	- TODO: support CCSprite CCAnimation
//	- TODO: Touchable Element dispatch to script engine
//	- TODO: 处理中文部分标点不能在行首的情况
//	- TODO: 处理英文单词折行，处理css inline-style折行的属性
//

NS_CC_EXT_BEGIN;

#define RMAX(a, b) ( a > b ? a : b )
#define RMIN(a, b) ( a < b ? a : b )

// pos of rich elements
struct RPos
{
	short x;	// x axis offset
	short y;	// y axis offset

	RPos(): x(0), y(0) {}
	RPos(short _x, short _y): x(_x), y(_y) {}
	inline RPos add(const RPos& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	inline RPos sub(const RPos& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
};

// size of rich elements
struct RSize
{
	short w;	// width
	short h;	// height

	RSize(): w(0), h(0) {}
	RSize(short _w, short _h): w(_w), h(_h) {}
};

// rect of rich elements
struct RRect
{
	RPos	pos; // left top
	RSize	size;

	inline short min_x() const
	{
		return pos.x;
	}
	inline short max_x() const
	{
		return pos.x + size.w;
	}
	inline short min_y() const
	{
		return pos.y - size.h;
	}
	inline short max_y() const
	{
		return pos.y;
	}
	inline bool is_zero() const
	{
		return size.w == 0 && size.h == 0;
	}
	inline void extend(const RRect& other)
	{
		pos.x = RMIN(pos.x, other.pos.x);
		pos.y = RMAX(pos.y, other.pos.y);

		short tmp_max_x = RMAX( max_x(), other.max_x() );
		short tmp_min_y = RMIN( min_y(), other.min_y() );
		
		size.w = tmp_max_x - pos.x;
		size.h = -(tmp_min_y - pos.y);

		size.w = RMAX(size.w, 0);
		size.h = RMAX(size.h, 0);
	}
	inline void subtract(const RRect& other)
	{
		pos.x = RMAX(pos.x, other.pos.x);
		pos.y = RMIN(pos.y, other.pos.y);

		short tmp_max_x = RMAX(max_x(), other.max_x());
		short tmp_min_y = RMIN(min_y(), other.min_y());

		size.w = tmp_max_x - pos.x;
		size.h = -(tmp_min_y - pos.y);

		size.w = RMAX(size.w, 0);
		size.h = RMAX(size.h, 0);
	}
};

// size absolute || ratio
struct ROptSize
{
	short absolute;
	float ratio; 

	ROptSize()
		: absolute(0), ratio(.0f)
	{
	}
};

// margin
struct RMargin
{
	short top;
	short right;
	short bottom;
	short left;

	RMargin()
		: top(0), right(0), bottom(0), left(0)
	{
	}
};

// metrics of rich elements
struct RMetrics
{
	RRect rect;
	RPos advance;
};

// metrics of textures
struct RTexture
{
	RTexture() 
		:rect(), texture(NULL) 
	{
	}
	~RTexture() 
	{
		CC_SAFE_RELEASE(texture);
	}

	inline void setTexture(CCTexture2D* _texture)
	{
		texture = _texture;
		texture->retain();
	}

	inline CCTexture2D* getTexture()
	{
		return texture;
	}

public:
	RRect rect;
private:
	CCTexture2D* texture;
};

// Rich Canvas
struct RRichCanvas
{
	class IRichNode* root;
	RRect rect;
};

// element list
typedef std::vector<class IRichElement*> element_list_t;

//
// Rich-Element protocol
//	- represent a single unit for compositor
//
class IRichElement
{
public:
	/**
	 * external interface
	 */
	// for parser
	virtual bool parse(class IRichParser* parser, const char** attr = NULL) = 0;
	// for compositor
	virtual bool composit(class IRichCompositor* compositor) = 0;
	// for renderer
	virtual void render(RRichCanvas canvas) = 0;

	/**
	 * state stack control
	 */
	virtual bool pushMetricsState() = 0;
	virtual bool pushRenderState() = 0;


	/**
	 * position & metrics
	 */
	virtual RPos getLocalPosition() const = 0;		// local position
	virtual void setLocalPosition(RPos pos) = 0;
	virtual void setLocalPositionX(short x) = 0;
	virtual void setLocalPositionY(short y) = 0;

	virtual RPos getGlobalPosition() = 0;		// global position	
	
	virtual RMetrics* getMetrics() = 0;		// element metrics


	/**
	 * render properties
	 */
	virtual RTexture* getTexture() = 0;
	virtual bool isBatchedDrawable() = 0;
	virtual unsigned int getColor() = 0;
	virtual const char* getFontAlias() = 0;

	/**
	 * cached composit control
	 */
	virtual bool isCachedComposit() = 0;	// add to cache, wait until flush
	virtual bool canLinewrap() = 0;
	virtual bool isNewlineBefore() = 0;
	virtual bool isNewlineFollow() = 0;
	virtual short getBaseline() = 0;		// position of baseline, min y
	virtual bool needBaselineCorrect() = 0; // for line cached composit, TODO: according to alignment

	virtual void onCachedCompositBegin(class ICompositCache* cache, RPos& pen) = 0;
	virtual void onCachedCompositEnd(class ICompositCache* cache, RPos& pen) = 0;

	/**
	 * children & parent access
	 */
	virtual void addChildren(IRichElement* child) = 0;
	virtual element_list_t* getChildren() = 0;
	virtual void removeAllChildren() = 0;
	
	virtual void setParent(IRichElement* parent) = 0;
	virtual IRichElement* getParent() = 0;

	virtual int	getID() = 0;
	virtual IRichElement* findChildByID(int _id) = 0;
};

//
// Parser protocol
//	- parse the rich string
//	- string or file must utf-8 decoded
// 
class IRichParser
{
public:
	// parse a utf8 format string 
	virtual element_list_t* parseString(const char* utf8_str) = 0;

	// parse a utf8 file
	virtual element_list_t* parseFile(const char* filename) = 0;

	// get container
	virtual class IRichNode* getContainer() = 0;

	// if treat string as plain text, do not parse
	virtual bool isPlainMode() = 0;
	virtual void setPlainMode(bool on) = 0;
};

//
// Compositor State
//
struct RMetricsState
{
	//
	// Alignment
	// 
	enum EAlign
	{
		// Horizon
		e_left = 0,
		e_right = 1,
		e_center = 2,
		e_justify,	// not supported!
		e_char,		// not supported!
		// Vertical
		e_bottom = 0,
		e_top = 1,
		e_middle = 2,
		e_baseline // not supported
	};

	RRect zone;	// current composit zone

	short pen_x;
	short pen_y;
	
	ICompositCache* elements_cache;

	RMetricsState()
		: pen_x(0), pen_y(0), elements_cache(NULL)
	{
	}
};

struct RRenderState
{
	unsigned int color;
	const char* font_alias;

	RRenderState()
		: color(0xffffffff), font_alias(DFONT_DEFAULT_FONTALIAS)
	{

	}
};


//
// Compositor protocol
//	- know how to compositor the rich elements
//
class IRichCompositor
{
public:
	virtual bool composit(IRichElement* root) = 0;

	// get rect
	virtual const RRect& getRect() const = 0;

	// reset all state & cached data
	virtual void reset() = 0;

	// get current composit state
	virtual RMetricsState* getMetricsState() = 0;
	// return new top state
	virtual RMetricsState* pushMetricsState() = 0;
	// return popped state
	virtual void popMetricsState() = 0;
	// revert & return initial state
	virtual RMetricsState* initMetricsState(const RMetricsState* new_init_state = NULL) = 0;

	// get current composit state
	virtual RRenderState* getRenderState() = 0;
	// return new top state
	virtual RRenderState* pushRenderState() = 0;
	// return popped state
	virtual void popRenderState() = 0;
	// revert & return initial state
	virtual RRenderState* initRenderState(const RRenderState* new_init_state = NULL) = 0;

	// get current font
	virtual class dfont::FontCatalog* getFont() = 0;

	// get container
	virtual class IRichNode* getContainer() = 0;

	// get root cache
	virtual class ICompositCache* getRootCache() = 0;
};

//
// Cached Elements Protocol
//	- for combined elements composit
//
class ICompositCache
{
public:
	virtual void appendElement(IRichElement* ele) = 0;
	virtual RRect flush(class IRichCompositor* compositor) = 0;
	//virtual element_list_t* getCachedElements() = 0;
	virtual void clear() = 0;

	virtual RMetricsState::EAlign getHAlign() = 0;
	virtual RMetricsState::EAlign getVAlign() = 0;
	virtual void setHAlign(RMetricsState::EAlign align) = 0;
	virtual void setVAlign(RMetricsState::EAlign align) = 0;
	virtual void setWrapline(bool wrap) = 0;
	virtual bool isWrapline() = 0;
	virtual short getSpacing() = 0;
	virtual short getPadding() = 0;
	virtual void setSpacing(short v) = 0;
	virtual void setPadding(short v) = 0;
};

//
// Rich Atlas protocol
//	- know how to render batched elements
//
class IRichAtlas
{
public:
	virtual void appendRichElement(IRichElement* element) = 0;
	virtual void resetRichElements() = 0;
	virtual void updateRichRenderData() = 0;
};

//
// Rich Node protocol
//	- describes how a rich control work
//
class IRichNode
{
public:
	virtual IRichParser* getParser() = 0;
	virtual IRichCompositor* getCompositor() = 0;
	virtual RSize getActualSize() = 0;

	// default properties
	virtual RSize getPreferredSize() = 0;
	virtual void setPreferredSize(RSize size) = 0;

	// content string functions
	virtual void setStringUTF8(const char* utf8_str) = 0;
	virtual void appendStringUTF8(const char* utf8_str) = 0;
	virtual const char* getStringUTF8() = 0;

	// overlay utility
	virtual void addOverlay(IRichElement* overlay) = 0;
	virtual void addCCNode(class CCNode* node) = 0;

	// batch utility
	virtual IRichAtlas* findAtlas(class CCTexture2D* texture, unsigned int color_rgba) = 0;
};

// touchable event
typedef void (CCObject::*SEL_RichEleClickHandler)(
	IRichNode* root, IRichElement* ele, int _id);
typedef void (CCObject::*SEL_RichEleMoveHandler)(
	IRichNode* root, IRichElement* ele, int _id,
	CCPoint location, CCPoint delta);

#define richclicked_selector(_SELECTOR)	(SEL_RichEleClickHandler)(&_SELECTOR)
#define richmoved_selector(_SELECTOR)	(SEL_RichEleMoveHandler)(&_SELECTOR)

//@Deprecate
// transfer parse utilities functions
const unsigned short cc_rich_char_slash = '\\';

extern "C"
{
	int cc_transfer_oct_value(unsigned short c);
	int cc_transfer_hex_value(unsigned short c);

	// return consumed char num
	int cc_transfer_integer(unsigned short* start, unsigned short* end, int& integer);
	int cc_transfer_angle_brackets_content(unsigned short* start, unsigned short* end, std::string& content);
	bool cc_parse_rect(std::string& str, RRect& rect);
	bool cc_parse_image_content(const std::string& content, std::string& filename, RRect& tex_rect, RRect& composit_rect);
	//int cc_transfer_phrase(unsigned short* start, unsigned short* end, IRichElement*& element);
};

NS_CC_EXT_END;

#endif//__CC_RICHBASE_H__

