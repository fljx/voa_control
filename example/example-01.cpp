#include "voa.hpp"

#include <cmath>
#include <iostream>
#include <print>
#include <string_view>

int main(int argc, char **argv)
{
	bool percent_sweep =
		argc > 1 && std::string_view(argv[1]) == "--percent";
	if (argc > 1 && !percent_sweep)
	{
		std::cerr << "Usage: example-01 [--percent]\n";
		return 2;
	}

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
	double min_attenuation = controller.absolute_min_db(); // 0.5 dB
	double max_attenuation = controller.absolute_max_db(); // 40.0 dB
	double step_size = 0.5; // Sweep step size in dB

	std::print("Starting Sweep from Minimum to Maximum Attenuation...\n");
	std::print(
		"--------------------------------------------------------\n");
	std::print("Target ({}) | Computed DAC Code (Raw Integer)\n",
			percent_sweep ? "%" : "dB");
	std::print(
		"--------------------------------------------------------\n");

	// Executive loop sweeping up from Minimum (0.5dB) to Maximum (40.0dB) attenuation
	for (double target_db = min_attenuation; target_db <= max_attenuation;
	     target_db += step_size)
	{
		double target_percent =
			100.0 * (1.0 - std::pow(10.0, -target_db / 10.0));
		try
		{
			uint32_t dac_code = percent_sweep
				? controller.calculate_dac_code(percent{target_percent})
				: controller.calculate_dac_code(dB{target_db});

			std::print("  {:8.2f} {}   |   {:5}\n",
				   percent_sweep ? target_percent : target_db,
				   percent_sweep ? "%" : "dB", dac_code);

			// In your embedded Linux production app, call your SPI/I2C peripheral here:
			// ioctl_write_dac(dac_code);
			// usleep(10000); // Small delay to let the MEMS mirror mechanically adjust
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error calculating for "
				  << (percent_sweep ? target_percent : target_db)
				  << (percent_sweep ? " %: " : " dB: ") << e.what()
				  << "\n";
		}
	}

	return 0;
}
