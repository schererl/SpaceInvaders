// **********************************************************************
// PUCRS/FACIN
// Programa de testes para manipulação de Imagens
//
// Marcio Sarroglia Pinho
//
// pinho@pucrs.br
// **********************************************************************
//
//  Para rodar no XCODE, veja o arquivo "InstrucoesXCODE.rtf"
//


#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
using namespace std;
#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#include <time.h>
#include "ImageClass.h"
#include "Temporizador.h"
#include<stdio.h>
#include<stdlib.h>

ImageClass Image, GameOver, YouWin;
Temporizador T;
double AccumDeltaT=0;
double tempoDisparo;
int contador_inimigos = 0;
int contador_inimigos_vivos = 0;
double velocidade_inimigos = 1;
float posicao_jogador = 5;
bool VerificaColisao(int x, int y, int& indice_inimigo);
void AtualizaInimigos(double time);
void GeraInimigos();
void ChegadaInimigo();
void CarregaVida();
void TiraVida();
void DesenhaVida();
void display( void );
int** LeituraArquivo(std::string nome_arquivo, int &l, int &c);
void GeraPaleta();
void GeraMatrizInimigos();
void GeraPlayer();
void GeraVida();
void GeraDisparo();
struct MatrixFile
{
    int** matriz;
    int linha;
    int coluna;
};

void LeMatrix(MatrixFile *modelo, std::string nome_arquivo);
void InterpretaMatrix(double escala_x, double escala_y, MatrixFile *matrix);
void DesenhaPixel();
MatrixFile paleta;
MatrixFile inimigo_1;
MatrixFile inimigo_2;
MatrixFile inimigo_3;
MatrixFile player;
MatrixFile player_hit;
MatrixFile vida;
MatrixFile disparo;
bool gameOver = false;
bool youWin = false;
void init(void)
{

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);// R G B

    int r;
    // Carrega a uma imagem
    r = Image.Load("images.png");
    if (!r) exit(1); // Erro na carga da imagem
    else cout << ("Imagem carregada!\n");


    r = GameOver.Load("gameOver.png");
    if (!r) exit(1); // Erro na carga da imagem
    else cout << ("Imagem carregada!\n");


    r = YouWin.Load("youWin.jpg");
    if (!r) exit(1); // Erro na carga da imagem
    else cout << ("Imagem carregada!\n");


    tempoDisparo = 10/1; //p/s
    srand(time(NULL));
    CarregaVida();
    GeraPaleta();
    GeraMatrizInimigos();
    GeraPlayer();
    GeraVida();
    GeraDisparo();
    std::cout << "tempo de disparo p/s: " << tempoDisparo << std::endl;
}

void reshape( int w, int h )
{
    cout << "RESHAPE !! " << endl;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela �rea OpenGL dentro da Janela

    glViewport(0, 0, w, h);  // Sistema de Referencia do Dispositivo
                            // ONDE aparece o desenho - VIEWPORT

    glOrtho(0,10,0,10,0,1); // Universo de Trabalho - Sistema de Referencia do Universo
                            // Define os Limites da minha aplica��o - WINDOW

    // Define os limites l�gicos da �rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void GeraVida(){LeMatrix(&vida, "vida.txt");}

void GeraMatrizInimigos()
{
    LeMatrix(&inimigo_1, "nave_1.txt");
    LeMatrix(&inimigo_2, "nave_2.txt");
    LeMatrix(&inimigo_3, "nave_3.txt");
}

void GeraPlayer()
{
    LeMatrix(&player, "player.txt");
    LeMatrix(&player_hit, "player_hit.txt");
}

void GeraPaleta(){ LeMatrix(&paleta, "PaletaCores.txt"); }

void GeraDisparo(){ LeMatrix(&disparo, "disparo.txt");}

void LeMatrix(MatrixFile *modelo, std::string nome_arquivo)
{
    modelo->matriz = LeituraArquivo(nome_arquivo, modelo->linha, modelo->coluna);
}

int** LeituraArquivo(std::string nome_arquivo, /*int **matriz,*/ int &l, int &c)
{

    int **matriz;
    ifstream txtFile;
    txtFile.open(nome_arquivo);

    txtFile >> l >> c;

    //aloca memoria dinâmicamente para a matriz
    matriz = (int **)malloc( l * (sizeof(int *)));
    for(int i = 0; i < l; i++)
    {
        matriz[i] = (int *)malloc( c * (sizeof(int)));
    }

    for(int i=0; i < l;i++)
    {
        for(int j=0;j<c ;j++)
        {
            txtFile >> matriz[i][j];
        }
    }

    txtFile.close();
    return matriz;
}


//******DESENHO DE OBJETOS*******//
void DesenhaVida()
{
    InterpretaMatrix(0.07, 0.07, &vida);
}
bool hit = false;
void DesenhaNave()
{
    if(hit == true)
    {
        InterpretaMatrix(0.2, 0.1, &player_hit);
    }
    else
        InterpretaMatrix(0.2, 0.1, &player);
}
void DesenhaMunicao()
{
    glPushMatrix();
    InterpretaMatrix(0.075, 0.075, &disparo);
    glPopMatrix();

}
void InterpretaMatrix(double escala_x, double escala_y, MatrixFile *matrix)
{
    MatrixFile *modelo = matrix;
    glPushMatrix();
    glScalef(escala_x, escala_y,1);
    for(int i = 0; i < modelo->linha; i++)
    {
        for(int j = 0; j < modelo->coluna; j++)
        {
            int cor = modelo->matriz[i][j];
            if(cor == 0) continue;
            glPushMatrix();
            glTranslatef(j, i,0);
            glColor3f(paleta.matriz[cor][0], paleta.matriz[cor][1], paleta.matriz[cor][2]);
            DesenhaPixel();
            glPopMatrix();
        }

    }
    glPopMatrix();
}

void DesenhaPixel()
{
    glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(1,0);
    glVertex2f(1,1);
    glVertex2f(0,1);
    glEnd();
}

//*******************************//




//*****CONTROLE DE INIMIGOS******//
int op = 0;
class Inimigo
{
    double x, y, vel, altura, largura, escala_x, escala_y;
    bool vivo;
    MatrixFile* modelo;

public:
    Inimigo()
    {
        if(op == 0)
        {
            modelo = &inimigo_1;
            escala_x = 0.1;
            escala_y = 0.2;
            altura = escala_y * modelo->linha;
            largura = escala_x * modelo->coluna;
            vel = 0.15;
        }
        if(op == 1)
        {
            modelo = &inimigo_2;
            escala_x = 0.2;
            escala_y = 0.1;
            altura = escala_y * modelo->linha + 0.05;
            largura = escala_x * modelo->coluna + 0.05;
            vel = 2.5;
        }
        if(op == 2)
        {
            modelo = &inimigo_3;
            escala_x = 0.1;
            escala_y = 0.2;
            altura = escala_y * modelo->linha + 0.05;
            largura = escala_x * modelo->coluna + 0.05;
            vel = 1.2;
        }
        op+=1;
        if(op > 2) op = 0;

        vivo = true;
        GeraPosicao(x, y);
        IncrementaVelocidade();
    }

    void Desenha()
    {
        InterpretaMatrix(escala_x, escala_y, modelo);
    }

    void Morto()
    {
        vivo = false;
        DecrementaVivos();
    }

    void IncrementaContador()
    {
        contador_inimigos+=1;
    }


    void ChegadaInimigo()
    {
        int indice_inimigo;
        if(VerificaColisao(getX(), 0 , indice_inimigo))
        {
            Morto();
            TiraVida();
            hit = true;
        }
    }

    bool Colisao(int x, int y)
    {
        //TODO: arrumar a construção dos modelos com um ponto central
        bool verificaY = y  >= getY() && y <= (getY() + getAltura() + 0.5);
        bool verificaX = (x >= getX() - 1) && (x <= getX() + getLargura() - 0.3);
        return verificaY && verificaX;
    }

    bool isAlive(){ return this->vivo; }
    double getX() { return this->x; }
    double getY() { return this->y; }
    double getAltura() { return this-> altura; }
    double getLargura() { return this-> largura; }
    double getVel(){ return this->vel; }
    void CaiY(double avanco){ this->y-=avanco; }

private:


    void DecrementaVivos()
    {
        contador_inimigos_vivos -= 1;
    }

    void IncrementaVelocidade()
    {
        if(velocidade_inimigos > 2.3) velocidade_inimigos = 1;
        velocidade_inimigos += 0.1;
    }
    void GeraPosicao(double &X, double &Y)
    {
        int tmp_x, tmp_y, indice;
        while(true)
        {
            tmp_x = (0 + (rand() % 9));
            tmp_y = (6 + (rand() % 8));

            if(!(tmp_x < 10 && tmp_x > 1 && tmp_y < 10 && tmp_y > 5)) continue;
            if(!VerificaColisao(tmp_x, tmp_y, indice)) break;
        }
        X = tmp_x;
        Y = tmp_y;
    }

};
Inimigo inimigos[20];
//colisao de x & y com inimigos
bool VerificaColisao(int x, int y, int &indice_inimigo)
{
    for(int i = 0; i < contador_inimigos; i++)
    {
        Inimigo tmp = inimigos[i];
        if(!tmp.isAlive()) continue;

        if(tmp.Colisao(x, y))
        {
            indice_inimigo = i;
            return true;
        }
    }
        indice_inimigo = -1;
        return false;
}
void AtualizaInimigos(double time)
{
    for(int i = 0; i < contador_inimigos; i++)
    {
        if(inimigos[i].isAlive())
        {
            glColor3f(0,0,1);
            glPushMatrix();
            glTranslatef(inimigos[i].getX(), inimigos[i].getY(), 0);
            inimigos[i].Desenha();
            inimigos[i].CaiY((time * inimigos[i].getVel()));
            inimigos[i].ChegadaInimigo();
            glPopMatrix();
        }
    }

}
void GeraInimigos()
{
    if(contador_inimigos_vivos < 4 && contador_inimigos < 20){
        Inimigo tmp;
        contador_inimigos_vivos +=1;
        inimigos[contador_inimigos] = tmp;
        inimigos[contador_inimigos].IncrementaContador();
    }
}
//*****************************//


//*****CONTROLE DE VIDA*********//
struct Vida
{
    double x;
    bool spawn;
};
Vida vidas[3];
int vidas_restantes = 3;
void CarregaVida()
{
    vidas[0].spawn = true;
    vidas[0].x = 0;

    vidas[1].spawn = true;
    vidas[1].x = -0.5;

    vidas[2].spawn = true;
    vidas[2].x = -1;
}
void TiraVida()
{
    if(vidas_restantes == 0) return;
    vidas[vidas_restantes-1].spawn = false;
    vidas_restantes--;
}
void AtualizaVidas()
{
    glPushMatrix();
    glTranslatef(5,5,0);
    glScalef(0.5,0.75,1);

    glPopMatrix();

    for(int i = 0; i < 3;i++)
    {
        if(!vidas[i].spawn) continue;
        glPushMatrix();
        glColor3f(0,125,75);
        glTranslatef(9 + vidas[i].x,9,0);
        glScalef(0.5,0.75,1);
        DesenhaVida();
        glPopMatrix();

    }
}
//***************************//


//********CONTROLE DE DISPARO******//
struct municao
{

    float X;
    float Y;
    bool  disparo;
};
municao tiros[500];
int contador_disparo = 0;
void Disparo()
{
  municao tmp;
  tmp.X = posicao_jogador + 0.9;
  tmp.Y = 1;
  tmp.disparo = true;
  tiros[contador_disparo] = tmp;
  contador_disparo+=1;
}
void AtualizaDisparo(double vel)
{
    for(int i = 0; i < contador_disparo; i++)
    {
        if(tiros[i].disparo == true)
        {
            glPushMatrix();
            tiros[i].Y+= vel;
            glTranslatef(tiros[i].X, tiros[i].Y, 0);
            DesenhaMunicao();
            glPopMatrix();
            int indice_inimigo = -1;
            if(tiros[i].Y > 10)
            {
                tiros[i].disparo = false;
            }

            if(VerificaColisao(tiros[i].X, tiros[i].Y, indice_inimigo))
            {
                if(inimigos[indice_inimigo].isAlive())
                {
                    inimigos[indice_inimigo].Morto();
                    tiros[i].disparo = false;
                }
            }
        }
    }
}
//********************************//
//TODO: fazer uma struct para o jogador
//*****CONTROLE DO JOGADOR******//
void AtualizaNave(double tempoPassado)
{
    double static tempoHit = 0;

    if(hit == true && tempoHit <= 0.5) tempoHit += tempoPassado;
    else
    {
        tempoHit = 0;
        hit = false;
    }

    int indice_inimigo;
    if(VerificaColisao(posicao_jogador, 1.5, indice_inimigo))
    {
        hit = true;
        inimigos[indice_inimigo].Morto();
        TiraVida();
    }

    glColor3f(1,0,0);       // Vermelho
    glPushMatrix();
    glTranslatef(posicao_jogador,0.5,0);
    DesenhaNave();
    glPopMatrix();
}
void MoveEsquerda()
{
    posicao_jogador+=0.5;
    if(posicao_jogador > 11.5) posicao_jogador = -1;
    display();
}
void MoveDireita()
{
    posicao_jogador-=0.5;

    if(posicao_jogador < -1) posicao_jogador = 10;
    display();
}
//******************************//
void VerificaFim()
{
    if(contador_inimigos >= 20 && contador_inimigos_vivos == 0) youWin = true;//cout << "fim";
    else if(vidas_restantes == 0) gameOver = true;
}

void display( void )
{
    double dt = T.getDeltaT();

    AccumDeltaT += dt;
    if (AccumDeltaT > 3) // imprime o frame rate a cada 3 segundos
    {
        AccumDeltaT =0;
        cout << "FPS: " << 1.0/dt << endl;
    }

	// Limpa a tela com a cor de fundo


	glClear(GL_COLOR_BUFFER_BIT);
    // Define os limites l�gicos da �rea OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(gameOver)
    {

    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)GameOver.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)GameOver.SizeY();

    GameOver.SetZoomH(zoomH);
    GameOver.SetZoomV(zoomV);
    GameOver.SetPos(0, 0);
    GameOver.Display();
    }
    else if(youWin)
    {

    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)YouWin.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)YouWin.SizeY();

    YouWin.SetZoomH(zoomH);
    YouWin.SetZoomV(zoomV);
    YouWin.SetPos(0, 0);
    YouWin.Display();

    }
    else
    {

    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)Image.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)Image.SizeY();

    Image.SetZoomH(zoomH);
    Image.SetZoomV(zoomV);
    Image.SetPos(0, 0);

    Image.Display();
    AtualizaDisparo(dt * tempoDisparo);
    AtualizaNave(dt);
    GeraInimigos();
    AtualizaInimigos(dt);
    AtualizaVidas();
    VerificaFim();

    }

	glutSwapBuffers();
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
// **********************************************************************

void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;
        case 'd':
            MoveEsquerda();
            break;
        case 'a':
            MoveDireita();
            break;
        case 'q':
            Disparo();
            break;
        case ' ':
            Disparo();
            break;
		default:
			break;
	}
}


// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
//
//
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
		case GLUT_KEY_UP:       // Se pressionar UP
			glutFullScreen ( ); // Vai para Full Screen
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar UP
			glutPositionWindow (50,50);
			glutReshapeWindow ( 700, 500 );
			break;
        case GLUT_KEY_LEFT:
            MoveDireita();
            break;
        case GLUT_KEY_RIGHT:
            MoveEsquerda();
            break;
		default:
			break;
	}
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
//
// **********************************************************************
int  main ( int argc, char** argv )
{
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de t�tulo da janela.
    glutCreateWindow    ( "Primeiro Programa em OpenGL" );

    // executa algumas inicializa��es
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // ser� chamada automaticamente quando
    // for necess�rio redesenhar a janela

    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalida��o da tela. A funcao "display"
    // ser� chamada automaticamente sempre que a
    // m�quina estiver ociosa (idle)

    glutIdleFunc(display);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // ser� chamada automaticamente quando
    // o usu�rio alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // ser� chamada automaticamente sempre
    // o usu�rio pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" ser� chamada
    // automaticamente sempre o usu�rio
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
