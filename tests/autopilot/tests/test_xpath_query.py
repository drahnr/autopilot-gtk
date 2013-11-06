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


class XPathQueryTest(AutopilotTestCase):
    """XPath queries"""

    def setUp(self):
        super(XPathQueryTest, self).setUp()
        self.app = self.launch_test_application(test_app, app_type='gtk')

    def xtest_have_path(self):
        """All children have a unique path"""

        widgets = set()
        self._get_widgets(self.app, widgets)

        seen_paths = set()
        for widget in widgets:
            path = widget.get_class_query_string()
            self.assertNotIn(path, seen_paths)
            seen_paths.add(path)

            # we can resolve the path back to the widget
            state = self.app.get_state_by_path(path)
            # state is an array with one (path, props) element
            props = state[0][1]
            self.assertEqual(props['id'], widget.id)
            self.assertEqual(props['visible'], widget.visible)

    def xtest_select_full_path(self):
        """Select widgets with full XPath"""

        # three buttons in main dialog's ButtonBox
        state = self.app.get_state_by_path('/Root/GtkWindow/GtkBox/GtkButtonBox/GtkButton')
        self.assertEqual(len(state), 3)
        labels = [str(props[1]['label']) for props in state]
        labels.sort()
        self.assertEqual(labels, ['Greet', 'gtk-delete', 'gtk-quit'])

        # select button with particular label
        for l in ['Greet', 'gtk-delete', 'gtk-quit']:
            state = self.app.get_state_by_path('/Root/GtkWindow/GtkBox/GtkButtonBox/GtkButton[label=%s]' % l)
            self.assertEqual(len(state), 1)
            self.assertEqual(state[0][1]['label'], l)

    def xtest_select_path_pattern(self):
        """Select widgets with XPath path pattern"""

        # three buttons in main dialog's ButtonBox
        state = self.app.get_state_by_path('//GtkWindow//GtkButton')
        self.assertEqual(len(state), 3)
        labels = [str(props[1]['label']) for props in state]
        labels.sort()
        self.assertEqual(labels, ['Greet', 'gtk-delete', 'gtk-quit'])

        # at least four buttons in the whole tree
        state = self.app.get_state_by_path('/Root//GtkButton')
        self.assertGreaterEqual(len(state), 4)

    def test_select_by_attribute(self):
        """Select widgets with attribute pattern"""

        state = self.app.get_state_by_path('//*[label="gtk-delete"]')
        self.assertEqual(len(state), 1, state)
        self.assertEqual(state[0][1]['label'], [0, 'gtk-delete'])
        self.assertTrue(state[0][0].endswith('/GtkButton'), state[0][0])

    # https://launchpad.net/bugs/1179806
    # TODO: Make this pass!
    @unittest.expectedFailure
    def test_select_by_attribute_spaces(self):
        """Select widgets with attribute pattern containing spaces"""

        for state_str in ('//*[label="Hello\\x20Color!"]', '//*[label="Hello Color!"]'):
            state = self.app.get_state_by_path(state_str)
            self.assertEqual(len(state), 1, str(state))
            self.assertEqual(state[0][1]['label'], 'Hello Color!')
            self.assertTrue(state[0][0].endswith('/GtkLabel'), state[0][0])

    @classmethod
    def _get_widgets(klass, obj, widget_set):
        """Recursively add all children of obj to widget_set"""

        for c in obj.get_children():
            widget_set.add(c)
            klass._get_widgets(c, widget_set)
