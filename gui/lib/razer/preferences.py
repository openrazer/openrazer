#!/usr/bin/env python3

import os, json
import razer.daemon_dbus
import razer.keyboard

class ChromaPreferences(object):
    ''' Retrieves and set persistant options. '''
    #
    # Default JSON Settings
    #   See "data/defaults/preferences.json"
    #
    # Settings Descriptions
    #
    # [group]       [setting]           [type]      [description]
    # editor        live_switch         boolean     Send profiles to keyboard as soon as they are selected.
    # editor        activate_on_save    boolean     Send profile to keyboard as soon as it is saved.
    # editor        live_preview        boolean     Update keyboard lights while editing a profile.
    #

    def __init__(self):
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

    def set_pref(self, group, setting, value):
        print('Set preference: "' + value + '" to "' + setting + '"')
        try:
            self.pref_data[group][setting] = value;
        except:
            print('Failed to write setting "' + value + '" for "' + setting + '" in "' + group + '".')

    def get_pref(self, group, setting, default_value='false'):
        try:
            # Read data from preferences, if it exists.
            value = self.pref_data[group][setting]
            return value
            print('Read preference: "' + value + '" from "' + setting + '"')
        except:
            # Should it be non-existent, return a fallback option.
            print("Preference '" + setting + "' in '" + group + "' doesn't exist.")
            self.set_pref(group, setting, default_value)
            return default_value

    def create_default_config(self):
        try:
            # Create the groups, then write the default values.
            default_buffer = {}
            default_buffer['chroma_editor'] = {}
            default_buffer['chroma_editor']['live_switch'] = True
            default_buffer['chroma_editor']['activate_on_save'] = True
            default_buffer['chroma_editor']['live_preview'] = True
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

    def refresh_pref_page(self, webkit):
        # Boolean options
        for setting in ['live_switch','live_preview','activate_on_save']:
            if (self.pref_data['chroma_editor'][setting] == 'true'):
                webkit.execute_script("$('#" + setting + "').prop('checked', true);")
