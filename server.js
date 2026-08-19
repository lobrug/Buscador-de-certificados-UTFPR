global.File = require('buffer').File;
const cheerio = require('cheerio');
const express = require('express');
const http = require('http');
const { Server } = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

app.use(express.static('public'));

let cacheFormulario = null;

async function obterOpcoesFormulario() {
    if (cacheFormulario) return cacheFormulario;
    
    try {
        const response = await fetch('https://apl.utfpr.edu.br/extensao/certificados/listaPublica');
        const html = await response.text();
        const $ = cheerio.load(html);
        
        const campi = [];
        $('select[name="txtCampus"] option').each((i, el) => {
            const id = $(el).val();
            const nome = $(el).text().trim();
            if (id && id !== "") {
                campi.push({ id: Number(id), nome });
            }
        });

        const anos = [];
        $('select[name="txtAno"] option').each((i, el) => {
            const valor = $(el).val();
            const texto = $(el).text().trim();
            anos.push({ valor, texto });
        });

        cacheFormulario = { campi, anos };
    } catch (error) {
        console.error(error);
        cacheFormulario = { campi: [], anos: [] };
    }
    
    return cacheFormulario;
}

app.get('/api/opcoes', async (req, res) => {
    const opcoes = await obterOpcoesFormulario();
    res.json(opcoes);
});

async function extrairEventosComRobo(ano, campusId, enviarLog) {
    const { default: puppeteer } = await import('puppeteer');
    const browser = await puppeteer.launch({ headless: "new" }); 
    const page = await browser.newPage();
    
    await page.goto('https://apl.utfpr.edu.br/extensao/certificados/listaPublica');
    await page.select('select[name="txtCampus"]', campusId.toString());
    
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
    enviarLog(`Eventos capturados: ${eventos.length}\n`);
    return eventos;
}

async function processarEvento(evento, nomeProcurado, ano, campusId, campusNome, enviarLog, certificadosEncontrados) {
    let offset = 0;
    let paginaAtual = 1;
    let temMaisPaginas = true;

    while (temMaisPaginas) {
        const url = offset === 0 
            ? 'https://apl.utfpr.edu.br/extensao/certificados/listaPublica' 
            : `https://apl.utfpr.edu.br/extensao/certificados/listaPublica/${offset}`;

        const bodyData = new URLSearchParams({ txtCampus: campusId.toString(), txtAno: ano.toString(), txtEvento: evento.id.toString() });

        try {
            const response = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: bodyData });
            const html = await response.text();
            const $ = cheerio.load(html);

            $('td').each((index, element) => {
                const textoTd = $(element).text().trim();
                if (textoTd.toLowerCase().includes(nomeProcurado.toLowerCase())) {
                    enviarLog(`Registro encontrado no evento: ${evento.nome}`);
                    
                    const linhaTr = $(element).closest('tr');
                    const linkEncontrado = linhaTr.find('a').attr('href');
                    
                    let linkCompleto = "Link indisponivel";
                    if (linkEncontrado) {
                        linkCompleto = linkEncontrado.startsWith('/') 
                            ? `https://apl.utfpr.edu.br${linkEncontrado}` 
                            : linkEncontrado;
                    }

                    certificadosEncontrados.push({
                        nomeCertificado: textoTd,
                        campus: campusNome,
                        evento: evento.nome,
                        id: evento.id,
                        pagina: paginaAtual,
                        link: linkCompleto
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
            } else {
                temMaisPaginas = false; 
            }
        } catch (error) {
            enviarLog(`Erro no evento ${evento.id}: ${error.message}`);
            temMaisPaginas = false; 
        }
    }
}

async function buscarCertificados(nomeProcurado, ano, campusSelecionado, enviarLog) {
    let certificadosEncontrados = [];
    const tamanhoDoLote = 15; 
    
    const opcoesForm = await obterOpcoesFormulario();
    const listaTodosCampi = opcoesForm.campi;
    let campiParaBuscar = [];
    
    if (campusSelecionado === 'todos') {
        campiParaBuscar = listaTodosCampi;
    } else {
        const campusEspecifico = listaTodosCampi.find(c => c.id == campusSelecionado);
        if (campusEspecifico) campiParaBuscar.push(campusEspecifico);
    }

    for (const campus of campiParaBuscar) {
        enviarLog(`\n--- ACESSANDO CAMPUS: ${campus.nome.toUpperCase()} ---`);
        const listaDeEventos = await extrairEventosComRobo(ano, campus.id, enviarLog);

        if (listaDeEventos.length === 0) {
            enviarLog("Nenhum evento encontrado neste campus.");
            continue; 
        }

        for (let i = 0; i < listaDeEventos.length; i += tamanhoDoLote) {
            const lote = listaDeEventos.slice(i, i + tamanhoDoLote);
            enviarLog(`Processando lote ${Math.floor(i / tamanhoDoLote) + 1}... (${lote.length} eventos)`);
            
            const promessasDoLote = lote.map(evento => 
                processarEvento(evento, nomeProcurado, ano, campus.id, campus.nome, enviarLog, certificadosEncontrados)
            );
            
            await Promise.all(promessasDoLote);
            await new Promise(resolve => setTimeout(resolve, 1000));
        }
    }

    enviarLog("\n--- RELATORIO FINAL ---");
    
    if (certificadosEncontrados.length > 0) {
        enviarLog(`Total de certificados encontrados: ${certificadosEncontrados.length}\n`);
        
        certificadosEncontrados.forEach((cert, index) => {
            enviarLog(`[${index + 1}] Campus: ${cert.campus}`);
            enviarLog(`Evento: ${cert.evento} (ID: ${cert.id})`);
            enviarLog(`Nome: ${cert.nomeCertificado}`);
            enviarLog(`Pagina: ${cert.pagina}`);
            enviarLog(`Link: ${cert.link}\n`);
        });
    } else {
        enviarLog(`Nenhum resultado encontrado.`);
    }
    
    enviarLog("--- FIM DA BUSCA ---");
}

io.on('connection', (socket) => {
    socket.on('iniciar', async (dados) => {
        const enviarLog = (mensagem) => socket.emit('log', mensagem);
        await buscarCertificados(dados.nome, dados.ano, dados.campus, enviarLog);
    });
});

server.listen(3000, () => {
    console.log('Servidor rodando em http://localhost:3000');
});