# Buscador de Certificados UTFPR

Uma ferramenta automatizada em Node.js com interface web para varrer as listas públicas do Sistema de Gestão de Certificados Eletrônicos da UTFPR. O sistema utiliza automação de navegador para mapear os eventos disponíveis e realiza buscas em tempo real através de WebSockets.

## 🛠️ Tecnologias Utilizadas

* **Node.js** (Back-end)
* **Express** (Servidor Web)
* **Socket.io** (Comunicação em tempo real para o terminal virtual)
* **Puppeteer** (Automação de navegador headless para extração de IDs)
* **Cheerio** (Extração e varredura rápida de HTML)

## 📋 Pré-requisitos

Certifique-se de ter o **Node.js** instalado na sua máquina. É recomendada a versão **LTS (20 ou superior)** para total compatibilidade com o sistema de importação das bibliotecas atuais.

## 🚀 Instalação

1. Coloque os arquivos do projeto em uma pasta, garantindo que a estrutura esteja da seguinte forma:

   ```text
   /pasta-do-projeto
   ├── server.js
   └── /public
       └── index.html
   ```

2. Abra o terminal na raiz da pasta do projeto.

3. Instale todas as dependências necessárias executando:

   ```bash
   npm install express socket.io puppeteer cheerio
   ```

## 💻 Como Executar

1. Com as bibliotecas instaladas, inicie o servidor rodando o comando:

   ```bash
   node server.js
   ```

2. O terminal exibirá uma mensagem confirmando que o servidor está online na porta 3000.

## 🌐 Como Usar a Interface

1. Mantenha o terminal aberto rodando o servidor.

2. Abra o seu navegador e acesse o endereço local:

   ```text
   http://localhost:3000
   ```

3. Digite o nome (ou parte dele) que deseja buscar e o ano base dos eventos.

4. Clique em "Buscar Agora" e ao final da busca, os certificados encontrados aparecerão no relatório final.