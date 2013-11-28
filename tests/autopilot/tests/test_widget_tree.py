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

from autopilot.introspection.dbus import StateNotFoundError
from autopilot.testcase import AutopilotTestCase
from testtools.matchers import raises

tests_dir = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))
test_app = os.path.join(tests_dir, 'hello_color.py')


class WidgetTreeTest(AutopilotTestCase):
    """Widget tree iteration and search"""

    def setUp(self):
        super(WidgetTreeTest, self).setUp()
        self.app = self.launch_test_application(test_app, app_type='gtk')

    def test_get_children_recursive(self):
        """Recursive get_children()

        This should not crash, and deliver valid widgets.
        """
        widgets = set()
        self._get_widgets(self.app, widgets)
        for c in widgets:
            self.assertIn('.Gtk', str(type(c)))
            self.assertGreaterEqual(c.id, 0)
            # uncomment this to get a dump of all widgets and properties
            #print(type(c))
            #for p in c.get_properties():
            #    print '  ', p, repr(getattr(c, p))

    def test_get_children_by_type(self):
        # multiple instances
        res = self.app.get_children_by_type('GtkWindow')
        self.assertGreaterEqual(len(res), 3)
        self.assertIn('.GtkWindow', str(type(res[0])))

        # one qualified instance
        res = self.app.get_children_by_type('GtkWindow', Children=['GtkBox'])
        self.assertGreaterEqual(len(res), 1)

        # no instances
        self.assertEqual(self.app.get_children_by_type('GtkTable'), [])

    def test_select_single_unique(self):
        """select_single() on widget types with only one instance"""

        for wtype in ('GtkMenuBar', 'GtkAboutDialog', 'GtkGrid'):
            w = self.app.select_single(wtype)
            self.assertIn('.' + wtype, str(type(w)))

    def test_select_single_nonunique(self):
        """select_single() on widget types with multiple instances"""

        # we have more than one instance of these
        for wtype in ('GtkButton', 'GtkEntry'):
            self.assertRaises(ValueError, self.app.select_single, wtype)

        # we have no instances of these
        for wtype in ('GtkTable', 'GtkRadioButton'):
            self.assertThat(
                lambda: self.app.select_single(wtype),
                raises(StateNotFoundError)
            )

        # qualified: visible property is not unique
        self.assertRaises(ValueError,
                          self.app.select_single, 'GtkButton', visible=True)

        # qualified: label property is unique within GtkButton
        w = self.app.select_single('GtkButton', label='gtk-quit')
        self.assertIn('.GtkButton', str(type(w)))
        self.assertEqual(w.label, 'gtk-quit')

    def test_select_single_noclass(self):
        """select_single() without specifying a class"""

        # gtk-delete label is unique to our Button
        w = self.app.select_single(label='gtk-delete')
        self.assertIn('.GtkButton', str(type(w)))
        self.assertEqual(w.label, 'gtk-delete')

        # gtk-quit label is not unique globally, it's also a menu item
        self.assertRaises(ValueError, self.app.select_single, label='gtk-quit')

        # ... but it is unique for focussable widgets (menus don't allow that)
        w = self.app.select_single(label='gtk-quit', can_focus=True)
        self.assertIn('.GtkButton', str(type(w)))
        self.assertEqual(w.label, 'gtk-quit')

    def test_select_many_string(self):
        """select_many() with string properties"""

        # by class, unqualified, multiple instances
        res = self.app.select_many('GtkButton')
        # we have three in our main window, plus some in the about dialog
        self.assertGreaterEqual(len(res), 3)
        self.assertIn('.GtkButton', str(type(res[0])))

        # .. but exactly three in the main window
        main_window = self.app.select_single('GtkWindow', Children=['GtkBox'], visible=True)
        res = main_window.select_many('GtkButton')
        self.assertEqual(len(res), 3)

        # by class, unqualified, single instance
        res = self.app.select_many('GtkMenuBar')
        self.assertEqual(len(res), 1)
        self.assertIn('.GtkMenuBar', str(type(res[0])))

        # by class, unqualified, no instance
        res = self.app.select_many('GtkTable')
        self.assertEqual(res, [])

        # by class, qualified
        res = self.app.select_many('GtkButton', label='Greet')
        self.assertEqual(len(res), 1)
        self.assertIn('.GtkButton', str(type(res[0])))
        self.assertEqual(res[0].label, 'Greet')

        # untyped
        res = self.app.select_many(label='gtk-delete')
        self.assertEqual(len(res), 1)
        self.assertIn('.GtkButton', str(type(res[0])))
        self.assertEqual(res[0].label, 'gtk-delete')

        res = self.app.select_many(label='gtk-quit')
        # button and menu item
        self.assertEqual(len(res), 2)

    def test_select_int(self):
        """select_*() with int properties"""

        # with class
        res = self.app.select_many('GtkButtonBox', border_width=5)
        self.assertEqual(len(res), 1)

        self.assertNotEqual(self.app.select_single('GtkButtonBox', border_width=5), None)

        # without class
        res = self.app.select_many(border_width=5)
        self.assertGreater(len(res), 2)

        self.assertNotEqual(self.app.select_single(border_width=2), None)

    def test_select_bool(self):
        """select_*() with boolean properties"""

        # with class
        res = self.app.select_many('GtkButton', visible=True)
        self.assertGreater(len(res), 2)

        res = self.app.select_many('GtkAboutDialog', visible=False)
        self.assertGreater(len(res), 0)

        # without class
        res = self.app.select_many(visible=True)
        self.assertGreater(len(res), 5)

        res = self.app.select_many(visible=False)
        self.assertGreater(len(res), 4)

    @classmethod
    def _get_widgets(klass, obj, widget_set):
        """Recursively add all children of obj to widget_set"""

        for c in obj.get_children():
            widget_set.add(c)
            klass._get_widgets(c, widget_set)
