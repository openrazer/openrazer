#!/usr/bin/env python3
#
# This tray applet is free software: you can redistribute it and/or modify
# it under the temms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This tray applet is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this tray applet. If not, see <http://www.gnu.org/licenses/>.
#
# Copyright (C) 2015-2016 Terry Cain <terry@terrys-home.co.uk>
#               2015-2016 Luke Horwell <lukehorwell37+code@gmail.com>
#

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Gdk', '3.0')
gi.require_version('AppIndicator3', '0.1')
from gi.repository import Gtk, Gdk, AppIndicator3 as appindicator
import collections, os
import sys, signal

import razer.daemon_dbus
import razer.keyboard
import razer.preferences
import razer.profiles


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
        # Initialize the DBUS Daemon
        self.daemon = razer.daemon_dbus.DaemonInterface()

        # Store Colour and active effect
        self.colour = (255, 255, 255)
        self.secondary_colour = (0, 0, 0)
        self.active_effect = None

        # Read preferences.
        self.preferences = razer.preferences.ChromaPreferences()

        # Determine which icon to display.
        icon_type = self.preferences.get_pref('tray_applet','icon_type','system')
        icon_fallback = 'ibus-keyboard'
        if icon_type == 'system' or icon_type == '' or icon_type == None:
            icon = icon_fallback

        if icon_type == 'logo':
            # TODO: Retrieve icon relative to the program.
            logo_path = '/usr/share/razer_tray_applet/tray_icon.png'

        if icon_type == 'animated':
            # TODO: Retrieve icon relative to the program.
            logo_path = '/usr/share/razer_tray_applet/tray_icon_animated.gif'

        if icon_type == 'logo' or icon_type == 'animated':
            if os.path.exists(logo_path):
                icon = logo_path
            else:
                print('Default tray icon is missing at "' + logo_path + '". Using fallback.')
                icon = icon_fallback

        if icon_type == 'custom':
            icon_path = self.preferences.get_pref('tray_applet','icon_path')
            if not os.path.exists(icon_path):
                print('Custom icon "' + icon_path + '" could not be found. Using fallback.')
                icon = icon_fallback
            else:
                icon = icon_path

        if not icon:
            print('Malformed icon type: "' + icon_type + '"')
            icon = icon_fallback

        # Create the indicator
        self.ind = appindicator.Indicator.new("razer-keyboard-indicator", icon, appindicator.IndicatorCategory.APPLICATION_STATUS)
        self.ind.set_status (appindicator.IndicatorStatus.ACTIVE)

        # Create a menu
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
        self.effect_menu_items["spectrum"] = Gtk.RadioMenuItem(group=None, label="Spectrum Effect")
        self.effect_menu_items["wave"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Wave Effect")
        self.effect_menu_items["reactive"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Reactive Effect")
        self.effect_menu_items["breath_r"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Breath Effect (Random)")
        self.effect_menu_items["breath_s"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Breath Effect (Single Colour)")
        self.effect_menu_items["breath_d"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Breath Effect (Dual Colour)")
        self.effect_menu_items["static"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="Static Effect")
        self.effect_menu_items["none"] = Gtk.RadioMenuItem(group=self.effect_menu_items["spectrum"], label="No Effect (Off)")

        for button_key, button in self.effect_menu_items.items():
            button.connect("activate", self.menuitem_keyboard_effect_response, button_key)
            button.show()
            self.effect_menu.append(button)

        self.brightness_menu_items = collections.OrderedDict()
        self.brightness_menu_items[255] = Gtk.RadioMenuItem(group=None, label="Brightness 100%")
        self.brightness_menu_items[192] = Gtk.RadioMenuItem(group=self.brightness_menu_items[255], label="Brightness 75%")
        self.brightness_menu_items[128] = Gtk.RadioMenuItem(group=self.brightness_menu_items[255], label="Brightness 50%")
        self.brightness_menu_items[64] = Gtk.RadioMenuItem(group=self.brightness_menu_items[255], label="Brightness 25%")
        self.brightness_menu_items[0] = Gtk.RadioMenuItem(group=self.brightness_menu_items[255], label="Brightness 0%")

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

        self.primary_colour_button = Gtk.MenuItem("Change Primary Colour... ({0})".format(AppIndicator.colour_to_hex(self.colour)))
        self.primary_colour_button.connect("activate", self.set_static_colour, 1)
        self.secondary_colour_button = Gtk.MenuItem("Change Secondary Colour... ({0})".format(AppIndicator.colour_to_hex(self.secondary_colour)))
        self.secondary_colour_button.connect("activate", self.set_static_colour, 2)
        self.menu.append(self.primary_colour_button)
        self.menu.append(self.secondary_colour_button)
        self.primary_colour_button.show()
        self.secondary_colour_button.show()

        sep3 = Gtk.SeparatorMenuItem()
        sep3.show()
        self.menu.append(sep3)

        if os.path.exists('/usr/share/razer_chroma_controller/chroma_controller.py'):
            open_config_button = Gtk.MenuItem("Open Configuration Utility")
            open_config_button.connect("activate", self.menuitem_open_config)
            open_config_button.show()
            self.menu.append(open_config_button)

            sep4 = Gtk.SeparatorMenuItem()
            sep4.show()
            self.menu.append(sep4)
        else:
            print('Chroma Controller GUI not installed. Not integrating to menu.')

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
        if widget.get_active():
            if effect_type == "breath_r":
                self.daemon.set_effect('breath', 1)
            elif effect_type == "breath_s":
                self.daemon.set_effect('breath', *self.colour)
            elif effect_type == "breath_d":
                self.daemon.set_effect('breath', self.colour[0], self.colour[1], self.colour[2], self.secondary_colour[0], self.secondary_colour[1], self.secondary_colour[2])
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

    def set_static_colour(self, widget, colour_index):
        """
        Sets the colour for effects.

        :param widget: MenuItem object
        :type widget: Gtk.MenuItem

        :param colour_index: index of colour
        :type colour_index: int
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

            if colour_index == 1:
                self.colour = razer.keyboard.KeyboardColour.gdk_colour_to_rgb(color_rgb)
                self.primary_colour_button.set_label("Change Primary Colour... ({0})".format(AppIndicator.colour_to_hex(self.colour)))
                print("[Change Primary Colour] New: {0}".format(AppIndicator.colour_to_hex(self.colour)))
            else:
                self.secondary_colour = razer.keyboard.KeyboardColour.gdk_colour_to_rgb(color_rgb)
                self.secondary_colour_button.set_label("Change Primary Colour... ({0})".format(AppIndicator.colour_to_hex(self.secondary_colour)))
                print("[Change Secondary Colour] New: {0}".format(AppIndicator.colour_to_hex(self.secondary_colour)))

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

    def menuitem_open_config(self, widget):
        """
        Opens the Chroma Configuration Tool.
        """
        os.system('/usr/share/razer_chroma_controller/chroma_controller.py')


def main():
    """
    Main function
    """
    # Kill the process when CTRL+C'd.
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    Gtk.main()
    sys.exit(0)

if __name__ == "__main__":
    indicator = AppIndicator()
    main()
