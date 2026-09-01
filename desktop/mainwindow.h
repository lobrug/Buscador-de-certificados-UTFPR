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
    void processarProximoCampus();
    void processarLoteEventos();
    void buscarPaginaCertificados(Evento evento, int offset);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;

    QList<QString> filaCampus;
    QList<Evento> filaEventos;
    QString nomeBuscado;
    QString campusAtual;
    QString anoAtual;

    int requisicoesAtivas;
    const int limiteConcorrencia = 15;
};

#endif // MAINWINDOW_H