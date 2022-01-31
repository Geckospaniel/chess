#include "BitImage.hh"

void BitImage::render(Window& win, const Vec2& position, Vec2 size, uint64_t val)
{
	size /= Vec2(width, height);

	for(unsigned x = 0; x < width; x++)
	{
		for(unsigned y = 0; y < height; y++)
		{
			unsigned bit = y + (x * height);
			if((val >> bit) & 1UL)
			{
				win.drawBox(position + size * Vec2(x, y), size, true, 1);

				//win.setColor(255, 0, 0);
				//win.drawBox(position + size * Vec2(x, y), size, false, 1);
			}
		}
	}
}
