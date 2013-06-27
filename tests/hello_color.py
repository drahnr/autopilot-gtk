#!/usr/bin/python

import sys
import os.path

from gi.repository import Gtk


class HelloColorApp(Gtk.Application):
    def __init__(self):
        self.widgets = Gtk.Builder.new()
        self.widgets.add_from_file((os.path.join(os.path.dirname(sys.argv[0]), 'hello_color.ui')))
        assert self.widgets.connect_signals(self) is None

    def run(self):
        self.widgets.get_object('window_app').show()
        Gtk.main()

    def on_quit(self, *args):
        Gtk.main_quit()

    def on_file_open(self, *args):
        md = Gtk.FileChooserDialog('Select a file..',
                                   parent=self.widgets.get_object('window_app'),
                                   buttons=(Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                            Gtk.STOCK_OPEN, Gtk.ResponseType.OK))
        result = md.run()
        md.hide()
        if result == Gtk.ResponseType.OK:
            self.widgets.get_object('label_status').set_text('Loaded %s' % md.get_filenames()[0])

    def on_button_greet(self, *args):
        name = self.widgets.get_object('entry_name').get_text()
        color = self.widgets.get_object('entry_color').get_text()

        md = Gtk.MessageDialog(message_type=Gtk.MessageType.INFO,
                               buttons=Gtk.ButtonsType.CLOSE,
                               text='Hello %s, you like %s.' % (name, color))
        md.run()
        md.hide()

    def on_button_clear(self, *args):
        self.widgets.get_object('entry_name').set_text('')
        self.widgets.get_object('entry_color').set_text('')
        self.widgets.get_object('label_status').set_text('')

    def on_about(self, *args):
        d = self.widgets.get_object('dialog_about')
        d.run()
        d.hide()

if __name__ == '__main__':
    HelloColorApp().run()
