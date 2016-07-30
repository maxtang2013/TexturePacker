//**********  MyPngWriter.h   **********************************************
//  Author:                    Paul Blackburn
//
//  Email:                     individual61@users.sourceforge.net
//
//  Version:                   0.5.4   (19 / II / 2009)
//
//  Description:               Library that allows plotting a 48 bit
//                             PNG image pixel by pixel, which can
//                             then be opened with a graphics program.
//
//  License:                   GNU General Public License
//                             Copyright 2002, 2003, 2004, 2005, 2006, 2007, 
//                             2008, 2009 Paul Blackburn
//
//  Website: Main:             http://MyPngWriter.sourceforge.net/
//           Sourceforge.net:  http://sourceforge.net/projects/MyPngWriter/
//           Freshmeat.net:    http://freshmeat.net/projects/MyPngWriter/
//
//  Documentation:             The header file (MyPngWriter.h) is commented, but for a
//                             quick reference document, and support,
//                             take a look at the website.
//
//*************************************************************************

/*
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * */
#ifndef _MYPNGWRITER_H_
#define _MYPNGWRITER_H_

#include <png.h>

#include <iostream>
#include <cmath>
#include <cwchar>
#include <string>

//png.h must be included before FreeType headers.
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>


#define PNG_BYTES_TO_CHECK (4)
#define PNGWRITER_DEFAULT_COMPRESSION (6)

class MyPngWriter 
{
 private:
   
   char * filename_;   
   char * textauthor_;   
   char * textdescription_;   
   char * texttitle_;   
   char * textsoftware_;   


   
   int height_;
   int width_;
   int  backgroundcolour_;
   int bit_depth_;
   int rowbytes_;
   int colortype_;
   int compressionlevel_;
   bool transformation_; // Required by Mikkel's patch
   
   unsigned char * * graph_;
   double filegamma_;
   double screengamma_;
   void circle_aux(int xcentre, int ycentre, int x, int y, int red, int green, int blue);
   void circle_aux_blend(int xcentre, int ycentre, int x, int y, double opacity, int red, int green, int blue);
   int check_if_png(char *file_name, png_FILE_p *fp);
   int read_png_info(png_FILE_p fp, png_structp *png_ptr, png_infop *info_ptr);
   int read_png_image(png_FILE_p fp, png_structp png_ptr, png_infop info_ptr,
 		       png_bytepp *image, png_uint_32 *width, png_uint_32 *height);
   void flood_fill_internal( int xstart, int ystart,  double start_red, double start_green, double start_blue, double fill_red, double fill_green, double fill_blue);
   void flood_fill_internal_blend( int xstart, int ystart, double opacity,  double start_red, double start_green, double start_blue, double fill_red, double fill_green, double fill_blue);

   
   /* The algorithms HSVtoRGB and RGBtoHSV were found at http://www.cs.rit.edu/~ncs/
    * which is a page that belongs to Nan C. Schaller, though
    * these algorithms appear to be the work of Eugene Vishnevsky. 
    * */
   void HSVtoRGB( double *r, double *g, double *b, double h, double s, double v ); 
   void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v );

   /* drwatop(), drawbottom() and filledtriangle() were contributed by Gurkan Sengun
    * ( <gurkan@linuks.mine.nu>, http://www.linuks.mine.nu/ )
    * */
   void drawtop(long x1,long y1,long x2,long y2,long x3, int red, int green, int blue);
   void drawbottom(long x1,long y1,long x2,long x3,long y3, int red, int green, int blue);
   void drawbottom_blend(long x1,long y1,long x2,long x3,long y3, double opacity, int red, int green, int blue);
   void drawtop_blend(long x1,long y1,long x2,long y2,long x3, double opacity, int red, int green, int blue);
   
 public:

   /* General Notes
    * It is important to remember that all functions that accept an argument of type "const char *" will also
    * accept "char *", this is done so you can have a changing filename (to make many PNG images in series 
    * with a different name, for example), and to allow you to use string type objects which can be easily 
    * turned into const char * (if theString is an object of type string, then it can be used as a const char *
    * by saying theString.c_str()).
    * It is also important to remember that whenever a function has a colour coeffiecient as its argument, 
    * that argument can be either an int from 0 to 65535 or a double from 0.0 to 1.0. 
    * It is important to make sure that you are calling the function with the type that you want.
    * Remember that 1 is an int, while 1.0 is a double, and will thus determine what version of the function 
    * will be used. Similarly, do not make the mistake of calling for example plot(x, y, 0.0, 0.0, 65535),
    * because
    * there is no plot(int, int, double, double, int).
    * Also, please note that plot() and read() (and the functions that use them internally) 
    * are protected against entering, for example, a colour coefficient that is over 65535
    * or over 1.0. Similarly, they are protected against negative coefficients. read() will return 0
    * when called outside the image range. This is actually useful as zero-padding should you need it.
    * */

   /* Compilation
    * A typical compilation would look like this:
    * 
    * g++ my_program.cc -o my_program freetype-config --cflags \
    *          -I/usr/local/include  -L/usr/local/lib -lpng -lpngwriter -lz -lfreetype
    * 
    * If you did not compile PNGwriter with FreeType support, then remove the
    * FreeType-related flags and add -DNO_FREETYPE above.
    * */
   
   /* Constructor
    * The constructor requires the width and the height of the image, the background colour for the
    * image and the filename of the file (a pointer or simple "myfile.png"). The background colour
    * can only be initialized to a shade of grey (once the object has been created you can do whatever 
    * you want, though), because generally one wants either a white (65535 or 1.0) or a black (0 or 0.0)
    * background to start with.
    * The default constructor creates a PNGwriter instance that is 250x250, white background,
    * and filename "out.png".
    * Tip: The filename can be given as easily as:
    * pngwriter mypng(300, 300, 0.0, "myfile.png");    
    * Tip: If you are going to create a PNGwriter instance for reading in a file that already exists, 
    * then width and height can be 1 pixel, and the size will be automatically adjusted once you use
    * readfromfile().
    * */
    MyPngWriter(int width, int height, int backgroundcolour, const char * filename);   

   /* Destructor
    * */
   ~MyPngWriter();  

   /* Assignment Operator
    * */
   MyPngWriter & operator = (const MyPngWriter & rhs);
      
   /*  Plot
    * With this function a pixel at coordinates (x, y) can be set to the desired colour. 
    * The pixels are numbered starting from (1, 1) and go to (width, height). 
    * As with most functions in PNGwriter, it has been overloaded to accept either int arguments 
    * for the colour coefficients, or those of type double. If they are of type int, 
    * they go from 0 to 65535. If they are of type double, they go from 0.0 to 1.0.
    * Tip: To plot using red, then specify plot(x, y, 1.0, 0.0, 0.0). To make pink, 
    * just add a constant value to all three coefficients, like this:
    * plot(x, y, 1.0, 0.4, 0.4). 
    * Tip: If nothing is being plotted to your PNG file, make sure that you remember
    * to close() the instance before your program is finished, and that the x and y position
    * is actually within the bounds of your image. If either is not, then PNGwriter will 
    * not complain-- it is up to you to check for this!
    * Tip: If you try to plot with a colour coefficient out of range, a maximum or minimum
    * coefficient will be assumed, according to the given coefficient. For example, attempting
    * to plot plot(x, y, 1.0,-0.2,3.7) will set the green coefficient to 0 and the red coefficient
    * to 1.0.
    * */
   void  plot(int x, int y, int red, int green, int blue, int alpha); 

   /* Figures
    * These functions draw basic shapes. Available in both int and double versions.
    * The line functions use the fast Bresenham algorithm. Despite the name, 
    * the square functions draw rectangles. The circle functions use a fast 
    * integer math algorithm. The filled circle functions make use of sqrt().
    * */
   void line(int xfrom, int yfrom, int xto, int yto, int red, int green,int  blue, int alpha);   

   unsigned char getAlpha(int x, int y);
   unsigned char getRed(int x, int  y);
   unsigned char getGreen(int x, int  y);
   unsigned char getBlue(int x, int  y);
   
   /* Clear
    * The whole image is set to black.
    * */ 
   void clear(void);    
   
   /* Close
    * Close the instance of the class, and write the image to disk.
    * Tip: If you do not call this function before your program ends, no image
    * will be written to disk.
    * */
   void close(void); 

   /* Read From File
    * Open the existing PNG image, and copy it into this instance of the class. It is important to mention 
    * that PNG variants are supported. Very generally speaking, most PNG files can now be read (as of version 0.5.4), 
    * but if they have an alpha channel it will be completely stripped. If the PNG file uses GIF-style transparency 
    * (where one colour is chosen to be transparent), PNGwriter will not read the image properly, but will not 
    * complain. Also, if any ancillary chunks are included in the PNG file (chroma, filter, etc.), it will render 
    * with a slightly different tonality. For the vast majority of PNGs, this should not be an issue. Note: 
    * If you read an 8-bit PNG, the internal representation of that instance of PNGwriter will be 8-bit (PNG 
    * files of less than 8 bits will be upscaled to 8 bits). To convert it to 16-bit, just loop over all pixels, 
    * reading them into a new instance of PNGwriter. New instances of PNGwriter are 16-bit by default.
    * */

   void readfromfile(char * name);  
   void readfromfile(const char * name); 

   /* Get Height
    * When you open a PNG with readfromfile() you can find out its height with this function.
    * */
   int getheight(void);
   
   /* Get Width
    * When you open a PNG with readfromfile() you can find out its width with this function.
    * */
   int getwidth(void);

   /* Set Compression Level
    * Set the compression level that will be used for the image. -1 is to use the  default,
    * 0 is none, 9 is best compression. 
    * Remember that this will affect how long it will take to close() the image. A value of 2 or 3
    * is good enough for regular use, but for storage or transmission you might want to take the time
    * to set it at 9.
    * */
    void setcompressionlevel(int level);

   /* Get Bit Depth
    * When you open a PNG with readfromfile() you can find out its bit depth with this function.
    * Mostly for troubleshooting uses.
    * */
   int getbitdepth(void);
   
   /* Get Colour Type
    * When you open a PNG with readfromfile() you can find out its colour type (libpng categorizes 
    * different styles of image data with this number).
    * Mostly for troubleshooting uses.
    * */
   int getcolortype(void);
   
   /* Set Gamma Coeff
    * Set the image's gamma (file gamma) coefficient. This is experimental, but use it if your image's colours seem too bright
    * or too dark. The default value of 0.5 should be fine. The standard disclaimer about Mac and PC gamma
    * settings applies.
    * */
   void setgamma(double gamma);

   
   /* Get Gamma Coeff
    * Get the image's gamma coefficient. This is experimental.
    * */
   double getgamma(void);

    /* Version Number
    * Returns the PNGwriter version number.
    */
  static double version(void);  

   /* Write PNG
    * Writes the PNG image to disk. You can still change the PNGwriter instance after this.
    * Tip: This is exactly the same as close(), but easier to remember.
    * Tip: To make a sequence of images using only one instance of PNGwriter, alter the image, change its name,
    * write_png(), then alter the image, change its name, write_png(), etc.
    */
   void write_png(void);

   void write_out_red_as_alpha();

   /* Invert
    * Inverts the image in RGB colourspace.
    * */
   void invert(void);

};
#endif
