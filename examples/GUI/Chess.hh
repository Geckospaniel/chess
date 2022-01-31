#ifndef CHESS_HEADER
#define CHESS_HEADER

#include "Application.hh"
#include "../../Engine.hh"

enum class WindowID
{
	Main
};

class Chess : public Application
{
public:
	Chess();
	~Chess();

	void onRender() override;
	void onMouseClick(bool left, bool right) override;

private:
	Vector2 <size_t> selected;
	Engine e;
};

#endif
