#!/usr/bin/env python3
#

from gi.repository import Gtk, Gdk, AppIndicator3 as appindicator
import collections
import sys

import razer.daemon_dbus, razer.keyboard


class AppIndicator:
    """
    Simple indicator applet which sits in the tray.
    """

    @staticmethod
    def colour_to_hex(colour):
        """
        Converts a tuple to #RRGGBB format

        :param colour: Tuple or list of 3 integers
        :type colour: tuple or list

        :return: Hex String
        :rtype: str
        """
        return "#{0:02X}{1:02X}{2:02X}".format(*colour)

    def __init__(self):
        self.daemon = razer.daemon_dbus.DaemonInterface()
        self.ind = appindicator.Indicator.new("example-simple-client", "/usr/share/razer_tray_applet/tray_icon.png", appindicator.IndicatorCategory.APPLICATION_STATUS)
        self.ind.set_status (appindicator.IndicatorStatus.ACTIVE)

        # Store Colour and active effect
        self.colour = (255, 255, 255)
        self.active_effect = None

        # create a menu
        self.menu = Gtk.Menu()
        # Create effects submenu
        effect_item = Gtk.MenuItem("Effects")
        effect_item.show()
        self.effect_menu = Gtk.Menu()
        self.effect_menu.show()
        effect_item.set_submenu(self.effect_menu)
        self.menu.append(effect_item)
        # Create brightness submenu
        brightness_item = Gtk.MenuItem("Brightness")
        brightness_item.show()
        self.brightness_menu = Gtk.Menu()
        self.brightness_menu.show()
        brightness_item.set_submenu(self.brightness_menu)
        self.menu.append(brightness_item)
        # Create game mode submenu
        game_mode_item = Gtk.MenuItem("Game Mode")
        game_mode_item.show()
        self.game_mode_menu = Gtk.Menu()
        self.game_mode_menu.show()
        game_mode_item.set_submenu(self.game_mode_menu)
        self.menu.append(game_mode_item)

        self.effect_menu_items = collections.OrderedDict()
        self.effect_menu_items["spectrum"] = Gtk.RadioMenuItem(None, "Spectrum Effect")
        self.effect_menu_items["wave"] = Gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Wave Effect")
        self.effect_menu_items["reactive"] = Gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Reactive Effect")
        self.effect_menu_items["breath"] = Gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Breath Effect")
        self.effect_menu_items["static"] = Gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Static Effect")
        self.effect_menu_items["none"] = Gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "No Effect (Off)")

        for button_key, button in self.effect_menu_items.items():
            button.connect("activate", self.menuitem_keyboard_effect_response, button_key)
            button.show()
            self.effect_menu.append(button)

        self.brightness_menu_items = collections.OrderedDict()
        self.brightness_menu_items[255] = Gtk.RadioMenuItem(None, "Brightness 100%")
        self.brightness_menu_items[192] = Gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 75%")
        self.brightness_menu_items[128] = Gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 50%")
        self.brightness_menu_items[64] = Gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 25%")
        self.brightness_menu_items[0] = Gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 0%")

        for button_key, button in self.brightness_menu_items.items():
            button.connect("activate", self.menuitem_brightness_response, button_key)
            button.show()
            self.brightness_menu.append(button)

        enable_game_mode_button = Gtk.MenuItem("Enable Game Mode")
        enable_game_mode_button.connect("activate", self.menuitem_enable_game_mode, True)
        enable_game_mode_button.show()
        self.game_mode_menu.append(enable_game_mode_button)

        disable_game_mode_button = Gtk.MenuItem("Disable Game Mode")
        disable_game_mode_button.connect("activate", self.menuitem_enable_game_mode, False)
        disable_game_mode_button.show()
        self.game_mode_menu.append(disable_game_mode_button)

        sep1 = Gtk.SeparatorMenuItem()
        sep1.show()
        self.menu.append(sep1)

        macro_button = Gtk.MenuItem("Enable Macro Keys")
        macro_button.connect("activate", self.menuitem_enable_macro_buttons_response, "macros")
        macro_button.show()
        self.menu.append(macro_button)

        sep2 = Gtk.SeparatorMenuItem()
        sep2.show()
        self.menu.append(sep2)

        color_button = Gtk.MenuItem("Change Colour...")
        color_status = Gtk.MenuItem(AppIndicator.colour_to_hex(self.colour))
        color_button.connect("activate", self.set_static_colour, color_status)
        self.menu.append(color_button)
        self.menu.append(color_status)
        color_button.show()
        color_status.show()
        color_status.set_sensitive(False)

        sep3 = Gtk.SeparatorMenuItem()
        sep3.show()
        self.menu.append(sep3)

        quit_button = Gtk.MenuItem("Quit")
        quit_button.connect("activate", self.quit, "quit")
        quit_button.show()
        self.menu.append(quit_button)

        self.menu.show()
        self.ind.set_menu(self.menu)

    def quit(self, widget, data=None):
        """
        Quits the application
        """
        Gtk.main_quit()

    def menuitem_keyboard_effect_response(self, widget, effect_type):
        """
        Click event for keyboard effects in the menu

        :param widget: MenuItem object
        :type widget: Gtk.RadioMenuItem

        :param effect_type: Type of effect
        :type effect_type: str
        """
        self.active_effect = effect_type
        if widget.active:
            if effect_type == "breath":
                self.daemon.set_effect('breath', *self.colour)
            elif effect_type == "none":
                self.daemon.set_effect('none', 1)
            elif effect_type == "reactive":
                self.daemon.set_effect('reactive', 1, *self.colour)
            elif effect_type == "spectrum":
                self.daemon.set_effect('spectrum')
            elif effect_type == "static":
                self.daemon.set_effect('static', *self.colour)
            elif effect_type == "wave":
                self.daemon.set_effect('wave', 1)

    def menuitem_brightness_response(self, widget, brightness):
        """
        Sets the brightness for the keyboard

        :param widget: MenuItem object
        :type widget: Gtk.RadioMenuItem

        :param brightness: Brightness value (scaled between 0-255)
        :type brightness: int
        """
        self.daemon.set_brightness(brightness)

    def menuitem_enable_macro_buttons_response(self, widget, string):
        """
        Enable macro keys

        :param widget: MenuItem object
        :type widget: Gtk.MenuItem

        :param string: Unused
        :type string: str
        """
        self.daemon.marco_keys(True)


    def menuitem_enable_game_mode(self, widget, enable):
        """
        Enables or disabled Game mode

        :param widget: MenuItem object
        :type widget: Gtk.MenuItem

        :param enable: Boolean
        :type enable: bool
        """
        self.daemon.game_mode(enable)

    def set_static_colour(self, widget, colour_menuitem):
        """
        Sets the colour for effects.

        :param widget: MenuItem object
        :type widget: Gtk.MenuItem

        :param colour_menuitem: MenuItem object
        :type colour_menuitem: Gtk.MenuItem
        """
        print("[Change Colour] Current: {0}".format(AppIndicator.colour_to_hex(self.colour)))

        # Create a colour selection dialog
        #colorsel = gtk.ColorSelection()
        color_selection_dlg = Gtk.ColorSelectionDialog('Change Static Colour')
        color_selection_result = color_selection_dlg.run()

        # If new colour is chosen.
        if color_selection_result == Gtk.ResponseType.OK:
            color_rgb = color_selection_dlg.get_color_selection().get_current_color()
            # Returns value between 0.0 - 1.0 * 255 = 8-bit RGB Value
            self.colour = razer.keyboard.Keyboard.gdk_colour_to_rgb(color_rgb)
            colour_menuitem.set_label(AppIndicator.colour_to_hex(self.colour)) # *array unpacks to seperate arguments
            print("[Change Colour] New: {0}".format(AppIndicator.colour_to_hex(self.colour)))

            # If 'static', 'reactive' or 'breath' mode is set, refresh the effect.
            if self.active_effect == 'static':
                print("[Change Colour] Refreshing Static Mode")
                self.menuitem_keyboard_effect_response(self.effect_menu_items["static"], 'static')
            elif self.active_effect == 'reactive':
                print("[Change Colour] Refreshing Reactive Mode")
                self.menuitem_keyboard_effect_response(self.effect_menu_items["reactive"], 'reactive')
            elif self.active_effect == 'breath':
                print("[Change Colour] Refreshing Breath Mode")
                self.menuitem_keyboard_effect_response(self.effect_menu_items["breath"], 'breath')

        color_selection_dlg.destroy()


def main():
    """
    Main function
    """
    try:
        Gtk.main()
    except KeyboardInterrupt:
        pass # Dont error if Ctrl+C'd
    sys.exit(0)

if __name__ == "__main__":
    indicator = AppIndicator()
    main()
