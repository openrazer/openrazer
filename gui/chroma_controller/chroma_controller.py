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
#

import os, sys, signal, inspect
from gi.repository import Gtk, Gdk, WebKit
import razer.daemon_dbus

# Default Settings & Preferences
### Primary colour used for effects
rgb_primary_red = 0
rgb_primary_green = 255
rgb_primary_blue = 0
### Colour used for breath mode.
rgb_secondary_red = 0
rgb_secondary_green = 0
rgb_secondary_blue = 255
current_effect = 'custom'
last_effect = 'unknown'
layout = 'en-gb'

class Paths(object):
    # Initializes paths and directories

    ## Where are we?
    location = os.path.dirname( os.path.abspath(inspect.getfile(inspect.currentframe())) )
    location_data = os.path.join(location, 'data/')
    location_installed = '/usr/share/razer_chroma_controller'

    ## Where user's configuration is saved
    save_root = os.path.expanduser('~') + '/.config/razer_chroma'
    save_profiles = save_root + '/profiles'
    save_backups = save_root + '/backups'

    ## Check 'data' folder exists.
    if not os.path.exists(location_data):
        print('Data folder is missing. Exiting.')
        exit()

    ## Check we have a folder to save data (eg. profiles)
    if not os.path.exists(save_root):
        print('Configuration folder does not exist. Creating',save_root,)
        os.makedirs(save_root)
        os.makedirs(save_profiles)
        os.makedirs(save_backups)

    # Does the 'dynamic' binary exist? (required for key profiles)
    ### In same folder as script.
    if os.path.exists(location + '/dynamic'):
        dynamicPath = location + '/dynamic'
    ### Compiled in Git project 'examples' folder
    elif os.path.exists(location + '/../../examples/dynamic'):
        dynamicPath = location + '/../../examples/dynamic'
    else:
        dynamicPath = 'notfound'

    ## Is a sudoers file present for the utility?
    ## (This bypasses the password authentication prompt on Ubuntu/Debian distros and
    ##  assumes the application is installed to the system)
    if os.path.exists('/etc/sudoers.d/razer_chroma_dynamic'):
        dynamicPath = '/usr/share/razer_chroma_controller/dynamic'
        dynamicExecType = 'sudoers'
    else:
        dynamicExecType = 'pkexec'

class ChromaController(object):
    ##################################################
    # Page Switcher
    ##################################################
    def show_menu(self, page):
        print("Opening menu '"+page+"'")

        # Hide all footer buttons
        webkit.execute_script('$("#retry").hide()')
        webkit.execute_script('$("#edit-save").hide()')
        webkit.execute_script('$("#edit-preview").hide()')
        webkit.execute_script('$("#cancel").hide()')
        webkit.execute_script('$("#close-window").hide()')
        webkit.execute_script('$("#pref-open").hide()')
        webkit.execute_script('$("#pref-save").hide()')

        if page == 'main_menu':
            webkit.execute_script('changeTitle("Configuration Menu")')
            webkit.execute_script('smoothFade(".menu_area",'+page+')')
            webkit.execute_script('$("#close-window").show()')
            webkit.execute_script('$("#pref-open").show()')
            self.refresh_profiles_list()

        elif page == 'not_detected':
            webkit.execute_script('changeTitle("Keyboard Not Detected")')

        elif page == 'profile_editor':
            global profile_name # TODO remove global
            webkit.execute_script('changeTitle("Edit ' + profile_name + '")')
            webkit.execute_script('smoothFade("#main_menu","#profile_editor")')
            webkit.execute_script('$("#cancel").show()')
            webkit.execute_script('$("#edit-preview").show()')
            webkit.execute_script('$("#edit-save").show()')

        elif page == 'preferences':
            webkit.execute_script('changeTitle("Preferences")')
            webkit.execute_script('$("#cancel").show()')
            webkit.execute_script('$("#pref-save").show()')

        else:
            print("Unknown menu '"+page+"'!")

    ##################################################
    # Page Initialization
    ##################################################
    def page_loaded(self, WebView, WebFrame):
        # Check if the Chroma is plugged in as soon as the page finishes loading.
        print("Detecting Chroma Keyboard... ", end='')
        # FIXME: NOT YET IMPLEMENTED
        # print('found at ####')
        print("\nfixme: page_loaded detect chroma keyboard")
        #  -- If it is, show the main menu.
        # --  If not, kindly ask the user to do so.
        self.show_menu('main_menu')

        # Load profiles and preferences
        # FIXME: Not yet implemented!
        #~ self.preferences('load')

        webkit.execute_script('instantProfileSwitch = false;') # Unimplemented instant profile change option.
        webkit.execute_script("$('#profiles-activate').show()")

        # Load list of profiles
        self.refresh_profiles_list()

        # Apply preferences
        ## FIXME: Unimplemented: Default starting colour.
        #~ webkit.execute_script('$("#rgb_primary_preview").css("background-color","rgba('+str(rgb_primary_red)+','+str(rgb_primary_green)+','+str(rgb_primary_blue)+',1.0)")')

        ## Write keyboard
        global layout # TODO remove global
        keyboard_layout_ffile = open(Paths.location_data+'/layouts/'+layout+'.html','r')
        for line in keyboard_layout_ffile:
          if '\n' == line[-1]:
            webkit.execute_script("$('#keyboard').append('"+line.split('\n')[0]+"')")
        print("Loaded keyboard layout '"+layout+"'")

        # Does 'dynamic' exist? Without this, profiles can't be activated!
        # See '/../examples/dynamic-examples/readme' for more details.
        # If this is integrated to dbus, this external binary won't be needed (nor require root privileges to execute)
        if Paths.dynamicPath == 'notfound':
            webkit.execute_script('$("#profiles-activate").')
            print("WARNING: Binary for 'dynamic' not found!")
            print("         You won't be able to activate any key profiles created with the utility without compiling the program and placing beside this script.")
            print("         See '/../examples/dynamic-examples/readme' for details about 'dynamic'.")

    def refresh_profiles_list(self):
        global webkit # TODO remove global
        webkit.execute_script('$("#profiles_list").html("")')
        for profile in ChromaProfiles.getFileList():
            item = "<option value='"+profile+"'>"+profile+"</option>"
            webkit.execute_script('$("#profiles_list").append("'+item+'")')

    ##################################################
    # Commands
    ##################################################
    def process_uri(self, view, frame, net_req, nav_act, pol_dec):
        uri = net_req.get_uri()
        frame.stop_loading()

        if uri.startswith('cmd://'):
            command = uri[6:]
            print("\nCommand: '"+command+"'")
            self.process_command(command)

        if uri.startswith('web://'):
            web_url = uri[6:]
            print('fixme:open web browser to URL')

    def process_command(self, command):
        global rgb_primary_red, rgb_primary_green, rgb_primary_blue, current_effect # TODO remove global
        global rgb_secondary_red, rgb_secondary_green, rgb_secondary_blue # TODO remove global
        global profile_name, profileMemory # TODO remove global
        global current_effect, last_effect # TODO remove global
        global webkit # TODO remove global

        if command == 'quit':
            quit()

        ## Effects & Keyboard Controls
        elif command.startswith('brightness'):
            value = int(command[11:])
            daemon.set_brightness(value)

        elif command.startswith('effect'):
            enabled_options = []

            if command == 'effect-none':
                current_effect = "none"
                daemon.set_effect('none')

            elif command == 'effect-spectrum':
                current_effect = "spectrum"
                daemon.set_effect('spectrum')

            elif command.startswith('effect-wave'):
                current_effect = "wave"
                daemon.set_effect('wave', int(command[12:])) # ?1 or ?2 for direction
                enabled_options = ['waves']

            elif command.startswith('effect-reactive'):
                current_effect = "reactive"
                global reactive_speed # TODO remove global
                if command.split('?')[1] == 'auto':
                    # Use the previous effect
                    daemon.set_effect('reactive', reactive_speed, rgb_primary_red, rgb_primary_green, rgb_primary_blue)
                else:
                    reactive_speed = int(command[16])
                    daemon.set_effect('reactive', reactive_speed, rgb_primary_red, rgb_primary_green, rgb_primary_blue)
                enabled_options = ['rgb_primary', 'reactive']

            elif command.startswith('effect-breath'):
                current_effect = "breath"
                global breath_random # TODO remove global
                breath_random = command[14]
                if breath_random == '1':  # Random mode
                    daemon.set_effect('breath', 1)
                    enabled_options = ['breath-select']
                else:
                    daemon.set_effect('breath', rgb_primary_red, rgb_primary_green, rgb_primary_blue, rgb_secondary_red, rgb_secondary_green, rgb_secondary_blue)
                    enabled_options = ['breath-random', 'rgb_primary', 'rgb_secondary']

            elif command == 'effect-static':
                current_effect = "static"
                daemon.set_effect('static', rgb_primary_red, rgb_primary_green, rgb_primary_blue)
                enabled_options = ['rgb_primary']

            # Fade between options for that effect, should it have been changed.
            if not current_effect == last_effect:
                # Effect changed, fade out all previous options.
                for element in ['rgb_primary', 'rgb_secondary', 'waves', 'reactive', 'breath-random', 'breath-select']:
                    webkit.execute_script('$("#' + element + '").fadeOut("fast")')

                # Fade in desired options for this effect.
                for element in enabled_options:
                    webkit.execute_script("setTimeout(function(){ $('#" + element + "').fadeIn('fast');}, 200)")
            last_effect = current_effect

        elif command == 'enable-marco-keys':
            daemon.marco_keys(True)
            webkit.execute_script('$("#marco-keys-enable").addClass("btn-disabled")')
            webkit.execute_script('$("#marco-keys-enable").html("Marco Keys In Use")')

        elif command == 'gamemode-enable':
            daemon.game_mode(True)
            webkit.execute_script('$("#game-mode-status").html("Enabled")')
            webkit.execute_script('$("#game-mode-enable").hide()')
            webkit.execute_script('$("#game-mode-disable").show()')

        elif command == 'gamemode-disable':
            daemon.game_mode(False)
            webkit.execute_script('$("#game-mode-status").html("Disabled")')
            webkit.execute_script('$("#game-mode-enable").show()')
            webkit.execute_script('$("#game-mode-disable").hide()')

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
            webkit.execute_script('$("#'+element+'_preview").css("background-color","rgba(' + str(red) + ',' + str(green) + ',' + str(blue) + ',1.0)")')
            webkit.execute_script('set_mode("set")')

            # Update global variables if applicable
            if element == 'rgb_primary':    # Primary effect colour
                rgb_primary_red = red
                rgb_primary_green = green
                rgb_primary_blue = blue
            elif element == 'rgb_secondary':   # Secondary effect colour (used for Breath mode)
                rgb_secondary_red = red
                rgb_secondary_green = green
                rgb_secondary_blue = blue
            elif element == 'rgb_tmp':      # Temporary colour while editing profiles.
                rgb_edit_red = red
                rgb_edit_green = green
                rgb_edit_blue = blue

            # Update static colour effects if currently in use.
            if current_effect == 'static':
                self.process_command('effect-static')
            elif current_effect == 'breath':
                self.process_command('effect-breath?0')
            elif current_effect == 'reactive':
                self.process_command('effect-reactive?auto')

        ## Opening different pages
        elif command == 'cancel-changes':
            self.show_menu('main_menu')

        elif command == 'pref-open':
            self.preferences('load')
            self.show_menu('preferences')

        elif command == 'pref-save':
            self.preferences('save')
            self.show_menu('main_menu')

        elif command.startswith('profile-edit'):
            profile_name = command.split('profile-edit?')[1].replace('%20', ' ')
            profile_path = Paths.save_profiles+'/' + profile_name

            # Dynamic Profile File Format:
            #   <cmd> <parm>
            #   1 <x> <y> <red> <green> <blue>
            # File MUST end with '0'.

            # Profile Memory within Python
            # [ mode, x. y. red. green. blue ]
            # Mode = 1 = static key at X,Y

            # Clear any existing colours / array memory
            cleared_text = 'rgb(128,128,128)'
            cleared_border = 'rgb(70,70,70)'
            profileMemory = []

            for posX in range(0,21):
              for posY in range(0,5):
                webkit.execute_script('$("#x'+str(posX)+'-y'+str(posY)+'").css("border","2px solid '+cleared_border+'")')
                webkit.execute_script('$("#x'+str(posX)+'-y'+str(posY)+'").css("color","'+cleared_text+'")')

            try:
              with open(profile_path,'r') as f:
                  profile_contents = f.read().splitlines()

              # Load new values
              for line in profile_contents:
                if line != '0':
                  posX = line.split(' ')[1]
                  posY = line.split(' ')[2]
                  red = line.split(' ')[3]
                  green = line.split(' ')[4]
                  blue = line.split(' ')[5]
                  rgbCSS = 'rgb('+red+','+green+','+blue+')'
                  webkit.execute_script('$("#x'+posX+'-y'+posY+'").css("border","2px solid '+rgbCSS+'")')
                  webkit.execute_script('$("#x'+posX+'-y'+posY+'").css("color","'+rgbCSS+'")')
                  profileMemory.append([1, int(posX), int(posY), int(red), int(green), int(blue)])

              print('Opened profile "' + profile_name + '"')
              self.show_menu('profile_editor')

            except:
              print('Problem opening "'+profile_path+'" for reading.')

        elif command.startswith('set-key'):
            # Replace any existing occurances first
            self.process_command('clear-key?'+command.split('set-key?')[1])

            # Parse position/colour information
            command = command.replace('%20',' ')
            posX = command.split('?')[1].strip('x').split('-')[0]
            posY = command.split('?')[1].split('-y')[1]
            color = command.split('?')[2]
            red = int(color.strip('rgb()').split(',')[0])
            green =int(color.strip('rgb()').split(',')[1])
            blue = int(color.strip('rgb()').split(',')[2])

            # Write to memory
            profileMemory.append([1, posX, posY, red, green, blue])

        elif command.startswith('clear-key'):
            command = command.replace('%20',' ')
            posX = command.split('?')[1].strip('x').split('-')[0]
            posY = command.split('?')[1].split('-y')[1]

            # Scan the profile in memory and erase any reference to it.
            row = 0
            for line in profileMemory:
              if str(line).startswith('[1, ' + posX + ", " + posY + ","):
                  del profileMemory[row]
              row = row + 1

        elif command.startswith('profile-activate'):
            command = command.replace('%20',' ')
            profile_name = command.split('profile-activate?')[1]
            webkit.execute_script('setCursor("wait")')
            ChromaProfiles.setProfile('file', profile_name)
            webkit.execute_script('$("#custom").html("Profile - ' + profile_name + '")')
            webkit.execute_script('$("#custom").prop("checked", true)')
            webkit.execute_script('setCursor("normal")')
            process_command(self, 'effect-profile?'+profile_name)

        elif command.startswith('profile-del'):
            # TODO: Instead of JS-based prompt, use PyGtk or within web page interface?
            profile_name = command.split('?')[1].replace('%20', ' ')
            os.remove(Paths.save_profiles +'/' + profile_name)
            print('Deleted profile: ' + Paths.save_profiles +'/' + profile_name)
            if os.path.exists(Paths.save_backups + '/' + profile_name):
                os.remove(Paths.save_backups +'/' + profile_name)
                print('Deleted backup copy: ' + Paths.save_backups +'/' + profile_name)
            print('Forcing refresh of profiles list...')
            self.refresh_profiles_list()

        elif command.startswith('profile-new'):
            # TODO: Instead of JS-based prompt, use PyGtk or within web page interface?
            profile_name = command.split('?')[1].replace('%20', ' ')
            profileMemory = []
            self.show_menu('profile_editor')

        elif command == 'profile-save':
            print('Saving profile "' + profile_name + '...')
            profile_path = Paths.save_profiles + '/' + profile_name

            # Backup if it's an existing copy, then erase original copy.
            if os.path.exists(profile_path):
                os.rename(profile_path, Paths.save_backups +'/' + profile_name)

            # Prepare to write to file
            profileSave = open(profile_path, "w")

            for line in profileMemory:
                lineBuffer=''
                for data in line:
                    lineBuffer = lineBuffer + str(data) + ' '

                profileSave.write(str(lineBuffer+'\n'))

            # Line must end with a zero ('0') to tell 'dynamic' this is the EOF.
            profileSave.write('0')
            profileSave.close()
            print('Saved to "'+profile_path+'".')
            self.show_menu('main_menu')

        else:
            print("         ... unimplemented!")

    ##################################################
    # Preferences
    ##################################################
    # Load or save preferences?
    def preferences(self, action):

        # FIXME: Incomplete
        return


    ##################################################
    # Initialization
    ##################################################
    def __init__(self):
        w = Gtk.Window(title="Razer BlackWidow Chroma Configuration")
        w.set_wmclass('razer_bcd_utility', 'razer_bcd_utility')
        w.set_position(Gtk.WindowPosition.CENTER)
        w.set_size_request(900, 600)
        w.set_resizable(False)
        w.set_icon_from_file(os.path.join(Paths.location_data, 'img/app-icon.svg'))
        w.connect("delete-event", Gtk.main_quit)

        # Create WebKit Container
        global webkit # TODO remove global
        webkit = WebKit.WebView()
        sw = Gtk.ScrolledWindow()
        sw.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sw.add(webkit)

        # Build an auto expanding box and add our scrolled window
        b = Gtk.VBox(homogeneous=False, spacing=0)
        b.pack_start(sw, expand=True, fill=True, padding=0)
        w.add(b)

        # Disable right click context menu
        webkit.props.settings.props.enable_default_context_menu = False

        # Load page
        webkit.open(os.path.join(Paths.location_data, 'chroma_controller.html'))

        # Process pages once they fully load.
        webkit.connect('load-finished',self.page_loaded)

        # Process any commands from the web page.
        webkit.connect('navigation-policy-decision-requested', self.process_uri)

        # Show the window.
        w.show_all()
        Gtk.main()

class ChromaProfiles(object):
    ##################################################
    # Profile Creation
    ##################################################
    # Print the file names of existing profiles.
    # Requires 'Paths' class.

    @staticmethod
    def getFileList():
        return os.listdir(Paths.save_profiles)

    @staticmethod
    def setProfile(source='memory', profileName=None):
        print("Applying profile '"+profileName+"' ... ", end='')
        global profileMemory
        if Paths.dynamicExecType == 'sudoers':
          print("using 'sudo' as sudoers file was detected.")
          print('---[ Dynamic Output ]-----------------------')
          if source == 'file':
            os.system('cat "' + Paths.save_profiles + '/' + profileName + '" |  sudo ' + Paths.dynamicPath)
          elif source == 'memory':
            os.system('echo "' + profileMemory + '" |  sudo ' + Paths.dynamicPath)
          print('\n--------------------------------------------')

        elif Paths.dynamicExecType == 'pkexec':
          print("using an authentication prompt to execute 'dynamic' with higher privileges to send data directly to the keyboard.")
          print('---[ Dynamic Output ]-----------------------')
          os.system('echo "' + profileMemory+ '" |  pkexec ' + Paths.dynamicPath)
          print('\n--------------------------------------------')


if __name__ == "__main__":
    # Kill the process when CTRL+C'd.
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    # Connect to the DBUS daemon and determine paths.
    daemon = razer.daemon_dbus.DaemonInterface()
    Paths()

    # Show Time!
    utilty = ChromaController()
