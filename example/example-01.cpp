#include "voa.hpp"

#include <iostream>
#include <print>

int main()
{
	// Hardware configuration matched to your 16-bit DAC circuit setup
	hardware_config hw_agiltron{
		.dac_max_code = 65535, // 16-bit DAC
		.hardware_vmax = 7.0, // Max safe voltage for VOAM-0B3111333
		.v_ref = 5.0, // 5V Reference
		.hardware_gain =
			1.5 // Op-amp scales 5V max up to 7.5V (covers the 7V VOA limit comfortably)
	};

	// Polynomial segments mirroring the Agiltron curve behavior
	std::vector<calibration_segment> cal_agiltron = {
		{ 0.5,
		  3.0,
		  { 0.0055, -0.121, 1.34,
		    -0.85 } }, // Low attenuation zone (Higher voltage)
		{ 3.0,
		  15.0,
		  { 0.0012, -0.052, 0.88, -2.1 } }, // Mid attenuation zone
		{ 15.0,
		  40.0,
		  { -0.0004, 0.031, -0.92,
		    8.5 } } // High attenuation zone (Lower voltage)
	};

	voa_controller controller(hw_agiltron, cal_agiltron);

	// Pull the absolute calibration bounds to feed our sweep dynamically
	double min_attenuation = controller.get_absolute_min_db(); // 0.5 dB
	double max_attenuation = controller.get_absolute_max_db(); // 40.0 dB
	double step_size = 0.5; // Sweep step size in dB

	std::print("Starting Sweep from Minimum to Maximum Attenuation...\n");
	std::print("--------------------------------------------------------\n");
	std::print("Target (dB) | Computed DAC Code (Raw Integer)\n");
	std::print("--------------------------------------------------------\n");

	// Executive loop sweeping up from Minimum (0.5dB) to Maximum (40.0dB) attenuation
	for (double target_db = min_attenuation; target_db <= max_attenuation;
	     target_db += step_size) {
		try {
			uint32_t dac_code =
				controller.calculate_dac_code(target_db);

			std::print("  {:5.2f} dB   |   {:5}\n", target_db, dac_code);

			// In your embedded Linux production app, call your SPI/I2C peripheral here:
			// ioctl_write_dac(dac_code);
			// usleep(10000); // Small delay to let the MEMS mirror mechanically adjust

		} catch (const std::exception &e) {
			std::cerr << "Error calculating for " << target_db
				  << " dB: " << e.what() << "\n";
		}
	}

	return 0;
}
