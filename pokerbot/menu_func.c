#include <stdlib.h>
#include "menu_func.h"
#include "send_input.h"
#include "basic_log.h"
#include "main_bot.h"

menu_node_t * m_new_node(int ID, menu_t *m)
{
	menu_node_t *n;

	n = malloc(sizeof(menu_node_t));
	ZeroMemory(n, sizeof(menu_node_t));

	n->ID = ID;
	n->menu = m;

	return n;
}

void m_destroy_node(menu_node_t *n)
{
	n->parent = NULL;
	n->ID = 0;

	menu_destroy(n->menu);
	n->menu = NULL;

	while (n->num_children--)
	{
		m_destroy_node(n->children[n->num_children]);
		n->children[n->num_children] = NULL;
	}
}

menu_node_t * m_add_node(menu_node_t *parent, menu_node_t *n)
{
	if (!parent || !n)
	{
		log_err(_T("m_add_node() -> nodes empty"));
		return NULL;
	}

	n->parent = parent;

	parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(menu_node_t *));

	if (!parent->children)
	{
		log_err(_T("m_add_node() -> realloc failed"));
		return NULL;
	}

	parent->children[parent->num_children] = n;
	parent->num_children++;

	return parent;
}

menu_node_t * m_find_node(menu_node_t *root, int ID)
{
	int i = 0;

	if (root->ID == ID)
		return root;

	for (i = 0; i < root->num_children; i++)
	{
		menu_node_t *n;
		if (n = m_find_node(root->children[i], ID))
			return n;
	}

	return NULL;
}

menu_node_t * m_add_node_to_ID(menu_node_t *root, int ID, menu_node_t *node)
{
	menu_node_t *ret;
	menu_node_t *n = m_find_node(root, ID);

	if (!n)
		return NULL;

	ret = m_add_node(n, node);

	return ret;
}

void select_all_parents(menu_node_t *node)
{
	if (node->parent)
	{
		select_all_parents(node->parent);

		if (node->parent->menu)
		{
			int i = 0;

			for (i = 0; i < node->parent->num_children; i++)
			{
				/* Select the proper menu option (the one that leads to this node) */
				if (node->parent->children[i]->ID == node->ID)
				{
					/* Meta nodes do not need selection */
					if (node->parent->menu)
					{
						menu_select_item(&node->parent->menu, i);						
						Sleep(100);
					}
					break;
				}
			}
		}
	}
}

int m_select_menu_path(menu_node_t *root, int menu_ID, int menu_ix)
{
	menu_node_t* n = m_find_node(root, menu_ID);

	if (!n)
	{
		log_err(_T("m_select_menu_path() -> Menu %d node not found"), menu_ID);
		return 1;
	}

	select_all_parents(n);
	/* Select current node option */
	menu_select_item(&n->menu, menu_ix);

	return 0;
}

menu_item_t * menu_new_item()
{
	menu_item_t *i = malloc(sizeof(menu_item_t));

	if (!i)
	{
		log_err(_T("Could not allocate memory for new menu item"));
		return NULL;
	}

	ZeroMemory(i, sizeof(menu_item_t));
	return i;
}

menu_t * menu_new_menu()
{
	menu_t *i = malloc(sizeof(menu_t));

	if (!i)
	{
		log_err(_T("Could not allocate memory for new menu"));
		return NULL;
	}

	ZeroMemory(i, sizeof(menu_t));
	return i;
}


int menu_select_item(menu_t **menu, int ID)
{
	int ret = 0;
	int i = 0;
	LVITEM lv_item;
	LPVOID address;
	menu_t *m = NULL;

	ZeroMemory(&lv_item, sizeof(lv_item));

	if (!*menu)
	{
		log_err(_T("Cannot select empty menu item"));
		return 1;
	}
	
	/* Refresh menu */
	if ((*menu)->refresh)
		*menu = menu_rebuild(*menu);

	m = *menu;

	for (i = 0; i < m->item_num; i++)
	{
		if ((m->items[i])->ID == ID)
			break;
	}
	if (i >= m->item_num)
	{
		log_err(_T("Didn't find item %d in menu %p"), ID, m);
		return 2;
	}

	if (!m->items[i]->handle && m->type != LISTVIEW)
	{
		log_err(_T("Menu item %d is without a handle"), i);
		return 3;
	}

	if (!m->parent)
	{
		log_err(_T("Cannot select from menu without a parent"));
		return 4;
	}

	if (m->selected && m->selected->ID == ID)
	{
		log_dbg(_T("Item %d already selected, moving on..."), ID);
		return 0;
	}

	log_dbg(_T("Selecting item %d from menu [%p]"), i, m->items[i]->handle);

	switch (m->type)
	{
	case TREEVIEW:
		TreeView_SelectItem(m->parent, (HTREEITEM)(m->items[i]->handle));
		break;

	case LISTVIEW:
		/* Deselect all */
		lv_item.iItem = -1;
		lv_item.mask = LVIF_STATE;
		lv_item.stateMask = LVIS_SELECTED;
		lv_item.state = 0;

		address = VirtualAllocEx(main_bot_get_app()->p_info.hProcess, NULL, sizeof(LVITEM),
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!address)
		{
			log_err(_T("Failed to reserve memory in app process"));
			return 5;
		}
		if (!WriteProcessMemory(main_bot_get_app()->p_info.hProcess, address, &lv_item, sizeof(lv_item), NULL))
		{
			log_dbg(_T("Failed to write listview item to process memory"));
			return 6;
		}

		SendMessage(m->parent, LVM_SETITEMSTATE, -1, (LPARAM)address);

		/* Select one */
		lv_item.iItem = ID;
		lv_item.mask = LVIF_STATE;
		lv_item.stateMask = LVIS_SELECTED;
		lv_item.state = LVIS_SELECTED;

		if (!WriteProcessMemory(main_bot_get_app()->p_info.hProcess, address, &lv_item, sizeof(lv_item), NULL))
		{
			log_dbg(_T("Failed to write listview item to process memory"));
			return 7;
		}

		SendMessage(m->parent, LVM_SETITEMSTATE, ID, (LPARAM)address);

		VirtualFreeEx(main_bot_get_app()->p_info.hProcess, address, 0, MEM_RELEASE);
		break;

	case BUTTON:
		send_click((HWND)m->items[i]->handle, 1, 1);
		break;
	default:
		log_err(_T("Tried to select menu with no type! [%p]"), m);
		break;
	}

	/* Set selected */
	m->selected = m->items[i];
	return 0;
}

int menu_destroy_item(menu_item_t *it)
{
	int i = 0;

	if (!it)
		return 0;

	it->handle = NULL;
	it->ID = 0;

	free(it);

	return 0;
}

int menu_destroy(menu_t *m)
{
	int i;

	if (!m)
		return 0;

	log_dbg(_T("Destroying menu"));

	m->parent = NULL;
	m->selected = NULL;

	for (i = 0; i < m->item_num; i++)
	if (m->items[i])
	{
		menu_destroy_item(m->items[i]);
		m->items[i] = NULL;
	}

	m->items = NULL;
	m->item_num = 0;

	free(m);

	return 0;
}

int menu_build_button(menu_t *m, int max)
{
	int num = 0;
	HWND prev;

	if (m->parent)
	{
		m->items = malloc(sizeof(menu_item_t *));
		if (!m->items)
		{
			log_err(_T("Failed to create space for button menu items"));
			return -1;
		}

		m->items[m->item_num] = menu_new_item();
		if (!m->items[m->item_num])
		{
			log_err(_T("Failed to create space for menu item %d"), m->item_num);
			return -1;
		}
		m->items[m->item_num]->handle = m->parent;
		m->items[m->item_num]->ID = m->item_num;
		m->item_num++;
		num++;

		prev = m->parent;
	}

	while (--max)
	{
		if (!(prev = GetNextWindow(prev, GW_HWNDNEXT)))
			return num;
		else
		{
			m->items = realloc(m->items, (m->item_num + 1) * sizeof(menu_item_t*));
			if (!m->items)
			{
				log_err(_T("Failed to reallocate space for button menu items"));
				return -1;
			}

			m->items[m->item_num] = menu_new_item();
			if (!m->items[m->item_num])
			{
				log_err(_T("Failed to create space for menu item %d"), m->item_num);
				return -1;
			}

			m->items[m->item_num]->handle = prev;
			m->items[m->item_num]->ID = m->item_num;
			m->item_num++;
			num++;
		}

	}

	return num;
}

int menu_build_tree(menu_t *m, HWND parent)
{
	log_dbg(_T("Building tree from parent: %p"), parent);

	HTREEITEM item = TreeView_GetRoot(parent);

	if (item)
	{
		log_dbg(_T("Found root: %p"), item);
		m->items = calloc(1, sizeof(menu_item_t *));
		if (!m->items)
		{
			log_err(_T("Failed to create space for tree items"));
			return -1;
		}

		m->items[m->item_num] = menu_new_item();
		if (!m->items[m->item_num])
		{
			log_err(_T("Failed to create space for root"));
			return -1;
		}

		m->items[m->item_num]->handle = item;
		m->items[m->item_num]->ID = m->item_num;
		m->item_num++;
	}
	else
	{
		log_err(_T("Cannot find root"));
		return -1;
	}

	while (item = TreeView_GetNextItem(parent, item, TVGN_NEXT))
	{
		m->items = realloc(m->items, (m->item_num + 1) * sizeof(menu_item_t*));
		if (!m->items)
		{
			log_err(_T("Cannot reallocate memory for tree items"));
			return -1;
		}

		m->items[m->item_num] = menu_new_item();
		if (!m->items[m->item_num])
		{
			log_err(_T("Failed to create space for item %d"), m->item_num);
			return -1;
		}
		m->items[m->item_num]->handle = item;
		m->items[m->item_num]->ID = m->item_num;
		m->item_num++;
	}

	return m->item_num;

}


int menu_build_list(menu_t *m, HWND parent)
{
	log_dbg(_T("Building list from parent: %p"), parent);

	/* How many items do we have? */
	int count = ListView_GetItemCount(parent);
	int i = 0;

	if (count)
	{
		log_dbg(_T("Found %d elements"), count);
		m->items = calloc(count, sizeof(menu_item_t *));
		if (!m->items)
		{
			log_err(_T("Cannot allocate space for list items"));
			return -1;
		}

		for (int i = 0; i < count; i++)
		{
			m->items[m->item_num] = menu_new_item();
			if (!m->items[m->item_num])
			{
				log_err(_T("Failed to create space for item %d"), m->item_num);
				return -1;
			}

			/* Lists do not have handles */
			m->items[m->item_num]->handle = NULL;
			m->items[m->item_num]->ID = m->item_num;
			m->item_num++;
		}
	}
	else
	{
		log_dbg(_T("List empty"));
		return 0;
	}

	return m->item_num;
}

menu_t * menu_rebuild(menu_t *m)
{
	int i = 0;

	if (!m)
	{
		log_err(_T("Cannot rebuild empty menu"));
		return NULL;
	}

	log_dbg("Rebuilding menu");

	menu_t *new_menu = menu_build(m->parent, m->type, (m->type == BUTTON ? m->item_num : 0), m->refresh);

	if (!new_menu)
	{
		log_err("Failed to rebuild menu");
		return NULL;
	}

	menu_destroy(m);
	return new_menu;
}

menu_t * menu_build(HWND parent, menu_type_t type, int max, int refresh)
{
	menu_t *m;

	if (!parent)
	{
		log_err(_T("Cannot create menu without a parent"));
		return NULL;
	}

	m = menu_new_menu();

	if (!m)
		return NULL;

	m->type = type;
	m->parent = parent;
	m->refresh = refresh;

	switch (type)
	{
	case TREEVIEW:
		menu_build_tree(m, parent);
		break;
	case BUTTON:
		menu_build_button(m, max);
		break;
	case LISTVIEW:
		menu_build_list(m, parent);
	}

	return m;
}