#include "cbase.h"
#include "ienginevgui.h"
#include "input.h"
#include <vgui_controls/Panel.h>
#include "view.h"
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "VGuiMatSurface/IMatSystemSurface.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IPanel.h>
#include <KeyValues.h>
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include <vgui/ILocalize.h>
#include "filesystem.h"
#include "../common/xbox/xboxstubs.h"
#include "steam/steam_api.h"
#include "cdll_client_int.h"
#include "vgui_basepanel.h"
#include <vgui_controls/QueryBox.h>
#include "vgui_controls/PropertyPage.h"
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/MessageDialog.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Menu.h>
#include "c_ai_basenpc.h"
#include "c_baseplayer.h"

#define BORDER_SPACING 8

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


/// <summary>
/// Spawn menu button data
/// </summary>
class SpawnMenuData_t
{
public:
	SpawnMenuData_t();

	void Parse(KeyValues* pKeyValuesData);

	char szName[64];
	char szEntity[64];
	char szIcon[64];
	char szCommand[64];
};

SpawnMenuData_t::SpawnMenuData_t()
{
	szName[0] = 0;
	szEntity[0] = 0;
	szIcon[0] = 0;
	szCommand[0] = 0;

}
void SpawnMenuData_t::Parse(KeyValues* pKeyValuesData)
{
	Q_strncpy(szName, pKeyValuesData->GetString("name"), 64);
	Q_strncpy(szIcon, pKeyValuesData->GetString("icon"), 64);
	Q_strncpy(szEntity, pKeyValuesData->GetString("ent"), 64);
	Q_strncpy(szCommand, pKeyValuesData->GetString("command"), 64);
}

class SpawnMenuButtonButton : public Button
{
	DECLARE_CLASS_SIMPLE(SpawnMenuButtonButton, Button);
public:
	SpawnMenuButtonButton(Panel* parent, const char* panelName, const char* buttonText) : Button(parent, panelName, buttonText)
	{
		m_pParent = parent;
		m_pContextMenu = nullptr;
	}
	virtual void OnMousePressed(MouseCode code)
	{
		if (m_bWeaponButton)
		{
			if (code == MOUSE_RIGHT)
			{
				OpenContextMenu();
			}
		}

		BaseClass::OnMousePressed(code);
	}
	void OpenContextMenu()
	{
		if (!m_pContextMenu)
		{
			m_pContextMenu = new Menu(this, "ContextMenu");
			m_pContextMenu->AddMenuItem("Option1", "Spawn In Place", "ContextInPlace", m_pParent);
			m_pContextMenu->AddMenuItem("Option2", "Give To Me", "ContextGive", m_pParent);
		}

		int x, y;
		vgui::input()->GetCursorPosition(x, y);
		m_pContextMenu->SetPos(x, y);
		m_pContextMenu->SetVisible(true);
	}
	bool m_bWeaponButton = false;
private:
	Menu* m_pContextMenu;
	Panel* m_pParent;
};
/// <summary>
/// Spawn menu button template
/// </summary>
///
class SpawnMenuButton : public Panel
{
	DECLARE_CLASS_SIMPLE(SpawnMenuButton, Panel);

public:
	SpawnMenuButton(Panel* parent, const char* text);
	SpawnMenuData_t* data;
	ImagePanel* m_pImage;
	SpawnMenuButtonButton* m_pButton;
private:
	virtual void OnCommand(const char* command);
	void HandleButtonCommand(const char* command);
};


SpawnMenuButton::SpawnMenuButton(Panel* parent, const char* text) : BaseClass(parent, "")
{
	SetSize(128, 160);
	m_pImage = new ImagePanel(this, "");
	m_pImage->SetSize(GetWide()-8, GetWide()-8);
	m_pImage->SetImage("spawnmenu/default");
	m_pImage->SetShouldScaleImage(true);
	m_pImage->SetPos(4, 4);
	m_pButton = new SpawnMenuButtonButton(this, text, text);
	m_pButton->SetContentAlignment(m_pButton->a_south);
	m_pButton->SetSize(GetWide(), GetTall());
	m_pButton->SetCommand("SpawnEntity");
	m_pButton->AddActionSignalTarget(this);
}

void SpawnMenuButton::OnCommand(const char* command)
{
	if (!Q_strcmp(command, "SpawnEntity"))
	{
		const char* cmd = data->szCommand;
		HandleButtonCommand(cmd);
	}
	if (!Q_strcmp(command, "ContextInPlace"))
	{
		HandleButtonCommand("SpawnEntity");
	}
	if (!Q_strcmp(command, "ContextGive"))
	{
		HandleButtonCommand("give");
	}
}

void SpawnMenuButton::HandleButtonCommand(const char* command)
{
	char finalcmd[128];
	if (!Q_strcmp(command, "SpawnNPC"))
	{
		strcpy(finalcmd, command);
		strcat(finalcmd, " ");
		strcat(finalcmd, data->szEntity);
	}
	else if (!Q_strcmp(command, "SpawnNPCWithWeapon"))
	{
		strcpy(finalcmd, command);
		strcat(finalcmd, " ");
		strcat(finalcmd, data->szEntity);
	}
	else if (!Q_strcmp(command, "SpawnEntity"))
	{
		strcpy(finalcmd, command);
		strcat(finalcmd, " ");
		strcat(finalcmd, data->szEntity);
	}
	else if (!Q_strcmp(command, "give"))
	{
		strcpy(finalcmd, command);
		strcat(finalcmd, " ");
		strcat(finalcmd, data->szEntity);
	}
	else
	{
		strcpy(finalcmd, command);
	}
	const char* finalout = finalcmd;
	engine->ServerCmd(finalout);

}
/// <summary>
/// Spawn menu for items
/// </summary>
class SpawnMenuItems : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(SpawnMenuItems, PropertyPage);
public:
	SpawnMenuItems(Panel* parent);
	void PopulateButtons();
	void PositionButtons();
private:
	CUtlVector<SpawnMenuButton*> v_items;
};

SpawnMenuItems::SpawnMenuItems(Panel* parent) : BaseClass(this, "Items")
{
	LoadControlSettings("resource/ui/spawnmenuitems.res");
	PopulateButtons();
	PositionButtons();
}

void SpawnMenuItems::PopulateButtons()
{
	KeyValues* kvRoot = new KeyValues("root");
	if (!kvRoot->LoadFromFile(filesystem, "scripts/spawnmenu_items.txt", "MOD"))
	{
		kvRoot->deleteThis();
	}

	KeyValues* pKV1 = kvRoot->FindKey("items");

	FOR_EACH_SUBKEY(pKV1, pKey)
	{
		SpawnMenuData_t* data = new SpawnMenuData_t;
		data->Parse(pKey);
		SpawnMenuButton* pButton = new SpawnMenuButton(this, data->szName);
		pButton->data = data;
		v_items.AddToTail(pButton);
	}
}

void SpawnMenuItems::PositionButtons()
{
	int row = 0;
	int indx = 0;

	InvalidateLayout();

	for (int i = 0; i < v_items.Count(); i++)
	{
		SpawnMenuButton* button = NULL;
		button = v_items.Element(i);
		button->m_pImage->SetImage(button->data->szIcon);

		int xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
		int ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		if ((xpos + button->GetWide() + BORDER_SPACING) > GetWide())
		{
			row++;
			indx = 0;
			xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
			ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		}
		button->SetPos(xpos, ypos);
		DevMsg("data button %s\n", button->data->szName);
		indx++;
	}
}


/// <summary>
/// Spawn menu for weapons
/// </summary>
class SpawnMenuWeapons : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(SpawnMenuWeapons, PropertyPage);
public:
	SpawnMenuWeapons(Panel* parent);
	void PopulateButtons();
	void PositionButtons();
private:
	CUtlVector<SpawnMenuButton*> v_weapons;
	CUtlVector<SpawnMenuButton*> v_ammo;
};

SpawnMenuWeapons::SpawnMenuWeapons(Panel* parent) : BaseClass(this, "Weapons")
{
	LoadControlSettings("resource/ui/spawnmenuweapons.res");
	PopulateButtons();
	PositionButtons();
}

void SpawnMenuWeapons::PopulateButtons()
{
	KeyValues* kvRoot = new KeyValues("root");
	if (!kvRoot->LoadFromFile(filesystem,"scripts/spawnmenu_weapons.txt","MOD"))
	{
		kvRoot->deleteThis();
	}

	KeyValues* pKV1 = kvRoot->FindKey("weapons");

	FOR_EACH_SUBKEY(pKV1, pKey)
	{
		SpawnMenuData_t* data = new SpawnMenuData_t;
		data->Parse(pKey);
		SpawnMenuButton* pButton = new SpawnMenuButton(this, data->szName);
		pButton->data = data;
		pButton->m_pButton->m_bWeaponButton = true;
		v_weapons.AddToTail(pButton);
	}
}

void SpawnMenuWeapons::PositionButtons()
{
	int row = 0;
	int indx = 0;

	InvalidateLayout();
	
	for (int i = 0; i < v_weapons.Count(); i++)
	{
		SpawnMenuButton* button = NULL;
		button = v_weapons.Element(i);
		button->m_pImage->SetImage(button->data->szIcon);
	
		int xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
		int ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		if ((xpos + button->GetWide() + BORDER_SPACING) > GetWide())
		{
			row++;
			indx = 0;
			xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
			ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		}
		button->SetPos(xpos, ypos);
		DevMsg("data button %s\n", button->data->szName);
		indx++;
	}
}
/// <summary>
/// Spawn menu for entities
/// </summary>
class SpawnMenuEntities : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(SpawnMenuEntities, PropertyPage);
public:
	SpawnMenuEntities(Panel* parent);
	void PopulateButtons();
	void PositionButtons();
	KeyValues* kvRoot;
private:
	CUtlVector<SpawnMenuButton*> v_buttons;
};

SpawnMenuEntities::SpawnMenuEntities(Panel* parent) : BaseClass(this, "Entities")
{
	LoadControlSettings("resource/ui/spawnmenuents.res");
	PopulateButtons();
	PositionButtons();
	
}

void SpawnMenuEntities::PopulateButtons()
{
	InvalidateLayout();
	kvRoot = new KeyValues("root");
	if (!kvRoot->LoadFromFile(filesystem, "scripts/spawnmenu_entities.txt", "MOD"))
	{
		kvRoot->deleteThis();
		Warning("Failed to load txt file\n");
	}
	KeyValues* pKV = kvRoot->FindKey("entities");

	FOR_EACH_SUBKEY(pKV, pKey)
	{
		SpawnMenuData_t* data = new SpawnMenuData_t;
		data->Parse(pKey);
		SpawnMenuButton* button = new SpawnMenuButton(this, data->szName);
		button->data = data;
		v_buttons.AddToTail(button);
	}
}

void SpawnMenuEntities::PositionButtons()
{
	int row = 0;
	int indx = 0;
	for (int i = 0; i < v_buttons.Count(); i++)
	{
		SpawnMenuButton* button = NULL;
		button = v_buttons.Element(i);
		if(button->data->szIcon)
			button->m_pImage->SetImage(button->data->szIcon);
		int xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
		int ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		if ((xpos + button->GetWide() + BORDER_SPACING) > GetWide())
		{
			row++;
			indx = 0;
			xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
			ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		}
		button->SetPos(xpos, ypos);
		DevMsg("data button %s x %i y %i\n", button->data->szName,xpos,ypos);
		indx++;
	}
}
/// <summary>
/// Spawn menu for npcs
/// </summary>
class SpawnMenuNPC : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(SpawnMenuNPC, PropertyPage);
public:
	SpawnMenuNPC(Panel* parent);
	void Init();
	void PopulateButtons();
	void PositionButtons();
	virtual void OnTick();

	virtual void OnDataChanged();

	KeyValues* kvRoot;
private:
	CUtlVector<SpawnMenuButton*> v_buttons;
	ComboBox* weapon_combobox;
};

SpawnMenuNPC::SpawnMenuNPC(Panel* parent) : BaseClass(this, "NPCs")
{
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
	weapon_combobox = new ComboBox(this, "WeaponsCombo", 4, false);
	weapon_combobox->AddItem("Pistol", NULL);
	weapon_combobox->AddItem("Shotgun", NULL);
	weapon_combobox->AddItem("Plasma Rifle", NULL);
	weapon_combobox->AddItem("Grenade Launcher", NULL);
	weapon_combobox->SetKeyBoardInputEnabled(false);
	LoadControlSettings("resource/ui/spawnmenunpc.res");
	PopulateButtons();
	PositionButtons();	
}

void SpawnMenuNPC::OnTick()
{
	OnDataChanged();
}
void SpawnMenuNPC::OnDataChanged()
{
	switch(weapon_combobox->GetActiveItem())
	{
		case 0:
			engine->ClientCmd("npc_create_equipment weapon_pistol\n");
			break;
		case 1:
			engine->ClientCmd("npc_create_equipment weapon_shotgun\n");
			break;
		case 2:
			engine->ClientCmd("npc_create_equipment weapon_smg1\n");
			break;
		case 3:
			engine->ClientCmd("npc_create_equipment weapon_fraglauncher\n");
			break;
		default:
			break;
	}
	
}

void SpawnMenuNPC::Init()
{
	
}

void SpawnMenuNPC::PopulateButtons()
{
	kvRoot = new KeyValues("root");
	if (!kvRoot->LoadFromFile(filesystem, "scripts/spawnmenu_npc.txt", "MOD"))
	{
		kvRoot->deleteThis();
		Warning("Failed to load txt file!\n");
	}
	KeyValues* pKV = kvRoot->FindKey("npcs");

	FOR_EACH_SUBKEY(pKV, pKey)
	{
		SpawnMenuData_t* data = new SpawnMenuData_t;
		data->Parse(pKey);
		SpawnMenuButton* button = new SpawnMenuButton(this,data->szName);
		button->data = data;
		v_buttons.AddToTail(button);
	}
}

void SpawnMenuNPC::PositionButtons()
{
	int row = 0;
	int indx = 0;
	for (int i = 0; i < v_buttons.Count(); i++)
	{
		SpawnMenuButton* button = NULL;
		button = v_buttons.Element(i);
		button->m_pImage->SetImage(button->data->szIcon);
		int xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
		int ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		if ((xpos + button->GetWide() + BORDER_SPACING) > GetWide())
		{
			row++;
			indx = 0;
			xpos = (button->GetWide() * indx) + (BORDER_SPACING * (indx + 1));
			ypos = ((button->GetTall() + BORDER_SPACING) * row) + BORDER_SPACING;
		}
		button->SetPos(xpos, ypos);
		DevMsg("data button %s\n", button->data->szName);
		indx++;
	}
}
/// <summary>
/// Spawn menu main class
/// </summary>
class SpawnMenu : public PropertyDialog
{
	DECLARE_CLASS_SIMPLE(SpawnMenu, PropertyDialog);
public:
	SpawnMenu(VPANEL parent);
};

SpawnMenu::SpawnMenu(VPANEL parent) : BaseClass(nullptr, "SpawnMenu")
{
	SetParent(parent);
	SetName("Spawn Menu");
	SetBounds(0, 0, 1280, 720);
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
	AddPage(new SpawnMenuNPC(this), "NPC");
	AddPage(new SpawnMenuWeapons(this), "Weapons");
	AddPage(new SpawnMenuItems(this), "Items");
	AddPage(new SpawnMenuEntities(this), "Entities");
	//MakePopup(false, false, false);
}

static vgui::DHANDLE<SpawnMenu> g_hBonusOptions;

void CloseSpawnMenu()
{
	if (g_hBonusOptions.Get())
		g_hBonusOptions->Close();
}
void OpenSpawnMenu()
{
	if (!g_hBonusOptions.Get())
	{
		vgui::VPANEL parent = enginevgui->GetPanel(PANEL_ROOT);
		if (parent == NULL)
		{
			Assert(0);
			return;
		}

		g_hBonusOptions.Set(new SpawnMenu(parent));
	}

	auto* pPanel = g_hBonusOptions.Get();


	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds(x, y, w, h);

	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos(x + w / 2 - mw / 2, y + h / 2 - mh / 2);

	pPanel->Activate();
}
CON_COMMAND(ToggleSpawnMenu, "")
{
	OpenSpawnMenu();
}

void In_Spawnmenu_Pressed(const CCommand& args)
{
	OpenSpawnMenu();
}
void In_Spawnmenu_Released(const CCommand& args)
{
	CloseSpawnMenu();
}

static ConCommand spawnmenuopen("+spawnmenu", In_Spawnmenu_Pressed);
static ConCommand spawnmenuclose("-spawnmenu", In_Spawnmenu_Released);