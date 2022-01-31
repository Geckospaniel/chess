#include "Window.hh"
#include "../../Debug.hh"

Window::Window(float widthDivide, float heightDivide)
{
	DBG(SDL_Log("Creating a window"));

	SDL_DisplayMode display;
	if(SDL_GetCurrentDisplayMode(0, &display) != 0)
	{
		SDL_Log("Unable to get display information: %s", SDL_GetError());
		return;
	}

	int width = display.w / widthDivide;
	int height = display.h / heightDivide;
	DBG(SDL_Log("Window size %u %u", width, height));

	window = SDL_CreateWindow("",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);

	if(window == NULL)
	{
		SDL_Log("Unable to create a window: %s", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if(renderer == NULL)
	{
		SDL_Log("Unable to create a renderer: %s", SDL_GetError());
		return;
	}

	DBG(SDL_Log("Created window id %d", SDL_GetWindowID(window)));
	windowSize = Vec2(width, height);
}

Window::Window(Window&& rhs)
	: window(rhs.window), renderer(rhs.renderer)
{
	DBG(SDL_Log("Moving window id %d", SDL_GetWindowID(rhs.window)));
	rhs.window = NULL;
	rhs.renderer = NULL;	
}

Window::~Window()
{
	close();
}

bool Window::hasID(uint32_t id)
{
	return id == SDL_GetWindowID(window);
}

void Window::show(bool state)
{
	if(state) SDL_ShowWindow(window);
	else SDL_HideWindow(window);
}

bool Window::isOpen()
{
	return window != NULL && renderer != NULL;
}

void Window::setColor(int r, int g, int b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void Window::clear(int r, int g, int b)
{
	setColor(r, g, b);
	SDL_RenderClear(renderer);
}

void Window::render()
{
	SDL_RenderPresent(renderer);
}

void Window::drawBox(Vec2 position, Vec2 size, bool filled, int add)
{
	drawBoxPixel(position.x * windowSize.x, position.y * windowSize.y,
			size.x * windowSize.x + add, size.y * windowSize.y + add, filled);
}

void Window::drawBoxPixel(int x, int y, int w, int h, bool filled)
{
	SDL_Rect rect { x, y, w, h };
	if(filled) SDL_RenderFillRect(renderer, &rect);
	else SDL_RenderDrawRect(renderer, &rect);
}

void Window::drawLine(Vec2 position1, Vec2 position2)
{
	position1 *= windowSize;
	position2 *= windowSize;

	drawLinePixel(position1.x, position1.y, position2.x, position2.y);
}

void Window::drawLinePixel(int x1, int y1, int x2, int y2)
{
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Window::close()
{
	if(SDL_GetWindowID(window) > 0)
	{
		DBG(SDL_Log("Destroying window id %d", SDL_GetWindowID(window)));
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		window = NULL;
		renderer = NULL;
	}
}

void Window::updateSize()
{
	int width;
	int height;

	SDL_GetWindowSize(window, &width, &height);
	windowSize = Vec2(width, height);
}

void Window::updateMouse()
{
	int x;
	int y;

	SDL_GetMouseState(&x, &y);
	mouse = Vec2(x, y) / windowSize;
}

Vec2 Window::getMouse()
{
	return mouse;
}
