#pragma once

#include <XPLM/XPLMDisplay.h>
#include <XPLM/XPLMMenus.h>
#include <Widgets/XPWidgets.h>

#include "Logger.h"

namespace xp11_va {
	class UI
	{
	public:
		UI();
		~UI();

		XPLMWindowID windowId() const { return window_id; }
		XPLMMenuID menuId() const { return menu_id; }

	private:
		Logger& logger;
		LogCallback logger_callback;

		int menu_container_idx = -1;
		XPLMMenuID menu_id = nullptr;
		bool setupMenu() noexcept;

		XPLMWindowID window_id = nullptr;
		XPWidgetID listbox_id = nullptr;
		bool setupWindow() noexcept;
	};
}
