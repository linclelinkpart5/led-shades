// Glasses-specific functions, may not work with other applications

#define SOLID_PIXEL 255
#define EMPTY_PIXEL 0

// Read and smooth light sensor input
#define BRIGHTNESS_SMOOTH_FACTOR 0.99
float smoothedBrightness = 0;
void readBrightness() {
    smoothedBrightness = smoothedBrightness * BRIGHTNESS_SMOOTH_FACTOR + analogRead(3) * (1.0 - BRIGHTNESS_SMOOTH_FACTOR);
}

// Send contents of bit frame to correct AS1130 registers
// Rough, needs to be streamlined
void writeBitFrame(byte frame, byte bitbuffer) {
    Wire.beginTransmission(I2C_ADDR_AS1130_R);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_ON_OFF_START);
    Wire.endTransmission();

    byte tempBits = 0;

    Wire.beginTransmission(I2C_ADDR_AS1130_R);
    Wire.write(0x00);
    for (byte i = 0; i < 12; i++) {
        tempBits = GlassesBits[i][bitbuffer];
        Wire.write(tempBits << 2);
        Wire.write(tempBits >> 6);
    }
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR_AS1130_L);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_ON_OFF_START);
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR_AS1130_L);
    Wire.write(0x00);
    for (byte i = 0; i < 12; i++) {
        tempBits = GlassesBits[i+12][bitbuffer];
        Wire.write(tempBits << 2);
        Wire.write(tempBits >> 6);
    }
    Wire.endTransmission();
}

// Send contents of PWM frame to correct AS1130 registers
// Rough, needs to be streamlined
void writePWMFrame(byte frame) {
    Wire.beginTransmission(I2C_ADDR_AS1130_R);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_BLINK_PWM_START);
    Wire.endTransmission();

    for (int x = 0; x < 12; x++) {
        Wire.beginTransmission(I2C_ADDR_AS1130_R);
        Wire.write(26+x*11);
        for (int y = 0; y < 8; y++) {
            Wire.write(GlassesPWM[x][y][0]);
        }
        Wire.endTransmission();
    }

    Wire.beginTransmission(I2C_ADDR_AS1130_L);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_BLINK_PWM_START);
    Wire.endTransmission();

    for (int x = 0; x < 12; x++) {
        Wire.beginTransmission(I2C_ADDR_AS1130_L);
        Wire.write(26+x*11);
        for (int y = 0; y < 8; y++) {
            Wire.write(GlassesPWM[x+12][y][0]);
        }
        Wire.endTransmission();
    }
}

// Send contents of bit frame to blink AS1130 registers
// Rough, needs to be streamlined
// Mostly used to clear blink frames, since blink is useless
void writeBlinkFrame(byte frame, byte bitbuffer) {
    Wire.beginTransmission(I2C_ADDR_AS1130_R);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_BLINK_PWM_START);
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR_AS1130_R);
    Wire.write(0x00);
    for (byte i = 0; i < 12; i++) {
        Wire.write(GlassesBits[i][bitbuffer] << 2);
        Wire.write(GlassesBits[i][bitbuffer] >> 6);
    }
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR_AS1130_L);
    Wire.write(REGISTER_SELECT);
    Wire.write(frame + MEMORY_BLINK_PWM_START);
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR_AS1130_L);
    Wire.write(0x00);
    for (byte i = 0; i < 12; i++) {
        Wire.write(GlassesBits[i+12][bitbuffer] << 2);
        Wire.write(GlassesBits[i+12][bitbuffer] >> 6);
    }
    Wire.endTransmission();
}

// Set PWM frame all to one value
// Usually used to clear or fill frame
// Bit and PWM frames interact, if you want to do bit graphics you must fill PWM frame
void fillPWMFrame(byte frame, byte value) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            GlassesPWM[x][y][frame] = value;
        }
    }
}

// Set PWM frame all to one value
// Usually used to clear or fill frame
// Bit and PWM frames interact, if you want to do PWM graphics you must fill bit on/off frame
void fillBitFrame(byte frame, byte value) {
    for (int x = 0; x < 24; x++) {
        GlassesBits[x][0] = 255*(value > 0);
        GlassesBits[x][1] = 255*(value > 0);
    }
}

// Fill blink frame with known value
// Usually zero because no one actually wants to use blink
void fillBlinkFrame(byte frame, byte value) {
    for (int x = 0; x < 24; x++) {
        GlassesBits[x][0] = 255*(value > 0);
        GlassesBits[x][1] = 255*(value > 0);
    }
    writeBlinkFrame(frame, 0);
}

void invertPWMFrame(byte frame) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            GlassesPWM[x][y][frame] = 255 - GlassesPWM[x][y][frame];
        }
    }
}

// Configure AS1130 chips to ideal startup settings
void glassesInit() {
    byte configOptions = 0;

    // reset chip again
    configOptions = (0 << AS1130_test_all) |
                    (0 << AS1130_auto_test) |
                    (0 << AS1130_manual_test) |
                    (0 << AS1130_init) |
                    (0 << AS1130_shdn);

    setShutdownTest(I2C_ADDR_AS1130_R, configOptions);
    setShutdownTest(I2C_ADDR_AS1130_L, configOptions);

    delay(5);

    configOptions = (1 << AS1130_test_all) |
                    (1 << AS1130_auto_test) |
                    (0 << AS1130_manual_test) |
                    (1 << AS1130_init) |
                    (1 << AS1130_shdn);

    setShutdownTest(I2C_ADDR_AS1130_R, configOptions);
    setShutdownTest(I2C_ADDR_AS1130_L, configOptions);

    delay(5);

    configOptions = (0 << AS1130_low_vdd_rst) |
                    (0 << AS1130_low_vdd_stat) |
                    (1 << AS1130_led_error_correction) |
                    (0 << AS1130_dot_corr) |
                    (0 << AS1130_common_addr);

    setConfigs(I2C_ADDR_AS1130_R, configOptions, 1);
    setConfigs(I2C_ADDR_AS1130_L, configOptions, 1);

    setMovie(I2C_ADDR_AS1130_R, 0, 0, 0, 0);
    setMovie(I2C_ADDR_AS1130_L, 0, 0, 0, 0);

    setMovieOptions(I2C_ADDR_AS1130_R, 0, 0, 0, 0);
    setMovieOptions(I2C_ADDR_AS1130_L, 0, 0, 0, 0);

    setMovieLooping(I2C_ADDR_AS1130_R, 1);
    setMovieLooping(I2C_ADDR_AS1130_L, 1);

    setBrightness(I2C_ADDR_AS1130_R, STARTING_BRIGHTNESS);
    setBrightness(I2C_ADDR_AS1130_L, STARTING_BRIGHTNESS);

    configOptions = (0 << AS1130_selected_pic) |
                    (0 << AS1130_watchdog) |
                    (0 << AS1130_por) |
                    (0 << AS1130_overtemp) |
                    (0 << AS1130_low_vdd) |
                    (0 << AS1130_open_err) |
                    (0 << AS1130_short_err) |
                    (0 << AS1130_movie_fin);

    setInterruptMask(I2C_ADDR_AS1130_R, configOptions);
    setInterruptMask(I2C_ADDR_AS1130_L, configOptions);

    setInterruptFrame(I2C_ADDR_AS1130_R, 0);
    setInterruptFrame(I2C_ADDR_AS1130_L, 0);

    setI2CWatchdog(I2C_ADDR_AS1130_R, 64, 1);
    setI2CWatchdog(I2C_ADDR_AS1130_L, 64, 1);

    setClockSync(I2C_ADDR_AS1130_R, AS1130_clock_speed_1MHz, AS1130_sync_IN);
    setClockSync(I2C_ADDR_AS1130_L, AS1130_clock_speed_1MHz, AS1130_sync_OUT);

    fillPWMFrame(0, 255);
    writePWMFrame(0);
    fillBitFrame(0, 0);
    writeBitFrame(0, 0);
    fillBlinkFrame(0, 0);
    writeBlinkFrame(0, 0);

    setFrame(I2C_ADDR_AS1130_R, 0, 1);
    setFrame(I2C_ADDR_AS1130_L, 0, 1);
}

// When switching from on/off to PWM or vice versa
// Need to blank or fill respectively to correct the masks
void switchDrawType(byte frame, byte enablePWM) {
    if (enablePWM == 0) {
        fillBitFrame(frame, 0);
        writeBitFrame(frame, 0);
        fillPWMFrame(frame, 255);
        writePWMFrame(frame);
    }
    else if (enablePWM == 1) {
        fillPWMFrame(frame, 0);
        writePWMFrame(frame);
        fillBitFrame(frame, 1);
        writeBitFrame(frame, 0);
    }
}

// Copy contents of PWM array one LED over
// Possible to do in hardware at some future date
// TODO: Add a "new" value, either a byte to fill, or a row/column buffer.
void hScrollPWM(bool increasing) {
    int i, j;
    for (i = 1; i < NUM_LED_COLS; i++) {
        for (j = 0; j < NUM_LED_ROWS; j++) {
            // Shift index i-1 into i.
            if (increasing) GlassesPWM[NUM_LED_COLS - i][j][0] = GlassesPWM[NUM_LED_COLS - i - 1][j][0];
            // Shift index i+1 into i.
            else GlassesPWM[i - 1][j][0] = GlassesPWM[i][j][0];
        }
    }
}

// // TODO: Add a "new row" value, either a byte to fill, or a row buffer.
// void vScrollPWM(bool increasing) {
//     int i, j;
//     for (i = 1; i < NUM_LED_COLS; i++) {
//         for (j = 0; j < NUM_LED_ROWS; j++) {
//             // Shift index i-1 into i.
//             if (increasing) GlassesPWM[NUM_LED_COLS - i][j][0] = GlassesPWM[NUM_LED_COLS - i - 1][j][0];
//             // Shift index i+1 into i.
//             else GlassesPWM[i - 1][j][0] = GlassesPWM[i][j][0];
//         }
//     }


//     if (increasing) {
//         for (int i = 1; i < NUM_LED_COLS; i++) {
//             for (int j = 0; j < NUM_LED_ROWS; j++) {
//                 GlassesPWM[NUM_LED_COLS - i][j][0] = GlassesPWM[NUM_LED_COLS-i-1][j][0];
//             }
//         }
//     }
//     else if (dir == 1) {
//         for (int i = 1; i < NUM_LED_COLS; i++) {
//             for (int j = 0; j < NUM_LED_ROWS; j++) {
//                 GlassesPWM[i-1][j][0] = GlassesPWM[i][j][0];
//             }
//         }
//     }
// }

// Copy contents of bit array one LED over
// Possible to do in hardware at some future date
void scrollBits(byte dir, byte bitbuffer) {
    if (dir == 0) {
        for (int i = 1; i < NUM_LED_COLS; i++) {
            GlassesBits[NUM_LED_COLS - i][bitbuffer] = GlassesBits[NUM_LED_COLS-i-1][bitbuffer];
        }
    }
    else if (dir == 1) {
        for (int i = 1; i < NUM_LED_COLS; i++) {
            GlassesBits[i-1][bitbuffer] = GlassesBits[i][bitbuffer];
        }
    }
}

// Fetch font character bitmap from flash
byte charBuffer[8] = {0};
void loadCharBuffer(byte character) {
    for (int i = 0; i< 8; i++) {
        charBuffer[i] = pgm_read_byte(Font[character]+i);
    }
}

// Determine flash address of text string
unsigned int currentStringAddress = 0;
void selectFlashString(byte string) {
    currentStringAddress = pgm_read_word(&MessageStringLookupTable[string]);
}

// Fetch a character value from a text string in flash
char loadStringChar(byte string, int character) {
    return (char) pgm_read_byte(currentStringAddress + character);
}

// Debounce button inputs (5ms interval)
#define DEBOUNCE_COUNT 5
#define HOLD_COUNT 200
volatile byte buttoncounts[2];
volatile byte buttonOldState = 0;
volatile byte buttonActivated = 0;
void buttonDebounce() {
    #if G_VERSION == 0
    if (!(PINB & (1 << 2))) {
    #else
    if (!(PIND & (1 << 4))) {
    #endif
        if (buttoncounts[0] < 255) buttoncounts[0]++;
        if (!(buttonOldState & (1 << 0)) && (buttoncounts[0] > DEBOUNCE_COUNT)) {
            buttonActivated |= (1 << 0);
            buttonOldState |= (1 << 0);
        }
    }
    else {
        buttoncounts[0] = 0;
        buttonOldState &= ~(1 << 0);
    }

    #if G_VERSION == 0
    if (!(PINB & (1 << 1))) {
    #else
    if (!(PIND & (1 << 3))) {
    #endif
        if (buttoncounts[1] < 255) buttoncounts[1]++;
        if (!(buttonOldState & (1 << 1)) && (buttoncounts[1] > DEBOUNCE_COUNT)) {
            buttonOldState |= (1 << 1);
            buttonActivated |= (1 << 1);
        }
    }
    else {
        buttoncounts[1] = 0;
        buttonOldState &= ~(1 << 1);
    }
}

// Detect activated buttons after debounce
boolean onButtonPressed(byte buttonSelect) {
    if (buttonActivated & (1 << buttonSelect)) {
        buttonActivated &= ~(1 << buttonSelect);
        return true;
    }
    else {
        return false;
    }
}

// Detect hold state for buttons after debounce
boolean onButtonHeld(byte buttonSelect) {
    if (buttoncounts[buttonSelect] > HOLD_COUNT) {
        return true;
    }
    else {
        return false;
    }
}

// Timer setup for 5ms interrupts
#define PRELOAD5MS 624
void setupTimerInterrupt() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = PRELOAD5MS;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
    TIMSK1 = (1 << OCIE1A);
    sei();
}

// Interrupt service routine
// Only button debounce for now
ISR(TIMER1_COMPA_vect) {
    buttonDebounce();
}

byte getCIE (byte value) {
    return pgm_read_byte(&Cie1931LookupTable[value]);
}


// Cycle through several brightness settings
byte brightness = STARTING_BRIGHTNESS;
void cycleBrightness() {
    brightness = (brightness + 16) % 256;

    byte newBrightness = getCIE(brightness);
    setBrightness(I2C_ADDR_AS1130_R, newBrightness);
    setBrightness(I2C_ADDR_AS1130_L, newBrightness);
}

byte qsine(int angle) {
    int cangle = abs(angle)%359;

    if (cangle < 90) return pgm_read_byte(&QsineLookupTable[cangle]);
    else if (cangle < 180) return 255-pgm_read_byte(&QsineLookupTable[179-cangle]);
    else if (cangle < 270) return 255-pgm_read_byte(&QsineLookupTable[cangle-180]);
    else return pgm_read_byte(&QsineLookupTable[359-cangle]);
}

void smartPlot(int x, int y, byte val) {
    if (x < 0 || x >= NUM_LED_COLS) return;
    if (y < 0 || y >= NUM_LED_ROWS) return;
    GlassesPWM[x][y][0] = val;
}

// brightness := [0.0, 1.0]
void smartPlotf(int x, int y, float brightness) {
    smartPlot(x, y, (byte)round(SOLID_PIXEL * brightness));
}

// integer part of x
int ipart(float x) {
    return (int)x;
}

// fractional part of x
float fpart(float x) {
    if (x < 0) return 1 - (x - floor(x));
    return x - floor(x);
}

float rfpart(float x) {
    return 1 - fpart(x);
}

void wuLine(float x0, float y0, float x1, float y1) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);

    float t;
    if (steep) {
        t = x0;
        x0 = y0;
        y0 = t;
        t = x1;
        x1 = y1;
        y1 = t;
        // swap(x0, y0);
        // swap(x1, y1);
    }

    if (x0 > x1) {
        t = x0;
        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
        // swap(x0, x1);
        // swap(y0, y1);
    }
    
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dy / dx;
    
    float xend;
    float yend;
    float xgap;
    int xpxl1, ypxl1;
    int xpxl2, ypxl2;

    // handle first endpoint
    xend = round(x0);
    yend = y0 + gradient * (xend - x0);
    xgap = rfpart(x0 + 0.5);
    xpxl1 = (int)xend; // this will be used in the main loop
    ypxl1 = (int)ipart(yend);
    
    if (steep) {
        smartPlotf(ypxl1,   xpxl1, rfpart(yend) * xgap);
        smartPlotf(ypxl1+1, xpxl1,  fpart(yend) * xgap);
    }
    else {
        smartPlotf(xpxl1, ypxl1  , rfpart(yend) * xgap);
        smartPlotf(xpxl1, ypxl1+1,  fpart(yend) * xgap);
    }

    float intery = yend + gradient; // first y-intersection for the main loop
    
    // handle second endpoint
    xend = round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5);
    xpxl2 = xend; //this will be used in the main loop
    ypxl2 = ipart(yend);
    if (steep) {
        smartPlotf(ypxl2  , xpxl2, rfpart(yend) * xgap);
        smartPlotf(ypxl2+1, xpxl2,  fpart(yend) * xgap);
    }
    else {
        smartPlotf(xpxl2, ypxl2,  rfpart(yend) * xgap);
        smartPlotf(xpxl2, ypxl2+1, fpart(yend) * xgap);
    }
    
    // main loop
    for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++) {
        if (steep) {
            smartPlotf(ipart(intery)  , x, rfpart(intery));
            smartPlotf(ipart(intery)+1, x,  fpart(intery));
        }
        else {
            smartPlotf(x, ipart(intery),  rfpart(intery));
            smartPlotf(x, ipart(intery)+1, fpart(intery));
        }
        intery = intery + gradient;
    }
}

// Anti-aliased line algorithm
// Adapted from Michael Abrash http://www.phatcode.net/res/224/files/html/ch42/42-02.html
void wuLineOld(int X0, int Y0, int X1, int Y1) {
    uint16_t IntensityShift, ErrorAdj, ErrorAcc;
    uint16_t ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;

    // make sure line runs from top to bottom
    if (Y0 > Y1) {
        Temp = Y0; Y0 = Y1; Y1 = Temp;
        Temp = X0; X0 = X1; X1 = Temp;
    }

    // first pixel
    GlassesPWM[X0][Y0][0] = 255;

    if ((DeltaX = X1 - X0) >= 0) {
        XDir = 1;
    }
    else {
        XDir = -1;
        DeltaX = -DeltaX;
    }

    if ((DeltaY = Y1 - Y0) == 0) {
        // horizontal line
        while (DeltaX-- != 0) {
        X0 += XDir;
        GlassesPWM[X0][Y0][0] = 255;
        }
        return;
    }

    if (DeltaX == 0) {
        // vertical line
        do {
            Y0++;
            GlassesPWM[X0][Y0][0] = 255;
        }
        while (--DeltaY != 0);
        return;
    }

    if (DeltaX == DeltaY) {
        // diagonal line
        do {
        X0 += XDir;
        Y0++;
        GlassesPWM[X0][Y0][0] = 255;
        }
        while (--DeltaY != 0);
        return;
    }

    // need an anti-aliased line
    ErrorAcc = 0;
    IntensityShift = 16 - 8;
    WeightingComplementMask = 256 - 1;

    if (DeltaY > DeltaX) {
        // y-major line
        ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY;
        while (--DeltaY) {
            ErrorAccTemp = ErrorAcc;
            ErrorAcc += ErrorAdj;
            if (ErrorAcc <= ErrorAccTemp) {
                X0 += XDir;
            }
            Y0++;
            Weighting = ErrorAcc >> IntensityShift;
            GlassesPWM[X0][Y0][0] = getCIE(255 - Weighting);
            GlassesPWM[X0][Y0 + 1][0] = getCIE(255 - (Weighting ^ WeightingComplementMask));
        }
        GlassesPWM[X1][Y1][0] = 255;
        return;
    }

    ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX;
    while (--DeltaX) {
        ErrorAccTemp = ErrorAcc;
        ErrorAcc += ErrorAdj;
        if (ErrorAcc <= ErrorAccTemp) {
            Y0++;
        }
        X0 += XDir;
        Weighting = ErrorAcc >> IntensityShift;
        GlassesPWM[X0][Y0][0] = getCIE(255 - Weighting);
        GlassesPWM[X0][Y0 + 1][0] = getCIE(255 - (Weighting ^ WeightingComplementMask));
    }
    GlassesPWM[X1][Y1][0] = 255;
}
