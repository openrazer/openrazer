#!/usr/bin/env python3

import os
import razer.daemon_dbus
import razer.preferences
import razer.keyboard

class ChromaProfiles(object):
    ''' Create, edit, delete and submit profiles to the keyboard. '''

    def __init__(self, dbus_object):
        self.preferences = razer.preferences.ChromaPreferences()
        self.profiles = {}
        self.active_profile = None
        self.daemon = dbus_object

        self.load_profiles()

    def load_profiles(self):
        """
        Load profiles
        """
        profiles = os.listdir(self.preferences.SAVE_PROFILES)

        for profile in profiles:
            keyboard = self.get_profile_from_file(profile)
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
            current_profile_path = os.path.join(self.preferences.SAVE_PROFILES, profile_name)
            current_profile_path_backup = os.path.join(self.preferences.SAVE_BACKUPS, profile_name)
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
        Gets active profile, if one isnt active then the first profile is returned. If no
        profiles are loaded then an empty profile is returned

        :return: Keyboard object
        :rtype: razer.keyboard.KeyboardColour
        """

        profile = razer.keyboard.KeyboardColour()
        try:
            profile = self.profiles[self.active_profile]
        except KeyError:
            if len(list(self.profiles.keys())) > 0:
                profile = self.profiles[list(self.profiles.keys())[0]]
        return profile

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

        profile_path = os.path.join(self.preferences.SAVE_PROFILES, profile_name)

        # Backup if it's an existing copy, then erase original copy.
        if os.path.exists(profile_path):
            os.rename(profile_path, os.path.join(self.preferences.SAVE_BACKUPS, profile_name))

        with open(os.path.join(self.preferences.SAVE_PROFILES, profile_name), 'wb') as profile_file:
            payload = self.profiles[profile_name].get_total_binary()
            profile_file.write(payload)

    def activate_profile_from_file(self, profile_name):
        print("Applying profile '{0}' ... ".format(profile_name), end='')
        with open(os.path.join(self.preferences.SAVE_PROFILES, profile_name), 'rb') as profile_file:
            payload = profile_file.read()
            keyboard = razer.keyboard.KeyboardColour()
            keyboard.get_from_total_binary(payload)
            self.daemon.set_custom_colour(keyboard)

    def activate_profile_from_memory(self):
        profile_name = self.get_active_profile_name()
        keyboard = self.get_active_profile()
        self.daemon.set_custom_colour(keyboard)
        print("Applying profile '{0}' from memory...".format(profile_name))

    def get_profile_from_file(self, profile_name):
        keyboard = razer.keyboard.KeyboardColour()
        with open(os.path.join(self.preferences.SAVE_PROFILES, profile_name), 'rb') as profile_file:
            payload = profile_file.read()
            keyboard.get_from_total_binary(payload)

        return keyboard
