# blackbox testing of autopilot API against gnome-system-log
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


from autopilot.testcase import AutopilotTestCase
from autopilot.matchers import Eventually
from testtools.matchers import Equals, NotEquals


class GnomeAppTest(AutopilotTestCase):
    """Test autopilot against an actual GNOME application"""

    def setUp(self):
        super(GnomeAppTest, self).setUp()
        self.patch_environment('LANGUAGE', '')
        self.patch_environment('LANG', '')
        self.patch_environment('LC_MESSAGES', 'C')
        self.app = self.launch_test_application('gnome-system-log', '/etc/issue')

    def test_filename_label(self):
        """Find file name label"""

        l = self.app.select_single('GtkLabel', label=u'<b>issue</b>')
        self.assertNotEqual(l, None)
        self.assertEqual(l.visible, True)

    def test_search(self):
        """Run a search"""

        revealer = self.app.select_single('GdRevealer')
        if revealer:
            # search bar not visible by default
            self.assertEqual(revealer.child_revealed, False)
        else:
            # g-s-l < 3.8 did not have the GdRevealer object yet
            findbar = self.app.select_single('LogviewFindbar')
            self.assertEqual(findbar.visible, False)

        search_btn = self.app.select_single('GtkToggleButton')
        self.assertNotEqual(search_btn, None)
        self.mouse.click_object(search_btn)

        # should trigger search bar
        if revealer:
            self.assertThat(lambda: revealer.child_revealed, Eventually(Equals(True)))
        else:
            self.assertThat(lambda: findbar.visible, Eventually(Equals(True)))
        search = self.app.select_single('GtkSearchEntry', visible=True)
        self.assertTrue(search.has_focus)

        # something that will not be in /etc/issue
        self.keyboard.type('Bogus12!')
        self.assertThat(lambda: self.app.select_single('GtkLabel', label=u'No matches found'),
                        Eventually(NotEquals(None)))
