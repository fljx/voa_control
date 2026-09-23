#ifndef VOA_HPP
#define VOA_HPP

#include <cstdint>
#include <vector>

class dB
{
    public:
	explicit constexpr dB(double value) : value_(value) {}

	constexpr double value() const { return value_; }

    private:
	double value_;
};

class percent
{
    public:
	explicit constexpr percent(double value) : value_(value) {}

	constexpr double value() const { return value_; }

    private:
	double value_;
};

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

	uint32_t calculate_dac_code_from_db(double target_db) const;
	double evaluate_polynomial(double x,
				   const std::vector<double> &coeffs) const;

    public:
	voa_controller(const hardware_config &hw,
		       const std::vector<calibration_segment> &cal_segments);

	uint32_t calculate_dac_code(dB target) const;
	uint32_t calculate_dac_code(percent target) const;

	// Helper methods to find absolute bounds dynamically from the JSON/Config structure
	double absolute_min_db() const;

	double absolute_max_db() const;
};

#endif // VOA_HPP
