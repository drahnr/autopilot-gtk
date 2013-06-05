

from autopilot.testcase import AutopilotTestCase
from testtools.matchers import NotEquals



class PropertyMatchingTest(AutopilotTestCase):

    def setUp(self):
        super(PropertyMatchingTest, self).setUp()
        self.app = self.launch_test_application('gedit')

    def test_integer_matches(self):
        """Test property matching for integers.

        Find an opaque GtkWindow in Gedit.
        """

        opaque_window = self.app.select_many('GtkWindow', opacity=1)
        self.assertThat(opaque_window, NotEquals(None))
        

    def test_string_matches(self):
        """Match a string property.

        Find an GtkImageMenuItem named 'BookmarkOpen' in Gedit.
        """

        bookmark_open_item = self.app.select_single('GtkImageMenuItem',
                                                    name='BookmarkOpen')
        self.assertThat(bookmark_open_item, NotEquals(None))
