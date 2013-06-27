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

from autopilot.testcase import AutopilotTestCase
from autopilot.matchers import Eventually
from testtools.matchers import Equals, NotEquals

tests_dir = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))
test_app = os.path.join(tests_dir, 'hello_color.py')


class ActionsTest(AutopilotTestCase):
    """Test performing actions in the UI and verify results"""

    def setUp(self):
        super(ActionsTest, self).setUp()
        self.app = self.launch_test_application(test_app, app_type='gtk')

    def test_greeting_keyboard(self):
        """Greeting with keyboard navigation"""

        entries = self.app.select_many('GtkEntry')
        self.assertEqual(len(entries), 2)
        # the upper entry is for the name, the lower for the color
        # FIXME: once we have proper names (LP# 1082391), replace this with an
        # assertion
        if entries[0].globalRect[1] < entries[1].globalRect[1]:
            (entry_name, entry_color) = entries
        else:
            (entry_color, entry_name) = entries

        # FIXME: This isn't necessary for real X, but under Xvfb there is no
        # default focus sometimes
        if not entry_name.has_focus:
            self.mouse.click_object(entry_name)

        # type in name and color
        self.keyboard.type('Joe')
        self.keyboard.press_and_release('Tab')
        self.keyboard.type('red')

        # entries should now have the typed text
        self.assertThat(entry_name.text, Eventually(Equals('Joe')))
        self.assertThat(entry_color.text, Eventually(Equals('red')))

        # should not have any dialogs
        self.assertEqual(self.app.select_single('GtkMessageDialog'), None)

        # focus and activate the "Greet" button
        self.keyboard.press_and_release('Tab')
        self.keyboard.press_and_release('Enter')

        # should get the greeting dialog
        self.assertThat(lambda: self.app.select_single('GtkMessageDialog', visible=True),
                        Eventually(NotEquals(None)))
        md = self.app.select_single('GtkMessageDialog')

        # we expect the message dialog to show the corresponding greeting
        self.assertNotEqual(md.select_single('GtkLabel',
                                             label=u'Hello Joe, you like red.'),
                            None)

        # close the dialog
        self.keyboard.press_and_release('Enter')
        self.assertThat(lambda: self.app.select_single('GtkMessageDialog', visible=True),
                        Eventually(Equals(None)))

    def test_greeting_mouse(self):
        """Greeting with mouse navigation"""

        entries = self.app.select_many('GtkEntry')
        self.assertEqual(len(entries), 2)
        # the upper entry is for the name, the lower for the color
        # FIXME: once we have proper names (LP# 1082391), replace this with an
        # assertion
        if entries[0].globalRect[1] < entries[1].globalRect[1]:
            (entry_name, entry_color) = entries
        else:
            (entry_color, entry_name) = entries

        # FIXME: This isn't necessary for real X, but under Xvfb there is no
        # default focus sometimes
        if not entry_name.has_focus:
            self.mouse.click_object(entry_name)

        # type in name and color
        self.keyboard.type('Joe')
        self.mouse.click_object(entry_color)
        self.keyboard.type('blue')

        # entries should now have the typed text
        self.assertThat(entry_name.text, Eventually(Equals('Joe')))
        self.assertThat(entry_color.text, Eventually(Equals('blue')))

        # should not have any dialogs
        self.assertEqual(self.app.select_single('GtkMessageDialog'), None)

        # focus and activate the "Greet" button
        btn = self.app.select_single('GtkButton', label='Greet')
        self.assertNotEqual(btn, None)
        self.mouse.click_object(btn)

        # should get the greeting dialog
        self.assertThat(lambda: self.app.select_single('GtkMessageDialog', visible=True),
                        Eventually(NotEquals(None)))
        md = self.app.select_single('GtkMessageDialog')

        # we expect the message dialog to show the corresponding greeting
        self.assertNotEqual(md.select_single('GtkLabel',
                                             label=u'Hello Joe, you like blue.'),
                            None)

        # close the dialog
        btn = md.select_single('GtkButton', label='gtk-close')
        self.mouse.click_object(btn)
        self.assertThat(lambda: self.app.select_single('GtkMessageDialog', visible=True),
                        Eventually(Equals(None)))

    def test_clear(self):
        """Using Clear button with mouse"""

        # type in name and color
        self.keyboard.type('Joe')
        self.keyboard.press_and_release('Tab')
        self.keyboard.type('blue')

        # clear
        btn = self.app.select_single('GtkButton', label='gtk-delete')
        self.mouse.click_object(btn)

        # entries should be clear now
        entries = self.app.select_many('GtkEntry')
        self.assertEqual(len(entries), 2)
        for e in entries:
            self.assertThat(e.text, Eventually(Equals('')))

    def test_menu(self):
        """Browse the menu"""

        file_menu = self.app.select_single('GtkMenuItem', label='_File')
        help_menu = self.app.select_single('GtkMenuItem', label='_Help')
        self.assertNotEqual(file_menu, None)
        self.assertNotEqual(help_menu, None)

        # the top-level menus should be visible and thus have a rect
        for m in (file_menu, help_menu):
            self.assertGreaterEqual(m.globalRect[0], 0)
            self.assertGreaterEqual(m.globalRect[1], 0)
            self.assertGreater(m.globalRect[2], 0)
            self.assertGreater(m.globalRect[3], 0)

        # the submenus are not visible by default
        m = self.app.select_single('GtkImageMenuItem', label='gtk-open')
        self.assertFalse(hasattr(m, 'globalRect'))

        # after opening, submenus should become visible
        self.mouse.click_object(file_menu)
        # FIXME: getting a reference to this object once and then just querying
        # it doesn't work
        self.assertThat(lambda: hasattr(self.app.select_single('GtkImageMenuItem',
                                                               label='gtk-open'),
                                        'globalRect'),
                        Eventually(Equals(True)))
        m = self.app.select_single('GtkImageMenuItem', label='gtk-open')
        self.assertGreaterEqual(m.globalRect[0], 0)
        self.assertGreaterEqual(m.globalRect[1], 0)
        self.assertGreater(m.globalRect[2], 0)
        self.assertGreater(m.globalRect[3], 0)
