global.File = require('buffer').File;
const cheerio = require('cheerio');
const express = require('express');
const http = require('http');
const { Server } = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

app.use(express.static('public'));

async function extrairEventosComRobo(ano, enviarLog) {
    const { default: puppeteer } = await import('puppeteer');
    const browser = await puppeteer.launch({ headless: "new" }); 
    const page = await browser.newPage();
    
    await page.goto('https://apl.utfpr.edu.br/extensao/certificados/listaPublica');
    await page.select('select[name="txtCampus"]', '2');
    
    const navigationPromise = page.waitForNavigation({ waitUntil: 'networkidle2' });
    await page.evaluate((anoEscolhido) => {
        const selectAno = document.querySelector('select[name="txtAno"]');
        selectAno.value = anoEscolhido;
        selectAno.dispatchEvent(new Event('change', { bubbles: true }));
    }, ano.toString());
    await navigationPromise;

    const eventos = await page.evaluate(() => {
        return Array.from(document.querySelectorAll('select[name="txtEvento"] option'))
                    .map(opt => ({ id: Number(opt.value), nome: opt.textContent.trim() }))
                    .filter(evt => evt.id !== 0 && !isNaN(evt.id));
    });

    await browser.close(); 
    enviarLog(`Sucesso! Capturamos ${eventos.length} eventos.\n`);
    return eventos;
}

async function buscarCertificados(nomeProcurado, ano, enviarLog) {
    const listaDeEventos = await extrairEventosComRobo(ano, enviarLog);

    if (listaDeEventos.length === 0) {
        return enviarLog("Nenhum evento encontrado. Encerrando.");
    }

    let certificadosEncontrados = [];

    enviarLog(`Iniciando varredura por "${nomeProcurado}" nas páginas dos eventos...\n
=====================  Esta etapa pode demorar um pouco  =====================\n`);

    for (let i = 0; i < listaDeEventos.length; i++) {
        const eventoId = listaDeEventos[i].id;
        const nomeDoEvento = listaDeEventos[i].nome;
        let encontrouNoEvento = false;
        let offset = 0;
        let paginaAtual = 1;
        let temMaisPaginas = true;

        enviarLog(`[${i + 1}/${listaDeEventos.length}] Verificando: ${nomeDoEvento}`);

        while (temMaisPaginas) {
            const url = offset === 0 
                ? 'https://apl.utfpr.edu.br/extensao/certificados/listaPublica' 
                : `https://apl.utfpr.edu.br/extensao/certificados/listaPublica/${offset}`;

            const bodyData = new URLSearchParams({ txtCampus: '2', txtAno: ano.toString(), txtEvento: eventoId.toString() });

            try {
                const response = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: bodyData });
                const html = await response.text();
                const $ = cheerio.load(html);

                $('td').each((index, element) => {
                    const textoTd = $(element).text().trim();
                    if (textoTd.toLowerCase().includes(nomeProcurado.toLowerCase())) {
                        enviarLog(`\n🎉 ACHOU! (Adicionando ao relatório final...)`);
                        encontrouNoEvento = true;
                        
                        // Salva os dados na nossa memória
                        certificadosEncontrados.push({
                            nomeCertificado: textoTd,
                            evento: nomeDoEvento,
                            id: eventoId,
                            pagina: paginaAtual
                        });
                    }
                });

                const proximoOffset = offset + 15;
                const existeProximaPagina = $('a').toArray().some(el => {
                    const href = $(el).attr('href');
                    return href && href.includes(`/${proximoOffset}`);
                });

                if (existeProximaPagina) {
                    offset = proximoOffset;
                    paginaAtual++;
                    await new Promise(resolve => setTimeout(resolve, 500)); 
                } else {
                    temMaisPaginas = false; 
                }
            } catch (error) {
                enviarLog(`⚠️ Erro no evento ${eventoId}: ${error.message}`);
                temMaisPaginas = false; 
            }
        } 
        await new Promise(resolve => setTimeout(resolve, 1000));
    }


    enviarLog("\n==================================================");
    enviarLog("  RELATÓRIO FINAL DE BUSCA");
    enviarLog("==================================================");
    
    if (certificadosEncontrados.length > 0) {
        enviarLog(`Foram encontrados ${certificadosEncontrados.length} certificados para "${nomeProcurado}" no ano de ${ano}:\n`);
        
        certificadosEncontrados.forEach((cert, index) => {
            enviarLog(`[${index + 1}] Evento: ${cert.evento} (ID: ${cert.id})`);
            enviarLog(`    Nome no certificado: ${cert.nomeCertificado}`);
            enviarLog(`    Página da lista: ${cert.pagina}\n`);
        });
    } else {
        enviarLog(`Nenhum certificado encontrado para "${nomeProcurado}" no ano de ${ano}.`);
    }
    
    enviarLog("==================================================\n");
    enviarLog("Fim da varredura total!");
}

io.on('connection', (socket) => {
    socket.on('iniciar', async (dados) => {
        const enviarLog = (mensagem) => socket.emit('log', mensagem);
        await buscarCertificados(dados.nome, dados.ano, enviarLog);
    });
});

server.listen(3000, () => {
    console.log('🌐 Servidor rodando! Acesse http://localhost:3000 no seu navegador.');
});