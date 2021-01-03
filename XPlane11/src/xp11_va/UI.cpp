#include "pch.h"
#include "UI.h"
#include "widgets/ListBox.h"

#include <XPLM/XPLMGraphics.h>
#include <Widgets/XPStandardWidgets.h>

static const char* plugin_name = "XP11/VoiceAttack Connector";
static const char* item_name = "Show Log";

namespace xp11_va {
	void menu_handler(void*, void*);
	void draw_window(XPLMWindowID, void*);
	int mouse_handler(XPLMWindowID, int, int, XPLMMouseStatus, void*);
	XPLMCursorStatus cursor_handler(XPLMWindowID, int, int, void*);
	int wheel_handler(XPLMWindowID, int, int, int, int, void*);
	void key_handler(XPLMWindowID, char, XPLMKeyFlags, char, void*, int);

	UI::UI() : logger(Logger::get()) {
		setupMenu() && setupWindow();
	}

	UI::~UI() {
		if (nullptr != menu_id) {
			XPLMDestroyMenu(menu_id);
		}

		if (nullptr != window_id) {
			XPLMDestroyWindow(window_id);
		}
	}

	bool UI::setupMenu() noexcept {
		menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), plugin_name, 0, 0);
		menu_id = XPLMCreateMenu(plugin_name, XPLMFindPluginsMenu(), menu_container_idx, menu_handler, this);
		if (nullptr == menu_id) { return false; }

		XPLMAppendMenuItem(menu_id, item_name, (void*)"show_log", 1);

		return true;
	}

	bool UI::setupWindow() noexcept {
		XPLMCreateWindow_t params;
		params.structSize = sizeof(XPLMCreateWindow_t);
		params.structSize = sizeof(params);
		params.visible = 0;
		params.drawWindowFunc = draw_window;
		params.handleMouseClickFunc = mouse_handler;
		params.handleRightClickFunc = mouse_handler;
		params.handleMouseWheelFunc = wheel_handler;
		params.handleKeyFunc = key_handler;
		params.handleCursorFunc = cursor_handler;
		params.refcon = this;
		params.layer = xplm_WindowLayerFloatingWindows;
		// Opt-in to styling our window like an X-Plane 11 native window
		// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
#ifdef XPLM301
		params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
#endif

		int left, bottom, right, top, width, height;
		XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);

		int center_x = (right - left) / 2;
		int center_y = (top - bottom) / 2;

		width = (right - left) / 4;
		height = (top - bottom) / 2;

		params.left = center_x - width / 2;
		params.bottom = center_y - height / 2;
		params.right = params.left + width;
		params.top = params.bottom + height;

		window_id = XPLMCreateWindowEx(&params);
		if (nullptr == window_id) { return false; }

		XPLMSetWindowPositioningMode(window_id, xplm_WindowPositionFree, -1);
		XPLMSetWindowResizingLimits(window_id, 200, 200, width, height);
		XPLMSetWindowTitle(window_id, plugin_name);

		XPLMGetWindowGeometry(window_id, &left, &top, &right, &bottom);

		listbox_id = XPCreateListBox(left, top, right, bottom, 1, nullptr, window_id);

		return true;
	}

	void menu_handler(void* in_menu_ref, void* in_item_ref) {
		Logger::get().Trace("menu_handler called");
		if (strcmp("show_log", (const char*)in_item_ref) == 0) {
			UI* ui = (UI*)in_menu_ref;
			XPLMSetWindowIsVisible(ui->windowId(), 1);
		}
	}

	void draw_window(XPLMWindowID window_id, void* in_refcon) {
		int left, top, right, bottom;

		XPLMSetGraphicsState(0, 0, 0, 0, 1, 1, 0);
		XPLMGetWindowGeometry(window_id, &left, &top, &right, &bottom);
		
	}

	int mouse_handler(XPLMWindowID, int, int, XPLMMouseStatus, void*) {
		return 0;
	}

	XPLMCursorStatus cursor_handler(XPLMWindowID, int, int, void*) {
		return xplm_CursorDefault;
	}

	int wheel_handler(XPLMWindowID, int, int, int, int, void*) {
		return 0;
	}

	void key_handler(XPLMWindowID, char, XPLMKeyFlags, char, void*, int) {

	}
}
