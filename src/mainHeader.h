#ifndef mainHeader.h
#define mainHeader.h

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#  include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <OpenImageIO/imageio.h>
#include <iostream>
#include <fstream>
#include <math.h>
OIIO_NAMESPACE_USING
using namespace std;

struct pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct image{
  pixel* pixmap;
  int W;
  int H;
  int channels;
};


image bufferImage;
image originalImage;
image energyImage;
float * heatMap;

float **filter;
int filterSize; //Filter size

void copyImageTo(image *source, image *target);
int readImage(char *filename, pixel **imageptr, int *w, int *h, int *ch);
void writeimage(char* outfilename);
void copyImageToBuffer(image *source);
double gaussianFunc(int x, int y, int sigma, int theta, int period, bool Rotated);
void applySobel();
void normalizeFilter(float **filter, int N);
void HorzFlip(float **filter, int columns, int rows);
void VerticalFlip(float **filter, int columns, int rows);

float clamp(float num, float min, float max){
  if(num < min){
    return min;
  } else if (num > max){
    return max;
  } else {
    return num;
  }
}
 

image applyFilter(float toCopy[3][3]){

    float* weight = new float[filterSize*filterSize];
    for (int row=0; row < filterSize; row++)
            for (int col=0; col < filterSize; col++)
                weight[row*filterSize+col] = toCopy[row][col];

    filter = new float*[filterSize];
    filter[0] = weight;
    for(int i = 1; i<filterSize; i++){
        filter[i] = filter[i-1] + filterSize;
    }

    HorzFlip(filter, filterSize, filterSize);
    VerticalFlip(filter, filterSize, filterSize);
    normalizeFilter(filter, filterSize);

    // cout << "The filter:" << endl;
    // for (int row=0; row < filterSize; row++){
    //     for (int col=0; col < filterSize; col++){
    //         cout << filter[row][col] << " ";
    //     }
    //     cout << endl;
    // }

    image temp;
    temp.H = bufferImage.H;
    temp.W = bufferImage.W;
    temp.channels = bufferImage.channels;
    temp.pixmap = new pixel[temp.H*temp.W];
    for (int row=0; row < temp.W; row++){
        for (int col=0; col < temp.H; col++){
        temp.pixmap[row+col*temp.W].r = 0;
        temp.pixmap[row+col*temp.W].b = 0;
        temp.pixmap[row+col*temp.W].g = 0;

        }
    }

    for (int y = 0; y < bufferImage.H; y++){
        for (int x = 0; x < bufferImage.W; x++){
        float resR = 0;
        float resG = 0;
        float resB = 0;
        for (int yk = 0; yk < filterSize; yk++){
            for (int xk = 0; xk < filterSize; xk++){
            int acx = -filterSize/2 + xk + x;
            int acy = -filterSize/2 + yk + y;
            if(acx < 0 || acx >= bufferImage.W || acy < 0 || acy >= bufferImage.H){
                continue;
            }
            resR += bufferImage.pixmap[acx+acy*bufferImage.W].r * filter[yk][xk];
            resG += bufferImage.pixmap[acx+acy*bufferImage.W].g * filter[yk][xk];
            resB += bufferImage.pixmap[acx+acy*bufferImage.W].b * filter[yk][xk];
            // printf("Acx  = %d, acy = %d, xk = %d, yk = %d\n", acx, acy, xk, yk);
            // cout << resR << endl;
            }
        }
        temp.pixmap[x+y*bufferImage.W].r = (unsigned char)clamp(resR, 0, 255);
        temp.pixmap[x+y*bufferImage.W].g = (unsigned char)clamp(resG, 0, 255);
        temp.pixmap[x+y*bufferImage.W].b = (unsigned char)clamp(resB, 0, 255);
        temp.pixmap[x+y*bufferImage.W].a = 255;
        }
    }
    return temp;
}

void swap_(float *a, float *b)
{
    float temp = *a;
    *a = *b;
    *b = temp; 
}

void HorzFlip(float **filter, int columns, int rows)
{
    for (int row = 0; row < rows; row++) {
        float *currentFloat = filter[row];
        for (int col = 0; col < columns / 2; col++) {
            swap_(currentFloat+col, currentFloat+columns-1-col);
        } 
    }
}

void VerticalFlip(float **filter, int columns, int rows)
{
    for (int column = 0; column < columns; column++) {
        for (int row = 0; row < rows/2; row++) {
            swap_(filter[row]+column, filter[rows-1-row]+column);
        }
    }
}

void normalizeFilter(float **filter, int N){

    float sumOfPositives = 0;
    float sumOfNegetives = 0;
    for (int x=0; x < N; x++){
        for (int y=0; y < N; y++){
            if(filter[x][y] > 0){
                sumOfPositives += filter[x][y];
            } else {
                sumOfNegetives += filter[x][y];
            }
        }
    }

    float toDivide = sumOfPositives;
    if(abs(sumOfNegetives) > sumOfPositives){
        toDivide = abs(sumOfNegetives);
    }
    for (int x=0; x < N; x++){
        for (int y=0; y < N; y++){
            filter[x][y] = filter[x][y] / toDivide;
        }
    }

}

void copyImageToBuffer(image source){

  bufferImage.pixmap = new pixel[source.W*source.H];
  bufferImage.W = source.W;
  bufferImage.H = source.H;

  for(int x = 0; x < bufferImage.W; x++){
      for(int y = 0; y < bufferImage.H; y++){
          bufferImage.pixmap[x+y*bufferImage.W].r = source.pixmap[x+y*bufferImage.W].r;
          bufferImage.pixmap[x+y*bufferImage.W].g = source.pixmap[x+y*bufferImage.W].g;
          bufferImage.pixmap[x+y*bufferImage.W].b = source.pixmap[x+y*bufferImage.W].b;
          bufferImage.pixmap[x+y*bufferImage.W].a = source.pixmap[x+y*bufferImage.W].a;
      }
  }
  glutReshapeWindow(bufferImage.W, bufferImage.H);
  glutPostRedisplay();
}

void copyImageTo(image *source, image *target){

  target->pixmap = new pixel[source->W*source->H];
  target->W = source->W;
  target->H = source->H;

  for(int x = 0; x < target->W; x++){
      for(int y = 0; y < target->H; y++){
          target->pixmap[x+y*target->W].r = source->pixmap[x+y*target->W].r;
          target->pixmap[x+y*target->W].g = source->pixmap[x+y*target->W].g;
          target->pixmap[x+y*target->W].b = source->pixmap[x+y*target->W].b;
          target->pixmap[x+y*target->W].a = source->pixmap[x+y*target->W].a;
      }
  }
  glutReshapeWindow(target->W, target->H);
  glutPostRedisplay();

}

void writeimage(char* outfilename){


    pixel *pixmap = new pixel[bufferImage.W*bufferImage.H];


    // create the oiio file handler for the image
    std::unique_ptr<ImageOutput> outimage = ImageOutput::create(outfilename);
    if(!outimage){
        cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
        return;
    }



    // Flip the read image to be in the correct orintation 
    for(int x = 0; x < bufferImage.W; x++){

        //keep a temporary memory of the column
        pixel *temp = new pixel[bufferImage.H];
        for(int q = 0; q < bufferImage.H; q++){
        temp[q] = bufferImage.pixmap[(bufferImage.H-1-q)*bufferImage.W+x];
        temp[q].a = 255;
        }

        //read the flipped column back to the image
        for(int y = 0; y < bufferImage.H; y++){
        pixmap[y*bufferImage.W+x] = temp[y];
        }
    }

    // open a file for writing the image. The file header will indicate an image of
    // width w, height h, and 4 channels per pixel (RGBA). All channels will be of
    // type unsigned char
    ImageSpec *spec = new ImageSpec(bufferImage.W, bufferImage.H, 4, TypeDesc::UINT8);
    if(!outimage->open(outfilename, *spec)){
        cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
        return;
    }

    // // write the image to the file. All channel values in the pixmap are taken to be
    // // unsigned chars
    if(!outimage->write_image(TypeDesc::UINT8, pixmap)){
        cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
        return;
    }
    else
        cout << "Image saved successfully!" << endl;

    // close the image file after the image is written
    if(!outimage->close()){
        cerr << "Could not close " << outfilename << ", error = " << geterror() << endl;
        return;
    }

}

// Read an image with a given filename and assignes it to the parameters passed
int readImage(char *filename, pixel **imageptr, int *w, int *h, int *ch){

  auto input = ImageInput::open(filename);
  if (!input){
    printf("Error encountred reading an image\n");
    return 0; 
  }
  
  const ImageSpec &spec = input->spec();
  bool wasAGrayScale = false;
  *w = spec.width;
  *h = spec.height;
  *ch = spec.nchannels;
  int W = spec.width;
  int H = spec.height;
  int channels = spec.nchannels;
  *imageptr = new pixel[(*w)*(*h)];



  
    unsigned char *temp;
    if(channels == 1){
        temp = new unsigned char[W*H];
        if(!input->read_image(TypeDesc::UINT8, temp)){
            cerr << "Error encotunred trying to read theimage file. Error message: " << geterror() << endl;
            return 0;
        }

        for(int x = 0; x < W; x++){
            for(int y = 0; y < H; y++){
                (*imageptr)[x+y*W].r = temp[x+y*W];
                (*imageptr)[x+y*W].g = temp[x+y*W];
                (*imageptr)[x+y*W].b = temp[x+y*W];
                (*imageptr)[x+y*W].a = 255;
            }
        }
    } else if(channels == 3) {
        temp = new unsigned char[3*W*H];
        if(!input->read_image(TypeDesc::UINT8, temp)){
            cerr << "Error encotunred trying to read theimage file. Error message: " << geterror() << endl;
            return 0;
        }
        for(int x = 0; x < W; x++){
            for(int y = 0; y < H; y++){
                (*imageptr)[x+y*W].r = temp[3*x+3*y*W];
                (*imageptr)[x+y*W].g = temp[3*x+3*y*W+1];
                (*imageptr)[x+y*W].b = temp[3*x+3*y*W+2];
                (*imageptr)[x+y*W].a = 255;
            }
        }
    } else {
        temp = new unsigned char[4*W*H];
        if(!input->read_image(TypeDesc::UINT8, temp)){
            cerr << "Error encotunred trying to read theimage file. Error message: " << geterror() << endl;
            return 0;
        }
        for(int x = 0; x < W; x++){
            for(int y = 0; y < H; y++){
                (*imageptr)[x+y*W].r = temp[4*x+4*y*W];
                (*imageptr)[x+y*W].g = temp[4*x+4*y*W+1];
                (*imageptr)[x+y*W].b = temp[4*x+4*y*W+2];
                (*imageptr)[x+y*W].a = temp[4*x+4*y*W+3];
            }
        }
    }


  for(int x = 0; x < W; x++){

    //keep a temporary memory of the column
    pixel *temp = new pixel[H];
    for(int q = 0; q < H; q++){
      temp[q] = (*imageptr)[(H-1-q)*W+x];
    }

    //read the flipped column back to the image
    for(int y = 0; y < H; y++){
      (*imageptr)[y*W+x] = temp[y];
    }
  }


  if(!input->close()){
    cerr << "Could not close " << filename << ", error = " << geterror() << endl;
    return 0;
  }
  return 1;
}

#endif