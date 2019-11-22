#define GLUT_DISABLE_ATEXIT_HACK
#include <gl/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdio>
#include "pila.h"
#include <unistd.h>
#include <cstring>
#define HEIGHT 600
#define WIDTH 600
#define ERASER_SIZE 10
#define SPRAY_POINTS 15
#define RAND_RANGE 40
#define TIMER_MOUSE 0
#define SQUARE_COLOR_SIZE 25
#define ROUTE "img.bmp"

typedef struct Pixel{
    unsigned char r,g,b;
}Pixel;

typedef struct{
    float x;
    float y;
    float xi;
    float yi;
} Coord;

typedef struct BMP
{
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;					//(4 Bytes) Reservado
	int offset;						//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;			//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int ancho;						//(4 Bytes) Ancho (numero de píxeles horizontales)
	int alto;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;			//(2 Bytes) Numero de planos de color
	short int profundidadColor;		//(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;				//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;			//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;				//(4 Bytes) Cantidad de colores usados
	int coloresImportantes;			//(4 Bytes) Cantidad de colores importantes
	Pixel **pixel; 			//Puntero a una tabla dinamica de caracteres de 2 dimenciones almacenara el valor del pixel en escala de gris (0-255)
}BMP;


unsigned char actualArrayColor[3];
float totalWindowArray[WIDTH*HEIGHT];
float* trimmedWindow;
unsigned int widthTrimmedArea;
unsigned int heightTrimmedArea;

typedef enum {FIRST, SECOND} PHASE;
typedef enum {FALSE,TRUE} BOOLEAN;
typedef enum {PENCIL, ERASER, SPRAY, PAINT_BOTTLE, TRIM} TOOL;
typedef enum {  BLACK,  WHITE,  RED,    GREEN,  BLUE,   YELLOW, ORANGE, PURPLE,
                CYAN,   PINK,   GRAY,   DGREEN, DBLUE,  BROWN,  DRED,   LYELLOW} COLOR;


BOOLEAN firstTime;
Coord coord;
TOOL tool;
BOOLEAN isClickPressed;
COLOR actualColor;
PHASE actualTrimPhase;
float xT, yT;
BOOLEAN firstTimeTrim;

void init(void);
void selectTool(TOOL tool);
void putPixel(int x, int y);
void setColor(int color);
void display(void);
void onMotion(int x,int y);
void onMouse(int button, int state, int x, int y);
void keyPressed (unsigned char key, int x, int y);
void printPencil();
void printEraser();
void printSpray();
void clearScreen();
void lineaBres(int x0, int y0, int xEnd, int yEnd);
void clickPressed(int timer);
void useBottle(void);
void leePixel(int x, int y, unsigned char *p);
int compara(unsigned char *a, unsigned char *b);
void paintBottle(int x, int y, unsigned char *otherColor, unsigned char *actualColor);
void makeGUI();
void changeColor(int x, int y);
void saveBMP(char *route);
void loadBMP( int number);
void text(int x, int y, char *word);
void useTrim();
void paintTrimmedArea();

int main(int argc, char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB );
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Paint a fish!");
    init();
    setColor(actualColor);
    glutDisplayFunc(clearScreen);
    glutDisplayFunc(display);
    glutMouseFunc(onMouse);
    glutMotionFunc(onMotion);
    glutKeyboardFunc(keyPressed);
    glutMainLoop();
    return 0;
}

void init(void){
   //glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0,1.0,1.0,0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, 0.0,HEIGHT);
    coord.x = 0;
    coord.y = 0;
    coord.yi = 0;
    coord.xi = 0;
    xT = yT = 0;
    tool = PENCIL;
    firstTime = TRUE;
    isClickPressed = FALSE;
    actualColor = BLACK;
    actualTrimPhase = FIRST;
    trimmedWindow = NULL;
    glFlush();
    glutSwapBuffers();
}

void selectTool(TOOL tool){
    switch(tool){
        case PENCIL:
            printPencil();
            break;
        case ERASER:
            printEraser();
            break;
        case SPRAY:
            printSpray();
            clickPressed(TIMER_MOUSE);
            break;
        case PAINT_BOTTLE:
            useBottle();
            break;
        case TRIM:
            useTrim();
            break;
    }
}

void putPixel(int x, int y){
        glBegin(GL_POINTS);
        glVertex2i(x,y);
        glEnd();
}

void setColor(int color){
    float red,green,blue;
    red=0;
    green=0;
    blue=0;
    actualArrayColor[0]=0;
    actualArrayColor[1]=0;
    actualArrayColor[2]=0;
    switch(color){
        case RED:
            red=1;
            actualArrayColor[0]=255;
            break;
        case GREEN:
            green=1;
            actualArrayColor[1]=255;
            break;
        case BLUE:
            blue=1;
            actualArrayColor[2]=255;
            break;
        case BLACK:
            break;
        case WHITE:
            blue=1;
            red=1;
            green=1;
            actualArrayColor[0]=255;
            actualArrayColor[1]=255;
            actualArrayColor[2]=255;
            break;
        case YELLOW:
            red=1;
            green=1;
            actualArrayColor[0]=255;
            actualArrayColor[1]=255;
            break;
        case ORANGE:
            red=1;
            green=.5;
            actualArrayColor[0]=255;
            actualArrayColor[1]=128;
            break;
        case PURPLE:
            red=.5;
            blue=1;
            actualArrayColor[0]=128;
            actualArrayColor[2]=255;
            break;
        case CYAN:
            green=1;
            blue=1;
            actualArrayColor[1]=255;
            actualArrayColor[2]=255;
            break;
        case PINK:
            red=1;
            blue=1;
            actualArrayColor[0]=255;
            actualArrayColor[2]=255;
            break;
        case GRAY:
            blue=.5;
            red=.5;
            green=.5;
            actualArrayColor[0]=128;
            actualArrayColor[1]=128;
            actualArrayColor[2]=128;
            break;
        case DGREEN:
            green=.5;
            actualArrayColor[1]=128;
            break;
        case DBLUE:
            blue=.5;
            actualArrayColor[3]=128;
            break;
        case BROWN:
            red=.5;
            green=.128;
            actualArrayColor[0]=128;
            actualArrayColor[1]=64;
            break;
        case DRED:
            red=.5;
            actualArrayColor[0]=128;
            break;
        case LYELLOW:
            red=1;
            green=1;
            blue=.5;
            actualArrayColor[0]=255;
            actualArrayColor[1]=255;
            actualArrayColor[2]=128;
            break;
        default:
            break;
    }
    glColor3f(red,green,blue);
}

void useTrim(){
    if(actualTrimPhase == FIRST){

        float temp;
        widthTrimmedArea = (int)abs(coord.x-coord.xi);
        heightTrimmedArea = (int)abs(coord.y-coord.yi);
        unsigned int size_area = widthTrimmedArea*heightTrimmedArea;
        if(size_area > 0){
            free(trimmedWindow);
            trimmedWindow = (float*)malloc(size_area*sizeof(float));
            if(coord.x < coord.xi && coord.yi < coord.y){
                xT = coord.x;
                yT = coord.yi;
                glReadPixels((int)coord.x,(int)coord.yi,
                         widthTrimmedArea,heightTrimmedArea,
                         GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
            }
            else if(coord.x < coord.xi && coord.y < coord.yi){
                xT = coord.x;
                yT = coord.y;
                glReadPixels((int)coord.x,(int)coord.y,
                         widthTrimmedArea,heightTrimmedArea,
                         GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
            }
            else if(coord.xi < coord.x && coord.y < coord.yi){
                xT = coord.xi;
                yT = coord.y;
                glReadPixels((int)coord.xi,(int)coord.y,
                         widthTrimmedArea,heightTrimmedArea,
                         GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
            }
            else {
                xT = coord.xi;
                yT = coord.yi;
                glReadPixels((int)coord.xi,(int)coord.yi,
                         widthTrimmedArea,heightTrimmedArea,
                         GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
            }
            firstTimeTrim = TRUE;
        }
        else{
            free(trimmedWindow);
            actualTrimPhase = FIRST;
        }
    }
    else if(actualTrimPhase == SECOND){
        if(firstTimeTrim == TRUE){
            if(coord.xi > xT && coord.yi > yT && coord.xi < (xT + widthTrimmedArea) && coord.yi< (yT+heightTrimmedArea))
                firstTimeTrim = FALSE;
            else{
                free(trimmedWindow);
                actualTrimPhase = FIRST;
            }
        }
        if(actualTrimPhase == SECOND){
            xT = coord.xi-widthTrimmedArea/2;
            yT = coord.yi-heightTrimmedArea/2;
            glRasterPos2f(0,0);
            glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,totalWindowArray);
            glutSwapBuffers();
            glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,totalWindowArray);

            glRasterPos2f(xT, yT);
            glDrawPixels(widthTrimmedArea,heightTrimmedArea,GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
            glutSwapBuffers();
            glDrawPixels(widthTrimmedArea,heightTrimmedArea,GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
        }
    }
}

void paintTrimmedArea(){
    unsigned int width = (int)abs(coord.x-coord.xi);
    unsigned int height = (int)abs(coord.y-coord.yi);
    setColor(WHITE);
    glBegin(GL_QUAD_STRIP);
    glVertex2i((int)coord.xi,(int)coord.yi);
    glVertex2i((int)coord.x,(int)coord.yi);
    glVertex2i((int)coord.xi,(int)coord.y);
    glVertex2i((int)coord.x,(int)coord.y);
    glEnd();
    glutSwapBuffers();
    glBegin(GL_QUAD_STRIP);
    glVertex2i((int)coord.xi,(int)coord.yi);
    glVertex2i((int)coord.x,(int)coord.yi);
    glVertex2i((int)coord.xi,(int)coord.y);
    glVertex2i((int)coord.x,(int)coord.y);
    glEnd();

    glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,totalWindowArray);

    glRasterPos2f(xT,yT);
    glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);
    glutSwapBuffers();
    glRasterPos2f(xT,yT);
    glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,trimmedWindow);

}

void display(void){
    if(firstTime){
        clearScreen();
        firstTime = FALSE;
    }
    if(!(coord.yi < SQUARE_COLOR_SIZE*2)){
        glClearColor(1.0,1.0,1.0,0);
        selectTool(tool);
        glutSwapBuffers();
    }
    makeGUI();
}

void onMotion(int x,int y){
    if(tool != TRIM){
        coord.xi = coord.x;
        coord.yi = coord.y;
        coord.x = x;
        coord.y = abs((float)HEIGHT-y);
    }
    else if (tool == TRIM && actualTrimPhase == FIRST){
        coord.x = x;
        coord.y = abs((float)HEIGHT-y);
    }
    else if(tool == TRIM && actualTrimPhase == SECOND){
        coord.xi = coord.x = x;
        coord.yi = coord.y = abs((float)HEIGHT-y);
    }
    glutPostRedisplay();
}


void onMouse(int button, int state, int x, int y){
    switch(button){
        case GLUT_LEFT_BUTTON:
            if(state == GLUT_DOWN){
                isClickPressed = TRUE;
                if(tool != TRIM){
                    coord.xi = coord.x = x;
                    coord.yi = coord.y = abs((float)HEIGHT-y);
                }
                else if(tool == TRIM && actualTrimPhase == FIRST){
                    coord.xi = coord.x = x;
                    coord.yi = coord.y = abs((float)HEIGHT-y);
                    glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,totalWindowArray);
                }
                else {
                    coord.xi = coord.x = x;
                    coord.yi = coord.y = abs((float)HEIGHT-y);
                }
                glutPostRedisplay();
            }
            else {
                isClickPressed = FALSE;
                if(tool == TRIM && actualTrimPhase == FIRST){
                    actualTrimPhase = SECOND;
                    paintTrimmedArea();
                }
                else{
                    actualTrimPhase = FIRST;
                }
            }
            break;
        case GLUT_RIGHT_BUTTON:
            coord.xi = coord.x = x;
            coord.yi = coord.y = abs((float)HEIGHT-y);
            changeColor(x, y);
            break;
    }
}

void keyPressed (unsigned char key, int x, int y) {

    switch(key)
    {
        case 'p':
        case 'P':
            tool = PENCIL;
            break;
        case 'e':
        case 'E':
            tool = ERASER;
            break;
        case 's':
        case 'S':
            tool = SPRAY;
            break;
        case 'b':
        case 'B':
            tool = PAINT_BOTTLE;
            break;
        case 'g':
        case 'G':
            char filename[FILENAME_MAX];
            cout << "Set the image's name: ";
            cin.getline(filename,FILENAME_MAX);
            if(filename[0]=='\0')
                strcpy(filename,ROUTE);
            else{
                int i = 0;
                while (filename[i]!='\0')
                    i++;
                filename[i++]='.';
                filename[i++]='b';
                filename[i++]='m';
                filename[i++]='p';
                filename[i++]='\0';
            }
            saveBMP(filename);
            system("cls");
            break;
        case 't':
        case 'T':
            tool = TRIM;
            actualTrimPhase = FIRST;
            break;
        case '1':
            loadBMP(1);
            makeGUI();
            break;
        case '2':
            loadBMP(2);
            makeGUI();
            break;
        case '3':
            loadBMP(3);
            makeGUI();
            break;
    }
}

void printPencil(){
    for(int i = 1 ; i <= 2 ; i++){
        lineaBres((int)coord.xi,
                  (int)coord.yi,
                  (int)coord.x,
                  (int)coord.y);
        glutSwapBuffers();
    }
}

void printEraser(){
    setColor(WHITE);
    glBegin(GL_QUAD_STRIP);
            glVertex2i((int)coord.xi-ERASER_SIZE,(int)coord.yi+ERASER_SIZE);
            glVertex2i((int)coord.xi-ERASER_SIZE,(int)coord.yi-ERASER_SIZE);
            glVertex2i((int)coord.xi+ERASER_SIZE,(int)coord.yi+ERASER_SIZE);
            glVertex2i((int)coord.xi+ERASER_SIZE,(int)coord.yi-ERASER_SIZE);
    glEnd();
    glutSwapBuffers();
    glBegin(GL_QUAD_STRIP);
            glVertex2i((int)coord.xi-10,(int)coord.yi+10);
            glVertex2i((int)coord.xi-10,(int)coord.yi-10);
            glVertex2i((int)coord.xi+10,(int)coord.yi+10);
            glVertex2i((int)coord.xi+10,(int)coord.yi-10);
    glEnd();
    setColor(actualColor);
}

void printSpray(){
    int points[2][SPRAY_POINTS];
    int i, j;
    srand(time(NULL));
    for( i = 0 ; i < SPRAY_POINTS ; i++){
        points[0][i] = rand()%RAND_RANGE + 1 + (int)coord.x;
        points[1][i] = rand()%RAND_RANGE + 1 + (int)coord.y;
    }
    for( i = 1 ; i <= 2 ; i++){
        for( j = 0 ; j < SPRAY_POINTS ; j++)
                putPixel(points[0][j],points[1][j]);
                glutSwapBuffers();
    }

    srand(time(NULL));
}

void clearScreen(){
    int i, j, k;
    setColor(WHITE);
    for(k=1;k<=2;k++){
        glBegin(GL_POINTS);
        for(j = 0 ; j < WIDTH ; j++){
            for(i = 0 ; i < HEIGHT ; i++){
                glVertex2i( j, i);
            }
        }
        glEnd();
        glutSwapBuffers();
    }
    setColor(actualColor);
}

void lineaBres(int x0, int y0, int xEnd, int yEnd){
    int p , dosDy , dosDymenosDx , dx, dy, x, y, i, pasox ,pasoy;
    pasox = 1;
    pasoy = 1;
    dx = xEnd - x0;
    dy = yEnd - y0;
    if( dx < 0 )
        dx = -dx;
    if( dy < 0 )
        dy = -dy;
    if( xEnd < x0 )
        pasox = -1;
    if( yEnd < y0 )
        pasoy = -1;
    x = x0;
    y = y0;

    if( dx > dy ){
        putPixel(x,y);
        p = 2 * dy - dx;
        dosDymenosDx = 2 * ( dy - dx );
        dosDy = 2 * dy;
        for( i = 0; i < dx; i++ ){
            if( p >= 0 ){
                y += pasoy;
                p += dosDymenosDx;
            }
            else
                p += dosDy;
            x += pasox;
            putPixel(x,y);
        }
    }
    else{
        putPixel(x,y);
        p = 2*dx - dy;
        dosDymenosDx = 2 * ( dx - dy );
        dosDy = 2*dx;
        for( i = 0; i < dy; i++ ){
            if( p >= 0 ){
                x += pasox;
                p += dosDymenosDx;
            }
            else
                p += dosDy;
            y += pasoy;
            putPixel(x,y);
        }
    }
}

void clickPressed(int timer){
    if(isClickPressed){
        printSpray();
        glutTimerFunc(30, clickPressed, TIMER_MOUSE);
    }
}

void useBottle(){
    unsigned char otherColor[3];
    leePixel((int)coord.xi,(int)coord.yi,otherColor);
    paintBottle((int)coord.xi,(int)coord.yi,otherColor,actualArrayColor);
}

void leePixel(int x, int y, unsigned char *p){
    glReadPixels(x,y,1,1,GL_RGB,GL_UNSIGNED_BYTE,p);
}
int compara(unsigned char *a, unsigned char *b){
    return (a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2]);
}

void paintBottle(int x, int y, unsigned char *otherColor, unsigned char *actualColor)
{
    int n=0;
    unsigned char color_actual[3];
    Pila *p = (Pila *)malloc(sizeof(Pila));
    Punto punto;
    inicializaPila(p);
    insertar(p,x,y,-1);
    while(!vacia(*p)){
        punto = pop(p);
        leePixel(punto.x,punto.y, color_actual);
        if( compara(color_actual,otherColor) && !compara(color_actual,actualColor)){
            putPixel(punto.x, punto.y);
            glutSwapBuffers();
            putPixel(punto.x, punto.y);
            glutSwapBuffers();
            n++;
            if(n==500){
                glFlush();
                n=0;
            }
            if(punto.direccion == -1){
                if(punto.x+1<WIDTH){
                    insertar(p,punto.x+1,punto.y,0);
                }
                if(punto.x-1>=0){
                    insertar(p,punto.x-1,punto.y,1);
                }
                if(punto.y+1<HEIGHT){
                    insertar(p,punto.x,punto.y+1,2);
                }
                if(punto.y-1>=0){
                    insertar(p,punto.x,punto.y-1,3);
                }
            }else if(punto.direccion == 0){
                if(punto.x+1<WIDTH){
                    insertar(p,punto.x+1,punto.y,0);
                }
                if(punto.y+1<HEIGHT){
                    insertar(p,punto.x,punto.y+1,2);
                }
                if(punto.y-1>=0){
                    insertar(p,punto.x,punto.y-1,3);
                }
            }else if(punto.direccion == 1){
                if(punto.x-1>=0){
                    insertar(p,punto.x-1,punto.y,1);
                }
                if(punto.y+1<WIDTH){
                    insertar(p,punto.x,punto.y+1,2);
                }
                if(punto.y-1>=0){
                    insertar(p,punto.x,punto.y-1,3);
                }
            }else if(punto.direccion == 2){
                if(punto.x+1<HEIGHT){
                    insertar(p,punto.x+1,punto.y,0);
                }
                if(punto.x-1>=0){
                    insertar(p,punto.x-1,punto.y,1);
                }
                if(punto.y+1<WIDTH){
                    insertar(p,punto.x,punto.y+1,2);
                }
            }else if(punto.direccion == 3){
                if(punto.x+1<WIDTH){
                    insertar(p,punto.x+1,punto.y,0);
                }
                if(punto.x-1>=0){
                    insertar(p,punto.x-1,punto.y,1);
                }
                if(punto.y-1>=0){
                    insertar(p,punto.x,punto.y-1,3);
                }
            }
        }
    }
}

void makeGUI(){
    for(int j = 0 ; j < 2 ; j++ ){
        for(int i = 0 ; i < 8; i++){
            int color = (8*j)+i;
            setColor(color);
            glBegin(GL_QUAD_STRIP);
            glVertex2i(i*SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE);
            glEnd();
            glFlush();
            glutSwapBuffers();
            glBegin(GL_QUAD_STRIP);
            glVertex2i(i*SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE);
            glVertex2i(i*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE,j*SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE);
            glEnd();
        }
    }

    setColor(BLACK);
    glBegin(GL_LINE_STRIP);
    glVertex2i(SQUARE_COLOR_SIZE*8,SQUARE_COLOR_SIZE*2);
    glVertex2i(WIDTH,SQUARE_COLOR_SIZE*2-1);
    glEnd();
    glFlush();
    glutSwapBuffers();
    glBegin(GL_LINE_STRIP);
    glVertex2i(SQUARE_COLOR_SIZE*8,SQUARE_COLOR_SIZE*2);
    glVertex2i(WIDTH,SQUARE_COLOR_SIZE*2-1);
    glEnd();

    text((int)(WIDTH-WIDTH/1.6),SQUARE_COLOR_SIZE+SQUARE_COLOR_SIZE/2,"Press: (P): Pencil   (E): Eraser   (S): Spray   (B): Flood Fill   (T): Trim");
    text((int)(WIDTH-WIDTH/1.6),SQUARE_COLOR_SIZE-SQUARE_COLOR_SIZE/2,"(RClick): Select Color    (G): Save   (1): Image 1   (2): Image 2   (3): Image 3");
    setColor(actualColor);
}

void changeColor(int x, int y){
    y = (int)abs(HEIGHT - y);
    int color;
    if( y < SQUARE_COLOR_SIZE*2 && x < SQUARE_COLOR_SIZE*8){
        if(y > SQUARE_COLOR_SIZE )
            color = 15;
        else
            color = 7;
        for(int i = 7 ; i >= 0; i --)
            if(x < SQUARE_COLOR_SIZE*i)
                color--;
        actualColor = (COLOR)color;
        setColor(color);
    }
}

void saveBMP(char *route){

    unsigned char windowArray[WIDTH][HEIGHT-SQUARE_COLOR_SIZE*2][3];

    int tam = (WIDTH*(HEIGHT-SQUARE_COLOR_SIZE*2)*sizeof(Pixel))+0x36+0x02,
        reservado = 0x00,
        offset = 0x36,
        metadatos = 0x28,
        ancho = WIDTH,
        alto = (HEIGHT-SQUARE_COLOR_SIZE*2),
        planos = 1,
        profundidad = 24,
        compresion = 0,
        tamPaleta = 0,
        pxmh = 0,
        pxmv = 0,
        coloresUsados = 0,
        coloresImportantes = 0,
        w,h;

    FILE *archivo = fopen( route, "wb+" );

    glReadPixels(0,(SQUARE_COLOR_SIZE*2),WIDTH,(HEIGHT-SQUARE_COLOR_SIZE*2),GL_RGB,GL_UNSIGNED_BYTE,windowArray);

    fseek( archivo,0, SEEK_SET);
	fwrite("BM",sizeof(char),2, archivo);//tipo
	fwrite(&tam,sizeof(int),1, archivo);//tam
	fwrite(&reservado,sizeof(int),1, archivo);//reservado
	fwrite(&offset,sizeof(int),1, archivo);//offset
	fwrite(&metadatos,sizeof(int),1, archivo);//tam metadatos
	fwrite(&ancho,sizeof(int),1, archivo);//ancho
	fwrite(&alto,sizeof(int),1, archivo);//alto
	fwrite(&planos,sizeof(short int),1, archivo);//planos
	fwrite(&profundidad,sizeof(short int),1, archivo);//profundidad (bits)
	fwrite(&compresion,sizeof(int),1, archivo);//compresion
	fwrite(&tamPaleta,sizeof(int),1, archivo);//tamPaleta
	fwrite(&pxmh,sizeof(int),1, archivo); //pixel por metro
	fwrite(&pxmv,sizeof(int),1, archivo); //pixel por metro
	fwrite(&coloresUsados,sizeof(int),1, archivo); //colores usados
	fwrite(&coloresImportantes,sizeof(int),1, archivo); //colores importantes

    Pixel p;

    for(w=0;w<WIDTH;w++){
        for(h=0;h<(HEIGHT-SQUARE_COLOR_SIZE*2);h++){
            p.r=windowArray[w][h][0];
            p.g=windowArray[w][h][1];
            p.b=windowArray[w][h][2];
            fwrite(&p.b,sizeof(char),1,archivo);
            fwrite(&p.g,sizeof(char),1,archivo);
            fwrite(&p.r,sizeof(char),1,archivo);
        }
    }
    fclose(archivo);
}

void loadBMP( int number ){
    unsigned char windowArray[WIDTH][HEIGHT][3];
    char* filename = "";
    switch(number){
        case 1:
            filename = "default/cat.bmp";
            break;
        case 2:
            filename = "default/star.bmp";
            break;
        case 3:
            filename = "default/girl.bmp";
            break;
    }

    BMP *img = (BMP*)malloc(sizeof(BMP));
    FILE *archivo = fopen( filename, "rb" );

    int w,h;
    fseek( archivo,0, SEEK_SET);
	fread(&img->bm,sizeof(char),2, archivo);//tipo
	fread(&img->tamano,sizeof(int),1, archivo);//tam
	fread(&img->reservado,sizeof(int),1, archivo);//reservado
	fread(&img->offset,sizeof(int),1, archivo);//offset
	fread(&img->tamanoMetadatos,sizeof(int),1, archivo);//tam metadatos
	fread(&img->ancho,sizeof(int),1, archivo);//ancho
	fread(&img->alto,sizeof(int),1, archivo);//alto
	fread(&img->numeroPlanos,sizeof(short int),1, archivo);//planos
	fread(&img->profundidadColor,sizeof(short int),1, archivo);//profundidad (bits)
	fread(&img->tipoCompresion,sizeof(int),1, archivo);//compresion
	fread(&img->tamanoEstructura,sizeof(int),1, archivo);//tamPaleta
	fread(&img->pxmh,sizeof(int),1, archivo); //pixel por metro
	fread(&img->pxmv,sizeof(int),1, archivo); //pixel por metro
	fread(&img->coloresUsados,sizeof(int),1, archivo); //colores usados
	fread(&img->coloresImportantes,sizeof(int),1, archivo); //colores importantes

    for(w=0;w<WIDTH;w++){
        for(h=0;h<HEIGHT;h++){
            fread(&windowArray[w][h][2],sizeof(char),1,archivo);
            fread(&windowArray[w][h][1],sizeof(char),1,archivo);
            fread(&windowArray[w][h][0],sizeof(char),1,archivo);
        }
    }

    glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,windowArray);
    glutSwapBuffers();
    glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,windowArray);
    glFlush();
    fclose(archivo);

}

void text(int x, int y, char *word) {
    int i;
    glColor3f(0,0,0);
    glRasterPos2f(x, y);
    for (i = 0; word[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, word[i]);
    glutSwapBuffers();
    glColor3f(0,0,0);
    glRasterPos2f(x, y);
    for (i = 0; word[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, word[i]);

    glRasterPos2f(0, 0);
}

