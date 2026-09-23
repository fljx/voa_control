# VOA Controller

This project contains a small C++ controller for a MEMS fiber-optic variable
attenuator (VOA), together with an example that converts requested optical
attenuation into raw DAC codes.

## How the VOA works

The Agiltron VOAM device uses an electrostatic MEMS rotating mirror. Applying a
control voltage changes the mirror position and therefore the amount of optical
power coupled through the device:

- Lower voltage produces lower attenuation.
- Higher voltage produces higher attenuation.
- The SM/PM version is specified for approximately 0 to 6 V drive.
- The electrical load is capacitive and has no polarity.
- The typical device provides up to approximately 40 dB attenuation.

The datasheet's attenuation-versus-voltage curve is nonlinear. Consequently,
a constant voltage step does not produce a constant attenuation step.

The datasheet also specifies a typical response time of about 0.5 ms and a
maximum repetition rate of 100 Hz. A real driver should allow the MEMS mirror
time to settle after changing the voltage.

## Software model

The public interface is defined in `libvoa/include/voa.hpp` and implemented in
`libvoa/src/voa.cpp`.

A `hardware_config` describes the DAC and analog output stage:

| Field | Meaning |
| --- | --- |
| `dac_max_code` | Largest raw code accepted by the DAC, 65535 for a 16-bit DAC |
| `hardware_vmax` | Maximum voltage allowed at the VOA |
| `v_ref` | DAC reference voltage |
| `hardware_gain` | Analog gain between the DAC output and the VOA |

A `calibration_segment` describes a range of target attenuation and a
polynomial that returns the required control voltage. Coefficients are stored
in descending polynomial order and evaluated with Horner's method. For the
current example the polynomials have two coefficients, so each is linear:

```text
voltage = slope * attenuation + offset
```

The example calibration is a piecewise-linear approximation to the inverse of
the datasheet curve. It uses these approximate voltage/attenuation points:

| Control voltage | Typical attenuation |
| ---: | ---: |
| 1 V | 0.4 dB |
| 2 V | 1.5 dB |
| 3 V | 6 dB |
| 4 V | 18 dB |
| 5 V | 35 dB |
| 6 V | 50 dB |

The example limits its requested sweep to 0.5-40 dB, so it remains within the
intended operating range. The values above are read approximately from the
datasheet graph. They are a starting point, not a replacement for calibration
against the actual VOA and optical measurement setup.

## DAC-code algorithm

`voa_controller::calculate_dac_code(dB{target_db})` performs the
following steps:

1. Find the calibration segment whose attenuation range contains `target_db`.
2. Evaluate that segment to calculate the required VOA voltage.
3. Clamp the voltage to `[0, hardware_vmax]`.
4. Calculate the maximum analog output as:

   ```text
   maximum_output = v_ref * hardware_gain
   ```

5. Normalize the requested voltage to the DAC range:

   ```text
   fraction = voltage / maximum_output
   dac_code = round(fraction * dac_max_code)
   ```

6. Clamp the normalized fraction to `[0, 1]` before converting it to the raw
   integer code.

If the target attenuation is outside all calibration segments,
the controller throws `std::out_of_range`.

The controller also accepts `percent`. Percent means the
percentage of optical power removed, not the percentage of the calibrated dB
range. It is converted to dB using:

```text
transmission = 1 - percent / 100
attenuation_db = -10 * log10(transmission)
```

For example, 90 percent attenuation corresponds to 10 dB. Values from 0 up
to, but not including, 100 percent are accepted. A value of 100 percent would
require infinite attenuation and is rejected. The resulting dB value must
still fall within the configured calibration segments.

For the example configuration:

```text
DAC range:       0..65535
DAC reference:   5 V
Analog gain:     1.2
VOA range:       0..6 V
```

Thus a 6 V control voltage corresponds to the maximum DAC code, and the
example's 40 dB target maps to a voltage below 6 V according to the approximate
inverse calibration.

## Example

The executable in `example/example-01.cpp` prints a sweep from the minimum to
the maximum attenuation represented by the calibration table:

```text
Target (dB) | Computed DAC Code (Raw Integer)
```

Configure and build from the project root:

```bash
cmake -S . -B build
cmake --build build --target example-01
```

Run the example:

```bash
./build/example-01.exe
```

The plotting helper can run the executable and plot the resulting sweep:

```bash
python ./example/plot_example.py --output ./build/voa-sweep.png
```

It can also plot previously captured output:

```bash
python ./example/plot_example.py --input ./build/example-01-output.txt --output ./build/voa-sweep.png
```

Run and plot the equivalent percent sweep:

```bash
./build/example-01.exe --percent
python ./example/plot_example.py --percent --output ./build/voa-percent-sweep.png
```

## Calibration and safety

The plotted curve should be monotonic, but its accuracy depends on the VOA,
optical wavelength, fiber type, connectors, analog circuit, DAC reference, and
measurement conditions. For a production system:

1. Apply known control voltages within the datasheet limits.
2. Measure the resulting attenuation with the intended optical setup.
3. Fit a monotonic inverse calibration from attenuation to voltage.
4. Replace the approximate example segments with the measured calibration.
5. Verify the voltage, optical power, temperature, and settling-time limits.

Do not drive the device beyond its specified voltage or optical-power limits.
The datasheet notes that connector alignment, cleanliness, wavelength, and
fiber core size can materially affect insertion loss and power handling.
