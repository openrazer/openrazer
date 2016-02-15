#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Chroma Config Tool is free software: you can redistribute it and/or modify
# it under the temms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Chroma Config Tool is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Chroma Config Tool. If not, see <http://www.gnu.org/licenses/>.
#
# Copyright (C) 2015-2016 Luke Horwell <lukehorwell37+code@gmail.com>
#               2015-2016 Terry Cain <terry@terrys-home.co.uk>

import os, sys, signal, json
from gi.repository import Gtk, Gdk, WebKit
import razer.daemon_dbus
import razer.keyboard
import razer.preferences
import razer.profiles

# Where is the application being ran?
LOCATION_DATA = os.path.abspath(os.path.join(os.path.dirname(__file__), 'data/'))


class ChromaController(object):
    ##################################################
    # Page Switcher
    ##################################################
    def show_menu(self, page):
        self.current_page = page
        print("Opening page '" + page + "'")

        # Hide all footer buttons
        for element in ['retry', 'edit-save', 'edit-preview', 'cancel', 'close-window', 'pref-open', 'pref-save']:
            self.webkit.execute_script('$("#' + element + '").hide()')

        if page == 'chroma_menu':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_menu.html'))

        elif page == 'profile_editor':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_profiles.html'))

        elif page == 'preferences':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_preferences.html'))

        elif page == 'controller_devices':
            self.webkit.open(os.path.join(LOCATION_DATA, 'controller_devices.html'))
        else:
            print("Unknown menu '" + page + "'!")

    ##################################################
    # Page Initialization
    ##################################################
    def page_loaded(self, WebView, WebFrame):
        print('Running page post-actions for "' + self.current_page + '"...')
        if self.current_page == 'chroma_menu':
            self.webkit.execute_script('instantProfileSwitch = false;') # Unimplemented instant profile change option.
            self.webkit.execute_script("$('#profiles-activate').show()")
            self.refresh_profiles_list()

            # If there are multiple devices on the system, show the "switch" button.
            if self.multi_device_present:
                self.webkit.execute_script("$('#multi-device-switcher').show()")

            # Tell JavaScript whether live profile switching is enabled.
            if self.preferences.get_pref('live_switch') == 'true':
                self.webkit.execute_script('live_switch = true;')
                self.webkit.execute_script('$("#profiles-activate").hide();')
            else:
                self.webkit.execute_script('live_switch = false;')

        elif self.current_page == 'profile_editor':
            js_exec = WebkitJavaScriptExecutor(self.webkit)
            kb_callback = WebkitJavaScriptExecutor(None, wrapper="keyboard_obj.load(function(){{{0}}});")


            js_exec << 'change_header("Edit ' + self.open_this_profile + '")'

            kb_callback << "keyboard_obj.set_layout(\"kb-" + self.kb_layout + "\")"

            # Load profile into keyboard.
            profile_name = self.open_this_profile
            self.profiles.set_active_profile(profile_name)
            if self.preferences.get_pref('live_preview') == 'true':
                self.profiles.activate_profile_from_memory()
            self.profiles.get_active_profile().backup_configuration()

            for pos_y, row in enumerate(self.profiles.get_profile(profile_name).get_rows_raw()):
                for pos_x, rgb in enumerate(row):
                    js_string = "keyboard_obj.set_key_colour({0},{1},\"#{2:02X}{3:02X}{4:02X}\")".format(pos_y, pos_x, rgb.red, rgb.green, rgb.blue)
                    kb_callback << js_string

            # IF BLACKWIDOW ULTIMATE < 2016
            # OR BLACKWIDOW CHROMA
            # disable space key and FN
            kb_callback << "keyboard_obj.disable_key(5,7)"
            kb_callback << "keyboard_obj.disable_key(5,12)"
            # Hide preview button if live previewing is enabled.
            if self.preferences.get_pref('live_preview') == 'true':
                kb_callback << '$("#edit-preview").hide();'


            kb_callback << "$(\"#cancel\").attr({onclick: \"cmd('cancel-changes?"+ self.cancel_changes + "?" + profile_name + "')\"})"

            js_exec << kb_callback
            js_exec.exec()

        elif self.current_page == 'preferences':
            self.preferences.refresh_pref_page(self.webkit)

        elif self.current_page == 'controller_devices':
            self.detect_devices()

        else:
            print('No post actions necessary.')


    ##################################################
    # Reusable Page Functions
    ##################################################
    def refresh_profiles_list(self):
        self.webkit.execute_script('$("#profiles_list").html("")')
        for profile in self.profiles.get_profiles():
            item = "<option value='"+profile+"'>"+profile+"</option>"
            self.webkit.execute_script('$("#profiles_list").append("'+item+'")')

    ##################################################
    # Commands
    ##################################################
    def process_uri(self, view, frame, net_req, nav_act, pol_dec):
        uri = net_req.get_uri()

        if uri.startswith('cmd://'):
            frame.stop_loading()
            command = uri[6:]
            print("\nCommand: '"+command+"'")
            self.process_command(command)

        if uri.startswith('web://'):
            frame.stop_loading()
            web_url = uri[13:]
            print('Opening web address: "http://' + web_url+ '"')
            os.system('xdg-open "http://' + web_url + '"')

    def process_command(self, command):
        if command == 'quit':
            quit()

        ## Effects & Keyboard Controls
        elif command.startswith('brightness'):
            value = int(command[11:])
            self.daemon.set_brightness(value)

        elif command.startswith('effect'):
            enabled_options = []

            if command == 'effect-none':
                self.current_effect = "none"
                self.daemon.set_effect('none')

            elif command == 'effect-spectrum':
                self.current_effect = "spectrum"
                self.daemon.set_effect('spectrum')

            elif command.startswith('effect-wave'):
                self.current_effect = "wave"
                wave_direction = int(command.split('?')[1])
                self.daemon.set_effect('wave', wave_direction) # ?1 or ?2 for direction
                enabled_options = ['waves']

            elif command.startswith('effect-reactive'):
                self.current_effect = "reactive"
                if command.split('?')[1] == 'auto':
                    # Use the previous effect
                    self.daemon.set_effect('reactive', self.reactive_speed, self.primary_rgb.red, self.primary_rgb.green, self.primary_rgb.blue)
                else:
                    self.reactive_speed = int(command.split('?')[1])
                    self.daemon.set_effect('reactive', self.reactive_speed, self.primary_rgb.red, self.primary_rgb.green, self.primary_rgb.blue)
                enabled_options = ['rgb_primary', 'reactive']

            elif command.startswith('effect-breath'):
                breath_random = int(command.split('?')[1])
                if breath_random == 1:  # Random mode
                    self.current_effect = "breath?random"
                    self.daemon.set_effect('breath', 1)
                    enabled_options = ['breath-select']
                else:
                    self.current_effect = "breath?colours"
                    self.daemon.set_effect('breath',
                                           self.primary_rgb.red, self.primary_rgb.green, self.primary_rgb.blue,
                                           self.secondary_rgb.red, self.secondary_rgb.green, self.secondary_rgb.blue)
                    enabled_options = ['breath-random', 'rgb_primary', 'rgb_secondary']

            elif command == 'effect-static':
                self.current_effect = "static"
                self.daemon.set_effect('static', self.primary_rgb.red, self.primary_rgb.green, self.primary_rgb.blue)
                enabled_options = ['rgb_primary']

            # Fade between options for that effect, should it have been changed.
            if not self.current_effect == self.last_effect:
                # Effect changed, fade out all previous options.
                for element in ['rgb_primary', 'rgb_secondary', 'waves', 'reactive', 'breath-random', 'breath-select']:
                    self.webkit.execute_script('$("#' + element + '").fadeOut("fast")')

                # Fade in desired options for this effect.
                for element in enabled_options:
                    self.webkit.execute_script("setTimeout(function(){ $('#" + element + "').fadeIn('fast');}, 200)")
            self.last_effect = self.current_effect

        elif command == 'enable-marco-keys':
            self.daemon.marco_keys(True)
            self.webkit.execute_script('$("#marco-keys-enable").addClass("btn-disabled")')
            self.webkit.execute_script('$("#marco-keys-enable").html("In Use")')

        elif command == 'gamemode-enable':
            self.daemon.game_mode(True)
            self.webkit.execute_script('$("#game-mode-status").html("Enabled")')
            self.webkit.execute_script('$("#game-mode-enable").hide()')
            self.webkit.execute_script('$("#game-mode-disable").show()')

        elif command == 'gamemode-disable':
            self.daemon.game_mode(False)
            self.webkit.execute_script('$("#game-mode-status").html("Disabled")')
            self.webkit.execute_script('$("#game-mode-enable").show()')
            self.webkit.execute_script('$("#game-mode-disable").hide()')

        ## Changing colours for this session.
        elif command.startswith('ask-color'):
            colorseldlg = Gtk.ColorSelectionDialog("Choose a colour")
            colorsel = colorseldlg.get_color_selection()

            if colorseldlg.run() == Gtk.ResponseType.OK:
                color = colorsel.get_current_color()
                red = int(color.red / 256)
                green = int(color.green / 256)
                blue = int(color.blue / 256)
                element = command.split('?')[1]
                command = 'set-color?'+element+'?'+str(red)+'?'+str(green)+'?'+str(blue)
                self.process_command(command)

            colorseldlg.destroy()

        elif command.startswith('set-color'):
            # Expects 4 parameters separated by '?' in order: element, red, green, blue (RGB = 0-255)
            colors = command.split('set-color?')[1]
            element = colors.split('?')[0]
            red = int(colors.split('?')[1])
            green = int(colors.split('?')[2])
            blue = int(colors.split('?')[3])
            print("Set colour of '{0}' to RGB: {1}, {2}, {3}".format(element, red, green, blue))
            self.webkit.execute_script('$("#'+element+'_preview").css("background-color","rgba(' + str(red) + ',' + str(green) + ',' + str(blue) + ',1.0)")')
            self.webkit.execute_script('set_mode("set")')

            if element == 'rgb_primary':    # Primary effect colour
                self.primary_rgb.set((red, green, blue))
            elif element == 'rgb_secondary':   # Secondary effect colour (used for Breath mode)
                self.secondary_rgb.set((red, green, blue))
            elif element == 'rgb_tmp':      # Temporary colour while editing profiles.
                rgb_edit_red = red
                rgb_edit_green = green
                rgb_edit_blue = blue

            # Update static colour effects if currently in use.
            if self.current_effect == 'static':
                self.process_command('effect-static')
            elif self.current_effect == 'breath?colours':
                self.process_command('effect-breath?0')
            elif self.current_effect == 'reactive':
                self.process_command('effect-reactive?auto')

        ## Opening different pages
        elif command.startswith('cancel-changes'):
            if command.find('?') > -1:
                command, cancel_type, cancel_args = command.split('?')

                if cancel_type == "new-profile":
                    self.profiles.remove_profile(cancel_args, del_from_fs=False)
                    if self.preferences.get_pref('live_switch') == 'true' or self.preferences.get_pref('live_preview') == 'true':
                        self.daemon.set_custom_colour(self.old_profile)
                elif cancel_type == "edit-profile":
                    self.profiles.get_active_profile().restore_configuration()
                    if self.preferences.get_pref('live_switch') == 'true' or self.preferences.get_pref('live_preview') == 'true':
                        self.daemon.set_custom_colour(self.old_profile)

                self.webkit.execute_script("$(\"#cancel\").attr({onclick: \"cmd('cancel-changes')\"})")
            self.show_menu('chroma_menu')

        ## Preferences
        elif command == 'pref-open':
            self.show_menu('preferences')

        elif command.startswith('pref-set?'):
            # pref-set? <setting> ? <value>
            setting = command.split('?')[1]
            value = command.split('?')[2]
            self.preferences.set_pref(setting, value)

        elif command == 'pref-revert':
            print('Reverted preferences.')
            self.preferences.load_pref()
            self.show_menu('chroma_menu')

        elif command == 'pref-save':
            self.preferences.save_pref()
            self.show_menu('chroma_menu')

        ## Profile Editor / Management
        elif command.startswith('profile-edit'):
            self.open_this_profile = command.split('profile-edit?')[1].replace('%20', ' ')
            self.old_profile = self.profiles.get_active_profile()
            self.cancel_changes = 'edit-profile'
            if self.open_this_profile is not None:
                self.show_menu('profile_editor')
            else:
                print('Refusing to open empty filename profile.')

        elif command.startswith('set-key'):
            # Parse position/colour information
            command = command.replace('%20',' ')
            row = int(command.split('?')[1])
            col = int(command.split('?')[2])
            color = command.split('?')[3]

            red = int(color.strip('rgb()').split(',')[0])
            green = int(color.strip('rgb()').split(',')[1])
            blue = int(color.strip('rgb()').split(',')[2])
            rgb = (red, green, blue)

            # Write to memory
            self.profiles.get_active_profile().set_key_colour(row, col, rgb)

            # Live preview (if 'live_preview' is enabled in preferences)
            if self.preferences.get_pref('live_preview') == 'true':
                self.profiles.activate_profile_from_memory()

        elif command.startswith('clear-key'):
            command = command.replace('%20',' ')
            row = int(command.split('?')[1])
            col = int(command.split('?')[2])

            self.profiles.get_active_profile().reset_key(row, col)

            # Live preview (if 'live_preview' is enabled in preferences)
            if self.preferences.get_pref('live_preview') == 'true':
                self.profiles.activate_profile_from_memory()

        elif command.startswith('profile-activate'):
            command = command.replace('%20',' ')
            profile_name = command.split('profile-activate?')[1]
            self.webkit.execute_script('set_cursor("html","wait")')
            self.profiles.activate_profile_from_file(profile_name)
            self.webkit.execute_script('$("#custom").html("Profile - ' + profile_name + '")')
            self.webkit.execute_script('$("#custom").prop("checked", true)')
            self.webkit.execute_script('set_cursor("html","normal")')

        elif command == 'profile-preview':
            self.profiles.activate_profile_from_memory()

        elif command.startswith('profile-del'):
            # TODO: Instead of JS-based prompt, use PyGtk or within web page interface?
            profile_name = command.split('?')[1].replace('%20', ' ')

            if len(profile_name) > 0:
                self.profiles.remove_profile(profile_name)

                print('Forcing refresh of profiles list...')
                self.refresh_profiles_list()

        elif command.startswith('profile-new'):
            # TODO: Instead of JS-based prompt, use PyGtk or within web page interface?
            profile_name = command.split('?')[1].replace('%20', ' ')

            self.cancel_changes = 'new-profile'
            self.old_profile = self.profiles.get_active_profile()
            self.open_this_profile = profile_name
            self.profiles.new_profile(profile_name)
            self.show_menu('profile_editor')


        elif command == 'profile-save':
            profile_name = self.profiles.get_active_profile_name()
            print('Saving profile "{0}" ...'.format(profile_name))
            self.profiles.save_profile(profile_name)
            print('Saved "{0}".'.format(profile_name))
            self.show_menu('chroma_menu')

            if self.preferences.get_pref('activate_on_save') == 'true':
                self.profiles.activate_profile_from_file(self.profiles.get_active_profile_name())

        ## Miscellaneous
        elif command == 'open-config-folder':
            os.system('xdg-open "' + self.preferences.SAVE_ROOT + '"')

        ## Multi-device Management
        elif command.startswith('set-device?'):
            serial = command.split('?')[1]
            self.set_device(serial)

        elif command == 'rescan-devices':
            self.webkit.execute_script('$("#detected-devices tr").remove();')
            self.detect_devices()

        elif command == 'change-device':
            self.show_menu('controller_devices')

        else:
            print("         ... unimplemented!")


    ##################################################
    # Multi-device support
    ##################################################
    def detect_devices(self):
        # FIXME: Only UI frontend implemented.
        print('fixme:ChromaController.detect_devices')

        # TODO:
        #   -  Program will detect all connected Razer devices and add them to the 'devices' table using JS function.
        #   -  If only one device is avaliable (such as only having one Chroma Keyboard), then automatically open that config page.

        # FIXME: Just a placebo...
        serial = self.daemon.get_serial_number()
        hardware_type = 'blackwidow_chroma'
        self.multi_device_present = True
        print('Found "{0}" (S/N: {1}).'.format(hardware_type, serial))
        self.webkit.execute_script('add_device("' + serial + '", "' + hardware_type + '")')

        # If this is the only Razer device that can be configured, skip the screen.
        if not self.multi_device_present:
            if hardware_type == 'blackwidow_chroma':
                self.show_menu('chroma_menu')

    def set_device(self, serial):
        print('fixme:ChromaController.set_device')
        # TODO:
        #   -  Program knows that this is the 'active' device for configuration.
        #   -  Changes the page based on the type of device.


        serial = self.daemon.get_serial_number()
        hardware_type = 'blackwidow_chroma'
        print('Configuring "{0}" (S/N: {1})".'.format(hardware_type, serial))

        # Open the relevant configuration menu for the selected device.
        if hardware_type == 'blackwidow_chroma' :
            self.current_page = 'chroma_menu'
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_menu.html'))


    ##################################################
    # Application Initialization
    ##################################################
    def __init__(self):
        """
        Initialise the class
        """
        w = Gtk.Window(title="Chroma Driver Configuration")
        w.set_wmclass('razer_bcd_utility', 'razer_bcd_utility')
        w.set_position(Gtk.WindowPosition.CENTER)
        w.set_size_request(1000, 600)
        w.set_resizable(False)
        w.set_icon_from_file(os.path.join(LOCATION_DATA, 'img/app-icon.svg'))
        w.connect("delete-event", Gtk.main_quit)

        if not os.path.exists(LOCATION_DATA):
            print('Data folder is missing. Exiting.')
            sys.exit(1)

        # Initialize Preferences
        self.preferences = razer.preferences.ChromaPreferences()

        # Set up the daemon
        try:
            # Connect to the DBUS
            self.daemon = razer.daemon_dbus.DaemonInterface()

            # Initalize Profiles
            self.profiles = razer.profiles.ChromaProfiles(self.daemon)

            # Load devices page normally.
            #~ self.current_page = 'controller_devices' # TODO: Multi-device not yet supported.
            self.current_page = 'chroma_menu'
            self.multi_device_present = False

            # "Globals"
            self.kb_layout = razer.keyboard.get_keyboard_layout()
            self.reactive_speed = 1
            self.primary_rgb = razer.keyboard.RGB(0, 255, 0)
            self.secondary_rgb = razer.keyboard.RGB(0, 0, 255)
            self.current_effect = 'custom'
            self.last_effect = 'unknown'
            self.open_this_profile = None

        except Exception as e:
            # Load an error page instead.
            print('There was a problem initializing the application or DBUS.')
            self.current_page = 'controller_service_error'
            print('Exception: ', e)


        # Create WebKit Container
        self.webkit = WebKit.WebView()
        settings = WebKit.WebSettings()
        # Needed so that can perform AJAX on file:// URLs
        settings.set_property('enable-file-access-from-file-uris', True)
        self.webkit.set_settings(settings)

        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.add(self.webkit)

        # Build an auto expanding box and add our scrolled window
        b = Gtk.VBox(homogeneous=False, spacing=0)
        b.pack_start(sw, expand=True, fill=True, padding=0)
        w.add(b)

        # Disable right click context menu
        self.webkit.props.settings.props.enable_default_context_menu = False

        # Post-actions after pages fully load.
        self.webkit.connect('load-finished',self.page_loaded)

        # Load the starting page
        self.webkit.open(os.path.join(LOCATION_DATA, self.current_page + '.html'))

        # Process any commands from the web page.
        self.webkit.connect('navigation-policy-decision-requested', self.process_uri)

        # Show the window.
        w.show_all()
        Gtk.main()


class WebkitJavaScriptExecutor(object):
    """
    Simple class to execute scripts
    """
    def __init__(self, webkit, script=None, wrapper=None):
        if wrapper is not None:
            self.wrapper = wrapper
        else:
            self.wrapper = "$(document).ready(function(){{{0}}});"
        self.lines = []
        self.webkit = webkit

        if script is not None:
            self.add(script)

    def add(self, line):
        """
        Adds a line to the collection

        :param line: Line to execute
        :type line: str

        :return: Returns a copy of the object
        :rtype: WebkitJavaScriptExecutor
        """
        line = str(line)

        if line.endswith(';'):
            self.lines.append(line)
        else:
            self.lines.append(line + ';')

        return self

    def exec(self):
        payload = str(self)
        self.webkit.execute_script(payload)

    def __lshift__(self, other):
        self.add(other)

        return self

    def __str__(self):
        lines = '\n' + '\n'.join(self.lines) + '\n'
        result = self.wrapper.format(lines)
        return result



if __name__ == "__main__":
    # Kill the process when CTRL+C'd.
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    ChromaController()
