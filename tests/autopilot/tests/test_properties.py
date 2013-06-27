# blackbox testing of autopilot API against our hello_color.py test GTK program
# Author: Martin Pitt <martin.pitt@ubuntu.com>
# Copyright (C) 2013 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os.path
import unittest

from autopilot.testcase import AutopilotTestCase

tests_dir = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))
test_app = os.path.join(tests_dir, 'hello_color.py')


class PropertyTest(AutopilotTestCase):
    """Widget properties"""

    def setUp(self):
        super(PropertyTest, self).setUp()
        self.app = self.launch_test_application(test_app, app_type='gtk')

    def test_button(self):
        """GtkButton properties"""

        btn_greet = self.app.select_single('GtkButton', label='Greet')
        self.assertNotEqual(btn_greet, None)
        btn_quit = self.app.select_single('GtkButton', label='gtk-quit')
        self.assertNotEqual(btn_quit, None)

        self.assertEqual(btn_greet.use_stock, False)
        self.assertEqual(btn_quit.use_stock, True)

        # only GtkButton, GtkFileChooserButton, and GtkComboBox have
        # focus-on-click, and we don't use the latter two
        self.assertEqual(btn_greet.focus_on_click, True)
        self.assertEqual(btn_quit.focus_on_click, True)

        # all buttons are visible and thus should have a rect
        self.assertTrue(btn_greet.visible)
        self.assertTrue(btn_quit.visible)
        self.assertEqual(len(btn_greet.globalRect), 4)
        self.assertEqual(len(btn_quit.globalRect), 4)

    def test_entry(self):
        """GtkEntry properties"""

        entries = self.app.select_many('GtkEntry')
        self.assertEqual(len(entries), 2)
        # the upper entry is for the name, the lower for the color
        # FIXME: once we have proper names (LP# 1082391), replace this with an
        # assertion
        if entries[0].globalRect[1] < entries[1].globalRect[1]:
            (entry_name, entry_color) = entries
        else:
            (entry_color, entry_name) = entries

        self.assertTrue(entry_name.visible)
        self.assertTrue(entry_color.visible)

        # the entries should have the same size and x alignment
        self.assertEqual(entry_name.globalRect[0], entry_color.globalRect[0])
        self.assertEqual(entry_name.globalRect[2:], entry_color.globalRect[2:])

        # FIXME: This isn't necessary for real X, but under Xvfb there is no
        # default focus sometimes
        if not entry_name.has_focus:
            self.mouse.click_object(entry_name)

        # first entry has default focus
        self.assertEqual(entry_name.has_focus, True)
        self.assertEqual(entry_color.has_focus, False)

        # both entries are empty by default
        self.assertEqual(entry_name.text, '')
        self.assertEqual(entry_color.text, '')

        # text-length is an unique property for GtkEntry
        self.assertEqual(entry_name.text_length, 0)
        self.assertEqual(entry_color.text_length, 0)

    #https://launchpad.net/bugs/1193342
    @unittest.expectedFailure
    def test_enum_properties(self):
        '''enum properties'''

        btn_greet = self.app.select_single('GtkButton', label='Greet')
        self.assertTrue(hasattr(btn_greet, 'relief'))
