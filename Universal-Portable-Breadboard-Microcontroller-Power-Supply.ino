// Universal Portable Breadboard Microcontroller Power Supply
// Code for Battery Voltage Monitoring (Serial Monitor Output ONLY)
// From Circuity of Curiosity - YouTube Channel

// --- Pin Definitions ---
// For ESP32 WROOM: An Analog-to-Digital Converter (ADC) capable pin for battery voltage sensing.
// GPIO 35 is generally a good choice on many ESP32 dev boards for ADC input.
// Ensure this pin is NOT used by other components in your project.
const int BATTERY_SENSE_PIN = 35; 

// --- Battery Measurement Configuration (for voltage divider) ---
// These values are for reading an 18650 battery (max 4.2V) safely with a 3.3V ESP32 WROOM ADC.
// You need to connect: Battery+ -- R1 -- (ADC_PIN) -- R2 -- GND
// R1 and R2 values should be precise for accurate readings.
const float R1_VALUE = 100000.0; // Resistance of the first resistor (e.g., 100k Ohm)
const float R2_VALUE = 47000.0;  // Resistance of the second resistor (e.g., 47k Ohm)

// ESP32's ADC has 12-bit resolution, so its maximum reading is 4095 (0 to 4095).
const float ADC_MAX_READING = 4095.0; 
// ESP32's internal ADC reference voltage is typically 3.3V.
const float ADC_REFERENCE_VOLTAGE = 3.3; 

// Define the full and empty voltages for your 18650 Li-ion battery.
// These define the 0% and 100% points for your battery percentage calculation.
const float BATTERY_FULL_VOLTAGE = 4.2; // A fully charged 18650 battery
const float BATTERY_EMPTY_VOLTAGE = 3.0; // Voltage at which to consider the battery "empty"

// --- Battery Read Interval ---
// How often (in milliseconds) we will read the battery voltage and update the display.
const unsigned long BATTERY_READ_INTERVAL_MS = 5000; // Read battery every 5 seconds

// --- Internal State Variables ---
unsigned long lastBatteryReadTime = 0; // Variable to track when the battery was last read


void setup() {
  // --- Serial Communication Setup ---
  // Start serial communication for debugging messages and displaying battery information.
  Serial.begin(115200); 
  Serial.println("Starting Portable Power Setup...");
  Serial.println("Setting up Battery Voltage Monitoring...");

  // --- Battery Sense Pin Configuration ---
  // Set the ADC pin as an input to read analog voltage from the voltage divider.
  pinMode(BATTERY_SENSE_PIN, INPUT);
  // Optional: Set ADC attenuation for specific voltage ranges if needed.
  // For voltage dividers scaling 4.2V down to ~1.4V-2.7V, ADC_11db is usually appropriate for full range.
  // If your readings seem off, you might research ESP32 ADC attenuation.
  // analogSetAttenuation(ADC_11db); // Uncomment if you need to set attenuation explicitly.
  Serial.println("---------------------------------------------");
  Serial.println("Remember to disconnect USB for portable power testing!");
  Serial.println("---------------------------------------------");
}


void loop() {
  // --- Battery Level Reporting Logic ---
  // Check if enough time has passed since the last battery reading.
  if (millis() - lastBatteryReadTime > BATTERY_READ_INTERVAL_MS) {
    lastBatteryReadTime = millis(); // Reset the timer for the next reading

    // --- Read Raw Analog Value ---
    int rawAdc = analogRead(BATTERY_SENSE_PIN); // Read the raw analog value from the voltage divider

    // --- Convert Raw ADC to Voltage at ADC Pin ---
    // Formula: (raw_value / max_adc_reading) * adc_reference_voltage
    float voltageAtAdcPin = (float)rawAdc / ADC_MAX_READING * ADC_REFERENCE_VOLTAGE;

    // --- Convert Voltage at ADC Pin to Actual Battery Voltage ---
    // This reverses the voltage division effect of R1 and R2 to get the actual battery voltage.
    // Formula: voltage_at_adc_pin * ((R1 + R2) / R2)
    float actualBatteryVoltage = voltageAtAdcPin * ((R1_VALUE + R2_VALUE) / R2_VALUE);

    // --- Print Data to Serial Monitor (for debugging) ---
    Serial.print("Raw ADC: "); Serial.print(rawAdc);
    Serial.print(", V_ADC: "); Serial.print(voltageAtAdcPin, 2); // Voltage read at the board's pin
    Serial.print("V, V_Batt: "); Serial.print(actualBatteryVoltage, 2); // Actual battery voltage

    // --- Calculate Battery Percentage ---
    // Map the actual battery voltage (e.g., 3.0V to 4.2V) to a percentage (0-100%).
    uint8_t batteryPercentage = map(actualBatteryVoltage * 100, // Multiply by 100 to work with integers
                                   BATTERY_EMPTY_VOLTAGE * 100,
                                   BATTERY_FULL_VOLTAGE * 100,
                                   0, 100);

    // Ensure the percentage stays within the valid 0-100 range in case of calibration errors or extreme readings.
    if (batteryPercentage < 0) batteryPercentage = 0;
    if (batteryPercentage > 100) batteryPercentage = 100;

    Serial.print(" -> Battery: "); Serial.print(batteryPercentage); Serial.println("%");
  }

  // --- Small Delay for Loop Efficiency ---
  // A small delay to prevent the loop from running too fast, which saves CPU cycles and power.
  delay(10); 
}