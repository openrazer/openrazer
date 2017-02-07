Changes that are from the Linux driver via WIN32 and WIN64 defines

(1) Update (literal array terminator):
```
// C2059: syntax error: '}'
#if defined(WIN32) || defined(_WIN64)
{ 0 }
#else
{ }
#endif
```
Applied to:
```
static const struct hid_device_id razer_devices[] = {
static const struct razer_key_translation chroma_keys[] = {
```
in file(s):
```
razer*_driver.c
```

(2) Update (static for duplicate functions):
```
//LNK2005 already defined in razer*_driver.obj
#if defined(WIN32) || defined(_WIN64)
static
#endif
```
Applied to:
```
struct razer_report razer_send_payload(
int razer_get_report(
void razer_set_device_mode(
```
in file(s):
```
razer*_driver.c
```

(3) Update (union cast):
```
//C2440 'type cast': cannot convert from 'unsigned char' to
'razer_kraken_effect_byte'
#if defined(WIN32) || defined(_WIN64)
unsigned char effect_byte1 = get_current_effect(dev);
union razer_kraken_effect_byte effect_byte;
memcpy(&effect_byte, &effect_byte1, sizeof(unsigned char));
#else
 union razer_kraken_effect_byte effect_byte = (union
razer_kraken_effect_byte)get_current_effect(dev);
#endif
```
Applied to:
```
static ssize_t razer_attr_read_mode_breath(
```
in file(s):
```
razerkraken_driver.c
```

(4) Update (device macro repurpose to DLL API calls):
```
#if defined(WIN32) || defined(_WIN64)
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(*, _name,
_mode, _show, _store)
#endif
```
Applied to:
```
static DEVICE_ATTR(
```
in file(s):
```
razer*_driver.c
```
