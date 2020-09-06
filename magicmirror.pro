TEMPLATE = subdirs

SUBDIRS = main widgets utils

widgets.depends = utils
main.depends = widgets utils
