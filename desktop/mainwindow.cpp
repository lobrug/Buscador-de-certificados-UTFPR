#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDate>
#include <QPushButton>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);
    connect(ui->btnBuscar, &QPushButton::clicked, this, &MainWindow::iniciarBusca);

    // Adiciona apenas o texto, sem o parâmetro de dados extra
    ui->comboAno->addItem("Varredura Completa (Todos)");
    int anoAtualData = QDate::currentDate().year();
    for (int i = anoAtualData; i >= 2013; i--) {
        ui->comboAno->addItem(QString::number(i));
    }

    ui->comboCampus->addItem("Varredura Completa (Todos)", "todos");
    ui->comboCampus->addItem("AdministradorGlobal", 14);
    ui->comboCampus->addItem("Apucarana", 2);
    ui->comboCampus->addItem("Campo Mourão", 3);
    ui->comboCampus->addItem("Cornélio Procópio", 13);
    ui->comboCampus->addItem("Curitiba", 1);
    ui->comboCampus->addItem("Diretoria de Graduação - Apucarana", 17);
    ui->comboCampus->addItem("Diretoria de Graduação - Campo Mourão", 18);
    ui->comboCampus->addItem("Diretoria de Graduação - Cornélio Procópio", 19);
    ui->comboCampus->addItem("Diretoria de Graduação - Curitiba", 26);
    ui->comboCampus->addItem("Diretoria de Graduação - Dois Vizinhos", 20);
    ui->comboCampus->addItem("Diretoria de Graduação - Francisco Beltrão", 21);
    ui->comboCampus->addItem("Diretoria de Graduação - Guarapuava", 22);
    ui->comboCampus->addItem("Diretoria de Graduação - Londrina", 16);
    ui->comboCampus->addItem("Diretoria de Graduação - Medianeira", 23);
    ui->comboCampus->addItem("Diretoria de Graduação - Pato Branco", 24);
    ui->comboCampus->addItem("Diretoria de Graduação - Ponta Grossa", 25);
    ui->comboCampus->addItem("Diretoria de Graduação - Santa Helena", 27);
    ui->comboCampus->addItem("Diretoria de Graduação - Toledo", 28);
    ui->comboCampus->addItem("Dois Vizinhos", 4);
    ui->comboCampus->addItem("Francisco Beltrão", 5);
    ui->comboCampus->addItem("Guarapuava", 6);
    ui->comboCampus->addItem("Londrina", 7);
    ui->comboCampus->addItem("Medianeira", 8);
    ui->comboCampus->addItem("Pato Branco", 9);
    ui->comboCampus->addItem("Ponta Grossa", 10);
    ui->comboCampus->addItem("PROGRAD", 15);
    ui->comboCampus->addItem("PROPPG", 29);
    ui->comboCampus->addItem("PROREC", 30);
    ui->comboCampus->addItem("Santa Helena", 11);
    ui->comboCampus->addItem("Toledo", 12);

    ui->comboCampus->setCurrentIndex(2);

    // Estilização base do terminal (Fundo escuro, fonte monoespaçada)
    ui->terminalLog->setStyleSheet("QPlainTextEdit { background-color: #0c0c0c; font-family: 'Consolas'; font-size: 13px; }");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iniciarBusca()
{
    nomeBuscado = ui->inputNome->text().trimmed();
    QString textoAno = ui->comboAno->currentText();
    anoAtual = (textoAno == "Varredura Completa (Todos)") ? "" : textoAno;
    QString selecaoCampus = ui->comboCampus->currentData().toString();

    if(nomeBuscado.isEmpty()) {
        ui->terminalLog->appendHtml("<span style='color:#ff5555;'>ERRO: Preencha o nome para realizar a busca.</span>");
        return;
    }

    ui->btnBuscar->setEnabled(false);
    ui->terminalLog->clear();
    filaCampus.clear();
    filaEventos.clear();
    requisicoesAtivas = 0;

    if (selecaoCampus == "todos") {
        for (int i = 1; i < ui->comboCampus->count(); ++i) {
            filaCampus.append(ui->comboCampus->itemData(i).toString());
        }
    } else {
        filaCampus.append(selecaoCampus);
    }

    ui->terminalLog->appendHtml(QString("<span style='color:#ffffff;'>Iniciando motor de busca por <b>'%1'</b> (%2)</span>").arg(nomeBuscado, anoAtual.isEmpty() ? "Todos os anos" : anoAtual));
    processarProximoCampus();
}

void MainWindow::processarProximoCampus()
{
    if (filaCampus.isEmpty()) {
        ui->terminalLog->appendHtml("<br><span style='color:#55ff55;'><b>[!] Varredura concluída com sucesso.</b></span>");
        ui->btnBuscar->setEnabled(true);
        return;
    }

    campusAtual = filaCampus.takeFirst();
    ui->terminalLog->appendHtml(QString("<br><span style='color:#ffdd55;'>&gt;&gt;&gt; ACESSANDO CÂMPUS ID: %1</span>").arg(campusAtual));

    QNetworkRequest request(QUrl("https://apl.utfpr.edu.br/extensao/certificados/listaPublica"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("txtCampus", campusAtual);
    params.addQueryItem("txtAno", anoAtual);

    QNetworkReply *reply = networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString html = QString::fromUtf8(reply->readAll());

            QRegularExpression selectRegex("<select name=\"txtEvento\"[^>]*>(.*?)</select>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch selectMatch = selectRegex.match(html);

            if (selectMatch.hasMatch()) {
                QString optionsHtml = selectMatch.captured(1);
                QRegularExpression optionRegex("<option\\s+value=\"(\\d+)\"[^>]*>([^<]+)</option>");
                QRegularExpressionMatchIterator optIt = optionRegex.globalMatch(optionsHtml);

                while (optIt.hasNext()) {
                    QRegularExpressionMatch optMatch = optIt.next();
                    QString id = optMatch.captured(1);
                    QString nome = optMatch.captured(2).trimmed();
                    if (id != "0" && !id.isEmpty()) {
                        filaEventos.append({id, nome});
                    }
                }
            }

            if(filaEventos.isEmpty()) {
                ui->terminalLog->appendHtml("<span style='color:#ffaa00;'>Nenhum evento localizado neste câmpus.</span>");
                processarProximoCampus();
            } else {
                ui->terminalLog->appendHtml(QString("<span style='color:#cccccc;'>Mapeados %1 eventos. Iniciando concorrência...</span>").arg(filaEventos.size()));
                processarLoteEventos();
            }
        } else {
            ui->terminalLog->appendHtml("<span style='color:#ff5555;'>Falha ao comunicar com a UTFPR: " + reply->errorString() + "</span>");
            processarProximoCampus();
        }
        reply->deleteLater();
    });
}

void MainWindow::processarLoteEventos()
{
    if (filaEventos.isEmpty() && requisicoesAtivas == 0) {
        processarProximoCampus();
        return;
    }

    while (requisicoesAtivas < limiteConcorrencia && !filaEventos.isEmpty()) {
        Evento evento = filaEventos.takeFirst();
        requisicoesAtivas++;
        buscarPaginaCertificados(evento, 0);
    }
}

void MainWindow::buscarPaginaCertificados(Evento evento, int offset)
{
    QString urlStr = "https://apl.utfpr.edu.br/extensao/certificados/listaPublica";
    if (offset > 0) urlStr += "/" + QString::number(offset);

    QNetworkRequest request((QUrl(urlStr)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("txtCampus", campusAtual);
    params.addQueryItem("txtAno", anoAtual);
    params.addQueryItem("txtEvento", evento.id);

    QNetworkReply *reply = networkManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());

    connect(reply, &QNetworkReply::finished, this, [this, reply, evento, offset]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString html = QString::fromUtf8(reply->readAll());

            QRegularExpression rowRegex("<tr[^>]*>(.*?)</tr>", QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatchIterator rowIt = rowRegex.globalMatch(html);

            while (rowIt.hasNext()) {
                QRegularExpressionMatch rowMatch = rowIt.next();
                QString rowContent = rowMatch.captured(1);

                if (rowContent.contains(nomeBuscado, Qt::CaseInsensitive)) {
                    QRegularExpression linkRegex("<a[^>]+href=\"([^\"]+)\"");
                    QRegularExpressionMatch linkMatch = linkRegex.match(rowContent);
                    QString link = linkMatch.hasMatch() ? linkMatch.captured(1) : "";

                    if (!link.isEmpty() && !link.startsWith("http")) {
                        link = "https://apl.utfpr.edu.br" + link;
                    }

                    ui->terminalLog->appendHtml(QString("<br><span style='color:#55ff55;'><b>[+] CERTIFICADO ENCONTRADO!</b></span>"
                                                        "<br><span style='color:#ffffff;'>Evento: %1</span>"
                                                        "<br><span style='color:#55aaff;'>Link: <a href='%2' style='color:#55aaff;'>%2</a></span>"
                                                        "<br><span style='color:#888888;'>Processando resto dos eventos...</span>").arg(evento.nome, link));
                }
            }

            int proximoOffset = offset + 15;
            QString targetHref = QString("/%1").arg(proximoOffset);

            if (html.contains(targetHref)) {
                buscarPaginaCertificados(evento, proximoOffset);
            } else {
                requisicoesAtivas--;
                processarLoteEventos();
            }
        } else {
            ui->terminalLog->appendHtml(QString("<span style='color:#ff5555;'>Erro no evento %1: %2</span>").arg(evento.id, reply->errorString()));
            requisicoesAtivas--;
            processarLoteEventos();
        }
        reply->deleteLater();
    });
}