#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>

#include <GLUT/glut.h>

extern int argc;
extern char** argv;


class GL : public Screen {
public:
  
  static void myDisplayFunc() {
    // gfx fun
  }
  
  GL() {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    
    glutInitWindowPosition(80, 80);
    glutInitWindowSize(500,500);
    
    glutCreateWindow("A Simple OpenGL Program");
    
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, 500.0, 500.0,0.0 );
    
    glBegin(GL_POINTS);
    glColor3f(1,1,1);
    glVertex2i(100,100);
    glEnd();
    
    glutDisplayFunc(myDisplayFunc);
    glFlush();
    glutMainLoop();
  }
  
  void blit() {
    
  }
};
