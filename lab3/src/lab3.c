// Laboration i spelfysik: Biljardbordet
// Av Ingemar Ragnemalm 2010, baserad på material av Tomas Szabo.
// 2012: Ported to OpenGL 3.2 by Justina Mickonytë and Ingemar R.
// 2013: Adapted to VectorUtils3 and MicroGlut.

// gcc lab3.c ../common/*.c -lGL -o lab3 -I../common

// Includes vary a bit with platforms.
// MS Windows needs GLEW or glee.
// For Mac, I used MicroGlut and Lightweight IDE.
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifdef __APPLE__
    #include <OpenGL/gl3.h>
    #include "../../common/mac/MicroGlut.h"
    // uses framework Cocoa
#else
    #include "../../common/Linux/MicroGlut.h" // #include <GL/glut.h>
    #include <GL/gl.h>
#endif

#include <sys/time.h>
#include "../../common/GL_utilities.h"
#include "../../common/VectorUtils3.h"
#include "../../common/loadobj.h"
#include "../../common/LoadTGA.h"
#include "../../common/zpr.h"

// initial width and heights
#define W 600
#define H 600

#define NEAR 1.0
#define FAR 100.0

#define NUM_LIGHTS 1
#define kBallSize 0.1

#define abs(x) (x > 0.0? x: -x)

void onTimer(int value);

static double startTime = 0;

void resetElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  startTime = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 0.000001;
}

float getElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  double currentTime = (double) timeVal.tv_sec
    + (double) timeVal.tv_usec * 0.000001;

  return currentTime - startTime;
}


typedef struct
{
  Model* model;
  GLuint textureId;
} ModelTexturePair;

typedef struct
{
  GLuint tex;
  GLfloat mass;

  vec3 X, P, L; // position, linear momentum, angular momentum
  mat4 R; // Rotation

  vec3 F, T; // accumulated force and torque

  mat3 Ji;
  vec3 omega; // Angular momentum
  vec3 v; // Change in velocity

} Ball;

typedef struct
{
    GLfloat diffColor[4], specColor[4],
    ka, kd, ks, shininess;  // coefficients and specular exponent
} Material;

Material ballMt = { { 1.0, 1.0, 1.0, 1.0 }, { 1.0, 1.0, 1.0, 0.0 },
                    0.1, 0.6, 1.0, 50
                },
        shadowMt = { { 0.0, 0.0, 0.0, 0.5 }, { 0.0, 0.0, 0.0, 0.5 },
                    0.1, 0.6, 1.0, 5.0
                },
        tableMt = { { 0.2, 0.1, 0.0, 1.0 }, { 0.4, 0.2, 0.1, 0.0 },
                    0.1, 0.6, 1.0, 5.0
                },
        tableSurfaceMt = { { 0.1, 0.5, 0.1, 1.0 }, { 0.0, 0.0, 0.0, 0.0 },
                    0.1, 0.6, 1.0, 0.0
                };


enum {kNumBalls = 4}; // Change as desired, max 16

//------------------------------Globals---------------------------------
ModelTexturePair tableAndLegs, tableSurf;
Model *sphere;
Ball ball[16]; // We only use kNumBalls but textures for all 16 are always loaded so they must exist. So don't change here, change above.

GLfloat deltaT, currentTime;

vec3 cam, point;

GLuint shader = 0;
GLint lastw = W, lasth = H;  // for resizing
//-----------------------------matrices------------------------------
mat4 projectionMatrix,
        viewMatrix, rotateMatrix, scaleMatrix, transMatrix, tmpMatrix;

//------------------------- lighting--------------------------------
vec3 lightSourcesColorArr[] = { {1.0f, 1.0f, 1.0f} }; // White light
GLfloat specularExponent[] = {50.0};
GLint directional[] = {0};
vec3 lightSourcesDirectionsPositions[] = { {0.0, 10.0, 0.0} };


//----------------------------------Utility functions-----------------------------------

void loadModelTexturePair(ModelTexturePair* modelTexturePair,
              char* model, char* texture)
{
  modelTexturePair->model = LoadModelPlus(model); // , shader, "in_Position", "in_Normal", "in_TexCoord");
  if (texture)
    LoadTGATextureSimple(texture, &modelTexturePair->textureId);
  else
    modelTexturePair->textureId = 0;
}

void renderModelTexturePair(ModelTexturePair* modelTexturePair)
{
    if(modelTexturePair->textureId)
        glUniform1i(glGetUniformLocation(shader, "objID"), 0);  // use texture
    else
        glUniform1i(glGetUniformLocation(shader, "objID"), 1); // use material color only

    glBindTexture(GL_TEXTURE_2D, modelTexturePair->textureId);
    glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);

    DrawModel(modelTexturePair->model, shader, "in_Position", "in_Normal", NULL);
}

void loadMaterial(Material mt)
{
    glUniform4fv(glGetUniformLocation(shader, "diffColor"), 1, &mt.diffColor[0]);
    glUniform1fv(glGetUniformLocation(shader, "shininess"), 1, &mt.shininess);
}

//---------------------------------- physics update and billiard table rendering ----------------------------------
void updateWorld()
{
    // Zero forces
    int i, j;
    for (i = 0; i < kNumBalls; i++)
    {
        ball[i].F = SetVector(0,0,0);
        ball[i].T = SetVector(0,0,0);
    }

    // Wall tests
    for (i = 0; i < kNumBalls; i++)
    {
        if (ball[i].X.x < -0.82266270 + kBallSize)
            ball[i].P.x = abs(ball[i].P.x);
        if (ball[i].X.x > 0.82266270 - kBallSize)
            ball[i].P.x = -abs(ball[i].P.x);
        if (ball[i].X.z < -1.84146270 + kBallSize)
            ball[i].P.z = abs(ball[i].P.z);
        if (ball[i].X.z > 1.84146270 - kBallSize)
            ball[i].P.z = -abs(ball[i].P.z);
    }

    // Detect collisions, calculate speed differences, apply forces
    float distance;
    vec3 n_hat;
    float v_rel;
    float elastic = 1;
    for (i = 0; i < kNumBalls; i++)
        for (j = i+1; j < kNumBalls; j++)
        {
            // YOUR CODE HERE
            n_hat = VectorSub(ball[j].X, ball[i].X);
            distance = Norm(n_hat);
            n_hat = Normalize(n_hat);
            if ( distance <= 2*kBallSize ) {
                //vec3 v_i = ScalarMult(ball[i].P, 1/ball[i].mass);
                //vec3 v_j = ScalarMult(ball[j].P, 1/ball[j].mass);
                vec3 v_ip = VectorAdd(ball[i].v, CrossProduct(ball[i].omega, ScalarMult(n_hat, kBallSize)));
                vec3 v_jp = VectorAdd(ball[j].v, CrossProduct(ball[j].omega, ScalarMult(n_hat, -kBallSize)));


                v_rel = DotProduct(VectorSub(ball[j].v, ball[i].v), n_hat);
                
                float impulse = -(1.0 + elastic)*v_rel/((1.0/ball[i].mass) + (1.0/ball[j].mass));
                
                //v_i = VectorAdd(v_i, ScalarMult(n_hat, impulse/ball[i].mass));
                //v_j = VectorSub(v_j, ScalarMult(n_hat, impulse/ball[j].mass));
                
                ball[i].F = VectorAdd(ball[i].F, ScalarMult(n_hat, -impulse/deltaT));
                ball[j].F = VectorAdd(ball[j].F, ScalarMult(n_hat, impulse/deltaT));

                ball[i].X = VectorAdd(ball[i].X, ScalarMult(n_hat, -(2*kBallSize-distance)/2-0.05));
                ball[j].X = VectorAdd(ball[j].X, ScalarMult(n_hat, (2*kBallSize-distance)/2+0.05));
            }
        }

    // Control rotation here to reflect
    // friction against floor, simplified as well as more correct
    float epsilon = 0.00001;
    float mu_k = 0.2;
    float mu_r = 0.98;
    for (i = 0; i < kNumBalls; i++)
    {
        // YOUR CODE HERE
        vec3 v_ground = VectorAdd(ball[i].v, CrossProduct(ball[i].omega, (vec3){0,-kBallSize,0}));
        vec3 Ff;

        if ( Norm(v_ground) > 0+epsilon ) {
            Ff = ScalarMult(Normalize(ball[i].v), -mu_k*9.82*ball[i].mass);
        }
        else {
            //Ff = ScalarMult(CrossProduct((vec3){0,-kBallSize,0}, ball[i].L), mu_r);
            Ff = ScalarMult(Normalize(ball[i].v), -mu_r*9.82*ball[i].mass);
        }
        ball[i].T = CrossProduct((vec3){0,-kBallSize,0}, Ff);
    }

// Update state, follows the book closely
    vec3 v;
    for (i = 0; i < kNumBalls; i++)
    {
        vec3 dX, dP, dL, dO;
        mat4 Rd;

        // Note: omega is not set. How do you calculate it?
        // YOUR CODE HERE
//        mat3 R3 = mat4tomat3(ball[i].R);
//        ball[i].omega = MultMat3Vec3(MultMat3(MultMat3(R3, ball[i].Ji), TransposeMat3(R3)), ball[i].L);

        ball[i].omega = MultMat3Vec3(ball[i].Ji, ball[i].L);

//      v := P * 1/mass
        ball[i].v = ScalarMult(ball[i].P, 1.0/(ball[i].mass));
        ball[i].omega = ScalarMult(CrossProduct((vec3){0, kBallSize, 0}, ball[i].v), 1/pow(kBallSize,2));
//      X := X + v*dT
        dX = ScalarMult(ball[i].v, deltaT); // dX := v*dT
        ball[i].X = VectorAdd(ball[i].X, dX); // X := X + dX
//      R := R + Rd*dT
        dO = ScalarMult(ball[i].omega, deltaT); // dO := omega*dT
        Rd = CrossMatrix(dO); // Calc dO, add to R
        Rd = Mult(Rd, ball[i].R); // Rotate the diff (NOTE: This was missing in early versions.)
        ball[i].R = MatrixAdd(ball[i].R, Rd);
//      P := P + F * dT
        dP = ScalarMult(ball[i].F, deltaT); // dP := F*dT
        ball[i].P = VectorAdd(ball[i].P, dP); // P := P + dP
//      L := L + t * dT
        dL = ScalarMult(ball[i].T, deltaT); // dL := T*dT
        ball[i].L = VectorAdd(ball[i].L, dL); // L := L + dL

        OrthoNormalizeMatrix(&ball[i].R);
    }
}

void renderBall(int ballNr)
{
    glBindTexture(GL_TEXTURE_2D, ball[ballNr].tex);

    // Ball with rotation
    transMatrix = T(ball[ballNr].X.x, kBallSize, ball[ballNr].X.z); // position
    tmpMatrix = Mult(transMatrix, ball[ballNr].R); // ball rotation
    tmpMatrix = Mult(viewMatrix, tmpMatrix);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, tmpMatrix.m);
    loadMaterial(ballMt);
    DrawModel(sphere, shader, "in_Position", "in_Normal", NULL);

    // Simple shadow
    glBindTexture(GL_TEXTURE_2D, 0);

    tmpMatrix = S(1.0, 0.0, 1.0);
    tmpMatrix = Mult(tmpMatrix, transMatrix);
    tmpMatrix = Mult(tmpMatrix, ball[ballNr].R);
    tmpMatrix = Mult(viewMatrix, tmpMatrix);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, tmpMatrix.m);
    loadMaterial(shadowMt);
    DrawModel(sphere, shader, "in_Position", "in_Normal", NULL);
}

void renderTable()
{
// Frame and legs, brown, no texture
    loadMaterial(tableMt);
    printError("loading material");
    renderModelTexturePair(&tableAndLegs);

// Table surface (green texture)
    loadMaterial(tableSurfaceMt);
    renderModelTexturePair(&tableSurf);
}
//-------------------------------------------------------------------------------------

void init()
{
    dumpInfo();  // shader info

    // GL inits
    glClearColor(0.1, 0.1, 0.3, 0);
    glClearDepth(1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    printError("GL inits");

    // Load shader
    shader = loadShaders("src/lab3.vert", "src/lab3.frag");
    printError("init shader");

    loadModelTexturePair(&tableAndLegs, "objects/tableandlegsnosurf.obj", 0);
    loadModelTexturePair(&tableSurf, "objects/tablesurf.obj", "objects/surface.tga");
    sphere = LoadModelPlus("objects/sphere.obj");

    projectionMatrix = perspective(90, 1.0, 0.1, 1000); // It would be silly to upload an uninitialized matrix
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);

    char *textureStr = malloc(128);
    int i;
    for(i = 0; i < kNumBalls; i++)
    {
        sprintf(textureStr, "objects/balls/%d.tga", i);
        LoadTGATextureSimple(textureStr, &ball[i].tex);
    }
    free(textureStr);

    // Initialize ball data, positions etc
    for (i = 0; i < kNumBalls; i++)
    {
        ball[i].mass = 1.0;
        ball[i].X = SetVector(0.0, 0.0, 0.0);
        ball[i].P = SetVector(((float)(i % 13))/ 50.0, 0.0, ((float)(i % 15))/50.0);
        ball[i].R = IdentityMatrix();
        ball[i].Ji = (mat3) {
            //80, 0, 0,
            //0, 80, 0,
            //0, 0, 80
            3.0/(1.0*ball[i].mass*pow(kBallSize, 2)), 0.0, 0.0,
            0.0, 3.0/(1.0*ball[i].mass*pow(kBallSize, 2)), 0.0,
            0.0, 0.0, 3.0/(1.0*ball[i].mass*pow(kBallSize, 2))
        };
    }
    ball[0].X = SetVector(0, 0, 0);
    ball[1].X = SetVector(0, 0, 0.5);
    ball[2].X = SetVector(0.0, 0, 1.0);
    ball[3].X = SetVector(0, 0, 1.5);
    ball[0].P = SetVector(1, 0, 1);
    ball[1].P = SetVector(0.3, 0, 0.3);
    ball[2].P = SetVector(0.5, 0, 0.2);
    ball[3].P = SetVector(0, 0, 1.00);

    //vec3 v;
    //for (i = 0; i < kNumBalls; i++ ){
    //    v = ScalarMult(ball[i].P, 1/ball[i].mass);
    //    ball[i].omega = ScalarMult(CrossProduct((vec3){0, kBallSize, 0}, v), 1/pow(kBallSize,2));
    //    ball[i].L = MultMat3Vec3(InvertMat3(ball[i].Ji), ball[i].omega);
    //}

    cam = SetVector(0, 2, 2);
    point = SetVector(0, 0, 0);
    zprInit(&viewMatrix, cam, point);  // camera controls

    resetElapsedTime();
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
    int i;
    // This function is called whenever it is time to render
    //  a new frame; due to the idle()-function below, this
    //  function will get called several times per second
    updateWorld();

    // Clear framebuffer & zbuffer
    glClearColor(0.1, 0.1, 0.3, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    int time = glutGet(GLUT_ELAPSED_TIME);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, viewMatrix.m);

    printError("uploading to shader");

    renderTable();

    for (i = 0; i < kNumBalls; i++)
        renderBall(i);

    printError("rendering");

    glutSwapBuffers();
}

void onTimer(int value)
{
    glutPostRedisplay();
    deltaT = getElapsedTime() - currentTime;
    currentTime = getElapsedTime();
    glutTimerFunc(20, &onTimer, value);
}

void reshape(GLsizei w, GLsizei h)
{
    lastw = w;
    lasth = h;

    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat) w / (GLfloat) h;
    projectionMatrix = perspective(90, ratio, 0.1, 1000);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

//-----------------------------main-----------------------------------------------
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(W, H);
    glutInitContextVersion(3, 2);
    glutCreateWindow ("Biljardbordet");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(20, &onTimer, 0);

    init();

    glutMainLoop();
    exit(0);
}
