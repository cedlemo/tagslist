##Tagslist
This is a module for Enlightenment 18 that displays the names of the desktops and 
allows to switch between them. Tagslist is fully compatible with the pager module.
Tagslist is updated when you add/remove desktops with pager or their change names. 

I created Tagslist because I needed a widget like the tagslist in Awesome WM. 
Tagslist is not intended to replace the pager module. In fact, I consider that 
Tagslist and the pager module are complementary. The pager module
is more usefull with a big size (on the desktop) and is not very usable on a small
desktop bar (enlightenement shelf).
###Overview
<img src="https://raw.github.com/cedlemo/tagslist/master/screenshot.jpg" width="576" height="324"     alt="Screenshot">

In this screenshot on the top left you can see the tagslist module in his basic configuration.

###Installation:
- Get the sources:

```
git clone git@github.com:cedlemo/tagslit.git
```
- Create Makefile (the --enable-homedir-install option force to install the module
in your user profile):

```
cd tagslist
./autogen.sh --enable-homedir-install
make install
```

- Activate the module in Enlightenment 18 
  - go to the modules window (Main Menu > Configuration > Control Panel > Extensions > Modules)
  - select system modules, find and activate tagslist.
  - on your shelf (desktop bar) right click and add tagslist.

###Usage:
Tagslist supports mouse events:
- Click on the name of a desktop to switch to this desktop.
- Use mouse wheel to swith to next/prev desktop.

###Customize the apparence:
You can easily modify the apparence of this module via the tagslist.edc file.
This one must have a part "label" which must be a text type. You can manage, for now,
two types of signals : "e,state,unselected" and "e,state,selected" which are emitted
when a desktop is not the current one and when a desktop is the selected/current desktop.

- Disable the tagslist module.
- Edit, modify and save the tagslist.edc in the tagslist directory.
- Regenerate and reinstall the module:

```
make install
```
- Re-activate the module.
- If there are no differences (cache problem maybe) try to relaunch E18 (no need to logout).

###TODO:
- Add client drag in (when you drag a client icon from the desktop to a tagslist desktop, the client
will be moved to the related desktop).
- Add a have client event ( when a desktop have window in it user could create 
an edc file that react to that)
- Add focused client event ( the edc file could have a specific configuration/part
 for the desktop that contains the focused client)
- Add a menu allowing user to make basic configuration.
cedlemo at gmx dot com
