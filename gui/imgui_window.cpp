#include "imgui_window.hpp"
#include "window_manager.hpp"

void ImWindow::Close()
{
	bool handled;
	OnClosing(handled);
	if (!handled)
	{
		Cleanup();
		initialized = false;
		WindowManager::Close(this);
	}
}

void ImWindow::Show()
{
	if (!initialized)
	{
		Init();
		initialized = true;
	}
}