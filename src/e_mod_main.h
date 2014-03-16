#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

/*gettext helpers*/
#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(string) dgettext(PACKAGE, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif

/*Global config for module*/
typedef struct _Config Config;
struct _Config
{
   E_Module * module;
   //E_Config_Dialog *config_dialog;
   //E_Menu *menu;
   Eina_List *instances, *handlers, *items;
};

/*Config for instance*/
typedef struct _Config_Item Config_Item;
struct _Config_Item
{
   const char *id;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);

extern Config * tagslist_conf;

#endif
