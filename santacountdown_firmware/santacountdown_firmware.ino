/*

   Santa Couuntdown Display

   A bit of Christmas fun for the Element 14 Holidays Competition

   Alistair MacDonald 2024

*/

#include <ESP8266WiFi.h>
#include <time.h>

#include <LEDMatrixDriver.hpp>
#include "font6x8.h"

// Settings

#define MATRIXWIDTH 4
#define LEDMATRIX_CS_PIN D8
// Note the DIN goes to D7 and CLK to D5 (max7219 to Lolin/Wemos D1 mini)

#define MY_WIFISSID "************"
#define MY_WIFIPSK  "************"

#define MY_NTP_SERVER "pool.ntp.org"
#define MY_TZ "GMT0BST,M3.5.0/1,M10.5.0"
// More time zone information can be found in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv


LEDMatrixDriver lmd(MATRIXWIDTH, LEDMATRIX_CS_PIN);

// A function to update the display
// inString it the test, and inOffset is the horizontal offset
void drawAll( String inString, int inOffset = 0 ) {
  int pos, offsetcol, val;

  // Loop though the colums of the display
  for ( int col = 0; col < (MATRIXWIDTH * 8); col++ ) {

    // Work out the charicter offset that we are sending
    offsetcol = col + inOffset;
    pos = (( inString[(offsetcol / 6) % inString.length()] - 32 ) * 6) + (offsetcol % 6);

    // Loop though the rows of the display
    for ( int row = 0; row < 8; row++ ) {
      // Work out the pixel state (checking for a space)
      val = ssd1306xled_font6x8[pos] & (0x01 << row);
      // Send the new pixel state to the deisplay driver
      lmd.setPixel( col, row, val );
    }

  }
}


// Calculate and return the number of "sleeps" (aka midnights) until Christmas morning
int sleepsToChristmas() {

  time_t now;
  tm tm;

  // Get the currnet time
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time

  // Work out the day of the year Christmas falls on (we need to add one on a leap year)
  int dayofyearChristmas = 359;
  // This conculates if it is a leap year
  if ( ( ( tm.tm_year % 4 ) == 0 ) &&
       ( ( ( tm.tm_year % 100 ) != 0 ) || ( ( tm.tm_year % 400 ) == 0 ) )
     ) {
    dayofyearChristmas ++;
  }

  // Return the number of sleeps to go
  return dayofyearChristmas - tm.tm_yday;

}

// The setup routine
void setup() {

  // Init the serial port for debugging
  Serial.begin(115200);

  // Init the clock / NTP client
  configTime(MY_TZ, MY_NTP_SERVER);

  // Init the display
  lmd.setEnabled(true);
  lmd.setIntensity(0);   // 0 = low, 10 = high
  lmd.display();

  // Connect to the WiFi so we can get the time
  Serial.print("Connecting to wifi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_WIFISSID, MY_WIFIPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}


// The main loop
void loop() {

  // Work out what to display
  String lineBuffer;
  int sleeps = sleepsToChristmas();
  // If it is on or after Christmas day use a default message
  if (sleeps<=0) {
    lineBuffer = "Happy Christmas!   ";
  }
  // If it is before Christmas day create the countdown message
  else {
    lineBuffer = String(sleeps) + " sleeps until Santa   ";
  }

  // Calculate the horisontal offset based on the microcontrolelr run time
  unsigned long now = millis();
  unsigned long offset = ( now / 128 ) % ( lineBuffer.length() * 6 );

  // Update the display
  drawAll( lineBuffer, offset );
  lmd.display();

  // Save a little power by delaying as few milliseconds
  delay(40);

}
