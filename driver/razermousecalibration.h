#ifndef DRIVER_RAZERMOUSECALIBRATION_H_
#define DRIVER_RAZERMOUSECALIBRATION_H_

#include "razercommon.h"


/*
 * Calibration  Functions
 */
struct razer_report razer_calib_set_mode(unsigned char sensor, unsigned char calib_mode);

struct razer_report razer_calib_set_parameters(unsigned char sensor, unsigned char calib_params[]);
/*
struct razer_report razer_calib_set_parameters(unsigned char param0, unsigned char param1, unsigned char param2, unsigned char param7);
*/

struct razer_report razer_calib_start_surface_data_acquisition(unsigned char sensor, unsigned char sensor1);
struct razer_report razer_calib_get_surface_data_acquisition(unsigned char sensor);


#endif

