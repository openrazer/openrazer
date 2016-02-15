#!/usr/bin/env python3

import os, json
import razer.daemon_dbus
import razer.keyboard

class ChromaPreferences(object):
    ''' Retrieves and set persistant options. '''
    #
    # Documented Settings for JSON
    #
    #   live_preview      <true/false>      Activate profiles on keyboard while editing?
    #   live_switch       <true/false>      Profiles are instantly changed on click?
    #   activate_on_save  <true/false>      Automatically activate a profile on save?
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

    def set_pref(self, setting, value):
        print('Set preference: "' + value + '" to "' + setting + '"')
        self.pref_data[setting] = value;

    def get_pref(self, setting, default_value=False):
        try:
            # Read data from preferences, if it exists.
            value = self.pref_data[setting]
            return value
            print('Read preference: "' + value + '" from "' + setting + '"')
        except:
            # Should it be non-existent, return a fallback option.
            print("Preference '" + setting + "' doesn't exist. ")
            self.set_pref(setting, default_value)
            return default_value

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

    def refresh_pref_page(self, webkit):
        # Boolean options
        for setting in ['live_switch','live_preview','activate_on_save']:
            if (self.pref_data[setting] == "true"):
                webkit.execute_script("$('#" + setting + "').prop('checked', true);")
