# blackbox testing of autopilot API against gnome-calculator
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
from testtools.matchers import Equals


class GnomeAppTest(AutopilotTestCase):
    """Test autopilot against an actual GNOME application"""

    def setUp(self):
        super(GnomeAppTest, self).setUp()
        self.patch_environment('LANGUAGE', '')
        self.patch_environment('LANG', '')
        self.patch_environment('LC_MESSAGES', 'C')
        self.app = self.launch_test_application('gnome-calculator')

    def test_builder_button(self):
        """Find button by builder ID"""

        l = self.app.select_single(BuilderName='calc_result_button')
        self.assertNotEqual(l, None)
        self.assertEqual(l.visible, True)
        self.assertEqual(l.label, '=')

    def test_calc(self):
        """Run a calculation"""

        display = self.app.select_single(BuilderName='displayitem')
        self.mouse.click_object(display)
        self.assertThat(display.buffer, Equals(''))

        self.keyboard.type('1+1')
        self.keyboard.press_and_release('Enter')
        self.assertThat(display.buffer, Eventually(Equals('2')))
