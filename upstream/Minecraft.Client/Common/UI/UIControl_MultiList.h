#pragma once

#include "UIControl_Base.h"
#include <vector>


class UIControl_MultiList : public UIControl_Base
{
protected:
	// iggy fast names for flash-side methods
	IggyName m_funcSetPossibleLabels;
	IggyName m_funcAddNewItem_Label;
	IggyName m_funcAddNewItem_Button;
	IggyName m_funcAddNewItem_MenuButton;
	IggyName m_funcAddNewItem_CheckBox;
	IggyName m_funcAddNewItem_Slider;
	IggyName m_funcAddNewItem_TextInput;
	IggyName m_funcSetCheckBox;
	IggyName m_funcGetCheckBox;
	IggyName m_funcSetSliderValue;
	IggyName m_funcGetSliderValue;
	IggyName m_funcSetItemLabel;
	IggyName m_funcHighlightItem;
	IggyName m_funcEnableItem;
	IggyName m_funcCheckElementExists;
	IggyName m_funcSetTouchFocus;
	IggyName m_removeAllItemsFunc;

	// stores caller-defined ids in insertion order (label = -1)
	// iggy uses positional index; getListIndex() converts id to index.
	std::vector<int> m_itemIds;
	int m_itemCount;
	int m_iCurrentSelection;

	std::unordered_map<int, int> m_sliderValues;

public:
	UIControl_MultiList();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(int id);
	void AddNewLabel(const wstring &label);
	void AddNewButton(const wstring &label, int id);
	void AddNewMenuButton(const wstring &label, int id);
	void AddNewCheckbox(const wstring &label, int id, bool checked);
	void AddNewSlider(const wstring &label, int id, int minVal, int maxVal, int step, int initialVal);
	void AddNewTextInput(const wstring &label, int id);
	bool GetCheckboxValue(int id);
	void SetCheckboxValue(int id, bool checked, bool bImmediate = true);
	int GetSliderValue(int id);
	void SetSliderValue(int id, int value, bool bImmediate = true);
	void SetSliderLabel(int id, const wstring &label, bool bImmediate = true);
	void HighlightItem(int id, bool animate = false);
	void EnableItem(int id, bool bEnable, bool bImmediate = true);
	void clearList();
	bool CheckElementExists(int id);
	int getCurrentSelection() const { return m_iCurrentSelection; }
	void updateChildFocus(int iChild) { m_iCurrentSelection = iChild; }
	int getIdFromIndex(int index) const;
	int getItemCount() const { return m_itemCount; }
	void SetTouchFocus(S32 iX, S32 iY, bool bRepeat);
	
	void handleSliderMove(int id, int newValue);

private:
	int getListIndex(int id) const;
};
