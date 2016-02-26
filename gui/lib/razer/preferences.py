#!/usr/bin/env python3

import os, json, shutil
import razer.daemon_dbus
import razer.keyboard

module_config_version = 2

class ChromaPreferences(object):
    ''' Retrieves and set persistant options. '''
    #
    # Default JSON Settings
    #   See "data/defaults/preferences.json"
    #
    # Settings Descriptions
    #
    # [group]           [setting]           [type]      [description]
    # editor            live_switch         boolean     Send profiles to keyboard as soon as they are selected.
    # editor            activate_on_save    boolean     Send profile to keyboard as soon as it is saved.
    # editor            live_preview        boolean     Update keyboard lights while editing a profile.

    # tray_applet       icon_type           string      "system", "logo" or "custom".
    # tray_applet       icon_path           string      ID or pathname for a custom applet icon.

    # startup           enabled             boolean     Automatically set preferences when tray applet starts?
    # startup           start_effect        string      Automatically set effect to X. ( 'spectrum', 'reactive', etc] or 'disabled' for no change.
    # startup           start_profile       string      Automatically set to profile X. ('start_effect' must be set to 'profile')
    # startup           start_brightness    integer     Automatically set brightness to X. Set to '0' for no change.
    # startup           start_macro         boolean     Automatically activate macro keys.
    # primary_colors    red                 integer     Default primary RED colour.
    # primary_colors    green               integer     Default primary GREEN colour.
    # primary_colors    blue                integer     Default primary BLUE colour.
    # secondary_colors  red                 integer     Default secondary RED colour.
    # secondary_colors  green               integer     Default secondary GREEN colour.
    # secondary_colors  blue                integer     Default secondary BLUE colour.
    #

    def __init__(self):
        """ Initializes the preferences module. """
        # Determine locations for storing data.
        self.SAVE_ROOT = os.path.expanduser('~') + '/.config/razer_chroma'
        self.SAVE_PROFILES = self.SAVE_ROOT + '/profiles'
        self.SAVE_BACKUPS = self.SAVE_ROOT + '/backups'

        # Check we have a folder to save data (eg. profiles)
        if not os.path.exists(self.SAVE_ROOT):
            print('Configuration folder does not exist. Creating', self.SAVE_ROOT)
            os.makedirs(self.SAVE_ROOT)
            os.makedirs(self.SAVE_PROFILES)
            os.makedirs(self.SAVE_BACKUPS)

        # Load preferences shared between GUI applications.
        self.pref_path = os.path.join(self.SAVE_ROOT, 'preferences.json')
        self.load_pref();

    def load_pref(self):
        """ Loads the preferences file from disk. """
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

        # Check the configuration version and warn if opening an older configuration.
        try:
            config_version = int(self.pref_data['config_version'])
        except:
            config_version = 0

        if config_version > module_config_version:
            print('\nWARNING: The preferences file is newer then the module in use. This could cause undesired glitches in applications. Consider updating the Python modules.')
            print('    Config Version: ' + str(config_version))
            print('    Module Version: ' + str(module_config_version))
            print(' ')

        if module_config_version > config_version:
            config_version = module_config_version
            print('\nWARNING: This is an older configuration. Some preferences may have been deprecated or changed. Saving will expect future instances to use the new format.\n')


    def save_pref(self):
        """ Commit the preferences stored in memory to disk. """
        print('Saving preferences to "' + self.pref_path + "'...")
        self.pref_data['config_version'] = module_config_version
        pref_file = open(self.pref_path, "w+")
        pref_file.write(json.dumps(self.pref_data))
        pref_file.close()

    def set_pref(self, group, setting, value):
        """ Write an option. """
        # Strings with spaces may have HTML codes, eg. %20 for a space.
        if type(value) is str:
            value = value.replace('%20', ' ')

        print('Set preference: "' + str(value) + '" to "' + setting + '" in "' + group + '"')
        # Check the group exists.
        try:
            self.pref_data[group]
        except:
            print('Failed to write group: ' + group)
            self.pref_data[group] = {}

        # Write value to group/setting.
        try:
            self.pref_data[group][setting] = value;
        except:
            print('Failed to write setting "' + str(value) + '" for "' + setting + '" in "' + group + '".')

    def get_pref(self, group, setting, default_value='false'):
        """ Read an option, optionally specifying a default value if none exists. """
        try:
            # Read data from preferences, if it exists.
            value = self.pref_data[group][setting]
            return value
            print('Read preference: "' + value + '" from "' + setting + '"')
        except:
            # Should it be non-existent, return a fallback option.
            print("Preference '" + setting + "' in '" + group + "' doesn't exist.")
            self.set_pref(group, setting, default_value)
            self.save_pref()
            return default_value

    def create_default_config(self):
        """ Generates a default configuration if none exists. """
        try:
            # Create the groups, then write the default values.
            default_buffer = {}
            default_buffer['config_version'] = module_config_version
            for group in ['chroma_editor', 'tray_applet', 'startup', 'primary_colors', 'secondary_colors']:
                default_buffer[group] = {}
            default_buffer['chroma_editor']['live_switch'] = 'true'
            default_buffer['chroma_editor']['activate_on_save'] = 'true'
            default_buffer['chroma_editor']['live_preview'] = 'true'
            default_buffer['tray_applet']['icon_type'] = 'system'
            default_buffer['tray_applet']['icon_path'] = ''
            default_buffer['startup']['enabled'] = 'false'
            default_buffer['startup']['start_effect'] = 'disabled'
            default_buffer['startup']['start_profile'] = ''
            default_buffer['startup']['start_brightness'] = 0
            default_buffer['startup']['start_macro'] = 'false'
            default_buffer['primary_colors']['red'] = 0
            default_buffer['primary_colors']['green'] = 255
            default_buffer['primary_colors']['blue'] = 0
            default_buffer['secondary_colors']['red'] = 255
            default_buffer['secondary_colors']['green'] = 0
            default_buffer['secondary_colors']['blue'] = 0
            default_settings = json.dumps(default_buffer)

            print('Creating new preferences file...')
            if os.path.exists(self.pref_path):
                print('Existing preferences file is corrupt or being forced overwritten.')
                os.rename(self.pref_path, self.pref_path+'.bak')
                print('Successfully backed up previous preferences JSON file.')

            pref_file = open(self.pref_path, "w")
            pref_file.write(default_settings)
            pref_file.close()
            print('Successfully written default preferences.')
        except Exception as e:
            # Couldn't create the default configuration.
            print('Failed to write default preferences.')
            print('Exception: ', e)

    def clear_config(self):
        """ Erases the configuration stored on disk. """
        print('Deleting configuration folder "' + self.SAVE_ROOT + '"...')
        shutil.rmtree(self.SAVE_ROOT)
        print('Successfully deleted configuration.')

