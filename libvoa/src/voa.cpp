#include "voa.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

double
voa_controller::evaluate_polynomial(double x,
				    const std::vector<double> &coeffs) const
{
	double result = 0.0;
	for (double coefficient : coeffs)
	{
		result = result * x + coefficient;
	}
	return result;
}

voa_controller::voa_controller(
	const hardware_config &hw,
	const std::vector<calibration_segment> &cal_segments)
	: hw_config(hw), segments(cal_segments)
{
}

uint32_t voa_controller::calculate_dac_code(double target_db) const
{
	auto it = std::find_if(segments.begin(), segments.end(),
			       [target_db](const calibration_segment &seg) {
				       return target_db >= seg.min_db &&
					      target_db <= seg.max_db;
			       });

	if (it == segments.end())
	{
		throw std::out_of_range(
			"Requested attenuation is outside calibrated limits.");
	}

	double target_voltage =
		evaluate_polynomial(target_db, it->coefficients);

	target_voltage = std::max(0.0, std::min(target_voltage,
						hw_config.hardware_vmax));

	double max_achievable_voltage =
		hw_config.v_ref * hw_config.hardware_gain;
	if (max_achievable_voltage <= 0.0)
		return 0;

	double normalized_fraction = target_voltage / max_achievable_voltage;
	normalized_fraction = std::max(0.0, std::min(normalized_fraction, 1.0));

	return static_cast<uint32_t>(
		std::round(normalized_fraction * hw_config.dac_max_code));
}

double voa_controller::get_absolute_min_db() const
{
	auto min_elem = std::min_element(segments.begin(), segments.end(),
					 [](const calibration_segment &a,
					    const calibration_segment &b) {
						 return a.min_db < b.min_db;
					 });
	return (min_elem != segments.end()) ? min_elem->min_db : 0.0;
}

double voa_controller::get_absolute_max_db() const
{
	auto max_elem = std::max_element(segments.begin(), segments.end(),
					 [](const calibration_segment &a,
					    const calibration_segment &b) {
						 return a.max_db < b.max_db;
					 });
	return (max_elem != segments.end()) ? max_elem->max_db : 0.0;
}