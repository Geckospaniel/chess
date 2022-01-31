#ifndef BIT_IMAGE_HEADER
#define BIT_IMAGE_HEADER

#include "Window.hh"
#include "../../Vector2.hh"

class BitImage
{
public:
	static void render(Window& win, const Vec2& position, Vec2 size, uint64_t val);

	static const int width = 7;
	static const int height = 9;
};

#endif
