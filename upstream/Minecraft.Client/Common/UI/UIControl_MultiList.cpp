#include "stdafx.h"
#include "UI.h"
#include "UIControl_MultiList.h"

// Multilist By aRockefeler Or aRockefort

UIControl_MultiList::UIControl_MultiList()
{
	m_itemCount = 0;
	m_iCurrentSelection = 0;
}

bool UIControl_MultiList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eButtonList); // reuses eButtonList, same as original
	bool success = UIControl_Base::setupControl(scene, parent, controlName);

	m_funcSetPossibleLabels     = registerFastName(L"SetPossibleLabels");     
	m_funcAddNewItem_Label      = registerFastName(L"addNewItem_Label");      
	m_funcAddNewItem_Button     = registerFastName(L"addNewItem_Button");     
	m_funcAddNewItem_MenuButton = registerFastName(L"addNewItem_MenuButton"); 
	m_funcAddNewItem_CheckBox   = registerFastName(L"addNewItem_CheckBox");   
	m_funcAddNewItem_Slider     = registerFastName(L"addNewItem_Slider");     
	m_funcAddNewItem_TextInput  = registerFastName(L"addNewItem_TextInput");  
	m_funcSetCheckBox           = registerFastName(L"SetCheckBox");           
	m_funcGetCheckBox           = registerFastName(L"GetCheckBox");           
	m_funcSetSliderValue        = registerFastName(L"SetSliderValue");        
	m_funcGetSliderValue        = registerFastName(L"GetSliderValue");        
	m_funcSetItemLabel          = registerFastName(L"SetItemLabel");          
	// these use the same pattern as UIControl_ButtonList
	m_funcHighlightItem         = registerFastName(L"HighlightItem");
	m_funcEnableItem            = registerFastName(L"EnableItem");
	m_funcCheckElementExists    = registerFastName(L"CheckElementExists");
	m_funcSetTouchFocus          = registerFastName(L"SetTouchFocus");
	m_removeAllItemsFunc         = registerFastName(L"removeAllItems");

	return success;
}

void UIControl_MultiList::init(int id)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (F64)id;
	IggyResult out = IggyPlayerCallMethodRS(
		m_parentScene->getMovie(),
		&result,
		getIggyValuePath(),
		m_initFunc,
		1,
		value);
}

// the internal element array
// Items are stored in insertion order so
// we walk m_itemIds to find the 0-based position of the given id
int UIControl_MultiList::getListIndex(int id) const
{
	for (size_t i = 0; i < m_itemIds.size(); ++i)
	{
		if (m_itemIds[i] == id)
			return (int)i;
	}
	return -1;
}

// returns the item id stored at the given position in m_itemIds or -1 if out of range
int UIControl_MultiList::getIdFromIndex(int index) const
{
	if (index >= 0 && index < (int)m_itemIds.size())
		return m_itemIds[index];
	return -1;
}

void UIControl_MultiList::AddNewLabel(const wstring &label)
{
	IggyDataValue result;
	IggyDataValue value[1];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type   = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcAddNewItem_Label, 1, value);

	m_itemIds.push_back(-1); 
	++m_itemCount;
}

void UIControl_MultiList::AddNewButton(const wstring &label, int id)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type     = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type   = IGGY_DATATYPE_number;
	value[1].number = (double)id;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcAddNewItem_Button, 2, value);

	m_itemIds.push_back(id);
	++m_itemCount;
}

void UIControl_MultiList::AddNewMenuButton(const wstring &label, int id)
{
	AddNewButton(label, id);
}

void UIControl_MultiList::AddNewCheckbox(const wstring &label, int id, bool checked)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type     = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type   = IGGY_DATATYPE_number;
	value[1].number = (double)id;

	value[2].type    = IGGY_DATATYPE_boolean;
	value[2].boolval = checked ? 1 : 0;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcAddNewItem_CheckBox, 3, value);

	m_itemIds.push_back(id);
	++m_itemCount;
}

void UIControl_MultiList::AddNewSlider(const wstring &label, int id, int minVal, int maxVal, int step, int initialVal)
{
	app.DebugPrintf("[MULTILIST] AddNewSlider: label=%ls id=%d min=%d max=%d step=%d init=%d\n",
		label.c_str(), id, minVal, maxVal, step, initialVal);
	IggyDataValue result;
	IggyDataValue value[5];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type     = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type   = IGGY_DATATYPE_number;
	value[1].number = (double)id;

	value[2].type   = IGGY_DATATYPE_number;
	value[2].number = (double)minVal;

	value[3].type   = IGGY_DATATYPE_number;
	value[3].number = (double)maxVal;

	value[4].type   = IGGY_DATATYPE_number;
	value[4].number = (double)initialVal;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcAddNewItem_Slider, 5, value);

	m_sliderValues.emplace(id, initialVal);
	m_itemIds.push_back(id);
	++m_itemCount;
}

void UIControl_MultiList::handleSliderMove(int id, int newValue) {
	auto it = m_sliderValues.find(id);

	if (it == m_sliderValues.end()) return;

	if (newValue != it->second) {
		int valueCount = 1;

		if (g_KBMInput.IsKBMActive() && g_KBMInput.HasAnyInput()) {
			if (newValue % 10 == 0) {
				ui.PlayUISFX(eSFX_Scroll);
				it->second = newValue;
			} else if (valueCount <= 20) {
				ui.PlayUISFX(eSFX_Scroll);
				it->second = newValue;
			}
		} else {
			ui.PlayUISFX(eSFX_Scroll);
			it->second = newValue;
		}

	}
}

void UIControl_MultiList::AddNewTextInput(const wstring &label, int id)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type     = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type   = IGGY_DATATYPE_number;
	value[1].number = (double)id;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcAddNewItem_TextInput, 2, value);

	m_itemIds.push_back(id);
	++m_itemCount;
}

bool UIControl_MultiList::GetCheckboxValue(int id)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)getListIndex(id);

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcGetCheckBox, 1, value);

	if (result.type == IGGY_DATATYPE_boolean)
		return result.boolval != 0;
	if (result.type == IGGY_DATATYPE_number)
		return (int)result.number == 1;
	return false;
}

void UIControl_MultiList::SetCheckboxValue(int id, bool checked, bool bImmediate)
{

	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)getListIndex(id);

	value[1].type    = IGGY_DATATYPE_boolean;
	value[1].boolval = checked ? 1 : 0;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcSetCheckBox, 2, value);
}

int UIControl_MultiList::GetSliderValue(int id)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)getListIndex(id);

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcGetSliderValue, 1, value);

	if (result.type == IGGY_DATATYPE_number)
		return (int)result.number;
	return 0;
}


void UIControl_MultiList::SetSliderValue(int id, int value, bool bImmediate)
{
	IggyDataValue result;
	IggyDataValue args[2];

	args[0].type   = IGGY_DATATYPE_number;
	args[0].number = (double)getListIndex(id);

	args[1].type   = IGGY_DATATYPE_number;
	args[1].number = (double)value;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcSetSliderValue, 2, args);
}

void UIControl_MultiList::SetSliderLabel(int id, const wstring &label, bool bImmediate)
{
	IggyDataValue result;
	IggyDataValue args[2];

	args[0].type   = IGGY_DATATYPE_number;
	args[0].number = (double)getListIndex(id);

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	args[1].type     = IGGY_DATATYPE_string_UTF16;
	args[1].string16 = stringVal;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcSetItemLabel, 2, args);
}

void UIControl_MultiList::HighlightItem(int id, bool animate)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)getListIndex(id);

	value[1].type    = IGGY_DATATYPE_boolean;
	value[1].boolval = animate ? 1 : 0;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcHighlightItem, 2, value);

	m_iCurrentSelection = id;
}

void UIControl_MultiList::EnableItem(int id, bool bEnable, bool bImmediate)
{
	if (!bImmediate)
		return; // deferred mode not implemented

	int listIndex = getListIndex(id);
	if (listIndex < 0)
		return;

	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)listIndex;

	value[1].type    = IGGY_DATATYPE_boolean;
	value[1].boolval = bEnable ? 1 : 0;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcEnableItem, 2, value);
}

void UIControl_MultiList::clearList()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_removeAllItemsFunc, 0, nullptr);

	m_itemIds.clear();
	m_sliderValues.clear();
	m_itemCount = 0;
	m_iCurrentSelection = 0;
}

bool UIControl_MultiList::CheckElementExists(int id)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type   = IGGY_DATATYPE_number;
	value[0].number = (double)id; 

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcCheckElementExists, 1, value);

	if (result.type == IGGY_DATATYPE_boolean)
		return result.boolval != 0;
	if (result.type == IGGY_DATATYPE_number)
		return (int)result.number != 0;
	return false;
}

void UIControl_MultiList::SetTouchFocus(S32 iX, S32 iY, bool bRepeat)
{
	IggyDataValue result;
	IggyDataValue value[3];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iX;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iY;
	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = bRepeat;

	IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
		getIggyValuePath(), m_funcSetTouchFocus, 3, value);
}
