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
import daemon_dbus

# Persistent colours
rgb_red = 0;
rgb_green = 255;
rgb_blue = 0;

class Paths():
    # Initializes paths and directories

    ## Where are we?
    location = os.path.dirname( os.path.abspath(inspect.getfile(inspect.currentframe())) )
    location_data = os.path.join(location, 'data/')

    ## Where user's configuration is saved
    dir_root = os.path.expanduser('~') + '/.config/razer_chroma'
    dir_profiles = dir_root + '/profiles'

    ## Check 'data' folder exists.
    if ( os.path.exists(location_data) == False ):
        print('Data folder is missing. Exiting.')
        exit()

    ## Check we have a folder to save data (eg. profiles)
    if ( os.path.exists(dir_root) == False ):
        print('Configuration folder does not exist. Creating',dir_root,)
        os.makedirs(dir_root)
        os.makedirs(dir_profiles)

    # Does the 'dynamic' binary exist? (required for key profiles)
    ### In same folder as script.
    if os.path.exists(location + '/dynamic') == True:
        dynamic = location + 'dynamic'

    ### Compiled in Git project 'examples' folder
    elif os.path.exists(location + '/../../examples/dynamic') == True:
        dynamic = location + '/../../examples/dynamic'
    else:
        dynamic = 'notfound'


class ChromaController(object):
    ##################################################
    # Page Switcher
    ##################################################
    def show_menu(self, page):
        print("Opening menu '"+page+"'")

        # Hide all footer buttons
        webkit.execute_script('$("#retry").hide()')
        webkit.execute_script('$("#edit-save").hide()')
        webkit.execute_script('$("#edit-discard").hide()')
        webkit.execute_script('$("#close-window").hide()')

        if page == 'main_menu':
            webkit.execute_script('changeTitle("Configuration Menu")')
            webkit.execute_script('$("#'+page+'").fadeIn("slow")')
            webkit.execute_script('$("#close-window").show()')

        elif page == 'not_detected':
            webkit.execute_script('changeTitle("Keyboard Not Detected")')
            return 0
        elif page == 'profile_editor':
            webkit.execute_script('changeTitle("Edit '+profile+'")')
            return 0
        elif page == 'preferences':
            webkit.execute_script('changeTitle("Preferences")')
            return 0
        else:
            print("Unknown menu '"+page+"'!")

    ##################################################
    # Chroma Keyboard Detection
    ##################################################
    # Check if the Chroma is plugged in as soon as the page finishes loading.
    def page_loaded(self, WebView, WebFrame):
        print("Detecting Chroma Keyboard... ", end='')
        # FIXME: NOT YET IMPLEMENTED
        # print('found at ####')
        print("\nfixme: page_loaded detect chroma keyboard")
        #  -- If it is, show the main menu.
        # --  If not, kindly ask the user to do so.
        self.show_menu('main_menu')

        # Load preferences

        # Apply preferences
        #~ webkit.execute_script('$("#rgb_preview").css("background-color","rgba('+str(rgb_red)+','+str(rgb_green)+','+str(rgb_blue)+',1.0)")')

        # Does 'dynamic' exist? Without this, profiles can't be activated!
        # See '/../examples/dynamic-examples/readme' for more details.
        # If this is integrated to dbus, this external binary won't be needed (nor require root privileges to execute)
        if Paths.dynamic == 'notfound':
            webkit.execute_script('$("#profiles-activate").addClass("btn-disabled")')
            print("WARNING: Binary for 'dynamic' not found!")
            print("         You won't be able to activate any key profiles created with the utility without compiling the program and placing beside this script.")
            print("         See '/../examples/dynamic-examples/readme' for details about 'dynamic'.")

    ##################################################
    # Commands
    ##################################################
    def process_uri(self, view, frame, net_req, nav_act, pol_dec):
        uri = net_req.get_uri()
        frame.stop_loading()

        if uri.startswith('cmd://'):
            command = uri[6:]
            print("Command: '"+command+"'")
            self.process_command(command)

        if uri.startswith('web://'):
            webURL = uri[6:]
            print('fixme:open web browser to URL')

    def process_command(self, command):
        global rgb_red, rgb_green, rgb_blue, current_effect
        global webkit
        if command == 'quit':
            quit()

        elif command.startswith('brightness'):
            value = int(command[11:])
            daemon.SetBrightness(value)

        elif command.startswith('effect'):
            if command == 'effect-none':
                current_effect = "none"
                daemon.SetEffect('none')
                webkit.execute_script('$("#rgb").fadeOut("fast")')
                webkit.execute_script('$("#waves").fadeOut("fast")')

            elif command == 'effect-spectrum':
                current_effect = "spectrum"
                daemon.SetEffect('spectrum')
                webkit.execute_script('$("#rgb").fadeOut("fast")')
                webkit.execute_script('$("#waves").fadeOut("fast")')

            elif command.startswith('effect-wave'):
                current_effect = "wave"
                daemon.SetEffect('wave',int(command[12:])) # ?1 or ?2 for direction
                webkit.execute_script('smoothFade("#rgb","#waves")')

            elif command == 'effect-reactive':
                current_effect = "reactive"
                daemon.SetEffect('reactive', rgb_red, rgb_green, rgb_blue)
                webkit.execute_script('smoothFade("#waves","#rgb")')

            elif command == 'effect-breath':
                current_effect = "breath"
                daemon.SetEffect('breath', rgb_red, rgb_green, rgb_blue)
                webkit.execute_script('smoothFade("#waves","#rgb")')

            elif command == 'effect-static':
                current_effect = "static"
                daemon.SetEffect('static', rgb_red, rgb_green, rgb_blue)
                webkit.execute_script('smoothFade("#waves","#rgb")')

        elif command == 'enablemarcokeys':
            daemon.MarcoKeys(True)
            webkit.execute_script('$("#marco-keys-enable").addClass("btn-disabled")')
            webkit.execute_script('$("#marco-keys-enable").html("Marco Keys In Use")')

        elif command == 'gamemode_enable':
            daemon.GameMode(True)
            webkit.execute_script('$("#game-mode-status").html("Enabled")')
            webkit.execute_script('$("#game-mode-enable").hide()')
            webkit.execute_script('$("#game-mode-disable").show()')

        elif command == 'gamemode_disable':
            daemon.GameMode(False)
            webkit.execute_script('$("#game-mode-status").html("Disabled")')
            webkit.execute_script('$("#game-mode-enable").show()')
            webkit.execute_script('$("#game-mode-disable").hide()')

        elif command == 'ask-color':
            colorseldlg = Gtk.ColorSelectionDialog("Choose a colour")
            colorsel = colorseldlg.get_color_selection()

            if colorseldlg.run() == Gtk.ResponseType.OK:
                color = colorsel.get_current_color()
                red = int(color.red / 256)
                green = int(color.green / 256)
                blue = int(color.blue / 256)
                command = 'set-color?'+str(red)+'?'+str(green)+'?'+str(blue)
                self.process_command(command)

            colorseldlg.destroy()

        elif command.startswith('set-color') == True:
            # Expects 3 parameters separated by '?' in order: red, green, blue (0-255)
            colors = command.split('set-color?')[1]
            rgb_red = int(colors.split('?')[0])
            rgb_green = int(colors.split('?')[1])
            rgb_blue = int(colors.split('?')[2])
            print("Set colour to: RGB("+str(rgb_red)+", "+str(rgb_green)+", "+str(rgb_blue)+")")
            webkit.execute_script('$("#rgb_preview").css("background-color","rgba('+str(rgb_red)+','+str(rgb_green)+','+str(rgb_blue)+',1.0)")')

            # Update static colour effects if currently in use.
            if current_effect == 'static':
                self.process_command('effect-static')
            elif current_effect == 'breath':
                self.process_command('effect-breath')
            elif current_effect == 'reactive':
                self.process_command('effect-reactive')
            webkit.execute_script('$("#rgb").fadeIn()')

        elif command == 'apply':
            return
            #~ daemon.GameMode(False)
            #~ webkit.execute_script('$("#game-mode-status").html("Disabled")')
            #~ webkit.execute_script('$("#game-mode-enable").show()')
            #~ webkit.execute_script('$("#game-mode-disable").hide()')

        else:
            print("         ... unimplemented!")


    ##################################################
    # Initialization
    ##################################################
    def __init__(self):
        w = Gtk.Window(title="Razer BlackWidow Chroma Configuration")
        w.set_wmclass('razer_bcd_utility', 'razer_bcd_utility')
        w.set_position(Gtk.WindowPosition.CENTER)
        w.set_size_request(800, 500)
        w.set_resizable(False)
        w.set_icon_from_file(os.path.join(Paths.location_data, 'img/app-icon.svg'))
        w.connect("delete-event", Gtk.main_quit)

        # Create WebKit Container
        global webkit
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


if __name__ == "__main__":
    # Kill the process when CTRL+C'd.
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    # Connect to the DBUS daemon and determine paths.
    daemon = daemon_dbus.DaemonInterface()
    Paths()

    # Show Time!
    utilty = ChromaController()
