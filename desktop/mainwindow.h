#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Evento {
    QString id;
    QString nome;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void iniciarBusca();
    void cancelarBusca();
    void processarProximoCampus();
    void processarLoteEventos();
    void buscarPaginaCertificados(Evento evento, int offset);
    void baixarCertificados();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;

    QList<QString> filaCampus;
    QList<Evento> filaEventos;
    QList<QString> certificadosEncontrados;

    QString nomeBuscado;
    QString campusAtual;
    QString anoAtual;

    int requisicoesAtivas;
    const int limiteConcorrencia = 50;

    // Variáveis de controle para progresso e cancelamento
    int totalEventosCampus;
    int eventosProcessadosCampus;
    bool buscaCancelada;
};

#endif // MAINWINDOW_H