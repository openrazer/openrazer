"""
A class that writes persistence data to disk when device state is updated.

Due to the architecture of the daemon, it's not as simple to know when
something has changed, so for now, an interval will periodically check for
changes in memory and write them to disk. This also avoids excessive
writes when lots of variables change at once.

This is essential because many desktop environments actually kill off
the daemon upon logout/shutdown, thereby persistence isn't retained across
sessions.

A known issue is that this doesn't monitor DPI changes via hardware buttons,
so this won't be persisted until the state is updated via the API.
"""
import time


class PersistenceAutoSave(object):
    def __init__(self, persistence, persistence_file, persistence_status, logger, interval, persistence_save_fn):
        self.persistence = persistence
        self.persistence_file = persistence_file
        self.persistence_status = persistence_status
        self.persistence_save_fn = persistence_save_fn
        self.logger = logger
        self.interval = interval

    def watch(self):
        # Run indefinitely until process is terminated
        while True:
            time.sleep(self.interval)

            if self.persistence_status["changed"]:
                self.logger.debug("State recently changed, writing to disk")
                self.persistence_status["changed"] = False
                self.persistence_save_fn(self.persistence_file)
