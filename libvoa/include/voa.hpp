#ifndef VOA_HPP
#define VOA_HPP

#include <cstdint>
#include <vector>

struct calibration_segment
{
	double min_db;
	double max_db;
	std::vector<double>
		coefficients; // Sorted from highest degree to lowest: c3, c2, c1, c0
};

struct hardware_config
{
	uint32_t dac_max_code; // e.g., 65535 for 16-bit
	double hardware_vmax; // 7.0V for VOAM-0B3111333
	double v_ref; // DAC Reference voltage
	double hardware_gain; // Op-amp gain multiplier
};

class voa_controller
{
    private:
	hardware_config hw_config;
	std::vector<calibration_segment> segments;

	double evaluate_polynomial(double x,
				  const std::vector<double> &coeffs) const;

    public:
	voa_controller(const hardware_config &hw,
		      const std::vector<calibration_segment> &cal_segments);

	uint32_t calculate_dac_code(double target_db) const;

	// Helper methods to find absolute bounds dynamically from the JSON/Config structure
	double get_absolute_min_db() const;

	double get_absolute_max_db() const;
};

#endif // VOA_HPP
