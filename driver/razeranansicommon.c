#include "razeranansicommon.h"

struct razer_report razer_anansi_enable_macro_flash()
{
	struct razer_report report = get_razer_report(0x03, 0x04, 0x04);
	report.arguments[0] = 0x00;
	report.arguments[1] = 0x07;
	report.arguments[2] = 0x05;
	report.arguments[3] = 0x05;
	
	return report;
}


