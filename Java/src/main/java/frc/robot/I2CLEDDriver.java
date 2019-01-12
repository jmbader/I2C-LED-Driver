package frc.robot;

import edu.wpi.first.wpilibj.I2C;

/**
 * i2c LED driver developed for FRC robots
 *
 * @author Ryan Pappa and Jacob Bader
 */
public class I2CLEDDriver {
    private final boolean DEBUG = true;
    private int numLEDs;
    private Mode currentMode;
    private I2C i2c;

    /**
     * Constructs an LED driver, defaults to the onboard port and address 8
     */
    public I2CLEDDriver() {
        // default to onboard port and address 8
        this.i2c = new I2C(I2C.Port.kOnboard, 8);
    }

    /**
     * @param port    the i2c port the driver is attached to
     * @param address the i2c address (usually 8) that the driver is at
     */
    public I2CLEDDriver(I2C.Port port, int address) {
        this.i2c = new I2C(port, address);
    }

    /**
     * Helper method that tells whether an int could be a byte
     * (in range [0, 255])
     * */
    private boolean isNotByte(int potentiallyByte) {
        return potentiallyByte < 0x00 || potentiallyByte > 0xFF;
    }

    /**
     * Writes a register by sending first the register, then the value over i2c
     * @throws IllegalArgumentException if the data is not a byte
     */
    private void writeRegister(Register register, int data)
            throws IllegalArgumentException{
        writeRegister(register.address, data);
    }

    /**
     * Writes a register by sending first the register, then the value over i2c
     * @throws IllegalArgumentException if the data or register is not a byte
     */
    private void writeRegister(int register, int data)
            throws IllegalArgumentException {
        if (isNotByte(register) || isNotByte(data)) throw new
                IllegalArgumentException("Registers and their data must be in "
                + "the range [0x00, 0xFF]");

        boolean aborted = i2c.writeBulk(new byte[]{(byte) register,
                (byte) data});

        if (DEBUG && aborted) System.out.println("i2c write aborted");
    }

    /**
     * Different display modes
     */
    public enum Mode {
        OFF(0x00), SOLID_COLOR(0x01), SWEEP_SOLID(0x02), SWEEP_RAINBOW(0x03),
        RAINBOW(0x04), CANDYCANE(0x05);

        private int value;
        Mode(int value) {
            this.value = value;
        }
    }

    /**
     * Register addresses
     */
    private enum Register {
        STATUS(0x00), LED_COUNT_0(0x01), LED_COUNT_1(0x08), MODE(0x02),
        BRIGHTNESS(0x03), DATA_0(0x04), DATA_1(0x05), DATA_2(0x06),
        DATA_3(0x07), COLOR_MODE(0x09);

        private int address;
        Register(int address) {
            this.address = address;
        }
    }

    /**
     * Sets the number of LEDs in the strip. This should be done before anything
     * else
     * @param numLEDs the number of LEDs
     */
    private void setNumLEDs(int numLEDs) {
        this.numLEDs = numLEDs;
        writeRegister(Register.LED_COUNT_0, numLEDs % 256); // 256^0 place
        writeRegister(Register.LED_COUNT_1, numLEDs / 256); // 256^1 place
    }

    /**
     * Set the brightness as an integer. Note this
     * <a href="https://learn.adafruit.com/led-tricks-gamma-correction/the-issue">may not</a>
     * correlate directly with the LEDs' brightness.
     * @param brightness an integer from 0 to 255 <br>
     *                   Values outside this range are adjusted to the endpoints
     */
    public void setBrightness(int brightness) {
        if(DEBUG && isNotByte(brightness)) System.out.println("Brightness set " +
                "to an invalid value, but was adjusted");
        if(brightness > 0xFF) brightness = 0xFF;
        if(brightness < 0x00) brightness = 0x00;
        // possibly add gamma correction ?
        writeRegister(Register.BRIGHTNESS, brightness);
    }

    /**
     * Set the brightness as a double. Note this
     * <a href="https://learn.adafruit.com/led-tricks-gamma-correction/the-issue">may not</a>
     * correlate directly with the LEDs' brightness.
     * @param brightness a double from 0.0 to 1.0 <br>
     *                   Values outside this range are adjusted to the endpoints
     */
    public void setBrightness(double brightness) {
        setBrightness(0xFF * brightness);
    }

    /**
     * sets the display mode
     * @param mode a mode from the enum {@link Mode}
     */
    public void setMode(Mode mode) {
        writeRegister(Register.MODE, mode.value);
        currentMode = mode;
    }

    /**
     * Sets the color of the strip, used in the "solid color" and "solid sweep"
     * modes
     * @param rgb a hex color given as an int, ex 0xff0000 for red or
     *            0x009999 for a teal color
     * @throws IllegalArgumentException if the given color is not a valid color
     */
    public void setRGB(int rgb) throws IllegalArgumentException {
        if(DEBUG && !(currentMode.value == 0x01 || currentMode.value == 0x02)) {
            System.out.println("Color set in a mode that doesn't take a color");
            return;
        }
        if (rgb < 0x000000 || rgb > 0xFFFFFF)
            throw new IllegalArgumentException("Colors must be between " +
                    "0x000000 and 0xFFFFFF (hex colors)");
        int r = rgb >> 16;
        int g = (rgb & 0x00FF00) >> 8;
        int b = rgb & 0x0000FF;

        writeRegister(Register.COLOR_MODE, 0x00);
        writeRegister(Register.DATA_0, r);
        writeRegister(Register.DATA_1, g);
        writeRegister(Register.DATA_2, b);
    }

    /**
     * Sets the color for applicable modes in HSV format. This enables easy
     * transitions through the rainbow, among other things
     * @param hue [0, 255]
     * @param saturation [0, 255]
     * @param value [0, 255]
     * @throws IllegalArgumentException if a value is outside the expected range
     */
    public void setHSV(int hue, int saturation, int value)
            throws IllegalArgumentException{
        if(!(currentMode.value == 0x01 || currentMode.value == 0x02)) {
            if(DEBUG) System.out.println("Color set in a mode that doesn't " +
                    "take a color");
            return;
        }
        if(isNotByte(hue) || isNotByte(saturation) || isNotByte(value)) throw new
                IllegalArgumentException("Hue, Saturation, and Value must all" +
                "be between 0 and 255");

        writeRegister(Register.COLOR_MODE, 0x01);
        writeRegister(Register.DATA_0, hue);
        writeRegister(Register.DATA_1, saturation);
        writeRegister(Register.DATA_2, value);
    }

    /**
     * The "speed" for applicable modes is the amount of delay in the animation
     * loop, so a higher value for speed is actually a slower animation.
     * Experiment to find a desireable value
     * @throws IllegalArgumentException if speed is outside [0, 255]
     */
    public void setSpeed(int speed) throws IllegalArgumentException {
        if(currentMode.value < 0x02) {
            if(DEBUG) System.out.println("Speed set in a mode that doesn't " +
                    "take a speed");
            return;
        }

        writeRegister(Register.DATA_3, speed);
    }

    /**
     * Sets the animation width. Currently only for candy cane mode.
     * @throws IllegalArgumentException if outside [0, 255]
     */
    public void setWidth(int width) throws IllegalArgumentException {
        if(currentMode.value != 0x05) {
            if(DEBUG) System.out.println("Width set in a mode that doesn't " +
                    "take a width");
            return;
        }

        writeRegister(Register.DATA_0, width);
    }

    /**
     * Sets the whole strip to the given color.
     * A helper method combining setting the mode and setting the color
     * @param rgbColor an rgb color in the hex format, ex 0xFF0AB1
     */
    public void displayColor(int rgbColor) {
        setRGB(rgbColor);
        setMode(Mode.SOLID_COLOR);
    }

    /**
     * Sets the whole strip to the given color.
     * A helper method combining setting the mode and setting the color
     * @param hue
     * @param sat
     * @param val
     */
    public void displayColor(int hue, int sat, int val) {
        setHSV(hue, sat, val);
        setMode(Mode.SOLID_COLOR);
    }

    /**
     * Turns off the whole strip
     */
    public void turnOff() {
        setMode(Mode.OFF);
    }
}
