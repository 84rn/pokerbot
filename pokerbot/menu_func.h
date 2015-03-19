#ifndef MENU_FUNC_H
#define MENU_FUNC_H

#include <Windows.h>
#include <CommCtrl.h>

/* MAIN MENU ITEMS */

enum m_node_id
{
	NID_MAIN_MENU,

	NID_POKER,

	NID_POKER_CASH,
	NID_POKER_FASTFORWARD,
	NID_POKER_SITNGOS,
	NID_POKER_TOURNAMENTS,
	NID_POKER_PLAYMONEY,
	NID_POKER_CASINO,

	NID_PPLAYMONEY_STAKES,
	NID_PPLAYMONEY_SEATS,
	NID_PPLAYMONEY_GAMENAME,
	NID_PPLAYMONEY_TABLES,

	NID_PCASHMONEY_TYPE,
	NID_PCASHMONEY_BLINDS,
	NID_PCASHMONEY_TABLE_SIZE,
	NID_PCASHMONEY_TABLES
};

enum m_main_menu_ix
{
	IX_POKER,
	// TODO: Add others
};

enum m_poker_ix
{
	IX_CASINO, 
	IX_PLAYMONEY,
	IX_TOURNAMENTS,
	IX_SITANDGOS,
	IX_FASTFORWARD,
	IX_CASH
};

enum m_pplaymoney_type_ix
{
	IX_PPFASTFORWARD,
	IX_PPFL_HOLDEM,
	IX_PPNL_HOLDEM,
	IX_PPPL_HOLDEM,
	IX_PPPT_OMAHA,
	IX_PPSTUD,
	IX_PPTSITANDGOS,
	IX_PPTOURNAMENTS
};

typedef enum menu_type_e
{
	NONE = 0,
	TREEVIEW,
	BUTTON,
	LISTVIEW
} menu_type_t;

typedef struct s_menu_item
{
	void *handle;
	int ID;

} menu_item_t;

typedef struct s_menu
{
	menu_item_t **items;
	menu_item_t *selected;
	void *parent;
	menu_type_t type;
	int item_num;

	int refresh;

} menu_t, *pmenu_t;


struct tag_menu_node
{	
	int num_children;

	int ID;
	menu_t *menu;
	struct tag_menu_node *parent;
	struct tag_menu_node **children;
};

typedef struct tag_menu_node menu_node_t;


menu_node_t * m_new_node(int ID, menu_t *m);
void m_destroy_node(menu_node_t *n);

menu_node_t * m_add_node(menu_node_t *parent, menu_node_t *n);
menu_node_t * m_add_node_to_ID(menu_node_t *root, int ID, menu_node_t *node);
menu_node_t * m_find_node(menu_node_t *root, int ID);

void select_all_parents(menu_node_t *node);
int m_select_menu_path(menu_node_t *root, int menu_ID, int menu_ix);

int menu_select_item(menu_t **menu, int ID);
menu_t * menu_build(HWND parent, menu_type_t type, int max, int refresh);
int menu_destroy(menu_t *m);
menu_t * menu_rebuild(menu_t *m);

#endif