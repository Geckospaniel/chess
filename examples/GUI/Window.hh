#ifndef WINDOW_HEADER
#define WINDOW_HEADER

#include "../../Vector2.hh"

#include <SDL2/SDL.h>

class Window
{
public:
	Window(float widthDivide, float heightDivide);
	Window(Window&& rhs);
	~Window();

	void setColor(int r, int g, int b);
	void clear(int r, int g, int b);
	void render();

	void drawBoxPixel(int x, int y, int w, int h, bool filled = true);
	void drawBox(Vec2 position, Vec2 size, bool filled = true, int add = 0);

	void drawLinePixel(int x1, int y1, int x2, int y2);
	void drawLine(Vec2 position1, Vec2 position2);

	bool hasID(uint32_t id);
	void show(bool state);
	bool isOpen();
	void close();

	void updateSize();
	void updateMouse();
	Vec2 getMouse();

private:
	Vec2 windowSize;
	Vec2 mouse;

	SDL_Window* window;
	SDL_Renderer* renderer;
};

#endif
