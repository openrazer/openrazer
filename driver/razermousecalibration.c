#include "razermousecalibration.h"

/* For DAE: Pixart 3389 sensor and NXP LPC11U35F microcontroller */
/* For Philips sensors we probably need pcaps */

/*
* Precalibrated razer mousemat settings (param7 ~~ liftoff offset values)
* (Probably better solution using razer files with the frontend app than hardcode here all precalib tables)
*
* static unsigned char destructor_2_mm[] = { level1, level2, ...., level10 }
*
*
* static unsigned char destructor_2_mm[] = { 0x3A, 0x38, 0x34, 0x28, 0x24, 0x1F, 0x1A, 0x16, 0x14, 0x12 }
* static unsigned char gigantus_mm[]
* static unsigned char sphex_v2_mm[]
* static unsigned char custom_qck_mm[]
*/

// unsigned char parameters[] = "";

/* ?¿?¿?¿?¿?¿?¿?¿?¿??¿?¿?¿??¿?¿?¿?
*
* Status Trans Packet Proto DataSize Class CMD Args
* 00     1f    0000   00    0b       0f    03  00 00 00 00 01 80 00 80 80 00 80 | ?¿?¿??¿? (without control response data ?¿?¿?)
*
* 00     1f    0000   00    06       0f    02  00 00 08 00 00 00 | ?¿??¿??¿
*/





/* Status Trans Packet Proto DataSize Class CMD Args
*  00     1f    0000   00    03       0b    03  000401                     | SET CALIB MODE (MODE ON) ~~ ENABLE CALIB OPERATION
*
*  00     1f    0000   00    03       0b    03  000400                     | SET CALIB MODE (MODE OFF) ~~ SET CALIB TO DEFAULTS
*/

/* 0x0b 	0x03 	args either 000500 or 000501 - could be ""enable/disable"" calibration in a Philips sensor
*
* Trans.id in philips or 4g infrared sensor (different type of microcontrollers ?¿?) ?¿?
*/

struct razer_report razer_calib_set_mode(unsigned char sensor, unsigned char calib_mode)
{
	struct razer_report report = get_razer_report(0x0B, 0x03, 0x03);
//	report.transaction_id.id = 0x1F;

	report.arguments[0] = 0x00;
	report.arguments[1] = sensor;
	report.arguments[2] = calib_mode;

	return report;
}


/*Status Trans Packet Proto DataSize Class CMD Args
* 00     1f    0000   00    0b       0b    05  00040A1083000000002800 | set mouse with calibration data
*
* param0 = 0A, param1 = 10, param2 = 83, param7= 28
*
* param0 = argument[2], param1 = argument[3], param2 = argument[4], param7 = argument[9]
*/

struct razer_report razer_calib_set_parameters(unsigned char sensor, unsigned char calib_params[])
{
        struct razer_report report = get_razer_report(0x0B, 0x05, 0x0B);
//	report.transaction_id.id = 0x1F;

        report.arguments[0] = 0x00;
        report.arguments[1] = sensor;
        report.arguments[2] = calib_params[0];
        report.arguments[3] = calib_params[1];
        report.arguments[4] = calib_params[2];
        report.arguments[5] = calib_params[3];
        report.arguments[6] = calib_params[4];
        report.arguments[7] = calib_params[5];
        report.arguments[8] = calib_params[6];
        report.arguments[9] = calib_params[7];
        report.arguments[10] = calib_params[8];
	report.arguments[11] = 0x00;

        return report;
}
/**************
struct razer_report razer_calib_set_parameters(unsigned char sensor, unsigned char param0, unsigned char param1, unsigned char param2, unsigned char param3, unsigned char param4, unsigned char param5, unsigned char param6, unsigned char param7, unsigned char param8)
{
        struct razer_report report = get_razer_report(0x0B, 0x05, 0x0B);
	report.transaction_id.id = 0x1F;

        report.arguments[0] = 0x00;
        report.arguments[1] = sensor;
        report.arguments[2] = param0;
        report.arguments[3] = param1;
        report.arguments[4] = param2;
        report.arguments[5] = param3;
        report.arguments[6] = param4;
        report.arguments[7] = param5;
        report.arguments[8] = param6;
        report.arguments[9] = param7;
        report.arguments[10] = param8;

        return report;
}
***************/

/*Status Trans Packet Proto DataSize Class CMD Args
* 00     1f    0000   00    04       0b    09  00040000 | start surface data acq.
*/

/* 0x0b 	0x09 	args 00050100 - could be START SURFACE DATA ACQUISITION when using a Philips sensor */

struct razer_report razer_calib_start_surface_data_acquisition(unsigned char sensor, unsigned char sensor1)
{
	struct razer_report report = get_razer_report(0x0B, 0x09, 0x04);
//	report.transaction_id.id = 0x1F;

        report.arguments[0] = 0x00;
        report.arguments[1] = sensor;
        report.arguments[2] = sensor1;
        report.arguments[3] = 0x00;

	return report;
}



/*Status Trans Packet Proto DataSize Class CMD Args
* 00     1f    0000   00    0b       0b    85  0004000000000000000000 | ask for surface data
*
* 02     1f    0000   00    0b       0b    85  000418EA02000000000058 | mouse response with surface data
*
* param0 = 18, param1 = EA, param2 = 02, param7= 58
*
* param0 = argument[2], param1 = argument[3], param2 = argument[4], param7 = argument[10]
*/


struct razer_report razer_calib_get_surface_data_acquisition(unsigned char sensor)
{
        struct razer_report report = get_razer_report(0x0B, 0x85, 0x0B);
//	report.transaction_id.id = 0x1F;

        report.arguments[0] = 0x00;
        report.arguments[1] = sensor;
        report.arguments[2] = 0x00;
        report.arguments[3] = 0x00;
        report.arguments[4] = 0x00;
        report.arguments[5] = 0x00;
        report.arguments[6] = 0x00;
        report.arguments[7] = 0x00;
        report.arguments[8] = 0x00;
        report.arguments[9] = 0x00;
        report.arguments[10] = 0x00;

       // struct razer_report response = razer_send_payload(usb_dev, &report);

       //  return sprintf(buf, "%d:%d:%d:%d\n", response.arguments[2], response.arguments[3], response.arguments[4], response.arguments[10]);
       return report;
}

