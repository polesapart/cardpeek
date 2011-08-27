/********************************************************************** 
*
* This file is part of Cardpeek, the smartcard reader utility.
*
* Copyright 2009-2011 by 'L1L1'
*
* Cardpeek is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Cardpeek is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Cardpeek.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <lauxlib.h>
#include "lua_ui.h"
#include "gui.h"
#include "cardtree.h"
#include "misc.h"
#include <string.h>
#include <stdlib.h>
#include "bytestring.h"
#include "lua_bytes.h"

/***********************************************************
 * USER INTERFACE FUNCTIONS
 */

GtkTreeIter* luaL_check_node_ref(lua_State *L, int p)
{
  void *ud = luaL_checkudata(L,p,"node_ref.type");
  luaL_argcheck(L, ud != NULL, p, "`node_ref' expected");
  return (GtkTreeIter *)ud;
}

void lua_push_node_ref(lua_State *L, GtkTreeIter *iter)
{
  GtkTreeIter *dst;

  if (iter==NULL)
    lua_pushnil(L);
  else
  {
    dst = (GtkTreeIter *)lua_newuserdata(L, sizeof(GtkTreeIter));
    luaL_getmetatable(L, "node_ref.type");
    lua_setmetatable(L, -2);
    *dst = *iter;
  }
}


int subr_ui_tree_add_node(lua_State* L)
{
	GtkTreeIter *parent;            /*1*/
	const char *classname;  	/*2*/
	const char *label;		/*3*/
	const char *id;             	/*4*/
	const char *size;             	/*5*/
	GtkTreeIter node;
	gboolean res;

	if (lua_isnoneornil(L,1))
		parent = NULL;
	else
		parent=luaL_check_node_ref(L,1);

	classname=lua_tostring(L,2);

	if (lua_isnoneornil(L,3))
		label = NULL;
	else
		label = lua_tostring(L,3);

	if (lua_isnoneornil(L,4))
		id = NULL;
	else
		id = lua_tostring(L,4);

	if (lua_isnoneornil(L,5))
		size = NULL;
	else
		size = lua_tostring(L,5);

	
	res = cardtree_node_append(CARDTREE,&node,parent,classname,label,id,size);

	if (res)
	{
		lua_push_node_ref(L,&node);
	}
	else
	{	
		lua_pushnil(L);
	}

	gui_update(1);
	return 1;
}

int subr_ui_tree_set_attribute(lua_State* L)
{
  GtkTreeIter *iter;
  const char *attribute_name;
  const char *attribute_value;

  iter = luaL_check_node_ref(L,1);
  attribute_name  = lua_tostring(L,2);
  attribute_value = lua_tostring(L,3);

  if (cardtree_attribute_set_by_name(CARDTREE,iter,attribute_name,attribute_value))
    lua_pushboolean(L,1);
  else
    lua_pushboolean(L,0);

  return 1;
}

int subr_ui_tree_get_attribute(lua_State* L)
{
  GtkTreeIter *iter;
  const char *attribute_name;
  const char *attribute_value;

  iter  	  = luaL_check_node_ref(L,1);
  attribute_name  = lua_tostring(L,2);

  if (cardtree_attribute_get_by_name(CARDTREE,iter,attribute_name,&attribute_value))
  {
      	lua_pushstring(L,attribute_value);
  }
  else
    lua_pushnil(L);

  return 1;
}

int subr_ui_tree_set_value(lua_State* L)
{
	GtkTreeIter *iter;
	char *value;
	bytestring_t *bs;

	iter = luaL_check_node_ref(L,1);
	if (lua_isnoneornil(L,2))
		value = NULL;
	else
	{
		bs = luaL_checkbytestring(L,2);
		value = bytestring_to_format("%S",bs);
	}

	if (cardtree_attribute_set(CARDTREE,iter,CC_VAL,value))
		lua_pushboolean(L,1);
	else
		lua_pushboolean(L,0);
	if (value)
		free(value);
	return 1;
}

int subr_ui_tree_get_value(lua_State* L)
{
	GtkTreeIter *iter;
	const char *value;

	iter = luaL_check_node_ref(L,1);

	if (cardtree_attribute_get(CARDTREE,iter,CC_VAL,&value))
	{
		lua_pushbytestring(L,bytestring_new_from_string(value));
	}
	else
		lua_pushnil(L);

	return 1;
}

int subr_ui_tree_set_alt_value(lua_State* L)
{
  GtkTreeIter *iter;
  const char *attribute_value;

  iter = luaL_check_node_ref(L,1);
  attribute_value = lua_tostring(L,2);

  if (cardtree_attribute_set(CARDTREE,iter,CC_ALT,attribute_value))
    lua_pushboolean(L,1);
  else
    lua_pushboolean(L,0);

  return 1;
	
}

int subr_ui_tree_get_alt_value(lua_State* L)
{
	GtkTreeIter *iter;
	const char *value;

	iter = luaL_check_node_ref(L,1);

	if (cardtree_attribute_get(CARDTREE,iter,CC_ALT,&value))
	{
		lua_pushstring(L,value);
	}
	else
		lua_pushnil(L);

	return 1;
}

int subr_ui_tree_get_node(lua_State* L)
{
/* 
	const char *path;
	char *classname;
	char *description;
	char *id;
	unsigned size;
	unsigned num_children;

	path = lua_tostring(L,1);
	if (!cardtree_node_get(CARDTREE,path,&classname,&description,&id,&size,&num_children))
	{
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);

	if (classname)
	{
		lua_pushstring(L,"class");
		lua_pushstring(L,classname);
		lua_settable(L,-3);
		free(classname);
	}

	if (description)
	{
		lua_pushstring(L,"description");
		lua_pushstring(L,description);
		lua_settable(L,-3);
		free(description);
	}

	if (id)
	{
		lua_pushstring(L,"id");
		lua_pushstring(L,id);
		lua_settable(L,-3);
		free(id);
	}

	lua_pushstring(L,"size");
	lua_pushinteger(L,size);
	lua_settable(L,-3);

	lua_pushstring(L,"num_children");
	lua_pushinteger(L,num_children);
	lua_settable(L,-3);
*/
#warning IMPLEMENT
	lua_pushnil(L);
	return 1;
}

int subr_ui_tree_delete_node(lua_State* L)
{
  GtkTreeIter* iter = luaL_check_node_ref(L,1);

  if (!cardtree_node_remove(CARDTREE,iter))
    lua_pushboolean(L,0);
  else
    lua_pushboolean(L,1);
  
  gui_update(1);
  return 1;
}
/*
 * FOR THE FUTURE MAYBE
 *  
gboolean create_search_context(lua_State* L, int **r_indices, char ***r_values, int *r_n_values)
{
  int i,index;
  char *label;
  char *value;
  int *indices;
  char **values;
  int n_values = (lua_gettop(L)-1)/2;

  if (n_values<1)
  	return FALSE;

  indices = g_new0(int,n_values);
  values  = g_new0(char *,n_values);

  n_values = 0;
  for (i=2;i<=lua_gettop(L);i+=2)
  {
  	if (!lua_isnoneornil(L,i))
    		label = lua_tostring(L,i);
	else
		continue;

	if (!lua_isnoneornil(L,i+1))
    		value = lua_tostring(L,i);
	else
		continue;

	if ((index = dyntree_model_column_name_to_index(CARDTREE,label))>=0)
	{
		indices[n_values] = index;
		values[n_values] = value;
		n_values ++;
	}
  }
  *r_indices  = indices;
  *r_values   = values;
  *r_n_values = n_values;
  return TRUE;
}

int subr_ui_find_node(lua_State* L)
{
  GtkTreeIter* iter; 
  int *indices;
  char **values;
  int n_values;
  GtkTreeIter* node;

  if (!lua_isnoneornil(L,1))
  	  iter = lua_check_node_ref(L,1);
  else
	  iter = NULL;

  if (create_search_context(L,&indices,&values,&n_valuues)==FALSE)
  {
	lua_pushnil(L);
  	return 1;
  }

  if (dyntree_model_iter_find_first(CARDTREE,&node,iter,indices,values,n_values))
    lua_push_node_ref(L,&node);
  else
    lua_pushnil(L);
  g_free(indices);
  g_free(values);
  return 1;
}

int subr_ui_find_all_nodes(lua_State* L)
{
  GtkTreeIter* iter; 
  int *indices;
  char **values;
  int n_values;
  GtkTreeIter* node;
  int i;

  if (!lua_isnoneornil(L,1))
  	  iter = lua_check_node_ref(L,1);
  else
	  iter = NULL;

  if (create_search_context(L,&indices,&values,&n_valuues)==FALSE)
  {
	lua_pushnil(L);
  	return 1;
  }


  lua_newtable(L);

  if (dyntree_model_iter_find_first(CARDTREE,&node,iter,indices,values,n_values))
  {
 	lua_pushinteger(L,1);
      	lua_push_node_ref(L,&node);
      	lua_settable(L,-3);

	i=2;
	while (dyntree_model_iter_find_next(CARDTREE,&node,iter,indices,values,n_values))
	{
		lua_pushinteger(L,i++);
	      	lua_push_node_ref(L,&node);
      		lua_settable(L,-3);
	}
  }

  return 1;
}
***/

int subr_ui_find_node(lua_State* L)
{
  GtkTreeIter* iter; 
  const char *label;
  const char *id;
  GtkTreeIter node;

  if (!lua_isnoneornil(L,1))
  	  iter = luaL_check_node_ref(L,1);
  else
	  iter = NULL;

  if (lua_isnoneornil(L,2))
		label = NULL;
	else
		label = lua_tostring(L,2);

	if (lua_isnoneornil(L,3))
		id = NULL;
	else
		id = lua_tostring(L,3);


  if (cardtree_find_first(CARDTREE,&node,iter,label,id))
    lua_push_node_ref(L,&node);
  else
    lua_pushnil(L);

  return 1;
}

int subr_ui_find_all_nodes(lua_State* L)
{
	GtkTreeIter* iter; 
	const char *label;
	const char *id;
	GtkTreeIter node;
	int i;

	if (!lua_isnoneornil(L,1))
		iter = luaL_check_node_ref(L,1);
	else
		iter = NULL;

	if (lua_isnoneornil(L,2))
		label = NULL;
	else
		label = lua_tostring(L,2);

	if (lua_isnoneornil(L,3))
		id = NULL;
	else
		id = lua_tostring(L,3);

	lua_newtable(L);

	if (cardtree_find_first(CARDTREE,&node,iter,label,id))
	{
		lua_pushinteger(L,1);
		lua_push_node_ref(L,&node);
		lua_settable(L,-3);

		i=2;
		while (cardtree_find_next(CARDTREE,&node,iter,label,id))
		{
			lua_pushinteger(L,i++);
			lua_push_node_ref(L,&node);
			lua_settable(L,-3);
		}
	}

	return 1;
}

int subr_ui_tree_to_xml(lua_State* L)
{
  GtkTreeIter *iter;
  char *res;
  
  if (lua_isnoneornil(L,1))
	iter = NULL;
  else
  	iter = luaL_check_node_ref(L,1);

  res = cardtree_to_xml(CARDTREE,iter);
  if (res==NULL)
  {  
    	lua_pushnil(L);
  }
  else  
  {
    	lua_pushstring(L,res);
    	free(res);
  }
  
  return 1;
}

int subr_ui_tree_save(lua_State* L)
{
  const char *filename;

  if (lua_isnoneornil(L,1))
    return luaL_error(L,"Expecting one parameter: a filename (string)");

  filename= lua_tostring(L,1);

  if (cardtree_to_xml_file(CARDTREE,NULL,filename)==0)
  {
    log_printf(LOG_ERROR,"Could not write xml data to '%s'",filename);
    lua_pushboolean(L,0);
  }
  else
  {
    log_printf(LOG_INFO,"Wrote card data to '%s'",filename);
    lua_pushboolean(L,1);
  }
  return 1;
}

int subr_ui_tree_load(lua_State* L)
{
  const char *filename;
  int retval;

  if (lua_isnoneornil(L,1))
    return luaL_error(L,"Expecting one parameter: a filename (string)");
  
  filename = lua_tostring(L,1);
  retval =  cardtree_from_xml_file(CARDTREE,NULL,filename);
  lua_pushboolean(L,retval);
  return 1;
}

int subr_ui_question(lua_State* L)
{
  const char* message;
  const char** items;
  unsigned item_count;
  unsigned i;
  int result;

  if (!lua_isstring(L,1) || !lua_istable(L,2))
    return luaL_error(L,"expecting a string and a table as arguments to this function");

  if (!lua_isnil(L,1))
    message = lua_tostring(L,1);
  else
    message = "";

  item_count = lua_objlen(L,2);
  items = malloc(sizeof(char *)*item_count);
  for (i=0;i<item_count;i++)
  {
    lua_rawgeti(L,2,i+1);
    if (lua_isstring(L,-1))
      items[i] = lua_tostring(L,-1);
    else
      items[i] = "(error)";
    lua_pop(L,1);
  }

  result = gui_question_l(message,item_count,items);
  free(items);

  if (result<0)
    lua_pushnil(L);
  else
    lua_pushinteger(L,result+1);
  return 1;
}
 
int subr_ui_readline(lua_State* L)
{
  const char* message;
  unsigned len;
  const char* default_value;
  char *value;

  if (!lua_isnoneornil(L,1))
    message = lua_tostring(L,1);
  else
    message = "Input";
  if (!lua_isnoneornil(L,2))
    len = lua_tointeger(L,2);
  else
    len = 40;
  if (!lua_isnoneornil(L,3))
    default_value = lua_tostring(L,3);
  else
    default_value = "";

  value = malloc(len+1);
  strncpy(value,default_value,len);
  gui_readline(message,len,value);
  lua_pushstring(L,value);
  free(value);
  return 1;
}

int subr_ui_select_file(lua_State* L)
{
  const char* title;
  const char* path;
  const char* filename;
  char **pair;

  title = lua_tostring(L,1);
  if (!lua_isnoneornil(L,2))
    path = lua_tostring(L,2);
  else
    path = NULL;
  if (!lua_isnoneornil(L,3))
    filename = lua_tostring(L,3);
  else
    filename = NULL;
  pair = gui_select_file(title,path,filename);
  if (pair[0])
  {
    lua_pushstring(L,pair[0]);
    g_free(pair[0]);
  }
  else
    lua_pushnil(L);

  if (pair[1])
  {
    lua_pushstring(L,pair[1]);
    g_free(pair[1]);
  }
  else
    lua_pushnil(L);
  return 2;
}

int subr_ui_about(lua_State* L)
{
  gui_about();
  return 0;
}

int subr_node_ref_tostring(lua_State* L)
{
  GtkTreeIter *iter;
  char buf[20];

  iter = luaL_check_node_ref(L,1);
  g_sprintf(buf,"<node:%p>",iter->user_data);
  lua_pushstring(L,buf);
  return 1;
}


static const struct luaL_reg nodereflib_mt [] = {
	{ "__tostring", subr_node_ref_tostring },
  	{NULL,NULL}  /* sentinel */
};

static const struct luaL_reg uilib [] = {
  {"tree_add_node", subr_ui_tree_add_node },
  {"tree_set_value", subr_ui_tree_set_value },
  {"tree_get_value", subr_ui_tree_get_value },
  {"tree_set_alt_value", subr_ui_tree_set_alt_value },
  {"tree_get_alt_value", subr_ui_tree_get_alt_value },
  {"tree_get_node", subr_ui_tree_get_node },
  {"tree_delete_node", subr_ui_tree_delete_node },
  {"tree_find_node", subr_ui_find_node },
  {"tree_find_all_nodes", subr_ui_find_all_nodes },
  {"tree_set_attribute", subr_ui_tree_set_attribute },
  {"tree_get_attribute", subr_ui_tree_get_attribute },
  {"tree_to_xml", subr_ui_tree_to_xml },
  {"tree_save",subr_ui_tree_save },
  {"tree_load",subr_ui_tree_load },
  {"readline",subr_ui_readline },
  {"question",subr_ui_question },
  {"select_file",subr_ui_select_file },
  {"about", subr_ui_about },
  {NULL,NULL}  /* sentinel */
};

int luaopen_ui(lua_State* L)
{
  luaL_newmetatable(L, "node_ref.type");
  luaL_openlib(L, NULL, nodereflib_mt, 0);
  luaL_openlib(L, "ui", uilib, 0);
  return 1;
}

