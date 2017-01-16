#ifndef SPLINES_H_INCLUDED
#define SPLINES_H_INCLUDED
#include <vector>
#include <fstream>
#include <stdlib.h>

using namespace std;

struct ponto2D{
    float coord[2];
};

class pontoLista{
    private:
        float coord[2];
        pontoLista* prox = NULL;
    public:
        pontoLista(float x, float y){
            coord[0]= x;
            coord[1]= y;
        }
        void setProx(pontoLista* p){ prox = p;}
        float getX(){ return coord[0]; }
        float getY(){ return coord[1]; }
        float* getCoord() { return coord; }
        pontoLista* getProx(){ return prox; }
        void mover(int x, int y, float windowSizeX, float windowSizeY){
            y = windowSizeY - y;
            if(x>0 && x< windowSizeX && y>0 && y<windowSizeY){
                coord[0]= x;
                coord[1]= y;
            }
        }
        ~pontoLista(){}
};

class B_Spline{
private:
    pontoLista *pri=NULL, *ult=NULL, *selecionado=NULL;
    int n_pontoListas=0;

    ponto2D splinePoint(float t, float p0[2], float p1[2], float p2[2], float p3[2]){
        ponto2D p;
        for (int i=0; i < 2; i++){
            p.coord[i]=   (   p0[i] +4*p1[i] +  p2[i]
                   + t*(-3*p0[i]          +3*p2[i]
                   + t*( 3*p0[i] -6*p1[i] +3*p2[i]
                   + t*(-  p0[i] +3*p1[i] -3*p2[i] +  p3[i]))))/6.0;
        }
        return p;
    }

public:
    B_Spline(){}

    void add_ponto(float x, float y, bool no_fim=true){
        pontoLista* p= new pontoLista(x, y);
        if  (no_fim){   //insere no fim
            if (pri==NULL){
                pri=ult=p;
            }else{
                ult->setProx(p);
                ult=p;
            }
        }
        else{   //insere no inicio
            p->setProx(pri);
            pri=p;
        }
        this->n_pontoListas++;
    }

    bool contato(pontoLista* p, float x, float y, float prec){
        return ( p->getX() < x+prec && p->getX() > x-prec &&
                 p->getY() < y+prec && p->getY() > y-prec     );
    }

    bool vazia(){
        return pri == NULL ? true : false;
    }

    void remover(float x, float y, float prec=0.1){
        printf("Removendo pontoLista [%.2f, %.2f]", x, y);
        pontoLista *p= pri, *d;
        if (!vazia()){
            if (contato(pri, x, y, prec)){
                pri= pri->getProx();
                delete p;
            }else{
                while (!contato(p->getProx(), x, y, prec)){
                    if(p->getProx() != ult)
                        p= p->getProx();
                    else
                        return;
                }
                d = p->getProx();
                if (d == ult)
                    ult=p;
                p->setProx(d->getProx());
                delete d;
            }
            this->n_pontoListas--;
            imprime();
        }
    }

    void selecionar(float x, float y, float prec=0.1){
        pontoLista* p=pri;
        while (p!=NULL && !contato(p, x, y, prec))
            p= p->getProx();
        selecionado= p;
    }

    void moverPontoControle(int x, int y, float windowSizeX, float windowSizeY){
        if (selecionado != NULL)
            selecionado->mover(x, y, windowSizeX, windowSizeY);
    }

    void imprime(){
        for (pontoLista *p= pri; p != NULL; p= p->getProx())
            printf("\n%d. [%.2f, %.2f]\n", 00, p->getX(), p->getY());
        printf("\n");
    }

    B_Spline* copia(){
        B_Spline *b2 = new B_Spline();
        for (pontoLista *p= pri; p != NULL; p= p->getProx())
            b2->add_ponto(p->getX(), p->getY());
        return b2;
    }

    void plotaPontos(bool enumerar=true, float PointSize=5.0, float R=0.0, float G=0.0, float B=0.0){
        glColor3f(R,G,B);
        glPointSize(PointSize);
        glBegin(GL_POINTS);
            for (pontoLista *p= pri; p != NULL; p= p->getProx())
                glVertex2fv(p->getCoord());
        glEnd();

        if ( enumerar ) {
            glMatrixMode( GL_MODELVIEW );
            int cont=0;
            for (pontoLista *p= pri; p != NULL; p= p->getProx()){
                //gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
                glRasterPos2f( p->getX()+PointSize, p->getY());
                char num[5];      sprintf(num,"%d", cont);
                string nome_pontoLista= "P" + string(num);
                for( int i=0; i< nome_pontoLista.length(); i++ )
                  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, nome_pontoLista[i] );
                cont++;
            }
        }
    }

    void plotaArestas(float LineWidth=2.0, float R=1.0, float G=1.0, float B=0.0){
        glColor3f(R, G, B);
        glLineWidth(LineWidth);
        glBegin(GL_LINE_STRIP);
            for (pontoLista *p= pri; p != NULL; p= p->getProx())
                glVertex2fv(p->getCoord());
        glEnd();
    }

    vector<ponto2D> listarPontos(float prec){
        vector<ponto2D> pontos;
        if (this->n_pontoListas > 3){
            float delta = 1.0/prec;
            pontoLista* p= this->pri;
            vector <pontoLista*> plot;
            for (int i=0; i < 3; i++){
                plot.push_back(p);
                p= p->getProx();
            }

            while (p != NULL){
               plot.push_back(p);
                   for (float u=0; u <= 1.0; u+=delta){
                        // invoca o avaliador para o par�metro u
                        ponto2D pAtual = splinePoint(u, plot[0]->getCoord(), plot[1]->getCoord(), plot[2]->getCoord(), plot[3]->getCoord());
//                        if(pontos.empty())
//                            pontos.push_back(pAtual);
//                        else if( u!=0)
//                            pontos.push_back(pAtual);
                        if(pontos.empty() or u!=0)
                            pontos.push_back(pAtual);
                   }
               plot.erase(plot.begin());
               p= p->getProx();
            }
        }
        return pontos;
    }

    void B_Splines_Cubica(float prec, float LineWidth=3.0, float R=0.0, float G=0.0, float B=1.0){
        vector<ponto2D> pontoListas = this->listarPontos(prec);

        glColor3f(R, G, B);
        glLineWidth(LineWidth);
        glBegin(GL_LINE_STRIP);
        for (int i=0; i< pontoListas.size(); i++)
            glVertex2fv(pontoListas[i].coord);
        glEnd();
    }

    void salvar(char* nome){
        ofstream salva;
        salva.open(nome);
        for (pontoLista *p= pri; p != NULL; p= p->getProx())
            salva<<p->getX()<<" "<<p->getY()<<endl;
        salva.close();
    }

    ~B_Spline(){
        pontoLista *p=pri, *q;
        if (p != NULL)
            q= p->getProx();
        while (q != NULL){
            delete p;
            p= q;
            q= q->getProx();
        }
    }
};


#endif // SPLINES_H_INCLUDED
