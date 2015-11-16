#!/usr/bin/env python
#

import pygtk
pygtk.require('2.0')
import gtk
import appindicator
import dbus
import collections

STATIC_RGB = [255, 0, 255]

class AppIndicatorExample:
    def __init__(self):
        # Load up teh DBUS
        system_bus = dbus.SystemBus()
        self.dbus_daemon_object = system_bus.get_object("org.voyagerproject.razer.daemon", "/")

        # Provides:
        # breath(byte red, byte green, byte blue)
        # none()
        # reactive(byte red, byte green, byte blue)
        # spectrum()
        # static(byte red, byte green, byte blue)
        # wave(byte direction)
        self.dbus_driver_effect_object = dbus.Interface(self.dbus_daemon_object, "org.voyagerproject.razer.daemon.driver_effect")

        # Provides:
        # enable_macro_keys()
        # raw_keyboard_brightness(byte brightness)
        self.dbus_daemon_controls = dbus.Interface(self.dbus_daemon_object, "org.voyagerproject.razer.daemon")

        self.ind = appindicator.Indicator ("example-simple-client", "/usr/share/razer_tray_applet/razer_icon.png", appindicator.CATEGORY_APPLICATION_STATUS)
        self.ind.set_status (appindicator.STATUS_ACTIVE)

        # create a menu
        self.menu = gtk.Menu()

        self.effect_menu_items = collections.OrderedDict()
        self.effect_menu_items["spectrum"] = gtk.RadioMenuItem(None, "Spectrum Effect")
        self.effect_menu_items["wave"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Wave Effect")
        self.effect_menu_items["reactive"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Reactive Effect")
        self.effect_menu_items["breath"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Breath Effect")
        self.effect_menu_items["static"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "Static Effect")
        self.effect_menu_items["none"] = gtk.RadioMenuItem(self.effect_menu_items["spectrum"], "No Effect (Off)")

        self.dbus_driver_effect_object.static(*STATIC_RGB)
        self.effect_menu_items["static"].set_active(True)

        for button_key, button in self.effect_menu_items.items():
            button.connect("activate", self.menuitem_keyboard_effect_response, button_key)
            button.show()
            self.menu.append(button)

        sep = gtk.SeparatorMenuItem()
        sep.show()
        self.menu.append(sep)

        self.brightness_menu_items = collections.OrderedDict()
        self.brightness_menu_items[255] = gtk.RadioMenuItem(None, "Brightness 100%")
        self.brightness_menu_items[192] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 75%")
        self.brightness_menu_items[128] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 50%")
        self.brightness_menu_items[64] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 25%")
        self.brightness_menu_items[0] = gtk.RadioMenuItem(self.brightness_menu_items[255], "Brightness 0%")

        for button_key, button in self.brightness_menu_items.items():
            button.connect("activate", self.menuitem_brightness_response, button_key)
            button.show()
            self.menu.append(button)

        sep2 = gtk.SeparatorMenuItem()
        sep2.show()
        self.menu.append(sep2)

        macro_button = gtk.MenuItem("Enable Macro Keys")
        macro_button.connect("activate", self.menuitem_enable_macro_buttons_response, "macros")
        macro_button.show()
        self.menu.append(macro_button)

        sep3 = gtk.SeparatorMenuItem()
        sep3.show()
        self.menu.append(sep3)

        quit_button = gtk.MenuItem("Quit")
        quit_button.connect("activate", self.quit, "quit")
        quit_button.show()
        self.menu.append(quit_button)

        self.menu.show()
        self.ind.set_menu(self.menu)

    def quit(self, widget, data=None):
        gtk.main_quit()

    def menuitem_keyboard_effect_response(self, widget, effect_type):
        if widget.active:
            if effect_type == "breath":
                print "[Effect] Breath mode"
                self.dbus_driver_effect_object.breath(*STATIC_RGB)
            elif effect_type == "none":
                print "[Effect] No effect (off)"
                self.dbus_driver_effect_object.none()
            elif effect_type == "reactive":
                print "[Effect] Reactive mode"
                self.dbus_driver_effect_object.reactive(*STATIC_RGB)
            elif effect_type == "spectrum":
                print "[Effect] Spectrum mode"
                self.dbus_driver_effect_object.spectrum()
            elif effect_type == "static":
                print "[Effect] Static mode"
                self.dbus_driver_effect_object.static(*STATIC_RGB)
            elif effect_type == "wave":
                print "[Effect] Wave mode"
                self.dbus_driver_effect_object.wave(1)

    def menuitem_brightness_response(self, widget, brightness):
        print "[Brightness] {0}%".format(round((100/255) * brightness, 0))
        self.dbus_daemon_controls.raw_keyboard_brightness(brightness)

    def menuitem_enable_macro_buttons_response(self, widget, string):
        print "[Driver] Enable macro keys"
        self.dbus_daemon_controls.enable_macro_keys()



def main():
    gtk.main()
    return 0

if __name__ == "__main__":
    indicator = AppIndicatorExample()
    main()
