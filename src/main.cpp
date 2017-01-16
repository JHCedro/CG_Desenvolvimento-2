#include <GL/glui.h>
#include "Splines.h"
#include <math.h>
#include <iostream>

using namespace std;

int precCurva = 30;  // Total de pontos intermedi�rios
float precRotacao = 10.0;

float alturaTela = 600, larguraTela = 800;
int ver_arestas = 0, move_mode = 0, ver_pontos=1, ver_curva=1, insere_no_fim=1, enumerar_pontos=1;

B_Spline* curvaAtual;
GLUI_Checkbox *GLUI_ver_pontos, *GLUI_enumerar_pontos, *GLUI_ver_arestas,
              *GLUI_ver_curva, *GLUI_fixar_eixo, *GLUI_grade_2D, *GLUI_grade_3D;
GLUI_Button *GLUI_restaurarVisao, *GLUI_limpar;


///VISTA 3D
int vista_3D=0, fixarEixoVertical=1, grade3D=1, grade2D=1;
float rotationX = 0.0, rotationY = 0.0;
int last_x, last_y;
int zoom = 0;
vector<ponto2D> pontosCurva;

float *vetor_normal(float p1[3], float p2[3],float p3[3]){
    float v1[3]={p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]},
          v2[3]={p3[0]-p2[0], p3[1]-p2[1], p3[2]-p2[2]};
    float *result=new float[3];
    result[0]=  v1[1]*v2[2]-v2[1]*v1[2];
    result[1]=-(v1[0]*v2[2]-v2[0]*v1[2]);
    result[2]=  v1[0]*v2[1]-v2[0]*v1[1];
    return result;
}

void init(void){
    glClearColor (.98, .98, .98, 0.0);
    glEnable(GL_NORMALIZE);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);               // Habilita Z-buffer
    glEnable(GL_LIGHTING);                 // Habilita luz
    glEnable(GL_LIGHT0);                   // habilita luz 0
    //glEnable(GL_CULL_FACE);

    // Cor da fonte de luz (RGBA)
    GLfloat cor_luz[]     = { 1.0, 1.0, 1.0, 1.0};
    // Posicao da fonte de luz. Ultimo parametro define se a luz sera direcional (0.0) ou tera uma posicional (1.0)
    GLfloat posicao_luz[] = { 0.0, 150.0, 0.0, 1.0};

    // Define parametros da luz
    glLightfv(GL_LIGHT0, GL_AMBIENT, cor_luz);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, cor_luz);
    glLightfv(GL_LIGHT0, GL_SPECULAR, cor_luz);
    glLightfv(GL_LIGHT0, GL_POSITION, posicao_luz);

    curvaAtual= new B_Spline();
}

void limparCurva(){
    delete curvaAtual;
    curvaAtual = new B_Spline();
    glutPostRedisplay();
}

void salvarCurva(bool nova=true){
    ///salvar uma nova curva
    curvaAtual->salvar("curva.txt");
}
void reshape(int w, int h)
{
    larguraTela=w;
    alturaTela=h;
}
void carregarCurva(){
    limparCurva();
    ifstream curva;
    curva.open("curva.txt");
    float x, y;
    while(curva.good())
    {
        curva>>x>>y;
        cout<<x<<" "<<y<<endl;
        curvaAtual->add_ponto(x,y);
    }
    curvaAtual->remover(x,y);
    curva.close();
    glutPostRedisplay();
}

void desenhaGrade2D(float L){
    glDisable(GL_LIGHTING);
    glLineWidth(0.1);
    glBegin(GL_LINES);
    glColor3f(.6, .6, .6);
    for(float i = 0; i < 1.0; i+=1.0/20.0){
        glVertex3f(larguraTela*i, 0, 0);
        glVertex3f(larguraTela*i, alturaTela, 0);

        glVertex3f(0, alturaTela*i, 0);
        glVertex3f(larguraTela, alturaTela*i, 0);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void display2D (void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, larguraTela, 0, alturaTela, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();
    glDisable(GL_LIGHTING);
    glClearColor(.8, .8, .8, 0);

    /// FUNCOES DO GLUI
//    if (ver_arestas) curvaAtual->plotaArestas(2.0f, 0.3f, 0.3f, 0.3f);
    if (ver_curva)   curvaAtual->B_Splines_Cubica(precCurva);
    if (ver_pontos)  curvaAtual->plotaPontos(enumerar_pontos);
    ver_pontos ? GLUI_enumerar_pontos->enable() : GLUI_enumerar_pontos->disable();

    if(grade2D)      desenhaGrade2D(larguraTela);

    glutSwapBuffers();
}

void desenhaEixos3D(float L){
    glDisable(GL_LIGHTING);
    glLineWidth(1.5);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(L, 0, 0);

    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, L, 0);

    glColor3f(0,0,1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, L);
    glEnd();

    if (grade3D){
        glLineWidth(1.0);
        glBegin(GL_LINES);
        glColor3f(.6, .6, .6);
        for(float i = -1.0; i <= 1.01; i+=1.0/20.0){
            glVertex3f(L*i, 0, -L);
            glVertex3f(L*i, 0,  L);

            glVertex3f(-L, 0, L*i);
            glVertex3f( L, 0, L*i);
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void restaurarVisao3D(){
    rotationX = 0.0;
    rotationY = 0.0;
    zoom = 0;
    glutPostRedisplay();
}

void setMaterial(void){
    // Material do objeto (neste caso, ruby). Parametros em RGBA
    GLfloat objeto_ambient[]   = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat objeto_difusa[]    = { 0.1, 0.5, 0.7, 1.0 };
    GLfloat objeto_especular[] = { 0.626959, 0.626959, 0.726959, 1.0 };
    GLfloat objeto_brilho[]    = { 50.0f };

    // Define os parametros da superficie a ser iluminada
    glMaterialfv(GL_FRONT, GL_AMBIENT, objeto_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, objeto_difusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, objeto_especular);
    glMaterialfv(GL_FRONT, GL_SHININESS, objeto_brilho);
}

void display3D(void){
    glEnable(GL_LIGHTING);
    setMaterial();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(45, (GLfloat) larguraTela/(GLfloat) alturaTela, 1.0, 4*larguraTela);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (1.5*larguraTela-zoom, 1.5*larguraTela-zoom, 1.5*larguraTela-zoom, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    setMaterial();
    glRotatef( rotationY, 0.0, 1.0, 0.0 );
    glRotatef( rotationX, 1.0, 0.0, 0.0 );

    ///Pontos da B-Spline
    vector<ponto2D> pontos = curvaAtual->listarPontos(precCurva);

//    ///Fechando a curva
//    if(!pontos.empty()){
//        ponto2D pFim=pontos.back(), pInicio = pontos.front();
//        for (float t=1.0/precCurva; t <= 1.01; t+=1.0/precCurva){
//            ponto2D p;
//            p.coord[0]= pFim.coord[0]+t*(pInicio.coord[0]-pFim.coord[0]);
//            p.coord[1]= pFim.coord[1]+t*(pInicio.coord[1]-pFim.coord[1]);
//            pontos.push_back(p);
//        }
//        pontos.push_back(pontos.front());
//    }

    ///Desenha superficie de rotacao
    float *n;
    for (float k=0; k <= 2*M_PI ; k+=precRotacao*M_PI/180){
        glEnable(GL_LIGHTING);
        for (int i=0; i < (int)pontos.size()-1; i++){
                float p1[3] = {cos(k) * pontos[i+1].coord[0], pontos[i+1].coord[1], sin(k) * pontos[i+1].coord[0]},
                      p2[3] = {cos(k) * pontos[i].coord[0], pontos[i].coord[1], sin(k) * pontos[i].coord[0]},
                      p3[3]= {cos(k+precRotacao*M_PI/180) * pontos[i].coord[0], pontos[i].coord[1],
                              sin(k+precRotacao*M_PI/180) * pontos[i].coord[0]},
                      p4[3]= {cos(k+precRotacao*M_PI/180) * pontos[i+1].coord[0], pontos[i+1].coord[1],
                              sin(k+precRotacao*M_PI/180) * pontos[i+1].coord[0]};
                n = vetor_normal(p1, p2, p3);
            glColor3f(1.0,0.0,0.0);

            glBegin(GL_POLYGON);
                            glNormal3fv( n );
                glVertex3fv( p1 );
                glVertex3fv( p2 );
                glVertex3fv( p3 );

            glEnd();
        glColor3f(0.0,0.0,1.0);
            glBegin(GL_POLYGON);
                            glNormal3fv( n );
                glVertex3fv( p1 );
                glVertex3fv( p3 );
                glVertex3fv( p4 );

            glEnd();
		delete n;
        }
    }

    desenhaEixos3D(700);

    glutSwapBuffers();
}

void alterarDisplay(){
    if (vista_3D == 0){
        vista_3D = 1;
        glutDisplayFunc(display3D);///Funcoes do GLUI
        GLUI_grade_2D->disable(); GLUI_ver_pontos->disable(); GLUI_enumerar_pontos->disable(); GLUI_ver_arestas->disable(); GLUI_ver_curva->disable();
        GLUI_grade_3D->enable(); GLUI_fixar_eixo->enable(); GLUI_restaurarVisao->enable();
    }
    else{
        vista_3D = 0;
        GLUI_grade_2D->enable(); GLUI_ver_pontos->enable(); GLUI_enumerar_pontos->enable(); GLUI_ver_arestas->enable(); GLUI_ver_curva->enable();
        GLUI_grade_3D->disable(); GLUI_fixar_eixo->disable(); GLUI_restaurarVisao->disable();
        glutDisplayFunc(display2D);
    }
    cout<<larguraTela<<endl<<alturaTela<<endl;
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
    if(vista_3D && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ){
        last_x = x;
        last_y = y;
    }
    else{
        y= alturaTela-y;
        if (button == GLUT_LEFT_BUTTON){
            if (state == GLUT_DOWN){
                printf("\nBotao esquerdo pressionado na posicao [%d, %d].\n", x, y);
                if (!move_mode)
                    curvaAtual->add_ponto(x, y, insere_no_fim);
                else{
                    curvaAtual->selecionar(x, y, 0.5*(alturaTela+larguraTela)/2.0*0.01);
                }
            }
        }
        if (button == GLUT_RIGHT_BUTTON){
            if (state == GLUT_UP){
                printf("\nBotao direito solto na posicao [%d, %d].\n", x, y);
                curvaAtual->remover(x, y, 0.5*(alturaTela+larguraTela)*0.01);
            }
        }
       if (button == 3 && zoom < 2000)
            zoom+=10;
        if (button == 4 && zoom > 0)
            zoom-=10;
    }
    cout<<x<<"   "<<y<<endl;
    glutPostRedisplay();
}

void motion(int x, int y){
    if(vista_3D){
        if(!fixarEixoVertical)
            rotationX -= (float) (y - last_y);
        rotationY -= (float) (x - last_x);

        last_x = x;
        last_y = y;
    }else{
        if(move_mode){
            curvaAtual->moverPontoControle(x, y, larguraTela, alturaTela);
        }
    }

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y){
   switch (key){
   case 27:
        delete curvaAtual;
        exit(0);
        break;
   case 'v':
       alterarDisplay();
       break;
   }
   glutPostRedisplay();
}

void GLUI_Menu(int main_window, int Wx, int Wy){
    GLUI *controle = GLUI_Master.create_glui( "Menu", 0, Wx, Wy);
    //GLUI_Panel *controle = new GLUI_Panel(opcoes, "Controles", true);
        //controle->set_alignment( GLUI_ALIGN_LEFT );
        GLUI_Spinner *precisao = new GLUI_Spinner( controle, "Precisao Curva:", &precCurva);
            precisao->set_int_limits( 1, 50);
            precisao->set_speed(0.5);

        GLUI_Spinner *precisaoR = new GLUI_Spinner( controle, "Precisao Rotacao:", &precRotacao);
            precisaoR->set_int_limits( 1, 60);
            precisaoR ->set_speed(0.5);

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "Botao esquerdo do mouse:");
        GLUI_RadioGroup *clique_esquerdo= new GLUI_RadioGroup(controle,&move_mode);
            new GLUI_RadioButton( clique_esquerdo, "Adicionar ponto" );
            new GLUI_RadioButton( clique_esquerdo, "Mover ponto" );

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "Inserir:");
        GLUI_RadioGroup *inserir= new GLUI_RadioGroup(controle,&insere_no_fim);
            new GLUI_RadioButton( inserir, "No in�cio" );
            new GLUI_RadioButton( inserir, "No fim" );

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "Visao 2D:");
        GLUI_grade_2D = new GLUI_Checkbox( controle, "Grade", &grade2D);
        GLUI_ver_pontos = new GLUI_Checkbox( controle, "Pontos", &ver_pontos);
        GLUI_enumerar_pontos= new GLUI_Checkbox( controle, "Nomear pontos", &enumerar_pontos);
        GLUI_ver_arestas = new GLUI_Checkbox( controle, "Arestas", &ver_arestas);
        GLUI_ver_curva = new GLUI_Checkbox( controle, "Curva", &ver_curva);
        GLUI_limpar = new GLUI_Button(controle, "Limpar pontos", 0, (GLUI_Update_CB)limparCurva);

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "Visao 3D:");
        GLUI_Button *vista = new GLUI_Button(controle, "Alternar 2D/3D", 0, (GLUI_Update_CB)alterarDisplay);
        GLUI_grade_3D = new GLUI_Checkbox( controle, "Grade", &grade3D);
        GLUI_fixar_eixo = new GLUI_Checkbox( controle, "Fixar eixo vertical", &fixarEixoVertical);
        GLUI_restaurarVisao = new GLUI_Button(controle, "Restaurar visao", 0, (GLUI_Update_CB)restaurarVisao3D);
            GLUI_grade_3D->disable();
            GLUI_fixar_eixo->disable();
            GLUI_restaurarVisao->disable();

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "Memoria:");

        GLUI_Button *salvarNova = new GLUI_Button(controle, "Salvar curva", true, (GLUI_Update_CB)salvarCurva);

        GLUI_Button *carregar = new GLUI_Button(controle, "Carregar curva", -1, (GLUI_Update_CB)carregarCurva);

        new GLUI_Separator( controle );
        new GLUI_StaticText( controle, "\n");
        GLUI_Button *sair = new GLUI_Button(controle, "Sair", 0, (GLUI_Update_CB)exit);

    controle->set_main_gfx_window( main_window );
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (larguraTela, alturaTela);
    glutInitWindowPosition (100, 100);

    int  main_window = glutCreateWindow ("B-Spline");
    GLUI_Menu(main_window, 100+larguraTela+10, 100);

    init ();
    glutDisplayFunc(display2D);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc( motion );
    glutMainLoop();
    glutMotionFunc( NULL );
    return 0;
}
