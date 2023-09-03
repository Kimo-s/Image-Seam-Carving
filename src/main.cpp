#include "imgui.h"
#include "mainHeader.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed
#endif
#define Square(x) ((x)*(x))

// Our state
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
char *outfile;
bool manualMode = false;


void removeSeam(int position){

  image temp;
  int H = bufferImage.H;
  int W = bufferImage.W;
  temp.H = bufferImage.H;
  temp.W = bufferImage.W - 1;
  temp.channels = bufferImage.channels;

  int curPixel = bufferImage.W/2;
  int *indicesToRemove = new int[H];

  if(position != -1){
    curPixel = position;
  } else {
    for(int x = 1; x < W; x++){
      if(heatMap[x+(H-2)*bufferImage.W] < heatMap[curPixel+(H-2)*bufferImage.W]){
        curPixel = x;
      }
      // printf("current value: %f\n", heatMap[x+(H-2)*bufferImage.W]);
      // fflush(stdout);
    }
  }
  
  // printf("smallets value: %f at %d\n", heatMap[curPixel+(H-2)*bufferImage.W], curPixel);
  // fflush(stdout);
  indicesToRemove[0] = curPixel;

  int yt = 1;
  for(int y = H-2; y >= 1; y--){
    float left, right, mid;
    left = 10000;
    right = 10000;
    mid = heatMap[curPixel+W*(y+1)];
    // printf("y=%d, curpixel = %d\n", y, curPixel);
    if(!(curPixel-1 < 0)){
      left = heatMap[(curPixel-1)+W*(y+1)];
    } 
    if(!(curPixel+1 > W)){
      right = heatMap[(curPixel+1)+W*(y+1)]; 
    }
    
    float minval = std::min(std::min(mid, right), left);
    if(minval == left){
      indicesToRemove[yt] = curPixel-1;
    } else if (minval == right){
      indicesToRemove[yt] = curPixel+1;
    } else if (minval == mid) {
      indicesToRemove[yt] = curPixel;
    }
    curPixel = indicesToRemove[yt];
    yt += 1;
    // printf("left: %f, right: %f, mid: %f, minval: %f\n", left, right, mid, minval);
    // fflush(stdout);
  }

  int tx = 0;
  float* newHeat = new float[H*W-1];
  temp.pixmap = (pixel*)malloc((H*W-1)*sizeof(pixel));
  for(int y = 0; y < H; y++){
    for(int x = 0; x < W; x++){
      if(x == indicesToRemove[(H-1)-y]){
        if(position != -1){
        bufferImage.pixmap[x+y*W].r = 30;
        bufferImage.pixmap[x+y*W].b = 30;
        bufferImage.pixmap[x+y*W].g = 100;
        }
        continue;
      }
      if(position == -1){
        temp.pixmap[tx+y*(W-1)] = bufferImage.pixmap[x+y*W];
        newHeat[tx+y*(W-1)] = heatMap[x+y*W];
      }
      tx += 1;
    }
    tx = 0;
  }

  if(position == -1){
    delete heatMap;
    heatMap = newHeat;
    delete bufferImage.pixmap;
    copyImageToBuffer(temp);
  }

}

image getHeatMap(image* edgeMap){

  image heatmapimage;
  int W = bufferImage.W;
  int H = bufferImage.H;
  heatmapimage.W = W;
  heatmapimage.H = H;
  heatmapimage.channels = bufferImage.channels;
  
  // printf("width: %d height: %d\n", W, H);

  pixel *edgemapArr = edgeMap->pixmap;
  heatMap = new float[W*H];
  

  // Copy last scanline of the image
  // for (int x = 0; x < W; x++){
  //   heatMap[x] = (float)edgemapArr[x].r/255;
  // }

  
  for (int y = 1; y < H; y++){
    for (int x = 0; x < W; x++){
      float left, right, mid;
      left = 0;
      right = 0;
      mid = heatMap[x+W*(y-1)];

      if(!(x-1 < 0)){
        left = heatMap[(x-1)+W*(y-1)];
      }

      if(!(x+1 > W)){
        right = heatMap[(x+1)+W*(y-1)]; 
      }

      float minval = std::min(std::min(mid, right), left);
      heatMap[x+W*y] = minval + (float)edgemapArr[(x)+W*(y-1)].r/255;
      // heatMap[x+W*y] = minval + heatMap[(x)+W*(y-1)]/6.0;
      // printf("%f %f\n", heatMap[x+W*y],minval);
      // fflush(stdout);
    }
  }
  // printf("test\n");
  // fflush(stdout);
  heatmapimage.pixmap = new pixel[W*H];
  float minval = 200000;
  float maxval = -200000;
  for (int y = 0; y < H; y++){
    for (int x = 0; x < W; x++){
      // printf("%f\n", heatMap[x+W*y]);
      // fflush(stdout);
      if(minval > heatMap[x+W*y]){
        minval = heatMap[x+W*y];
      }

      if(maxval < heatMap[x+W*y]){
        maxval = heatMap[x+W*y];
      }
      
    }
  }

  for (int y = 0; y < H; y++){
    for (int x = 0; x < W; x++){
      heatmapimage.pixmap[x+y*W].r = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
      heatmapimage.pixmap[x+y*W].b = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
      heatmapimage.pixmap[x+y*W].g = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
    }
  }
  return heatmapimage;
}

void applySobel(){ 

  filterSize = 3;

  image sobelHorzRes;
  float sobelHorz[3][3]= {{1,0,-1},{2,0,-2},{1,0,-1}};
  sobelHorzRes = applyFilter(sobelHorz);

  image sobelHorzRes2;
  float sobelHorz2[3][3]= {{-1,0,1},{-2,0,2},{-1,0,1}};
  sobelHorzRes2 = applyFilter(sobelHorz2);

  image sobelVertRes;
  float sobelVert[3][3]= {{1,2,1},{0,0,0},{-1,-2,-1}};
  sobelVertRes = applyFilter(sobelVert);

  image sobelVertRes2;
  float sobelVert2[3][3]= {{-1,-2,-1},{0,0,0},{1,2,1}};
  sobelVertRes2 = applyFilter(sobelVert2);

  heatMap = new float[sobelVertRes.H*sobelVertRes.W];
  for (int y = 0; y < sobelVertRes.H; y++){
    for (int x = 0; x < sobelVertRes.W; x++){
      sobelVertRes.pixmap[x+y*sobelVertRes.W].r = sqrt( Square((float)sobelVertRes.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelHorzRes.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelHorzRes2.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelVertRes2.pixmap[x+y*sobelVertRes.W].r));
      sobelVertRes.pixmap[x+y*sobelVertRes.W].b = sqrt( Square((float)sobelVertRes.pixmap[x+y*sobelVertRes.W].b) + Square((float)sobelHorzRes.pixmap[x+y*sobelVertRes.W].b) + Square((float)sobelHorzRes2.pixmap[x+y*sobelVertRes.W].b) + Square((float)sobelVertRes2.pixmap[x+y*sobelVertRes.W].b));
      sobelVertRes.pixmap[x+y*sobelVertRes.W].g = sqrt( Square((float)sobelVertRes.pixmap[x+y*sobelVertRes.W].g) + Square((float)sobelHorzRes.pixmap[x+y*sobelVertRes.W].g) + Square((float)sobelHorzRes2.pixmap[x+y*sobelVertRes.W].g) + Square((float)sobelVertRes2.pixmap[x+y*sobelVertRes.W].g));
      heatMap[x+y*sobelVertRes.W] = sqrt( Square((float)sobelVertRes.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelHorzRes.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelHorzRes2.pixmap[x+y*sobelVertRes.W].r) + Square((float)sobelVertRes2.pixmap[x+y*sobelVertRes.W].r));
    }
  }

  printf("width: %d height: %d\n", sobelVertRes.W, sobelVertRes.H);
  energyImage = sobelVertRes;
  getHeatMap(&sobelVertRes);
  // copyImageToBuffer(getHeatMap());
}


void my_display_code()
{
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Controls"); 

    static int i1 = 0;
    static int seamsToRemove = 1;
    ImGui::SliderInt("slider int", &i1, 0, bufferImage.W);


    if (ImGui::Button("Visualize path")){
      removeSeam(i1);
    }

    if (ImGui::Button("Apply Sobel")){ 
      applySobel();
    }

    if (ImGui::Button("Show energy map")){
      copyImageToBuffer(energyImage);
    }

    if (ImGui::Button("Show calcuated heat map")){
      image temp;
      temp.H = bufferImage.H;
      temp.W = bufferImage.W;
      int W = temp.W;
      int H = temp.H;
      temp.channels = bufferImage.channels;

      temp.pixmap = new pixel[temp.H*temp.W];
      float minval = 200000.0f;
      float maxval = -200000.0f;
      for (int y = 0; y < temp.H; y++){
        for (int x = 0; x < temp.W; x++){
          if(minval > heatMap[x+W*y]){
            minval = heatMap[x+W*y];
          }

          if(maxval < heatMap[x+W*y]){
            maxval = heatMap[x+W*y];
          }
        }
      }
      for (int y = 0; y < H; y++){
        for (int x = 0; x < W; x++){
          temp.pixmap[x+y*W].r = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
          temp.pixmap[x+y*W].b = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
          temp.pixmap[x+y*W].g = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
        }
      }
      copyImageToBuffer(temp);
    }

    if (ImGui::Button("Reset to original image")){
      copyImageToBuffer(originalImage);
    }
    

    ImGui::SliderInt("Seams to remove", &seamsToRemove, 1, 40);
    if (ImGui::Button("Remove seam")){
      for(int i = 0; i < seamsToRemove; i++){
        removeSeam(-1);
      }
    }

    if (ImGui::Button("Setup energy map manually")){
      heatMap = new float[bufferImage.H*bufferImage.W];
      for (int y = 0; y < bufferImage.H; y++){
        for (int x = 0; x < bufferImage.W; x++){
          heatMap[x+bufferImage.W*y] = 300;
        }
      }
      manualMode = true;
    }

    ImGui::SameLine();
    if(manualMode){
      ImGui::Text("Manual mode is on");
      if (ImGui::Button("End selection mode")){
        image temp;
        temp.H = bufferImage.H;
        temp.W = bufferImage.W;
        int W = temp.W;
        int H = temp.H;
        temp.channels = bufferImage.channels;

        temp.pixmap = new pixel[temp.H*temp.W];
        float minval = 200000.0f;
        float maxval = -200000.0f;
        for (int y = 0; y < temp.H; y++){
          for (int x = 0; x < temp.W; x++){
            if(minval > heatMap[x+W*y]){
              minval = heatMap[x+W*y];
            }

            if(maxval < heatMap[x+W*y]){
              maxval = heatMap[x+W*y];
            }
          }
        }
        for (int y = 0; y < H; y++){
          for (int x = 0; x < W; x++){
            temp.pixmap[x+y*W].r = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
            temp.pixmap[x+y*W].b = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
            temp.pixmap[x+y*W].g = 255*(heatMap[x+W*y]-minval)/(maxval-minval);
          }
        }
        getHeatMap(&temp);
        manualMode = false;
        copyImageToBuffer(originalImage);
      }
    }

    // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void handleMouse(int button, int state, int x, int y){
  ImGui_ImplGLUT_MouseFunc(button, state, x, y);

  // printf("location x=%d y=%d, image width %d\n", x , y, bufferImage.W);
  if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && manualMode){
    int squareWidth = 40;
    for (int yt = -squareWidth/2; yt < squareWidth/2; yt++){
      for (int xt = -squareWidth/2; xt < squareWidth/2; xt++){
        if(xt+x > bufferImage.W-1 || xt+x < 0 || yt+(bufferImage.H-y) > bufferImage.H-1 || yt+(bufferImage.H-y) < 0){
          continue;
        }
        heatMap[(xt+x)+bufferImage.W*(yt+(bufferImage.H-y))] = -1000;
        bufferImage.pixmap[(xt+x)+bufferImage.W*(yt+(bufferImage.H-y))].r += 30;
        bufferImage.pixmap[(xt+x)+bufferImage.W*(yt+(bufferImage.H-y))].b += 40;
        bufferImage.pixmap[(xt+x)+bufferImage.W*(yt+(bufferImage.H-y))].g += 50;
      }
    }
  }
}

void glut_display_func()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    my_display_code();

    // Rendering
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    // glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);


    // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0,0);

    glDrawPixels(bufferImage.W, bufferImage.H, GL_RGBA, GL_UNSIGNED_BYTE, bufferImage.pixmap);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();
    glutPostRedisplay();
}

void handleReshape(int w, int h){
  // glutReshapeWindow(bufferImage.W, bufferImage.H);
  // if(h != bufferImage.H){
  //   glutReshapeWindow(bufferImage.W, bufferImage.H);
  // }
  glViewport(0, 0, w, h);

  // define the drawing coordinate system on the viewport
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  ImGui_ImplGLUT_ReshapeFunc(w,h);
}

void parser(int argSize, char** argv){

  int i;
  int n = 0;
  if(argSize < 2){
    std::cout << "Not enough arguments supplied. Please supply at least the name of the image file and optionally the output file name." << endl;
    exit(0);
  }
  for(i = 1; i < argSize; i++){
    if(n == 0){
      readImage(argv[i], &originalImage.pixmap, &originalImage.W, &originalImage.H, &originalImage.channels);
      copyImageTo(&originalImage, &bufferImage);

    } else if (n == 1){
      outfile = argv[i];
    }
    n += 1;
    
  }
}

int main(int argc, char** argv)
{
    // Create GLUT window
    glutInit(&argc, argv);
#ifdef __FREEGLUT_EXT_H__
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutCreateWindow("Dear ImGui GLUT+OpenGL2 Example");

    parser(argc, argv);

    glutInitWindowSize(bufferImage.W, bufferImage.H);

    // Setup functions
    ImGui_ImplGLUT_InstallFuncs();
    glutDisplayFunc(glut_display_func);
    glutReshapeFunc(handleReshape);
    glutMouseFunc(handleMouse);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL2_Init();

    // initial setup
    applySobel();

    glutMainLoop();

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
