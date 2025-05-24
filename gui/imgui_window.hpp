#ifndef IMGUI_WINDOW_HPP
#define IMGUI_WINDOW_HPP

#include "imgui.h"
#include "pch/std.hpp"

class ImWindow
{
protected:
	bool shown = false;
	bool ended = false;
	bool initialized;

	virtual void Init() {}

	virtual void Cleanup() {}

	void End() { ImGui::End(); ended = true; }

	virtual void OnClosing(bool& handled) {}

	virtual void OnShown() {}

	virtual void DrawContent() = 0;

public:
	virtual ~ImWindow() = default;

	virtual const char* GetName() = 0;

	virtual void Draw()
	{
		ended = false;
		bool wasShown = shown;
		auto result = ImGui::Begin(GetName(), &shown);
		if (!result)
		{
			if (wasShown)
			{
				Close();
			}
			return;
		}

		DrawContent();

		if (!ended)
		{
			ImGui::End();
		}
	}

	void Close();

	void Show();
};

using ImWindowPtr = std::shared_ptr<ImWindow>;

#endif