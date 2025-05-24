#ifndef WINDOW_MANAGER_HPP
#define WINDOW_MANAGER_HPP

#include "imgui_window.hpp"
#include "pch/std.hpp"

class WindowManager
{
	friend class ImWindow;
	static std::vector<ImWindowPtr> vec;

	static void Close(ImWindow* window)
	{
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			if (it->get() == window)
			{
				vec.erase(it);
				break;
			}
		}
	}

public:
	template <typename T, class... TArgs>
	static ImWindowPtr Show(TArgs&& ... args)
	{
		auto ptr = std::make_shared<T>(std::forward<TArgs>(args)...);
		vec.push_back(ptr);
		vec->Show();
		return std::move(ptr);
	}

	static void Draw()
	{
		for (auto& win : vec)
		{
			win->Draw();
		}
	}
};

#endif