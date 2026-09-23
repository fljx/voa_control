#include "voa.hpp"

#include <iostream>
#include <print>

int main()
{
	// Hardware configuration matched to your 16-bit DAC circuit setup
	hardware_config hw_agiltron{
		.dac_max_code = 65535, // 16-bit DAC
		.hardware_vmax = 6.0, // Datasheet SM/PM driving-voltage range
		.v_ref = 5.0, // 5V Reference
		.hardware_gain =
			1.2 // Op-amp scales the DAC range to the 6V VOA limit
	};

	// Piecewise-linear inverse of the datasheet's typical attenuation-vs-voltage
	// curve. Coefficients are voltage = slope * attenuation + offset.
	std::vector<calibration_segment> cal_agiltron = {
		{0.5, 1.5, {1.0 / 1.1, 1.0 - 0.4 / 1.1}},
		{1.5, 6.0, {1.0 / 4.5, 2.0 - 1.5 / 4.5}},
		{6.0, 18.0, {1.0 / 12.0, 3.0 - 6.0 / 12.0}},
		{18.0, 35.0, {1.0 / 17.0, 4.0 - 18.0 / 17.0}},
		{35.0, 40.0, {1.0 / 15.0, 5.0 - 35.0 / 15.0}}};

	voa_controller controller(hw_agiltron, cal_agiltron);

	// Pull the absolute calibration bounds to feed our sweep dynamically
	double min_attenuation = controller.get_absolute_min_db(); // 0.5 dB
	double max_attenuation = controller.get_absolute_max_db(); // 40.0 dB
	double step_size = 0.5; // Sweep step size in dB

	std::print("Starting Sweep from Minimum to Maximum Attenuation...\n");
	std::print(
		"--------------------------------------------------------\n");
	std::print("Target (dB) | Computed DAC Code (Raw Integer)\n");
	std::print(
		"--------------------------------------------------------\n");

	// Executive loop sweeping up from Minimum (0.5dB) to Maximum (40.0dB) attenuation
	for (double target_db = min_attenuation; target_db <= max_attenuation;
	     target_db += step_size)
	{
		try
		{
			uint32_t dac_code =
				controller.calculate_dac_code(target_db);

			std::print("  {:5.2f} dB   |   {:5}\n", target_db,
				   dac_code);

			// In your embedded Linux production app, call your SPI/I2C peripheral here:
			// ioctl_write_dac(dac_code);
			// usleep(10000); // Small delay to let the MEMS mirror mechanically adjust
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error calculating for " << target_db
				  << " dB: " << e.what() << "\n";
		}
	}

	return 0;
}
