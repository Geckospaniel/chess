#include "Application.hh"
#include "../../Debug.hh"

#include <SDL2/SDL.h>

Application::Application()
{
	DBG(SDL_Log("Initializing SDL"));
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return;
	}
}

Application::~Application()
{
	DBG(SDL_Log("Quitting SDL"));
	SDL_Quit();
}

bool Application::start()
{
	started = true;
	for(auto& window : windows)
		window.show(true);

	uint64_t lastTicks = SDL_GetTicks();

	while(running)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_WINDOWEVENT:
					for(size_t i = 0; i < windows.size(); i++)
					{
						if(windows[i].hasID(event.window.windowID))
						{
							activeWindow = i;
							break;
						}
					}

					switch(event.window.event)
					{
						case SDL_WINDOWEVENT_CLOSE:
						{
							DBG(SDL_Log("Closing window %lu", activeWindow));
							windows[activeWindow].close();

							//	FIXME multiple windows are supported but for some reason it's never actually removed
							//	FIXME set running to false only if all windows are closed

							running = false;
							break;
						}

						case SDL_WINDOWEVENT_RESIZED:
						{
							DBG(SDL_Log("Resize window %lu", activeWindow));
							windows[activeWindow].updateSize();
							break;
						}

					}
				break;

				case SDL_MOUSEMOTION:
				{
					windows[activeWindow].updateMouse();
					onMouseMove(static_cast <WindowID> (activeWindow));
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					bool left = event.button.button == SDL_BUTTON_LEFT;
					bool right = event.button.button == SDL_BUTTON_RIGHT;

					onMouseClick(left, right);
					break;
				}

				case SDL_QUIT: running = false; break;
			}
		}

		uint32_t ticks = SDL_GetTicks();
		double delta = (double)(ticks - lastTicks) / 1000.0f;
		lastTicks = ticks;

		if(fpsCap > 0)
		{
			SDL_Delay(fpsCap * 1000);
			delta = fpsCap;
		}

		onUpdate(delta);
		onRender();
	}
	
	return true;
}

void Application::addWindow(float divW, float divH)
{
	windows.emplace_back(divW, divH);

	if(!windows.back().isOpen())
	{
		running = false;
		return;
	}

	if(started) windows.back().show(true);
}

void Application::capFPS(unsigned cap)
{
	DBG(SDL_Log("Capped FPS to %u", cap));
	fpsCap = 1.0 / cap;
}

bool Application::keyPressed(const char* key)
{
	const Uint8* state = SDL_GetKeyboardState(NULL);
	return state[SDL_GetScancodeFromKey(SDL_GetKeyFromName(key))];
}

void Application::onMouseMove(WindowID) {}
void Application::onUpdate(double) {}
void Application::onMouseClick(bool, bool) {}
