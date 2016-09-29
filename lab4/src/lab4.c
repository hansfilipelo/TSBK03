// Demo of heavily simplified sprite engine
// by Ingemar Ragnemalm 2009
// used as base for lab 4 in TSBK03.
// OpenGL 3 conversion 2013.

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include "../../common/mac/MicroGlut.h"
// uses framework Cocoa
#else
#include <GL/gl.h>
#include "../../common/Linux/MicroGlut.h"
#endif

#include <stdlib.h>
#include "../../common/LoadTGA.h"
#include "SpriteLight.h"
#include "../../common/GL_utilities.h"

// Lägg till egna globaler här efter behov.

float maxDistanceSq = 20000;
float cohesionWeight = 0.002;
float alignmentWeight = 0.01;
float avoidanceWeight = 0.5;

FPoint calculateAvoidance(SpritePtr i, SpritePtr j){
    FPoint avoidance;
    avoidance.h = i->position.h - j->position.h;
    avoidance.v = i->position.v - j->position.v;

    float distance = sqrt(pow(avoidance.h, 2) + pow(avoidance.v, 2));

    avoidance.h *= (1-distance/sqrt(maxDistanceSq))/distance;
    avoidance.v *= (1-distance/sqrt(maxDistanceSq))/distance;

    return avoidance;
}

void SpriteBehavior() // Din kod!
{
    // Lägg till din labbkod här. Det går bra att ändra var som helst i
    // koden i övrigt, men mycket kan samlas här. Du kan utgå från den
    // globala listroten, gSpriteRoot, för att kontrollera alla sprites
    // hastigheter och positioner, eller arbeta från egna globaler.

    SpritePtr i = gSpriteRoot;
    SpritePtr j;

    while (i != NULL){
        unsigned count = 0;
        j = gSpriteRoot;
        i->speedDiff = (FPoint){0,0};
        i->averagePosition = (FPoint){0,0};
        i->avoidanceVector = (FPoint){0,0};
        i->speedSetter = (FPoint){0,0};

        while (j != NULL){
            if (i != j && pow(j->position.h - i->position.h, 2) + pow(j->position.v - i->position.v, 2) < maxDistanceSq ){
                i->averagePosition.h += j->position.h;
                i->averagePosition.v += j->position.v;
                
                i->speedDiff.h += j->speed.h - i->speed.h;
                i->speedDiff.v += j->speed.v - i->speed.v;
                
                FPoint avoidance = calculateAvoidance(i, j);
                i->avoidanceVector.h += avoidance.h;
                i->avoidanceVector.v += avoidance.v;
                
                count += 1;
            }
            j = j->next;
        }

        if (count != 0 ){
            i->averagePosition.h /= count;
            i->averagePosition.v /= count;
            i->speedDiff.h /= count;
            i->speedDiff.v /= count;
            i->avoidanceVector.h /= count;
            i->avoidanceVector.v /= count;
            i->speedSetter.h = (i->averagePosition.h - i->position.h)*cohesionWeight 
                + i->speedDiff.h*alignmentWeight
                + i->avoidanceVector.h*avoidanceWeight;
            i->speedSetter.v = (i->averagePosition.v - i->position.v)*cohesionWeight 
                + i->speedDiff.v*alignmentWeight
                + i->avoidanceVector.v*avoidanceWeight;
        }
        i = i->next;
    }

    i = gSpriteRoot;
    while ( i != NULL ){
        //printf("%f\n", i->averagePosition.h*cohesionWeight);
        i->speed.h += i->speedSetter.h;
        i->speed.v += i->speedSetter.v;
        i = i->next;
    }


}

// Drawing routine
void Display()
{
    SpritePtr sp;

    glClearColor(0, 0, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawBackground();

    SpriteBehavior(); // Din kod!

    // Loop though all sprites. (Several loops in real engine.)
    sp = gSpriteRoot;
    do
    {
        HandleSprite(sp); // Callback in a real engine
        DrawSprite(sp);
        sp = sp->next;
    } while (sp != NULL);

    glutSwapBuffers();
}

void Reshape(int h, int v)
{
    glViewport(0, 0, h, v);
    gWidth = h;
    gHeight = v;
}

void Timer(int value)
{
    glutTimerFunc(20, Timer, 0);
    glutPostRedisplay();
}

// Example of user controllable parameter
float someValue = 0.0;

void Key(unsigned char key,
        __attribute__((unused)) int x,
        __attribute__((unused)) int y)
{
    switch (key)
    {
        case '+':
            cohesionWeight += 0.0001;
            printf("cohesionWeight = %f\n", cohesionWeight);
            break;
        case '-':
            cohesionWeight -= 0.0001;
            printf("cohesionWeight = %f\n", cohesionWeight);
            break;
        case 0x1b:
            exit(0);
    }
}

void Init()
{
    TextureData *sheepFace, *blackFace, *dogFace, *foodFace;

    LoadTGATextureSimple("img/leaves.tga", &backgroundTexID); // Bakgrund

    sheepFace = GetFace("img/sheep.tga"); // Ett får
    blackFace = GetFace("img/blackie.tga"); // Ett svart får
    dogFace = GetFace("img/dog.tga"); // En hund
    foodFace = GetFace("img/mat.tga"); // Mat

    NewSprite(sheepFace, 100, 200, 1, 1);
    NewSprite(sheepFace, 200, 100, 1.5, -1);
    NewSprite(sheepFace, 250, 200, -1, 1.5);
    NewSprite(sheepFace, 200, 200, -1, 1.5);
    NewSprite(sheepFace, 100, 100, 1, 1.5);
    NewSprite(sheepFace, 500, 200, -1, 1.5);
    NewSprite(sheepFace, 200, 300, -1, 1.5);
    NewSprite(sheepFace, 100, 300, -1, 1.5);
    NewSprite(sheepFace, 300, 100, -1, 1.5);

}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutInitContextVersion(3, 2);
    glutCreateWindow("SpriteLight demo / Flocking");

    glutDisplayFunc(Display);
    glutTimerFunc(20, Timer, 0); // Should match the screen synch
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);

    InitSpriteLight();
    Init();

    glutMainLoop();
    return 0;
}
