#pragma once

#include <Widgets/XPWidgets.h>
#include <stdlib.h>

#define LISTBOX_ITEM_HEIGHT 12

/************************************************************************
 * LISTBOX
 ************************************************************************
 *
 * This code helps do a listbox.  Since x-plane must be 
 * running to track the mouse, this listbox is effectively 
 * asynchronous and non-modal to the code...you call the function and 
 * some time later your callback is called.
 *
 * However, due to the way the listbox is structured, it will appear to
 * be somewhat modal to the user in that the next click after the listbox 
 * is called must belong to it.
 *
 */

/************************************************************************
 * LISTBOX SELECTION WIDGET
 ************************************************************************
 * 
 * This widget implements a standard pick-one-from-many-style selection menu 
 * button.  The text is taken from the current item.  The descriptor is
 * the items, newline-terminated.
 *
 * A message is sent whenever a new item is picked by the user.
 * 
 */

#define	xpWidgetClass_ListBox					10
 
enum {
	// This is the item number of the current item, starting at 0.
	xpProperty_ListBoxCurrentItem					= 1900,
	// This will add an item to the list box at the end.
	xpProperty_ListBoxAddItem						= 1901,
	// This will clear the list box and then add the items.
	xpProperty_ListBoxAddItemsWithClear				= 1902,
	// This will clear the list box.
	xpProperty_ListBoxClear							= 1903,
	// This will insert an item into the list box at the index.
	xpProperty_ListBoxInsertItem					= 1904,
	// This will delete an item from the list box at the index.
	xpProperty_ListBoxDeleteItem					= 1905,
	// This stores the pointer to the listbox data.
	xpProperty_ListBoxData							= 1906,
	// This stores the max Listbox Items.
	xpProperty_ListBoxMaxListBoxItems				= 1907,
	// This stores the highlight state.
	xpProperty_ListBoxHighlighted					= 1908,
	// This stores the scrollbar Min.
	xpProperty_ListBoxScrollBarMin					= 1909,
	// This stores the scrollbar Max.
	xpProperty_ListBoxScrollBarMax					= 1910,
	// This stores the scrollbar SliderPosition.
	xpProperty_ListBoxScrollBarSliderPosition		= 1911,
	// This stores the scrollbar ScrollBarPageAmount.
	xpProperty_ListBoxScrollBarPageAmount			= 1912
};

enum {
	// This message is sent when an item is picked.
	// param 1 is the widget that was picked, param 2
	// is the item number.
	xpMessage_ListBoxItemSelected				= 1900
};

// This structure represents a listbox internally...it consists of arrays
// per item and some general stuff.
struct	XPListBoxData_t {
	// Per item:
	std::vector<std::string>	Items;		// The name of the item
	std::vector<int>			Lefts;		// The rectangle of the item, relative to the top left corner of the listbox/
	std::vector<int>			Rights;
};

/*
 * XPCreateListBox
 *
 * This routine makes a listbox widget for you.  You must provide
 * a container for this widget, like a window for it to sit in.
 *
 */
XPWidgetID XPCreateListBox(
	int                  inLeft,
	int                  inTop,
	int                  inRight,
	int                  inBottom,
	int                  inVisible,
	const char*          inDescriptor,
	XPWidgetID           inContainer);

int XPListBoxProc(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2);

int XPListBoxGetItemNumber(XPListBoxData_t* pListBoxData, int inX, int inY);
void XPListBoxFillWithData(XPListBoxData_t* pListBoxData, const char* inItems, int Width);
void XPListBoxAddItem(XPWidgetID listboxId, const char* pBuffer);
void XPListBoxAddItem(XPListBoxData_t* pListBoxData, char* pBuffer, int Width);
void XPListBoxClear(XPListBoxData_t* pListBoxData);
void XPListBoxInsertItem(XPListBoxData_t* pListBoxData, char* pBuffer, int Width, int CurrentItem);
void XPListBoxDeleteItem(XPListBoxData_t* pListBoxData, int CurrentItem);

