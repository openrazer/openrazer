# SPDX-License-Identifier: GPL-2.0-or-later

import configparser

class PersistenceConfigParser(configparser.ConfigParser):
    status: dict[str, bool]
