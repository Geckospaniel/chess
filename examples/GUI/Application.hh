#ifndef APPLICATION_HEADER
#define APPLICATION_HEADER

#include "Window.hh"

#include <vector>
#include <cstdio>

enum class WindowID;

class Application
{
public:
	Application();
	virtual ~Application();

	bool start();

protected:
	void addWindow(float divW, float divH);
	inline Window& window(WindowID id) { return windows[static_cast <size_t> (id)]; }

	virtual void onMouseClick(bool left, bool right);
	virtual void onMouseMove(WindowID id);
	virtual void onUpdate(double delta);
	virtual void onRender()=0;

	void capFPS(unsigned cap);
	bool keyPressed(const char* key);
	bool running = true;

private:
	size_t activeWindow;
	std::vector <Window> windows;

	bool started = false;
	double fpsCap = 0.0;
};

#endif
