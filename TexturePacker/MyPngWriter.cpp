//**********  MyPngWriter.cc   **********************************************
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

#include "MyPngWriter.h"


//Constructor for int colour levels, char * filename
//////////////////////////////////////////////////////////////////////////
MyPngWriter::MyPngWriter(int x, int y, int backgroundcolour, const char * filename)
{
   width_ = x;
   height_ = y;
   backgroundcolour_ = backgroundcolour;
   compressionlevel_ = -2;
   filegamma_ = 0.6;
   transformation_ = 0;

   textauthor_ = new char[255];
   textdescription_ = new char[255];
   texttitle_ = new char[strlen(filename)+1];
   textsoftware_ = new char[255];
   filename_ = new char[strlen(filename)+1];

   strcpy(textauthor_, "MyPngWriter Author: Paul Blackburn");
   strcpy(textdescription_, "http://MyPngWriter.sourceforge.net/");
   strcpy(textsoftware_, "MyPngWriter: An easy to use graphics library.");
   strcpy(texttitle_, filename);
   strcpy(filename_, filename);

   if((width_<0)||(height_<0))
     {
	std::cerr << " MyPngWriter::MyPngWriter - ERROR **: Constructor called with negative height or width. Setting width and height to 1." << std::endl;
	width_ = 1;
	height_ = 1;
     }

   if(backgroundcolour_ >255)
     {
	std::cerr << " MyPngWriter::MyPngWriter - WARNING **: Constructor called with background colour greater than 65535. Setting to 65535."<<std::endl;
	backgroundcolour_ = 255;
     }

   if(backgroundcolour_ <0)
     {
	std::cerr << " MyPngWriter::MyPngWriter - WARNING **: Constructor called with background colour lower than 0. Setting to 0."<<std::endl;
	backgroundcolour_ = 0;
     }

   int kkkk;

   bit_depth_ = 8; //Default bit depth for new images
   colortype_=2;
   screengamma_ = 2.2;

   graph_ = (png_bytepp)malloc(height_ * sizeof(png_bytep));
   if(graph_ == NULL)
     {
	std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
     }

   for (kkkk = 0; kkkk < height_; kkkk++)
     {
        graph_[kkkk] = (png_bytep)malloc(4*width_ * sizeof(png_byte));
	if(graph_[kkkk] == NULL)
	  {
	     std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
	  }
     }

   if(graph_ == NULL)
     {
	std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
     }

   int tempindex;
   for(int hhh = 0; hhh<width_;hhh++)
     {
	for(int vhhh = 0; vhhh<height_;vhhh++)
	  {
	     tempindex = 4*hhh;
	     graph_[vhhh][tempindex] = (char)(backgroundcolour_%256);
	     graph_[vhhh][tempindex+1] = (char)(backgroundcolour_%256);
	     graph_[vhhh][tempindex+2] = (char)(backgroundcolour_%256);
	     graph_[vhhh][tempindex+3] = (char)(backgroundcolour_%256);
	  }
     }

};

//Destructor
///////////////////////////////////////
MyPngWriter::~MyPngWriter()
{
   delete [] filename_;
   delete [] textauthor_;
   delete [] textdescription_;
   delete [] texttitle_;
   delete [] textsoftware_;

   for (int jjj = 0; jjj < height_; jjj++) free(graph_[jjj]);
   free(graph_);
};

// Overloading operator =
/////////////////////////////////////////////////////////
MyPngWriter & MyPngWriter::operator = (const MyPngWriter & rhs)
{
   if( this==&rhs)
     {
	return *this;
     }

   width_ = rhs.width_;
   height_ = rhs.height_;
   backgroundcolour_ = rhs.backgroundcolour_;
   compressionlevel_ = rhs.compressionlevel_;
   filegamma_ = rhs.filegamma_;
   transformation_ = rhs.transformation_;

   filename_ = new char[strlen(rhs.filename_)+1];
   textauthor_ = new char[strlen(rhs.textauthor_)+1];
   textdescription_ = new char[strlen(rhs.textdescription_)+1];
   textsoftware_ = new char[strlen(rhs.textsoftware_)+1];
   texttitle_ = new char[strlen(rhs.texttitle_)+1];

   strcpy(textauthor_, rhs.textauthor_);
   strcpy(textdescription_, rhs.textdescription_);
   strcpy(textsoftware_,rhs.textsoftware_);
   strcpy(texttitle_, rhs.texttitle_);
   strcpy(filename_, rhs.filename_);

   int kkkk;

   bit_depth_ = rhs.bit_depth_;
   colortype_= rhs.colortype_;
   screengamma_ = rhs.screengamma_;

   graph_ = (png_bytepp)malloc(height_ * sizeof(png_bytep));
   if(graph_ == NULL)
     {
	std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
     }

   for (kkkk = 0; kkkk < height_; kkkk++)
     {
        graph_[kkkk] = (png_bytep)malloc(6*width_ * sizeof(png_byte));
	if(graph_[kkkk] == NULL)
	  {
	     std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
	  }
     }

   if(graph_ == NULL)
     {
	std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
     }

   int tempindex;
   for(int hhh = 0; hhh<width_;hhh++)
     {
	for(int vhhh = 0; vhhh<height_;vhhh++)
	  {
	     tempindex=4*hhh;
	     graph_[vhhh][tempindex] = rhs.graph_[vhhh][tempindex];
	     graph_[vhhh][tempindex+1] = rhs.graph_[vhhh][tempindex+1];
	     graph_[vhhh][tempindex+2] = rhs.graph_[vhhh][tempindex+2];
	     graph_[vhhh][tempindex+3] = rhs.graph_[vhhh][tempindex+3];
	  }
     }

   return *this;
}

///////////////////////////////////////////////////////////////
void MyPngWriter::plot(int x, int y, int red, int green, int blue, int alpha)
{
   int tempindex;

   if(red > 255)
     {
	red = 255;
     }
   if(green > 255)
     {
	green = 255;
     }
   if(blue > 255)
     {
	blue = 255;
     }

   if(red < 0)
     {
	red = 0;
     }
   if(green < 0)
     {
	green = 0;
     }
   if(blue < 0)
     {
	blue = 0;
     }

   if((bit_depth_ == 8))
     {
	    if( (y<height_) && (y>0) && (x>0) && (x<width_) )
	      {
	         tempindex = 4*x;
	         graph_[y][tempindex] = (unsigned char)(red);
	         graph_[y][tempindex+1] = (unsigned char)(green);
	         graph_[y][tempindex+2] = (unsigned char)(blue);
             graph_[y][tempindex+3] = (unsigned char)(alpha);
	      };
     }
};

unsigned char  MyPngWriter::getAlpha(int x, int y)
{
    if((bit_depth_ == 8))
    {
        if( (y<height_) && (y>0) && (x>0) && (x<width_) )
        {
            return graph_[y][4*x+3];
        }
    }

    return 0;
}

unsigned char MyPngWriter::getRed(int x, int  y)
{
    if((bit_depth_ == 8))
    {
        if( (y<height_) && (y>0) && (x>0) && (x<width_) )
        {
            return graph_[y][4*x+0];
        }
    }

    return 0;
}
unsigned char MyPngWriter::getGreen(int x, int  y)
{
    if((bit_depth_ == 8))
    {
        if( (y<height_) && (y>0) && (x>0) && (x<width_) )
        {
            return graph_[y][4*x+1];
        }
    }

    return 0;
}

unsigned char MyPngWriter::getBlue(int x, int  y)
{
    if((bit_depth_ == 8))
    {
        if( (y<height_) && (y>0) && (x>0) && (x<width_) )
        {
            return graph_[y][4*x+2];
        }
    }

    return 0;
}

///////////////////////////////////////////////////////
void MyPngWriter::clear()
{
   int pen = 0;
   int pencil = 0;
   int tempindex;

   if(bit_depth_==8)
     {
	for(pen = 0; pen<width_;pen++)
	  {
	     for(pencil = 0; pencil<height_;pencil++)
	       {
		  tempindex=4*pen;
		  graph_[pencil][tempindex] = 0;
		  graph_[pencil][tempindex+1] = 0;
		  graph_[pencil][tempindex+2] = 0;
          graph_[pencil][tempindex+3] = 0;
	       }
	  }
     }

};


///////////////////////////////////////////////////////
void MyPngWriter::close()
{
   png_FILE_p      fp;
   png_structp     png_ptr;
   png_infop       info_ptr;

   fp = fopen(filename_, "wb");
   if( fp == NULL)
     {
	std::cerr << " MyPngWriter::close - ERROR **: Error creating file (fopen() returned NULL pointer)." << std::endl;
	perror(" MyPngWriter::close - ERROR **");
	return;
     }

   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   info_ptr = png_create_info_struct(png_ptr);
   png_init_io(png_ptr, fp);
   if(compressionlevel_ != -2)
     {
	png_set_compression_level(png_ptr, compressionlevel_);
     }
   else
     {
	png_set_compression_level(png_ptr, PNGWRITER_DEFAULT_COMPRESSION);
     }

   png_set_IHDR(png_ptr, info_ptr, width_, height_,
		bit_depth_, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

   if(filegamma_ < 1.0e-1)
     {
	filegamma_ = 0.5;  // Modified in 0.5.4 so as to be the same as the usual gamma.
     }

   png_set_gAMA(png_ptr, info_ptr, filegamma_);

   time_t          gmt;
   png_time        mod_time;
   png_text        text_ptr[5];
   time(&gmt);
   png_convert_from_time_t(&mod_time, gmt);
   png_set_tIME(png_ptr, info_ptr, &mod_time);
   text_ptr[0].key = "Title";
   text_ptr[0].text = texttitle_;
   text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[1].key = "Author";
   text_ptr[1].text = textauthor_;
   text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[2].key = "Description";
   text_ptr[2].text = textdescription_;
   text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[3].key = "Creation Time";
   text_ptr[3].text = png_convert_to_rfc1123(png_ptr, &mod_time);
   text_ptr[3].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[4].key = "Software";
   text_ptr[4].text = textsoftware_;
   text_ptr[4].compression = PNG_TEXT_COMPRESSION_NONE;
   png_set_text(png_ptr, info_ptr, text_ptr, 5);

   png_write_info(png_ptr, info_ptr);
   png_write_image(png_ptr, graph_);
   png_write_end(png_ptr, info_ptr);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   fclose(fp);
}

////////////////Reading routines/////////////////////
/////////////////////////////////////////////////

// Modified with Mikkel's patch
void MyPngWriter::readfromfile(char * name)
{
   png_FILE_p      fp;
   png_structp     png_ptr;
   png_infop       info_ptr;
   unsigned char   **image;
   unsigned long   width, height;
   int bit_depth, color_type, interlace_type;
   //   png_uint_32     i;
   //
   fp = fopen(name,"rb");
   if (fp==NULL)
     {
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file \"" << std::flush;
	std::cerr << name <<std::flush;
	std::cerr << "\"." << std::endl << std::flush;
	perror(" MyPngWriter::readfromfile - ERROR **");
	return;
     }

   if(!check_if_png(name, &fp))
     {
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". This may not be a valid png file. (check_if_png() failed)." << std::endl;
	// fp has been closed already if check_if_png() fails.
	return;
     }

//   Code as it was before Sven's patch
/*   if(!read_png_info(fp, &png_ptr, &info_ptr))
     {
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_info() failed." << std::endl;
	// fp has been closed already if read_png_info() fails.
	return;
     }

   if(!read_png_image(fp, png_ptr, info_ptr, &image, &width, &height))
     {
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_image() failed." << std::endl;
	// fp has been closed already if read_png_image() fails.
	return;
     }

   //stuff should now be in image[][].

 */ //End of code as it was before Sven's patch.
   
   //Sven's patch starts here
   ////////////////////////////////////
 
   /*
     
   if(!read_png_info(fp, &png_ptr, &info_ptr)) 
     {
	 
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_info() failed." << std::endl; 
	// fp has been closed already if read_png_info() fails. 
	return; 
     } 
   
   // UPDATE: Query info struct to get header info BEFORE reading the image  
   
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL); 
   bit_depth_ = bit_depth; 
   colortype_ = color_type; 
   
   if(color_type == PNG_COLOR_TYPE_PALETTE) 
     { 
	png_set_expand(png_ptr); 
	png_read_update_info(png_ptr, info_ptr); 
     } 
   
   if(!read_png_image(fp, png_ptr, info_ptr, &image, &width, &height)) 
     { 
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_image() failed." << std::endl; 
	// fp has been closed already if read_png_image() fails. 
	return; 
     } 
   
   //stuff should now be in image[][]. 
   */
   //Sven's patch ends here.
   ////////////////////////////////
   
   // Mikkel's patch starts here
   // ///////////////////////////////////
   
   if(!read_png_info(fp, &png_ptr, &info_ptr)) 
     {
	 
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_info() failed." << std::endl; 
	// fp has been closed already if read_png_info() fails. 
	  return; 
     } 
   
   //Input transformations  
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL); 
   bit_depth_ = bit_depth; 
   colortype_ = color_type; 
   
    
   // Changes palletted image to RGB
   if(color_type == PNG_COLOR_TYPE_PALETTE /*&& bit_depth<8*/) 
     { 
	// png_set_expand(png_ptr);  
	png_set_palette_to_rgb(png_ptr);  // Just an alias of png_set_expand()
	transformation_ = 1; 
     } 
   
   // Transforms grescale images of less than 8 bits to 8 bits.
   if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8) 
     { 
	// png_set_expand(png_ptr); 
	png_set_gray_1_2_4_to_8(png_ptr);  // Just an alias of the above.
	transformation_ = 1; 
     } 
   
   // Completely strips the alpha channel.
 //  if(color_type & PNG_COLOR_MASK_ALPHA) 
 //    { 
	//png_set_strip_alpha(png_ptr); 
	//transformation_ = 1; 
 //    } 
   
   // Converts greyscale images to RGB.
   if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) // Used to be RGB, fixed it. 
     { 
	png_set_gray_to_rgb(png_ptr); 
	transformation_ = 1; 
     } 
	 
	 // If any of the above were applied,
   if(transformation_) 
     { 
	 // png_set_gray_to_rgb(png_ptr);   //Is this really needed here?
	 
	 // After setting the transformations, libpng can update your png_info structure to reflect any transformations 
	 // you've requested with this call. This is most useful to update the info structure's rowbytes field so you can 
	 // use it to allocate your image memory. This function will also update your palette with the correct screen_gamma 
	 // and background if these have been given with the calls above.
	 
	png_read_update_info(png_ptr, info_ptr); 
	
	// Just in case any of these have changed?
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL); 
	bit_depth_ = bit_depth; 
	colortype_ = color_type; 
     } 
   
   if(!read_png_image(fp, png_ptr, info_ptr, &image, &width, &height)) 
     { 
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". read_png_image() failed." << std::endl; 
	// fp has been closed already if read_png_image() fails. 
	return; 
     } 
   
   //stuff should now be in image[][].
   
   // Mikkel's patch ends here
   // //////////////////////////////
   // 
   if( image == NULL)
     {
	std::cerr << " MyPngWriter::readfromfile - ERROR **: Error opening file " << name << ". Can't assign memory (after read_png_image(), image is NULL)." << std::endl;
	fclose(fp);
	return;
     }

   //First we must get rid of the image already there, and free the memory.
   int jjj;
   for (jjj = 0; jjj < height_; jjj++) free(graph_[jjj]);
   free(graph_);

   //Must reassign the new size of the read image
   width_ = width;
   height_ = height;

   //Graph now is the image.
   graph_ = image;

   rowbytes_ = png_get_rowbytes(png_ptr, info_ptr);

	// This was part of the original source, but has been moved up.
/*
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
   bit_depth_ = bit_depth;
   colortype_ = color_type;
*/
  // if(color_type == PNG_COLOR_TYPE_PALETTE /*&& bit_depth<8*/   )
 /*    {
	png_set_expand(png_ptr);
     }

   if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8)
     {
	png_set_expand(png_ptr);
     }

   if(color_type & PNG_COLOR_MASK_ALPHA)
     {
	png_set_strip_alpha(png_ptr);
     }

   if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
     {
	png_set_gray_to_rgb(png_ptr);
     }

*/

   if((bit_depth_ !=16)&&(bit_depth_ !=8))
     {
	std::cerr << " MyPngWriter::readfromfile() - WARNING **: Input file is of unsupported type (bad bit_depth). Output will be unpredictable.\n";
     }

// Thanks to Mikkel's patch, MyPngWriter should now be able to handle these color types:

/* 
color_type     - describes which color/alpha channels are present.
                     PNG_COLOR_TYPE_GRAY                        (bit depths 1, 2, 4, 8, 16)
                     PNG_COLOR_TYPE_GRAY_ALPHA                        (bit depths 8, 16)
					 PNG_COLOR_TYPE_PALETTE                        (bit depths 1, 2, 4, 8)
                     PNG_COLOR_TYPE_RGB                        (bit_depths 8, 16)
                     PNG_COLOR_TYPE_RGB_ALPHA                        (bit_depths 8, 16)

                     PNG_COLOR_MASK_PALETTE
                     PNG_COLOR_MASK_COLOR
					 
color types.  Note that not all combinations are legal 
#define PNG_COLOR_TYPE_GRAY 0
#define PNG_COLOR_TYPE_PALETTE  (PNG_COLOR_MASK_COLOR (2) | PNG_COLOR_MASK_PALETTE (1) )
#define PNG_COLOR_TYPE_RGB        (PNG_COLOR_MASK_COLOR (2) )
#define PNG_COLOR_TYPE_RGB_ALPHA  (PNG_COLOR_MASK_COLOR (2) | PNG_COLOR_MASK_ALPHA (4) )
#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA (4) )

aliases 
#define PNG_COLOR_TYPE_RGBA  PNG_COLOR_TYPE_RGB_ALPHA
#define PNG_COLOR_TYPE_GA  PNG_COLOR_TYPE_GRAY_ALPHA

 These describe the color_type field in png_info. 
 color type masks 
#define PNG_COLOR_MASK_PALETTE    1
#define PNG_COLOR_MASK_COLOR      2
#define PNG_COLOR_MASK_ALPHA      4


					 */
					 
					 
 //  if(colortype_ !=2)
 //    {
	//std::cerr << " MyPngWriter::readfromfile() - WARNING **: Input file is of unsupported type (bad color_type). Output will be unpredictable.\n";
 //    }

   screengamma_ = 2.2;
   double          file_gamma,screen_gamma;
   screen_gamma = screengamma_;
   if (png_get_gAMA(png_ptr, info_ptr, &file_gamma))
     {
	png_set_gamma(png_ptr,screen_gamma,file_gamma);
     }
   else
     {
	png_set_gamma(png_ptr, screen_gamma,0.45);
     }

   filegamma_ = file_gamma;

   fclose(fp);
}

///////////////////////////////////////////////////////

void MyPngWriter::readfromfile(const char * name)
{
   this->readfromfile((char *)(name));
}

/////////////////////////////////////////////////////////
int MyPngWriter::check_if_png(char *file_name, png_FILE_p *fp)
{
   char    sig[PNG_BYTES_TO_CHECK];

   if ( /*(*fp = fopen(file_name, "rb")) */  *fp == NULL) // Fixed 10 10 04
     {
	//   exit(EXIT_FAILURE);
	std::cerr << " MyPngWriter::check_if_png - ERROR **: Could not open file  " << file_name << " to read." << std::endl;
	perror(" MyPngWriter::check_if_png - ERROR **");
	return 0;
     }

   if (fread(sig, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK)
     {
	//exit(EXIT_FAILURE);
	std::cerr << " MyPngWriter::check_if_png - ERROR **: File " << file_name << " does not appear to be a valid PNG file." << std::endl;
	perror(" MyPngWriter::check_if_png - ERROR **");
	fclose(*fp);
	return 0;
     }
   
   if (png_sig_cmp( (png_bytep) sig, (png_size_t)0, PNG_BYTES_TO_CHECK) /*png_check_sig((png_bytep) sig, PNG_BYTES_TO_CHECK)*/ ) 
     {
	std::cerr << " MyPngWriter::check_if_png - ERROR **: File " << file_name << " does not appear to be a valid PNG file. png_check_sig() failed." << std::endl;
	fclose(*fp);
	return 0;
     }
   
   
   
   return 1; //Success
}

///////////////////////////////////////////////////////
int MyPngWriter::read_png_info(png_FILE_p fp, png_structp *png_ptr, png_infop *info_ptr)
{
   *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (*png_ptr == NULL)
     {
	std::cerr << " MyPngWriter::read_png_info - ERROR **: Could not create read_struct." << std::endl;
	fclose(fp);
	return 0;
	//exit(EXIT_FAILURE);
     }
   *info_ptr = png_create_info_struct(*png_ptr);
   if (*info_ptr == NULL)
     {
	png_destroy_read_struct(png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	std::cerr << " MyPngWriter::read_png_info - ERROR **: Could not create info_struct." << std::endl;
	//exit(EXIT_FAILURE);
	fclose(fp);
	return 0;
     }
   if (setjmp((*png_ptr)->jmpbuf)) /*(setjmp(png_jmpbuf(*png_ptr)) )*//////////////////////////////////////
     {
	png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
	std::cerr << " MyPngWriter::read_png_info - ERROR **: This file may be a corrupted PNG file. (setjmp(*png_ptr)->jmpbf) failed)." << std::endl;
	fclose(fp);
	return 0;
	//exit(EXIT_FAILURE);
     }
   png_init_io(*png_ptr, fp);
   png_set_sig_bytes(*png_ptr, PNG_BYTES_TO_CHECK);
   png_read_info(*png_ptr, *info_ptr);

   return 1;
}

////////////////////////////////////////////////////////////
int MyPngWriter::read_png_image(png_FILE_p fp, png_structp png_ptr, png_infop info_ptr,
			      png_bytepp *image, png_uint_32 *width, png_uint_32 *height)
{
   unsigned int i,j;

   *width = png_get_image_width(png_ptr, info_ptr);
   *height = png_get_image_height(png_ptr, info_ptr);

   if( width == NULL)
     {
	std::cerr << " MyPngWriter::read_png_image - ERROR **: png_get_image_width() returned NULL pointer." << std::endl;
	fclose(fp);
	return 0;
     }

   if( height == NULL)
     {
	std::cerr << " MyPngWriter::read_png_image - ERROR **: png_get_image_height() returned NULL pointer." << std::endl;
	fclose(fp);
	return 0;
     }

   if ((*image = (png_bytepp)malloc(*height * sizeof(png_bytep))) == NULL)
     {
	std::cerr << " MyPngWriter::read_png_image - ERROR **: Could not allocate memory for reading image." << std::endl;
	fclose(fp);
	return 0;
	//exit(EXIT_FAILURE);
     }
   for (i = 0; i < *height; i++)
     {
	(*image)[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
	if ((*image)[i] == NULL)
	  {
	     for (j = 0; j < i; j++) free((*image)[j]);
	     free(*image);
	     fclose(fp);
	     std::cerr << " MyPngWriter::read_png_image - ERROR **: Could not allocate memory for reading image." << std::endl;
	     return 0;
	     //exit(EXIT_FAILURE);
	  }
     }
   png_read_image(png_ptr, *image);

   return 1;
}

///////////////////////////////////
int MyPngWriter::getheight(void)
{
   return height_;
}

int MyPngWriter::getwidth(void)
{
   return width_;
}


int MyPngWriter::getbitdepth(void)
{
   return bit_depth_;
}

int MyPngWriter::getcolortype(void)
{
   return colortype_;
}

double MyPngWriter::getgamma(void)
{
   return filegamma_;
}

void MyPngWriter::setgamma(double gamma)
{
   filegamma_ = gamma;
}


void MyPngWriter::setcompressionlevel(int level)
{
   if( (level < -1)||(level > 9) )
     {
	std::cerr << " MyPngWriter::setcompressionlevel - ERROR **: Called with wrong compression level: should be -1 to 9, was: " << level << "." << std::endl;
     }
   compressionlevel_ = level;
}

/*
int MyPngWriter::getcompressionlevel(void)
{
   return png_get_compression_level(png_ptr);
}
*/

double MyPngWriter::version(void)
{
   const char *a = "Jeramy Webb (jeramyw@gmail.com), Mike Heller (mkheller@gmail.com)"; // For their generosity ;-)
   char b = a[27];
   b++;
   return (0);
}

void MyPngWriter::write_png(void)
{
   this->close();
}

void MyPngWriter::write_out_red_as_alpha()
{
    unsigned char * *data = (png_bytepp)malloc(height_ * sizeof(png_bytep));
    if(data == NULL)
    {
        std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
    }

    for (int kkkk = 0; kkkk < height_; kkkk++)
    {
        data[kkkk] = (png_bytep)malloc(width_ * sizeof(png_byte));
        if(graph_[kkkk] == NULL)
        {
            std::cerr << " MyPngWriter::MyPngWriter - ERROR **:  Not able to allocate memory for image." << std::endl;
        }
    }

    for (int i=0; i<height_; ++i)
        for (int j=0; j<width_; ++j)
            data[i][j] = graph_[i][j*4];


    png_FILE_p      fp;
    png_structp     png_ptr;
    png_infop       info_ptr;

    fp = fopen(filename_, "wb");
    if( fp == NULL)
    {
        std::cerr << " MyPngWriter::close - ERROR **: Error creating file (fopen() returned NULL pointer)." << std::endl;
        perror(" MyPngWriter::close - ERROR **");
        return;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    if(compressionlevel_ != -2)
    {
        png_set_compression_level(png_ptr, compressionlevel_);
    }
    else
    {
        png_set_compression_level(png_ptr, PNGWRITER_DEFAULT_COMPRESSION);
    }

    png_set_IHDR(png_ptr, info_ptr, width_, height_,
        bit_depth_, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if(filegamma_ < 1.0e-1)
    {
        filegamma_ = 0.5;  // Modified in 0.5.4 so as to be the same as the usual gamma.
    }

    png_set_gAMA(png_ptr, info_ptr, filegamma_);

    time_t          gmt;
    png_time        mod_time;
    png_text        text_ptr[5];
    time(&gmt);
    png_convert_from_time_t(&mod_time, gmt);
    png_set_tIME(png_ptr, info_ptr, &mod_time);
    text_ptr[0].key = "Title";
    text_ptr[0].text = texttitle_;
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[1].key = "Author";
    text_ptr[1].text = textauthor_;
    text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[2].key = "Description";
    text_ptr[2].text = textdescription_;
    text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[3].key = "Creation Time";
    text_ptr[3].text = png_convert_to_rfc1123(png_ptr, &mod_time);
    text_ptr[3].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[4].key = "Software";
    text_ptr[4].text = textsoftware_;
    text_ptr[4].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png_ptr, info_ptr, text_ptr, 5);

    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, data);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}


//////////////////////////////////////////////////////
void MyPngWriter::line(int xfrom, int yfrom, int xto, int yto, int red, int green,int  blue, int alpha)
{
    //  Bresenham Algorithm.
    //
    int dy = yto - yfrom;
    int dx = xto - xfrom;
    int stepx, stepy;

    if (dy < 0)
    {
        dy = -dy;  stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    if (dx < 0)
    {
        dx = -dx;  stepx = -1;
    }
    else
    {
        stepx = 1;
    }
    dy <<= 1;     // dy is now 2*dy
    dx <<= 1;     // dx is now 2*dx

    this->plot(xfrom,yfrom,red,green,blue,alpha);

    if (dx > dy)
    {
        int fraction = dy - (dx >> 1);

        while (xfrom != xto)
        {
            if (fraction >= 0)
            {
                yfrom += stepy;
                fraction -= dx;
            }
            xfrom += stepx;
            fraction += dy;
            this->plot(xfrom,yfrom,red,green,blue, alpha);
        }
    }
    else
    {
        int fraction = dx - (dy >> 1);
        while (yfrom != yto)
        {
            if (fraction >= 0)
            {
                xfrom += stepx;
                fraction -= dy;
            }
            yfrom += stepy;
            fraction += dx;
            this->plot(xfrom,yfrom,red,green,blue,alpha);
        }
    }

}
