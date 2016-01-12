#!/usr/bin/env python
#

import pygtk
pygtk.require('2.0')
import gtk
import appindicator
import collections
import sys

import razer.daemon_dbus

### Primary colour used for effects
rgb_primary_red = 0
rgb_primary_green = 255
rgb_primary_blue = 0
### Colour used for breath mode.
rgb_secondary_red = 0
rgb_secondary_green = 0
rgb_secondary_blue = 255
# Current effect in use at start not currently known.
ACTIVE_EFFECT = 'unknown'

# TODO: Port to Python3 ?

class AppIndicator:
    def __init__(self):
        self.daemon = razer.daemon_dbus.DaemonInterface()
        self.ind = appindicator.Indicator ("example-simple-client", "/usr/share/razer_tray_applet/tray_icon.png", appindicator.CATEGORY_APPLICATION_STATUS)
        self.ind.set_status (appindicator.STATUS_ACTIVE)

        # create a menu
        self.menu = gtk.Menu()
        # Create effects submenu
        effect_item = gtk.MenuItem("Effects")
        effect_item.show()
        self.effect_menu = gtk.Menu()
        self.effect_menu.show()
        effect_item.set_submenu(self.effect_menu)
        self.menu.append(effect_item)
        # Create brightness submenu
        brightness_item = gtk.MenuItem("Brightness")
        brightness_item.show()
        self.brightness_menu = gtk.Menu()
        self.brightness_menu.show()
        brightness_item.set_submenu(self.brightness_menu)
        self.menu.append(brightness_item)
        # Create game mode submenu
        game_mode_item = gtk.MenuItem("Game Mode")
        game_mode_item.show()
        self.game_mode_menu = gtk.Menu()
        self.game_mode_menu.show()
        game_mode_item.set_submenu(self.game_mode_menu)
        self.menu.append(game_mode_item)



        self.effect_menu_items = collections.OrderedDict()
        self.effect_menu_items["spectrum"] = gtk.RadioMenuItem(None, "Spectrum Effect")
        self.effect_menu_items["wave"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Wave Effect")
        self.effect_menu_items["reactive"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Reactive Effect")
        self.effect_menu_items["breath"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Breath Effect")
        self.effect_menu_items["static"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Static Effect")
        self.effect_menu_items["none"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "No Effect (Off)")

        for button_key, button in self.effect_menu_items.items():
            button.connect("activate", self.menuitem_keyboard_effect_response, button_key)
            button.show()
            self.effect_menu.append(button)

        self.brightness_menu_items = collections.OrderedDict()
        self.brightness_menu_items[255] = gtk.RadioMenuItem(None, "Brightness 100%")
        self.brightness_menu_items[192] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 75%")
        self.brightness_menu_items[128] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 50%")
        self.brightness_menu_items[64] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 25%")
        self.brightness_menu_items[0] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 0%")

        for button_key, button in self.brightness_menu_items.items():
            button.connect("activate", self.menuitem_brightness_response, button_key)
            button.show()
            self.brightness_menu.append(button)

        enable_game_mode_button = gtk.MenuItem("Enable Game Mode")
        enable_game_mode_button.connect("activate", self.menuitem_enable_game_mode, True)
        enable_game_mode_button.show()
        self.game_mode_menu.append(enable_game_mode_button)

        disable_game_mode_button = gtk.MenuItem("Disable Game Mode")
        disable_game_mode_button.connect("activate", self.menuitem_enable_game_mode, False)
        disable_game_mode_button.show()
        self.game_mode_menu.append(disable_game_mode_button)

        sep1 = gtk.SeparatorMenuItem()
        sep1.show()
        self.menu.append(sep1)

        macro_button = gtk.MenuItem("Enable Macro Keys")
        macro_button.connect("activate", self.menuitem_enable_macro_buttons_response, "macros")
        macro_button.show()
        self.menu.append(macro_button)

        sep2 = gtk.SeparatorMenuItem()
        sep2.show()
        self.menu.append(sep2)

        color_button = gtk.MenuItem("Change Primary Colour...")
        color_status = gtk.MenuItem(str(rgb_primary_red) + ', ' + str(rgb_primary_green) + ', ' + str(rgb_primary_blue))
        color_button.connect("activate", self.set_color, 'primary', color_status)
        self.menu.append(color_button)
        self.menu.append(color_status)
        color_button.show()
        color_status.show()
        color_status.set_sensitive(False)

        sep3 = gtk.SeparatorMenuItem()
        sep3.show()
        self.menu.append(sep3)

        color2_button = gtk.MenuItem("Change Secondary Colour...")
        color2_status = gtk.MenuItem(str(rgb_secondary_red) + ', ' + str(rgb_secondary_green) + ', ' + str(rgb_secondary_blue))
        color2_button.connect("activate", self.set_color, 'secondary', color_status)
        self.menu.append(color2_button)
        self.menu.append(color2_status)
        color2_button.show()
        color2_status.show()
        color2_status.set_sensitive(False)

        sep4 = gtk.SeparatorMenuItem()
        sep4.show()
        self.menu.append(sep4)

        quit_button = gtk.MenuItem("Quit")
        quit_button.connect("activate", self.quit, "quit")
        quit_button.show()
        self.menu.append(quit_button)

        self.menu.show()
        self.ind.set_menu(self.menu)

    def quit(self, widget, data=None):
        gtk.main_quit()

    def menuitem_keyboard_effect_response(self, widget, effect_type):
        global ACTIVE_EFFECT
        ACTIVE_EFFECT = effect_type
        if widget.active:
            if effect_type == "breath":
                self.daemon.set_effect('breath', rgb_primary_red, rgb_primary_green, rgb_primary_blue, rgb_secondary_red, rgb_secondary_green, rgb_secondary_blue)
            elif effect_type == "none":
                self.daemon.set_effect('none',1)
            elif effect_type == "reactive":
                self.daemon.set_effect('reactive', 1, rgb_primary_red, rgb_primary_green, rgb_primary_blue)
            elif effect_type == "spectrum":
                self.daemon.set_effect('spectrum')
            elif effect_type == "static":
                self.daemon.set_effect('static', rgb_primary_red, rgb_primary_green, rgb_primary_blue)
            elif effect_type == "wave":
                self.daemon.set_effect('wave', 1)

    def menuitem_brightness_response(self, widget, brightness):
        self.daemon.set_brightness(brightness)

    def menuitem_enable_macro_buttons_response(self, widget, string):
        self.daemon.marco_keys(True)


    def menuitem_enable_game_mode(self, widget, enable):
        if enable:
            self.daemon.game_mode(True)
        else:
            self.daemon.game_mode(False)

    def set_color(self, widget, target_color, color_status):
        global rgb_primary_red, rgb_primary_green, rgb_primary_blue
        global rgb_secondary_red, rgb_secondary_green, rgb_secondary_blue
        global ACTIVE_EFFECT

        # Create a colour selection dialog
        if target_color == 'primary':
            string_output = str(rgb_primary_red) + ", " + str(rgb_primary_green) + ", " + str(rgb_primary_blue)
            print("[Colour] Current primary: " + string_output)
            colorseldlg = gtk.ColorSelectionDialog('Change Primary Colour')
        elif target_color == 'secondary':
            string_output = str(rgb_secondary_red) + ", " + str(rgb_secondary_green) + ", " + str(rgb_secondary_blue)
            print("[Colour] Current secondary: "+ string_output)
            colorseldlg = gtk.ColorSelectionDialog('Change Secondary Colour')

        response = colorseldlg.run()

        # If new colour is chosen.
        if response == gtk.RESPONSE_OK:
            colorsel = colorseldlg.colorsel
            colorhex = colorsel.get_current_color()
            color_rgb = gtk.gdk.Color(str(colorhex))
            # Returns value between 0.0 - 1.0 * 255 = 8-bit RGB Value
            red = int(getattr(color_rgb, 'red_float') * 255)
            green = int(getattr(color_rgb, 'green_float') * 255)
            blue = int(getattr(color_rgb, 'blue_float') * 255)
            string_output = str(red) + ", " + str(green) + ", " + str(blue)

            # Set the relevant colour
            if target_color == 'primary':
                rgb_primary_red = red
                rgb_primary_green = green
                rgb_primary_blue = blue
                color_status.set_label(string_output)
                print "[Colour] Set primary to: " + string_output + " (" + str(colorhex) + ")"

            elif target_color == 'secondary':
                rgb_secondary_red = red
                rgb_secondary_green = green
                rgb_secondary_blue = blue
                color_status.set_label(string_output)
                print "[Colour] Set secondary to: " + str(string_output) + " (" + str(colorhex) + ")"

            # If 'static', 'reactive' or 'breath' mode is set, refresh the effect.
            if ACTIVE_EFFECT == 'static':
                print "[Colour] Refreshing Static Mode"
                self.menuitem_keyboard_effect_response(self.effect_menu_items["static"], 'static')
            elif ACTIVE_EFFECT == 'reactive':
                print "[Colour] Refreshing Reactive Mode"
                self.menuitem_keyboard_effect_response(self.effect_menu_items["reactive"], 'reactive')
            elif ACTIVE_EFFECT == 'breath':
                print "[Colour] Refreshing Breath Mode"
                self.menuitem_keyboard_effect_response(self.effect_menu_items["breath"], 'breath')

        colorseldlg.destroy()


def main():
    try:
        gtk.main()
    except KeyboardInterrupt:
        pass # Dont error if Ctrl+C'd
    sys.exit(0)

if __name__ == "__main__":
    indicator = AppIndicator()
    main()
