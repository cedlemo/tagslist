#include <e.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;

struct _Instance 
{
  E_Gadcon_Client *gcc;
  Evas_Object * widget; /* table of label*/
  Eina_List * items; /* ref to all objects in table widget */
  Eina_Bool horiz_orient: 1; /*orientation of th widegt horizontal by default*/
};

typedef struct _Desk Desk;
struct _Desk
{
  E_Desk * desk;
  Evas_Object * label;
  int x;
  int y;
};
/* Functions Proto Requirement for Gadcon*/
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);

/* Functions helpers*/
static void _fill_desks_widget(Instance * instance, E_Gadcon *gc);
static void _update_selected_desk_widget(Instance * instance, E_Gadcon *gc);
static void _update_names_desks_widget(Instance * instance, E_Gadcon *gc, E_Desk *desk);
static void _instance_free( Instance * inst);
static Eina_Bool _is_horizontal(E_Gadcon_Orient orient);
static Eina_Bool _pager_cb_event_zone_desk_count_set(void *data, int type, void *event);
static Eina_Bool _pager_cb_event_desk_show(void *data, int type, void *event);
static Eina_Bool _pager_cb_event_desk_name_change(void *data, int type, void *event);
static void _pager_desk_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj , void *event_info);
static void _pager_desk_cb_mouse_up(void *data, Evas *e, Evas_Object *obj , void *event_info); 


/* Define the class and gadcon function this module provides*/
static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "tagslist", 
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

static Config_Item *_config_item_get(const char *id);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *tagslist_conf = NULL;

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
  Instance *inst;
  E_Gadcon_Client *gcc;
  char buf[4096]; //PATH_MAX
  const char *mod_dir;

  inst = E_NEW(Instance, 1);   
  snprintf(buf, sizeof(buf), "%s/tagslist.edj",e_module_dir_get(tagslist_conf->module));

  inst->widget = e_table_add(gc->evas);
  e_table_homogenous_set(inst->widget, 0);
  _fill_desks_widget(inst, gc);
  evas_object_show(inst->widget);
  
  gcc = e_gadcon_client_new(gc, name, id, style, inst->widget);
  gcc->data = inst;
  inst->gcc = gcc;

  tagslist_conf->instances = NULL;
  tagslist_conf->instances = eina_list_append(tagslist_conf->instances, inst);

    /* Handlers for mouse events*/
   /*evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
				                            _button_cb_mouse_down, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
                                    _tclock_cb_mouse_in, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
                                    _tclock_cb_mouse_out, inst);
  */
  return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
  
   if( !(inst = gcc->data)) return;
   
   /* Remove mouse handlers 
    evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_DOWN,
                                  _tclock_cb_mouse_down);
    evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_IN,
                                  _tclock_cb_mouse_in);
    evas_object_event_callback_del(inst->tclock, EVAS_CALLBACK_MOUSE_OUT,
                                  _tclock_cb_mouse_out);
    */
 // if( inst->items )
 // {
 //   
 // } 
  _instance_free(inst); 
  tagslist_conf->instances = eina_list_remove(tagslist_conf->instances, inst);
  E_FREE(inst);
}
static void _instance_free( Instance * inst)
{
  Eina_List *l;
  Desk * o;
  EINA_LIST_FREE(inst->items,o)
  {
    evas_object_del(o->label);
    E_FREE(o);
  }
  inst->items = NULL;
 
  if (inst->widget)
  {
    e_table_unpack(inst->widget);
  }
}
static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
  Instance *inst;
  Evas_Coord mw, mh;

  inst = gcc->data;
  mw = 0, mh = 0;
  //edje_object_size_min_get(inst->widget, &mw, &mh);
  //if ((mw < 1) || (mh < 1))
    //edje_object_size_min_calc(inst->widget, &mw, &mh);
  e_table_size_min_get(inst->widget, &mw, &mh);
  if (mw < 4) mw = 4;
  if (mh < 4) mh = 4;
  e_gadcon_client_min_size_set(gcc, mw, mh);   
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class) 
{
   return "tagslist";
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas) 
{
   Evas_Object *o;
   char buf[4096];

   if (!tagslist_conf->module) return NULL;

   snprintf(buf, sizeof(buf), "%s/e-module-tagslist.edj", 
	    e_module_dir_get(tagslist_conf->module));

   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   Config_Item *ci;

   ci = _config_item_get(NULL);
   return ci->id;
}

static Eina_Bool _is_horizontal(E_Gadcon_Orient orient)
{
  Eina_Bool ret = EINA_TRUE;
  switch(orient)
  {
    case E_GADCON_ORIENT_VERT:
    case E_GADCON_ORIENT_LEFT:
    case E_GADCON_ORIENT_RIGHT:
    case E_GADCON_ORIENT_CORNER_LT:
    case E_GADCON_ORIENT_CORNER_RT:
    case E_GADCON_ORIENT_CORNER_LB:
    case E_GADCON_ORIENT_CORNER_RB:
      ret = EINA_FALSE;
      break;
    case E_GADCON_ORIENT_TOP:
    case E_GADCON_ORIENT_BOTTOM:
    case E_GADCON_ORIENT_CORNER_TL:
    case E_GADCON_ORIENT_CORNER_TR:
    case E_GADCON_ORIENT_CORNER_BL:
    case E_GADCON_ORIENT_CORNER_BR:
    case E_GADCON_ORIENT_HORIZ:
    case E_GADCON_ORIENT_FLOAT:default:
      ret = EINA_TRUE;
  }

  return ret;
}

static void _fill_desks_widget(Instance * instance, E_Gadcon *gc)
{
  int x, y, i, j;
  char buf[4096];
  snprintf(buf, sizeof(buf), "%s/tagslist.edj",e_module_dir_get(tagslist_conf->module));
  e_zone_desk_count_get(gc->zone, &x, &y);
  e_table_freeze(instance->widget);
  instance->horiz_orient = _is_horizontal(gc->orient); 
  for(i=0; i<x; i++)
  {
    for(j=0; j<y; j++)
    {
      Desk * desk_object;
      E_Desk * desk;
      desk = e_desk_at_xy_get(gc->zone, i, j);
      if(desk)
      {
        desk_object = E_NEW(Desk, 1);
        desk_object->desk = desk;
        desk_object->x = i;
        desk_object->y = j;
        desk_object->label = edje_object_add(gc->evas);
        if(!e_theme_edje_object_set(desk_object->label,  
	              "base/theme/modules/tagslist", "modules/tagslist/main"))
        edje_object_file_set(desk_object->label, buf, "modules/tagslist/main");
        edje_object_part_text_set(desk_object->label, "label", desk->name);
        
        if (desk_object->desk == e_desk_current_get(gc->zone))
		      edje_object_signal_emit(desk_object->label, "e,state,selected", "e");
	      else
		      edje_object_signal_emit(desk_object->label, "e,state,unselected", "e");
        
        evas_object_event_callback_add(desk_object->label, EVAS_CALLBACK_MOUSE_WHEEL,
                                  _pager_desk_cb_mouse_wheel, gc->zone); 	
	      evas_object_event_callback_add(desk_object->label, EVAS_CALLBACK_MOUSE_UP,
                                  _pager_desk_cb_mouse_up, desk_object);
        int w,h;
        edje_object_size_min_calc(desk_object->label, &w, &h);

        if (instance->horiz_orient)
          e_table_pack(instance->widget, desk_object->label, i, j, 1, 1);
        else
          e_table_pack(instance->widget, desk_object->label, j, i, 1, 1);
        
        e_table_pack_options_set(desk_object->label, 1, 1, 1, 1, 0.5, 0.5, w, h, -1, -1);  
        evas_object_show(desk_object->label);
        instance->items = eina_list_append(instance->items, desk_object);
      }
    }
  }
  e_table_thaw(instance->widget);
}

static void _update_selected_desk_widget(Instance * instance, E_Gadcon *gc)
{
  int x, y, i, j, index;
  e_zone_desk_count_get(gc->zone, &x, &y);
  if ( x*y != eina_list_count(instance->items) )
  {
    return;
  }
  index = 0;
  e_table_freeze(instance->widget);
  for(i=0; i<x; i++)
  {
    for(j=0; j<y; j++)
    {
      index+=1;
      Evas_Object * label;
      E_Desk * desk;
      desk = e_desk_at_xy_get(gc->zone, i, j);
      Desk * desk_object;
      desk_object = (Desk *) eina_list_nth(instance->items, index -1); 
      // TODO try to simplify
      label = desk_object->label;
      edje_object_part_text_set(label, "label", desk->name);
    
      if(desk == e_desk_current_get(desk->zone)) 
		    edje_object_signal_emit(label, "e,state,selected", "e");
	    else
		    edje_object_signal_emit(label, "e,state,unselected", "e");
  
      int w,h;
      edje_object_size_min_calc(label, &w, &h);
      e_table_pack_options_set(label, 1, 1, 1, 1, 0.5, 0.5, w, h, -1, -1);  
    }
  }
  e_table_thaw(instance->widget);
}

static void 
_update_names_desks_widget(Instance * instance, E_Gadcon *gc, E_Desk *desk)
{
  int x, y, i, j, index;
  e_zone_desk_count_get(gc->zone, &x, &y);
  if ( x*y != eina_list_count(instance->items) )
  {
    return;
  }
  index = 0;
  e_table_freeze(instance->widget);
  for(i=0; i<x; i++)
  {
    for(j=0; j<y; j++)
    {
      index+=1;
      E_Desk * current_desk;
      current_desk = e_desk_at_xy_get(gc->zone, i, j);
    
      if(desk == current_desk)
      {  
        Evas_Object * label;
        Desk * desk_object;
        desk_object = (Desk *) eina_list_nth(instance->items, index -1);
        label = desk_object->label; 
        edje_object_part_text_set(label, "label", desk->name);
        int w,h;
        edje_object_size_min_calc(label, &w, &h);
        e_table_pack_options_set(label, 1, 1, 1, 1, 0.5, 0.5, w, h, -1, -1);  
	    }
    }
  }
  e_table_thaw(instance->widget);
}

static Eina_Bool
_pager_cb_event_zone_desk_count_set(void *data, int type, void *event)
{
  Eina_List *l;
  Instance * inst;
  EINA_LIST_FOREACH(tagslist_conf->instances, l, inst)
  {
    _instance_free(inst);
    _fill_desks_widget(inst, inst->gcc->gadcon);
    if (inst->widget) _gc_orient(inst->gcc, inst->gcc->gadcon->orient);
  }   
  return ECORE_CALLBACK_PASS_ON; 
}

static Eina_Bool
_pager_cb_event_desk_show(void *data, int type, void *event)
{
  Eina_List *l;
  Instance * inst;
  EINA_LIST_FOREACH(tagslist_conf->instances, l, inst)
  {
    _update_selected_desk_widget(inst, inst->gcc->gadcon);
    if (inst->widget) _gc_orient(inst->gcc, inst->gcc->gadcon->orient);
  }   
  return ECORE_CALLBACK_PASS_ON; 
}

static Eina_Bool
_pager_cb_event_desk_name_change(void *data, int type, void *event)
{
  E_Event_Desk_Name_Change *ev = event;
  Eina_List *l;
  Instance * inst;
  EINA_LIST_FOREACH(tagslist_conf->instances, l, inst)
  {
    _update_names_desks_widget(inst, inst->gcc->gadcon, ev->desk);
    if (inst->widget) _gc_orient(inst->gcc, inst->gcc->gadcon->orient);
  }   
  return ECORE_CALLBACK_PASS_ON;  
}

static void
_pager_desk_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj , void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   E_Zone * zone;
   zone = (E_Zone *) data;
   ev = event_info;
   e_zone_desk_linear_flip_by(zone, ev->z);
}

static void
_pager_desk_cb_mouse_up(void *data, Evas *e, Evas_Object *obj , void *event_info) 
{
  Evas_Event_Mouse_Up *ev;
  Desk * desk_object;
  ev = event_info;
  desk_object = (Desk *) data;
  if(ev->button ==1) // && (!pd->pager->dragging) && (!pd->pager->just_dragged)) 
    e_desk_show(desk_object->desk);
}

static Config_Item *
_config_item_get(const char *id) 
{
  Eina_List *l = NULL;
  Config_Item *ci;


  if (!id)
  {
	  /* Create id */
	  char buf[128];
    int num = 0;
    if (tagslist_conf->items)
	  {
	    ci = eina_list_last(tagslist_conf->items)->data;
      const char *p;
	    p = strrchr(ci->id, '.');
	    if (p) num = atoi(p + 1) + 1;
	  }
	  snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, num);
	  id = buf;
  }
  else
  {
	  for (l = tagslist_conf->items; l; l = l->next) 
	  {
	    ci = l->data;
	    if (!ci->id) continue;
	    if (!strcmp(ci->id, id))
	    {
		    return ci;
	    }
	  }
  }

  ci = E_NEW(Config_Item, 1);
  ci->id = eina_stringshare_add(id);

  tagslist_conf->items = eina_list_append(tagslist_conf->items, ci);
  return ci;
}

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Tagslist" };

EAPI void *
e_modapi_init(E_Module *m) 
{
  char buf[4096];
  #if HAVE_LOCALE_H
    setlocale(LC_ALL, "");
  #endif
  snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
  bindtextdomain(PACKAGE, buf);
  bind_textdomain_codeset(PACKAGE, "UTF-8");

  conf_item_edd = E_CONFIG_DD_NEW("Tagslist_Config_Item", Config_Item);
  conf_edd = E_CONFIG_DD_NEW("Tagslist_Config", Config);
   
  #undef T
  #define T Config_Item
  #undef D
  #define D conf_item_edd
  E_CONFIG_VAL(D, T, id, STR);
   
  #undef T
  #define T Config
  #undef D
  #define D conf_edd
  E_CONFIG_LIST(D, T, items, conf_item_edd);
   
  tagslist_conf = e_config_domain_load("module.tagslist", conf_edd);
  if (!tagslist_conf) 
  {
	  Config_Item *ci;
	
	  tagslist_conf = E_NEW(Config, 1);
	  ci = E_NEW(Config_Item, 1);
	  ci->id = eina_stringshare_add("0");
	
	  tagslist_conf->items = eina_list_append(tagslist_conf->items, ci);
  }
   
  E_LIST_HANDLER_APPEND(tagslist_conf->handlers, E_EVENT_DESK_NAME_CHANGE, _pager_cb_event_desk_name_change, NULL);
  E_LIST_HANDLER_APPEND(tagslist_conf->handlers, E_EVENT_DESK_SHOW, _pager_cb_event_desk_show, NULL);
  E_LIST_HANDLER_APPEND(tagslist_conf->handlers, E_EVENT_ZONE_DESK_COUNT_SET, _pager_cb_event_zone_desk_count_set, NULL);
  
  tagslist_conf->module = m;
  e_gadcon_provider_register(&_gc_class);
  return m;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
  tagslist_conf->module = NULL;
  e_gadcon_provider_unregister(&_gc_class);
  /*Delete dialog for configuration if needed*/
  /*if (tagslist_conf->config_dialog)
     e_object_del(E_OBJECT(tagslist_conf->config_dialog));
  */
  /*delete main menu of the widget if needed*/
  /*if (tagslist_conf->menu) 
  {
	  e_menu_post_deactivate_callback_set(tagslist_conf->menu, NULL, NULL);
	  e_object_del(E_OBJECT(tagslist_conf->menu));
	  tagslist_conf->menu = NULL;
  }*/
  /*Remove items*/
  while (tagslist_conf->items) 
  {
	  Config_Item *ci;
	
	  ci = tagslist_conf->items->data;
	  if (ci->id) eina_stringshare_del(ci->id);
	  tagslist_conf->items = eina_list_remove_list(tagslist_conf->items, tagslist_conf->items);
	  E_FREE(ci);
  }

  E_FREE(tagslist_conf);
  E_CONFIG_DD_FREE(conf_item_edd);
  E_CONFIG_DD_FREE(conf_edd);
  return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.tagslist", conf_edd, tagslist_conf);
   return 1;
}
