from razer.client.devices import RazerDevice as __RazerDevice


class RazerFirefly(__RazerDevice):
    def trigger_reactive(self) -> bool:
        """
        Trigger a reactive flash
        """
        return self._dbus_interfaces['device'].triggerReactive()