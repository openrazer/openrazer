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

# Default Settings & Preferences
SAVE_ROOT = os.path.expanduser('~') + '/.config/razer_chroma'
SAVE_PROFILES = SAVE_ROOT + '/profiles'
SAVE_BACKUPS = SAVE_ROOT + '/backups'

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

        if page == 'main_menu':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_menu.html'))

        elif page == 'profile_editor':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_profiles.html'))

        elif page == 'preferences':
            self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_preferences.html'))

        else:
            print("Unknown menu '" + page + "'!")

    ##################################################
    # Page Initialization
    ##################################################
    def page_loaded(self, WebView, WebFrame):
        print('Running page post-actions for "' + self.current_page + '"...')
        if self.current_page == 'main_menu':
            self.webkit.execute_script('instantProfileSwitch = false;') # Unimplemented instant profile change option.
            self.webkit.execute_script("$('#profiles-activate').show()")
            self.refresh_profiles_list()

        elif self.current_page == 'profile_editor':
            self.webkit.execute_script('change_header("Edit ' + self.open_this_profile + '")')

            # Initialize keyboard object.
            self.webkit.execute_script("keyboard_obj.load();")
            self.webkit.execute_script("keyboard_obj.set_layout(\"kb-" + self.kb_layout + "\")")

            # Load profile into keyboard.
            profile_name = self.open_this_profile
            self.profiles.set_active_profile(profile_name)
            for pos_y, row in enumerate(self.profiles.get_profile(profile_name).get_rows_raw()):
                for pos_x, rgb in enumerate(row):
                    js_string = "keyboard_obj.set_key_colour({0},{1},\"#{2:02X}{3:02X}{4:02X}\")".format(pos_y, pos_x, rgb.red, rgb.green, rgb.blue)
                    self.webkit.execute_script(js_string)

            # IF BLACKWIDOW ULTIMATE < 2016
            # OR BLACKWIDOW CHROMA
            # disable space key and FN
            self.webkit.execute_script("keyboard_obj.disable_key(5,7)")
            self.webkit.execute_script("keyboard_obj.disable_key(5,12)")

        elif self.current_page == 'preferences':
            self.preferences.refresh_pref_page(self.webkit);

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

                self.webkit.execute_script("$(\"#cancel\").attr({onclick: \"cmd('cancel-changes')\"})")
            self.show_menu('main_menu')

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
            self.show_menu('main_menu')

        elif command == 'pref-save':
            self.preferences.save_pref()
            self.show_menu('main_menu')

        ## Profile Editor / Management
        elif command.startswith('profile-edit'):
            self.open_this_profile = command.split('profile-edit?')[1].replace('%20', ' ')
            if self.open_this_profile != None:
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

        elif command.startswith('clear-key'):
            command = command.replace('%20',' ')
            row = int(command.split('?')[1])
            col = int(command.split('?')[2])

            self.profiles.get_active_profile().reset_key(row, col)

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
            self.profiles.new_profile(profile_name)
            # Clear editor
            self.webkit.execute_script("keyboard_obj.set_layout(\"kb-" + self.kb_layout + "\")")
            self.webkit.execute_script("keyboard_obj.clear_all_keys()")
            self.webkit.execute_script("keyboard_obj.disable_key(5,7)")
            self.webkit.execute_script("keyboard_obj.disable_key(5,12)")

            self.webkit.execute_script("$(\"#cancel\").attr({onclick: \"cmd('cancel-changes?new-profile?" + profile_name + "')\"})")


            self.show_menu('profile_editor')

        elif command == 'profile-save':
            profile_name = self.profiles.get_active_profile_name()
            print('Saving profile "{0}" ...'.format(profile_name))

            self.profiles.save_profile(profile_name)

            print('Saved "{0}".'.format(profile_name))

            self.show_menu('main_menu')

        ## Miscellaneous
        elif command == 'open-config-folder':
            os.system('xdg-open "' + SAVE_ROOT + '"')

        else:
            print("         ... unimplemented!")

    ##################################################
    # Application Initialization
    ##################################################
    def __init__(self):
        """
        Initialise the class
        """
        w = Gtk.Window(title="Razer BlackWidow Chroma Configuration")
        w.set_wmclass('razer_bcd_utility', 'razer_bcd_utility')
        w.set_position(Gtk.WindowPosition.CENTER)
        w.set_size_request(1000, 600)
        w.set_resizable(False)
        w.set_icon_from_file(os.path.join(LOCATION_DATA, 'img/app-icon.svg'))
        w.connect("delete-event", Gtk.main_quit)

        if not os.path.exists(LOCATION_DATA):
            print('Data folder is missing. Exiting.')
            sys.exit(1)

        ## Check we have a folder to save data (eg. profiles)
        if not os.path.exists(SAVE_ROOT):
            print('Configuration folder does not exist. Creating',SAVE_ROOT)
            os.makedirs(SAVE_ROOT)
            os.makedirs(SAVE_PROFILES)
            os.makedirs(SAVE_BACKUPS)

        # Set up the daemon
        self.daemon = razer.daemon_dbus.DaemonInterface()

        # Profiles
        self.profiles = ChromaProfiles(self.daemon)

        # Preferences
        self.preferences = ChromaPreferences()

        # "Globals"
        self.kb_layout = razer.keyboard.get_keyboard_layout()
        self.reactive_speed = 1
        self.primary_rgb = razer.keyboard.RGB(0, 255, 0)
        self.secondary_rgb = razer.keyboard.RGB(0, 0, 255)
        self.current_effect = 'custom'
        self.last_effect = 'unknown'

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

        # Load page
        self.current_page = 'main_menu'
        self.webkit.open(os.path.join(LOCATION_DATA, 'chroma_menu.html'))

        # Post-actions after pages fully load.
        self.webkit.connect('load-finished',self.page_loaded)

        # Process any commands from the web page.
        self.webkit.connect('navigation-policy-decision-requested', self.process_uri)

        # Show the window.
        w.show_all()
        Gtk.main()

class ChromaProfiles(object):
    def __init__(self, dbus_object):
        self.profiles = {}
        self.active_profile = None
        self.daemon = dbus_object

        self.load_profiles()

    def load_profiles(self):
        """
        Load profiles
        """
        profiles = os.listdir(SAVE_PROFILES)

        for profile in profiles:
            keyboard = ChromaProfiles.get_profile_from_file(profile)
            self.profiles[profile] = keyboard

    def remove_profile(self, profile_name, del_from_fs=True):
        """
        Delete profile, from memory and optionally the system.

        :param profile_name: Profile name
        :type profile_name: str

        :param del_from_fs: Delete from the file system
        :type del_from_fs: bool
        """
        if del_from_fs:
            current_profile_path = os.path.join(SAVE_PROFILES, profile_name)
            current_profile_path_backup = os.path.join(SAVE_BACKUPS, profile_name)
            os.remove(current_profile_path)
            # print('Deleted profile: {0}'.format(current_profile_path))
            if os.path.exists(current_profile_path_backup):
                os.remove(current_profile_path_backup)
                # print('Deleted backup copy: ' + current_profile_path_backup)

        if profile_name in self.profiles:
            del self.profiles[profile_name]

    def new_profile(self, profile_name):
        """
        Create new profile

        :param profile_name: Profile name
        :type profile_name: str
        """
        self.active_profile = profile_name
        self.profiles[profile_name] = razer.keyboard.KeyboardColour()

    def set_active_profile(self, profile_name):
        """
        Set the active profile name

        :param profile_name: Profile name
        :type profile_name: str
        """
        if profile_name in self.profiles:
            self.active_profile = profile_name

    def get_active_profile(self):
        """
        Gets active profile

        :return: Keyboard object
        :rtype: razer.keyboard.KeyboardColour
        """
        return self.profiles[self.active_profile]

    def get_active_profile_name(self):
        """
        Gets active profile

        :return: Profile name
        :rtype: str
        """
        return self.active_profile

    def get_profiles(self):
        """
        Get a list of profiles

        :return: List of profiles
        :rtype: list
        """
        return self.profiles.keys()

    def get_profile(self, profile_name):
        """
        Get a profile

        :param profile_name: Profile
        :type profile_name: str

        :return: Keyboard object
        :rtype: razer.keyboard.KeyboardColour
        """
        return self.profiles[profile_name]

    def save_profile(self, profile_name):

        profile_path = os.path.join(SAVE_PROFILES, profile_name)

        # Backup if it's an existing copy, then erase original copy.
        if os.path.exists(profile_path):
            os.rename(profile_path, os.path.join(SAVE_BACKUPS, profile_name))

        with open(os.path.join(SAVE_PROFILES, profile_name), 'wb') as profile_file:
            payload = self.profiles[profile_name].get_total_binary()
            profile_file.write(payload)

    def activate_profile_from_file(self, profile_name):
        print("Applying profile '{0}' ... ".format(profile_name), end='')
        with open(os.path.join(SAVE_PROFILES, profile_name), 'rb') as profile_file:
            payload = profile_file.read()
            keyboard = razer.keyboard.KeyboardColour()
            keyboard.get_from_total_binary(payload)
            self.daemon.set_custom_colour(keyboard)

    def activate_profile_from_memory(self):
        profile_name = self.get_active_profile_name()
        print("Applying profile '{0}' from memory...".format(profile_name))
        print('fixme:activate_profile_from_memory')

    @staticmethod
    def get_profile_from_file(profile_name):
        keyboard = razer.keyboard.KeyboardColour()

        with open(os.path.join(SAVE_PROFILES, profile_name), 'rb') as profile_file:
            payload = profile_file.read()
            keyboard.get_from_total_binary(payload)

        return keyboard

class ChromaPreferences(object):
    #
    # Documented Settings for JSON
    #
    #   live_preview      <true/false>      Activate profiles on keyboard while editing?
    #   live_switch       <true/false>      Profiles are instantly changed on click?
    #   activate_on_save  <true/false>      Automatically activate a profile on save?
    #

    def __init__(self):
        self.pref_path = os.path.join(SAVE_ROOT, 'preferences.json')
        self.load_pref();

    def load_pref(self):
        print('Loading preferences from "' + self.pref_path + "'...")
        # Does it exist?
        if not os.path.exists(self.pref_path):
            self.create_default_config()

        # Load data into memory.
        try:
            with open(self.pref_path) as pref_file:
                self.pref_data = json.load(pref_file)
            print('Successfully loaded preferences.')
        except:
            self.create_default_config();

    def save_pref(self):
        print('Saving preferences to "' + self.pref_path + "'...")
        pref_file = open(self.pref_path, "w+")
        pref_file.write(json.dumps(self.pref_data))
        pref_file.close()

    def set_pref(self, setting, value):
        print('Set preference: "' + value + '" to "' + setting + '"')
        self.pref_data[setting] = value;

    def get_pref(self, setting, default_value=False):
        print('Read preference: "' + value + '" from "' + setting + '"')
        try:
            # Read data from preferences, if it exists.
            get_data = self.pref_data[setting]
            return get_data
        except:
            # Should it be non-existent, return a fallback option.
            print("Preference '" + setting + "' doesn't exist. ")
            self.set_pref(setting, default_value)
            return default_value

    def refresh_pref_page(self, webkit):
        # Boolean options
        for setting in ['live_switch','live_preview','activate_on_save']:
            if (self.pref_data[setting] == "true"):
                webkit.execute_script("$('#" + setting + "').prop('checked', true);")

    def create_default_config(self):
        default_settings = '{\n "live_switch" : "false",\n "live_preview" : "false",\n "activate_on_save" : "false" \n}'

        print('Creating new preferences file...')
        if os.path.exists(self.pref_path):
            print('Failed to parse JSON preferences!')
            os.rename(self.pref_path, self.pref_path+'.bak')
            print('Successfully backed up problematic preferences JSON file.')

        pref_file = open(self.pref_path, "w")
        pref_file.write(default_settings)
        pref_file.close()
        print('Successfully written default preferences.')


if __name__ == "__main__":
    # Kill the process when CTRL+C'd.
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    ChromaController()
